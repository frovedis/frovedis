#ifndef HASHTABLE_HPP
#define HASHTABLE_HPP

#include <vector>
#include <utility>

#include "../core/set_operations.hpp"
#include "../core/radix_sort.hpp"
#include "../core/lower_bound.hpp"
#include "../core/vector_operations.hpp"

#if defined(_SX) || defined(__ve__) // might be used in x86
#define FIND_MISS_VLEN 1024
#define FIND_MISS_VLEN_EACH 256
#define FIND_MISS_THR 16
#define FIND_MISS_ALIGN_SIZE 128
#else
#define FIND_MISS_VLEN 4
#define FIND_MISS_VLEN_EACH 1
#define FIND_MISS_THR 1
#define FIND_MISS_ALIGN_SIZE 1
#endif
#define HASH_TABLE_SIZE_MULT 3

namespace frovedis {

/* used in dfcolumn.hpp */
template <class K>
inline
size_t myhash(const K& key, size_t size) {
  unsigned long ukey = reinterpret_cast<const unsigned long&>(key);
  return ukey % size;
}

// 32bit values are treated separately
template <>
inline
size_t myhash(const int& key, size_t size) {
  unsigned int ukey = reinterpret_cast<const unsigned int&>(key);
  return ukey % size;
}

template <>
inline
size_t myhash(const unsigned int& key, size_t size) {
  return key % size;
}

template <>
inline
size_t myhash(const float& key, size_t size) {
  unsigned int ukey = reinterpret_cast<const unsigned int&>(key);
  return ukey % size;
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
  std::vector<int> is_filled; // to allow key to accept any value
  std::vector<K> conflict_key; // sorted
  std::vector<V> conflict_val;
};

// should be 64bit
// http://d.hatena.ne.jp/zariganitosh/20090716/1247709137
// http://tools.m-bsys.com/calculators/prime_number_generator.php
inline std::vector<long long> prime_numbers() {
  std::vector<long long> ret = {
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
  return ret;
}

// modified version of find_condition
template <class K, class V>
std::vector<size_t> find_miss_write_table(const K* kp, const V* vp, size_t size,
                                          const K* table_keyp, V* table_valp, 
                                          const size_t* hashp) {
  if(size == 0) {
    return std::vector<size_t>();
  }
  std::vector<size_t> rettmp(size);
  auto rettmpp = rettmp.data();

  size_t each = size / FIND_MISS_VLEN; // maybe 0
  auto aligned_each = each * sizeof(K) / FIND_MISS_ALIGN_SIZE;
  if(aligned_each % 2 == 0 && aligned_each != 0)
    aligned_each -= 1;
  each = aligned_each * FIND_MISS_ALIGN_SIZE / sizeof(K);
  size_t rest = size - each * FIND_MISS_VLEN;

  size_t out_ridx[FIND_MISS_VLEN];
  size_t out_ridx_0[FIND_MISS_VLEN_EACH];
  size_t out_ridx_1[FIND_MISS_VLEN_EACH];
  size_t out_ridx_2[FIND_MISS_VLEN_EACH];
  size_t out_ridx_3[FIND_MISS_VLEN_EACH];
// never remove this vreg! this is needed following vovertake
#pragma _NEC vreg(out_ridx_0)
#pragma _NEC vreg(out_ridx_1)
#pragma _NEC vreg(out_ridx_2)
#pragma _NEC vreg(out_ridx_3)
  for(size_t i = 0; i < FIND_MISS_VLEN_EACH; i++) {
    auto v0 = i;
    auto v1 = i + FIND_MISS_VLEN_EACH;
    auto v2 = i + FIND_MISS_VLEN_EACH * 2;
    auto v3 = i + FIND_MISS_VLEN_EACH * 3;
    out_ridx_0[i] = each * v0;
    out_ridx_1[i] = each * v1;
    out_ridx_2[i] = each * v2;
    out_ridx_3[i] = each * v3;
  }

  if(each < FIND_MISS_THR) {
    size_t current = 0;
    for(size_t i = 0; i < size; i++) {
      if(kp[i] != table_keyp[hashp[i]]) {
        rettmpp[current++] = i;
      } else {
        table_valp[hashp[i]] = vp[i];
      }
    }
    std::vector<size_t> ret(current);
    auto retp = ret.data();
    for(size_t i = 0; i < current; i++) {
      retp[i] = rettmpp[i];
    }
    return ret;
  } else {
#pragma _NEC vob
    for(size_t j = 0; j < each; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
      for(size_t i = 0; i < FIND_MISS_VLEN_EACH; i++) {
        auto idx0 = j + each * i;
        auto idx1 = j + each * (i + FIND_MISS_VLEN_EACH);
        auto idx2 = j + each * (i + FIND_MISS_VLEN_EACH * 2);
        auto idx3 = j + each * (i + FIND_MISS_VLEN_EACH * 3);
        auto key0 = kp[idx0];
        auto key1 = kp[idx1];
        auto key2 = kp[idx2];
        auto key3 = kp[idx3];
        auto hash_idx0 = hashp[idx0];
        auto hash_idx1 = hashp[idx1];
        auto hash_idx2 = hashp[idx2];
        auto hash_idx3 = hashp[idx3];
        auto tab0 = table_keyp[hash_idx0];
        auto tab1 = table_keyp[hash_idx1];
        auto tab2 = table_keyp[hash_idx2];
        auto tab3 = table_keyp[hash_idx3];
        if(key0 != tab0) {
          rettmpp[out_ridx_0[i]++] = idx0;
        } else {
          table_valp[hash_idx0] = vp[idx0];
        }
        if(key1 != tab1) {
          rettmpp[out_ridx_1[i]++] = idx1;
        } else {
          table_valp[hash_idx1] = vp[idx1];
        }
        if(key2 != tab2) {
          rettmpp[out_ridx_2[i]++] = idx2;
        } else {
          table_valp[hash_idx2] = vp[idx2];
        }
        if(key3 != tab3) {
          rettmpp[out_ridx_3[i]++] = idx3;
        } else {
          table_valp[hash_idx3] = vp[idx3];
        }
      }
    }
    
    size_t rest_idx_start = each * FIND_MISS_VLEN;
    size_t rest_idx = rest_idx_start;
    if(rest != 0) {
      for(size_t j = 0; j < rest; j++) {
        if(kp[j + rest_idx_start] != table_keyp[hashp[j + rest_idx_start]]) {
          rettmpp[rest_idx++] = j + rest_idx_start;
        } else {
          table_valp[hashp[j + rest_idx_start]] = vp[j + rest_idx_start];
        }
      }
    }
    for(size_t i = 0; i < FIND_MISS_VLEN_EACH; i++) {
      auto v0 = i;
      auto v1 = i + FIND_MISS_VLEN_EACH;
      auto v2 = i + FIND_MISS_VLEN_EACH * 2;
      auto v3 = i + FIND_MISS_VLEN_EACH * 3;
      out_ridx[v0] = out_ridx_0[i];
      out_ridx[v1] = out_ridx_1[i];
      out_ridx[v2] = out_ridx_2[i];
      out_ridx[v3] = out_ridx_3[i];
    }
    size_t sizes[FIND_MISS_VLEN];
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - each * FIND_MISS_VLEN;
    total += rest_size;
    std::vector<size_t> ret(total);
    auto retp = ret.data();
    size_t current = 0;
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        retp[current + j] = rettmpp[each * i + j];
      }
      current += sizes[i];
    }
    for(size_t j = 0; j < rest_size; j++) {
      retp[current + j] = rettmpp[rest_idx_start + j];
    }
    return ret;
  }
}

template <class K, class V>
std::vector<size_t> find_miss_unique_check(const K* kp, const V* vp,
                                           size_t size,
                                           const K* table_keyp, V* table_valp, 
                                           const size_t* hashp,
                                           const size_t* unique_checkerp,
                                           int& is_unique_ok) {
  is_unique_ok = true;
  if(size == 0) {
    return std::vector<size_t>();
  }
  std::vector<size_t> rettmp(size);
  auto rettmpp = rettmp.data();

  size_t each = size / FIND_MISS_VLEN; // maybe 0
  auto aligned_each = each * sizeof(K) / FIND_MISS_ALIGN_SIZE;
  if(aligned_each % 2 == 0 && aligned_each != 0)
    aligned_each -= 1;
  each = aligned_each * FIND_MISS_ALIGN_SIZE / sizeof(K);
  size_t rest = size - each * FIND_MISS_VLEN;

  size_t out_ridx[FIND_MISS_VLEN];
  size_t out_ridx_0[FIND_MISS_VLEN_EACH];
  size_t out_ridx_1[FIND_MISS_VLEN_EACH];
  size_t out_ridx_2[FIND_MISS_VLEN_EACH];
  size_t out_ridx_3[FIND_MISS_VLEN_EACH];
// never remove this vreg! this is needed following vovertake
#pragma _NEC vreg(out_ridx_0)
#pragma _NEC vreg(out_ridx_1)
#pragma _NEC vreg(out_ridx_2)
#pragma _NEC vreg(out_ridx_3)
  for(size_t i = 0; i < FIND_MISS_VLEN_EACH; i++) {
    auto v0 = i;
    auto v1 = i + FIND_MISS_VLEN_EACH;
    auto v2 = i + FIND_MISS_VLEN_EACH * 2;
    auto v3 = i + FIND_MISS_VLEN_EACH * 3;
    out_ridx_0[i] = each * v0;
    out_ridx_1[i] = each * v1;
    out_ridx_2[i] = each * v2;
    out_ridx_3[i] = each * v3;
  }

  if(each < FIND_MISS_THR) {
    size_t current = 0;
    for(size_t i = 0; i < size; i++) {
      if(kp[i] != table_keyp[hashp[i]]) {
        rettmpp[current++] = i;
      } else if(unique_checkerp[hashp[i]] == i) {
        table_valp[hashp[i]] = vp[i];
      } else is_unique_ok = false;
    }
    std::vector<size_t> ret(current);
    auto retp = ret.data();
    for(size_t i = 0; i < current; i++) {
      retp[i] = rettmpp[i];
    }
    return ret;
  } else {
#pragma _NEC vob
    for(size_t j = 0; j < each; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
      for(size_t i = 0; i < FIND_MISS_VLEN_EACH; i++) {
        auto idx0 = j + each * i;
        auto idx1 = j + each * (i + FIND_MISS_VLEN_EACH);
        auto idx2 = j + each * (i + FIND_MISS_VLEN_EACH * 2);
        auto idx3 = j + each * (i + FIND_MISS_VLEN_EACH * 3);
        auto key0 = kp[idx0];
        auto key1 = kp[idx1];
        auto key2 = kp[idx2];
        auto key3 = kp[idx3];
        auto hash_idx0 = hashp[idx0];
        auto hash_idx1 = hashp[idx1];
        auto hash_idx2 = hashp[idx2];
        auto hash_idx3 = hashp[idx3];
        auto tab0 = table_keyp[hash_idx0];
        auto tab1 = table_keyp[hash_idx1];
        auto tab2 = table_keyp[hash_idx2];
        auto tab3 = table_keyp[hash_idx3];
        auto uniq0 = unique_checkerp[hash_idx0];
        auto uniq1 = unique_checkerp[hash_idx1];
        auto uniq2 = unique_checkerp[hash_idx2];
        auto uniq3 = unique_checkerp[hash_idx3];
        if(key0 != tab0) {
          rettmpp[out_ridx_0[i]++] = idx0;
        } else if(uniq0 == idx0) {
          table_valp[hash_idx0] = vp[idx0];
        } else is_unique_ok = false;
        if(key1 != tab1) {
          rettmpp[out_ridx_1[i]++] = idx1;
        } else if(uniq1 == idx1) {
          table_valp[hash_idx1] = vp[idx1];
        } else is_unique_ok = false;
        if(key2 != tab2) {
          rettmpp[out_ridx_2[i]++] = idx2;
        } else if(uniq2 == idx2) {
          table_valp[hash_idx2] = vp[idx2];
        } else is_unique_ok = false;
        if(key3 != tab3) {
          rettmpp[out_ridx_3[i]++] = idx3;
        } else if(uniq3 == idx3) {
          table_valp[hash_idx3] = vp[idx3];
        } else is_unique_ok = false;
      }
    }
    
    size_t rest_idx_start = each * FIND_MISS_VLEN;
    size_t rest_idx = rest_idx_start;
    if(rest != 0) {
      for(size_t j = 0; j < rest; j++) {
        if(kp[j + rest_idx_start] != table_keyp[hashp[j + rest_idx_start]]) {
          rettmpp[rest_idx++] = j + rest_idx_start;
        } else if (unique_checkerp[hashp[j + rest_idx_start]] ==
                   j + rest_idx_start) {
          table_valp[hashp[j + rest_idx_start]] = vp[j + rest_idx_start];
        } else is_unique_ok = false;
      }
    }
    for(size_t i = 0; i < FIND_MISS_VLEN_EACH; i++) {
      auto v0 = i;
      auto v1 = i + FIND_MISS_VLEN_EACH;
      auto v2 = i + FIND_MISS_VLEN_EACH * 2;
      auto v3 = i + FIND_MISS_VLEN_EACH * 3;
      out_ridx[v0] = out_ridx_0[i];
      out_ridx[v1] = out_ridx_1[i];
      out_ridx[v2] = out_ridx_2[i];
      out_ridx[v3] = out_ridx_3[i];
    }
    size_t sizes[FIND_MISS_VLEN];
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - each * FIND_MISS_VLEN;
    total += rest_size;
    std::vector<size_t> ret(total);
    auto retp = ret.data();
    size_t current = 0;
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        retp[current + j] = rettmpp[each * i + j];
      }
      current += sizes[i];
    }
    for(size_t j = 0; j < rest_size; j++) {
      retp[current + j] = rettmpp[rest_idx_start + j];
    }
    return ret;
  }
}

template <class K>
std::vector<size_t> find_miss(const K* kp, size_t size,
                              const K* table_keyp, const size_t* hashp) {
  if(size == 0) {
    return std::vector<size_t>();
  }
  std::vector<size_t> rettmp(size);
  auto rettmpp = rettmp.data();

  size_t each = size / FIND_MISS_VLEN; // maybe 0
  auto aligned_each = each * sizeof(K) / FIND_MISS_ALIGN_SIZE;
  if(aligned_each % 2 == 0 && aligned_each != 0)
    aligned_each -= 1;
  each = aligned_each * FIND_MISS_ALIGN_SIZE / sizeof(K);
  size_t rest = size - each * FIND_MISS_VLEN;

  size_t out_ridx[FIND_MISS_VLEN];
  size_t out_ridx_0[FIND_MISS_VLEN_EACH];
  size_t out_ridx_1[FIND_MISS_VLEN_EACH];
  size_t out_ridx_2[FIND_MISS_VLEN_EACH];
  size_t out_ridx_3[FIND_MISS_VLEN_EACH];
// never remove this vreg! this is needed following vovertake
#pragma _NEC vreg(out_ridx_0)
#pragma _NEC vreg(out_ridx_1)
#pragma _NEC vreg(out_ridx_2)
#pragma _NEC vreg(out_ridx_3)
  for(size_t i = 0; i < FIND_MISS_VLEN_EACH; i++) {
    auto v0 = i;
    auto v1 = i + FIND_MISS_VLEN_EACH;
    auto v2 = i + FIND_MISS_VLEN_EACH * 2;
    auto v3 = i + FIND_MISS_VLEN_EACH * 3;
    out_ridx_0[i] = each * v0;
    out_ridx_1[i] = each * v1;
    out_ridx_2[i] = each * v2;
    out_ridx_3[i] = each * v3;
  }

  if(each < FIND_MISS_THR) {
    size_t current = 0;
    for(size_t i = 0; i < size; i++) {
      if(kp[i] != table_keyp[hashp[i]]) {
        rettmpp[current++] = i;
      } 
    }
    std::vector<size_t> ret(current);
    auto retp = ret.data();
    for(size_t i = 0; i < current; i++) {
      retp[i] = rettmpp[i];
    }
    return ret;
  } else {
#pragma _NEC vob
    for(size_t j = 0; j < each; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
      for(size_t i = 0; i < FIND_MISS_VLEN_EACH; i++) {
        auto idx0 = j + each * i;
        auto idx1 = j + each * (i + FIND_MISS_VLEN_EACH);
        auto idx2 = j + each * (i + FIND_MISS_VLEN_EACH * 2);
        auto idx3 = j + each * (i + FIND_MISS_VLEN_EACH * 3);
        auto key0 = kp[idx0];
        auto key1 = kp[idx1];
        auto key2 = kp[idx2];
        auto key3 = kp[idx3];
        auto hash_idx0 = hashp[idx0];
        auto hash_idx1 = hashp[idx1];
        auto hash_idx2 = hashp[idx2];
        auto hash_idx3 = hashp[idx3];
        auto tab0 = table_keyp[hash_idx0];
        auto tab1 = table_keyp[hash_idx1];
        auto tab2 = table_keyp[hash_idx2];
        auto tab3 = table_keyp[hash_idx3];
        if(key0 != tab0) {
          rettmpp[out_ridx_0[i]++] = idx0;
        }
        if(key1 != tab1) {
          rettmpp[out_ridx_1[i]++] = idx1;
        }
        if(key2 != tab2) {
          rettmpp[out_ridx_2[i]++] = idx2;
        }
        if(key3 != tab3) {
          rettmpp[out_ridx_3[i]++] = idx3;
        }
      }
    }
    
    size_t rest_idx_start = each * FIND_MISS_VLEN;
    size_t rest_idx = rest_idx_start;
    if(rest != 0) {
      for(size_t j = 0; j < rest; j++) {
        if(kp[j + rest_idx_start] != table_keyp[hashp[j + rest_idx_start]]) {
          rettmpp[rest_idx++] = j + rest_idx_start;
        }
      }
    }
    for(size_t i = 0; i < FIND_MISS_VLEN_EACH; i++) {
      auto v0 = i;
      auto v1 = i + FIND_MISS_VLEN_EACH;
      auto v2 = i + FIND_MISS_VLEN_EACH * 2;
      auto v3 = i + FIND_MISS_VLEN_EACH * 3;
      out_ridx[v0] = out_ridx_0[i];
      out_ridx[v1] = out_ridx_1[i];
      out_ridx[v2] = out_ridx_2[i];
      out_ridx[v3] = out_ridx_3[i];
    }
    size_t sizes[FIND_MISS_VLEN];
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - each * FIND_MISS_VLEN;
    total += rest_size;
    std::vector<size_t> ret(total);
    auto retp = ret.data();
    size_t current = 0;
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        retp[current + j] = rettmpp[each * i + j];
      }
      current += sizes[i];
    }
    for(size_t j = 0; j < rest_size; j++) {
      retp[current + j] = rettmpp[rest_idx_start + j];
    }
    return ret;
  }
}

template <class K, class V>
std::vector<size_t> find_miss_read_table(const K* kp, V* vp, size_t size,
                                         const K* table_keyp,
                                         const V* table_valp, 
                                         const size_t* hashp) {
  if(size == 0) {
    return std::vector<size_t>();
  }
  std::vector<size_t> rettmp(size);
  auto rettmpp = rettmp.data();

  size_t each = size / FIND_MISS_VLEN; // maybe 0
  auto aligned_each = each * sizeof(K) / FIND_MISS_ALIGN_SIZE;
  if(aligned_each % 2 == 0 && aligned_each != 0)
    aligned_each -= 1;
  each = aligned_each * FIND_MISS_ALIGN_SIZE / sizeof(K);
  size_t rest = size - each * FIND_MISS_VLEN;

  size_t out_ridx[FIND_MISS_VLEN];
  size_t out_ridx_0[FIND_MISS_VLEN_EACH];
  size_t out_ridx_1[FIND_MISS_VLEN_EACH];
  size_t out_ridx_2[FIND_MISS_VLEN_EACH];
  size_t out_ridx_3[FIND_MISS_VLEN_EACH];
// never remove this vreg! this is needed following vovertake
#pragma _NEC vreg(out_ridx_0)
#pragma _NEC vreg(out_ridx_1)
#pragma _NEC vreg(out_ridx_2)
#pragma _NEC vreg(out_ridx_3)
  for(size_t i = 0; i < FIND_MISS_VLEN_EACH; i++) {
    auto v0 = i;
    auto v1 = i + FIND_MISS_VLEN_EACH;
    auto v2 = i + FIND_MISS_VLEN_EACH * 2;
    auto v3 = i + FIND_MISS_VLEN_EACH * 3;
    out_ridx_0[i] = each * v0;
    out_ridx_1[i] = each * v1;
    out_ridx_2[i] = each * v2;
    out_ridx_3[i] = each * v3;
  }

  if(each < FIND_MISS_THR) {
    size_t current = 0;
    for(size_t i = 0; i < size; i++) {
      if(kp[i] != table_keyp[hashp[i]]) {
        rettmpp[current++] = i;
      } else {
        vp[i] = table_valp[hashp[i]];
      }
    }
    std::vector<size_t> ret(current);
    auto retp = ret.data();
    for(size_t i = 0; i < current; i++) {
      retp[i] = rettmpp[i];
    }
    return ret;
  } else {
#pragma _NEC vob
    for(size_t j = 0; j < each; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
      for(size_t i = 0; i < FIND_MISS_VLEN_EACH; i++) {
        auto idx0 = j + each * i;
        auto idx1 = j + each * (i + FIND_MISS_VLEN_EACH);
        auto idx2 = j + each * (i + FIND_MISS_VLEN_EACH * 2);
        auto idx3 = j + each * (i + FIND_MISS_VLEN_EACH * 3);
        auto key0 = kp[idx0];
        auto key1 = kp[idx1];
        auto key2 = kp[idx2];
        auto key3 = kp[idx3];
        auto hash_idx0 = hashp[idx0];
        auto hash_idx1 = hashp[idx1];
        auto hash_idx2 = hashp[idx2];
        auto hash_idx3 = hashp[idx3];
        auto tab0 = table_keyp[hash_idx0];
        auto tab1 = table_keyp[hash_idx1];
        auto tab2 = table_keyp[hash_idx2];
        auto tab3 = table_keyp[hash_idx3];
        if(key0 != tab0) {
          rettmpp[out_ridx_0[i]++] = idx0;
        } else {
          vp[idx0] = table_valp[hash_idx0];
        }
        if(key1 != tab1) {
          rettmpp[out_ridx_1[i]++] = idx1;
        } else {
          vp[idx1] = table_valp[hash_idx1];
        }
        if(key2 != tab2) {
          rettmpp[out_ridx_2[i]++] = idx2;
        } else {
          vp[idx2] = table_valp[hash_idx2];
        }
        if(key3 != tab3) {
          rettmpp[out_ridx_3[i]++] = idx3;
        } else {
          vp[idx3] = table_valp[hash_idx3];
        }
      }
    }
    
    size_t rest_idx_start = each * FIND_MISS_VLEN;
    size_t rest_idx = rest_idx_start;
    if(rest != 0) {
      for(size_t j = 0; j < rest; j++) {
        if(kp[j + rest_idx_start] != table_keyp[hashp[j + rest_idx_start]]) {
          rettmpp[rest_idx++] = j + rest_idx_start;
        } else {
          vp[j + rest_idx_start] = table_valp[hashp[j + rest_idx_start]];
        }
      }
    }
    for(size_t i = 0; i < FIND_MISS_VLEN_EACH; i++) {
      auto v0 = i;
      auto v1 = i + FIND_MISS_VLEN_EACH;
      auto v2 = i + FIND_MISS_VLEN_EACH * 2;
      auto v3 = i + FIND_MISS_VLEN_EACH * 3;
      out_ridx[v0] = out_ridx_0[i];
      out_ridx[v1] = out_ridx_1[i];
      out_ridx[v2] = out_ridx_2[i];
      out_ridx[v3] = out_ridx_3[i];
    }
    size_t sizes[FIND_MISS_VLEN];
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - each * FIND_MISS_VLEN;
    total += rest_size;
    std::vector<size_t> ret(total);
    auto retp = ret.data();
    size_t current = 0;
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        retp[current + j] = rettmpp[each * i + j];
      }
      current += sizes[i];
    }
    for(size_t j = 0; j < rest_size; j++) {
      retp[current + j] = rettmpp[rest_idx_start + j];
    }
    return ret;
  }
}

template <class K, class V>
std::vector<size_t>
find_miss_read_table_checkfill(const K* kp, V* vp, size_t size,
                               const K* table_keyp, const V* table_valp,
                               const int* is_filledp, const size_t* hashp,
                               std::vector<size_t>& clear_miss) {
  if(size == 0) {
    clear_miss.resize(0);
    return std::vector<size_t>();
  }
  std::vector<size_t> rettmp(size);
  auto rettmpp = rettmp.data();
  std::vector<size_t> clear_miss_tmp(size);
  auto clear_miss_tmpp = clear_miss_tmp.data();

  size_t each = size / FIND_MISS_VLEN; // maybe 0
  auto aligned_each = each * sizeof(K) / FIND_MISS_ALIGN_SIZE;
  if(aligned_each % 2 == 0 && aligned_each != 0)
    aligned_each -= 1;
  each = aligned_each * FIND_MISS_ALIGN_SIZE / sizeof(K);
  size_t rest = size - each * FIND_MISS_VLEN;

  size_t out_ridx[FIND_MISS_VLEN];
  size_t out_ridx_0[FIND_MISS_VLEN_EACH];
  size_t out_ridx_1[FIND_MISS_VLEN_EACH];
  size_t out_ridx_2[FIND_MISS_VLEN_EACH];
  size_t out_ridx_3[FIND_MISS_VLEN_EACH];
// never remove this vreg! this is needed following vovertake
#pragma _NEC vreg(out_ridx_0)
#pragma _NEC vreg(out_ridx_1)
#pragma _NEC vreg(out_ridx_2)
#pragma _NEC vreg(out_ridx_3)

  size_t out2_ridx[FIND_MISS_VLEN];
  size_t out2_ridx_0[FIND_MISS_VLEN_EACH];
  size_t out2_ridx_1[FIND_MISS_VLEN_EACH];
  size_t out2_ridx_2[FIND_MISS_VLEN_EACH];
  size_t out2_ridx_3[FIND_MISS_VLEN_EACH];
// never remove this vreg! this is needed following vovertake
#pragma _NEC vreg(out2_ridx_0)
#pragma _NEC vreg(out2_ridx_1)
#pragma _NEC vreg(out2_ridx_2)
#pragma _NEC vreg(out2_ridx_3)

  for(size_t i = 0; i < FIND_MISS_VLEN_EACH; i++) {
    auto v0 = i;
    auto v1 = i + FIND_MISS_VLEN_EACH;
    auto v2 = i + FIND_MISS_VLEN_EACH * 2;
    auto v3 = i + FIND_MISS_VLEN_EACH * 3;
    out_ridx_0[i] = each * v0;
    out_ridx_1[i] = each * v1;
    out_ridx_2[i] = each * v2;
    out_ridx_3[i] = each * v3;
    out2_ridx_0[i] = each * v0;
    out2_ridx_1[i] = each * v1;
    out2_ridx_2[i] = each * v2;
    out2_ridx_3[i] = each * v3;
  }

  if(each < FIND_MISS_THR) {
    size_t current = 0;
    size_t current_clear_miss = 0;
    for(size_t i = 0; i < size; i++) {
      if(!is_filledp[hashp[i]]) {
        clear_miss_tmpp[current_clear_miss++] = i;
      } else if(kp[i] != table_keyp[hashp[i]]) {
        rettmpp[current++] = i;
      } else {
        vp[i] = table_valp[hashp[i]];
      }
    }

    clear_miss.resize(current_clear_miss);
    auto clear_missp = clear_miss.data();
    for(size_t i = 0; i < current_clear_miss; i++) {
      clear_missp[i] = clear_miss_tmpp[i];
    }
    std::vector<size_t> ret(current);
    auto retp = ret.data();
    for(size_t i = 0; i < current; i++) {
      retp[i] = rettmpp[i];
    }

    return ret;
  } else {

#pragma _NEC vob
    for(size_t j = 0; j < each; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
      for(size_t i = 0; i < FIND_MISS_VLEN_EACH; i++) {
        auto idx0 = j + each * i;
        auto idx1 = j + each * (i + FIND_MISS_VLEN_EACH);
        auto idx2 = j + each * (i + FIND_MISS_VLEN_EACH * 2);
        auto idx3 = j + each * (i + FIND_MISS_VLEN_EACH * 3);
        auto key0 = kp[idx0];
        auto key1 = kp[idx1];
        auto key2 = kp[idx2];
        auto key3 = kp[idx3];
        auto hash_idx0 = hashp[idx0];
        auto hash_idx1 = hashp[idx1];
        auto hash_idx2 = hashp[idx2];
        auto hash_idx3 = hashp[idx3];
        auto tab0 = table_keyp[hash_idx0];
        auto tab1 = table_keyp[hash_idx1];
        auto tab2 = table_keyp[hash_idx2];
        auto tab3 = table_keyp[hash_idx3];
        auto isfill0 = is_filledp[hash_idx0];
        auto isfill1 = is_filledp[hash_idx1];
        auto isfill2 = is_filledp[hash_idx2];
        auto isfill3 = is_filledp[hash_idx3];
        if(!isfill0) {
          clear_miss_tmpp[out2_ridx_0[i]++] = idx0;
        } else if(key0 != tab0) {
          rettmpp[out_ridx_0[i]++] = idx0;
        } else {
          vp[idx0] = table_valp[hash_idx0];
        }
        if(!isfill1) {
          clear_miss_tmpp[out2_ridx_1[i]++] = idx1;
        } else if(key1 != tab1) {
          rettmpp[out_ridx_1[i]++] = idx1;
        } else {
          vp[idx1] = table_valp[hash_idx1];
        }
        if(!isfill2) {
          clear_miss_tmpp[out2_ridx_2[i]++] = idx2;
        } else if(key2 != tab2) {
          rettmpp[out_ridx_2[i]++] = idx2;
        } else {
          vp[idx2] = table_valp[hash_idx2];
        }
        if(!isfill3) {
          clear_miss_tmpp[out2_ridx_3[i]++] = idx3;
        } else if(key3 != tab3) {
          rettmpp[out_ridx_3[i]++] = idx3;
        } else {
          vp[idx3] = table_valp[hash_idx3];
        }
      }
    }
    
    size_t rest_idx_start = each * FIND_MISS_VLEN;
    size_t rest_idx = rest_idx_start;
    size_t rest_idx2 = rest_idx_start;
    if(rest != 0) {
      for(size_t j = 0; j < rest; j++) {
        if (!is_filledp[hashp[j + rest_idx_start]]) {
          clear_miss_tmpp[rest_idx2++] = j + rest_idx_start;
        } else if(kp[j + rest_idx_start] !=
                  table_keyp[hashp[j + rest_idx_start]]) {
          rettmpp[rest_idx++] = j + rest_idx_start;
        } else {
          vp[j + rest_idx_start] = table_valp[hashp[j + rest_idx_start]];
        }
      }
    }
    for(size_t i = 0; i < FIND_MISS_VLEN_EACH; i++) {
      auto v0 = i;
      auto v1 = i + FIND_MISS_VLEN_EACH;
      auto v2 = i + FIND_MISS_VLEN_EACH * 2;
      auto v3 = i + FIND_MISS_VLEN_EACH * 3;
      out_ridx[v0] = out_ridx_0[i];
      out_ridx[v1] = out_ridx_1[i];
      out_ridx[v2] = out_ridx_2[i];
      out_ridx[v3] = out_ridx_3[i];
      out2_ridx[v0] = out2_ridx_0[i];
      out2_ridx[v1] = out2_ridx_1[i];
      out2_ridx[v2] = out2_ridx_2[i];
      out2_ridx[v3] = out2_ridx_3[i];
    }
    size_t sizes[FIND_MISS_VLEN];
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - each * FIND_MISS_VLEN;
    total += rest_size;
    std::vector<size_t> ret(total);
    auto retp = ret.data();
    size_t current = 0;
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        retp[current + j] = rettmpp[each * i + j];
      }
      current += sizes[i];
    }
    for(size_t j = 0; j < rest_size; j++) {
      retp[current + j] = rettmpp[rest_idx_start + j];
    }

    size_t sizes2[FIND_MISS_VLEN];
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      sizes2[i] = out2_ridx[i] - each * i;
    }
    size_t total2 = 0;
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      total2 += sizes2[i];
    }
    size_t rest_size2 = rest_idx2 - each * FIND_MISS_VLEN;
    total2 += rest_size2;
    clear_miss.resize(total2);
    auto clear_missp = clear_miss.data();
    size_t current2 = 0;
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      for(size_t j = 0; j < sizes2[i]; j++) {
        clear_missp[current2 + j] = clear_miss_tmpp[each * i + j];
      }
      current2 += sizes2[i];
    }
    for(size_t j = 0; j < rest_size2; j++) {
      clear_missp[current2 + j] = clear_miss_tmpp[rest_idx_start + j];
    }

    return ret;
  }
}

template <class K>
std::vector<size_t>
find_miss_checkfill(const K* kp, size_t size, const K* table_keyp, 
                    const int* is_filledp, const size_t* hashp,
                    std::vector<size_t>& clear_miss) {
  if(size == 0) {
    clear_miss.resize(0);
    return std::vector<size_t>();
  }
  std::vector<size_t> rettmp(size);
  auto rettmpp = rettmp.data();
  std::vector<size_t> clear_miss_tmp(size);
  auto clear_miss_tmpp = clear_miss_tmp.data();

  size_t each = size / FIND_MISS_VLEN; // maybe 0
  auto aligned_each = each * sizeof(K) / FIND_MISS_ALIGN_SIZE;
  if(aligned_each % 2 == 0 && aligned_each != 0)
    aligned_each -= 1;
  each = aligned_each * FIND_MISS_ALIGN_SIZE / sizeof(K);
  size_t rest = size - each * FIND_MISS_VLEN;

  size_t out_ridx[FIND_MISS_VLEN];
  size_t out_ridx_0[FIND_MISS_VLEN_EACH];
  size_t out_ridx_1[FIND_MISS_VLEN_EACH];
  size_t out_ridx_2[FIND_MISS_VLEN_EACH];
  size_t out_ridx_3[FIND_MISS_VLEN_EACH];
// never remove this vreg! this is needed following vovertake
#pragma _NEC vreg(out_ridx_0)
#pragma _NEC vreg(out_ridx_1)
#pragma _NEC vreg(out_ridx_2)
#pragma _NEC vreg(out_ridx_3)

  size_t out2_ridx[FIND_MISS_VLEN];
  size_t out2_ridx_0[FIND_MISS_VLEN_EACH];
  size_t out2_ridx_1[FIND_MISS_VLEN_EACH];
  size_t out2_ridx_2[FIND_MISS_VLEN_EACH];
  size_t out2_ridx_3[FIND_MISS_VLEN_EACH];
// never remove this vreg! this is needed following vovertake
#pragma _NEC vreg(out2_ridx_0)
#pragma _NEC vreg(out2_ridx_1)
#pragma _NEC vreg(out2_ridx_2)
#pragma _NEC vreg(out2_ridx_3)

  for(size_t i = 0; i < FIND_MISS_VLEN_EACH; i++) {
    auto v0 = i;
    auto v1 = i + FIND_MISS_VLEN_EACH;
    auto v2 = i + FIND_MISS_VLEN_EACH * 2;
    auto v3 = i + FIND_MISS_VLEN_EACH * 3;
    out_ridx_0[i] = each * v0;
    out_ridx_1[i] = each * v1;
    out_ridx_2[i] = each * v2;
    out_ridx_3[i] = each * v3;
    out2_ridx_0[i] = each * v0;
    out2_ridx_1[i] = each * v1;
    out2_ridx_2[i] = each * v2;
    out2_ridx_3[i] = each * v3;
  }

  if(each < FIND_MISS_THR) {
    size_t current = 0;
    size_t current_clear_miss = 0;
    for(size_t i = 0; i < size; i++) {
      if(!is_filledp[hashp[i]]) {
        clear_miss_tmpp[current_clear_miss++] = i;
      } else if(kp[i] != table_keyp[hashp[i]]) {
        rettmpp[current++] = i;
      }
    }

    clear_miss.resize(current_clear_miss);
    auto clear_missp = clear_miss.data();
    for(size_t i = 0; i < current_clear_miss; i++) {
      clear_missp[i] = clear_miss_tmpp[i];
    }
    std::vector<size_t> ret(current);
    auto retp = ret.data();
    for(size_t i = 0; i < current; i++) {
      retp[i] = rettmpp[i];
    }

    return ret;
  } else {

#pragma _NEC vob
    for(size_t j = 0; j < each; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
      for(size_t i = 0; i < FIND_MISS_VLEN_EACH; i++) {
        auto idx0 = j + each * i;
        auto idx1 = j + each * (i + FIND_MISS_VLEN_EACH);
        auto idx2 = j + each * (i + FIND_MISS_VLEN_EACH * 2);
        auto idx3 = j + each * (i + FIND_MISS_VLEN_EACH * 3);
        auto key0 = kp[idx0];
        auto key1 = kp[idx1];
        auto key2 = kp[idx2];
        auto key3 = kp[idx3];
        auto hash_idx0 = hashp[idx0];
        auto hash_idx1 = hashp[idx1];
        auto hash_idx2 = hashp[idx2];
        auto hash_idx3 = hashp[idx3];
        auto tab0 = table_keyp[hash_idx0];
        auto tab1 = table_keyp[hash_idx1];
        auto tab2 = table_keyp[hash_idx2];
        auto tab3 = table_keyp[hash_idx3];
        auto isfill0 = is_filledp[hash_idx0];
        auto isfill1 = is_filledp[hash_idx1];
        auto isfill2 = is_filledp[hash_idx2];
        auto isfill3 = is_filledp[hash_idx3];
        if(!isfill0) {
          clear_miss_tmpp[out2_ridx_0[i]++] = idx0;
        } else if(key0 != tab0) {
          rettmpp[out_ridx_0[i]++] = idx0;
        }
        if(!isfill1) {
          clear_miss_tmpp[out2_ridx_1[i]++] = idx1;
        } else if(key1 != tab1) {
          rettmpp[out_ridx_1[i]++] = idx1;
        }
        if(!isfill2) {
          clear_miss_tmpp[out2_ridx_2[i]++] = idx2;
        } else if(key2 != tab2) {
          rettmpp[out_ridx_2[i]++] = idx2;
        }
        if(!isfill3) {
          clear_miss_tmpp[out2_ridx_3[i]++] = idx3;
        } else if(key3 != tab3) {
          rettmpp[out_ridx_3[i]++] = idx3;
        }
      }
    }
    
    size_t rest_idx_start = each * FIND_MISS_VLEN;
    size_t rest_idx = rest_idx_start;
    size_t rest_idx2 = rest_idx_start;
    if(rest != 0) {
      for(size_t j = 0; j < rest; j++) {
        if (!is_filledp[hashp[j + rest_idx_start]]) {
          clear_miss_tmpp[rest_idx2++] = j + rest_idx_start;
        } else if(kp[j + rest_idx_start] !=
                  table_keyp[hashp[j + rest_idx_start]]) {
          rettmpp[rest_idx++] = j + rest_idx_start;
        }
      }
    }
    for(size_t i = 0; i < FIND_MISS_VLEN_EACH; i++) {
      auto v0 = i;
      auto v1 = i + FIND_MISS_VLEN_EACH;
      auto v2 = i + FIND_MISS_VLEN_EACH * 2;
      auto v3 = i + FIND_MISS_VLEN_EACH * 3;
      out_ridx[v0] = out_ridx_0[i];
      out_ridx[v1] = out_ridx_1[i];
      out_ridx[v2] = out_ridx_2[i];
      out_ridx[v3] = out_ridx_3[i];
      out2_ridx[v0] = out2_ridx_0[i];
      out2_ridx[v1] = out2_ridx_1[i];
      out2_ridx[v2] = out2_ridx_2[i];
      out2_ridx[v3] = out2_ridx_3[i];
    }
    size_t sizes[FIND_MISS_VLEN];
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - each * FIND_MISS_VLEN;
    total += rest_size;
    std::vector<size_t> ret(total);
    auto retp = ret.data();
    size_t current = 0;
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        retp[current + j] = rettmpp[each * i + j];
      }
      current += sizes[i];
    }
    for(size_t j = 0; j < rest_size; j++) {
      retp[current + j] = rettmpp[rest_idx_start + j];
    }

    size_t sizes2[FIND_MISS_VLEN];
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      sizes2[i] = out2_ridx[i] - each * i;
    }
    size_t total2 = 0;
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      total2 += sizes2[i];
    }
    size_t rest_size2 = rest_idx2 - each * FIND_MISS_VLEN;
    total2 += rest_size2;
    clear_miss.resize(total2);
    auto clear_missp = clear_miss.data();
    size_t current2 = 0;
    for(size_t i = 0; i < FIND_MISS_VLEN; i++) {
      for(size_t j = 0; j < sizes2[i]; j++) {
        clear_missp[current2 + j] = clear_miss_tmpp[each * i + j];
      }
      current2 += sizes2[i];
    }
    for(size_t j = 0; j < rest_size2; j++) {
      clear_missp[current2 + j] = clear_miss_tmpp[rest_idx_start + j];
    }

    return ret;
  }
}

template <class K, class V>
unique_hashtable<K,V>::unique_hashtable(const std::vector<K>& k,
                                        const std::vector<V>& v) {
  size_t size = k.size();
  if(v.size() != size)
    throw std::runtime_error("sizes of key and value are different");
  auto primes = prime_numbers();
  size_t target_table_size = size * HASH_TABLE_SIZE_MULT;
  auto table_size_cand_it =
    std::lower_bound(primes.begin(), primes.end(), target_table_size);
  if(table_size_cand_it == primes.end())
    throw std::runtime_error("unique_hash_table: requested size too large");
  size_t table_size = *table_size_cand_it;
  key.resize(table_size);
  val.resize(table_size);
  is_filled.resize(table_size);
  const K* keyp = k.data();
  const V* valp = v.data();
  K* table_keyp = key.data();
  V* table_valp = val.data();
  int* is_filledp = is_filled.data();
  std::vector<size_t> hash(size);
  auto hashp = hash.data();

  if(sizeof(K) == 4) {
    const uint32_t* keyp_hash = reinterpret_cast<const uint32_t*>(keyp);
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < size; i++) {
      size_t hashval = keyp_hash[i] % table_size;
      table_keyp[hashval] = keyp[i];
      is_filledp[hashval] = true;
      hashp[i] = hashval;
    }
  } else if (sizeof(K) == 8) {
    const uint64_t* keyp_hash = reinterpret_cast<const uint64_t*>(keyp);
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < size; i++) {
      size_t hashval = keyp_hash[i] % table_size;
      table_keyp[hashval] = keyp[i];
      is_filledp[hashval] = true;
      hashp[i] = hashval;
    }
  } else {
    throw std::runtime_error("unique_hashtable only supports key whose size is 4 or 8");
  }
  auto missed = find_miss_write_table(keyp, valp, size, table_keyp,
                                      table_valp, hashp);
  auto missed_size = missed.size();
  if(missed_size > 0) {
    RLOG(DEBUG) << "missed = " << missed_size << "/" << size
                << ", table_size = " << table_size << std::endl;
    conflict_key.resize(missed_size);
    conflict_val.resize(missed_size);
    auto conflict_keyp = conflict_key.data();
    auto conflict_valp = conflict_val.data();
    auto missedp = missed.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < missed_size; i++) {
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
  auto primes = prime_numbers();
  size_t target_table_size = size * HASH_TABLE_SIZE_MULT;
  auto table_size_cand_it =
    std::lower_bound(primes.begin(), primes.end(), target_table_size);
  if(table_size_cand_it == primes.end())
    throw std::runtime_error("unique_hash_table: requested size too large");
  size_t table_size = *table_size_cand_it;
  key.resize(table_size);
  val.resize(table_size);
  is_filled.resize(table_size);
  const K* keyp = k.data();
  const V* valp = v.data();
  K* table_keyp = key.data();
  V* table_valp = val.data();
  int* is_filledp = is_filled.data();
  std::vector<size_t> hash(size);
  auto hashp = hash.data();

  is_unique_ok = true;
  std::vector<size_t> unique_checker(table_size,
                                     std::numeric_limits<size_t>::max());
  size_t* unique_checkerp = unique_checker.data();

  if(sizeof(K) == 4) {
    const uint32_t* keyp_hash = reinterpret_cast<const uint32_t*>(keyp);
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < size; i++) {
      size_t hashval = keyp_hash[i] % table_size;
      table_keyp[hashval] = keyp[i];
      is_filledp[hashval] = true;
      unique_checkerp[hashval] = i;
      hashp[i] = hashval;
    }
  } else if (sizeof(K) == 8) {
    const uint64_t* keyp_hash = reinterpret_cast<const uint64_t*>(keyp);
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < size; i++) {
      size_t hashval = keyp_hash[i] % table_size;
      table_keyp[hashval] = keyp[i];
      is_filledp[hashval] = true;
      unique_checkerp[hashval] = i;
      hashp[i] = hashval;
    }
  } else {
    throw std::runtime_error("unique_hashtable only supports key whose size is 4 or 8");
  }

  auto missed = find_miss_unique_check(keyp, valp, size, table_keyp,
                                       table_valp, hashp, unique_checkerp,
                                       is_unique_ok);
  auto missed_size = missed.size();
  if(missed_size > 0) {
    RLOG(DEBUG) << "missed = " << missed_size << "/" << size
                << ", table_size = " << table_size << std::endl;
    conflict_key.resize(missed_size);
    conflict_val.resize(missed_size);
    auto conflict_keyp = conflict_key.data();
    auto conflict_valp = conflict_val.data();
    auto missedp = missed.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < missed_size; i++) {
      conflict_keyp[i] = keyp[missedp[i]];
      conflict_valp[i] = valp[missedp[i]];
    }
    radix_sort(conflict_key, conflict_val);
    // we need to check uniqueness again, because missed values might have 
    // duplicated values, which is not captured in find_miss_unique_check!
    if(!set_is_unique(conflict_key)) is_unique_ok = false;
  } 
}

// for set, accept duplicated keys
template <class K, class V>
unique_hashtable<K,V>::unique_hashtable(const std::vector<K>& k) {
  size_t size = k.size();
  auto primes = prime_numbers();
  size_t target_table_size = size * HASH_TABLE_SIZE_MULT;
  auto table_size_cand_it =
    std::lower_bound(primes.begin(), primes.end(), target_table_size);
  if(table_size_cand_it == primes.end())
    throw std::runtime_error("unique_hash_table: requested size too large");
  size_t table_size = *table_size_cand_it;
  key.resize(table_size);
  is_filled.resize(table_size);
  const K* keyp = k.data();
  K* table_keyp = key.data();
  int* is_filledp = is_filled.data();
  std::vector<size_t> hash(size);
  auto hashp = hash.data();

  if(sizeof(K) == 4) {
    const uint32_t* keyp_hash = reinterpret_cast<const uint32_t*>(keyp);
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < size; i++) {
      size_t hashval = keyp_hash[i] % table_size;
      table_keyp[hashval] = keyp[i];
      is_filledp[hashval] = true;
      hashp[i] = hashval;
    }
  } else if (sizeof(K) == 8) {
    const uint64_t* keyp_hash = reinterpret_cast<const uint64_t*>(keyp);
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < size; i++) {
      size_t hashval = keyp_hash[i] % table_size;
      table_keyp[hashval] = keyp[i];
      is_filledp[hashval] = true;
      hashp[i] = hashval;
    }
  } else {
    throw std::runtime_error("unique_hashtable only supports key whose size is 4 or 8");
  }
  auto missed = find_miss(keyp, size, table_keyp, hashp);
  auto missed_size = missed.size();
  if(missed_size > 0) {
    RLOG(DEBUG) << "missed = " << missed_size << "/" << size
                << ", table_size = " << table_size << std::endl;
    conflict_key.resize(missed_size);
    auto conflict_keyp = conflict_key.data();
    auto missedp = missed.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < missed_size; i++) {
      conflict_keyp[i] = keyp[missedp[i]];
    }
    radix_sort(conflict_key);
  } 
}

template <class K, class V> 
std::vector<V> unique_hashtable<K,V>::lookup(const std::vector<K>& k) {
  size_t size = k.size();
  size_t table_size = key.size();
  std::vector<V> v(size);
  const K* keyp = k.data();
  V* valp = v.data();
  K* table_keyp = key.data();
  V* table_valp = val.data();
  std::vector<size_t> hash(size);
  auto hashp = hash.data();

  if(sizeof(K) == 4) {
    const uint32_t* keyp_hash = reinterpret_cast<const uint32_t*>(keyp);
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < size; i++) {
      hashp[i] = keyp_hash[i] % table_size;
    }
  } else if (sizeof(K) == 8) {
    const uint64_t* keyp_hash = reinterpret_cast<const uint64_t*>(keyp);
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < size; i++) {
      hashp[i] = keyp_hash[i] % table_size;
    }
  } 
  auto missed = find_miss_read_table(keyp, valp, size, table_keyp,
                                     table_valp, hashp);

  auto missed_size = missed.size();
  if(missed_size > 0) {
    std::vector<K> missed_key(missed_size);
    K* missed_keyp = missed_key.data();
    auto missedp = missed.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < missed_size; i++) {
      missed_keyp[i] = keyp[missedp[i]];
    }
    auto missed_val_idx = lower_bound(conflict_key, missed_key);
    size_t* missed_val_idxp = missed_val_idx.data();
    V* conflict_valp = conflict_val.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < missed_size; i++) {
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
  const K* keyp = k.data();
  V* valp = v.data();
  K* table_keyp = key.data();
  V* table_valp = val.data();
  std::vector<size_t> hash(size);
  auto hashp = hash.data();

  if(sizeof(K) == 4) {
    const uint32_t* keyp_hash = reinterpret_cast<const uint32_t*>(keyp);
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < size; i++) {
      hashp[i] = keyp_hash[i] % table_size;
    }
  } else if (sizeof(K) == 8) {
    const uint64_t* keyp_hash = reinterpret_cast<const uint64_t*>(keyp);
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < size; i++) {
      hashp[i] = keyp_hash[i] % table_size;
    }
  }
  auto is_filledp = is_filled.data();
  std::vector<size_t> clear_miss;
  auto missed = find_miss_read_table_checkfill(keyp, valp, size, table_keyp,
                                               table_valp, is_filledp, hashp,
                                               clear_miss);

  auto missed_size = missed.size();
  if(missed_size > 0) {
    std::vector<K> retmisstmp(missed_size);
    auto retmisstmpp = retmisstmp.data();
    std::vector<K> missed_key(missed_size);
    K* missed_keyp = missed_key.data();
    auto missedp = missed.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < missed_size; i++) {
      missed_keyp[i] = keyp[missedp[i]];
    }
    auto missed_val_idx = lower_bound(conflict_key, missed_key);
    auto missed_val_idxp = missed_val_idx.data();
    auto conflict_valp = conflict_val.data();
    auto conflict_val_size = conflict_val.size();
    size_t crntpos = 0;
    auto conflict_keyp = conflict_key.data();
    for(size_t i = 0; i < missed_size; i++) {
      if(missed_val_idxp[i] < conflict_val_size &&
         conflict_keyp[missed_val_idxp[i]] == missed_keyp[i]) {
        valp[missedp[i]] = conflict_valp[missed_val_idxp[i]];
      } else {
        // slow, but shouldn't be that often...
        retmisstmpp[crntpos++] = missedp[i]; 
      }
    }
    std::vector<size_t> retmisstmp2(crntpos);
    auto retmisstmp2p = retmisstmp2.data();
    for(size_t i = 0; i < crntpos; i++) retmisstmp2p[i] = retmisstmpp[i];
    retmiss = set_union(clear_miss, retmisstmp2);
  } else {
    retmiss.swap(clear_miss);
  }
  
  return v;
}

template <class K, class V> 
std::vector<int>
unique_hashtable<K,V>::check_existence(const std::vector<K>& k) {
  size_t size = k.size();
  size_t table_size = key.size();
  std::vector<V> v(size);
  const K* keyp = k.data();
  K* table_keyp = key.data();
  std::vector<size_t> hash(size);
  auto hashp = hash.data();

  if(sizeof(K) == 4) {
    const uint32_t* keyp_hash = reinterpret_cast<const uint32_t*>(keyp);
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < size; i++) {
      hashp[i] = keyp_hash[i] % table_size;
    }
  } else if (sizeof(K) == 8) {
    const uint64_t* keyp_hash = reinterpret_cast<const uint64_t*>(keyp);
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < size; i++) {
      hashp[i] = keyp_hash[i] % table_size;
    }
  }
  auto is_filledp = is_filled.data();
  std::vector<size_t> clear_miss;
  auto missed = find_miss_checkfill(keyp, size, table_keyp, is_filledp, hashp,
                                    clear_miss);
  auto missed_size = missed.size();
  std::vector<int> ret(size, 1);
  auto retp = ret.data();
  auto clear_miss_size = clear_miss.size();
  auto clear_missp = clear_miss.data();
  for(size_t i = 0; i < clear_miss_size; i++) {
    retp[clear_missp[i]] = 0;
  }
  if(missed_size > 0) {
    std::vector<K> missed_key(missed_size);
    auto missed_keyp = missed_key.data();
    auto missedp = missed.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < missed_size; i++) {
      missed_keyp[i] = keyp[missedp[i]];
    }
    auto missed_val_idx = lower_bound(conflict_key, missed_key);
    auto missed_val_idxp = missed_val_idx.data();
    auto conflict_val_size = conflict_val.size();
    auto conflict_keyp = conflict_key.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < missed_size; i++) {
      if(missed_val_idxp[i] == conflict_val_size ||
         conflict_keyp[missed_val_idxp[i]] != missed_keyp[i]) {
        retp[missedp[i]] = 0;
      }
    }
  }
  return ret;
}

template <class K, class V> 
std::vector<K>
unique_hashtable<K,V>::all_keys() {
  auto hit = vector_find_one(is_filled);
  auto hit_size = hit.size();
  auto conflict_key_size = conflict_key.size();
  auto ret_size = hit_size + conflict_key_size;
  std::vector<K> ret(ret_size);
  auto retp = ret.data();
  auto keyp = key.data();
  auto hitp = hit.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < hit_size; i++) {
    retp[i] = keyp[hitp[i]];
  }
  auto restretp = retp + hit_size;
  auto conflict_keyp = conflict_key.data();
  for(size_t i = 0; i < conflict_key_size; i++) {
    restretp[i] = conflict_keyp[i];
  }
  return ret;
}

}
#endif
