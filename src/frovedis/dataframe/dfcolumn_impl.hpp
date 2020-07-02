#ifndef DFCOLUMN__IMPL_HPP
#define DFCOLUMN__IMPL_HPP

#include "dfcolumn.hpp"

#include <limits>
#include <climits>
#include "../core/set_operations.hpp"
#include "../core/radix_sort.hpp"
#include "hashtable.hpp"
#include "join.hpp"
#include "../text/float_to_words.hpp"
#include "../text/char_int_conv.hpp"

#include <boost/lexical_cast.hpp>

#define GROUPBY_VLEN 256
#define FIND_VALUE_VLEN 256

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
std::vector<std::string> as_string_helper(std::vector<T>& val,
                                          const std::vector<size_t>& nulls) {
  // value of NULL position should always be max
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
    retnulls.resize(current);
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
  auto valp = val.data();
  auto newvalp = newval.data();
  for(size_t i = 0; i < val_size; i++) {
    newvalp[i] = valp[i];
  }
  for(size_t i = 0; i < to_append_size; i++) {
    newvalp[val_size + i] = std::numeric_limits<T>::max();
  }
  val.swap(newval);
  auto nulls_size = nulls.size();
  auto new_nulls_size = nulls_size + to_append_size;
  std::vector<size_t> new_nulls(new_nulls_size);
  auto new_nullsp = new_nulls.data();
  auto nullsp = nulls.data();
  for(size_t i = 0; i < nulls_size; i++) new_nullsp[i] = nullsp[i];
  for(size_t i = 0; i < to_append_size; i++) {
    new_nullsp[nulls_size + i] = val_size + i;
  }
  nulls.swap(new_nulls);
}

template <class T>
void group_by_vector_sum_helper
(const std::vector<T>& val,
 const std::vector<size_t>& split_idx,
 std::vector<T>& ret) {
  auto valp = val.data();
  auto split_idxp = split_idx.data();
  size_t size = val.size();
  if(size == 0) return;
  int valid[GROUPBY_VLEN];
  for(int i = 0; i < GROUPBY_VLEN; i++) valid[i] = true;
  size_t val_idx[GROUPBY_VLEN];
  size_t val_idx_stop[GROUPBY_VLEN];
  size_t out_idx[GROUPBY_VLEN];
  size_t next_group_idx[GROUPBY_VLEN];
  T current_val[GROUPBY_VLEN];
  size_t group_size = split_idx.size() - 1;
  auto retp = ret.data();

  size_t each = ceil_div(size, size_t(GROUPBY_VLEN));
  if(each % 2 == 0) each++;
  auto start_it = split_idx.begin();
  for(size_t i = 0; i < GROUPBY_VLEN; i++) {
    start_it = std::lower_bound(start_it, split_idx.end()-1, each * i);
    if(start_it == split_idx.end()-1) {
      val_idx[i] = size;
      out_idx[i] = group_size;
      next_group_idx[i] = size;
    } else {
      val_idx[i] = *start_it;
      out_idx[i] = start_it - split_idx.begin();
      next_group_idx[i] = split_idxp[out_idx[i]+1];
    }
  }
  for(int i = 0; i < GROUPBY_VLEN - 1; i++) {
    val_idx_stop[i] = val_idx[i + 1];
  }
  val_idx_stop[GROUPBY_VLEN-1] = size;
  for(size_t i = 0; i < GROUPBY_VLEN; i++) {
    if(val_idx[i]  == val_idx_stop[i]) {
      valid[i] = false;
      out_idx[i] = group_size;
    }
    else valid[i] = true;
  }
  size_t max_size = 0;
  for(int i = 0; i < GROUPBY_VLEN; i++) {
    auto current = val_idx_stop[i] - val_idx[i];
    if(max_size < current) max_size = current;
  }
  for(size_t i = 0; i < GROUPBY_VLEN; i++) current_val[i] = 0; // sum

  auto shift_split_idxp = split_idxp + 1;
  for(size_t j = 0; j < max_size; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(int i = 0; i < GROUPBY_VLEN; i++) {
      if(valid[i]) {
        if(val_idx[i] == next_group_idx[i]) {
          retp[out_idx[i]++] = current_val[i];
          current_val[i] = 0; // sum
          next_group_idx[i] = shift_split_idxp[out_idx[i]];
        }
        current_val[i] += valp[val_idx[i]]; // sum
        val_idx[i]++;
        if(val_idx[i] == val_idx_stop[i]) {valid[i] = false;}
      }
    }
  }
#pragma cdir nodep
#pragma _NEC ivdep
  for(int i = 0; i < GROUPBY_VLEN; i++) {
    if(out_idx[i] != group_size) {
      retp[out_idx[i]] = current_val[i];
    }
  }
}

template <class T>
std::vector<std::vector<T>>
sum_helper(std::vector<T>& org_val,
           std::vector<size_t>& grouped_idx,
           std::vector<size_t>& idx_split,
           std::vector<std::vector<size_t>>& hash_divide,
           std::vector<size_t>& nulls) {
  T* org_valp = org_val.data();
  size_t* nullsp = nulls.data();
  size_t nullssize = nulls.size();
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < nullssize; i++) {
    org_valp[nullsp[i]] = 0;
  }
  size_t valsize = grouped_idx.size();
  std::vector<T> val(valsize);
  auto valp = val.data();
  auto grouped_idxp = grouped_idx.data();
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < valsize; i++) {
    valp[i] = org_valp[grouped_idxp[i]];
  }
  size_t splitsize = idx_split.size();
  std::vector<T> ret(splitsize-1);
  auto retp = ret.data();
  size_t* idx_splitp = idx_split.data();
#if defined(_SX) || defined(__ve__)
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
    group_by_vector_sum_helper(val, idx_split, ret);
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
  auto nodesize = hash_divide.size();
  std::vector<std::vector<T>> hashed_ret(nodesize);
  for(size_t i = 0; i < nodesize; i++) {
    auto each_size = hash_divide[i].size();
    hashed_ret[i].resize(each_size);
    auto hash_dividep = hash_divide[i].data();
    auto hashed_retp = hashed_ret[i].data();
    for(size_t j = 0; j < each_size; j++) {
      hashed_retp[j] = retp[hash_dividep[j]];
    }
  }
  // restore the null value
  auto max = std::numeric_limits<T>::max();
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < nullssize; i++) {
    org_valp[nullsp[i]] = max;
  }
  return hashed_ret;
}

template <class T>
void group_by_vector_max_helper
(const std::vector<T>& val,
 const std::vector<size_t>& split_idx,
 std::vector<T>& ret) {
  auto valp = val.data();
  auto split_idxp = split_idx.data();
  size_t size = val.size();
  if(size == 0) return;
  int valid[GROUPBY_VLEN];
  for(int i = 0; i < GROUPBY_VLEN; i++) valid[i] = true;
  size_t val_idx[GROUPBY_VLEN];
  size_t val_idx_stop[GROUPBY_VLEN];
  size_t out_idx[GROUPBY_VLEN];
  size_t next_group_idx[GROUPBY_VLEN];
  T current_val[GROUPBY_VLEN];
  size_t group_size = split_idx.size() - 1;
  auto retp = ret.data();

  size_t each = ceil_div(size, size_t(GROUPBY_VLEN));
  if(each % 2 == 0) each++;
  auto start_it = split_idx.begin();
  for(size_t i = 0; i < GROUPBY_VLEN; i++) {
    start_it = std::lower_bound(start_it, split_idx.end()-1, each * i);
    if(start_it == split_idx.end()-1) {
      val_idx[i] = size;
      out_idx[i] = group_size;
      next_group_idx[i] = size;
    } else {
      val_idx[i] = *start_it;
      out_idx[i] = start_it - split_idx.begin();
      next_group_idx[i] = split_idxp[out_idx[i]+1];
    }
  }
  for(int i = 0; i < GROUPBY_VLEN - 1; i++) {
    val_idx_stop[i] = val_idx[i + 1];
  }
  val_idx_stop[GROUPBY_VLEN-1] = size;
  for(size_t i = 0; i < GROUPBY_VLEN; i++) {
    if(val_idx[i]  == val_idx_stop[i]) {
      valid[i] = false;
      out_idx[i] = group_size;
    }
    else valid[i] = true;
  }
  size_t max_size = 0;
  for(int i = 0; i < GROUPBY_VLEN; i++) {
    auto current = val_idx_stop[i] - val_idx[i];
    if(max_size < current) max_size = current;
  }
  auto min = std::numeric_limits<T>::min();
  for(size_t i = 0; i < GROUPBY_VLEN; i++) current_val[i] = min; // max

  auto shift_split_idxp = split_idxp + 1;
  for(size_t j = 0; j < max_size; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(int i = 0; i < GROUPBY_VLEN; i++) {
      if(valid[i]) {
        if(val_idx[i] == next_group_idx[i]) {
          retp[out_idx[i]++] = current_val[i];
          current_val[i] = min; // max
          next_group_idx[i] = shift_split_idxp[out_idx[i]];
        }
        if(current_val[i] < valp[val_idx[i]])
          current_val[i] = valp[val_idx[i]]; // max
        val_idx[i]++;
        if(val_idx[i] == val_idx_stop[i]) {valid[i] = false;}
      }
    }
  }
#pragma cdir nodep
#pragma _NEC ivdep
  for(int i = 0; i < GROUPBY_VLEN; i++) {
    if(out_idx[i] != group_size) {
      retp[out_idx[i]] = current_val[i];
    }
  }
}

template <class T>
std::vector<std::vector<T>>
max_helper(std::vector<T>& org_val,
           std::vector<size_t>& grouped_idx,
           std::vector<size_t>& idx_split,
           std::vector<std::vector<size_t>>& hash_divide,
           std::vector<size_t>& nulls) {
  T* org_valp = org_val.data();
  size_t* nullsp = nulls.data();
  size_t nullssize = nulls.size();
  auto min = std::numeric_limits<T>::min();
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < nullssize; i++) {
    org_valp[nullsp[i]] = min;
  }
  size_t valsize = grouped_idx.size();
  std::vector<T> val(valsize);
  auto valp = val.data();
  auto grouped_idxp = grouped_idx.data();
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < valsize; i++) {
    valp[i] = org_valp[grouped_idxp[i]];
  }
  size_t splitsize = idx_split.size();
  std::vector<T> ret(splitsize-1);
  auto retp = ret.data();
  size_t* idx_splitp = idx_split.data();
#if defined(_SX) || defined(__ve__)
  if(valsize / splitsize > GROUPBY_VLEN * 2) {
    size_t end = 0;
    size_t start = 0;
    for(size_t i = 0; i < splitsize-1; i++) {
      start = end;
      end = idx_splitp[i+1];
      T max = std::numeric_limits<T>::min();
      for(size_t j = start; j < end; j++) {
        if(max < valp[j]) max = valp[j];
      }
      retp[i] = max;
    }
  } else {
    group_by_vector_max_helper(val, idx_split, ret);
  }
#else  
  size_t end = 0;
  size_t start = 0;
  for(size_t i = 0; i < splitsize-1; i++) {
    start = end;
    end = idx_splitp[i+1];
    T max = std::numeric_limits<T>::min();
    for(size_t j = start; j < end; j++) {
      if(max < valp[j]) max = valp[j];
    }
    retp[i] = max;
  }
#endif
  auto nodesize = hash_divide.size();
  std::vector<std::vector<T>> hashed_ret(nodesize);
  for(size_t i = 0; i < nodesize; i++) {
    auto each_size = hash_divide[i].size();
    hashed_ret[i].resize(each_size);
    auto hash_dividep = hash_divide[i].data();
    auto hashed_retp = hashed_ret[i].data();
    for(size_t j = 0; j < each_size; j++) {
      hashed_retp[j] = retp[hash_dividep[j]];
    }
  }
  // restore the null value
  auto max = std::numeric_limits<T>::max();
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < nullssize; i++) {
    org_valp[nullsp[i]] = max;
  }
  return hashed_ret;
}

template <class T>
void group_by_vector_min_helper
(const std::vector<T>& val,
 const std::vector<size_t>& split_idx,
 std::vector<T>& ret) {
  auto valp = val.data();
  auto split_idxp = split_idx.data();
  size_t size = val.size();
  if(size == 0) return;
  int valid[GROUPBY_VLEN];
  for(int i = 0; i < GROUPBY_VLEN; i++) valid[i] = true;
  size_t val_idx[GROUPBY_VLEN];
  size_t val_idx_stop[GROUPBY_VLEN];
  size_t out_idx[GROUPBY_VLEN];
  size_t next_group_idx[GROUPBY_VLEN];
  T current_val[GROUPBY_VLEN];
  size_t group_size = split_idx.size() - 1;
  auto retp = ret.data();

  size_t each = ceil_div(size, size_t(GROUPBY_VLEN));
  if(each % 2 == 0) each++;
  auto start_it = split_idx.begin();
  for(size_t i = 0; i < GROUPBY_VLEN; i++) {
    start_it = std::lower_bound(start_it, split_idx.end()-1, each * i);
    if(start_it == split_idx.end()-1) {
      val_idx[i] = size;
      out_idx[i] = group_size;
      next_group_idx[i] = size;
    } else {
      val_idx[i] = *start_it;
      out_idx[i] = start_it - split_idx.begin();
      next_group_idx[i] = split_idxp[out_idx[i]+1];
    }
  }
  for(int i = 0; i < GROUPBY_VLEN - 1; i++) {
    val_idx_stop[i] = val_idx[i + 1];
  }
  val_idx_stop[GROUPBY_VLEN-1] = size;
  for(size_t i = 0; i < GROUPBY_VLEN; i++) {
    if(val_idx[i]  == val_idx_stop[i]) {
      valid[i] = false;
      out_idx[i] = group_size;
    }
    else valid[i] = true;
  }
  size_t max_size = 0;
  for(int i = 0; i < GROUPBY_VLEN; i++) {
    auto current = val_idx_stop[i] - val_idx[i];
    if(max_size < current) max_size = current;
  }
  auto max = std::numeric_limits<T>::max();
  for(size_t i = 0; i < GROUPBY_VLEN; i++) current_val[i] = max; // min

  auto shift_split_idxp = split_idxp + 1;
  for(size_t j = 0; j < max_size; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(int i = 0; i < GROUPBY_VLEN; i++) {
      if(valid[i]) {
        if(val_idx[i] == next_group_idx[i]) {
          retp[out_idx[i]++] = current_val[i];
          current_val[i] = max; // min
          next_group_idx[i] = shift_split_idxp[out_idx[i]];
        }
        if(current_val[i] > valp[val_idx[i]])
          current_val[i] = valp[val_idx[i]]; // min
        val_idx[i]++;
        if(val_idx[i] == val_idx_stop[i]) {valid[i] = false;}
      }
    }
  }
#pragma cdir nodep
#pragma _NEC ivdep
  for(int i = 0; i < GROUPBY_VLEN; i++) {
    if(out_idx[i] != group_size) {
      retp[out_idx[i]] = current_val[i];
    }
  }
}

template <class T>
std::vector<std::vector<T>>
min_helper(std::vector<T>& org_val,
           std::vector<size_t>& grouped_idx,
           std::vector<size_t>& idx_split,
           std::vector<std::vector<size_t>>& hash_divide,
           std::vector<size_t>& nulls) {
  T* org_valp = org_val.data();
  size_t valsize = grouped_idx.size();
  std::vector<T> val(valsize);
  auto valp = val.data();
  auto grouped_idxp = grouped_idx.data();
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < valsize; i++) {
    valp[i] = org_valp[grouped_idxp[i]];
  }
  size_t splitsize = idx_split.size();
  std::vector<T> ret(splitsize-1);
  auto retp = ret.data();
  size_t* idx_splitp = idx_split.data();
#if defined(_SX) || defined(__ve__)
  if(valsize / splitsize > GROUPBY_VLEN * 2) {
    size_t end = 0;
    size_t start = 0;
    for(size_t i = 0; i < splitsize-1; i++) {
      start = end;
      end = idx_splitp[i+1];
      T min = std::numeric_limits<T>::max();
      for(size_t j = start; j < end; j++) {
        if(min > valp[j]) min = valp[j];
      }
      retp[i] = min;
    }
  } else {
    group_by_vector_min_helper(val, idx_split, ret);
  }
#else  
  size_t end = 0;
  size_t start = 0;
  for(size_t i = 0; i < splitsize-1; i++) {
    start = end;
    end = idx_splitp[i+1];
    T min = std::numeric_limits<T>::max();
    for(size_t j = start; j < end; j++) {
      if(min > valp[j]) min = valp[j];
    }
    retp[i] = min;
  }
#endif
  auto nodesize = hash_divide.size();
  std::vector<std::vector<T>> hashed_ret(nodesize);
  for(size_t i = 0; i < nodesize; i++) {
    auto each_size = hash_divide[i].size();
    hashed_ret[i].resize(each_size);
    auto hash_dividep = hash_divide[i].data();
    auto hashed_retp = hashed_ret[i].data();
    for(size_t j = 0; j < each_size; j++) {
      hashed_retp[j] = retp[hash_dividep[j]];
    }
  }
  return hashed_ret;
}

template <class T>
T sum_helper2(std::vector<T>& val,
              std::vector<size_t>& nulls) {
  size_t valsize = val.size();
  size_t nullssize = nulls.size();
  T* valp = &val[0];
  size_t* nullsp = &nulls[0];
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < nullssize; i++) {
    valp[nullsp[i]] = 0;
  }
  T total = 0;
  for(size_t i = 0; i < valsize; i++) {
    total += valp[i];
  }
  T max = std::numeric_limits<T>::max();
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < nullssize; i++) {
    valp[nullsp[i]] = max;
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
  T min = std::numeric_limits<T>::min();
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < nullssize; i++) {
    valp[nullsp[i]] = min;
  }
  T current_max = std::numeric_limits<T>::min();
  for(size_t i = 0; i < valsize; i++) {
    if(current_max < valp[i]) current_max = valp[i];
  }
  T max = std::numeric_limits<T>::max();
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < nullssize; i++) {
    valp[nullsp[i]] = max;
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
std::vector<size_t> typed_dfcolumn<T>::sizes() {
  return val.template viewas_dvector<T>().sizes();
}

template <class T>
dvector<std::string> typed_dfcolumn<T>::as_string() {
  return val.map(as_string_helper<T>, nulls).
    template moveto_dvector<std::string>();
}

template <class T>
words dfcolumn_as_words_helper(std::vector<T>& v,
                               const std::vector<size_t>& nulls,
                               const std::string& nullstr) {
  auto nulls_size = nulls.size();
  auto nullsp = nulls.data();
  auto vp = v.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob  
  for(size_t i = 0; i < nulls_size; i++) {
    vp[nullsp[i]] = 0; // max is too long for creating words
  }
  auto ws = number_to_words<T>(v);
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob  
  for(size_t i = 0; i < nulls_size; i++) {
    vp[nullsp[i]] = std::numeric_limits<T>::max();
  }
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
  return ws;
}

template <class T>
node_local<words> typed_dfcolumn<T>::as_words(bool quote_escape, // not used
                                              const std::string& nullstr) {
  if(contain_nulls)
    return val.map(dfcolumn_as_words_helper<T>, nulls, broadcast(nullstr));
  else 
    return val.map(+[](const std::vector<T>& v){return number_to_words<T>(v);});
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
 node_local<std::vector<size_t>>& to_store_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
  time_spent t(DEBUG);
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  auto exdata = val.map(global_extract_helper<T>, exchanged_idx);
  t.show("global_exract_helper: ");
  auto exchanged_back = alltoall_exchange(exdata).map(flatten<T>);
  t.show("alltoall_exchange + flatten: ");
  ret->val = exchanged_back.map
    (+[](std::vector<T>& val, std::vector<size_t>& idx) {
      auto valp = val.data();
      auto idxp = idx.data();
      auto size = idx.size();
      std::vector<T> ret(size);
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

void create_merge_map(std::vector<size_t>& nodeid,
                      std::vector<size_t>& split,
                      std::vector<std::vector<size_t>>& merge_map);

#if defined(_SX) || defined(__ve__)
// loop raking version
template <class T>
std::vector<size_t> find_value(std::vector<T>& val, T tofind) {
  auto size = val.size();
  if(size == 0) return std::vector<size_t>();
  auto vp = val.data();
  std::vector<size_t> idxtmp(size);
  auto idxtmpp = idxtmp.data();
  size_t each = size / FIND_VALUE_VLEN; // maybe 0
  if(each % 2 == 0 && each > 1) each--;
  size_t rest = size - each * FIND_VALUE_VLEN;
  size_t out_ridx[FIND_VALUE_VLEN];
// never remove this vreg! this is needed folowing vovertake
// though this prevents ftrace...
#pragma _NEC vreg(out_ridx)
  for(size_t i = 0; i < FIND_VALUE_VLEN; i++) {
    out_ridx[i] = each * i;
  }
  if(each == 0) {
    size_t current = 0;
    for(size_t i = 0; i < size; i++) {
      if(vp[i] == tofind) {
        idxtmpp[current] = i;
        current++;
      }
    }
    std::vector<size_t> found(current);
    auto foundp = found.data();
    for(size_t i = 0; i < current; i++) {
      foundp[i] = idxtmpp[i];
    }
    return found;
  } else {
#pragma _NEC vob
    for(size_t j = 0; j < each; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
      for(size_t i = 0; i < FIND_VALUE_VLEN; i++) {
        auto loaded_v = vp[j + each * i];
        if(loaded_v == tofind) {
          idxtmpp[out_ridx[i]] = j + each * i;
          out_ridx[i]++;
        }
      }
    }
    size_t rest_idx_start = each * FIND_VALUE_VLEN;
    size_t rest_idx = rest_idx_start;
    if(rest != 0) {
      for(size_t j = 0; j < rest; j++) {
        auto loaded_v = vp[j + rest_idx_start]; 
        if(loaded_v == tofind) {
          idxtmpp[rest_idx] = j + rest_idx_start;
          rest_idx++;
        }
      }
    }
    size_t sizes[FIND_VALUE_VLEN];
    for(size_t i = 0; i < FIND_VALUE_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < FIND_VALUE_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - each * FIND_VALUE_VLEN;
    total += rest_size;
    std::vector<size_t> found(total);
    auto foundp = found.data();
    size_t current = 0;
    for(size_t i = 0; i < FIND_VALUE_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        foundp[current + j] = idxtmpp[each * i + j];
      }
      current += sizes[i];
    }
    for(size_t j = 0; j < rest_size; j++) {
      foundp[current + j] = idxtmpp[rest_idx_start + j];
    }
    return found;
  }
}
#else
template <class T>
std::vector<size_t> find_value(std::vector<T>& val, T tofind) {
  auto size = val.size();
  auto vp = val.data();
  if(size == 0) return std::vector<size_t>();
  std::vector<size_t> idxtmp(size);
  auto idxtmpp = idxtmp.data();
  size_t current = 0;
  for(size_t i = 0; i < size; i++) {
    if(vp[i] == tofind) {
      idxtmpp[current] = i;
      current++;
    }
  }
  std::vector<size_t> found(current);
  auto foundp = found.data();
  for(size_t i = 0; i < current; i++) {
    foundp[i] = idxtmpp[i];
  }
  return found;
}
#endif

template <class T>
void group_by_impl(node_local<std::vector<T>>& val,
                   node_local<std::vector<size_t>>& nulls,
                   node_local<std::vector<size_t>>& local_idx,
                   node_local<std::vector<size_t>>& split_idx,
                   node_local<std::vector<std::vector<size_t>>>& hash_divide,
                   node_local<std::vector<std::vector<size_t>>>& merge_map,
                   node_local<std::vector<T>>& retval,
                   node_local<std::vector<size_t>>& retnulls) {
  auto hashsplit_group = val.map
    (+[](std::vector<T>& val,
         std::vector<size_t>& idx,
         std::vector<size_t>& split,
         std::vector<std::vector<size_t>>& hash_divide) {
      // null is also used, so extract is not used
      auto valp = val.data();
      auto idxp = idx.data();
      auto size = idx.size();
      std::vector<T> newval(size);
      auto newvalp = newval.data();
      for(size_t i = 0; i < size; i++) {newvalp[i] = valp[idxp[i]];}
      radix_sort(newvalp, idxp, size); 
      split = set_separate(newval);
      auto split_size = split.size();
      std::vector<T> group(split_size-1);
      auto groupp = group.data();
      auto splitp = split.data();
      for(size_t i = 0; i < split_size-1; i++) {
        groupp[i] = newvalp[splitp[i]];
      }
      std::vector<size_t> iota(split_size-1);
      auto iotap = iota.data();
      for(size_t i = 0; i < split_size-1; i++) iotap[i] = i;
      std::vector<std::vector<T>> hashsplit_group;
      split_by_hash(group, hashsplit_group, iota, hash_divide);
      return hashsplit_group;
    }, local_idx, split_idx, hash_divide);
  auto exchanged_group = alltoall_exchange(hashsplit_group);
  retval = exchanged_group.map
    (+[](std::vector<std::vector<T>>& exchanged_group,
         std::vector<std::vector<size_t>>& merge_map,
         std::vector<size_t>& nulls) {
      size_t nodesize = exchanged_group.size();
      size_t total = 0;
      for(size_t i = 0; i < nodesize; i++) {
        total += exchanged_group[i].size();
      }
      std::vector<size_t> nodeid(total);
      auto nodeidp = nodeid.data();
      size_t current = 0;
      for(size_t i = 0; i < nodesize; i++) {
        auto current_size = exchanged_group[i].size();
        for(size_t j = 0; j < current_size; j++) {
          nodeidp[current + j] = i;
        }
        current += current_size;
      }
      auto flat_exchanged_group = flatten(exchanged_group);
      auto flat_exchanged_groupp = flat_exchanged_group.data();
      radix_sort(flat_exchanged_groupp, nodeidp, total);
      auto sep = set_separate(flat_exchanged_group);
      merge_map.resize(nodesize);
      for(size_t i = 0; i < nodesize; i++) {
        merge_map[i].resize(exchanged_group[i].size());
      }
      create_merge_map(nodeid, sep, merge_map);
      auto sepsize = sep.size();
      std::vector<T> retval(sepsize-1);
      auto retvalp = retval.data();
      auto sepp = sep.data();
      for(size_t i = 0; i < sepsize-1; i++) {
        retvalp[i] = flat_exchanged_groupp[sepp[i]];
      }
      if(sepsize > 1 && // there is data and at least one group
         retvalp[sepp[sepsize-2]] == std::numeric_limits<T>::max()) {
        nulls.resize(1);
        nulls[0] = sepsize-2;
      }
      return retval;
    }, merge_map, retnulls);
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::group_by
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map) {
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  group_by_impl(val, nulls, local_idx, split_idx, hash_divide, merge_map,
                ret->val, ret->nulls);
  ret->contain_nulls_check();
  return ret;
}

template <class T>
void multi_group_by_sort_helper(std::vector<T>& val,
                                std::vector<size_t>& local_idx) {
  size_t size = local_idx.size();
  std::vector<T> val2(size);
  T* valp = &val[0];
  T* val2p = &val2[0];
  size_t* local_idxp = &local_idx[0];
  for(size_t i = 0; i < size; i++) {
    val2p[i] = valp[local_idxp[i]];
  }
  radix_sort(val2p, local_idxp, size);
}

template <class T>
void
typed_dfcolumn<T>::multi_group_by_sort
(node_local<std::vector<size_t>>& local_idx) {
  val.mapv(multi_group_by_sort_helper<T>, local_idx);
}

template <class T>
std::vector<size_t> 
multi_group_by_sort_split_helper(std::vector<T>& val,
                                 std::vector<size_t>& local_idx) {
  size_t size = local_idx.size();
  std::vector<T> val2(size);
  T* valp = &val[0];
  T* val2p = &val2[0];
  size_t* local_idxp = &local_idx[0];
  for(size_t i = 0; i < size; i++) {
    val2p[i] = valp[local_idxp[i]];
  }
  radix_sort(val2p, local_idxp, size);
  return set_separate(val2);
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::multi_group_by_sort_split
(node_local<std::vector<size_t>>& local_idx) {
  return val.map(multi_group_by_sort_split_helper<T>, local_idx);
}

template <class T>
std::vector<size_t>
multi_group_by_split_helper(std::vector<T>& val,
                            std::vector<size_t>& local_idx) {
  size_t size = local_idx.size();
  std::vector<T> val2(size);
  T* valp = &val[0];
  T* val2p = &val2[0];
  size_t* local_idxp = &local_idx[0];
  for(size_t i = 0; i < size; i++) {
    val2p[i] = valp[local_idxp[i]];
  }
  return set_separate(val2);
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::multi_group_by_split
(node_local<std::vector<size_t>>& local_idx) {
  return val.map(multi_group_by_split_helper<T>, local_idx);
}

template <class T>
std::vector<T>
multi_group_by_extract_helper(std::vector<T>& val,
                              std::vector<size_t>& local_idx,
                              std::vector<size_t>& split_idx) {
  auto valp = val.data();
  auto local_idxp = local_idx.data();
  auto split_idxp = split_idx.data();
  auto size = split_idx.size();
  std::vector<T> ret(size-1);
  auto retp = ret.data();
  for(size_t i = 0; i < size-1; i++) {
    retp[i] = valp[local_idxp[split_idxp[i]]];
  }
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::multi_group_by_extract
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 bool check_nulls) {
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->val = val.map(multi_group_by_extract_helper<T>, local_idx, split_idx);
  ret->contain_nulls = contain_nulls;
  if(contain_nulls && check_nulls) {
    ret->nulls = ret->val.map(+[](std::vector<T>& val) {
        return find_value(val, std::numeric_limits<T>::max());
      });
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  return ret;
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
node_local<std::vector<size_t>> 
typed_dfcolumn<T>::calc_hash_base() {
  return val.map(calc_hash_base_helper<T>);
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
      hash_basep[i] =
        ((hash_basep[i] << shift) |
         (hash_basep[i] >> (sizeof(size_t)*CHAR_BIT - shift)))
        + static_cast<size_t>(valp[i]);
    }
  }
  int shift;
  SERIALIZE(shift)
};

template <class T>
void 
typed_dfcolumn<T>::calc_hash_base(node_local<std::vector<size_t>>& hash_base,
                                  int shift) {
  val.mapv(calc_hash_base_helper2<T>(shift), hash_base);
}

template <class T>
node_local<std::vector<T>>
multi_group_by_exchange_helper
(node_local<std::vector<T>>& val,
 node_local<std::vector<std::vector<size_t>>>& hash_divide) {
  auto tmp = val.map
    (+[](std::vector<T>& val,
         std::vector<std::vector<size_t>>& hash_divide) {
      auto valp = val.data();
      std::vector<std::vector<T>> ret(hash_divide.size());
      for(size_t i = 0; i < hash_divide.size(); i++) {
        auto size = hash_divide[i].size();
        ret[i].resize(size);
        auto retp = ret[i].data();
        auto hash_dividep = hash_divide[i].data();
        for(size_t j = 0; j < size; j++) {
          retp[j] = valp[hash_dividep[j]];
        }
      }
      return ret;
    }, hash_divide);
  auto exchanged = alltoall_exchange(tmp);
  return exchanged.map(flatten<T>);
}

template <class T>
std::shared_ptr<dfcolumn> 
typed_dfcolumn<T>::multi_group_by_exchange
(node_local<std::vector<std::vector<size_t>>>& hash_divide) {
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->val = multi_group_by_exchange_helper(val, hash_divide);
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  ret->contain_nulls = contain_nulls;
  return ret;
}

template <class T>
std::vector<std::vector<size_t>>
count_helper(std::vector<T>& val,
             std::vector<size_t>& grouped_idx,
             std::vector<size_t>& idx_split,
             std::vector<std::vector<size_t>>& hash_divide,
             std::vector<size_t>& nulls);

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::sum
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_agg = val.map(sum_helper<T>, local_grouped_idx, local_idx_split,
                           hash_divide, nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<T>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         size_t row_size) {
      std::vector<T> newval(row_size);
      auto newvalp = newval.data();
      for(size_t i = 0; i < exchanged.size(); i++) {
        auto currentp = exchanged[i].data();
        auto current_size = exchanged[i].size();
        auto merge_mapp = merge_map[i].data();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t j = 0; j < current_size; j++) {
          newvalp[merge_mapp[j]] += currentp[j];
        }
      }
      return newval;
    }, merge_map, row_sizes);
  ret->val = std::move(newval);
  if(contain_nulls) {
    // in the case of sum, default value (= 0) cannot be used to check null
    auto local_agg = val.map(count_helper<T>, local_grouped_idx,
                             local_idx_split, hash_divide, nulls);
    auto exchanged = alltoall_exchange(local_agg);
    ret->nulls = exchanged.map
      (+[](std::vector<std::vector<size_t>>& exchanged,
           std::vector<std::vector<size_t>>& merge_map,
           size_t row_size,
           std::vector<T>& val) {
        std::vector<size_t> newval(row_size);
        auto newvalp = newval.data();
        for(size_t i = 0; i < exchanged.size(); i++) {
          auto currentp = exchanged[i].data();
          auto current_size = exchanged[i].size();
          auto merge_mapp = merge_map[i].data();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            newvalp[merge_mapp[j]] += currentp[j];
          }
        }
        auto retnulls = find_value(newval, size_t(0));
        auto retnullsp = retnulls.data();
        auto retnullssize = retnulls.size();
        auto valp = val.data();
        auto max = std::numeric_limits<T>::max();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < retnullssize; i++) {
          valp[retnullsp[i]] = max;
        }
        return retnulls;
      }, merge_map, row_sizes, ret->val);
    ret->contain_nulls_check();
  }
  return ret;
}

template <class T>
std::vector<std::vector<size_t>>
count_helper(std::vector<T>& val,
             std::vector<size_t>& grouped_idx,
             std::vector<size_t>& idx_split,
             std::vector<std::vector<size_t>>& hash_divide,
             std::vector<size_t>& nulls) {
  size_t nullssize = nulls.size();
  if(nullssize == 0) {
    size_t splitsize = idx_split.size();
    std::vector<size_t> ret(splitsize-1);
    size_t* idx_splitp = &idx_split[0];
    size_t* retp = &ret[0];
    for(size_t i = 0; i < splitsize-1; i++) {
      retp[i] = idx_splitp[i+1] - idx_splitp[i];
    }
    auto nodesize = hash_divide.size();
    std::vector<std::vector<size_t>> hashed_ret(nodesize);
    for(size_t i = 0; i < nodesize; i++) {
      auto each_size = hash_divide[i].size();
      hashed_ret[i].resize(each_size);
      auto hash_dividep = hash_divide[i].data();
      auto hashed_retp = hashed_ret[i].data();
      for(size_t j = 0; j < each_size; j++) {
        hashed_retp[j] = retp[hash_dividep[j]];
      }
    }
    return hashed_ret;
  } else { // slow: takes same time as sum
    auto size = val.size();
    std::vector<size_t> tmp(size, 1);
    return sum_helper(tmp, grouped_idx, idx_split, hash_divide, nulls);
  }
}

template <class T>
std::shared_ptr<dfcolumn>
count_impl(node_local<std::vector<T>>& val,
           node_local<std::vector<size_t>>& nulls,
           node_local<std::vector<size_t>>& local_grouped_idx,
           node_local<std::vector<size_t>>& local_idx_split,
           node_local<std::vector<std::vector<size_t>>>& hash_divide,
           node_local<std::vector<std::vector<size_t>>>& merge_map,
           node_local<size_t>& row_sizes) {
  auto ret = std::make_shared<typed_dfcolumn<size_t>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_agg = val.map(count_helper<T>, local_grouped_idx, local_idx_split,
                           hash_divide, nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<size_t>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         size_t row_size) {
      std::vector<size_t> newval(row_size);
      auto newvalp = newval.data();
      for(size_t i = 0; i < exchanged.size(); i++) {
        auto currentp = exchanged[i].data();
        auto current_size = exchanged[i].size();
        auto merge_mapp = merge_map[i].data();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t j = 0; j < current_size; j++) {
          newvalp[merge_mapp[j]] += currentp[j];
        }
      }
      return newval;
    }, merge_map, row_sizes);
  ret->val = std::move(newval);
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::count
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  return count_impl(val, nulls, local_grouped_idx, local_idx_split,
                    hash_divide, merge_map, row_sizes);
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::avg
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  auto ret = std::make_shared<typed_dfcolumn<double>>();
  auto sum_column = std::dynamic_pointer_cast<typed_dfcolumn<T>>
    (sum(local_grouped_idx, local_idx_split, hash_divide, merge_map,
         row_sizes));
  auto count_column = std::dynamic_pointer_cast<typed_dfcolumn<size_t>>
    (count(local_grouped_idx, local_idx_split, hash_divide,
           merge_map, row_sizes));
  ret->val = sum_column->val.map
    (+[](std::vector<T>& val, std::vector<size_t>& count) {
      auto size = val.size();
      auto valp = val.data();
      auto countp = count.data();
      std::vector<double> ret(size);
      auto retp = ret.data();
      for(size_t i = 0; i < size; i++) {
        retp[i] =
          static_cast<double>(valp[i]) / static_cast<double>(countp[i]);
      }
      return ret;
    }, count_column->val);
  if(sum_column->contain_nulls){
    ret->nulls = sum_column->nulls;
    ret->val.mapv
      (+[](std::vector<double>& val, std::vector<size_t>& nulls) {
        auto valp = val.data();
        auto nullsp = nulls.data();
        auto size = nulls.size();
        auto max = std::numeric_limits<T>::max();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < size; i++) {
          valp[nullsp[i]] = max;
        }
      }, ret->nulls);
    ret->contain_nulls = true;
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
    ret->contain_nulls = false;
  }
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::max
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_agg = val.map(max_helper<T>, local_grouped_idx, local_idx_split,
                           hash_divide, nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<T>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         size_t row_size) {
      std::vector<T> newval(row_size);
      auto newvalp = newval.data();
      auto min = std::numeric_limits<T>::min();
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
    ret->nulls = ret->val.map(+[](std::vector<T>& val) {
        auto nulls = find_value(val, std::numeric_limits<T>::min());
        auto valp = val.data();
        auto nullsp = nulls.data();
        auto size = nulls.size();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < size; i++)
          valp[nullsp[i]] = std::numeric_limits<T>::max();
        return nulls;
      });
    ret->contain_nulls_check();
  }
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::min
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_agg = val.map(min_helper<T>, local_grouped_idx, local_idx_split,
                           hash_divide, nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<T>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         size_t row_size) {
      std::vector<T> newval(row_size);
      auto newvalp = newval.data();
      auto max = std::numeric_limits<T>::max();
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
    ret->nulls = ret->val.map(+[](std::vector<T>& val) {
        return find_value(val, std::numeric_limits<T>::max());
      });
    ret->contain_nulls_check();
  }
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

node_local<std::vector<size_t>>
limit_nulls_head(node_local<std::vector<size_t>>& nulls, 
                 const std::vector<size_t>& sizes,
                 size_t limit);

node_local<std::vector<size_t>>
limit_nulls_tail(node_local<std::vector<size_t>>& nulls, 
                 const std::vector<size_t>& sizes,
                 const std::vector<size_t>& new_sizes,
                 size_t limit);

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::head(size_t limit) {
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->val = val.template viewas_dvector<T>().head(limit).moveto_node_local();
  if(contain_nulls) {
    ret->nulls = limit_nulls_head(nulls, sizes(), limit);
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::tail(size_t limit) {
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->val = val.template viewas_dvector<T>().tail(limit).moveto_node_local();
  auto new_sizes = ret->val.template viewas_dvector<T>().sizes();
  if(contain_nulls) {
    ret->nulls = limit_nulls_tail(nulls, sizes(), new_sizes, limit);
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  return ret;
}

template <class T>
void typed_dfcolumn<T>::save(const std::string& file) {
  val.template viewas_dvector<T>().savebinary(file);
  auto dv_nulls = nulls.template viewas_dvector<size_t>();
  auto sizes = val.template viewas_dvector<T>().sizes();
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
  auto valp = val.data();
  auto idxp = idx.data();
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
      part_valp[i] = valp[start+i];
      part_idxp[i] = idxp[start+i];
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
    part_valp[i] = valp[start+i];
    part_idxp[i] = idxp[start+i];
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
  auto valp = val.data();
  auto idxp = idx.data();
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
      part_valp[i] = valp[start+i];
      part_idxp[i] = idxp[start+i];
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
    part_valp[i] = valp[start+i];
    part_idxp[i] = idxp[start+i];
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
std::string get_dftype_name(){throw std::runtime_error("unknown type");}

template<> std::string get_dftype_name<int>();
template<> std::string get_dftype_name<unsigned int>();
template<> std::string get_dftype_name<long>();
template<> std::string get_dftype_name<unsigned long>();
template<> std::string get_dftype_name<float>();
template<> std::string get_dftype_name<double>();

template <class T>
std::string typed_dfcolumn<T>::dtype() const {
  return get_dftype_name<T>();
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
