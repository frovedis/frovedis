#include "dfcolumn.hpp"

#include <limits>
#include <climits>
#include "join.hpp"
#include "dfcolumn_helper.hpp"

using namespace std;

namespace frovedis {

template <class T>
void hash_join_eq_helper(std::vector<std::vector<T>>& left_vals,
                         std::vector<std::vector<size_t>>& left_idxs,
                         std::vector<std::vector<T>>& right_vals,
                         std::vector<std::vector<size_t>>& right_idxs,
                         std::vector<size_t>& left_idx_out,
                         std::vector<size_t>& right_idx_out) {
  time_spent t(DEBUG);
  size_t left_size = 0;
  for(size_t i = 0; i < left_vals.size(); i++)
    left_size += left_vals[i].size();
  std::vector<T> left_val(left_size);
  std::vector<size_t> left_idx(left_size);
  T* left_valp = &left_val[0];
  size_t* left_idxp = &left_idx[0];
  size_t current = 0;
  for(size_t i = 0; i < left_vals.size(); i++) {
    T* left_valsp = &left_vals[i][0];
    size_t* left_idxsp = &left_idxs[i][0];
    size_t left_vals_size = left_vals[i].size();
    T* left_valp_current = left_valp + current;
    size_t* left_idxp_current = left_idxp + current;
    for(size_t j = 0; j < left_vals_size; j++) {
      left_valp_current[j] = left_valsp[j];
      left_idxp_current[j] = left_idxsp[j];
    }
    current += left_vals_size;
  }
  size_t right_size = 0;
  for(size_t i = 0; i < right_vals.size(); i++)
    right_size += right_vals[i].size();
  std::vector<T> right_val(right_size);
  std::vector<size_t> right_idx(right_size);
  T* right_valp = &right_val[0];
  size_t* right_idxp = &right_idx[0];
  current = 0;
  for(size_t i = 0; i < right_vals.size(); i++) {
    T* right_valsp = &right_vals[i][0];
    size_t* right_idxsp = &right_idxs[i][0];
    size_t right_vals_size = right_vals[i].size();
    T* right_valp_current = right_valp + current;
    size_t* right_idxp_current = right_idxp + current;
    for(size_t j = 0; j < right_vals_size; j++) {
      right_valp_current[j] = right_valsp[j];
      right_idxp_current[j] = right_idxsp[j];
    }
    current += right_vals_size;
  }
  t.show("prepare: ");
  equi_join(left_val, left_idx, right_val, right_idx,
            left_idx_out, right_idx_out);
  t.show("join: ");
}

template <class T>
std::vector<size_t>
outer_hash_join_eq_helper(std::vector<std::vector<T>>& left_vals,
                          std::vector<std::vector<size_t>>& left_idxs,
                          std::vector<std::vector<T>>& right_vals,
                          std::vector<std::vector<size_t>>& right_idxs,
                          std::vector<size_t>& left_idx_out,
                          std::vector<size_t>& right_idx_out) {
  size_t left_size = 0;
  for(size_t i = 0; i < left_vals.size(); i++)
    left_size += left_vals[i].size();
  std::vector<T> left_val(left_size);
  std::vector<size_t> left_idx(left_size);
  T* left_valp = &left_val[0];
  size_t* left_idxp = &left_idx[0];
  size_t current = 0;
  for(size_t i = 0; i < left_vals.size(); i++) {
    T* left_valsp = &left_vals[i][0];
    size_t* left_idxsp = &left_idxs[i][0];
    size_t left_vals_size = left_vals[i].size();
    T* left_valp_current = left_valp + current;
    size_t* left_idxp_current = left_idxp + current;
    for(size_t j = 0; j < left_vals_size; j++) {
      left_valp_current[j] = left_valsp[j];
      left_idxp_current[j] = left_idxsp[j];
    }
    current += left_vals_size;
  }
  size_t right_size = 0;
  for(size_t i = 0; i < right_vals.size(); i++)
    right_size += right_vals[i].size();
  std::vector<T> right_val(right_size);
  std::vector<size_t> right_idx(right_size);
  T* right_valp = &right_val[0];
  size_t* right_idxp = &right_idx[0];
  current = 0;
  for(size_t i = 0; i < right_vals.size(); i++) {
    T* right_valsp = &right_vals[i][0];
    size_t* right_idxsp = &right_idxs[i][0];
    size_t right_vals_size = right_vals[i].size();
    T* right_valp_current = right_valp + current;
    size_t* right_idxp_current = right_idxp + current;
    for(size_t j = 0; j < right_vals_size; j++) {
      right_valp_current[j] = right_valsp[j];
      right_idxp_current[j] = right_idxsp[j];
    }
    current += right_vals_size;
  }
  return outer_equi_join(left_val, left_idx, right_val, right_idx,
                         left_idx_out, right_idx_out);
}

template <class T>
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<T>::hash_join_eq
  (std::shared_ptr<dfcolumn>& right,
   // might be filtered index
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx) {
  time_spent t(DEBUG);
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<T>>(right);
  if(!static_cast<bool>(right2)) 
    throw std::runtime_error("hash_join_eq: column types are different");
  auto left_split_val =
    make_node_local_allocate<std::vector<std::vector<T>>>();
  auto left_split_idx =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  auto right_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto right_split_val =
    make_node_local_allocate<std::vector<std::vector<T>>>();
  auto right_split_idx =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  t.show("hash_join_eq, init: ");
  if(contain_nulls) {
    auto left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    auto left_non_null_val = val.map(extract_non_null<T>, left_full_local_idx,
                                     nulls, left_non_null_idx);
    t.show("hash_join_eq, extract_non_null left: ");
    auto left_global_idx = local_to_global_idx(left_non_null_idx);
    left_non_null_val.mapv(split_by_hash<T>, left_split_val, left_global_idx,
                           left_split_idx);
    t.show("hash_join_eq, split_by_hash left: ");
  } else {
    auto left_non_null_val = val.map(extract_helper2<T>, left_full_local_idx);
    auto left_global_idx = local_to_global_idx(left_full_local_idx);
    left_non_null_val.mapv(split_by_hash<T>, left_split_val, left_global_idx,
                           left_split_idx);
    t.show("hash_join_eq, split_by_hash left: ");
  }
  auto left_exchanged_val = alltoall_exchange(left_split_val);
  auto left_exchanged_idx = alltoall_exchange(left_split_idx);
  t.show("hash_join_eq, alltoall_exchange left: ");
  auto& right_val = right2->val;
  auto& right_nulls = right2->nulls;
  if(right2->contain_nulls) {
    auto right_non_null_val =
      right_val.map(extract_non_null<T>, right_full_local_idx, right_nulls,
                    right_non_null_idx);
    t.show("hash_join_eq, extract_non_null right: ");
    auto right_global_idx = local_to_global_idx(right_non_null_idx);
    right_non_null_val.mapv(split_by_hash<T>, right_split_val,
                            right_global_idx, right_split_idx);
    t.show("hash_join_eq, split_by_hash right: ");
  } else {
    auto right_non_null_val =
      right_val.map(extract_helper2<T>, right_full_local_idx);
    t.show("hash_join_eq, extract_non_null right: ");
    auto right_global_idx = local_to_global_idx(right_full_local_idx);
    right_non_null_val.mapv(split_by_hash<T>, right_split_val,
                            right_global_idx, right_split_idx);
    t.show("hash_join_eq, split_by_hash right: ");
  }
  auto right_exchanged_val = alltoall_exchange(right_split_val);
  auto right_exchanged_idx = alltoall_exchange(right_split_idx);
  t.show("hash_join_eq, alltoall_exchange right: ");
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto right_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  left_exchanged_val.mapv(hash_join_eq_helper<T>, left_exchanged_idx,
                          right_exchanged_val, right_exchanged_idx,
                          left_idx_ret, right_idx_ret);
  t.show("hash_join_eq, hash_join_eq_helper: ");
  return std::make_pair(std::move(left_idx_ret), std::move(right_idx_ret));
}

template <class T>
std::tuple<node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>>
typed_dfcolumn<T>::outer_hash_join_eq
  (std::shared_ptr<dfcolumn>& right,
   // might be filtered index
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<T>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("outer_hash_join_eq: column types are different");
  auto left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto left_split_val =
    make_node_local_allocate<std::vector<std::vector<T>>>();
  auto left_split_idx =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  auto right_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto right_split_val =
    make_node_local_allocate<std::vector<std::vector<T>>>();
  auto right_split_idx =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  if(contain_nulls) {
    auto left_non_null_val = val.map(extract_non_null<T>, left_full_local_idx,
                                     nulls, left_non_null_idx);
    auto left_global_idx = local_to_global_idx(left_non_null_idx);
    left_non_null_val.mapv(split_by_hash<T>, left_split_val, left_global_idx,
                           left_split_idx);
  } else {
    auto left_non_null_val = val.map(extract_helper2<T>, left_full_local_idx);
    auto left_global_idx = local_to_global_idx(left_full_local_idx);
    left_non_null_val.mapv(split_by_hash<T>, left_split_val, left_global_idx,
                           left_split_idx);
  }
  auto left_exchanged_val = alltoall_exchange(left_split_val);
  auto left_exchanged_idx = alltoall_exchange(left_split_idx);
  auto& right_val = right2->val;
  auto& right_nulls = right2->nulls;
  if(right2->contain_nulls) {
    auto right_non_null_val =
      right_val.map(extract_non_null<T>, right_full_local_idx, right_nulls,
                    right_non_null_idx);
    auto right_global_idx = local_to_global_idx(right_non_null_idx);
    right_non_null_val.mapv(split_by_hash<T>, right_split_val,
                            right_global_idx, right_split_idx);
  } else {
    auto right_non_null_val =
      right_val.map(extract_helper2<T>, right_full_local_idx);
    auto right_global_idx = local_to_global_idx(right_full_local_idx);
    right_non_null_val.mapv(split_by_hash<T>, right_split_val,
                            right_global_idx, right_split_idx);
  }
  auto right_exchanged_val = alltoall_exchange(right_split_val);
  auto right_exchanged_idx = alltoall_exchange(right_split_idx);
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto right_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto null_idx_ret = left_exchanged_val.map(outer_hash_join_eq_helper<T>,
                                             left_exchanged_idx,
                                             right_exchanged_val,
                                             right_exchanged_idx,
                                             left_idx_ret,
                                             right_idx_ret);
  return std::make_tuple(std::move(left_idx_ret), std::move(right_idx_ret),
                         std::move(null_idx_ret));
}

// TODO: if right is not filtered, creating/sending index is not needed
// (also for outer, star_join)
template <class T>
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<T>::bcast_join_eq
(std::shared_ptr<dfcolumn>& right,
 // might be filtered index
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<T>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("bcast_join_eq: column types are different");
  node_local<std::vector<size_t>> left_non_null_idx;
  node_local<std::vector<T>> left_non_null_val;
  if(contain_nulls) {
    left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    left_non_null_val = val.map(extract_non_null<T>, left_full_local_idx,
                                nulls, left_non_null_idx);
  } else {
    left_non_null_val = val.map(extract_helper2<T>, left_full_local_idx);
    left_non_null_idx = std::move(left_full_local_idx);
  }
  auto right_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto& right_val = right2->val;
  auto& right_nulls = right2->nulls;
  node_local<std::vector<T>> right_non_null_val;
  node_local<std::vector<size_t>> right_global_idx;
  if(right2->contain_nulls) {
    right_non_null_val =
      right_val.map(extract_non_null<T>, right_full_local_idx, right_nulls,
                    right_non_null_idx);
    right_global_idx = local_to_global_idx(right_non_null_idx);
  } else {
    right_non_null_val = right_val.map(extract_helper2<T>,
                                       right_full_local_idx);
    right_global_idx = local_to_global_idx(right_full_local_idx);
  }
  // TODO: write allreduce for dvector (for PoD) for better performance
  // (also for outer, star_join)
  auto right_non_null_val_bcast =
    broadcast(right_non_null_val.template viewas_dvector<T>().gather());
  auto right_global_idx_bcast =
    broadcast(right_global_idx.template viewas_dvector<size_t>().gather());
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto right_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  left_non_null_val.mapv(equi_join<T>, left_non_null_idx,
                         right_non_null_val_bcast, right_global_idx_bcast,
                         left_idx_ret, right_idx_ret);
  return std::make_pair(std::move(left_idx_ret), std::move(right_idx_ret));
}

template <class T>
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<T>::bcast_join_lt
(std::shared_ptr<dfcolumn>& right,
 // might be filtered index
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<T>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("bcast_join_lt: column types are different");
  node_local<std::vector<size_t>> left_non_null_idx;
  node_local<std::vector<T>> left_non_null_val;
  if(contain_nulls) {
    left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    left_non_null_val = val.map(extract_non_null<T>, left_full_local_idx,
                                nulls, left_non_null_idx);
  } else {
    left_non_null_val = val.map(extract_helper2<T>, left_full_local_idx);
    left_non_null_idx = std::move(left_full_local_idx);
  }
  auto right_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto& right_val = right2->val;
  auto& right_nulls = right2->nulls;
  node_local<std::vector<T>> right_non_null_val;
  node_local<std::vector<size_t>> right_global_idx;
  if(right2->contain_nulls) {
    right_non_null_val =
      right_val.map(extract_non_null<T>, right_full_local_idx, right_nulls,
                    right_non_null_idx);
    right_global_idx = local_to_global_idx(right_non_null_idx);
  } else {
    right_non_null_val = right_val.map(extract_helper2<T>,
                                       right_full_local_idx);
    right_global_idx = local_to_global_idx(right_full_local_idx);
  }
  // TODO: write allreduce for dvector (for PoD) for better performance
  // (also for outer, star_join)
  auto right_non_null_val_bcast =
    broadcast(right_non_null_val.template viewas_dvector<T>().gather());
  auto right_global_idx_bcast =
    broadcast(right_global_idx.template viewas_dvector<size_t>().gather());
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto right_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  left_non_null_val.mapv(lt_join<T>, left_non_null_idx,
                         right_non_null_val_bcast, right_global_idx_bcast,
                         left_idx_ret, right_idx_ret);
  return std::make_pair(std::move(left_idx_ret), std::move(right_idx_ret));
}

template <class T>
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<T>::bcast_join_le
(std::shared_ptr<dfcolumn>& right,
 // might be filtered index
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<T>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("bcast_join_le: column types are different");
  node_local<std::vector<size_t>> left_non_null_idx;
  node_local<std::vector<T>> left_non_null_val;
  if(contain_nulls) {
    left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    left_non_null_val = val.map(extract_non_null<T>, left_full_local_idx,
                                nulls, left_non_null_idx);
  } else {
    left_non_null_val = val.map(extract_helper2<T>, left_full_local_idx);
    left_non_null_idx = std::move(left_full_local_idx);
  }
  auto right_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto& right_val = right2->val;
  auto& right_nulls = right2->nulls;
  node_local<std::vector<T>> right_non_null_val;
  node_local<std::vector<size_t>> right_global_idx;
  if(right2->contain_nulls) {
    right_non_null_val =
      right_val.map(extract_non_null<T>, right_full_local_idx, right_nulls,
                    right_non_null_idx);
    right_global_idx = local_to_global_idx(right_non_null_idx);
  } else {
    right_non_null_val = right_val.map(extract_helper2<T>,
                                       right_full_local_idx);
    right_global_idx = local_to_global_idx(right_full_local_idx);
  }
  // TODO: write allreduce for dvector (for PoD) for better performance
  // (also for outer, star_join)
  auto right_non_null_val_bcast =
    broadcast(right_non_null_val.template viewas_dvector<T>().gather());
  auto right_global_idx_bcast =
    broadcast(right_global_idx.template viewas_dvector<size_t>().gather());
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto right_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  left_non_null_val.mapv(le_join<T>, left_non_null_idx,
                         right_non_null_val_bcast, right_global_idx_bcast,
                         left_idx_ret, right_idx_ret);
  return std::make_pair(std::move(left_idx_ret), std::move(right_idx_ret));
}

template <class T>
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<T>::bcast_join_gt
(std::shared_ptr<dfcolumn>& right,
 // might be filtered index
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<T>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("bcast_join_lt: column types are different");
  node_local<std::vector<size_t>> left_non_null_idx;
  node_local<std::vector<T>> left_non_null_val;
  if(contain_nulls) {
    left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    left_non_null_val = val.map(extract_non_null<T>, left_full_local_idx,
                                nulls, left_non_null_idx);
  } else {
    left_non_null_val = val.map(extract_helper2<T>, left_full_local_idx);
    left_non_null_idx = std::move(left_full_local_idx);
  }
  auto right_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto& right_val = right2->val;
  auto& right_nulls = right2->nulls;
  node_local<std::vector<T>> right_non_null_val;
  node_local<std::vector<size_t>> right_global_idx;
  if(right2->contain_nulls) {
    right_non_null_val =
      right_val.map(extract_non_null<T>, right_full_local_idx, right_nulls,
                    right_non_null_idx);
    right_global_idx = local_to_global_idx(right_non_null_idx);
  } else {
    right_non_null_val = right_val.map(extract_helper2<T>,
                                       right_full_local_idx);
    right_global_idx = local_to_global_idx(right_full_local_idx);
  }
  // TODO: write allreduce for dvector (for PoD) for better performance
  // (also for outer, star_join)
  auto right_non_null_val_bcast =
    broadcast(right_non_null_val.template viewas_dvector<T>().gather());
  auto right_global_idx_bcast =
    broadcast(right_global_idx.template viewas_dvector<size_t>().gather());
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto right_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  left_non_null_val.mapv(gt_join<T>, left_non_null_idx,
                         right_non_null_val_bcast, right_global_idx_bcast,
                         left_idx_ret, right_idx_ret);
  return std::make_pair(std::move(left_idx_ret), std::move(right_idx_ret));
}

template <class T>
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<T>::bcast_join_ge
(std::shared_ptr<dfcolumn>& right,
 // might be filtered index
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<T>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("bcast_join_lt: column types are different");
  node_local<std::vector<size_t>> left_non_null_idx;
  node_local<std::vector<T>> left_non_null_val;
  if(contain_nulls) {
    left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    left_non_null_val = val.map(extract_non_null<T>, left_full_local_idx,
                                nulls, left_non_null_idx);
  } else {
    left_non_null_val = val.map(extract_helper2<T>, left_full_local_idx);
    left_non_null_idx = std::move(left_full_local_idx);
  }
  auto right_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto& right_val = right2->val;
  auto& right_nulls = right2->nulls;
  node_local<std::vector<T>> right_non_null_val;
  node_local<std::vector<size_t>> right_global_idx;
  if(right2->contain_nulls) {
    right_non_null_val =
      right_val.map(extract_non_null<T>, right_full_local_idx, right_nulls,
                    right_non_null_idx);
    right_global_idx = local_to_global_idx(right_non_null_idx);
  } else {
    right_non_null_val = right_val.map(extract_helper2<T>,
                                       right_full_local_idx);
    right_global_idx = local_to_global_idx(right_full_local_idx);
  }
  // TODO: write allreduce for dvector (for PoD) for better performance
  // (also for outer, star_join)
  auto right_non_null_val_bcast =
    broadcast(right_non_null_val.template viewas_dvector<T>().gather());
  auto right_global_idx_bcast =
    broadcast(right_global_idx.template viewas_dvector<size_t>().gather());
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto right_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  left_non_null_val.mapv(ge_join<T>, left_non_null_idx,
                         right_non_null_val_bcast, right_global_idx_bcast,
                         left_idx_ret, right_idx_ret);
  return std::make_pair(std::move(left_idx_ret), std::move(right_idx_ret));
}

template <class T>
std::tuple<node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>>
typed_dfcolumn<T>::outer_bcast_join_eq
  (std::shared_ptr<dfcolumn>& right,
   // might be filtered index
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<T>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("bcast_join_eq: column types are different");
  node_local<std::vector<size_t>> left_non_null_idx;
  node_local<std::vector<T>> left_non_null_val;
  if(contain_nulls) {
    left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    left_non_null_val = val.map(extract_non_null<T>, left_full_local_idx,
                                nulls, left_non_null_idx);
  } else {
    left_non_null_val = val.map(extract_helper2<T>, left_full_local_idx);
    left_non_null_idx = std::move(left_full_local_idx);
  }
  auto right_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto& right_val = right2->val;
  auto& right_nulls = right2->nulls;
  node_local<std::vector<T>> right_non_null_val;
  node_local<std::vector<size_t>> right_global_idx;
  if(right2->contain_nulls) {
    right_non_null_val =
      right_val.map(extract_non_null<T>, right_full_local_idx, right_nulls,
                    right_non_null_idx);
    right_global_idx = local_to_global_idx(right_non_null_idx);
  } else {
    right_non_null_val = right_val.map(extract_helper2<T>,
                                       right_full_local_idx);
    right_global_idx = local_to_global_idx(right_full_local_idx);
  }
  auto right_non_null_val_bcast =
    broadcast(right_non_null_val.template viewas_dvector<T>().gather());
  auto right_global_idx_bcast =
    broadcast(right_global_idx.template viewas_dvector<size_t>().gather());
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto right_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto null_idx_ret = left_non_null_val.map(outer_equi_join<T>,
                                            left_non_null_idx,
                                            right_non_null_val_bcast,
                                            right_global_idx_bcast,
                                            left_idx_ret, right_idx_ret);
  return std::make_tuple(std::move(left_idx_ret), std::move(right_idx_ret),
                         std::move(null_idx_ret));
}

template <class T>
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<T>::star_join_eq
  (std::shared_ptr<dfcolumn>& right,
   // might be filtered index
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<T>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("star_join_eq: column types are different");
  node_local<std::vector<size_t>> left_non_null_idx;
  node_local<std::vector<T>> left_non_null_val;
  if(contain_nulls) {
    left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    left_non_null_val = val.map(extract_non_null<T>, left_full_local_idx,
                                nulls, left_non_null_idx);
  } else {
    left_non_null_val = val.map(extract_helper2<T>, left_full_local_idx);
    left_non_null_idx = left_full_local_idx; // cannot move; reused
  }
  auto right_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto& right_val = right2->val;
  auto& right_nulls = right2->nulls;
  node_local<std::vector<T>> right_non_null_val;
  node_local<std::vector<size_t>> right_global_idx;
  if(right2->contain_nulls) {
    right_non_null_val =
      right_val.map(extract_non_null<T>, right_full_local_idx, right_nulls,
                    right_non_null_idx);
    right_global_idx = local_to_global_idx(right_non_null_idx);
  } else {
    right_non_null_val = right_val.map(extract_helper2<T>,
                                       right_full_local_idx);
    right_global_idx = local_to_global_idx(right_full_local_idx);
  }
  auto right_non_null_val_bcast = 
    broadcast(right_non_null_val.template viewas_dvector<T>().gather());
  auto right_global_idx_bcast =
    broadcast(right_global_idx.template viewas_dvector<size_t>().gather());
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto right_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto missed = 
    left_non_null_val.map(unique_equi_join2<T>, left_non_null_idx,
                          right_non_null_val_bcast, right_global_idx_bcast,
                          right_idx_ret);
  return std::make_pair(std::move(right_idx_ret), std::move(missed));
}

template <class T>
node_local<std::vector<size_t>> 
typed_dfcolumn<T>::calc_hash_base_multi_join(std::shared_ptr<dfcolumn>& c) {
  return calc_hash_base();
}

template <class T>
void 
typed_dfcolumn<T>::calc_hash_base_multi_join
(node_local<std::vector<size_t>>& hash_base, int shift,
 std::shared_ptr<dfcolumn>& c) {
  calc_hash_base(hash_base, shift);
}


template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<int>::hash_join_eq
  (std::shared_ptr<dfcolumn>& right,
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<unsigned int>::hash_join_eq
  (std::shared_ptr<dfcolumn>& right,
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<long>::hash_join_eq
  (std::shared_ptr<dfcolumn>& right,
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<unsigned long>::hash_join_eq
  (std::shared_ptr<dfcolumn>& right,
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<float>::hash_join_eq
  (std::shared_ptr<dfcolumn>& right,
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<double>::hash_join_eq
  (std::shared_ptr<dfcolumn>& right,
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);

template
std::tuple<node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>>
typed_dfcolumn<int>::outer_hash_join_eq
  (std::shared_ptr<dfcolumn>& right,
   // might be filtered index
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);
template
std::tuple<node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>>
typed_dfcolumn<unsigned int>::outer_hash_join_eq
  (std::shared_ptr<dfcolumn>& right,
   // might be filtered index
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);
template
std::tuple<node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>>
typed_dfcolumn<long>::outer_hash_join_eq
  (std::shared_ptr<dfcolumn>& right,
   // might be filtered index
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);
template
std::tuple<node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>>
typed_dfcolumn<unsigned long>::outer_hash_join_eq
  (std::shared_ptr<dfcolumn>& right,
   // might be filtered index
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);
template
std::tuple<node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>>
typed_dfcolumn<float>::outer_hash_join_eq
  (std::shared_ptr<dfcolumn>& right,
   // might be filtered index
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);
template
std::tuple<node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>>
typed_dfcolumn<double>::outer_hash_join_eq
  (std::shared_ptr<dfcolumn>& right,
   // might be filtered index
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);

template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<int>::bcast_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<unsigned int>::bcast_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<long>::bcast_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<unsigned long>::bcast_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<float>::bcast_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<double>::bcast_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);

template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<int>::bcast_join_lt
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<unsigned int>::bcast_join_lt
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<long>::bcast_join_lt
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<unsigned long>::bcast_join_lt
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<float>::bcast_join_lt
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<double>::bcast_join_lt
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);

template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<int>::bcast_join_le
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<unsigned int>::bcast_join_le
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<long>::bcast_join_le
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<unsigned long>::bcast_join_le
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<float>::bcast_join_le
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<double>::bcast_join_le
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);

template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<int>::bcast_join_gt
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<unsigned int>::bcast_join_gt
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<long>::bcast_join_gt
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<unsigned long>::bcast_join_gt
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<float>::bcast_join_gt
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<double>::bcast_join_gt
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);

template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<int>::bcast_join_ge
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<unsigned int>::bcast_join_ge
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<long>::bcast_join_ge
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<unsigned long>::bcast_join_ge
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<float>::bcast_join_ge
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<double>::bcast_join_ge
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx);

template
std::tuple<node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>>
typed_dfcolumn<int>::outer_bcast_join_eq
  (std::shared_ptr<dfcolumn>& right,
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);
template
std::tuple<node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>>
typed_dfcolumn<unsigned int>::outer_bcast_join_eq
  (std::shared_ptr<dfcolumn>& right,
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);
template
std::tuple<node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>>
typed_dfcolumn<long>::outer_bcast_join_eq
  (std::shared_ptr<dfcolumn>& right,
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);
template
std::tuple<node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>>
typed_dfcolumn<unsigned long>::outer_bcast_join_eq
  (std::shared_ptr<dfcolumn>& right,
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);
template
std::tuple<node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>>
typed_dfcolumn<float>::outer_bcast_join_eq
  (std::shared_ptr<dfcolumn>& right,
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);
template
std::tuple<node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>>
typed_dfcolumn<double>::outer_bcast_join_eq
  (std::shared_ptr<dfcolumn>& right,
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);

template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<int>::star_join_eq
  (std::shared_ptr<dfcolumn>& right,
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<unsigned int>::star_join_eq
  (std::shared_ptr<dfcolumn>& right,
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<long>::star_join_eq
  (std::shared_ptr<dfcolumn>& right,
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<unsigned long>::star_join_eq
  (std::shared_ptr<dfcolumn>& right,
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<float>::star_join_eq
  (std::shared_ptr<dfcolumn>& right,
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);
template
std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<double>::star_join_eq
  (std::shared_ptr<dfcolumn>& right,
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx);

template
node_local<std::vector<size_t>> 
typed_dfcolumn<int>::calc_hash_base_multi_join(std::shared_ptr<dfcolumn>& c);
template
node_local<std::vector<size_t>> 
typed_dfcolumn<unsigned int>::calc_hash_base_multi_join(std::shared_ptr<dfcolumn>& c);
template
node_local<std::vector<size_t>> 
typed_dfcolumn<long>::calc_hash_base_multi_join(std::shared_ptr<dfcolumn>& c);
template
node_local<std::vector<size_t>> 
typed_dfcolumn<unsigned long>::calc_hash_base_multi_join(std::shared_ptr<dfcolumn>& c);
template
node_local<std::vector<size_t>> 
typed_dfcolumn<float>::calc_hash_base_multi_join(std::shared_ptr<dfcolumn>& c);
template
node_local<std::vector<size_t>> 
typed_dfcolumn<double>::calc_hash_base_multi_join(std::shared_ptr<dfcolumn>& c);

template void 
typed_dfcolumn<int>::calc_hash_base_multi_join
(node_local<std::vector<size_t>>& hash_base, int shift,
 std::shared_ptr<dfcolumn>& c);
template void 
typed_dfcolumn<unsigned int>::calc_hash_base_multi_join
(node_local<std::vector<size_t>>& hash_base, int shift,
 std::shared_ptr<dfcolumn>& c);
template void 
typed_dfcolumn<long>::calc_hash_base_multi_join
(node_local<std::vector<size_t>>& hash_base, int shift,
 std::shared_ptr<dfcolumn>& c);
template void 
typed_dfcolumn<unsigned long>::calc_hash_base_multi_join
(node_local<std::vector<size_t>>& hash_base, int shift,
 std::shared_ptr<dfcolumn>& c);
template void 
typed_dfcolumn<float>::calc_hash_base_multi_join
(node_local<std::vector<size_t>>& hash_base, int shift,
 std::shared_ptr<dfcolumn>& c);
template void 
typed_dfcolumn<double>::calc_hash_base_multi_join
(node_local<std::vector<size_t>>& hash_base, int shift,
 std::shared_ptr<dfcolumn>& c);

// ----- dic_string -----
std::pair<node_local<std::vector<size_t>>, node_local<std::vector<size_t>>>
typed_dfcolumn<dic_string>::hash_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(right);
  if(!static_cast<bool>(right2)) 
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
  if(!static_cast<bool>(right2))
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
  if(!static_cast<bool>(right2))
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
  if(!static_cast<bool>(right2))
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
  if(!static_cast<bool>(right2))
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

// in dfcolumn_filter_eq.cc
std::vector<size_t>
dic_string_equal_prepare_helper(const std::vector<size_t>& rval,
                                const std::vector<size_t>& trans);

// same as equal_prepare; argument is not shared_ptr
node_local<vector<size_t>>
typed_dfcolumn<dic_string>::
equal_prepare_multi_join(typed_dfcolumn<dic_string>& right) {
  if(dic == right.dic) {
    return right.val;
  } else {
    auto& left_dic = *dic;
    auto& right_dic = *(right.dic);
    compressed_words to_lookup;
    auto right_num_words = right_dic.num_words();
    to_lookup.cwords.swap(right_dic.cwords);
    to_lookup.lens.swap(right_dic.lens);
    to_lookup.lens_num.swap(right_dic.lens_num);
    to_lookup.order.resize(right_num_words);
    auto orderp = to_lookup.order.data();
    for(size_t i = 0; i < right_num_words; i++) orderp[i] = i;
    auto trans_table = broadcast(left_dic.lookup(to_lookup));
    to_lookup.cwords.swap(right_dic.cwords);
    to_lookup.lens.swap(right_dic.lens);
    to_lookup.lens_num.swap(right_dic.lens_num);
    return right.val.map(dic_string_equal_prepare_helper, trans_table);
  }
}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::calc_hash_base_multi_join
(std::shared_ptr<dfcolumn>& left) {
  auto left2 = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(left);
  if(!static_cast<bool>(left2))
    throw std::runtime_error("multi_join: column types are different");
  auto thisval = left2->equal_prepare_multi_join(*this);
  return thisval.map(calc_hash_base_helper<size_t>);
}

void typed_dfcolumn<dic_string>::calc_hash_base_multi_join
(node_local<std::vector<size_t>>& hash_base, int shift,
 std::shared_ptr<dfcolumn>& left) {
  auto left2 = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(left);
  if(!static_cast<bool>(left2))
    throw std::runtime_error("multi_join: column types are different");
  auto thisval = left2->equal_prepare_multi_join(*this);
  thisval.mapv(calc_hash_base_helper2<size_t>(shift), hash_base);
}

// ----- string -----
std::pair<node_local<std::vector<size_t>>, node_local<std::vector<size_t>>>
typed_dfcolumn<string>::hash_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<string>>(right);
  if(!static_cast<bool>(right2)) 
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
typed_dfcolumn<string>::outer_hash_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<string>>(right);
  if(!static_cast<bool>(right2))
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
typed_dfcolumn<string>::bcast_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<string>>(right);
  if(!static_cast<bool>(right2))
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
typed_dfcolumn<string>::outer_bcast_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<string>>(right);
  if(!static_cast<bool>(right2))
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
typed_dfcolumn<string>::star_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<string>>(right);
  if(!static_cast<bool>(right2))
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

// in dfcolumn_filter_eq.cc
void create_string_trans_table(my_map<string,size_t>& leftdic,
                               my_map<string,size_t>& rightdic,
                               vector<size_t>& from,
                               vector<size_t>& to);
vector<size_t> equal_prepare_helper(vector<size_t>& val,
                                    vector<size_t>& from,
                                    vector<size_t>& to);

// same as equal_prepare; argument is not shared_ptr
node_local<vector<size_t>>
typed_dfcolumn<string>::
equal_prepare_multi_join(typed_dfcolumn<string>& right) {
  auto& rightdic = *(right.dic);
  auto rightdicnl = rightdic.viewas_node_local();
  auto nlfrom = make_node_local_allocate<std::vector<size_t>>();
  auto nlto = make_node_local_allocate<std::vector<size_t>>();
  dic->viewas_node_local().mapv(create_string_trans_table, rightdicnl,
                                nlfrom, nlto);
  auto bcastfrom = broadcast(nlfrom.moveto_dvector<size_t>().gather());
  auto bcastto = broadcast(nlto.moveto_dvector<size_t>().gather());
  return right.val.map(equal_prepare_helper, bcastfrom, bcastto);
}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::calc_hash_base_multi_join
(std::shared_ptr<dfcolumn>& left) {
  auto left2 = std::dynamic_pointer_cast<typed_dfcolumn<string>>(left);
  if(!static_cast<bool>(left2))
    throw std::runtime_error("multi_join: column types are different");
  auto thisval = left2->equal_prepare_multi_join(*this);
  return thisval.map(calc_hash_base_helper<size_t>);
}

void typed_dfcolumn<string>::calc_hash_base_multi_join
(node_local<std::vector<size_t>>& hash_base, int shift,
 std::shared_ptr<dfcolumn>& left) {
  auto left2 = std::dynamic_pointer_cast<typed_dfcolumn<string>>(left);
  if(!static_cast<bool>(left2))
    throw std::runtime_error("multi_join: column types are different");
  auto thisval = left2->equal_prepare_multi_join(*this);
  thisval.mapv(calc_hash_base_helper2<size_t>(shift), hash_base);
}

}
