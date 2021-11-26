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
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0 ; i < size; i++) {
        retp[i] = valp[idxp[i]];
      }
      return ret;
    }, to_store_idx);
  t.show("store: ");
  if(contain_nulls) {
    /*
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
    */
    // assume that val always contains max() iff NULL; this is much faster
    // datetime_t does not become max()
    ret->nulls = ret->val.map(+[](std::vector<datetime_t>& val) {
        return vector_find_eq(val, std::numeric_limits<datetime_t>::max());
      });
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
  exchanged_val.mapv(set_multimerge_pair<datetime_t,size_t>, exchanged_idx,
                     res_val, res_idx);
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  if(contain_nulls) {
    // if contain_nulls, sorted column is not reused,
    // since calculating null posistion is not trivial
    ret->val = make_node_local_allocate<std::vector<datetime_t>>();
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  } else {
    ret->val = std::move(res_val);
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
  exchanged_val.mapv(set_multimerge_pair_desc<datetime_t,size_t>, exchanged_idx,
                     res_val, res_idx);
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  if(contain_nulls) {
    // if contain_nulls, sorted column is not reused,
    // since calculating null posistion is not trivial
    ret->val = make_node_local_allocate<std::vector<datetime_t>>();
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  } else {
    ret->val = std::move(res_val);
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
  exchanged_val.mapv(set_multimerge_pair<datetime_t,size_t>, exchanged_idx,
                     res_val, res_idx);
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  if(contain_nulls) {
    // if contain_nulls, sorted column is not reused,
    // since calculating null posistion is not trivial
    ret->val = make_node_local_allocate<std::vector<datetime_t>>();
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  } else {
    ret->val = std::move(res_val);
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
  exchanged_val.mapv(set_multimerge_pair_desc<datetime_t,size_t>, exchanged_idx,
                     res_val, res_idx);
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  if(contain_nulls) {
    // if contain_nulls, sorted column is not reused,
    // since calculating null posistion is not trivial
    ret->val = make_node_local_allocate<std::vector<datetime_t>>();
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  } else {
    ret->val = std::move(res_val);
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
      auto min = std::numeric_limits<datetime_t>::lowest();
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
        auto nulls = find_value(val, std::numeric_limits<datetime_t>::lowest());
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
  std::cout << std::endl;
  std::cout << "nulls: ";
  for(auto& i: nulls.gather()) {
    for(auto j: i) std::cout << j << " ";
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "contain_nulls: " << contain_nulls << std::endl;
  if(spillable) {
    std::cout << "spill_initialized: " << spill_initialized << std::endl;
    std::cout << "already_spilled_to_disk: " << already_spilled_to_disk
              << std::endl;
    std::cout << "cleared: " << cleared << std::endl;
    std::cout << "spill_state: " << spill_state << std::endl;
    std::cout << "spill_size_cache: " << spill_size_cache << std::endl;
    if(spill_initialized) {
      std::cout << "spill_path: ";
      auto spill_paths = spill_path.gather();
      for(auto& p: spill_paths) std::cout << p << ", ";
      std::cout << std::endl;
    }
  }
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

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::union_columns
(const std::vector<std::shared_ptr<dfcolumn>>& cols) {
  auto cols_size = cols.size();
  if(cols_size == 0) {
    return std::make_shared<typed_dfcolumn<datetime>>(val, nulls);
  }
  std::vector<std::shared_ptr<typed_dfcolumn<datetime>>> rights(cols_size);
  for(size_t i = 0; i < cols_size; i++) {
    rights[i] = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(cols[i]);
    if(!rights[i]) throw std::runtime_error("union_columns: different type");
  }
  auto val_colsp =
    make_node_local_allocate<std::vector<std::vector<datetime_t>*>>();
  auto nulls_colsp =
    make_node_local_allocate<std::vector<std::vector<size_t>*>>();
  val.mapv(+[](std::vector<datetime_t>& val,
               std::vector<std::vector<datetime_t>*>& val_colsp)
           {val_colsp.push_back(&val);}, val_colsp);
  nulls.mapv(+[](std::vector<size_t>& nulls,
                 std::vector<std::vector<size_t>*>& nulls_colsp)
             {nulls_colsp.push_back(&nulls);}, nulls_colsp);
  for(size_t i = 0; i < cols_size; i++) {
    rights[i]->val.mapv(+[](std::vector<datetime_t>& val,
                            std::vector<std::vector<datetime_t>*>& val_colsp)
                        {val_colsp.push_back(&val);}, val_colsp);
    rights[i]->nulls.mapv(+[](std::vector<size_t>& nulls,
                              std::vector<std::vector<size_t>*>& nulls_colsp)
                          {nulls_colsp.push_back(&nulls);}, nulls_colsp);
  }
  auto newval = make_node_local_allocate<std::vector<datetime_t>>();
  auto newnulls = make_node_local_allocate<std::vector<size_t>>();
  newval.mapv(union_columns_helper<datetime_t>,
              newnulls, val_colsp, nulls_colsp);
  return std::make_shared<typed_dfcolumn<datetime>>(std::move(newval),
                                                    std::move(newnulls));
}

node_local<std::vector<size_t>>
typed_dfcolumn<datetime>::filter_eq(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(right);
  if(!right2) throw std::runtime_error("filter_eq: column types are different");
  auto filtered_idx = val.map(filter_eq_helper<datetime_t, datetime_t>,
                              right2->val);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<datetime>::filter_eq_immed(std::shared_ptr<dfscalar>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfscalar<datetime_t>>(right);
  if(!right2) throw std::runtime_error("filter_eq_immed: types are different");
  auto filtered_idx = val.map(filter_eq_immed_helper<datetime_t, datetime_t>,
                              broadcast(right2->val));
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<datetime>::filter_neq(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(right);
  if(!right2) 
    throw std::runtime_error("filter_neq: column types are different");
  auto filtered_idx = val.map(filter_neq_helper<datetime_t, datetime_t>,
                              right2->val);
  if(right2->contain_nulls)
    filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<datetime>::filter_neq_immed(std::shared_ptr<dfscalar>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfscalar<datetime_t>>(right);
  if(!right2) throw std::runtime_error("filter_neq_immed: types are different");
  auto filtered_idx = val.map(filter_neq_immed_helper<datetime_t, datetime_t>,
                              broadcast(right2->val));
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<datetime>::filter_lt(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(right);
  if(!right2) throw std::runtime_error("filter_lt: column types are different");
  auto filtered_idx = val.map(filter_lt_helper<datetime_t, datetime_t>,
                              right2->val);
  if(right2->contain_nulls)
    filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<datetime>::filter_lt_immed(std::shared_ptr<dfscalar>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfscalar<datetime_t>>(right);
  if(!right2) throw std::runtime_error("filter_lt_immed: types are different");
  auto filtered_idx = val.map(filter_lt_immed_helper<datetime_t, datetime_t>,
                              broadcast(right2->val));
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<datetime>::filter_le(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(right);
  if(!right2) throw std::runtime_error("filter_le: column types are different");
  auto filtered_idx = val.map(filter_le_helper<datetime_t, datetime_t>,
                              right2->val);
  if(right2->contain_nulls)
    filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<datetime>::filter_le_immed(std::shared_ptr<dfscalar>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfscalar<datetime_t>>(right);
  if(!right2) throw std::runtime_error("filter_le_immed: types are different");
  auto filtered_idx = val.map(filter_le_immed_helper<datetime_t, datetime_t>,
                              broadcast(right2->val));
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<datetime>::filter_gt(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(right);
  if(!right2) throw std::runtime_error("filter_gt: column types are different");
  auto filtered_idx = val.map(filter_gt_helper<datetime_t, datetime_t>,
                              right2->val);
  if(right2->contain_nulls)
    filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<datetime>::filter_gt_immed(std::shared_ptr<dfscalar>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfscalar<datetime_t>>(right);
  if(!right2) throw std::runtime_error("filter_gt_immed: types are different");
  auto filtered_idx = val.map(filter_gt_immed_helper<datetime_t, datetime_t>,
                              broadcast(right2->val));
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<datetime>::filter_ge(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(right);
  if(!right2) throw std::runtime_error("filter_ge: column types are different");
  auto filtered_idx = val.map(filter_ge_helper<datetime_t, datetime_t>,
                              right2->val);
  if(right2->contain_nulls)
    filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<datetime>::filter_ge_immed(std::shared_ptr<dfscalar>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfscalar<datetime_t>>(right);
  if(!right2) throw std::runtime_error("filter_ge_immed: types are different");
  auto filtered_idx = val.map(filter_ge_immed_helper<datetime_t, datetime_t>,
                              broadcast(right2->val));
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

struct datetime_extract_helper {
  datetime_extract_helper(){}
  datetime_extract_helper(datetime_type type) : type(type) {}
  std::vector<int> operator()(const std::vector<datetime_t>& d) {
    auto size = d.size();
    std::vector<int> ret(size);
    auto dp = d.data();
    auto retp = ret.data();
    if(type == datetime_type::year) {
      for(size_t i = 0; i < size; i++) retp[i] = year_from_datetime(dp[i]);
    } else if(type == datetime_type::month) {
      for(size_t i = 0; i < size; i++) retp[i] = month_from_datetime(dp[i]);
    } else if(type == datetime_type::day) {
      for(size_t i = 0; i < size; i++) retp[i] = day_from_datetime(dp[i]);
    } else if(type == datetime_type::hour) {
      for(size_t i = 0; i < size; i++) retp[i] = hour_from_datetime(dp[i]);
    } else if(type == datetime_type::minute) {
      for(size_t i = 0; i < size; i++) retp[i] = minute_from_datetime(dp[i]);
    } else if(type == datetime_type::second) {
      for(size_t i = 0; i < size; i++) retp[i] = second_from_datetime(dp[i]);
    } else if(type == datetime_type::nanosecond) {
      for(size_t i = 0; i < size; i++)
        retp[i] = nanosecond_from_datetime(dp[i]);
    } else if(type == datetime_type::quarter) {
      for(size_t i = 0; i < size; i++) retp[i] = quarter_from_datetime(dp[i]);
    } else if(type == datetime_type::dayofweek) {
      for(size_t i = 0; i < size; i++) retp[i] = dayofweek_from_datetime(dp[i]);
    } else if(type == datetime_type::dayofyear) {
      for(size_t i = 0; i < size; i++) retp[i] = dayofyear_from_datetime(dp[i]);
    } else if(type == datetime_type::weekofyear) {
      for(size_t i = 0; i < size; i++)
        retp[i] = weekofyear_from_datetime(dp[i]);
    } else throw std::runtime_error("unsupported datetime_type");
    return ret;
  }
  
  datetime_type type;
  SERIALIZE(type)
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_extract(datetime_type type) {
  auto ex = val.map(datetime_extract_helper(type)).
    mapv(+[](std::vector<int>& v, std::vector<size_t>& nulls) {
        auto vp = v.data();
        auto nullsp = nulls.data();
        auto nulls_size = nulls.size();
        auto max = std::numeric_limits<int>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < nulls_size; i++) {
          vp[nullsp[i]] = max;
        }
      }, nulls);
  return std::make_shared<typed_dfcolumn<int>>(std::move(ex), nulls);
}


struct datetime_diff_helper {
  datetime_diff_helper(){}
  datetime_diff_helper(datetime_type type) : type(type) {}
  std::vector<int> operator()(const std::vector<datetime_t>& d1,
                              const std::vector<datetime_t>& d2) {
    auto size = d1.size();
    std::vector<int> ret(size);
    auto d1p = d1.data();
    auto d2p = d2.data();
    auto retp = ret.data();
    if(type == datetime_type::year) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_diff_year(d1p[i], d2p[i]);
    } else if(type == datetime_type::month) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_month(d1p[i], d2p[i]);
    } else if(type == datetime_type::day) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_day(d1p[i], d2p[i]);
    } else if(type == datetime_type::hour) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_hour(d1p[i], d2p[i]);
    } else if(type == datetime_type::minute) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_minute(d1p[i], d2p[i]);
    } else if(type == datetime_type::second) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_second(d1p[i], d2p[i]);
    } else if(type == datetime_type::nanosecond) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_diff_nanosecond(d1p[i], d2p[i]);
    } else throw std::runtime_error("unsupported datetime_type");
    return ret;
  }
  
  datetime_type type;
  SERIALIZE(type)
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_diff(const std::shared_ptr<dfcolumn>& right,
                                        datetime_type type) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(right);
  if(!right2) throw std::runtime_error("datetime_diff: type mismatch");
  auto newval = val.map(datetime_diff_helper(type), right2->val);
  if(contain_nulls || right2->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right2->nulls);
    newval.mapv(reset_null<int>, newnulls);
    return std::make_shared<typed_dfcolumn<int>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

struct datetime_diff_im_helper {
  datetime_diff_im_helper(){}
  datetime_diff_im_helper(datetime_type type, datetime_t im)
    : type(type), im(im) {}
  std::vector<int> operator()(const std::vector<datetime_t>& d) {
    auto size = d.size();
    std::vector<int> ret(size);
    auto dp = d.data();
    auto retp = ret.data();
    if(type == datetime_type::year) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_diff_year(dp[i], im);
    } else if(type == datetime_type::month) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_month(dp[i], im);
    } else if(type == datetime_type::day) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_day(dp[i], im);
    } else if(type == datetime_type::hour) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_hour(dp[i], im);
    } else if(type == datetime_type::minute) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_minute(dp[i], im);
    } else if(type == datetime_type::second) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_second(dp[i], im);
    } else if(type == datetime_type::nanosecond) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_diff_nanosecond(dp[i], im);
    } else throw std::runtime_error("unsupported datetime_type");
    return ret;
  }
  
  datetime_type type;
  datetime_t im;
  SERIALIZE(type, im)
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_diff_im(datetime_t right,
                                           datetime_type type) {
  auto newval = val.map(datetime_diff_im_helper(type, right));
  if(contain_nulls) {
    newval.mapv(reset_null<int>, nulls);
    return std::make_shared<typed_dfcolumn<int>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

struct rdatetime_diff_im_helper {
  rdatetime_diff_im_helper(){}
  rdatetime_diff_im_helper(datetime_type type, datetime_t im)
    : type(type), im(im) {}
  std::vector<int> operator()(const std::vector<datetime_t>& d) {
    auto size = d.size();
    std::vector<int> ret(size);
    auto dp = d.data();
    auto retp = ret.data();
    if(type == datetime_type::year) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_diff_year(im, dp[i]);
    } else if(type == datetime_type::month) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_month(im, dp[i]);
    } else if(type == datetime_type::day) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_day(im, dp[i]);
    } else if(type == datetime_type::hour) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_hour(im, dp[i]);
    } else if(type == datetime_type::minute) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_minute(im, dp[i]);
    } else if(type == datetime_type::second) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_second(im, dp[i]);
    } else if(type == datetime_type::nanosecond) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_diff_nanosecond(im, dp[i]);
    } else throw std::runtime_error("unsupported datetime_type");
    return ret;
  }
  
  datetime_type type;
  datetime_t im;
  SERIALIZE(type, im)
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::rdatetime_diff_im(datetime_t right,
                                            datetime_type type) {
  auto newval = val.map(rdatetime_diff_im_helper(type, right));
  if(contain_nulls) {
    newval.mapv(reset_null<int>, nulls);
    return std::make_shared<typed_dfcolumn<int>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

struct datetime_add_helper {
  datetime_add_helper(){}
  datetime_add_helper(datetime_type type) : type(type) {}
  std::vector<datetime_t> operator()(const std::vector<datetime_t>& d1,
                                     const std::vector<int>& d2) {
    auto size = d1.size();
    std::vector<datetime_t> ret(size);
    auto d1p = d1.data();
    auto d2p = d2.data();
    auto retp = ret.data();
    if(type == datetime_type::year) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_add_year(d1p[i], d2p[i]);
    } else if(type == datetime_type::month) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_add_month(d1p[i], d2p[i]);
    } else if(type == datetime_type::day) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_add_day(d1p[i], d2p[i]);
    } else if(type == datetime_type::hour) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_add_hour(d1p[i], d2p[i]);
    } else if(type == datetime_type::minute) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_add_minute(d1p[i], d2p[i]);
    } else if(type == datetime_type::second) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_add_second(d1p[i], d2p[i]);
    } else if(type == datetime_type::nanosecond) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_add_nanosecond(d1p[i], d2p[i]);
    } else throw std::runtime_error("unsupported datetime_type");
    return ret;
  }
  
  datetime_type type;
  SERIALIZE(type)
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_add(const std::shared_ptr<dfcolumn>& right,
                                       datetime_type type) {
  auto right2 =
    std::dynamic_pointer_cast<typed_dfcolumn<int>>(right->type_cast("int"));
  auto newval = val.map(datetime_add_helper(type), right2->val);
  if(contain_nulls || right2->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right2->nulls);
    newval.mapv(reset_null<datetime_t>, newnulls);
    return std::make_shared<typed_dfcolumn<datetime>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<datetime_t>();
    return std::make_shared<typed_dfcolumn<datetime>>(std::move(dvval));
  }
}


struct datetime_add_im_helper {
  datetime_add_im_helper(){}
  datetime_add_im_helper(datetime_type type, int val)
    : type(type), val(val) {}
  std::vector<datetime_t> operator()(const std::vector<datetime_t>& d) {
    auto size = d.size();
    std::vector<datetime_t> ret(size);
    auto dp = d.data();
    auto retp = ret.data();
    if(type == datetime_type::year) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_add_year(dp[i], val);
    } else if(type == datetime_type::month) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_add_month(dp[i], val);
    } else if(type == datetime_type::day) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_add_day(dp[i], val);
    } else if(type == datetime_type::hour) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_add_hour(dp[i], val);
    } else if(type == datetime_type::minute) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_add_minute(dp[i], val);
    } else if(type == datetime_type::second) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_add_second(dp[i], val);
    } else if(type == datetime_type::nanosecond) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_add_nanosecond(dp[i], val);
    } else throw std::runtime_error("unsupported datetime_type");
    return ret;
  }
  
  datetime_type type;
  int val;
  SERIALIZE(type, val)
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_add_im(int right,
                                          datetime_type type) {
  auto newval = val.map(datetime_add_im_helper(type, right));
  if(contain_nulls) {
    newval.mapv(reset_null<datetime_t>, nulls);
    auto newnulls = nulls;
    return std::make_shared<typed_dfcolumn<datetime>>(std::move(newval),
                                                      std::move(nulls));
  } else {
    auto dvval = newval.template moveto_dvector<datetime_t>();
    return std::make_shared<typed_dfcolumn<datetime>>(std::move(dvval));
  }
}


struct datetime_sub_helper {
  datetime_sub_helper(){}
  datetime_sub_helper(datetime_type type) : type(type) {}
  std::vector<datetime_t> operator()(const std::vector<datetime_t>& d1,
                                     const std::vector<int>& d2) {
    auto size = d1.size();
    std::vector<datetime_t> ret(size);
    auto d1p = d1.data();
    auto d2p = d2.data();
    auto retp = ret.data();
    if(type == datetime_type::year) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_sub_year(d1p[i], d2p[i]);
    } else if(type == datetime_type::month) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_sub_month(d1p[i], d2p[i]);
    } else if(type == datetime_type::day) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_sub_day(d1p[i], d2p[i]);
    } else if(type == datetime_type::hour) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_sub_hour(d1p[i], d2p[i]);
    } else if(type == datetime_type::minute) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_sub_minute(d1p[i], d2p[i]);
    } else if(type == datetime_type::second) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_sub_second(d1p[i], d2p[i]);
    } else if(type == datetime_type::nanosecond) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_sub_nanosecond(d1p[i], d2p[i]);
    } else throw std::runtime_error("unsupported datetime_type");
    return ret;
  }
  
  datetime_type type;
  SERIALIZE(type)
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_sub(const std::shared_ptr<dfcolumn>& right,
                                       datetime_type type) {
  auto right2 =
    std::dynamic_pointer_cast<typed_dfcolumn<int>>(right->type_cast("int"));
  auto newval = val.map(datetime_sub_helper(type), right2->val);
  if(contain_nulls || right2->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right2->nulls);
    newval.mapv(reset_null<datetime_t>, newnulls);
    return std::make_shared<typed_dfcolumn<datetime>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<datetime_t>();
    return std::make_shared<typed_dfcolumn<datetime>>(std::move(dvval));
  }
}

struct datetime_sub_im_helper {
  datetime_sub_im_helper(){}
  datetime_sub_im_helper(datetime_type type, int val)
    : type(type), val(val) {}
  std::vector<datetime_t> operator()(const std::vector<datetime_t>& d) {
    auto size = d.size();
    std::vector<datetime_t> ret(size);
    auto dp = d.data();
    auto retp = ret.data();
    if(type == datetime_type::year) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_sub_year(dp[i], val);
    } else if(type == datetime_type::month) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_sub_month(dp[i], val);
    } else if(type == datetime_type::day) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_sub_day(dp[i], val);
    } else if(type == datetime_type::hour) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_sub_hour(dp[i], val);
    } else if(type == datetime_type::minute) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_sub_minute(dp[i], val);
    } else if(type == datetime_type::second) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_sub_second(dp[i], val);
    } else if(type == datetime_type::nanosecond) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_sub_nanosecond(dp[i], val);
    } else throw std::runtime_error("unsupported datetime_type");
    return ret;
  }
  
  datetime_type type;
  int val;
  SERIALIZE(type, val)
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_sub_im(int right,
                                          datetime_type type) {
  auto newval = val.map(datetime_sub_im_helper(type, right));
  if(contain_nulls) {
    newval.mapv(reset_null<datetime_t>, nulls);
    auto newnulls = nulls;
    return std::make_shared<typed_dfcolumn<datetime>>(std::move(newval),
                                                      std::move(nulls));
  } else {
    auto dvval = newval.template moveto_dvector<datetime_t>();
    return std::make_shared<typed_dfcolumn<datetime>>(std::move(dvval));
  }
}


struct datetime_truncate_helper {
  datetime_truncate_helper(){}
  datetime_truncate_helper(datetime_type type) : type(type) {}
  std::vector<datetime_t> operator()(const std::vector<datetime_t>& d) {
    auto size = d.size();
    std::vector<datetime_t> ret(size);
    auto dp = d.data();
    auto retp = ret.data();
    if(type == datetime_type::year) {
      for(size_t i = 0; i < size; i++) retp[i] = datetime_truncate_year(dp[i]);
    } else if(type == datetime_type::month) {
      for(size_t i = 0; i < size; i++) retp[i] = datetime_truncate_month(dp[i]);
    } else if(type == datetime_type::day) {
      for(size_t i = 0; i < size; i++) retp[i] = datetime_truncate_day(dp[i]);
    } else if(type == datetime_type::hour) {
      for(size_t i = 0; i < size; i++) retp[i] = datetime_truncate_hour(dp[i]);
    } else if(type == datetime_type::minute) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_truncate_minute(dp[i]);
    } else if(type == datetime_type::second) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_truncate_second(dp[i]);
    } else if(type == datetime_type::quarter) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_truncate_quarter(dp[i]);
    } else throw std::runtime_error("unsupported datetime_type");
    return ret;
  }
  
  datetime_type type;
  SERIALIZE(type)
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_truncate(datetime_type type) {
  auto ex = val.map(datetime_truncate_helper(type)).
    mapv(+[](std::vector<datetime_t>& v, std::vector<size_t>& nulls) {
        auto vp = v.data();
        auto nullsp = nulls.data();
        auto nulls_size = nulls.size();
        auto max = std::numeric_limits<datetime_t>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < nulls_size; i++) {
          vp[nullsp[i]] = max;
        }
      }, nulls);
  auto newnulls = nulls;
  return std::make_shared<typed_dfcolumn<datetime>>(std::move(ex),
                                                    std::move(newnulls));
}


struct datetime_months_between_helper {
  datetime_months_between_helper(){}
  std::vector<double> operator()(const std::vector<datetime_t>& d1,
                                 const std::vector<datetime_t>& d2) {
    auto size = d1.size();
    std::vector<double> ret(size);
    auto d1p = d1.data();
    auto d2p = d2.data();
    auto retp = ret.data();
    for(size_t i = 0; i < size; i++)
      retp[i] = datetime_months_between(d1p[i], d2p[i]);
    return ret;
  }
  SERIALIZE_NONE
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_months_between
(const std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(right);
  if(!right2)
    throw std::runtime_error("datetime_months_between: type mismatch");
  auto newval = val.map(datetime_months_between_helper(), right2->val);
  if(contain_nulls || right2->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right2->nulls);
    newval.mapv(reset_null<double>, newnulls);
    return std::make_shared<typed_dfcolumn<double>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<double>();
    return std::make_shared<typed_dfcolumn<double>>(std::move(dvval));
  }
}


struct datetime_next_day_helper {
  datetime_next_day_helper(){}
  std::vector<datetime_t> operator()(const std::vector<datetime_t>& d1,
                                     const std::vector<int>& d2) {
    auto size = d1.size();
    std::vector<datetime_t> ret(size);
    auto d1p = d1.data();
    auto d2p = d2.data();
    auto retp = ret.data();
    for(size_t i = 0; i < size; i++)
      retp[i] = datetime_next_day(d1p[i], d2p[i]);
    return ret;
  }
  
  SERIALIZE_NONE
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_next_day
(const std::shared_ptr<dfcolumn>& right) {
  auto right2 =
    std::dynamic_pointer_cast<typed_dfcolumn<int>>(right->type_cast("int"));
  auto newval = val.map(datetime_next_day_helper(), right2->val);
  if(contain_nulls || right2->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right2->nulls);
    newval.mapv(reset_null<datetime_t>, newnulls);
    return std::make_shared<typed_dfcolumn<datetime>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<datetime_t>();
    return std::make_shared<typed_dfcolumn<datetime>>(std::move(dvval));
  }
}


struct datetime_next_day_im_helper {
  datetime_next_day_im_helper(){}
  datetime_next_day_im_helper(int val) : val(val) {}
  std::vector<datetime_t> operator()(const std::vector<datetime_t>& d) {
    auto size = d.size();
    std::vector<datetime_t> ret(size);
    auto dp = d.data();
    auto retp = ret.data();
    for(size_t i = 0; i < size; i++)
      retp[i] = datetime_next_day(dp[i], val);
    return ret;
  }
  
  int val;
  SERIALIZE(val)
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_next_day_im(int right) {
  auto newval = val.map(datetime_next_day_im_helper(right));
  if(contain_nulls) {
    newval.mapv(reset_null<datetime_t>, nulls);
    auto newnulls = nulls;
    return std::make_shared<typed_dfcolumn<datetime>>(std::move(newval),
                                                      std::move(nulls));
  } else {
    auto dvval = newval.template moveto_dvector<datetime_t>();
    return std::make_shared<typed_dfcolumn<datetime>>(std::move(dvval));
  }
}

}
