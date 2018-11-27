#ifndef RADIX_SORT_HPP
#define RADIX_SORT_HPP

#include <vector>
#include <stdint.h>

#include "prefix_sum.hpp"

#define RADIX_SORT_VLEN 256

namespace frovedis {

// supported K is int type, and only 0 or positive data
template <class K, class V>
void radix_sort(K* key_array, V* val_array, size_t size) {
  size_t key_size = sizeof(K);
  size_t bucket_ldim = RADIX_SORT_VLEN + 1;
  size_t num_bucket = 1 << 8; // 8bit == 256
  // bucket_table is columnar (VLEN + 1) by num_bucket matrix
  // "1" is to avoid bank conflict, but reused for "rest" of the data
  std::vector<size_t> bucket_table(num_bucket * bucket_ldim);
  std::vector<size_t> px_bucket_table(num_bucket * bucket_ldim);
  size_t* bucket_tablep = &bucket_table[0];
  size_t* px_bucket_tablep = &px_bucket_table[0];

  K max = 0;
  for(size_t i = 0; i < size; i++) {
    if(key_array[i] > max) max = key_array[i];
  }

  size_t max_key_size = 0;
  size_t tmp = 0xFF;
  for(size_t i = 1; i < key_size + 1; i++) {
    if(max < tmp) {
      max_key_size = i;
      break;
    } else {
      tmp = (tmp << 8) | 0xFF;
    }
  }

  std::vector<size_t> pos(size);
  size_t* posp = &pos[0];
  std::vector<K> key_array_tmpv(size);
  std::vector<V> val_array_tmpv(size);
  K* key_array_tmp = &key_array_tmpv[0];
  V* val_array_tmp = &val_array_tmpv[0];
  int next_is_tmp = 1;
  size_t block_size = size / RADIX_SORT_VLEN;
  if(block_size % 2 == 0 && block_size != 0) block_size -= 1;
  size_t rest = size - RADIX_SORT_VLEN * block_size;
  for(size_t d = 1; d <= max_key_size; d++) { // d: digit
    for(size_t i = 0; i < bucket_table.size(); i++) bucket_tablep[i] = 0;
    K *key_src, *key_dst;
    V *val_src, *val_dst;
    if(next_is_tmp) {
      key_src = key_array; key_dst = key_array_tmp;
      val_src = val_array; val_dst = val_array_tmp;
    } else {
      key_src = key_array_tmp; key_dst = key_array;
      val_src = val_array_tmp; val_dst = val_array;
    }
    for(size_t b = 0; b < block_size; b++) { // b: block
      // these loops are sparated to improve vectorization
#pragma cdir nodep
#pragma _NEC ivdep
      for(int v = 0; v < RADIX_SORT_VLEN; v++) { // vector loop, loop raking
        auto key = key_src[block_size * v + b];
        int bucket = (key >> (d - 1) * 8) & 0xFF;
        posp[block_size * v + b] = bucket_tablep[bucket_ldim * bucket + v];
      }
#pragma cdir nodep
#pragma _NEC ivdep
      for(int v = 0; v < RADIX_SORT_VLEN; v++) { // vector loop, loop raking
        auto key = key_src[block_size * v + b];
        int bucket = (key >> (d - 1) * 8) & 0xFF;
        bucket_tablep[bucket_ldim * bucket + v]++;
      }
    }
    size_t v = RADIX_SORT_VLEN;
    for(int b = 0; b < rest; b++) { // not vector loop
      auto key = key_src[block_size * v + b];
      int bucket = (key >> (d - 1) * 8) & 0xFF;
      posp[block_size * v + b] = bucket_tablep[bucket_ldim * bucket + v];
      bucket_tablep[bucket_ldim * bucket + v]++;
    }
    // preparing for the copy
    prefix_sum(bucket_tablep, px_bucket_tablep + 1,
               num_bucket * bucket_ldim - 1);
    // now copy the data to the bucket
#pragma _NEC vob
    for(size_t b = 0; b < block_size; b++) { // b: block
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
      for(int v = 0; v < RADIX_SORT_VLEN; v++) { // vector loop, loop raking
        auto key = key_src[block_size * v + b];
        int bucket = (key >> (d - 1) * 8) & 0xFF;
        size_t to = px_bucket_tablep[bucket_ldim * bucket + v] +
          posp[block_size * v + b];
        key_dst[to] = key;
        val_dst[to] = val_src[block_size * v + b];
      }
    }
    v = RADIX_SORT_VLEN;
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t b = 0; b < rest; b++) {
      auto key = key_src[block_size * v + b];
      int bucket = (key >> (d - 1) * 8) & 0xFF;
      size_t to = px_bucket_tablep[bucket_ldim * bucket + v] +
        posp[block_size * v + b];
      key_dst[to] = key;
      val_dst[to] = val_src[block_size * v + b];
    }
    next_is_tmp = 1 - next_is_tmp;
  }

  if(!next_is_tmp) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t i = 0; i < size; i++) {
      key_array[i] = key_array_tmp[i];
      val_array[i] = val_array_tmp[i];
    }
  }
}

template <class K, class V>
void radix_sort_desc(K* key_array, V* val_array, size_t size) {
  size_t key_size = sizeof(K);
  size_t bucket_ldim = RADIX_SORT_VLEN + 1;
  size_t num_bucket = 1 << 8; // 8bit == 256
  // bucket_table is columnar (VLEN + 1) by num_bucket matrix
  // "1" is to avoid bank conflict, but reused for "rest" of the data
  std::vector<size_t> bucket_table(num_bucket * bucket_ldim);
  std::vector<size_t> px_bucket_table(num_bucket * bucket_ldim);
  size_t* bucket_tablep = &bucket_table[0];
  size_t* px_bucket_tablep = &px_bucket_table[0];

  K max = 0;
  for(size_t i = 0; i < size; i++) {
    if(key_array[i] > max) max = key_array[i];
  }

  size_t max_key_size = 0;
  size_t tmp = 0xFF;
  for(size_t i = 1; i < key_size + 1; i++) {
    if(max < tmp) {
      max_key_size = i;
      break;
    } else {
      tmp = (tmp << 8) | 0xFF;
    }
  }

  std::vector<size_t> pos(size);
  size_t* posp = &pos[0];
  std::vector<K> key_array_tmpv(size);
  std::vector<V> val_array_tmpv(size);
  K* key_array_tmp = &key_array_tmpv[0];
  V* val_array_tmp = &val_array_tmpv[0];
  int next_is_tmp = 1;
  size_t block_size = size / RADIX_SORT_VLEN;
  if(block_size % 2 == 0 && block_size != 0) block_size -= 1;
  size_t rest = size - RADIX_SORT_VLEN * block_size;
  for(size_t d = 1; d <= max_key_size; d++) { // d: digit
    for(size_t i = 0; i < bucket_table.size(); i++) bucket_tablep[i] = 0;
    K *key_src, *key_dst;
    V *val_src, *val_dst;
    if(next_is_tmp) {
      key_src = key_array; key_dst = key_array_tmp;
      val_src = val_array; val_dst = val_array_tmp;
    } else {
      key_src = key_array_tmp; key_dst = key_array;
      val_src = val_array_tmp; val_dst = val_array;
    }
    for(size_t b = 0; b < block_size; b++) { // b: block
#pragma cdir nodep
#pragma _NEC ivdep
      for(int v = 0; v < RADIX_SORT_VLEN; v++) { // vector loop, loop raking
        auto key = key_src[block_size * v + b];
        int bucket = 0xFF - ((key >> (d - 1) * 8) & 0xFF); // desc
        posp[block_size * v + b] = bucket_tablep[bucket_ldim * bucket + v];
      }
#pragma cdir nodep
#pragma _NEC ivdep
      for(int v = 0; v < RADIX_SORT_VLEN; v++) { // vector loop, loop raking
        auto key = key_src[block_size * v + b];
        int bucket = 0xFF - ((key >> (d - 1) * 8) & 0xFF); // desc
        bucket_tablep[bucket_ldim * bucket + v]++;
      }
    }
    size_t v = RADIX_SORT_VLEN;
    for(int b = 0; b < rest; b++) { // not vector loop
      auto key = key_src[block_size * v + b];
      int bucket = 0xFF - ((key >> (d - 1) * 8) & 0xFF); // desc
      posp[block_size * v + b] = bucket_tablep[bucket_ldim * bucket + v];
      bucket_tablep[bucket_ldim * bucket + v]++;
    }
    // preparing for the copy
    prefix_sum(bucket_tablep, px_bucket_tablep + 1,
               num_bucket * bucket_ldim - 1);
    // now copy the data to the bucket
#pragma _NEC vob
    for(size_t b = 0; b < block_size; b++) { // b: block
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
      for(int v = 0; v < RADIX_SORT_VLEN; v++) { // vector loop, loop raking
        auto key = key_src[block_size * v + b];
        int bucket = 0xFF - ((key >> (d - 1) * 8) & 0xFF); // desc
        size_t to = px_bucket_tablep[bucket_ldim * bucket + v] +
          posp[block_size * v + b];
        key_dst[to] = key;
        val_dst[to] = val_src[block_size * v + b];
      }
    }
    v = RADIX_SORT_VLEN;
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t b = 0; b < rest; b++) {
      auto key = key_src[block_size * v + b];
      int bucket = 0xFF - ((key >> (d - 1) * 8) & 0xFF); // desc
      size_t to = px_bucket_tablep[bucket_ldim * bucket + v] +
        posp[block_size * v + b];
      key_dst[to] = key;
      val_dst[to] = val_src[block_size * v + b];
    }
    next_is_tmp = 1 - next_is_tmp;
  }

  if(!next_is_tmp) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t i = 0; i < size; i++) {
      key_array[i] = key_array_tmp[i];
      val_array[i] = val_array_tmp[i];
    }
  }
}

template <class K, class V>
void radix_sort(std::vector<K>& key_array, std::vector<V>& val_array) {
  size_t size = key_array.size();
  if(val_array.size() != size)
    throw std::runtime_error("radix_sort: different size of arrays");
  K* kp = &key_array[0];
  V* vp = &val_array[0];
  size_t pos_size = 0;
  for(size_t i = 0; i < size; i++) {
    if(kp[i] >= 0) pos_size++;
  }
  std::vector<K> key_array_pos(pos_size);
  std::vector<V> val_array_pos(pos_size);
  std::vector<K> key_array_neg(size-pos_size);
  std::vector<V> val_array_neg(size-pos_size);
  K* kpp = &key_array_pos[0];
  K* knp = &key_array_neg[0];
  V* vpp = &val_array_pos[0];
  V* vnp = &val_array_neg[0];
  size_t pos_current = 0;
  size_t neg_current = 0;
  for(size_t i = 0; i < size; i++) {
    if(kp[i] >= 0) {
      kpp[pos_current] = kp[i];
      vpp[pos_current++] = vp[i];
    } else {
      knp[neg_current] = -kp[i];
      vnp[neg_current++] = vp[i];
    }
  }
  if(sizeof(K) == 8) {
    uint64_t* knp_ = reinterpret_cast<uint64_t*>(knp);
    uint64_t* kpp_ = reinterpret_cast<uint64_t*>(kpp);
    radix_sort_desc(knp_, vnp, neg_current);
    radix_sort(kpp_, vpp, pos_current);
  } else if (sizeof(K) == 4) {
    uint32_t* knp_ = reinterpret_cast<uint32_t*>(knp);
    uint32_t* kpp_ = reinterpret_cast<uint32_t*>(kpp);
    radix_sort_desc(knp_, vnp, neg_current);
    radix_sort(kpp_, vpp, pos_current);
  } else if (sizeof(K) == 2) {
    uint16_t* knp_ = reinterpret_cast<uint16_t*>(knp);
    uint16_t* kpp_ = reinterpret_cast<uint16_t*>(kpp);
    radix_sort_desc(knp_, vnp, neg_current);
    radix_sort(kpp_, vpp, pos_current);
  } else if (sizeof(K) == 1) {
    uint8_t* knp_ = reinterpret_cast<uint8_t*>(knp);
    uint8_t* kpp_ = reinterpret_cast<uint8_t*>(kpp);
    radix_sort_desc(knp_, vnp, neg_current);
    radix_sort(kpp_, vpp, pos_current);
  }
  for(size_t i = 0; i < neg_current; i++) {
    kp[i] = -knp[i];
    vp[i] = vnp[i];
  }
  for(size_t i = 0; i < pos_current; i++) {
    kp[neg_current+i] = kpp[i];
    vp[neg_current+i] = vpp[i];
  }
}

template <class K, class V>
void radix_sort_desc(std::vector<K>& key_array, std::vector<V>& val_array) {
  size_t size = key_array.size();
  if(val_array.size() != size)
    throw std::runtime_error("radix_sort: different size of arrays");
  K* kp = &key_array[0];
  V* vp = &val_array[0];
  size_t pos_size = 0;
  for(size_t i = 0; i < size; i++) {
    if(kp[i] >= 0) pos_size++;
  }
  std::vector<K> key_array_pos(pos_size);
  std::vector<V> val_array_pos(pos_size);
  std::vector<K> key_array_neg(size-pos_size);
  std::vector<V> val_array_neg(size-pos_size);
  K* kpp = &key_array_pos[0];
  K* knp = &key_array_neg[0];
  V* vpp = &val_array_pos[0];
  V* vnp = &val_array_neg[0];
  size_t pos_current = 0;
  size_t neg_current = 0;
  for(size_t i = 0; i < size; i++) {
    if(kp[i] >= 0) {
      kpp[pos_current] = kp[i];
      vpp[pos_current++] = vp[i];
    } else {
      knp[neg_current] = -kp[i];
      vnp[neg_current++] = vp[i];
    }
  }
  if(sizeof(K) == 8) {
    uint64_t* knp_ = reinterpret_cast<uint64_t*>(knp);
    uint64_t* kpp_ = reinterpret_cast<uint64_t*>(kpp);
    radix_sort(knp_, vnp, neg_current);
    radix_sort_desc(kpp_, vpp, pos_current);
  } else if (sizeof(K) == 4) {
    uint32_t* knp_ = reinterpret_cast<uint32_t*>(knp);
    uint32_t* kpp_ = reinterpret_cast<uint32_t*>(kpp);
    radix_sort(knp_, vnp, neg_current);
    radix_sort_desc(kpp_, vpp, pos_current);
  } else if (sizeof(K) == 2) {
    uint16_t* knp_ = reinterpret_cast<uint16_t*>(knp);
    uint16_t* kpp_ = reinterpret_cast<uint16_t*>(kpp);
    radix_sort(knp_, vnp, neg_current);
    radix_sort_desc(kpp_, vpp, pos_current);
  } else if (sizeof(K) == 1) {
    uint8_t* knp_ = reinterpret_cast<uint8_t*>(knp);
    uint8_t* kpp_ = reinterpret_cast<uint8_t*>(kpp);
    radix_sort(knp_, vnp, neg_current);
    radix_sort_desc(kpp_, vpp, pos_current);
  }
  for(size_t i = 0; i < pos_current; i++) {
    kp[i] = kpp[i];
    vp[i] = vpp[i];
  }
  for(size_t i = 0; i < neg_current; i++) {
    kp[pos_current+i] = -knp[i];
    vp[pos_current+i] = vnp[i];
  }
}

}
#endif
