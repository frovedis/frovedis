#ifndef DFCOLUMN__IMPL_HPP
#define DFCOLUMN__IMPL_HPP

#include "dfcolumn.hpp"

#include <limits>
#include "set_operations.hpp"
#include "../core/radix_sort.hpp"
#include "hashtable.hpp"
#include "join.hpp"

#include <boost/lexical_cast.hpp>

#define GROUPBY_VLEN 256

namespace frovedis {

#if defined(_SX) || defined(__ve__)
template <class K, class V>
void sort_pair(std::vector<K>& key_array, std::vector<V>& val_array) {
  radix_sort(key_array, val_array);
}
template <class K, class V>
void sort_pair_desc(std::vector<K>& key_array, std::vector<V>& val_array) {
  radix_sort_desc(key_array, val_array);
}
#else
template <class K, class V>
struct sort_pair_helper {
  bool operator()(const std::pair<K,V>& left,
                  const std::pair<K,V>& right) {
    return left.first < right.first;
  }
};
template <class K, class V>
void sort_pair(std::vector<K>& key_array, std::vector<V>& val_array) {
  size_t size = key_array.size();
  if(val_array.size() != size)
    throw std::runtime_error("sort_pair: different size of arrays");
  std::vector<std::pair<K,V>> kv_pair(size);
  for(size_t i = 0; i < size; i++) {
    kv_pair[i].first = key_array[i];
    kv_pair[i].second = val_array[i];
  }
  std::stable_sort(kv_pair.begin(), kv_pair.end(), sort_pair_helper<K,V>());
  for(size_t i = 0; i < size; i++) {
    key_array[i] = kv_pair[i].first;
    val_array[i] = kv_pair[i].second;
  }
}
template <class K, class V>
struct sort_pair_desc_helper {
  bool operator()(const std::pair<K,V>& left,
                  const std::pair<K,V>& right) {
    return left.first > right.first;
  }
};
template <class K, class V>
void sort_pair_desc(std::vector<K>& key_array, std::vector<V>& val_array) {
  size_t size = key_array.size();
  if(val_array.size() != size)
    throw std::runtime_error("sort_pair: different size of arrays");
  std::vector<std::pair<K,V>> kv_pair(size);
  for(size_t i = 0; i < size; i++) {
    kv_pair[i].first = key_array[i];
    kv_pair[i].second = val_array[i];
  }
  std::stable_sort(kv_pair.begin(), kv_pair.end(),
                   sort_pair_desc_helper<K,V>());
  for(size_t i = 0; i < size; i++) {
    key_array[i] = kv_pair[i].first;
    val_array[i] = kv_pair[i].second;
  }
}
#endif

template <class T>
std::vector<std::string> as_string_helper(const std::vector<T>& val) {
  std::vector<std::string> ret(val.size());
  for(size_t i = 0; i < val.size(); i++) {
    if(val[i] ==  std::numeric_limits<T>::max()) ret[i] = "NULL";
    else ret[i] = boost::lexical_cast<std::string>(val[i]);
  }
  return ret;
}

template <class T>
std::vector<size_t> filter_eq_helper(std::vector<T>& left,
                                     std::vector<T>& right) {
  std::vector<size_t> tmp(left.size()); // large enough size
  size_t current = 0;
  T* leftp = &left[0];
  T* rightp = &right[0];
  size_t* tmpp = &tmp[0];
  size_t size = left.size();
  for(size_t i = 0; i < size; i++) {
    if(leftp[i] == rightp[i]) tmpp[current++] = i;
  }
  std::vector<size_t> ret(current);
  size_t* retp = &ret[0];
  for(size_t i = 0; i < current; i++) retp[i] = tmpp[i];
  return ret;
}

template <class T>
std::vector<size_t> filter_eq_immed_helper(std::vector<T>& left,
                                           T right) {
  std::vector<size_t> tmp(left.size()); // large enough size
  size_t current = 0;
  T* leftp = &left[0];
  size_t* tmpp = &tmp[0];
  size_t size = left.size();
  for(size_t i = 0; i < size; i++) {
    if(leftp[i] == right) tmpp[current++] = i;
  }
  std::vector<size_t> ret(current);
  size_t* retp = &ret[0];
  for(size_t i = 0; i < current; i++) retp[i] = tmpp[i];
  return ret;
}

template <class T>
std::vector<size_t> filter_neq_helper(std::vector<T>& left,
                                      std::vector<T>& right) {
  std::vector<size_t> tmp(left.size()); // large enough size
  size_t current = 0;
  T* leftp = &left[0];
  T* rightp = &right[0];
  size_t* tmpp = &tmp[0];
  size_t size = left.size();
  for(size_t i = 0; i < size; i++) {
    if(leftp[i] != rightp[i]) tmpp[current++] = i;
  }
  std::vector<size_t> ret(current);
  size_t* retp = &ret[0];
  for(size_t i = 0; i < current; i++) retp[i] = tmpp[i];
  return ret;
}

template <class T>
std::vector<size_t> filter_neq_immed_helper(std::vector<T>& left,
                                           T right) {
  std::vector<size_t> tmp(left.size()); // large enough size
  size_t current = 0;
  T* leftp = &left[0];
  size_t* tmpp = &tmp[0];
  size_t size = left.size();
  for(size_t i = 0; i < size; i++) {
    if(leftp[i] != right) tmpp[current++] = i;
  }
  std::vector<size_t> ret(current);
  size_t* retp = &ret[0];
  for(size_t i = 0; i < current; i++) retp[i] = tmpp[i];
  return ret;
}

template <class T>
std::vector<size_t> filter_lt_helper(std::vector<T>& left,
                                     std::vector<T>& right) {
  std::vector<size_t> tmp(left.size()); // large enough size
  size_t current = 0;
  T* leftp = &left[0];
  T* rightp = &right[0];
  size_t* tmpp = &tmp[0];
  size_t size = left.size();
  for(size_t i = 0; i < size; i++) {
    if(leftp[i] < rightp[i]) tmpp[current++] = i;
  }
  std::vector<size_t> ret(current);
  size_t* retp = &ret[0];
  for(size_t i = 0; i < current; i++) retp[i] = tmpp[i];
  return ret;
}

template <class T>
std::vector<size_t> filter_lt_immed_helper(std::vector<T>& left,
                                           T right) {
  std::vector<size_t> tmp(left.size()); // large enough size
  size_t current = 0;
  T* leftp = &left[0];
  size_t* tmpp = &tmp[0];
  size_t size = left.size();
  for(size_t i = 0; i < size; i++) {
    if(leftp[i] < right) tmpp[current++] = i;
  }
  std::vector<size_t> ret(current);
  size_t* retp = &ret[0];
  for(size_t i = 0; i < current; i++) retp[i] = tmpp[i];
  return ret;
}

template <class T>
std::vector<size_t> filter_le_helper(std::vector<T>& left,
                                     std::vector<T>& right) {
  std::vector<size_t> tmp(left.size()); // large enough size
  size_t current = 0;
  T* leftp = &left[0];
  T* rightp = &right[0];
  size_t* tmpp = &tmp[0];
  size_t size = left.size();
  for(size_t i = 0; i < size; i++) {
    if(leftp[i] <= rightp[i]) tmpp[current++] = i;
  }
  std::vector<size_t> ret(current);
  size_t* retp = &ret[0];
  for(size_t i = 0; i < current; i++) retp[i] = tmpp[i];
  return ret;
}

template <class T>
std::vector<size_t> filter_le_immed_helper(std::vector<T>& left,
                                           T right) {
  std::vector<size_t> tmp(left.size()); // large enough size
  size_t current = 0;
  T* leftp = &left[0];
  size_t* tmpp = &tmp[0];
  size_t size = left.size();
  for(size_t i = 0; i < size; i++) {
    if(leftp[i] <= right) tmpp[current++] = i;
  }
  std::vector<size_t> ret(current);
  size_t* retp = &ret[0];
  for(size_t i = 0; i < current; i++) retp[i] = tmpp[i];
  return ret;
}

template <class T>
std::vector<size_t> filter_gt_helper(std::vector<T>& left,
                                     std::vector<T>& right) {
  std::vector<size_t> tmp(left.size()); // large enough size
  size_t current = 0;
  T* leftp = &left[0];
  T* rightp = &right[0];
  size_t* tmpp = &tmp[0];
  size_t size = left.size();
  for(size_t i = 0; i < size; i++) {
    if(leftp[i] > rightp[i]) tmpp[current++] = i;
  }
  std::vector<size_t> ret(current);
  size_t* retp = &ret[0];
  for(size_t i = 0; i < current; i++) retp[i] = tmpp[i];
  return ret;
}

template <class T>
std::vector<size_t> filter_gt_immed_helper(std::vector<T>& left,
                                           T right) {
  std::vector<size_t> tmp(left.size()); // large enough size
  size_t current = 0;
  T* leftp = &left[0];
  size_t* tmpp = &tmp[0];
  size_t size = left.size();
  for(size_t i = 0; i < size; i++) {
    if(leftp[i] > right) tmpp[current++] = i;
  }
  std::vector<size_t> ret(current);
  size_t* retp = &ret[0];
  for(size_t i = 0; i < current; i++) retp[i] = tmpp[i];
  return ret;
}

template <class T>
std::vector<size_t> filter_ge_helper(std::vector<T>& left,
                                     std::vector<T>& right) {
  std::vector<size_t> tmp(left.size()); // large enough size
  size_t current = 0;
  T* leftp = &left[0];
  T* rightp = &right[0];
  size_t* tmpp = &tmp[0];
  size_t size = left.size();
  for(size_t i = 0; i < size; i++) {
    if(leftp[i] >= rightp[i]) tmpp[current++] = i;
  }
  std::vector<size_t> ret(current);
  size_t* retp = &ret[0];
  for(size_t i = 0; i < current; i++) retp[i] = tmpp[i];
  return ret;
}

template <class T>
std::vector<size_t> filter_ge_immed_helper(std::vector<T>& left,
                                           T right) {
  std::vector<size_t> tmp(left.size()); // large enough size
  size_t current = 0;
  T* leftp = &left[0];
  size_t* tmpp = &tmp[0];
  size_t size = left.size();
  for(size_t i = 0; i < size; i++) {
    if(leftp[i] >= right) tmpp[current++] = i;
  }
  std::vector<size_t> ret(current);
  size_t* retp = &ret[0];
  for(size_t i = 0; i < current; i++) retp[i] = tmpp[i];
  return ret;
}

template <class T>
std::vector<T> extract_helper(std::vector<T>& val,
                              std::vector<size_t>& idx,
                              std::vector<size_t>& nulls,
                              std::vector<size_t>& retnulls) {
  size_t size = idx.size();
  std::vector<T> ret(size);
  T* valp = &val[0];
  T* retp = &ret[0];
  size_t* idxp = &idx[0];
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < size; i++) {
    retp[i] = valp[idxp[i]];
  }
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
    std::vector<size_t> retnulls(current);
    size_t* retnullsp = &retnulls[0];
    for(size_t i = 0; i < current; i++) {
      retnullsp[i] = rettmpp[i];
    }
  }
  return ret;
}

// when contain_nulls == false
template <class T>
std::vector<T> extract_helper2(std::vector<T>& val,
                               std::vector<size_t>& idx) {
  size_t size = idx.size();
  std::vector<T> ret(size);
  T* valp = &val[0];
  T* retp = &ret[0];
  size_t* idxp = &idx[0];
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < size; i++) {
    retp[i] = valp[idxp[i]];
  }
  return ret;
}

template <class T>
std::vector<std::vector<T>>
global_extract_helper(std::vector<T>& val,
                      std::vector<std::vector<size_t>>& exchanged_idx) {
  size_t size = exchanged_idx.size();
  std::vector<std::vector<T>> ret(size);
  T* valp = &val[0];
  for(size_t i = 0; i < size; i++) {
    size_t exchanged_size = exchanged_idx[i].size();
    ret[i].resize(exchanged_size);
    size_t* exchanged_idxp = &exchanged_idx[i][0];
    T* retp = &ret[i][0];
    for(size_t j = 0; j < exchanged_size; j++) {
      retp[j] = valp[exchanged_idxp[j]];
    }
  }
  return ret;
}

template <class T>
unique_hashtable<size_t, T>
create_hash_from_partition(std::vector<std::vector<size_t>>& part_idx,
                           std::vector<std::vector<T>>& exchanged_val) {
  size_t total = 0;
  size_t size = part_idx.size();
  if(exchanged_val.size() != size)
    throw std::runtime_error("size error in create_hash_from_partition");
  for(size_t i = 0; i < size; i++) total += part_idx[i].size();
  std::vector<size_t> flat_part_idx(total);
  std::vector<T> flat_exchanged_val(total);
  size_t* flat_part_idxp = &flat_part_idx[0];
  T* flat_exchanged_valp = &flat_exchanged_val[0];
  size_t current = 0;
  for(size_t i = 0; i < size; i++) {
    size_t part_size = part_idx[i].size();
    if(exchanged_val[i].size() != part_size)
      throw std::runtime_error("size error in create_hash_from_partition");
    size_t* part_idxp = &part_idx[i][0];
    T* exchanged_valp = &exchanged_val[i][0];
    for(size_t j = 0; j < part_size; j++) {
      flat_part_idxp[current] = part_idxp[j];
      flat_exchanged_valp[current++] = exchanged_valp[j];
    }
  }
  return unique_hashtable<size_t, T>(flat_part_idx, flat_exchanged_val);
}

template <class T>
std::vector<T> call_lookup(std::vector<size_t>& global_idx,
                           unique_hashtable<size_t, T>& hashtable) {
  return hashtable.lookup(global_idx);
}

std::vector<std::vector<size_t>> make_partition_idx(std::vector<size_t>& idx);

std::vector<std::vector<size_t>>
global_extract_null_helper(std::vector<size_t>& nulls,
                           std::vector<std::vector<size_t>>& exchanged_idx);

std::vector<size_t>
global_extract_null_helper2(unique_hashtable<size_t, int>& hashtable,
                            std::vector<size_t>& global_idx,
                            int& null_exists);

unique_hashtable<size_t, int>
create_null_hash_from_partition(std::vector<std::vector<size_t>>& part_idx,
                                int& null_exists);

template <class T>
std::vector<size_t> get_local_index_helper(std::vector<T>& val) {
  size_t size = val.size();
  std::vector<size_t> ret(size);
  size_t* retp = &ret[0];
  for(size_t i = 0; i < size; i++) retp[i] = i;
  return ret;
}

template <class T>
std::vector<T> extract_non_null(std::vector<T>& val,
                                std::vector<size_t>& idx,
                                std::vector<size_t>& nulls,
                                std::vector<size_t>& non_null_idx) {
  auto tmp = set_difference(idx, nulls);
  non_null_idx.swap(tmp);
  size_t non_null_size = non_null_idx.size();
  std::vector<T> non_null_val(non_null_size);
  T* non_null_valp = &non_null_val[0];
  T* valp = &val[0];
  size_t* non_null_idxp = &non_null_idx[0];
  for(size_t i = 0; i < non_null_size; i++) {
    non_null_valp[i] = valp[non_null_idxp[i]];
  }
  return non_null_val;
}

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
    for(size_t j = 0; j < left_vals[i].size(); j++) {
      left_valp[current] = left_valsp[j];
      left_idxp[current++] = left_idxsp[j];
    }
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
    for(size_t j = 0; j < right_vals[i].size(); j++) {
      right_valp[current] = right_valsp[j];
      right_idxp[current++] = right_idxsp[j];
    }
  }
  return outer_equi_join(left_val, left_idx, right_val, right_idx,
                         left_idx_out, right_idx_out);
}

template <class T>
void append_nulls_helper(std::vector<T>& val, std::vector<size_t>& to_append,
                         std::vector<size_t>& nulls) {
  size_t val_size = val.size();
  size_t to_append_size = to_append.size();
  std::vector<T> newval(val_size+to_append_size);
  for(size_t i = 0; i < val_size; i++) {
    newval[i] = val[i];
  }
  for(size_t i = 0; i < to_append_size; i++) {
    newval[val_size + i] = std::numeric_limits<T>::max();
  }
  val.swap(newval);
  nulls.resize(to_append_size);
  size_t* nullsp = &nulls[0];
  for(size_t i = 0; i < to_append_size; i++) nullsp[i] = val_size + i;
}

template <class T>
std::vector<size_t> calc_hash_base_helper(std::vector<T>& val) {
  size_t size = val.size();
  std::vector<size_t> ret(size);
  T* valp = &val[0];
  size_t* retp = &ret[0];
  for(size_t i = 0; i < size; i++) {
    retp[i] = static_cast<size_t>(valp[i]);
  }
  return ret;
}

template <class T>
struct calc_hash_base_helper2 {
  calc_hash_base_helper2(){}
  calc_hash_base_helper2(int shift) : shift(shift) {}
  void operator()(std::vector<T>& val, std::vector<size_t>& hash_base) {
    size_t size = val.size();
    std::vector<size_t> ret(size);
    T* valp = &val[0];
    size_t* hash_basep = &hash_base[0];
    for(size_t i = 0; i < size; i++) {
      hash_basep[i] = (hash_basep[i] << shift) + static_cast<size_t>(valp[i]);
    }
  }
  int shift;
  SERIALIZE(shift)
};

template <class T>
std::vector<size_t>
group_by_helper(std::vector<std::vector<T>>& split_val,
                std::vector<std::vector<size_t>>& split_idx,
                std::vector<size_t>& global_idx) {
  auto val = flatten(split_val);
  auto idx = flatten(split_idx);
  sort_pair(val, idx);
  global_idx.swap(idx);
  auto r =  set_separate(val);
  return set_separate(val);
}

template <class T>
void multi_group_by_sort_helper(std::vector<T>& val,
                                std::vector<size_t>& local_idx) {
  size_t size = val.size();
  std::vector<T> val2(size);
  T* valp = &val[0];
  T* val2p = &val2[0];
  size_t* local_idxp = &local_idx[0];
  for(size_t i = 0; i < size; i++) {
    val2p[i] = valp[local_idxp[i]];
  }
  sort_pair(val2, local_idx);
}

template <class T>
std::vector<size_t> 
multi_group_by_split_helper(std::vector<T>& val,
                            std::vector<size_t>& local_idx) {
  size_t size = val.size();
  std::vector<T> val2(size);
  T* valp = &val[0];
  T* val2p = &val2[0];
  size_t* local_idxp = &local_idx[0];
  for(size_t i = 0; i < size; i++) {
    val2p[i] = valp[local_idxp[i]];
  }
  return set_separate(val2);
}


std::vector<size_t> count_helper(std::vector<size_t>& idx_split,
                                 std::vector<size_t>& nulls);

template <class T>
std::vector<T> sum_helper(std::vector<T>& val,
                          std::vector<size_t>& idx_split,
                          std::vector<size_t>& nulls,
                          std::vector<size_t>& newnulls) {
  size_t splitsize = idx_split.size();
  size_t nullssize = nulls.size();
  std::vector<T> ret(splitsize-1);
  T* valp = &val[0];
  size_t* idx_splitp = &idx_split[0];
  size_t* nullsp = &nulls[0];
  T* retp = &ret[0];
  for(size_t i = 0; i < nullssize; i++) {
    valp[nullsp[i]] = 0;
  }
#if defined(_SX) || defined(__ve__)
  size_t valsize = val.size();
  if(valsize / splitsize > GROUPBY_VLEN * 2) {
    size_t end = 0;
    size_t start = 0;
    for(size_t i = 0; i < splitsize-1; i++) {
      start = end;
      end = idx_splitp[i+1];
      T total = 0;
      for(size_t j = start; j < end; j++) {
        total += valp[j];
      }
      retp[i] = total;
    }
  } else { // for improving vectorization
    size_t blocksize = (splitsize - 1) / GROUPBY_VLEN;
    size_t remain = (splitsize - 1) % GROUPBY_VLEN;
    size_t current_idx_splitp[GROUPBY_VLEN];
    size_t idx_splitp_stop[GROUPBY_VLEN];
    T* current_retp;
    int valid[GROUPBY_VLEN];
    for(size_t i = 0; i < blocksize + 1; i++) {
      current_retp = retp + i * GROUPBY_VLEN;
      size_t len = i < blocksize ? GROUPBY_VLEN : remain;
      for(size_t j = 0; j < len; j++) {
        current_idx_splitp[j] = idx_splitp[i * GROUPBY_VLEN + j];
        idx_splitp_stop[j] = idx_splitp[i * GROUPBY_VLEN + j + 1];
        valid[j] = true;
      }
      while(1) {
#pragma cdir nodep
#pragma _NEC ivdep
        for(size_t j = 0; j < len; j++) {
          if(valid[j]) current_retp[j] += valp[current_idx_splitp[j]++];
          if(current_idx_splitp[j] == idx_splitp_stop[j]) valid[j] = false;
        }
        int any_valid = false;
        for(size_t j = 0; j < len; j++) {
          if(valid[j] == true) any_valid = true;
        }
        if(any_valid == false) break;
      }
    }
  }
#else  
  size_t end = 0;
  size_t start = 0;
  for(size_t i = 0; i < splitsize-1; i++) {
    start = end;
    end = idx_splitp[i+1];
    T total = 0;
    for(size_t j = start; j < end; j++) {
      total += valp[j];
    }
    retp[i] = total;
  }
#endif
  if(nullssize != 0) {
    auto count = count_helper(idx_split, nulls);
    size_t zeros = 0;
    size_t countsize = count.size();
    size_t* countp = &count[0];
    for(size_t i = 0; i < countsize; i++) {
      if(countp[i] == 0) zeros++;
    }
    newnulls.resize(zeros);
    size_t* newnullsp = &newnulls[0];
    size_t current = 0;
    for(size_t i = 0; i < countsize; i++) {
      if(countp[i] == 0) newnullsp[current++] = i;
    }
    for(size_t i = 0; i < zeros; i++) {
      retp[newnullsp[i]] = std::numeric_limits<T>::max();
    }
  }
  return ret;
}

template <class T>
std::vector<double> avg_helper(std::vector<T>& val,
                               std::vector<size_t>& idx_split,
                               std::vector<size_t>& nulls,
                               std::vector<size_t>& newnulls) {
  size_t splitsize = idx_split.size();
  size_t nullssize = nulls.size();
  std::vector<double> ret(splitsize-1);
  T* valp = &val[0];
  size_t* idx_splitp = &idx_split[0];
  size_t* nullsp = &nulls[0];
  double* retp = &ret[0];
  for(size_t i = 0; i < nullssize; i++) {
    valp[nullsp[i]] = 0;
  }
#if defined(_SX) || defined(__ve__)
  size_t valsize = val.size();
  if(valsize / splitsize > GROUPBY_VLEN * 2) {
    size_t end = 0;
    size_t start = 0;
    for(size_t i = 0; i < splitsize-1; i++) {
      start = end;
      end = idx_splitp[i+1];
      T total = 0;
      for(size_t j = start; j < end; j++) {
        total += valp[j];
      }
      retp[i] = total;
    }
  } else { // for improving vectorization
    size_t blocksize = (splitsize - 1) / GROUPBY_VLEN;
    size_t remain = (splitsize - 1) % GROUPBY_VLEN;
    size_t current_idx_splitp[GROUPBY_VLEN];
    size_t idx_splitp_stop[GROUPBY_VLEN];
    int valid[GROUPBY_VLEN];
    for(size_t i = 0; i < blocksize + 1; i++) {
      double* current_retp = retp + i * GROUPBY_VLEN;
      size_t len = i < blocksize ? GROUPBY_VLEN : remain;
      T current_work[len];
      for(size_t j = 0; j < len; j++) {
        current_idx_splitp[j] = idx_splitp[i * GROUPBY_VLEN + j];
        idx_splitp_stop[j] = idx_splitp[i * GROUPBY_VLEN + j + 1];
        valid[j] = true;
        current_work[j] = 0;
      }
      while(1) {
#pragma cdir nodep
#pragma _NEC ivdep
        for(size_t j = 0; j < len; j++) {
          if(valid[j]) current_work[j] += valp[current_idx_splitp[j]++];
          if(current_idx_splitp[j] == idx_splitp_stop[j]) valid[j] = false;
        }
        int any_valid = false;
        for(size_t j = 0; j < len; j++) {
          if(valid[j] == true) any_valid = true;
        }
        if(any_valid == false) {
          for(size_t j = 0; j < len; j++) {
            current_retp[j] = static_cast<double>(current_work[j]);
          }
          break;
        }
      }
    }
  }
#else  
  size_t end = 0;
  size_t start = 0;
  for(size_t i = 0; i < splitsize-1; i++) {
    start = end;
    end = idx_splitp[i+1];
    T total = 0;
    for(size_t j = start; j < end; j++) {
      total += valp[j];
    }
    retp[i] = static_cast<double>(total);
  }
#endif

  auto count = count_helper(idx_split, nulls);
  size_t zeros = 0;
  size_t countsize = count.size();
  size_t* countp = &count[0];
  for(size_t i = 0; i < splitsize - 1; i++) {
    if(countp[i] == 0) zeros++;
    else {
      retp[i] /= countp[i];
    }
  }
  newnulls.resize(zeros);
  size_t* newnullsp = &newnulls[0];
  size_t current = 0;
  for(size_t i = 0; i < countsize; i++) {
    if(countp[i] == 0) newnullsp[current++] = i;
  }
  for(size_t i = 0; i < zeros; i++) {
    retp[newnullsp[i]] = std::numeric_limits<T>::max();
  }

  return ret;
}

template <class T>
std::vector<T> max_helper(std::vector<T>& val,
                          std::vector<size_t>& idx_split,
                          std::vector<size_t>& nulls,
                          std::vector<size_t>& newnulls) {
  size_t splitsize = idx_split.size();
  size_t nullssize = nulls.size();
  std::vector<T> ret(splitsize-1);
  T* valp = &val[0];
  size_t* idx_splitp = &idx_split[0];
  size_t* nullsp = &nulls[0];
  T* retp = &ret[0];
  for(size_t i = 0; i < nullssize; i++) {
    valp[nullsp[i]] = std::numeric_limits<T>::min();
  }
#if defined(_SX) || defined(__ve__)
  size_t valsize = val.size();
  if(valsize / splitsize > GROUPBY_VLEN * 2) {
    size_t end = 0;
    size_t start = 0;
    for(size_t i = 0; i < splitsize-1; i++) {
      start = end;
      end = idx_splitp[i+1];
      T total = 0;
      for(size_t j = start; j < end; j++) {
        total += valp[j];
      }
      retp[i] = total;
    }
  } else { // for improving vectorization
    size_t blocksize = (splitsize - 1) / GROUPBY_VLEN;
    size_t remain = (splitsize - 1) % GROUPBY_VLEN;
    size_t current_idx_splitp[GROUPBY_VLEN];
    size_t idx_splitp_stop[GROUPBY_VLEN];
    T* current_retp;
    int valid[GROUPBY_VLEN];
    for(size_t i = 0; i < blocksize + 1; i++) {
      current_retp = retp + i * GROUPBY_VLEN;
      size_t len = i < blocksize ? GROUPBY_VLEN : remain;
      for(size_t j = 0; j < len; j++) {
        current_idx_splitp[j] = idx_splitp[i * GROUPBY_VLEN + j];
        idx_splitp_stop[j] = idx_splitp[i * GROUPBY_VLEN + j + 1];
        current_retp[j] = std::numeric_limits<T>::min();
        valid[j] = true;
      }
      while(1) {
#pragma cdir nodep
#pragma _NEC ivdep
        for(size_t j = 0; j < len; j++) {
          if(valid[j]) {
            T valtmp = valp[current_idx_splitp[j]++];
            if(current_retp[j] < valtmp) current_retp[j] = valtmp;
          }
          if(current_idx_splitp[j] == idx_splitp_stop[j]) valid[j] = false;
        }
        int any_valid = false;
        for(size_t j = 0; j < len; j++) {
          if(valid[j] == true) any_valid = true;
        }
        if(any_valid == false) break;
      }
    }
  }
#else  
  size_t end = 0;
  size_t start = 0;
  for(size_t i = 0; i < splitsize-1; i++) {
    start = end;
    end = idx_splitp[i+1];
    T current_max = std::numeric_limits<T>::min();
    for(size_t j = start; j < end; j++) {
      if(current_max < valp[j]) current_max = valp[j];
    }
    retp[i] = current_max;
  }
#endif
  if(nullssize != 0) {
    auto count = count_helper(idx_split, nulls);
    size_t zeros = 0;
    size_t countsize = count.size();
    size_t* countp = &count[0];
    for(size_t i = 0; i < countsize; i++) {
      if(countp[i] == 0) zeros++;
    }
    newnulls.resize(zeros);
    size_t* newnullsp = &newnulls[0];
    size_t current = 0;
    for(size_t i = 0; i < countsize; i++) {
      if(countp[i] == 0) newnullsp[current++] = i;
    }
    for(size_t i = 0; i < zeros; i++) {
      retp[newnullsp[i]] = std::numeric_limits<T>::max();
    }
  }
  return ret;
}

template <class T>
std::vector<T> min_helper(std::vector<T>& val,
                          std::vector<size_t>& idx_split,
                          std::vector<size_t>& nulls,
                          std::vector<size_t>& newnulls) {
  size_t splitsize = idx_split.size();
  size_t nullssize = nulls.size();
  std::vector<T> ret(splitsize-1);
  T* valp = &val[0];
  size_t* idx_splitp = &idx_split[0];
  T* retp = &ret[0];
  // null part of val is already max; do nothing
#if defined(_SX) || defined(__ve__)
  size_t valsize = val.size();
  if(valsize / splitsize > GROUPBY_VLEN * 2) {
    size_t end = 0;
    size_t start = 0;
    for(size_t i = 0; i < splitsize-1; i++) {
      start = end;
      end = idx_splitp[i+1];
      T total = 0;
      for(size_t j = start; j < end; j++) {
        total += valp[j];
      }
      retp[i] = total;
    }
  } else { // for improving vectorization
    size_t blocksize = (splitsize - 1) / GROUPBY_VLEN;
    size_t remain = (splitsize - 1) % GROUPBY_VLEN;
    size_t current_idx_splitp[GROUPBY_VLEN];
    size_t idx_splitp_stop[GROUPBY_VLEN];
    T* current_retp;
    int valid[GROUPBY_VLEN];
    for(size_t i = 0; i < blocksize + 1; i++) {
      current_retp = retp + i * GROUPBY_VLEN;
      size_t len = i < blocksize ? GROUPBY_VLEN : remain;
      for(size_t j = 0; j < len; j++) {
        current_idx_splitp[j] = idx_splitp[i * GROUPBY_VLEN + j];
        idx_splitp_stop[j] = idx_splitp[i * GROUPBY_VLEN + j + 1];
        current_retp[j] = std::numeric_limits<T>::max();
        valid[j] = true;
      }
      while(1) {
#pragma cdir nodep
#pragma _NEC ivdep
        for(size_t j = 0; j < len; j++) {
          if(valid[j]) {
            T valtmp = valp[current_idx_splitp[j]++];
            if(current_retp[j] > valtmp) current_retp[j] = valtmp;
          }
          if(current_idx_splitp[j] == idx_splitp_stop[j]) valid[j] = false;
        }
        int any_valid = false;
        for(size_t j = 0; j < len; j++) {
          if(valid[j] == true) any_valid = true;
        }
        if(any_valid == false) break;
      }
    }
  }
#else  
  size_t end = 0;
  size_t start = 0;
  for(size_t i = 0; i < splitsize-1; i++) {
    start = end;
    end = idx_splitp[i+1];
    T current_min = std::numeric_limits<T>::max();
    for(size_t j = start; j < end; j++) {
      if(current_min > valp[j]) current_min = valp[j];
    }
    retp[i] = current_min;
  }
#endif
  if(nullssize != 0) {
    auto count = count_helper(idx_split, nulls);
    size_t zeros = 0;
    size_t countsize = count.size();
    size_t* countp = &count[0];
    for(size_t i = 0; i < countsize; i++) {
      if(countp[i] == 0) zeros++;
    }
    newnulls.resize(zeros);
    size_t* newnullsp = &newnulls[0];
    size_t current = 0;
    for(size_t i = 0; i < countsize; i++) {
      if(countp[i] == 0) newnullsp[current++] = i;
    }
    for(size_t i = 0; i < zeros; i++) {
      retp[newnullsp[i]] = std::numeric_limits<T>::max();
    }
  }
  return ret;
}

template <class T>
T sum_helper2(std::vector<T>& val,
              std::vector<size_t>& nulls) {
  size_t valsize = val.size();
  size_t nullssize = nulls.size();
  T* valp = &val[0];
  size_t* nullsp = &nulls[0];
  for(size_t i = 0; i < nullssize; i++) {
    valp[nullsp[i]] = 0;
  }
  T total = 0;
  for(size_t i = 0; i < valsize; i++) {
    total += valp[i];
  }
  return total;
}

template <class T>
T max_helper2(std::vector<T>& val,
              std::vector<size_t>& nulls) {
  size_t valsize = val.size();
  size_t nullssize = nulls.size();
  T* valp = &val[0];
  size_t* nullsp = &nulls[0];
  for(size_t i = 0; i < nullssize; i++) {
    valp[nullsp[i]] = std::numeric_limits<T>::min();
  }
  T current_max = std::numeric_limits<T>::min();
  for(size_t i = 0; i < valsize; i++) {
    if(current_max < valp[i]) current_max = valp[i];
  }
  return current_max;
}

template <class T>
T min_helper2(std::vector<T>& val,
              std::vector<size_t>& nulls/* not used because already max */) {
  size_t valsize = val.size();
  T* valp = &val[0];
  T current_min = std::numeric_limits<T>::max();
  for(size_t i = 0; i < valsize; i++) {
    if(current_min > valp[i]) current_min = valp[i];
  }
  return current_min;
}

template <class T>
size_t typed_dfcolumn<T>::size() {
  return val.template viewas_dvector<T>().size();
}

template <class T>
dvector<std::string> typed_dfcolumn<T>::as_string() {
  return val.map(as_string_helper<T>).template moveto_dvector<std::string>();
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_eq(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<T>>(right);
  if(!right2) throw std::runtime_error("filter_eq: column types are different");
  auto filtered_idx = val.map(filter_eq_helper<T>, right2->val);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_eq_immed(const T& right) {
  auto filtered_idx = val.map(filter_eq_immed_helper<T>, broadcast(right));
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_neq(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<T>>(right);
  if(!right2) 
    throw std::runtime_error("filter_neq: column types are different");
  auto filtered_idx = val.map(filter_neq_helper<T>, right2->val);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_neq_immed(const T& right) {
  auto filtered_idx = val.map(filter_neq_immed_helper<T>, broadcast(right));
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_lt(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<T>>(right);
  if(!right2) throw std::runtime_error("filter_lt: column types are different");
  auto filtered_idx = val.map(filter_lt_helper<T>, right2->val);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_lt_immed(const T& right) {
  auto filtered_idx = val.map(filter_lt_immed_helper<T>, broadcast(right));
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_le(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<T>>(right);
  if(!right2) throw std::runtime_error("filter_le: column types are different");
  auto filtered_idx = val.map(filter_le_helper<T>, right2->val);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_le_immed(const T& right) {
  auto filtered_idx = val.map(filter_le_immed_helper<T>, broadcast(right));
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_gt(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<T>>(right);
  if(!right2) throw std::runtime_error("filter_gt: column types are different");
  auto filtered_idx = val.map(filter_gt_helper<T>, right2->val);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_gt_immed(const T& right) {
  auto filtered_idx = val.map(filter_gt_immed_helper<T>, broadcast(right));
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_ge(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<T>>(right);
  if(!right2) throw std::runtime_error("filter_ge: column types are different");
  auto filtered_idx = val.map(filter_ge_helper<T>, right2->val);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_ge_immed(const T& right) {
  auto filtered_idx = val.map(filter_ge_immed_helper<T>, broadcast(right));
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_is_null() {return nulls;}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_is_not_null() {
  auto local_idx = get_local_index();
  if(contain_nulls)
    return local_idx.map(set_difference<size_t>, nulls);
  else return local_idx;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::extract(node_local<std::vector<size_t>>& idx) {
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  auto retnulls = make_node_local_allocate<std::vector<size_t>>();
  if(contain_nulls) {
    ret->val = val.map(extract_helper<T>, idx, nulls, retnulls);
    ret->nulls = std::move(retnulls);
    ret->contain_nulls_check();
  } else {
    ret->val = val.map(extract_helper2<T>, idx);
    ret->nulls = std::move(retnulls);
  }
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::global_extract
(node_local<std::vector<size_t>>& global_idx,
 node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
  time_spent t(DEBUG);
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  auto exdata = val.map(global_extract_helper<T>, exchanged_idx);
  t.show("global_exract_helper: ");
  auto exchanged_back = alltoall_exchange(exdata);
  t.show("alltoall_exchange: ");
  auto hashes = partitioned_idx.map(create_hash_from_partition<T>,
                                    exchanged_back);
  t.show("create_hash_from_partition: ");
  ret->val = global_idx.map(call_lookup<T>, hashes);
  t.show("call_lookup: ");
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

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::get_local_index() {
  return val.map(get_local_index_helper<T>);
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
  if(!right2) 
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
  if(!right2)
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
  if(!right2)
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
std::tuple<node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>>
typed_dfcolumn<T>::outer_bcast_join_eq
  (std::shared_ptr<dfcolumn>& right,
   // might be filtered index
   node_local<std::vector<size_t>>& left_full_local_idx, 
   node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<T>>(right);
  if(!right2)
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
  if(!right2)
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
void
typed_dfcolumn<T>::append_nulls(node_local<std::vector<size_t>>& to_append) {
  val.mapv(append_nulls_helper<T>, to_append, nulls);
  contain_nulls = true;
}

template <class T>
node_local<std::vector<size_t>> 
typed_dfcolumn<T>::calc_hash_base() {
  return val.map(calc_hash_base_helper<T>);
}

template <class T>
void 
typed_dfcolumn<T>::calc_hash_base(node_local<std::vector<size_t>>& hash_base,
                                  int shift) {
    val.mapv(calc_hash_base_helper2<T>(shift), hash_base);
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::group_by(node_local<std::vector<size_t>>& global_idx) {
  auto split_val = make_node_local_allocate<std::vector<std::vector<T>>>();
  auto split_idx =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  val.mapv(split_by_hash<T>, split_val, global_idx, split_idx);
  auto exchanged_idx = alltoall_exchange(split_idx);
  auto exchanged_val = alltoall_exchange(split_val);
  return exchanged_val.map(group_by_helper<T>, exchanged_idx, global_idx);
}

template <class T>
void
typed_dfcolumn<T>::multi_group_by_sort
(node_local<std::vector<size_t>>& local_idx) {
  val.mapv(multi_group_by_sort_helper<T>, local_idx);
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::multi_group_by_split
(node_local<std::vector<size_t>>& local_idx) {
    return val.map(multi_group_by_split_helper<T>, local_idx);
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::sum
(node_local<std::vector<size_t>>& grouped_idx,
 node_local<std::vector<size_t>>& idx_split,
 node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  auto newcol = std::dynamic_pointer_cast<typed_dfcolumn<T>>
    (global_extract(grouped_idx, partitioned_idx, exchanged_idx));
  if(!newcol)
    throw std::runtime_error("sum: internal error (dynamic_pointer_cast)");
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  ret->val = newcol->val.map(sum_helper<T>, idx_split, newcol->nulls,
                             ret->nulls);
  if(contain_nulls) ret->contain_nulls_check();
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::count
(node_local<std::vector<size_t>>& grouped_idx,
 node_local<std::vector<size_t>>& idx_split,
 node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
  auto newcol = std::dynamic_pointer_cast<typed_dfcolumn<T>>
    (global_extract(grouped_idx, partitioned_idx, exchanged_idx));
  if(!newcol)
    throw std::runtime_error("count: internal error (dynamic_pointer_cast)");
  auto retval = idx_split.map(count_helper, newcol->get_nulls());
  auto retnulls = make_node_local_allocate<std::vector<size_t>>();
  auto ret = std::make_shared<typed_dfcolumn<size_t>>();
  ret->val = std::move(retval);
  ret->nulls = std::move(retnulls);
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::avg
(node_local<std::vector<size_t>>& grouped_idx,
 node_local<std::vector<size_t>>& idx_split,
 node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
  auto newcol = std::dynamic_pointer_cast<typed_dfcolumn<T>>
    (global_extract(grouped_idx, partitioned_idx, exchanged_idx));
  if(!newcol)
    throw std::runtime_error("avg: internal error (dynamic_pointer_cast)");
  auto retnulls = make_node_local_allocate<std::vector<size_t>>();
  auto retval = newcol->val.map(avg_helper<T>, idx_split, newcol->nulls,
                                retnulls);
  auto ret = std::make_shared<typed_dfcolumn<double>>();
  ret->val = std::move(retval);
  ret->nulls = std::move(retnulls);
  if(contain_nulls) ret->contain_nulls_check();
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::max
(node_local<std::vector<size_t>>& grouped_idx,
 node_local<std::vector<size_t>>& idx_split,
 node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  auto newcol = std::dynamic_pointer_cast<typed_dfcolumn<T>>
    (global_extract(grouped_idx, partitioned_idx, exchanged_idx));
  if(!newcol)
    throw std::runtime_error("max: internal error (dynamic_pointer_cast)");
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  ret->val = newcol->val.map(max_helper<T>, idx_split, newcol->nulls,
                             ret->nulls);
  if(contain_nulls) ret->contain_nulls_check();
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::min
(node_local<std::vector<size_t>>& grouped_idx,
 node_local<std::vector<size_t>>& idx_split,
 node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  auto newcol = std::dynamic_pointer_cast<typed_dfcolumn<T>>
    (global_extract(grouped_idx, partitioned_idx, exchanged_idx));
  if(!newcol)
    throw std::runtime_error("min: internal error (dynamic_pointer_cast)");
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  ret->val = newcol->val.map(min_helper<T>, idx_split, newcol->nulls,
                             ret->nulls);
  if(contain_nulls) ret->contain_nulls_check();
  return ret;
}
 
template <class T>
size_t typed_dfcolumn<T>::count() {
  size_t size = val.template viewas_dvector<T>().size();
  if(contain_nulls) {
    size_t nullsize = nulls.template viewas_dvector<size_t>().size();
    return size - nullsize;
  } else return size;
}

template <class T>
T typed_dfcolumn<T>::sum() {
  auto sums = val.map(sum_helper2<T>, nulls).gather();
  T* sumsp = &sums[0];
  size_t size = sums.size();
  T total = 0;
  for(size_t i = 0; i < size; i++) {
    total += sumsp[i];
  }
  return total;
}

template <class T>
double typed_dfcolumn<T>::avg() {
  size_t ct = count();
  double total = static_cast<double>(sum());
  return total/static_cast<double>(ct);
}

template <class T>
T typed_dfcolumn<T>::max() {
  auto maxs = val.map(max_helper2<T>, nulls).gather();
  T* maxsp = &maxs[0];
  size_t size = maxs.size();
  T current_max = std::numeric_limits<T>::min();
  for(size_t i = 0; i < size; i++) {
    if(current_max < maxsp[i]) current_max = maxsp[i];
  }
  return current_max;
}

template <class T>
T typed_dfcolumn<T>::min() {
  auto mins = val.map(min_helper2<T>, nulls).gather();
  T* minsp = &mins[0];
  size_t size = mins.size();
  T current_min = std::numeric_limits<T>::max();
  for(size_t i = 0; i < size; i++) {
    if(current_min > minsp[i]) current_min = minsp[i];
  }
  return current_min;
}

template <class T>
void typed_dfcolumn<T>::debug_print() {
  std::cout << "values: ";
  for(auto& i: val.gather()) {
    for(auto j: i) std::cout << j << " ";
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
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::head(size_t limit) {
  return std::make_shared<typed_dfcolumn<T>>
    (this->as_dvector<T>().head(limit));
}

template <class T>
void typed_dfcolumn<T>::save(const std::string& file) {
  val.template viewas_dvector<T>().savebinary(file);
  auto dv_nulls = nulls.template viewas_dvector<size_t>();
  auto sizes = val.template viewas_dvector<T>().sizes();
  std::vector<size_t> pxsizes(sizes.size());
  for(size_t i = 1; i < pxsizes.size(); i++) {
    pxsizes[i] += pxsizes[i-1] + sizes[i-1];
  }
  auto nl_sizes = make_node_local_scatter(pxsizes);
  dv_nulls.map<size_t>(shift_local_index(), nl_sizes).
    savebinary(file+"_nulls");
}

template <class T>
std::vector<T> dfcolumn_sort_local(std::vector<T>& val,
                                   std::vector<size_t>& idx) {
  size_t size = val.size();
  idx.resize(size);
  size_t selfid = static_cast<size_t>(get_selfid());
  size_t nodeinfo = selfid << DFNODESHIFT;
  size_t* idxp = &idx[0];
  for(size_t i = 0; i < size; i++) {
    idxp[i] = i + nodeinfo;
  }
  sort_pair(val, idx);
  int nodesize = get_nodesize();
  size_t each = ceil_div(size, size_t(nodesize));
  size_t part_size = ceil_div(size, each);
  std::vector<T> part(part_size);
  T* partp = &part[0];
  T* valp = &val[0];
  for(size_t i = 0; i < part_size; i++) {
    partp[i] = valp[each * i];
  }
  return part;
}

template <class T>
void dfcolumn_sort_partition(std::vector<T>& val,
                             std::vector<size_t>& idx,
                             std::vector<std::vector<T>>& part_val,
                             std::vector<std::vector<size_t>>& part_idx,
                             std::vector<T>& part) {
  int nodesize = get_nodesize();
  part_val.resize(nodesize);
  part_idx.resize(nodesize);
  size_t size = val.size();
  auto last_it = val.begin();
  for(size_t i = 0; i < nodesize - 1; i++) {
    auto it = std::lower_bound(val.begin(), val.end(), part[i]);
    size_t start = last_it - val.begin();
    size_t end = it - val.begin();
    size_t part_size = end - start;
    part_val[i].resize(part_size);
    part_idx[i].resize(part_size);
    T* part_valp = &part_val[i][0];
    size_t* part_idxp = &part_idx[i][0];
    for(size_t i = 0; i < part_size; i++) {
      part_valp[i] = val[start+i];
      part_idxp[i] = idx[start+i];
    }
    last_it = it;
  }
  size_t start = last_it - val.begin();
  size_t end = size;
  size_t part_size = end - start;
  part_val[nodesize-1].resize(part_size);
  part_idx[nodesize-1].resize(part_size);
  T* part_valp = &part_val[nodesize-1][0];
  size_t* part_idxp = &part_idx[nodesize-1][0];
  for(size_t i = 0; i < part_size; i++) {
    part_valp[i] = val[start+i];
    part_idxp[i] = idx[start+i];
  }
}

// based on distributed sample sort
// http://www3.cs.stonybrook.edu/~rezaul/Spring-2012/CSE613/
// CSE613-lecture-16.pdf
// here, pivot includes fist element to support already sorted entries
template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::sort(node_local<std::vector<size_t>>& res_idx) {
  auto to_sort = val; // need to copy because of implace sort
  auto idx = make_node_local_allocate<std::vector<size_t>>();
  auto part_cand =
    to_sort.map(dfcolumn_sort_local<T>, idx).reduce(set_merge<T>);
  size_t nodesize = get_nodesize();
  std::vector<T> part(nodesize-1);
  size_t part_cand_size = part_cand.size();
  T* partp = &part[0];
  T* part_candp = &part_cand[0];
  for(size_t i = 0; i < nodesize - 1; i++) {
    auto idx = std::min((i+1) * nodesize, part_cand_size - 1);
    partp[i] = part_candp[idx];
  }
  auto part_val = make_node_local_allocate<std::vector<std::vector<T>>>();
  auto part_idx = make_node_local_allocate<std::vector<std::vector<size_t>>>();
  to_sort.mapv(dfcolumn_sort_partition<T>, idx, part_val, part_idx,
               broadcast(part));
  auto exchanged_val = alltoall_exchange(part_val);
  auto exchanged_idx = alltoall_exchange(part_idx);
  auto res_val = make_node_local_allocate<std::vector<T>>();
  res_idx = make_node_local_allocate<std::vector<size_t>>();
  exchanged_val.mapv(set_multimerge_pair<T,size_t>, exchanged_idx,
                     res_val, res_idx);
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->val = std::move(res_val);
  if(contain_nulls) {
    auto exnulls = nulls.map(global_extract_null_helper, exchanged_idx);
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

template <class T>
std::vector<T> dfcolumn_sort_local_desc(std::vector<T>& val,
                                        std::vector<size_t>& idx) {
  size_t size = val.size();
  idx.resize(size);
  size_t selfid = static_cast<size_t>(get_selfid());
  size_t nodeinfo = selfid << DFNODESHIFT;
  size_t* idxp = &idx[0];
  for(size_t i = 0; i < size; i++) {
    idxp[i] = i + nodeinfo;
  }
  sort_pair_desc(val, idx);
  int nodesize = get_nodesize();
  size_t each = ceil_div(size, size_t(nodesize));
  size_t part_size = ceil_div(size, each);
  std::vector<T> part(part_size);
  T* partp = &part[0];
  T* valp = &val[0];
  for(size_t i = 0; i < part_size; i++) {
    partp[i] = valp[each * i];
  }
  return part;
}

template <class T>
void dfcolumn_sort_partition_desc(std::vector<T>& val,
                                  std::vector<size_t>& idx,
                                  std::vector<std::vector<T>>& part_val,
                                  std::vector<std::vector<size_t>>& part_idx,
                                  std::vector<T>& part) {
  int nodesize = get_nodesize();
  part_val.resize(nodesize);
  part_idx.resize(nodesize);
  size_t size = val.size();
  auto last_it = val.begin();
  for(size_t i = 0; i < nodesize - 1; i++) {
    auto it = std::lower_bound(val.begin(), val.end(), part[i],
                               std::greater<T>());
    size_t start = last_it - val.begin();
    size_t end = it - val.begin();
    size_t part_size = end - start;
    part_val[i].resize(part_size);
    part_idx[i].resize(part_size);
    T* part_valp = &part_val[i][0];
    size_t* part_idxp = &part_idx[i][0];
    for(size_t i = 0; i < part_size; i++) {
      part_valp[i] = val[start+i];
      part_idxp[i] = idx[start+i];
    }
    last_it = it;
  }
  size_t start = last_it - val.begin();
  size_t end = size;
  size_t part_size = end - start;
  part_val[nodesize-1].resize(part_size);
  part_idx[nodesize-1].resize(part_size);
  T* part_valp = &part_val[nodesize-1][0];
  size_t* part_idxp = &part_idx[nodesize-1][0];
  for(size_t i = 0; i < part_size; i++) {
    part_valp[i] = val[start+i];
    part_idxp[i] = idx[start+i];
  }
}


template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::sort_desc(node_local<std::vector<size_t>>& res_idx) {
  auto to_sort = val; // need to copy because of implace sort
  auto idx = make_node_local_allocate<std::vector<size_t>>();
  auto part_cand =
    to_sort.map(dfcolumn_sort_local_desc<T>, idx).reduce(set_merge_desc<T>);
  size_t nodesize = get_nodesize();
  std::vector<T> part(nodesize-1);
  size_t part_cand_size = part_cand.size();
  T* partp = &part[0];
  T* part_candp = &part_cand[0];
  for(size_t i = 0; i < nodesize - 1; i++) {
    auto idx = std::min((i+1) * nodesize, part_cand_size - 1);
    partp[i] = part_candp[idx];
  }
  auto part_val = make_node_local_allocate<std::vector<std::vector<T>>>();
  auto part_idx = make_node_local_allocate<std::vector<std::vector<size_t>>>();
  to_sort.mapv(dfcolumn_sort_partition_desc<T>, idx, part_val, part_idx,
               broadcast(part));
  auto exchanged_val = alltoall_exchange(part_val);
  auto exchanged_idx = alltoall_exchange(part_idx);
  auto res_val = make_node_local_allocate<std::vector<T>>();
  res_idx = make_node_local_allocate<std::vector<size_t>>();
  exchanged_val.mapv(set_multimerge_pair_desc<T,size_t>, exchanged_idx,
                     res_val, res_idx);
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->val = std::move(res_val);
  if(contain_nulls) {
    auto exnulls = nulls.map(global_extract_null_helper, exchanged_idx);
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

template <class T>
std::vector<T> dfcolumn_sort_with_idx_local(std::vector<T>& val,
                                            std::vector<size_t>& idx) {
  size_t size = val.size();
  sort_pair(val, idx);
  int nodesize = get_nodesize();
  size_t each = ceil_div(size, size_t(nodesize));
  size_t part_size = ceil_div(size, each);
  std::vector<T> part(part_size);
  T* partp = &part[0];
  T* valp = &val[0];
  for(size_t i = 0; i < part_size; i++) {
    partp[i] = valp[each * i];
  }
  return part;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::sort_with_idx(node_local<std::vector<size_t>>& idx,
                                 node_local<std::vector<size_t>>& res_idx) {
  auto to_sort = val; // need to copy because of implace sort
  auto part_cand =
    to_sort.map(dfcolumn_sort_with_idx_local<T>, idx).reduce(set_merge<T>);
  size_t nodesize = get_nodesize();
  std::vector<T> part(nodesize-1);
  size_t part_cand_size = part_cand.size();
  T* partp = &part[0];
  T* part_candp = &part_cand[0];
  for(size_t i = 0; i < nodesize - 1; i++) {
    auto idx = std::min((i+1) * nodesize, part_cand_size - 1);
    partp[i] = part_candp[idx];
  }
  auto part_val = make_node_local_allocate<std::vector<std::vector<T>>>();
  auto part_idx = make_node_local_allocate<std::vector<std::vector<size_t>>>();
  to_sort.mapv(dfcolumn_sort_partition<T>, idx, part_val, part_idx,
               broadcast(part));
  auto exchanged_val = alltoall_exchange(part_val);
  auto exchanged_idx = alltoall_exchange(part_idx);
  auto res_val = make_node_local_allocate<std::vector<T>>();
  res_idx = make_node_local_allocate<std::vector<size_t>>();
  exchanged_val.mapv(set_multimerge_pair<T,size_t>, exchanged_idx,
                     res_val, res_idx);
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->val = std::move(res_val);
  if(contain_nulls) {
    auto exnulls = nulls.map(global_extract_null_helper, exchanged_idx);
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

template <class T>
std::vector<T> dfcolumn_sort_with_idx_local_desc(std::vector<T>& val,
                                                 std::vector<size_t>& idx) {
  size_t size = val.size();
  sort_pair_desc(val, idx);
  int nodesize = get_nodesize();
  size_t each = ceil_div(size, size_t(nodesize));
  size_t part_size = ceil_div(size, each);
  std::vector<T> part(part_size);
  T* partp = &part[0];
  T* valp = &val[0];
  for(size_t i = 0; i < part_size; i++) {
    partp[i] = valp[each * i];
  }
  return part;
}

template <class T>
std::shared_ptr<dfcolumn> typed_dfcolumn<T>::
sort_with_idx_desc(node_local<std::vector<size_t>>& idx,
                   node_local<std::vector<size_t>>& res_idx) {
  auto to_sort = val; // need to copy because of implace sort
  auto part_cand = to_sort.map(dfcolumn_sort_with_idx_local_desc<T>, idx).
    reduce(set_merge_desc<T>);
  size_t nodesize = get_nodesize();
  std::vector<T> part(nodesize-1);
  size_t part_cand_size = part_cand.size();
  T* partp = &part[0];
  T* part_candp = &part_cand[0];
  for(size_t i = 0; i < nodesize - 1; i++) {
    auto idx = std::min((i+1) * nodesize, part_cand_size - 1);
    partp[i] = part_candp[idx];
  }
  auto part_val = make_node_local_allocate<std::vector<std::vector<T>>>();
  auto part_idx = make_node_local_allocate<std::vector<std::vector<size_t>>>();
  to_sort.mapv(dfcolumn_sort_partition_desc<T>, idx, part_val, part_idx,
               broadcast(part));
  auto exchanged_val = alltoall_exchange(part_val);
  auto exchanged_idx = alltoall_exchange(part_idx);
  auto res_val = make_node_local_allocate<std::vector<T>>();
  res_idx = make_node_local_allocate<std::vector<size_t>>();
  exchanged_val.mapv(set_multimerge_pair_desc<T,size_t>, exchanged_idx,
                     res_val, res_idx);
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->val = std::move(res_val);
  if(contain_nulls) {
    auto exnulls = nulls.map(global_extract_null_helper, exchanged_idx);
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

template <class T>
void typed_dfcolumn<T>::contain_nulls_check() {
  if(nulls.template viewas_dvector<size_t>().size() == 0)
    contain_nulls = false;
  else contain_nulls = true;
}

template <class T>
std::string get_type_name(){throw std::runtime_error("unknown type");}

template<> std::string get_type_name<int>();
template<> std::string get_type_name<unsigned int>();
template<> std::string get_type_name<long>();
template<> std::string get_type_name<unsigned long>();
template<> std::string get_type_name<float>();
template<> std::string get_type_name<double>();

template <class T>
std::string typed_dfcolumn<T>::dtype() const {
  return get_type_name<T>();
}

template <class T, class U>
std::vector<U> do_static_cast(const std::vector<T>& v) {
  const T* vp = v.data();
  size_t size = v.size();
  std::vector<U> ret(size);
  U* retp = ret.data();
  for(size_t i = 0; i < size; i++) {
    retp[i] = static_cast<U>(vp[i]);
  }
  return ret;
}

template <class T>
dvector<float> typed_dfcolumn<T>::as_dvector_float() {
  auto dv = as_dvector<T>();
  return dv.moveto_node_local().map(do_static_cast<T,float>).
    template moveto_dvector<float>();
}

template <class T>
dvector<double> typed_dfcolumn<T>::as_dvector_double() {
  auto dv = as_dvector<T>();
  return dv.moveto_node_local().map(do_static_cast<T,double>).
    template moveto_dvector<double>();
}

}
#endif
