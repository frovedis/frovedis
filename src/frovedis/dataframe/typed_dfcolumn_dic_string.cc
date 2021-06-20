#include "dfcolumn_impl.hpp"
#if !(defined(_SX) || defined(__ve__))
#include <unordered_set>
#endif

namespace frovedis {

using namespace std;

template class typed_dfcolumn<dic_string>;

std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::group_by
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map) {
  auto ret = std::make_shared<typed_dfcolumn<dic_string>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  group_by_impl(val, nulls, local_idx, split_idx, hash_divide, merge_map,
                ret->val, ret->nulls);
  ret->dic = dic;
  ret->contain_nulls_check();
  return ret;
}

void typed_dfcolumn<dic_string>::multi_group_by_sort
(node_local<std::vector<size_t>>& local_idx) {
  val.mapv(multi_group_by_sort_helper<size_t>, local_idx);
}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::multi_group_by_sort_split
(node_local<std::vector<size_t>>& local_idx) {
  return val.map(multi_group_by_sort_split_helper<size_t>, local_idx);
}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::multi_group_by_split
(node_local<std::vector<size_t>>& local_idx) {
  return val.map(multi_group_by_split_helper<size_t>, local_idx);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::multi_group_by_extract
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 bool check_nulls) {
  auto ret = std::make_shared<typed_dfcolumn<dic_string>>();
  ret->val = val.map(multi_group_by_extract_helper<size_t>, local_idx, split_idx);
  ret->contain_nulls = contain_nulls;
  if(contain_nulls && check_nulls) {
    ret->nulls = ret->val.map(+[](std::vector<size_t>& val) {
        return find_value(val, std::numeric_limits<size_t>::max());        
      });
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  ret->dic = dic;
  return ret;
}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::calc_hash_base() {
  return val.map(calc_hash_base_helper<size_t>);
}

void typed_dfcolumn<dic_string>::calc_hash_base
(node_local<std::vector<size_t>>& hash_base, int shift) {
  val.mapv(calc_hash_base_helper2<size_t>(shift), hash_base);
}

std::shared_ptr<dfcolumn> 
typed_dfcolumn<dic_string>::multi_group_by_exchange
(node_local<std::vector<std::vector<size_t>>>& hash_divide) {
  auto ret = std::make_shared<typed_dfcolumn<dic_string>>();
  ret->val = multi_group_by_exchange_helper(val, hash_divide);
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  ret->contain_nulls = contain_nulls;
  ret->dic = dic;
  return ret;
}

void typed_dfcolumn<dic_string>::append_nulls
(node_local<std::vector<size_t>>& to_append) {
  val.mapv(append_nulls_helper<size_t>, to_append, nulls);
  contain_nulls = true;
}

std::pair<node_local<std::vector<size_t>>, node_local<std::vector<size_t>>>
typed_dfcolumn<dic_string>::hash_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(right);
  if(!right2) 
    throw std::runtime_error("hash_join_eq: column types are different");

  auto left_split_val =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  auto left_split_idx =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  auto right_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto right_split_val =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  auto right_split_idx =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  if(contain_nulls) {
    auto left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    auto left_non_null_val = val.map(extract_non_null<size_t>,
                                     left_full_local_idx, nulls,
                                     left_non_null_idx);
    auto left_global_idx = local_to_global_idx(left_non_null_idx);
    left_non_null_val.mapv(split_by_hash<size_t>, left_split_val,
                           left_global_idx, left_split_idx);
  } else {
    auto left_non_null_val = val.map(extract_helper2<size_t>,
                                     left_full_local_idx);
    auto left_global_idx = local_to_global_idx(left_full_local_idx);
    left_non_null_val.mapv(split_by_hash<size_t>, left_split_val,
                           left_global_idx,left_split_idx);
  }
  auto left_exchanged_val = alltoall_exchange(left_split_val);
  auto left_exchanged_idx = alltoall_exchange(left_split_idx);

  auto right_val = equal_prepare(right2);
  auto& right_nulls = right2->nulls;
  if(right2->contain_nulls) {
    auto right_non_null_val =
      right_val.map(extract_non_null<size_t>, right_full_local_idx,
                    right_nulls, right_non_null_idx);
    auto right_global_idx = local_to_global_idx(right_non_null_idx);
    right_non_null_val.mapv(split_by_hash<size_t>, right_split_val,
                            right_global_idx, right_split_idx);
  } else {
    auto right_non_null_val =
      right_val.map(extract_helper2<size_t>, right_full_local_idx);
    auto right_global_idx = local_to_global_idx(right_full_local_idx);
    right_non_null_val.mapv(split_by_hash<size_t>, right_split_val,
                            right_global_idx, right_split_idx);
  }
  auto right_exchanged_val = alltoall_exchange(right_split_val);
  auto right_exchanged_idx = alltoall_exchange(right_split_idx);
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto right_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  left_exchanged_val.mapv(hash_join_eq_helper<size_t>, left_exchanged_idx,
                          right_exchanged_val, right_exchanged_idx,
                          left_idx_ret, right_idx_ret);
  return std::make_pair(std::move(left_idx_ret), std::move(right_idx_ret));
}

std::tuple<node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>>
typed_dfcolumn<dic_string>::outer_hash_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(right);
  if(!right2)
    throw std::runtime_error("outer_hash_join_eq: column types are different");
  auto left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto left_split_val =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  auto left_split_idx =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  auto right_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto right_split_val =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  auto right_split_idx =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  if(contain_nulls) {
    auto left_non_null_val = val.map(extract_non_null<size_t>,
                                     left_full_local_idx, nulls,
                                     left_non_null_idx);
    auto left_global_idx = local_to_global_idx(left_non_null_idx);
    left_non_null_val.mapv(split_by_hash<size_t>, left_split_val,
                           left_global_idx, left_split_idx);
  } else {
    auto left_non_null_val = val.map(extract_helper2<size_t>,
                                     left_full_local_idx);
    auto left_global_idx = local_to_global_idx(left_full_local_idx);
    left_non_null_val.mapv(split_by_hash<size_t>, left_split_val,
                           left_global_idx, left_split_idx);
  }
  auto left_exchanged_val = alltoall_exchange(left_split_val);
  auto left_exchanged_idx = alltoall_exchange(left_split_idx);
  auto right_val = equal_prepare(right2);
  auto& right_nulls = right2->nulls;
  if(right2->contain_nulls) {
    auto right_non_null_val =
      right_val.map(extract_non_null<size_t>, right_full_local_idx,
                    right_nulls, right_non_null_idx);
    auto right_global_idx = local_to_global_idx(right_non_null_idx);
    right_non_null_val.mapv(split_by_hash<size_t>, right_split_val,
                            right_global_idx, right_split_idx);
  } else {
    auto right_non_null_val =
      right_val.map(extract_helper2<size_t>, right_full_local_idx);
    auto right_global_idx = local_to_global_idx(right_full_local_idx);
    right_non_null_val.mapv(split_by_hash<size_t>, right_split_val,
                            right_global_idx, right_split_idx);
  }
  auto right_exchanged_val = alltoall_exchange(right_split_val);
  auto right_exchanged_idx = alltoall_exchange(right_split_idx);
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto right_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto null_idx_ret = left_exchanged_val.map(outer_hash_join_eq_helper<size_t>,
                                             left_exchanged_idx,
                                             right_exchanged_val,
                                             right_exchanged_idx,
                                             left_idx_ret,
                                             right_idx_ret);
  return std::make_tuple(std::move(left_idx_ret), std::move(right_idx_ret),
                         std::move(null_idx_ret));
}

std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<dic_string>::bcast_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(right);
  if(!right2)
    throw std::runtime_error("bcast_join_eq: column types are different");
  node_local<std::vector<size_t>> left_non_null_idx;
  node_local<std::vector<size_t>> left_non_null_val;
  if(contain_nulls) {
    left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    left_non_null_val = val.map(extract_non_null<size_t>, left_full_local_idx,
                                nulls, left_non_null_idx);
  } else {
    left_non_null_val = val.map(extract_helper2<size_t>, left_full_local_idx);
    left_non_null_idx = std::move(left_full_local_idx);
  }
  auto right_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto right_val = equal_prepare(right2);
  auto& right_nulls = right2->nulls;
  node_local<std::vector<size_t>> right_non_null_val;
  node_local<std::vector<size_t>> right_global_idx;
  if(right2->contain_nulls) {
    right_non_null_val =
      right_val.map(extract_non_null<size_t>, right_full_local_idx,
                    right_nulls, right_non_null_idx);
    right_global_idx = local_to_global_idx(right_non_null_idx);
  } else {
    right_non_null_val = right_val.map(extract_helper2<size_t>,
                                       right_full_local_idx);
    right_global_idx = local_to_global_idx(right_full_local_idx);
  }
  auto right_non_null_val_bcast =
    broadcast(right_non_null_val.template viewas_dvector<size_t>().gather());
  auto right_global_idx_bcast =
    broadcast(right_global_idx.template viewas_dvector<size_t>().gather());
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto right_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  left_non_null_val.mapv(equi_join<size_t>, left_non_null_idx,
                         right_non_null_val_bcast, right_global_idx_bcast,
                         left_idx_ret, right_idx_ret);
  return std::make_pair(std::move(left_idx_ret), std::move(right_idx_ret));
}

std::tuple<node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>>
typed_dfcolumn<dic_string>::outer_bcast_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(right);
  if(!right2)
    throw std::runtime_error("bcast_join_eq: column types are different");
  node_local<std::vector<size_t>> left_non_null_idx;
  node_local<std::vector<size_t>> left_non_null_val;
  if(contain_nulls) {
    left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    left_non_null_val = val.map(extract_non_null<size_t>, left_full_local_idx,
                                nulls, left_non_null_idx);
  } else {
    left_non_null_val = val.map(extract_helper2<size_t>, left_full_local_idx);
    left_non_null_idx = std::move(left_full_local_idx);
  }
  auto right_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto right_val = equal_prepare(right2);
  auto& right_nulls = right2->nulls;
  node_local<std::vector<size_t>> right_non_null_val;
  node_local<std::vector<size_t>> right_global_idx;
  if(right2->contain_nulls) {
    right_non_null_val =
      right_val.map(extract_non_null<size_t>, right_full_local_idx,
                    right_nulls, right_non_null_idx);
    right_global_idx = local_to_global_idx(right_non_null_idx);
  } else {
    right_non_null_val = right_val.map(extract_helper2<size_t>,
                                       right_full_local_idx);
    right_global_idx = local_to_global_idx(right_full_local_idx);
  }
  auto right_non_null_val_bcast =
    broadcast(right_non_null_val.template viewas_dvector<size_t>().gather());
  auto right_global_idx_bcast =
    broadcast(right_global_idx.template viewas_dvector<size_t>().gather());
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto right_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto null_idx_ret = left_non_null_val.map(outer_equi_join<size_t>,
                                            left_non_null_idx,
                                            right_non_null_val_bcast,
                                            right_global_idx_bcast,
                                            left_idx_ret, right_idx_ret);
  return std::make_tuple(std::move(left_idx_ret), std::move(right_idx_ret),
                         std::move(null_idx_ret));
}

std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<dic_string>::star_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(right);
  if(!right2)
    throw std::runtime_error("star_join_eq: column types are different");
  node_local<std::vector<size_t>> left_non_null_idx;
  node_local<std::vector<size_t>> left_non_null_val;
  if(contain_nulls) {
    left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    left_non_null_val = val.map(extract_non_null<size_t>, left_full_local_idx,
                                nulls, left_non_null_idx);
  } else {
    left_non_null_val = val.map(extract_helper2<size_t>, left_full_local_idx);
    left_non_null_idx = left_full_local_idx;
  }
  auto right_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto right_val = equal_prepare(right2);
  auto& right_nulls = right2->nulls;
  node_local<std::vector<size_t>> right_non_null_val;
  node_local<std::vector<size_t>> right_global_idx;
  if(right2->contain_nulls) {
    right_non_null_val =
      right_val.map(extract_non_null<size_t>, right_full_local_idx,
                    right_nulls, right_non_null_idx);
    right_global_idx = local_to_global_idx(right_non_null_idx);
  } else {
    right_non_null_val = right_val.map(extract_helper2<size_t>,
                                       right_full_local_idx);
    right_global_idx = local_to_global_idx(right_full_local_idx);
  }
  auto right_non_null_val_bcast = 
    broadcast(right_non_null_val.template viewas_dvector<size_t>().gather());
  auto right_global_idx_bcast =
    broadcast(right_global_idx.template viewas_dvector<size_t>().gather());
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto right_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto missed = 
    left_non_null_val.map(unique_equi_join2<size_t>, left_non_null_idx,
                          right_non_null_val_bcast, right_global_idx_bcast,
                          right_idx_ret);
  return std::make_pair(std::move(right_idx_ret), std::move(missed));
}

vector<size_t> filter_like_helper(const dict& dic,
                                  const std::string& pattern,
                                  int wild_card) {
  auto num_words = dic.num_words();
  std::vector<size_t> order(num_words);
  auto orderp = order.data();
  for(size_t i = 0; i < num_words; i++) orderp[i] = i;
  auto ws = decompress_compressed_words(dic.cwords, dic.lens, dic.lens_num,
                                        order);
  return like(ws, pattern, wild_card);
}

vector<size_t> filter_not_like_helper(const dict& dic,
                                      const std::string& pattern,
                                      int wild_card) {
  auto num_words = dic.num_words();
  std::vector<size_t> order(num_words);
  auto orderp = order.data();
  for(size_t i = 0; i < num_words; i++) orderp[i] = i;
  auto ws = decompress_compressed_words(dic.cwords, dic.lens, dic.lens_num,
                                        order);
  auto hit = like(ws, pattern, wild_card);
  return set_difference(order, hit);
}

#if !(defined(_SX) || defined(__ve__))
vector<size_t> filter_like_join(std::vector<size_t>& left,
                                std::vector<size_t>& left_idx, 
                                std::vector<size_t>& right) {
  std::unordered_set<size_t> right_set;
  for(size_t i = 0; i < right.size(); i++) {
    right_set.insert(right[i]);
  }
  vector<size_t> ret;
  for(size_t i = 0; i < left.size(); i++) {
    auto it = right_set.find(left[i]);
    if(it != right_set.end()) {
      ret.push_back(left_idx[i]);
    }
  }
  return ret;
}
#else
std::vector<size_t> filter_like_join(std::vector<size_t>& left,
                                     std::vector<size_t>& left_idx,
                                     std::vector<size_t>& right) {
  vector<size_t> dummy(right.size());
  unique_hashtable<size_t, size_t> ht(right, dummy);
  std::vector<size_t> missed;
  ht.lookup(left, missed); // ret val not used
  return shrink_missed(left_idx, missed);
}
#endif

// implement as join, since matched strings might be many
node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::
filter_like(const std::string& pattern, int wild_card) {
  auto right_non_null_val = dic->map(filter_like_helper, 
                                     broadcast(pattern), 
                                     broadcast(wild_card));
  auto left_full_local_idx = get_local_index();
  node_local<std::vector<size_t>> left_non_null_idx;
  node_local<std::vector<size_t>> left_non_null_val;
  if(contain_nulls) {
    left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    left_non_null_val = val.map(extract_non_null<size_t>, left_full_local_idx,
                                nulls, left_non_null_idx);
  } else {
    left_non_null_val = val.map(extract_helper2<size_t>, left_full_local_idx);
    left_non_null_idx = std::move(left_full_local_idx);
  }
  auto right_non_null_val_bcast =
    broadcast(right_non_null_val.template viewas_dvector<size_t>().gather());
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  return left_non_null_val.map(filter_like_join, left_non_null_idx,
                               right_non_null_val_bcast);
}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::
filter_not_like(const std::string& pattern, int wild_card) {
  auto right_non_null_val = dic->map(filter_not_like_helper, 
                                     broadcast(pattern), 
                                     broadcast(wild_card));
  auto left_full_local_idx = get_local_index();
  node_local<std::vector<size_t>> left_non_null_idx;
  node_local<std::vector<size_t>> left_non_null_val;
  if(contain_nulls) {
    left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    left_non_null_val = val.map(extract_non_null<size_t>, left_full_local_idx,
                                nulls, left_non_null_idx);
  } else {
    left_non_null_val = val.map(extract_helper2<size_t>, left_full_local_idx);
    left_non_null_idx = std::move(left_full_local_idx);
  }
  auto right_non_null_val_bcast =
    broadcast(right_non_null_val.template viewas_dvector<size_t>().gather());
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  return left_non_null_val.map(filter_like_join, left_non_null_idx,
                               right_non_null_val_bcast);
}

std::vector<size_t>
dic_string_sort_prepare_helper(const std::vector<size_t>& val,
                               const std::vector<size_t>& order,
                               const std::vector<size_t>& nulls) {
  auto orderp = order.data();
  auto valp = val.data();
  auto val_size = val.size();
  std::vector<size_t> new_val(val_size);
  auto new_valp = new_val.data();
  auto null_value = std::numeric_limits<size_t>::max();
#pragma _NEC ivdep
  for(size_t i = 0; i < val_size; i++) {
    if(valp[i] != null_value) new_valp[i] = orderp[valp[i]];
  }
  auto nullsp = nulls.data();
  auto nulls_size = nulls.size();
#pragma _NEC ivdep
  for(size_t i = 0; i < nulls_size; i++) {
    new_valp[nullsp[i]] = null_value;
  }
  return new_val;
}

typed_dfcolumn<size_t>
typed_dfcolumn<dic_string>::sort_prepare() {
  typed_dfcolumn<size_t> rescol;

  auto local_dic = dic->get(0);
  auto num_words = local_dic.num_words();
  std::vector<size_t> order(num_words);
  auto orderp = order.data();
  for(size_t i = 0; i < num_words; i++) orderp[i] = i;
  lexical_sort_compressed_words(local_dic.cwords, local_dic.lens,
                                local_dic.lens_num, order);
  rescol.val = val.map(dic_string_sort_prepare_helper, broadcast(order),
                       nulls);
  rescol.nulls = nulls; // copy
  return rescol;
}

shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::sort(node_local<vector<size_t>>& idx) {
  auto tmpcol = sort_prepare();
  return tmpcol.sort(idx);
}

shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::sort_desc(node_local<vector<size_t>>& idx) {
  auto tmpcol = sort_prepare();
  return tmpcol.sort_desc(idx);
}

shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::sort_with_idx(node_local<vector<size_t>>& idx,
                                          node_local<vector<size_t>>& res_idx) {
  auto tmpcol = sort_prepare();
  return tmpcol.sort_with_idx(idx, res_idx);
}

shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::
sort_with_idx_desc(node_local<vector<size_t>>& idx,
                   node_local<vector<size_t>>& res_idx) {
  auto tmpcol = sort_prepare();
  return tmpcol.sort_with_idx_desc(idx, res_idx);
}


node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::filter_is_null() {return nulls;}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::filter_is_not_null() {
  auto local_idx = get_local_index();
  if(contain_nulls)
    return local_idx.map(set_difference<size_t>, nulls);
  else return local_idx;
}

std::vector<size_t>
dic_string_equal_prepare_helper(const std::vector<size_t>& rval,
                                const std::vector<size_t>& trans) {
  auto rval_size = rval.size();
  std::vector<size_t> ret(rval_size);
  auto retp = ret.data();
  auto rvalp = rval.data();
  auto transp = trans.data();
  auto NOT_FOUND_TRANS = numeric_limits<size_t>::max();
  auto NOT_FOUND = NOT_FOUND_TRANS - 1;
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < rval_size; i++) {
    auto v = transp[rvalp[i]];
    if(v == NOT_FOUND_TRANS) retp[i] = NOT_FOUND; // to distinguish from NULL
    else retp[i] = v;
  }
  return ret;
}

node_local<vector<size_t>>
typed_dfcolumn<dic_string>::
equal_prepare(shared_ptr<typed_dfcolumn<dic_string>>& right) {
  auto left_local_dic = dic->get(0);
  auto right_local_dic = right->dic->get(0);
  compressed_words to_lookup;
  auto right_num_words = right_local_dic.num_words();
  to_lookup.cwords.swap(right_local_dic.cwords);
  to_lookup.lens.swap(right_local_dic.lens);
  to_lookup.lens_num.swap(right_local_dic.lens_num);
  to_lookup.order.resize(right_num_words);
  auto orderp = to_lookup.order.data();
  for(size_t i = 0; i < right_num_words; i++) orderp[i] = i;
  auto trans_table = broadcast(left_local_dic.lookup(to_lookup));
  return right->val.map(dic_string_equal_prepare_helper, trans_table);
}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::filter_eq(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(right);
  if(!right2) throw std::runtime_error("filter_eq: column types are different");
  auto rightval = equal_prepare(right2);
  auto filtered_idx = val.map(filter_eq_helper<size_t, size_t>, rightval);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

// TODO: check right null; should not be included
node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::filter_neq(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(right);
  if(!right2) throw std::runtime_error("filter_eq: column types are different");
  auto rightval = equal_prepare(right2);
  auto filtered_idx = val.map(filter_neq_helper<size_t, size_t>, rightval);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::filter_eq_immed(std::shared_ptr<dfscalar>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfscalar<std::string>>(right);
  if(!right2) throw std::runtime_error("filter string column with non string");
  auto rightval = right2->val;
  return val.map
    (+[](std::vector<size_t>& val, const dict& dic, 
         const std::string& right, const std::vector<size_t>& nulls,
         bool contain_nulls) {
      words right_word;
      right_word.chars = char_to_int(right);
      right_word.starts = {0};
      right_word.lens = {right.size()};
      auto right_cword = make_compressed_words(right_word);
      auto lookedup = dic.lookup(right_cword);
      if(lookedup[0] != numeric_limits<size_t>::max()) {
        auto filtered_idx =
          filter_eq_immed_helper<size_t,size_t>(val, lookedup[0]);
        if(contain_nulls) return set_difference(filtered_idx, nulls);
        else return filtered_idx;
      }
      else {
        return std::vector<size_t>();
      }
    }, *dic, broadcast(rightval), nulls, broadcast(contain_nulls));
}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::filter_neq_immed(std::shared_ptr<dfscalar>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfscalar<std::string>>(right);
  if(!right2) throw std::runtime_error("filter string column with non string");
  auto rightval = right2->val;
  return val.map
    (+[](std::vector<size_t>& val, const dict& dic, 
         const std::string& right, const std::vector<size_t>& nulls,
         bool contain_nulls) {
      words right_word;
      right_word.chars = char_to_int(right);
      right_word.starts = {0};
      right_word.lens = {right.size()};
      auto right_cword = make_compressed_words(right_word);
      auto lookedup = dic.lookup(right_cword);
      if(lookedup[0] != numeric_limits<size_t>::max()) {
        auto filtered_idx =
          filter_neq_immed_helper<size_t,size_t>(val, lookedup[0]);
        if(contain_nulls) return set_difference(filtered_idx, nulls);
        else return filtered_idx;
      }
      else {
        return std::vector<size_t>();
      }
    }, *dic, broadcast(rightval), nulls, broadcast(contain_nulls));
}

node_local<std::vector<size_t>> typed_dfcolumn<dic_string>::get_local_index() {
  return val.map(get_local_index_helper<size_t>);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::extract(node_local<std::vector<size_t>>& idx) {
  auto ret = std::make_shared<typed_dfcolumn<dic_string>>();
  auto retnulls = make_node_local_allocate<std::vector<size_t>>();
  if(contain_nulls) {
    ret->val = val.map(extract_helper<size_t>, idx, nulls, retnulls);
    ret->nulls = std::move(retnulls);
    ret->contain_nulls_check();
  } else {
    ret->val = val.map(extract_helper2<size_t>, idx);
    ret->nulls = std::move(retnulls);
  }
  ret->dic = dic;
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::global_extract
(node_local<std::vector<size_t>>& global_idx,
 node_local<std::vector<size_t>>& to_store_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
  auto ret = std::make_shared<typed_dfcolumn<dic_string>>();
  auto exdata = val.map(global_extract_helper<size_t>, exchanged_idx);
  auto exchanged_back = alltoall_exchange(exdata).map(flatten<size_t>);
  ret->val = exchanged_back.map
    (+[](std::vector<size_t>& val, std::vector<size_t>& idx) {
      auto valp = val.data();
      auto idxp = idx.data();
      auto size = idx.size();
      std::vector<size_t> ret(size);
      auto retp = ret.data();
#pragma cdir nodep
#pragma _NEC ivdep
      for(size_t i = 0 ; i < size; i++) {
        retp[i] = valp[idxp[i]];
      }
      return ret;
    }, to_store_idx);
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
  ret->dic = dic;
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::count
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  return count_impl(val, nulls, local_grouped_idx, local_idx_split,
                    hash_divide, merge_map, row_sizes);
}

size_t typed_dfcolumn<dic_string>::count() {
  size_t size = val.viewas_dvector<size_t>().size();
  if(contain_nulls) {
    size_t nullsize = nulls.template viewas_dvector<size_t>().size();
    return size - nullsize;
  } else return size;
}

// TODO: shrink unused dic
void typed_dfcolumn<dic_string>::save(const std::string& file) {
  vector<size_t> new_starts; // not used
  auto to_save =
    int_to_vchar(concat_words(dic->get(0).decompress(), "\n", new_starts));
  savebinary_local(to_save, file + "_dic");
  val.viewas_dvector<size_t>().savebinary(file + "_idx");
  auto dv_nulls = nulls.template viewas_dvector<size_t>();
  auto sizes = val.template viewas_dvector<size_t>().sizes();
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

std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::head(size_t limit) {
  auto ret = std::make_shared<typed_dfcolumn<dic_string>>();
  ret->val = val.viewas_dvector<size_t>().head(limit).moveto_node_local();
  if(contain_nulls) {
    ret->nulls = limit_nulls_head(nulls, sizes(), limit);
    ret->contain_nulls = true;
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  ret->dic = dic;
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::tail(size_t limit) {
  auto ret = std::make_shared<typed_dfcolumn<dic_string>>();
  ret->val = val.viewas_dvector<size_t>().tail(limit).moveto_node_local();
  auto new_sizes = ret->val.template viewas_dvector<size_t>().sizes();
  if(contain_nulls) {
    ret->nulls = limit_nulls_tail(nulls, sizes(), new_sizes, limit);
    ret->contain_nulls = true;
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  ret->dic = dic;
  return ret;
}

void dfcolumn_replace_nullstr(words& ws,
                              const std::vector<size_t>& nulls,
                              const std::string& nullstr) {
  auto nulls_size = nulls.size();
  auto nullsp = nulls.data();
  auto nullstrvec = char_to_int(nullstr);
  auto nullstr_size = nullstr.size();
  auto chars_size = ws.chars.size();
  auto newchars_size = chars_size + nullstr_size;
  auto charsp = ws.chars.data();
  std::vector<int> newchars(newchars_size);
  auto newcharsp = newchars.data();
  for(size_t i = 0; i < chars_size; i++) newcharsp[i] = charsp[i];
  auto nullstrvecp = nullstrvec.data();
  for(size_t i = 0; i < nullstr_size; i++) {
    newcharsp[chars_size + i] = nullstrvecp[i];
  }
  ws.chars.swap(newchars);
  size_t nullstart = chars_size;
  size_t nulllens = nullstr_size;
  auto startsp = ws.starts.data();
  auto lensp = ws.lens.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob  
  for(size_t i = 0; i < nulls_size; i++) {
    startsp[nullsp[i]] = nullstart;
    lensp[nullsp[i]] = nulllens;
  }
}

node_local<words>
typed_dfcolumn<dic_string>::as_words(size_t precision, // not used
                                     const std::string& datetime_fmt, // not used
                                     bool quote_escape,
                                     const std::string& nullstr) {
  auto nl_words = dic->map
    (+[](const dict& d, const std::vector<size_t>& val, const std::vector<size_t>& nulls) {
      auto nulls_size = nulls.size();
      if(nulls_size == 0) return d.index_to_words(val);
      else {
        auto val_size = val.size();
        std::vector<size_t> newval(val_size);
        auto newvalp = newval.data();
        auto valp = val.data();
        for(size_t i = 0; i < val_size; i++) newvalp[i] = valp[i];
        auto nullsp = nulls.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < nulls_size; i++) newvalp[nullsp[i]] = 0;
        if(d.num_words() == 0) { // does this happen?
          words w;
          w.chars = char_to_int("NULL");
          w.starts = {0};
          w.lens = {w.chars.size()};
          auto tmpd = make_dict(w);
          return tmpd.index_to_words(newval);
        } else {
          return d.index_to_words(newval);  
        }
      }
    }, val, nulls);
  if(contain_nulls)
    nl_words.mapv(dfcolumn_replace_nullstr, nulls, broadcast(nullstr));
  if(quote_escape) nl_words.mapv(quote_and_escape);
  return nl_words;
}

void typed_dfcolumn<dic_string>::debug_print() {
  std::cout << "dtype: " << dtype() << std::endl;
  std::cout << "dic: " << std::endl;
  dic->get(0).print();
  std::cout << "val: " << std::endl;
  for(auto& i: val.gather()) {
    for(auto j: i) {
      std::cout << j << " ";
    }
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

void typed_dfcolumn<dic_string>::contain_nulls_check() {
  if(nulls.template viewas_dvector<size_t>().size() == 0)
    contain_nulls = false;
  else contain_nulls = true;
}

void typed_dfcolumn<dic_string>::init(node_local<words>& ws,
                                      bool allocate_nulls) {
  auto cws = ws.map(make_compressed_words);
  init_compressed(cws, allocate_nulls);
}

void typed_dfcolumn<dic_string>::init_compressed
(node_local<compressed_words>& cws, bool allocate_nulls) {
  time_spent t(DEBUG);
  if(allocate_nulls) nulls = make_node_local_allocate<vector<size_t>>();
  auto nl_dict = cws.map(make_dict_from_compressed);
  t.show("init_compressed, create dict locally: ");
  auto local_dict = nl_dict.reduce(merge_dict);
  t.show("init_compressed, merge_dict: ");
  dic = make_shared<node_local<dict>>(broadcast(local_dict));
  t.show("init_compressed, broadcast dict: ");
  val = dic->map(+[](const dict& d, const compressed_words& c)
                 {return d.lookup(c);}, cws);
  t.show("init_compressed, lookup: ");
}

size_t typed_dfcolumn<dic_string>::size() {
  return val.viewas_dvector<size_t>().size();
}

std::vector<size_t> typed_dfcolumn<dic_string>::sizes() {
  return val.template viewas_dvector<size_t>().sizes();
}

dict union_columns_dic_string_create_dic(std::vector<dict*>& dic_colsp) {
  auto dic_colsp_size = dic_colsp.size();
  if(dic_colsp_size == 0) return dict();
  else if(dic_colsp_size == 1) return *dic_colsp[0];
  else if(dic_colsp_size == 2) {
    return merge_dict(*dic_colsp[0], *dic_colsp[1]);
  } else {
    auto left_size = ceil_div(dic_colsp_size, size_t(2));
    auto right_size = dic_colsp_size - left_size;
    vector<dict*> left(left_size);
    vector<dict*> right(right_size);
    for(size_t i = 0; i < left_size; i++) {
      left[i] = dic_colsp[i];
    }
    for(size_t i = 0; i < right_size; i++) {
      right[i] = dic_colsp[left_size + i];
    }
    auto left_merged = union_columns_dic_string_create_dic(left);
    auto right_merged = union_columns_dic_string_create_dic(right);
    return merge_dict(left_merged, right_merged);
  }
}

// same as string, but copied to avoid confusion
void union_columns_dic_string_resize_newval
(std::vector<size_t>& newval, std::vector<std::vector<size_t>*>& val_colsp) {
  auto cols_size = val_colsp.size();
  std::vector<size_t> val_sizes(cols_size);
  auto val_colspp = val_colsp.data();
  auto val_sizesp = val_sizes.data();
  for(size_t i = 0; i < cols_size; i++) {
    val_sizesp[i] = val_colspp[i]->size();
  }
  size_t total_val_size = 0;
  for(size_t i = 0; i < cols_size; i++) {
    total_val_size += val_sizesp[i];
  }
  newval.resize(total_val_size);
}

node_local<vector<size_t>>
union_columns_dic_string_prepare(dict& local_newdic,
                                 std::shared_ptr<node_local<dict>>& dic,
                                 node_local<std::vector<size_t>>& val) {
  auto local_dic = dic->get(0);
  compressed_words to_lookup;
  auto num_words = local_dic.num_words();
  to_lookup.cwords.swap(local_dic.cwords);
  to_lookup.lens.swap(local_dic.lens);
  to_lookup.lens_num.swap(local_dic.lens_num);
  to_lookup.order.resize(num_words);
  auto orderp = to_lookup.order.data();
  for(size_t i = 0; i < num_words; i++) orderp[i] = i;
  auto trans_table = broadcast(local_newdic.lookup(to_lookup));
  return val.map(dic_string_equal_prepare_helper, trans_table);
}

// same as string, but copied to avoid confusion
void union_columns_dic_string_update_nulls
(std::vector<size_t>& newnulls,
 std::vector<std::vector<size_t>*>& val_colsp,
 std::vector<std::vector<size_t>*>& nulls_colsp) {

  auto cols_size = val_colsp.size();
  std::vector<size_t> val_sizes(cols_size);
  auto val_colspp = val_colsp.data();
  auto val_sizesp = val_sizes.data();
  for(size_t i = 0; i < cols_size; i++) {
    val_sizesp[i] = val_colspp[i]->size();
  }
  std::vector<size_t> nulls_sizes(cols_size);
  auto nulls_colspp = nulls_colsp.data();
  auto nulls_sizesp = nulls_sizes.data();
  for(size_t i = 0; i < cols_size; i++) {
    nulls_sizesp[i] = nulls_colspp[i]->size();
  }
  size_t total_nulls_size = 0;
  for(size_t i = 0; i < cols_size; i++) {
    total_nulls_size += nulls_sizesp[i];
  }
  newnulls.resize(total_nulls_size);
  auto crnt_newnullsp = newnulls.data();
  auto crnt_shift = 0;
  for(size_t i = 0; i < cols_size; i++) {
    auto nulls_size = nulls_sizesp[i];
    auto nullsp = nulls_colspp[i]->data();
    for(size_t j = 0; j < nulls_size; j++) {
      crnt_newnullsp[j] = nullsp[j] + crnt_shift;
    }
    crnt_shift += val_sizesp[i];
    crnt_newnullsp += nulls_size;
  }
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::union_columns
(const std::vector<std::shared_ptr<dfcolumn>>& cols) {
  auto cols_size = cols.size();
  if(cols_size == 0) {
    return std::make_shared<typed_dfcolumn<dic_string>>(dic, val, nulls);
  }
  std::vector<std::shared_ptr<typed_dfcolumn<dic_string>>> rights(cols_size);
  for(size_t i = 0; i < cols_size; i++) {
    rights[i] = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(cols[i]);
    if(!rights[i]) throw std::runtime_error("union_columns: different type");
  }
  auto val_colsp =
    make_node_local_allocate<std::vector<std::vector<size_t>*>>();
  auto nulls_colsp =
    make_node_local_allocate<std::vector<std::vector<size_t>*>>();
  auto dic_colsp =
    make_node_local_allocate<std::vector<dict*>>();
  val.mapv(+[](std::vector<size_t>& val,
               std::vector<std::vector<size_t>*>& val_colsp)
           {val_colsp.push_back(&val);}, val_colsp);
  nulls.mapv(+[](std::vector<size_t>& nulls,
                 std::vector<std::vector<size_t>*>& nulls_colsp)
             {nulls_colsp.push_back(&nulls);}, nulls_colsp);
  dic->mapv(+[](dict& dic, std::vector<dict*>& dic_colsp)
            {dic_colsp.push_back(&dic);}, dic_colsp);
  for(size_t i = 0; i < cols_size; i++) {
    rights[i]->val.mapv(+[](std::vector<size_t>& val,
                            std::vector<std::vector<size_t>*>& val_colsp)
                        {val_colsp.push_back(&val);}, val_colsp);
    rights[i]->nulls.mapv(+[](std::vector<size_t>& nulls,
                              std::vector<std::vector<size_t>*>& nulls_colsp)
                          {nulls_colsp.push_back(&nulls);}, nulls_colsp);
    rights[i]->dic->mapv(+[](dict& dic, std::vector<dict*>& dic_colsp)
                         {dic_colsp.push_back(&dic);}, dic_colsp);
  }
  auto newval = make_node_local_allocate<std::vector<size_t>>();
  auto newnulls = make_node_local_allocate<std::vector<size_t>>();
  auto newdic = std::make_shared<node_local<dict>>
    (dic_colsp.map(union_columns_dic_string_create_dic));
              
  newval.mapv(union_columns_dic_string_resize_newval, val_colsp);
  auto crnt_pos = broadcast(size_t(0));
  auto local_newdic = newdic->get(0);
  {
    auto crnt_newval = union_columns_dic_string_prepare(local_newdic, dic, val);
    newval.mapv
      (+[](std::vector<size_t>& newval, std::vector<size_t>& val, size_t& pos) {
        auto crnt_newvalp = newval.data() + pos;
        auto valp = val.data();
        auto val_size = val.size();
        for(size_t i = 0; i < val_size; i++) {crnt_newvalp[i] = valp[i];}
        pos += val_size;
      }, crnt_newval, crnt_pos);
  }
  for(size_t i = 0; i < cols_size; i++) {
    auto crnt_newval = union_columns_dic_string_prepare
      (local_newdic, rights[i]->dic, rights[i]->val);
    newval.mapv
      (+[](std::vector<size_t>& newval, std::vector<size_t>& val, size_t& pos) {
        auto crnt_newvalp = newval.data() + pos;
        auto valp = val.data();
        auto val_size = val.size();
        for(size_t i = 0; i < val_size; i++) {crnt_newvalp[i] = valp[i];}
        pos += val_size;
      }, crnt_newval, crnt_pos);
  }
  newnulls.mapv(union_columns_dic_string_update_nulls, val_colsp, nulls_colsp);
  return std::make_shared<typed_dfcolumn<dic_string>>
    (std::move(newdic), std::move(newval), std::move(newnulls));
}

// same as equal_prepare; argument is not shared_ptr
node_local<vector<size_t>>
typed_dfcolumn<dic_string>::
equal_prepare_multi_join(typed_dfcolumn<dic_string>& right) {
  auto left_local_dic = dic->get(0);
  auto right_local_dic = right.dic->get(0);
  compressed_words to_lookup;
  auto right_num_words = right_local_dic.num_words();
  to_lookup.cwords.swap(right_local_dic.cwords);
  to_lookup.lens.swap(right_local_dic.lens);
  to_lookup.lens_num.swap(right_local_dic.lens_num);
  to_lookup.order.resize(right_num_words);
  auto orderp = to_lookup.order.data();
  for(size_t i = 0; i < right_num_words; i++) orderp[i] = i;
  auto trans_table = broadcast(left_local_dic.lookup(to_lookup));
  return right.val.map(dic_string_equal_prepare_helper, trans_table);
}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::calc_hash_base_multi_join
(std::shared_ptr<dfcolumn>& left) {
  auto left2 = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(left);
  if(!left2)
    throw std::runtime_error("multi_join: column types are different");
  auto thisval = left2->equal_prepare_multi_join(*this);
  return thisval.map(calc_hash_base_helper<size_t>);
}

void typed_dfcolumn<dic_string>::calc_hash_base_multi_join
(node_local<std::vector<size_t>>& hash_base, int shift,
 std::shared_ptr<dfcolumn>& left) {
  auto left2 = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(left);
  if(!left2)
    throw std::runtime_error("multi_join: column types are different");
  auto thisval = left2->equal_prepare_multi_join(*this);
  thisval.mapv(calc_hash_base_helper2<size_t>(shift), hash_base);
}

bool typed_dfcolumn<dic_string>::is_unique() {
  auto& typed_col = dynamic_cast<typed_dfcolumn<dic_string>&>(*this);
  auto key = typed_col.val.viewas_dvector<size_t>().gather();
  auto nulls_count = typed_col.get_nulls().viewas_dvector<size_t>().size();
  return (nulls_count <= 1) && vector_is_unique(key);
}

// for spill-restore

// TODO: spill dic; currently it is shared_ptr,
// so it is difficult to deallocate them
void typed_dfcolumn<dic_string>::spill_to_disk() {
  if(already_spilled_to_disk) {
    val.mapv(+[](std::vector<size_t>& v){
        std::vector<size_t> tmp;
        tmp.swap(v);
      });
    nulls.mapv(+[](std::vector<size_t>& n){
        std::vector<size_t> tmp;
        tmp.swap(n);
      });
  } else {
    val.mapv(+[](std::vector<size_t>& v, std::string& spill_path){
        savebinary_local(v, spill_path+"/val");
        std::vector<size_t> tmp;
        tmp.swap(v);
      }, spill_path);
    nulls.mapv(+[](std::vector<size_t>& n, std::string& spill_path){
        savebinary_local(n, spill_path+"/nulls");
        std::vector<size_t> tmp;
        tmp.swap(n);
      }, spill_path);
    already_spilled_to_disk = true;
  }
  cleared = true;
}

void typed_dfcolumn<dic_string>::restore_from_disk() {
  val.mapv(+[](std::vector<size_t>& v, std::string& spill_path){
      v = loadbinary_local<size_t>(spill_path+"/val");
    }, spill_path);
  nulls.mapv(+[](std::vector<size_t>& n, std::string& spill_path){
      n = loadbinary_local<size_t>(spill_path+"/nulls");
    }, spill_path);
  cleared = false;
}

size_t typed_dfcolumn<dic_string>::calc_spill_size() {
  auto valsize = size();
  auto nullsize = nulls.map(+[](std::vector<size_t>& n){return n.size();}).
    reduce(+[](const size_t l, const size_t r){return l + r;});
  return valsize * sizeof(size_t) + nullsize * sizeof(size_t);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::type_cast(const std::string& to_type) {
  std::shared_ptr<dfcolumn> ret;
  if(to_type == "boolean") {
    auto ddic = dic->get(0).decompress();
    auto b_words_to_bool_map = broadcast(words_to_bool(ddic));
    auto newcol = b_words_to_bool_map.map(vector_take<int,int>, val);
    ret = std::make_shared<typed_dfcolumn<int>>(std::move(newcol), nulls);
  } else {
    throw std::runtime_error("dic_string column doesn't support casting to: " + to_type);
  }
  return ret;
}

}
