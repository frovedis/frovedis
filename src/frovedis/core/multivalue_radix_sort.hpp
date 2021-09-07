#ifndef MULTIVALUE_RADIX_SORT_HPP
#define MULTIVALUE_RADIX_SORT_HPP

#include "radix_sort.hpp"
#include "multivalue_utility.hpp"

namespace frovedis {

template <class K, class V>
void multivalue_radix_sort(K* key_array, V* val_array, size_t size,
                           int num_values, bool positive_only = false) {
  std::vector<K> tmp_key(size);
  std::vector<size_t> idx(size);
  auto idxp = idx.data();
  for(size_t i = 0; i < size; i++) idxp[i] = i;
  auto tmp_keyp = tmp_key.data();
  for(size_t i = 0; i < num_values; i++) {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t j = 0; j < size; j++) {
      tmp_keyp[j] = key_array[idxp[j] * num_values + (num_values - i - 1)];
    }
    radix_sort(tmp_keyp, idxp, size, positive_only);
  }
  std::vector<V> tmp_val(size);
  auto tmp_valp = tmp_val.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < size; i++) {
    tmp_valp[i] = val_array[idxp[i]];
  }
  for(size_t i = 0; i < size; i++) val_array[i] = tmp_valp[i];
  std::vector<K> tmp_ret(size * num_values);
  auto tmp_retp = tmp_ret.data();
  multivalue_extract(key_array, idxp, size, size, num_values, tmp_retp);
  for(size_t i = 0; i < size * num_values; i++) key_array[i] = tmp_retp[i];
}

template <class K>
void multivalue_radix_sort(K* key_array, size_t size,
                           int num_values, bool positive_only = false) {
  std::vector<K> tmp_key(size);
  std::vector<size_t> idx(size);
  auto idxp = idx.data();
  for(size_t i = 0; i < size; i++) idxp[i] = i;
  auto tmp_keyp = tmp_key.data();
  for(size_t i = 0; i < num_values; i++) {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t j = 0; j < size; j++) {
      tmp_keyp[j] = key_array[idxp[j] * num_values + (num_values - i - 1)];
    }
    radix_sort(tmp_keyp, idxp, size, positive_only);
  }
  std::vector<K> tmp_ret(size * num_values);
  auto tmp_retp = tmp_ret.data();
  multivalue_extract(key_array, idxp, size, size, num_values, tmp_retp);
  for(size_t i = 0; i < size * num_values; i++) key_array[i] = tmp_retp[i];
}

template <class K, class V>
void multivalue_radix_sort_desc(K* key_array, V* val_array, size_t size,
                                int num_values, bool positive_only = false) {
  std::vector<K> tmp_key(size);
  std::vector<size_t> idx(size);
  auto idxp = idx.data();
  for(size_t i = 0; i < size; i++) idxp[i] = i;
  auto tmp_keyp = tmp_key.data();
  for(size_t i = 0; i < num_values; i++) {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t j = 0; j < size; j++) {
      tmp_keyp[j] = key_array[idxp[j] * num_values + (num_values - i - 1)];
    }
    radix_sort_desc(tmp_keyp, idxp, size, positive_only);
  }
  std::vector<V> tmp_val(size);
  auto tmp_valp = tmp_val.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < size; i++) {
    tmp_valp[i] = val_array[idxp[i]];
  }
  for(size_t i = 0; i < size; i++) val_array[i] = tmp_valp[i];
  std::vector<K> tmp_ret(size * num_values);
  auto tmp_retp = tmp_ret.data();
  multivalue_extract(key_array, idxp, size, size, num_values, tmp_retp);
  for(size_t i = 0; i < size * num_values; i++) key_array[i] = tmp_retp[i];
}

template <class K>
void multivalue_radix_sort_desc(K* key_array, size_t size,
                                int num_values, bool positive_only = false) {
  std::vector<K> tmp_key(size);
  std::vector<size_t> idx(size);
  auto idxp = idx.data();
  for(size_t i = 0; i < size; i++) idxp[i] = i;
  auto tmp_keyp = tmp_key.data();
  for(size_t i = 0; i < num_values; i++) {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t j = 0; j < size; j++) {
      tmp_keyp[j] = key_array[idxp[j] * num_values + (num_values - i - 1)];
    }
    radix_sort_desc(tmp_keyp, idxp, size, positive_only);
  }
  std::vector<K> tmp_ret(size * num_values);
  auto tmp_retp = tmp_ret.data();
  multivalue_extract(key_array, idxp, size, size, num_values, tmp_retp);
  for(size_t i = 0; i < size * num_values; i++) key_array[i] = tmp_retp[i];
}

template <class K, class V>
void multivalue_radix_sort(std::vector<K>& key_array, std::vector<V>& val_array,
                           int num_values, bool positive_only = false) {
  if(key_array.size() / num_values != val_array.size())
    throw std::runtime_error("multivalue_radix_sort: different size of arrays");
  if(key_array.size() % num_values != 0)
    throw std::runtime_error("multivalue_radix_sort: wrong num_values");
  multivalue_radix_sort(key_array.data(), val_array.data(),
                        key_array.size()/num_values,
                        num_values, positive_only);
}

template <class K>
void multivalue_radix_sort(std::vector<K>& key_array, int num_values,
                           bool positive_only = false) {
  if(key_array.size() % num_values != 0)
    throw std::runtime_error("multivalue_radix_sort: wrong num_values");
  multivalue_radix_sort(key_array.data(), key_array.size()/num_values,
                        num_values, positive_only);
}

template <class K, class V>
void multivalue_radix_sort_desc(std::vector<K>& key_array,
                                std::vector<V>& val_array,
                           int num_values, bool positive_only = false) {
  if(key_array.size() / num_values != val_array.size())
    throw std::runtime_error("multivalue_radix_sort: different size of arrays");
  if(key_array.size() % num_values != 0)
    throw std::runtime_error("multivalue_radix_sort: wrong num_values");
  multivalue_radix_sort_desc(key_array.data(), val_array.data(),
                             key_array.size()/num_values,
                             num_values, positive_only);
}

template <class K>
void multivalue_radix_sort_desc(std::vector<K>& key_array, int num_values,
                                bool positive_only = false) {
  if(key_array.size() % num_values != 0)
    throw std::runtime_error("multivalue_radix_sort: wrong num_values");
  multivalue_radix_sort_desc(key_array.data(), key_array.size()/num_values,
                             num_values, positive_only);
}

}
#endif
