#ifndef HASHTABLE_HPP
#define HASHTABLE_HPP

#include <vector>
#include <utility>

#include "set_operations.hpp"
#include "../core/radix_sort.hpp"

#if defined(_SX) || defined(__ve__) // might be used in x86
#define UNIQUE_HASH_VLEN 256
#define VECTOR_BINARY_SEARCH_VLEN 256
#else
#define UNIQUE_HASH_VLEN 1
#define VECTOR_BINARY_SEARCH_VLEN 1
#endif
#define HASH_TABLE_SIZE_MULT 3

namespace frovedis {

// Originally written by Haoran
// different from std::lower_bound, always returns idx that is inside of 
// the array. Need to check if the returned idx points to the same value!
template <class T>
std::vector<size_t> vector_binary_search(std::vector<T>& sorted,
                                         std::vector<T>& values) { 
  size_t num_values = values.size(); // to search
  size_t num_elem = sorted.size();

  if(num_values == 0) return std::vector<size_t>();
  if(num_elem == 0)
    throw std::runtime_error("zero sized vector is not supported");

  std::vector<size_t> ret(num_values);
  //range identifier for each value
    
  std::vector<size_t> left(VECTOR_BINARY_SEARCH_VLEN); 
  std::vector<size_t> right(VECTOR_BINARY_SEARCH_VLEN);
  std::vector<int> is_finished(VECTOR_BINARY_SEARCH_VLEN);
  auto* leftp = &left[0];
  auto* rightp = &right[0];
  auto* sortedp = &sorted[0];
  auto* is_finishedp = &is_finished[0];

  size_t block_size = num_values / VECTOR_BINARY_SEARCH_VLEN;
  size_t remain = num_values % VECTOR_BINARY_SEARCH_VLEN;

  for(size_t i = 0; i < block_size + 1; i++) {
    size_t offset = i * VECTOR_BINARY_SEARCH_VLEN;
    auto* retp = &ret[0] + offset;
    auto* valuesp = &values[0] + offset;
    size_t len = i < block_size ? VECTOR_BINARY_SEARCH_VLEN : remain;
    for(size_t j = 0; j < len; j++) {
      leftp[j] = 0;
      rightp[j] = num_elem - 1;
      is_finishedp[j] = 0;
    }
    bool is_finished_all = 0;
    while(is_finished_all == 0){
      for(size_t j = 0; j < len; j++){
        if(is_finishedp[j] == 0 && leftp[j] != rightp[j]) {
          size_t mid = (leftp[j] + rightp[j])/2;
          if(valuesp[j] < sortedp[mid]) {
            rightp[j] = mid;
          } else if(valuesp[j] > sortedp[mid]) {
            leftp[j] = mid + 1;
          } else { // equal
            retp[j] = mid;
            is_finishedp[j] = 1;
          }
        } else if(is_finishedp[j] == 0 && leftp[j] == rightp[j]) {
          retp[j] = leftp[j];
          is_finishedp[j] = 1;
        }          
      }
      is_finished_all = 1;
      for(size_t k = 0; k < len; k++){
        if(is_finishedp[k] == 0) {
          is_finished_all = 0;
          break;
        }
      }
    }
  }
  return ret;
}

/*
  To avoid the BUG(?) of SX compiler, casted to int
 */
template <class K>
inline
size_t myhash(K key, size_t size) {
  int ukey = static_cast<unsigned int>(key) & 0x7FFFFFFF;
  int usize = size;
  return ukey % usize;
}

template <>
inline
size_t myhash(unsigned long long key, size_t size) {
  // used mainly for global index... 26 is for supporting 512 nodes
  unsigned int upper = (key >> 26) & 0x7FFFFFFF;
  unsigned int lower = key & 0x7FFFFFFF;
  int ukey = (upper + lower) & 0x7FFFFFFF;;
  int usize = size;
  return ukey % usize;
}

template <>
inline
size_t myhash(unsigned long key, size_t size) {
  unsigned int upper = (key >> 26) & 0x7FFFFFFF;
  unsigned int lower = key & 0x7FFFFFFF;
  int ukey = (upper + lower) & 0x7FFFFFFF;;
  int usize = size;
  return ukey % usize;
}

// TODO: create non SX version? (used directly in dfcolumn.hpp)
template <class K, class V>
class unique_hashtable {
public:
  unique_hashtable(){} // required to return from map
  unique_hashtable(const std::vector<K>& k, const std::vector<V>& v);
  // for creating set (used hash join), accept duplicated keys
  unique_hashtable(const std::vector<K>& k); 
  unique_hashtable(const std::vector<K>& k, const std::vector<V>& v,
                   int& is_unique_ok); // check uniqueness
  std::vector<K> all_keys(); // used for hash_join
  std::vector<V> lookup(const std::vector<K>& k);
  std::vector<V> lookup(const std::vector<K>& k, std::vector<size_t>& miss);
  std::vector<int> check_existence(const std::vector<K>& k);
  void debug_print() {
    std::cout << "key: ";
    for(size_t i = 0; i < key.size(); i++) {
      std::cout << key[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "val: ";
    for(size_t i = 0; i < val.size(); i++) {
      std::cout << val[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "is_filled: ";
    for(size_t i = 0; i < is_filled.size(); i++) {
      std::cout << is_filled[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "conflict_key: ";
    for(size_t i = 0; i < conflict_key.size(); i++) {
      std::cout << conflict_key[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "conflict_val: ";
    for(size_t i = 0; i < conflict_val.size(); i++) {
      std::cout << conflict_val[i] << " ";
    }
    std::cout << std::endl;
  }
  void clear() { // to save memory
    std::vector<K> key_tmp; key_tmp.swap(key);
    std::vector<V> val_tmp; val_tmp.swap(val);
    std::vector<int> is_filled_tmp; is_filled_tmp.swap(is_filled);
    std::vector<K> conflict_key_tmp; conflict_key_tmp.swap(conflict_key);
    std::vector<V> conflict_val_tmp; conflict_val_tmp.swap(conflict_val);
  }
private:
  std::vector<K> key;
  std::vector<V> val;
  std::vector<int> is_filled;
  std::vector<K> conflict_key; // sorted
  std::vector<V> conflict_val;
};

template <class K, class V>
unique_hashtable<K,V>::unique_hashtable(const std::vector<K>& k,
                                        const std::vector<V>& v) {
  size_t size = k.size();
  if(v.size() != size)
    throw std::runtime_error("sizes of key and value are different");
  // http://d.hatena.ne.jp/zariganitosh/20090716/1247709137
  // http://tools.m-bsys.com/calculators/prime_number_generator.php
  long long primes[] = { // should be 64bit
    8 + 3,
    16 + 3,
    32 + 5,
    64 + 3,
    128 + 3,
    256 + 27,
    512 + 9,
    1024 + 9,
    2048 + 5,
    4096 + 3,
    8192 + 27,
    16384 + 43,
    32768 + 3,
    65536 + 45,
    131072 + 29,
    262144 + 3,
    524288 + 21,
    1048576 + 7,
    2097152 + 17,
    4194304 + 15,
    8388608 + 9,
    16777216 + 43,
    33554432 + 35,
    67108864 + 15,
    134217728 + 29,
    268435456 + 3,
    536870912 + 11,
    1073741824 + 85,
    2147483648 + 11,
    4294967296 + 15,
    8589934592 + 17,
    17179869184 + 25,
    34359738368 + 53, // 32GB, primes + 32
  };
  size_t target_table_size = size * HASH_TABLE_SIZE_MULT;
  auto* table_size_candp =
    std::lower_bound(primes, primes + 33, target_table_size);
  if(table_size_candp == primes + 33)
    throw std::runtime_error("unique_hash_table: requested size too large");
  size_t table_size = *table_size_candp;
  key.resize(table_size);
  val.resize(table_size);
  is_filled.resize(table_size);
  const K* keyp = &k[0];
  const V* valp = &v[0];
  K* table_keyp = &key[0];
  V* table_valp = &val[0];
  std::vector<size_t> missed(size);
  size_t* missedp = &missed[0];
  size_t missed_idx = 0;
  for(size_t i = 0; i < table_size; i++) is_filled[i] = false;
  int* is_filledp = &is_filled[0];
  size_t hash[UNIQUE_HASH_VLEN];
#pragma _NEC vreg(hash)

  size_t block_size = size / UNIQUE_HASH_VLEN;
  size_t remain_size = size % UNIQUE_HASH_VLEN;
  for(size_t b = 0; b < block_size; b++) {
    size_t offset = b * UNIQUE_HASH_VLEN;
    auto keyoff = keyp + offset;
    auto valoff = valp + offset;
    for(size_t i = 0; i < UNIQUE_HASH_VLEN; i++) {
      hash[i] = myhash(keyoff[i], table_size);
    }    
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t i = 0; i < UNIQUE_HASH_VLEN; i++) {
      if(is_filledp[hash[i]] == false) {
        table_keyp[hash[i]] = keyoff[i];
        is_filledp[hash[i]] = true;
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t i = 0; i < UNIQUE_HASH_VLEN; i++) {
      if(table_keyp[hash[i]] == keyoff[i]) {
        table_valp[hash[i]] = valoff[i];
      } else {
        missedp[missed_idx++] = i + offset;
      }
    }
  }
  size_t offset = block_size * UNIQUE_HASH_VLEN;
  auto keyoff = keyp + offset;
  auto valoff = valp + offset;
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < remain_size; i++) {
    hash[i] = myhash(keyoff[i], table_size);
  }    
  for(size_t i = 0; i < remain_size; i++) {
    if(is_filledp[hash[i]] == false) {
      table_keyp[hash[i]] = keyoff[i];
      is_filledp[hash[i]] = true;
    }
  }
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < remain_size; i++) {
    if(table_keyp[hash[i]] == keyoff[i]) {
      table_valp[hash[i]] = valoff[i];
    } else {
      missedp[missed_idx++] = i + offset;      
    }
  }
  if(missed_idx > 0) {
    RLOG(DEBUG) << "missed = " << missed_idx << "/" << size
                << ", table_size = " << table_size << std::endl;
    conflict_key.resize(missed_idx);
    conflict_val.resize(missed_idx);
    K* conflict_keyp = &conflict_key[0];
    V* conflict_valp = &conflict_val[0];
    for(size_t i = 0; i < missed_idx; i++) {
      conflict_keyp[i] = keyp[missedp[i]];
      conflict_valp[i] = valp[missedp[i]];
    }
    radix_sort(conflict_key, conflict_val);
  }
}

// If duplicated keys are passed, is_uniqe_ok is set to false
// and the result is incorrect
template <class K, class V>
unique_hashtable<K,V>::unique_hashtable(const std::vector<K>& k,
                                        const std::vector<V>& v,
                                        int& is_unique_ok) {
  size_t size = k.size();
  if(v.size() != size)
    throw std::runtime_error("sizes of key and value are different");
  // http://d.hatena.ne.jp/zariganitosh/20090716/1247709137
  // http://tools.m-bsys.com/calculators/prime_number_generator.php
  long long primes[] = { // should be 64bit
    8 + 3,
    16 + 3,
    32 + 5,
    64 + 3,
    128 + 3,
    256 + 27,
    512 + 9,
    1024 + 9,
    2048 + 5,
    4096 + 3,
    8192 + 27,
    16384 + 43,
    32768 + 3,
    65536 + 45,
    131072 + 29,
    262144 + 3,
    524288 + 21,
    1048576 + 7,
    2097152 + 17,
    4194304 + 15,
    8388608 + 9,
    16777216 + 43,
    33554432 + 35,
    67108864 + 15,
    134217728 + 29,
    268435456 + 3,
    536870912 + 11,
    1073741824 + 85,
    2147483648 + 11,
    4294967296 + 15,
    8589934592 + 17,
    17179869184 + 25,
    34359738368 + 53, // 32GB, primes + 32
  };
  size_t target_table_size = size * HASH_TABLE_SIZE_MULT;
  auto* table_size_candp =
    std::lower_bound(primes, primes + 33, target_table_size);
  if(table_size_candp == primes + 33)
    throw std::runtime_error("unique_hash_table: requested size too large");
  size_t table_size = *table_size_candp;
  key.resize(table_size);
  val.resize(table_size);
  is_filled.resize(table_size);
  const K* keyp = &k[0];
  const V* valp = &v[0];
  K* table_keyp = &key[0];
  V* table_valp = &val[0];
  std::vector<size_t> missed(size);
  size_t* missedp = &missed[0];
  size_t missed_idx = 0;
  for(size_t i = 0; i < table_size; i++) is_filled[i] = false;
  int* is_filledp = &is_filled[0];
  size_t hash[UNIQUE_HASH_VLEN];
#pragma _NEC vreg(hash)
  is_unique_ok = true;
  std::vector<size_t> unique_checker(table_size,
                                     std::numeric_limits<size_t>::max());
  size_t* unique_checkerp = &unique_checker[0];

  size_t block_size = size / UNIQUE_HASH_VLEN;
  size_t remain_size = size % UNIQUE_HASH_VLEN;
  for(size_t b = 0; b < block_size; b++) {
    size_t offset = b * UNIQUE_HASH_VLEN;
    auto keyoff = keyp + offset;
    auto valoff = valp + offset;
    for(size_t i = 0; i < UNIQUE_HASH_VLEN; i++) {
      hash[i] = myhash(keyoff[i], table_size);
    }    
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t i = 0; i < UNIQUE_HASH_VLEN; i++) {
      if(is_filledp[hash[i]] == false) {
        table_keyp[hash[i]] = keyoff[i];
        is_filledp[hash[i]] = true;
        unique_checkerp[hash[i]] = i;
      } else if(table_keyp[hash[i]] == keyoff[i]) {
        is_unique_ok = false;
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t i = 0; i < UNIQUE_HASH_VLEN; i++) {
      if(unique_checkerp[hash[i]] == i) {
        table_valp[hash[i]] = valoff[i];
      } else if(table_keyp[hash[i]] != keyoff[i]) {
        missedp[missed_idx++] = i + offset;
      } else {
        is_unique_ok = false;
      }
    }
    if(is_unique_ok == false) return;
  }
  size_t offset = block_size * UNIQUE_HASH_VLEN;
  auto keyoff = keyp + offset;
  auto valoff = valp + offset;
  for(size_t i = 0; i < remain_size; i++) {
    hash[i] = myhash(keyoff[i], table_size);
  }    
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < remain_size; i++) {
    if(is_filledp[hash[i]] == false) {
      table_keyp[hash[i]] = keyoff[i];
      is_filledp[hash[i]] = true;
      unique_checkerp[hash[i]] = i;
    } else if(table_keyp[hash[i]] == keyoff[i]) {
      is_unique_ok = false;
    }
  }
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < remain_size; i++) {
    if(unique_checkerp[hash[i]] == i) {
      table_valp[hash[i]] = valoff[i];
    } else if(table_keyp[hash[i]] != keyoff[i]) {
      missedp[missed_idx++] = i + offset;
    } else {
      is_unique_ok = false;
    }
  }
  if(is_unique_ok == false) return;
  if(missed_idx > 0) {
    RLOG(DEBUG) << "missed = " << missed_idx << "/" << size
                << ", table_size = " << table_size << std::endl;
    conflict_key.resize(missed_idx);
    conflict_val.resize(missed_idx);
    K* conflict_keyp = &conflict_key[0];
    V* conflict_valp = &conflict_val[0];
    for(size_t i = 0; i < missed_idx; i++) {
      conflict_keyp[i] = keyp[missedp[i]];
      conflict_valp[i] = valp[missedp[i]];
    }
    radix_sort(conflict_key, conflict_val);
  }
}

// for set, accept duplicated keys
template <class K, class V>
unique_hashtable<K,V>::unique_hashtable(const std::vector<K>& k) {
  size_t size = k.size();
  long long primes[] = { // should be 64bit
    8 + 3,
    16 + 3,
    32 + 5,
    64 + 3,
    128 + 3,
    256 + 27,
    512 + 9,
    1024 + 9,
    2048 + 5,
    4096 + 3,
    8192 + 27,
    16384 + 43,
    32768 + 3,
    65536 + 45,
    131072 + 29,
    262144 + 3,
    524288 + 21,
    1048576 + 7,
    2097152 + 17,
    4194304 + 15,
    8388608 + 9,
    16777216 + 43,
    33554432 + 35,
    67108864 + 15,
    134217728 + 29,
    268435456 + 3,
    536870912 + 11,
    1073741824 + 85,
    2147483648 + 11,
    4294967296 + 15,
    8589934592 + 17,
    17179869184 + 25,
    34359738368 + 53, // 32GB, primes + 32
  };
  size_t target_table_size = size * HASH_TABLE_SIZE_MULT;
  auto* table_size_candp =
    std::lower_bound(primes, primes + 33, target_table_size);
  if(table_size_candp == primes + 33)
    throw std::runtime_error("unique_hash_table: requested size too large");
  size_t table_size = *table_size_candp;
  key.resize(table_size);
  is_filled.resize(table_size);
  const K* keyp = &k[0];
  K* table_keyp = &key[0];
  std::vector<size_t> missed(size);
  size_t* missedp = &missed[0];
  size_t missed_idx = 0;
  for(size_t i = 0; i < table_size; i++) is_filled[i] = false;
  int* is_filledp = &is_filled[0];
  size_t hash[UNIQUE_HASH_VLEN];
#pragma _NEC vreg(hash)

  size_t block_size = size / UNIQUE_HASH_VLEN;
  size_t remain_size = size % UNIQUE_HASH_VLEN;
  for(size_t b = 0; b < block_size; b++) {
    size_t offset = b * UNIQUE_HASH_VLEN;
    auto keyoff = keyp + offset;
    for(size_t i = 0; i < UNIQUE_HASH_VLEN; i++) {
      hash[i] = myhash(keyoff[i], table_size);
    }    
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t i = 0; i < UNIQUE_HASH_VLEN; i++) {
      if(is_filledp[hash[i]] == false) {
        table_keyp[hash[i]] = keyoff[i];
        is_filledp[hash[i]] = true;
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t i = 0; i < UNIQUE_HASH_VLEN; i++) {
      if(table_keyp[hash[i]] == keyoff[i]) {
        ;
      } else {
        missedp[missed_idx++] = i + offset;
      }
    }
  }
  size_t offset = block_size * UNIQUE_HASH_VLEN;
  auto keyoff = keyp + offset;
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < remain_size; i++) {
    hash[i] = myhash(keyoff[i], table_size);
  }    
  for(size_t i = 0; i < remain_size; i++) {
    if(is_filledp[hash[i]] == false) {
      table_keyp[hash[i]] = keyoff[i];
      is_filledp[hash[i]] = true;
    }
  }
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < remain_size; i++) {
    if(table_keyp[hash[i]] != keyoff[i]) {
      missedp[missed_idx++] = i + offset;      
    }
  }
  if(missed_idx > 0) {
    RLOG(DEBUG) << "(set) missed = " << missed_idx << "/" << size
                << ", table_size = " << table_size << std::endl;
    conflict_key.resize(missed_idx);
    conflict_val.resize(missed_idx); // dummy
    K* conflict_keyp = &conflict_key[0];
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t i = 0; i < missed_idx; i++) {
      conflict_keyp[i] = keyp[missedp[i]];
    }
    radix_sort(conflict_key, conflict_val);
    auto tmp = set_unique(conflict_key);
    tmp.swap(conflict_key);
  }
}

template <class K, class V> 
std::vector<V> unique_hashtable<K,V>::lookup(const std::vector<K>& k) {
  size_t size = k.size();
  size_t table_size = key.size();
  std::vector<V> v(size);
  const K* keyp = &k[0];
  V* valp = &v[0];
  K* table_keyp = &key[0];
  V* table_valp = &val[0];
  std::vector<size_t> missed(size);
  size_t* missedp = &missed[0];
  size_t missed_idx = 0;
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < size; i++) {
    size_t hash = myhash(keyp[i], table_size);
    if(table_keyp[hash] == keyp[i]) {
      valp[i] = table_valp[hash];
    } else {
      missedp[missed_idx++] = i;      
    }
  }
  if(missed_idx > 0) {
    std::vector<K> missed_key(missed_idx);
    K* missed_keyp = &missed_key[0];
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t i = 0; i < missed_idx; i++) {
      missed_keyp[i] = keyp[missedp[i]];
    }
    auto missed_val_idx = vector_binary_search(conflict_key, missed_key);
    size_t* missed_val_idxp = &missed_val_idx[0];
    V* conflict_valp = &conflict_val[0];
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t i = 0; i < missed_idx; i++) {
      valp[missedp[i]] = conflict_valp[missed_val_idxp[i]];
    }
  }
  return v;
}

template <class K, class V> 
std::vector<V> unique_hashtable<K,V>::lookup(const std::vector<K>& k,
                                             std::vector<size_t>& retmiss) {
  size_t size = k.size();
  size_t table_size = key.size();
  std::vector<V> v(size);
  const K* keyp = &k[0];
  V* valp = &v[0];
  K* table_keyp = &key[0];
  V* table_valp = &val[0];
  std::vector<size_t> missed(size);
  size_t* missedp = &missed[0];
  size_t missed_idx = 0;
  int* is_filledp = &is_filled[0];
  std::vector<size_t> misstmp0(size);
  size_t* misstmp0p = &misstmp0[0];
  size_t misstmp0_idx = 0;
  std::vector<size_t> misstmp1(size);
  size_t* misstmp1p = &misstmp1[0];
  size_t misstmp1_idx = 0;
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < size; i++) {
    size_t hash = myhash(keyp[i], table_size);
    if(is_filledp[hash]) {
      if(table_keyp[hash] == keyp[i]) {
        valp[i] = table_valp[hash];
      } else missedp[missed_idx++] = i; // check conflict
      // need to use different array for vectorization
    } else misstmp0p[misstmp0_idx++] = i; // clearly miss
  }
  if(missed_idx > 0) {
    if(conflict_key.size() > 0) {
      std::vector<K> missed_key(missed_idx);
      K* missed_keyp = &missed_key[0];
#pragma cdir nodep
#pragma _NEC ivdep
      for(size_t i = 0; i < missed_idx; i++) {
        missed_keyp[i] = keyp[missedp[i]];
      }
      auto missed_val_idx = vector_binary_search(conflict_key, missed_key);
      size_t* missed_val_idxp = &missed_val_idx[0];
      V* conflict_valp = &conflict_val[0];
      K* conflict_keyp = &conflict_key[0];
#pragma cdir nodep
#pragma _NEC ivdep
      for(size_t i = 0; i < missed_idx; i++) {
        if(conflict_keyp[missed_val_idxp[i]] == missed_keyp[i])
          valp[missedp[i]] = conflict_valp[missed_val_idxp[i]];
        else
          misstmp1p[misstmp1_idx++] = missedp[i];
      }
    } else { // not conflict; all of them are miss
      for(size_t i = 0; i < missed_idx; i++) {
        misstmp1p[misstmp1_idx++] = missedp[i];
      }
    }
    misstmp0.resize(misstmp0_idx);
    misstmp1.resize(misstmp1_idx);
    retmiss = set_union(misstmp0, misstmp1);
  } else {
    misstmp0.resize(misstmp0_idx);
    retmiss.swap(misstmp0);
  }
  return v;
}

template <class K, class V> 
std::vector<int>
unique_hashtable<K,V>::check_existence(const std::vector<K>& k) {
  size_t size = k.size();
  size_t table_size = key.size();
  std::vector<int> ret(size);
  const K* keyp = &k[0];
  V* retp = &ret[0];
  K* table_keyp = &key[0];
  std::vector<size_t> missed(size);
  size_t* missedp = &missed[0];
  size_t missed_idx = 0;
  int* is_filledp = &is_filled[0];
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < size; i++) {
    size_t hash = myhash(keyp[i], table_size);
    if(is_filledp[hash]) {
      if(table_keyp[hash] == keyp[i]) {
        retp[i] = 1;
      } else {
        missedp[missed_idx++] = i;      
      }
    }
  }
  
  if(missed_idx > 0) {
    if(conflict_key.size() > 0) {
      std::vector<K> missed_key(missed_idx);
      K* missed_keyp = &missed_key[0];
#pragma cdir nodep
#pragma _NEC ivdep
      for(size_t i = 0; i < missed_idx; i++) {
        missed_keyp[i] = keyp[missedp[i]];
      }
      auto missed_val_idx = vector_binary_search(conflict_key, missed_key);
      size_t* missed_val_idxp = &missed_val_idx[0];
      K* conflict_keyp = &conflict_key[0];
#pragma cdir nodep
#pragma _NEC ivdep
      for(size_t i = 0; i < missed_idx; i++) {
        if(conflict_keyp[missed_val_idxp[i]] == missed_keyp[i])
          retp[missedp[i]] = 1;
      }
    } // else not conflict; no need to check
  }
  return ret;
}

template <class K, class V> 
std::vector<K>
unique_hashtable<K,V>::all_keys() {
  int* is_filledp = is_filled.data();
  size_t size = key.size();
  if(size == 0) {
    return std::vector<K>();
  }
  std::vector<K> keytmp(size);
  K* keytmpp = keytmp.data();
  K* keyp = key.data();
  
  size_t each = size / UNIQUE_HASH_VLEN; // maybe 0
  if(each % 2 == 0 && each > 1) each--;
  size_t rest = size - each * UNIQUE_HASH_VLEN;
  size_t out_ridx[UNIQUE_HASH_VLEN];
// never remove this vreg! this is needed folowing vovertake
// though this prevents ftrace...
#pragma _NEC vreg(out_ridx)
  for(size_t i = 0; i < UNIQUE_HASH_VLEN; i++) {
    out_ridx[i] = each * i;
  }
  if(each == 0) {
    size_t current = 0;
    for(size_t i = 0; i < size; i++) {
      if(is_filledp[i]) {
        keytmpp[current] = keyp[i];
        current++;
      }
    }
    std::vector<K> ret(current);
    K* retp = ret.data();
    for(size_t i = 0; i < current; i++) {
      retp[i] = keytmpp[i];
    }
    return ret;
  } else {
#pragma _NEC vob
    for(size_t j = 0; j < each; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
      for(size_t i = 0; i < UNIQUE_HASH_VLEN; i++) {
        auto loaded_is_filled = is_filledp[j + each * i];
        if(loaded_is_filled) {
          keytmpp[out_ridx[i]] = keyp[j + each * i];
          out_ridx[i]++;
        }
      }
    }
    size_t rest_idx_start = each * UNIQUE_HASH_VLEN;
    size_t rest_idx = rest_idx_start;
    if(rest != 0) {
      for(size_t j = 0; j < rest; j++) {
        auto loaded_is_filled = is_filledp[j + rest_idx_start]; 
        if(loaded_is_filled != 0) {
          keytmpp[rest_idx] = keyp[j + rest_idx_start];
          rest_idx++;
        }
      }
    }
    size_t sizes[UNIQUE_HASH_VLEN];
    for(size_t i = 0; i < UNIQUE_HASH_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < UNIQUE_HASH_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - each * UNIQUE_HASH_VLEN;
    total += rest_size;
    size_t conflict_key_size = conflict_key.size();
    total += conflict_key_size;

    std::vector<K> ret(total);
    K* retp = ret.data();
    size_t current = 0;
    for(size_t i = 0; i < UNIQUE_HASH_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        retp[current + j] = keytmpp[each * i + j];
      }
      current += sizes[i];
    }
    for(size_t j = 0; j < rest_size; j++) {
      retp[current + j] = keytmpp[rest_idx_start + j];
    }
    current += rest_size;
    K* conflict_keyp = &conflict_key[0];
    for(size_t i = 0; i < conflict_key_size; i++) {
      retp[current + i] = conflict_keyp[i];
    }

    return ret;
  }
}

}
#endif
