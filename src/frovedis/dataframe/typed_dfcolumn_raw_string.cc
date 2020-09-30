#include "dfcolumn_impl.hpp"
#include "../text/char_int_conv.hpp"

namespace frovedis {

using namespace std;

template class typed_dfcolumn<raw_string>;

vector<compressed_words>
compressed_words_align_as_helper(compressed_words& cws,
                                 vector<size_t>& alltoall_sizes) {
  auto nodesize = alltoall_sizes.size();
  vector<compressed_words> ret(nodesize);
  size_t current = 0;
  for(size_t i = 0; i < nodesize; i++) {
    size_t extract_size = alltoall_sizes[i];
    vector<size_t> to_extract(extract_size);
    auto to_extractp = to_extract.data();
    for(size_t j = 0; j < extract_size; j++) {
      to_extractp[j] = current + j;
    }
    ret[i] = cws.extract(to_extract);
    current += extract_size;
  }
  return ret;
}

void compressed_words_align_as(node_local<compressed_words>& comp_words,
                               const vector<size_t>& mysizes,
                               const vector<size_t>& dst) {
  if(dst.size() != get_nodesize()) 
    throw std::runtime_error
      ("align_as: size of dst is not equal to node size");
  size_t dsttotal = 0;
  size_t selftotal = 0;
  for(size_t i = 0; i < dst.size(); i++) dsttotal += dst[i];
  for(size_t i = 0; i < mysizes.size(); i++) selftotal += mysizes[i];
  if(dsttotal != selftotal)
    throw std::runtime_error
      ("align_as: total size of src and dst does not match");
  bool is_same = true;
  for(size_t i = 0; i < dst.size(); i++) {
    if(dst[i] != mysizes[i]) {
      is_same = false;
      break;
    }
  }
  if(is_same) return;
  // align_as_calc_alltoall_sizes is in dvector.hpp
  auto alltoall_sizes = broadcast(mysizes).map(align_as_calc_alltoall_sizes,
                                               broadcast(dst));
  auto comp_words_toex = comp_words.map(compressed_words_align_as_helper,
                                        alltoall_sizes);
  comp_words.mapv(+[](compressed_words& cw){cw.clear();});
  auto comp_words_exchanged = alltoall_exchange(comp_words_toex);
  comp_words_toex.mapv(+[](vector<compressed_words>& vcw)
                       {for(auto& cw: vcw) cw.clear();});
  comp_words = comp_words_exchanged.map(merge_multi_compressed_words);
}

void typed_dfcolumn<raw_string>::align_as(const vector<size_t>& dst) {
  // overwrite
  compressed_words_align_as(comp_words, sizes(), dst);
}

// use "NULL" as string for NULL, which will not be used directly anyway
void raw_string_append_nulls_helper(compressed_words& cw,
                                    std::vector<size_t>& to_append,
                                    std::vector<size_t>& nulls) {
  auto num_words = cw.num_words();
  size_t to_append_size = to_append.size();
  words null_words;
  string nullstr = "NULL";
  auto nullstr_size = nullstr.size();
  null_words.chars = char_to_int(nullstr);
  null_words.starts.resize(to_append_size);
  null_words.lens.resize(to_append_size);
  auto startsp = null_words.starts.data();
  auto lensp = null_words.lens.data();
  for(size_t i = 0; i < to_append_size; i++) {
    startsp[i] = 0;
    lensp[i] = nullstr_size;
  }
  auto comp_null_words = make_compressed_words(null_words);
  auto newcw = merge_compressed_words(cw, comp_null_words);
  cw.cwords.swap(newcw.cwords);
  cw.lens.swap(newcw.lens);
  cw.lens_num.swap(newcw.lens_num);
  cw.order.swap(newcw.order);
  auto nulls_size = nulls.size();
  auto new_nulls_size = nulls_size + to_append_size;
  std::vector<size_t> new_nulls(new_nulls_size);
  auto new_nullsp = new_nulls.data();
  auto nullsp = nulls.data();
  for(size_t i = 0; i < nulls_size; i++) new_nullsp[i] = nullsp[i];
  for(size_t i = 0; i < to_append_size; i++) {
    new_nullsp[nulls_size + i] = num_words + i;
  }
  nulls.swap(new_nulls);
}

void typed_dfcolumn<raw_string>::append_nulls
(node_local<std::vector<size_t>>& to_append) {
  comp_words.mapv(raw_string_append_nulls_helper, to_append, nulls);
  contain_nulls = true;
}

node_local<std::vector<size_t>>
typed_dfcolumn<raw_string>::
filter_like(const std::string& pattern, int wild_card) {
  auto hit =
    comp_words.map(+[](const compressed_words& cws, 
                       const std::string& p, int wild_card) {
                     return like(cws.decompress(), p, wild_card);
                   }, broadcast(pattern), broadcast(wild_card));
  if(contain_nulls) {
    return hit.map(set_difference<size_t>, nulls);
  } else {
    return hit;
  }
}

node_local<std::vector<size_t>>
typed_dfcolumn<raw_string>::
filter_not_like(const std::string& pattern, int wild_card) {
  auto hit =
    comp_words.map(+[](const compressed_words& cws, 
                       const std::string& p, int wild_card) {
        auto num_words = cws.num_words();
        auto hit_like = like(cws.decompress(), p, wild_card);
        std::vector<size_t> iota(num_words);
        auto iotap = iota.data();
        for(size_t i = 0; i < num_words; i++) iotap[i] = i;
        return set_difference(iota, hit_like);
      }, broadcast(pattern), broadcast(wild_card));
  if(contain_nulls) {
    return hit.map(set_difference<size_t>, nulls);
  } else {
    return hit;
  }
}

node_local<std::vector<size_t>>
typed_dfcolumn<raw_string>::filter_is_null() {return nulls;}

node_local<std::vector<size_t>>
typed_dfcolumn<raw_string>::filter_is_not_null() {
  auto local_idx = get_local_index();
  if(contain_nulls)
    return local_idx.map(set_difference<size_t>, nulls);
  else return local_idx;
}

node_local<std::vector<size_t>> typed_dfcolumn<raw_string>::get_local_index() {
  return comp_words.map(+[](const compressed_words& cws) {
      auto num_words = cws.num_words();
      std::vector<size_t> r(num_words);
      auto rp = r.data();
      for(size_t i = 0; i < num_words; i++) rp[i] = i;
      return r;
    });
}

compressed_words
raw_string_extract_helper(const compressed_words& cws,
                          const std::vector<size_t>& idx,
                          const std::vector<size_t>& nulls,
                          std::vector<size_t>& retnulls) {
  compressed_words ret = cws.extract(idx);
  size_t size = idx.size();
  size_t nullssize = nulls.size();
  if(nullssize != 0) {
    std::vector<int> dummy(nullssize);
    auto nullhash = unique_hashtable<size_t, int>(nulls, dummy);
    auto isnull = nullhash.check_existence(idx);
    int* isnullp = &isnull[0];
    std::vector<size_t> rettmp(size);
    size_t* rettmpp = &rettmp[0];
    size_t current = 0;
    for(size_t i = 0; i < size; i++) {
      if(isnullp[i] == 1) {
        rettmpp[current++] = i;
      }
    }
    retnulls.resize(current);
    size_t* retnullsp = &retnulls[0];
    for(size_t i = 0; i < current; i++) {
      retnullsp[i] = rettmpp[i];
    }
  }
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<raw_string>::extract(node_local<std::vector<size_t>>& idx) {
  auto ret = std::make_shared<typed_dfcolumn<raw_string>>();
  auto retnulls = make_node_local_allocate<std::vector<size_t>>();
  if(contain_nulls) {
    ret->comp_words = comp_words.map(raw_string_extract_helper, idx,
                                     nulls, retnulls);
    ret->nulls = std::move(retnulls);
    ret->contain_nulls_check();
  } else {
    ret->comp_words = comp_words.map(+[](const compressed_words& cws,
                                         const std::vector<size_t>& idx)
                                     {return cws.extract(idx);}, idx);
    ret->nulls = std::move(retnulls);
  }
  return ret;
}

std::vector<compressed_words>
raw_string_global_extract_helper(const compressed_words& cws,
                                 const std::vector<std::vector<size_t>>&
                                 exchanged_idx) {
  size_t size = exchanged_idx.size();
  std::vector<compressed_words> ret(size);
  for(size_t i = 0; i < size; i++) {
    ret[i] = cws.extract(exchanged_idx[i]);
  }
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<raw_string>::global_extract
(node_local<std::vector<size_t>>& global_idx,
 node_local<std::vector<size_t>>& to_store_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
  auto ret = std::make_shared<typed_dfcolumn<raw_string>>();
  auto exdata = comp_words.map(raw_string_global_extract_helper, exchanged_idx);
  auto exchanged_back =
    alltoall_exchange(exdata).map(merge_multi_compressed_words);
  ret->comp_words =
    exchanged_back.map(+[](const compressed_words& cw,
                           const std::vector<size_t>& idx)
                       {return cw.extract(idx);}, to_store_idx);
  if(contain_nulls) {
    auto exnulls = nulls.map(global_extract_null_helper, exchanged_idx);
    auto exchanged_back_nulls = alltoall_exchange(exnulls);
    auto null_exists = make_node_local_allocate<int>();
    auto nullhashes = exchanged_back_nulls.map(create_null_hash_from_partition,
                                               null_exists);
    ret->nulls = nullhashes.map(global_extract_null_helper2, global_idx,
                                null_exists);
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  return ret;
}

size_t typed_dfcolumn<raw_string>::count() {
  size_t size = this->size();
  if(contain_nulls) {
    size_t nullsize = nulls.template viewas_dvector<size_t>().size();
    return size - nullsize;
  } else return size;
}

// representation of NULL is not normalized, which is OK because of _nulls file
void typed_dfcolumn<raw_string>::save(const std::string& file) {
  comp_words.map(+[](const compressed_words& cws) {
      vector<size_t> new_starts; // not used
      return int_to_vchar(concat_words(cws.decompress(), "\n", new_starts));
    }).moveto_dvector<char>().savebinary(file);
  auto dv_nulls = nulls.template viewas_dvector<size_t>();
  auto sizes = this->sizes();
  auto sizesp = sizes.data();
  std::vector<size_t> pxsizes(sizes.size());
  auto pxsizesp = pxsizes.data();
  auto pxsizessize = pxsizes.size();
  for(size_t i = 1; i < pxsizessize; i++) {
    pxsizesp[i] += pxsizesp[i-1] + sizesp[i-1];
  }
  auto nl_sizes = make_node_local_scatter(pxsizes);
  dv_nulls.map<size_t>(shift_local_index(), nl_sizes).
    savebinary(file+"_nulls");
}

compressed_words raw_string_head_helper(const compressed_words& cws,
                                        size_t to_trunc) {
  auto num_words = cws.num_words();
  if(num_words > to_trunc) {
    std::vector<size_t> idx(to_trunc);
    auto idxp = idx.data();
    for(size_t i = 0; i < to_trunc; i++) idxp[i] = i;
    return cws.extract(idx);
  } else return cws;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<raw_string>::head(size_t limit) {
  auto ret = std::make_shared<typed_dfcolumn<raw_string>>();
  auto all_sizes = sizes();
  size_t node_size = get_nodesize();
  std::vector<size_t> trunc_sizes(node_size);
  auto to_trunc_size = limit;
  for(size_t i = 0; i < node_size; i++) {
    trunc_sizes[i] = std::min(to_trunc_size, all_sizes[i]);
    if(to_trunc_size > all_sizes[i]) {
      to_trunc_size -= all_sizes[i];
    } else to_trunc_size = 0;
  }
  auto dtrunc_sizes = make_node_local_scatter(trunc_sizes);
  ret->comp_words = comp_words.map(raw_string_head_helper, dtrunc_sizes);

  if(contain_nulls) {
    ret->nulls = limit_nulls_head(nulls, sizes(), limit);
    ret->contain_nulls_check();    
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  return ret;
}


compressed_words raw_string_tail_helper(const compressed_words& cws,
                                        size_t to_trunc) {
  auto num_words = cws.num_words();
  if(num_words > to_trunc) {
    auto shift = num_words - to_trunc;
    std::vector<size_t> idx(to_trunc);
    auto idxp = idx.data();
    for(size_t i = 0; i < to_trunc; i++) idxp[i] = shift + i;
    return cws.extract(idx);
  } else return cws;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<raw_string>::tail(size_t limit) {
  auto ret = std::make_shared<typed_dfcolumn<raw_string>>();
  auto all_sizes = sizes();
  size_t node_size = get_nodesize();
  std::vector<size_t> trunc_sizes(node_size);
  auto to_trunc_size = limit;
  for(size_t i = 0; i < node_size; i++) {
    auto current_node = node_size - i - 1;
    trunc_sizes[current_node] =
      std::min(to_trunc_size, all_sizes[current_node]);
    if(to_trunc_size > all_sizes[current_node]) {
      to_trunc_size -= all_sizes[current_node];
    } else to_trunc_size = 0;
  }
  auto dtrunc_sizes = make_node_local_scatter(trunc_sizes);
  ret->comp_words = comp_words.map(raw_string_tail_helper, dtrunc_sizes);

  if(contain_nulls) {
    ret->nulls = limit_nulls_tail(nulls, all_sizes, trunc_sizes, limit);
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  return ret;
}

// defined in typed_dfcolumn_dic_string.cc
void dfcolumn_replace_nullstr(words& ws,
                              const std::vector<size_t>& nulls,
                              const std::string& nullstr);
node_local<words>
typed_dfcolumn<raw_string>::as_words(size_t precision, // not used
                                     const std::string& datetime_fmt, // not used
                                     bool quote_escape,
                                     const std::string& nullstr) {
  auto nl_words = comp_words.map(+[](const compressed_words& cws)
                                 {return cws.decompress();});
  if(contain_nulls)
    nl_words.mapv(dfcolumn_replace_nullstr, nulls, broadcast(nullstr));
  if(quote_escape) nl_words.mapv(quote_and_escape);
  return nl_words;
}

void typed_dfcolumn<raw_string>::debug_print() {
  std::cout << "dtype: " << dtype() << std::endl;
  for(auto& i: comp_words.gather()) {
      i.print();
  }
  std::cout << "nulls: ";
  for(auto& i: nulls.gather()) {
    for(auto j: i) std::cout << j << " ";
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "contain_nulls: " << contain_nulls << std::endl;
}

void typed_dfcolumn<raw_string>::contain_nulls_check() {
  if(nulls.template viewas_dvector<size_t>().size() == 0)
    contain_nulls = false;
  else contain_nulls = true;
}

void typed_dfcolumn<raw_string>::init(node_local<words>& ws,
                                      bool allocate_nulls) {
  auto cws = ws.map(make_compressed_words);
  init_compressed(std::move(cws), allocate_nulls);
}

void typed_dfcolumn<raw_string>::init_compressed
(node_local<compressed_words>&& cws, bool allocate_nulls) {
  if(allocate_nulls) nulls = make_node_local_allocate<vector<size_t>>();
  comp_words = std::move(cws);
}

void typed_dfcolumn<raw_string>::init_compressed
(node_local<compressed_words>& cws, bool allocate_nulls) {
  if(allocate_nulls) nulls = make_node_local_allocate<vector<size_t>>();
  comp_words = cws;
}

size_t typed_dfcolumn<raw_string>::size() {
  auto sizes = this->sizes();
  size_t ret = 0;
  auto sizesp = sizes.data();
  auto sizes_size = sizes.size();
  for(size_t i = 0; i < sizes_size; i++) ret += sizesp[i];
  return ret;
}

std::vector<size_t> typed_dfcolumn<raw_string>::sizes() {
  return comp_words.map(+[](const compressed_words& cws)
                        {return cws.num_words();}).gather();
}

}
