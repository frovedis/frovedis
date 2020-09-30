#include "dfcolumn_impl.hpp"
#include "../text/datetime_to_words.hpp"

namespace frovedis {

using namespace std;

template class typed_dfcolumn<datetime>;

// defined in typed_dfcolumn_dic_string.cc
void dfcolumn_replace_nullstr(words& ws,
                              const std::vector<size_t>& nulls,
                              const std::string& nullstr);

node_local<words>
typed_dfcolumn<datetime>::as_words(size_t precision, // not used
                                   const std::string& datetime_fmt,
                                   bool quote_escape,
                                   const std::string& nullstr) {
  auto nl_words = val.map(+[](const std::vector<datetime_t>& v,
                              const std::string& fmt) {
                            return datetime_to_words(v, fmt);
                          }, broadcast(datetime_fmt));
  if(contain_nulls)
    nl_words.mapv(dfcolumn_replace_nullstr, nulls, broadcast(nullstr));
  if(quote_escape) nl_words.mapv(quote_and_escape);
  return nl_words;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::extract(node_local<std::vector<size_t>>& idx) {
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  auto retnulls = make_node_local_allocate<std::vector<size_t>>();
  if(contain_nulls) {
    ret->val = val.map(extract_helper<datetime_t>, idx, nulls, retnulls);
    ret->nulls = std::move(retnulls);
    ret->contain_nulls_check();
  } else {
    ret->val = val.map(extract_helper2<datetime_t>, idx);
    ret->nulls = std::move(retnulls);
  }
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::global_extract
(node_local<std::vector<size_t>>& global_idx,
 node_local<std::vector<size_t>>& to_store_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
  time_spent t(DEBUG);
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  auto exdata = val.map(global_extract_helper<datetime_t>, exchanged_idx);
  t.show("global_exract_helper: ");
  auto exchanged_back = alltoall_exchange(exdata).map(flatten<datetime_t>);
  t.show("alltoall_exchange + flatten: ");
  ret->val = exchanged_back.map
    (+[](std::vector<datetime_t>& val, std::vector<size_t>& idx) {
      auto valp = val.data();
      auto idxp = idx.data();
      auto size = idx.size();
      std::vector<datetime_t> ret(size);
      auto retp = ret.data();
#pragma cdir nodep
#pragma _NEC ivdep
      for(size_t i = 0 ; i < size; i++) {
        retp[i] = valp[idxp[i]];
      }
      return ret;
    }, to_store_idx);
  t.show("store: ");
  if(contain_nulls) {
    auto exnulls = nulls.map(global_extract_null_helper, exchanged_idx);
    t.show("global_extract_null_helper: ");
    auto exchanged_back_nulls = alltoall_exchange(exnulls);
    t.show("alltoall_exchange (nulls): ");
    auto null_exists = make_node_local_allocate<int>();
    auto nullhashes = exchanged_back_nulls.map(create_null_hash_from_partition,
                                               null_exists);
    t.show("create_null_hash_from_partition: ");
    ret->nulls = nullhashes.map(global_extract_null_helper2, global_idx,
                                null_exists);
    t.show("global_extract_null_helper2: ");
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::sort(node_local<std::vector<size_t>>& res_idx) {
  auto to_sort = val; // need to copy because of implace sort
  auto idx = make_node_local_allocate<std::vector<size_t>>();
  auto part_cand =
    to_sort.map(dfcolumn_sort_local<datetime_t>, idx).
    reduce(set_merge<datetime_t>);
  size_t nodesize = get_nodesize();
  std::vector<datetime_t> part(nodesize-1);
  size_t part_cand_size = part_cand.size();
  datetime_t* partp = &part[0];
  datetime_t* part_candp = &part_cand[0];
  for(size_t i = 0; i < nodesize - 1; i++) {
    auto idx = std::min((i+1) * nodesize, part_cand_size - 1);
    partp[i] = part_candp[idx];
  }
  auto part_val =
    make_node_local_allocate<std::vector<std::vector<datetime_t>>>();
  auto part_idx = make_node_local_allocate<std::vector<std::vector<size_t>>>();
  to_sort.mapv(dfcolumn_sort_partition<datetime_t>, idx, part_val, part_idx,
               broadcast(part));
  auto exchanged_val = alltoall_exchange(part_val);
  auto exchanged_idx = alltoall_exchange(part_idx);
  auto res_val = make_node_local_allocate<std::vector<datetime_t>>();
  res_idx = make_node_local_allocate<std::vector<size_t>>();
  // exchanged_idx will be destructed by set_multimerge_pair
  node_local<std::vector<std::vector<size_t>>> exnulls;
  if(contain_nulls)
    exnulls = nulls.map(global_extract_null_helper, exchanged_idx); 
  exchanged_val.mapv(set_multimerge_pair<datetime_t,size_t>, exchanged_idx,
                     res_val, res_idx);
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  ret->val = std::move(res_val);
  if(contain_nulls) {
    auto exchanged_back_nulls = alltoall_exchange(exnulls);
    auto null_exists = make_node_local_allocate<int>();
    auto nullhashes = exchanged_back_nulls.map(create_null_hash_from_partition,
                                               null_exists);
    ret->nulls = nullhashes.map(global_extract_null_helper2, res_idx,
                                null_exists);
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::sort_desc(node_local<std::vector<size_t>>& res_idx) {
  auto to_sort = val; // need to copy because of implace sort
  auto idx = make_node_local_allocate<std::vector<size_t>>();
  auto part_cand =
    to_sort.map(dfcolumn_sort_local_desc<datetime_t>, idx).
    reduce(set_merge_desc<datetime_t>);
  size_t nodesize = get_nodesize();
  std::vector<datetime_t> part(nodesize-1);
  size_t part_cand_size = part_cand.size();
  datetime_t* partp = &part[0];
  datetime_t* part_candp = &part_cand[0];
  for(size_t i = 0; i < nodesize - 1; i++) {
    auto idx = std::min((i+1) * nodesize, part_cand_size - 1);
    partp[i] = part_candp[idx];
  }
  auto part_val =
    make_node_local_allocate<std::vector<std::vector<datetime_t>>>();
  auto part_idx = make_node_local_allocate<std::vector<std::vector<size_t>>>();
  to_sort.mapv(dfcolumn_sort_partition_desc<datetime_t>, idx, part_val,
               part_idx, broadcast(part));
  auto exchanged_val = alltoall_exchange(part_val);
  auto exchanged_idx = alltoall_exchange(part_idx);
  auto res_val = make_node_local_allocate<std::vector<datetime_t>>();
  res_idx = make_node_local_allocate<std::vector<size_t>>();
  // exchanged_idx will be destructed by set_multimerge_pair
  node_local<std::vector<std::vector<size_t>>> exnulls;
  if(contain_nulls)
    exnulls = nulls.map(global_extract_null_helper, exchanged_idx); 
  exchanged_val.mapv(set_multimerge_pair_desc<datetime_t,size_t>, exchanged_idx,
                     res_val, res_idx);
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  ret->val = std::move(res_val);
  if(contain_nulls) {
    auto exchanged_back_nulls = alltoall_exchange(exnulls);
    auto null_exists = make_node_local_allocate<int>();
    auto nullhashes = exchanged_back_nulls.map(create_null_hash_from_partition,
                                               null_exists);
    ret->nulls = nullhashes.map(global_extract_null_helper2, res_idx,
                                null_exists);
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  return ret;
}

std::shared_ptr<dfcolumn> typed_dfcolumn<datetime>::
sort_with_idx(node_local<std::vector<size_t>>& idx,
              node_local<std::vector<size_t>>& res_idx) {
  auto to_sort = val; // need to copy because of implace sort
  auto part_cand =
    to_sort.map(dfcolumn_sort_with_idx_local<datetime_t>, idx).
    reduce(set_merge<datetime_t>);
  size_t nodesize = get_nodesize();
  std::vector<datetime_t> part(nodesize-1);
  size_t part_cand_size = part_cand.size();
  datetime_t* partp = &part[0];
  datetime_t* part_candp = &part_cand[0];
  for(size_t i = 0; i < nodesize - 1; i++) {
    auto idx = std::min((i+1) * nodesize, part_cand_size - 1);
    partp[i] = part_candp[idx];
  }
  auto part_val =
    make_node_local_allocate<std::vector<std::vector<datetime_t>>>();
  auto part_idx = make_node_local_allocate<std::vector<std::vector<size_t>>>();
  to_sort.mapv(dfcolumn_sort_partition<datetime_t>, idx, part_val, part_idx,
               broadcast(part));
  auto exchanged_val = alltoall_exchange(part_val);
  auto exchanged_idx = alltoall_exchange(part_idx);
  auto res_val = make_node_local_allocate<std::vector<datetime_t>>();
  res_idx = make_node_local_allocate<std::vector<size_t>>();
  // exchanged_idx will be destructed by set_multimerge_pair
  node_local<std::vector<std::vector<size_t>>> exnulls;
  if(contain_nulls)
    exnulls = nulls.map(global_extract_null_helper, exchanged_idx); 
  exchanged_val.mapv(set_multimerge_pair<datetime_t,size_t>, exchanged_idx,
                     res_val, res_idx);
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  ret->val = std::move(res_val);
  if(contain_nulls) {
    auto exchanged_back_nulls = alltoall_exchange(exnulls);
    auto null_exists = make_node_local_allocate<int>();
    auto nullhashes = exchanged_back_nulls.map(create_null_hash_from_partition,
                                               null_exists);
    ret->nulls = nullhashes.map(global_extract_null_helper2, res_idx,
                                null_exists);
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  return ret;
}

std::shared_ptr<dfcolumn> typed_dfcolumn<datetime>::
sort_with_idx_desc(node_local<std::vector<size_t>>& idx,
                   node_local<std::vector<size_t>>& res_idx) {
  auto to_sort = val; // need to copy because of implace sort
  auto part_cand =
    to_sort.map(dfcolumn_sort_with_idx_local_desc<datetime_t>, idx).
    reduce(set_merge_desc<datetime_t>);
  size_t nodesize = get_nodesize();
  std::vector<datetime_t> part(nodesize-1);
  size_t part_cand_size = part_cand.size();
  datetime_t* partp = &part[0];
  datetime_t* part_candp = &part_cand[0];
  for(size_t i = 0; i < nodesize - 1; i++) {
    auto idx = std::min((i+1) * nodesize, part_cand_size - 1);
    partp[i] = part_candp[idx];
  }
  auto part_val =
    make_node_local_allocate<std::vector<std::vector<datetime_t>>>();
  auto part_idx = make_node_local_allocate<std::vector<std::vector<size_t>>>();
  to_sort.mapv(dfcolumn_sort_partition_desc<datetime_t>, idx, part_val,
               part_idx, broadcast(part));
  auto exchanged_val = alltoall_exchange(part_val);
  auto exchanged_idx = alltoall_exchange(part_idx);
  auto res_val = make_node_local_allocate<std::vector<datetime_t>>();
  res_idx = make_node_local_allocate<std::vector<size_t>>();
  // exchanged_idx will be destructed by set_multimerge_pair
  node_local<std::vector<std::vector<size_t>>> exnulls;
  if(contain_nulls)
    exnulls = nulls.map(global_extract_null_helper, exchanged_idx); 
  exchanged_val.mapv(set_multimerge_pair_desc<datetime_t,size_t>, exchanged_idx,
                     res_val, res_idx);
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  ret->val = std::move(res_val);
  if(contain_nulls) {
    auto exchanged_back_nulls = alltoall_exchange(exnulls);
    auto null_exists = make_node_local_allocate<int>();
    auto nullhashes = exchanged_back_nulls.map(create_null_hash_from_partition,
                                               null_exists);
    ret->nulls = nullhashes.map(global_extract_null_helper2, res_idx,
                                null_exists);
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::group_by
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map) {
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  group_by_impl(val, nulls, local_idx, split_idx, hash_divide, merge_map,
                ret->val, ret->nulls);
  ret->contain_nulls_check();
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::multi_group_by_extract
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 bool check_nulls) {
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  ret->val =
    val.map(multi_group_by_extract_helper<datetime_t>, local_idx, split_idx);
  ret->contain_nulls = contain_nulls;
  if(contain_nulls && check_nulls) {
    ret->nulls = ret->val.map(+[](std::vector<datetime_t>& val) {
        return find_value(val, std::numeric_limits<datetime_t>::max());
      });
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  return ret;
}

std::shared_ptr<dfcolumn> 
typed_dfcolumn<datetime>::multi_group_by_exchange
(node_local<std::vector<std::vector<size_t>>>& hash_divide) {
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  ret->val = multi_group_by_exchange_helper(val, hash_divide);
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  ret->contain_nulls = contain_nulls;
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::max
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_agg = val.map(max_helper<datetime_t>, local_grouped_idx,
                           local_idx_split, hash_divide, nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<datetime_t>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         size_t row_size) {
      std::vector<datetime_t> newval(row_size);
      auto newvalp = newval.data();
      auto min = std::numeric_limits<datetime_t>::min();
      for(size_t i = 0; i < row_size; i++) {
        newvalp[i] = min;
      }
      for(size_t i = 0; i < exchanged.size(); i++) {
        auto currentp = exchanged[i].data();
        auto current_size = exchanged[i].size();
        auto merge_mapp = merge_map[i].data();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t j = 0; j < current_size; j++) {
          if(newvalp[merge_mapp[j]] < currentp[j])
            newvalp[merge_mapp[j]] = currentp[j];
        }
      }
      return newval;
    }, merge_map, row_sizes);
  ret->val = std::move(newval);
  if(contain_nulls) {
    ret->nulls = ret->val.map(+[](std::vector<datetime_t>& val) {
        auto nulls = find_value(val, std::numeric_limits<datetime_t>::min());
        auto valp = val.data();
        auto nullsp = nulls.data();
        auto size = nulls.size();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < size; i++)
          valp[nullsp[i]] = std::numeric_limits<datetime_t>::max();
        return nulls;
      });
    ret->contain_nulls_check();
  }
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::min
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_agg = val.map(min_helper<datetime_t>, local_grouped_idx,
                           local_idx_split, hash_divide, nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<datetime_t>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         size_t row_size) {
      std::vector<datetime_t> newval(row_size);
      auto newvalp = newval.data();
      auto max = std::numeric_limits<datetime_t>::max();
      for(size_t i = 0; i < row_size; i++) {
        newvalp[i] = max;
      }
      for(size_t i = 0; i < exchanged.size(); i++) {
        auto currentp = exchanged[i].data();
        auto current_size = exchanged[i].size();
        auto merge_mapp = merge_map[i].data();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t j = 0; j < current_size; j++) {
          if(newvalp[merge_mapp[j]] > currentp[j])
            newvalp[merge_mapp[j]] = currentp[j];
        }
      }
      return newval;
    }, merge_map, row_sizes);
  ret->val = std::move(newval);
  if(contain_nulls) {
    ret->nulls = ret->val.map(+[](std::vector<datetime_t>& val) {
        return find_value(val, std::numeric_limits<datetime_t>::max());
      });
    ret->contain_nulls_check();
  }
  return ret;
}

void typed_dfcolumn<datetime>::debug_print() {
  std::cout << "dtype: " << dtype() << std::endl;
  std::cout << "values: ";
  for(auto& i: val.gather()) {
    for(auto j: i) printf("%lx ", j);
    std::cout << ": ";
  }
  std::cout << "nulls: ";
  for(auto& i: nulls.gather()) {
    for(auto j: i) std::cout << j << " ";
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "contain_nulls: " << contain_nulls << std::endl;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::head(size_t limit) {
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  ret->val =
    val.template viewas_dvector<datetime_t>().head(limit).moveto_node_local();
  if(contain_nulls) {
    ret->nulls = limit_nulls_head(nulls, sizes(), limit);
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::tail(size_t limit) {
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  ret->val =
    val.template viewas_dvector<datetime_t>().tail(limit).moveto_node_local();
  auto new_sizes = ret->val.template viewas_dvector<datetime_t>().sizes();
  if(contain_nulls) {
    ret->nulls = limit_nulls_tail(nulls, sizes(), new_sizes, limit);
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  return ret;
}


}
