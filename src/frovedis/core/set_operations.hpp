#ifndef SET_OPERATIONS_HPP
#define SET_OPERATIONS_HPP

#include <vector>
#include <algorithm>
#include <utility>
#include "../core/utility.hpp"
#include "../core/radix_sort.hpp" // for reverse
#include "../core/lower_bound.hpp"
#include "../core/upper_bound.hpp"


#define SET_VLEN 768
#define SET_VLEN_EACH 256

namespace frovedis {
#if !(defined(_SX) || defined(__ve__))
template <class T>
std::vector<T> set_intersection(const T* leftp, size_t left_size,
                                const T* rightp, size_t right_size) {
  std::vector<T> ret;
  std::set_intersection(leftp, leftp + left_size,
                        rightp, rightp + right_size,
                        std::back_inserter(ret));
  return ret;
}

template <class T>
std::vector<T> set_union(const T* leftp, size_t left_size,
                         const T* rightp, size_t right_size) {
  std::vector<T> ret;
  std::set_union(leftp, leftp + left_size,
                 rightp, rightp + right_size,
                 std::back_inserter(ret));
  return ret;
}

template <class T>
std::vector<T> set_difference(const T* leftp, size_t left_size,
                              const T* rightp, size_t right_size) {
  std::vector<T> ret;
  std::set_difference(leftp, leftp + left_size,
                      rightp, rightp + right_size,
                      std::back_inserter(ret));
  return ret;
}

template <class T>
std::vector<T> set_merge(const T* leftp, size_t left_size,
                         const T* rightp, size_t right_size) {
  std::vector<T> ret;
  std::merge(leftp, leftp + left_size,
             rightp, rightp + right_size,
             std::back_inserter(ret));
  return ret;
}

template <class T>
std::vector<T> set_merge_desc(const T* leftp, size_t left_size,
                              const T* rightp, size_t right_size) {
  std::vector<T> ret;
  std::merge(leftp, leftp + left_size,
             rightp, rightp + right_size,
             std::back_inserter(ret), std::greater<T>());
  return ret;
}

template <class K, class V>
struct set_merge_pair_helper {
  bool operator()(const std::pair<K,V>& left,
                  const std::pair<K,V>& right) {
    return left.first < right.first;
  }
};

template <class T, class K>
void set_merge_pair(const T* leftp,
                    const K* left_valp,
                    size_t left_size,
                    const T* rightp,
                    const K* right_valp,
                    size_t right_size,
                    std::vector<T>& out,
                    std::vector<K>& out_val) {
  std::vector<std::pair<T,K>> left_pair(left_size);
  std::vector<std::pair<T,K>> right_pair(right_size);
  std::vector<std::pair<T,K>> out_pair;
  for(size_t i = 0; i < left_size; i++) {
    left_pair[i].first = leftp[i];
    left_pair[i].second = left_valp[i];
  }
  for(size_t i = 0; i < right_size; i++) {
    right_pair[i].first = rightp[i];
    right_pair[i].second = right_valp[i];
  }
  std::merge(left_pair.begin(), left_pair.end(),
             right_pair.begin(), right_pair.end(),
             std::back_inserter(out_pair), set_merge_pair_helper<T,K>());
  out.resize(left_size + right_size);
  out_val.resize(left_size + right_size);
  for(size_t i = 0; i < left_size + right_size; i++) {
    out[i] = out_pair[i].first;
    out_val[i] = out_pair[i].second;
  }
}

template <class K, class V>
struct set_merge_pair_desc_helper {
  bool operator()(const std::pair<K,V>& left,
                  const std::pair<K,V>& right) {
    return left.first > right.first;
  }
};

template <class T, class K>
void set_merge_pair_desc(const T* leftp,
                         const K* left_valp,
                         size_t left_size,
                         const T* rightp,
                         const K* right_valp,
                         size_t right_size,
                         std::vector<T>& out,
                         std::vector<K>& out_val) {
  std::vector<std::pair<T,K>> left_pair(left_size);
  std::vector<std::pair<T,K>> right_pair(right_size);
  std::vector<std::pair<T,K>> out_pair;
  for(size_t i = 0; i < left_size; i++) {
    left_pair[i].first = leftp[i];
    left_pair[i].second = left_valp[i];
  }
  for(size_t i = 0; i < right_size; i++) {
    right_pair[i].first = rightp[i];
    right_pair[i].second = right_valp[i];
  }
  std::merge(left_pair.begin(), left_pair.end(),
             right_pair.begin(), right_pair.end(),
             std::back_inserter(out_pair), set_merge_pair_desc_helper<T,K>());
  out.resize(left_size + right_size);
  out_val.resize(left_size + right_size);
  for(size_t i = 0; i < left_size + right_size; i++) {
    out[i] = out_pair[i].first;
    out_val[i] = out_pair[i].second;
  }
}

template <class T>
std::vector<size_t> set_separate(const T* keyp, size_t size) {
  std::vector<size_t> ret;
  if(size == 0) {
    ret.push_back(0);
    return ret;
  } else {
    T current = keyp[0];
    ret.push_back(0);
    for(size_t i = 1; i < size; i++) {
      if(keyp[i] != current) {
        current = keyp[i];
        ret.push_back(i);
      }
    }
    ret.push_back(size);
    return ret;
  }
}

template <class T>
std::vector<T> set_unique(const T* keyp, size_t size) {
  std::vector<T> ret;
  if(size == 0) {
    return ret;
  } else {
    T current = keyp[0];
    ret.push_back(current);
    for(size_t i = 1; i < size; i++) {
      if(keyp[i] != current) {
        current = keyp[i];
        ret.push_back(current);
      }
    }
    return ret;
  }
}

template <class T>
int set_is_unique(const T* keyp, size_t size) {
  if(size == 0) {
    return true;
  } else {
    T current = keyp[0];
    for(size_t i = 1; i < size; i++) {
      if(keyp[i] != current) current = keyp[i];
      else return false;
    }
    return true;
  }
}

#else // SX

template <class T>
void advance_to_split_vreg(size_t* left_idx, size_t left_size, const T* left) {
  int advance_done[SET_VLEN];
  int advance_done_0[SET_VLEN_EACH];
  int advance_done_1[SET_VLEN_EACH];
  int advance_done_2[SET_VLEN_EACH];
  size_t left_idx_0[SET_VLEN_EACH];
  size_t left_idx_1[SET_VLEN_EACH];
  size_t left_idx_2[SET_VLEN_EACH];
  T last_0[SET_VLEN_EACH];
  T last_1[SET_VLEN_EACH];
  T last_2[SET_VLEN_EACH];
  T loaded_0[SET_VLEN_EACH];
  T loaded_1[SET_VLEN_EACH];
  T loaded_2[SET_VLEN_EACH];
#pragma _NEC vreg(advance_done_0)
#pragma _NEC vreg(advance_done_1)
#pragma _NEC vreg(advance_done_2)
#pragma _NEC vreg(left_idx_0)
#pragma _NEC vreg(left_idx_1)
#pragma _NEC vreg(left_idx_2)
#pragma _NEC vreg(last_0)
#pragma _NEC vreg(last_1)
#pragma _NEC vreg(last_2)
#pragma _NEC vreg(loaded_0)
#pragma _NEC vreg(loaded_1)
#pragma _NEC vreg(loaded_2)
  for(int i = 0; i < SET_VLEN; i++) {
    if(left_idx[i] == left_size) advance_done[i] = true;
    else advance_done[i] = false;
  }
  advance_done[0] = true;
  for(size_t i = 0; i < SET_VLEN_EACH; i++) {
    advance_done_0[i] = advance_done[i];
    advance_done_1[i] = advance_done[SET_VLEN_EACH + i];
    advance_done_2[i] = advance_done[SET_VLEN_EACH * 2 + i];
    left_idx_0[i] = left_idx[i];
    left_idx_1[i] = left_idx[SET_VLEN_EACH + i];
    left_idx_2[i] = left_idx[SET_VLEN_EACH * 2 + i];
  }
  for(int i = 0; i < SET_VLEN_EACH; i++) {
    if(!advance_done_0[i]) last_0[i] = left[left_idx_0[i]-1];
    if(!advance_done_1[i]) last_1[i] = left[left_idx_1[i]-1];
    if(!advance_done_2[i]) last_2[i] = left[left_idx_2[i]-1];
  }
  int anyvalid = true;
  while(anyvalid) {
    anyvalid = false;
    for(int i = 0; i < SET_VLEN_EACH; i++) {
      if(!advance_done_0[i]) loaded_0[i] = left[left_idx_0[i]];
      if(!advance_done_1[i]) loaded_1[i] = left[left_idx_1[i]];
      if(!advance_done_2[i]) loaded_2[i] = left[left_idx_2[i]];
    }
    for(int i = 0; i < SET_VLEN_EACH; i++) {
      if(!advance_done_0[i]) {
        if(loaded_0[i] == last_0[i] && left_idx_0[i] < left_size) {
          left_idx_0[i]++;
          anyvalid = true;
        } else {
          advance_done_0[i] = true;
        }
      }
      if(!advance_done_1[i]) {
        if(loaded_1[i] == last_1[i] && left_idx_1[i] < left_size) {
          left_idx_1[i]++;
          anyvalid = true;
        } else {
          advance_done_1[i] = true;
        }
      }
      if(!advance_done_2[i]) {
        if(loaded_2[i] == last_2[i] && left_idx_2[i] < left_size) {
          left_idx_2[i]++;
          anyvalid = true;
        } else {
          advance_done_2[i] = true;
        }
      }
    }
  }
  for(size_t i = 0; i < SET_VLEN_EACH; i++) {
    left_idx[i] = left_idx_0[i];
    left_idx[SET_VLEN_EACH + i] = left_idx_1[i];
    left_idx[SET_VLEN_EACH * 2 + i] = left_idx_2[i];
  }
}

template <class T>
void set_intersection_vreg(const T* lp, const T* rp, T* op, 
                           size_t* left_idx, size_t* right_idx,
                           size_t* left_idx_stop, size_t* right_idx_stop,
                           size_t* out_idx) {
#include "set_operations.incl1" // decl/init vregs
  int anyvalid = true;
  while(anyvalid) {
    anyvalid = false;
#include "set_operations.incl2"
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_0[j] != left_idx_stop_0[j] &&
         right_idx_0[j] != right_idx_stop_0[j]) {
        anyvalid = true;
        if(leftelm0[j] < rightelm0[j]) {
          left_idx_0[j]++;
        } else {
          if(!(rightelm0[j] < leftelm0[j])) {
            op[out_idx_0[j]++] = leftelm0[j];
            left_idx_0[j]++;
          }
          right_idx_0[j]++;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_1[j] != left_idx_stop_1[j] &&
         right_idx_1[j] != right_idx_stop_1[j]) {
        anyvalid = true;
        if(leftelm1[j] < rightelm1[j]) {
          left_idx_1[j]++;
        } else {
          if(!(rightelm1[j] < leftelm1[j])) {
            op[out_idx_1[j]++] = leftelm1[j];
            left_idx_1[j]++;
          }
          right_idx_1[j]++;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_2[j] != left_idx_stop_2[j] &&
         right_idx_2[j] != right_idx_stop_2[j]) {
        anyvalid = true;
        if(leftelm2[j] < rightelm2[j]) {
          left_idx_2[j]++;
        } else {
          if(!(rightelm2[j] < leftelm2[j])) {
            op[out_idx_2[j]++] = leftelm2[j];
            left_idx_2[j]++;
          }
          right_idx_2[j]++;
        }
      }
    }
  }
  for(size_t i = 0; i < SET_VLEN_EACH; i++) {
    out_idx[i] = out_idx_0[i];
    out_idx[SET_VLEN_EACH * 1 + i] = out_idx_1[i];
    out_idx[SET_VLEN_EACH * 2 + i] = out_idx_2[i];
  }
}

template <class T>
std::vector<T> set_intersection(const T* leftp, size_t left_size,
                                const T* rightp, size_t right_size) {
  if(left_size == 0 || right_size == 0) return std::vector<T>();

  size_t each = ceil_div(left_size, size_t(SET_VLEN));
  if(each % 2 == 0) each++;
  
  size_t left_idx[SET_VLEN];
  size_t right_idx[SET_VLEN];
  size_t left_idx_stop[SET_VLEN];
  size_t right_idx_stop[SET_VLEN];
  size_t out_idx[SET_VLEN];
  size_t out_idx_save[SET_VLEN];
  std::vector<T> out(left_size);
  auto outp = out.data();

  for(int i = 0; i < SET_VLEN; i++) {
    size_t pos = each * i;
    if(pos < left_size) {
      left_idx[i] = pos;
      out_idx[i] = pos;
      out_idx_save[i] = pos;
    } else {
      left_idx[i] = left_size;
      out_idx[i] = left_size;
      out_idx_save[i] = left_size;
    }
  }

  advance_to_split_vreg(left_idx, left_size, leftp);

  T left_start[SET_VLEN];
  for(size_t i = 0; i < SET_VLEN; i++) {
    if(left_idx[i] < left_size) left_start[i] = leftp[left_idx[i]];
    else left_start[i] = 0;
  }
  lower_bound(rightp, right_size, left_start, SET_VLEN, right_idx);
  for(size_t i = 0; i < SET_VLEN; i++) {
    if(left_idx[i] == left_size) right_idx[i] = right_size; // left is not valid
  }
  for(size_t i = 0; i < SET_VLEN-1; i++) {
    if(left_idx[i] < left_size) left_start[i] = leftp[left_idx[i+1]-1];
    else left_start[i] = 0;
  }
  left_start[SET_VLEN-1] = leftp[left_size-1];
  upper_bound(rightp, right_size, left_start, SET_VLEN, right_idx_stop);
  for(size_t i = 0; i < SET_VLEN; i++) {
    if(left_idx[i] == left_size || right_idx[i] == right_size)
      right_idx_stop[i] = right_size;
  }
  for(int i = 0; i < SET_VLEN - 1; i++) {
    left_idx_stop[i] = left_idx[i + 1];
  }
  left_idx_stop[SET_VLEN-1] = left_size;
  
  set_intersection_vreg(leftp, rightp, outp, left_idx, right_idx,
                        left_idx_stop, right_idx_stop,
                        out_idx);

  size_t total = 0;
  for(size_t i = 0; i < SET_VLEN; i++) {
    total += out_idx[i] - out_idx_save[i];
  }
  std::vector<T> ret(total);
  T* retp = &ret[0];
  size_t current = 0;
  for(size_t i = 0; i < SET_VLEN; i++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t j = 0; j < out_idx[i] - out_idx_save[i]; j++) {
      retp[current + j] = outp[out_idx_save[i] + j];
    }
    current += out_idx[i] - out_idx_save[i];
  }
  return ret;
}

template <class T>
void set_union_vreg(const T* lp, const T* rp, T* op,
                    size_t* left_idx, size_t* right_idx,
                    size_t* left_idx_stop, size_t* right_idx_stop,
                    size_t* out_idx) {
#include "set_operations.incl1"
  int anyvalid = true;
  while(anyvalid) {
    anyvalid = false;
#include "set_operations.incl2"
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_0[j] != left_idx_stop_0[j] &&
         right_idx_0[j] != right_idx_stop_0[j]) {
        anyvalid = true;
        if(rightelm0[j] < leftelm0[j]) {
          op[out_idx_0[j]] = rightelm0[j];
          right_idx_0[j]++;
        } else {
          op[out_idx_0[j]] = leftelm0[j];
          if(!(leftelm0[j] < rightelm0[j])) {
            right_idx_0[j]++;
          }
          left_idx_0[j]++;
        }
        out_idx_0[j]++;
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_1[j] != left_idx_stop_1[j] &&
         right_idx_1[j] != right_idx_stop_1[j]) {
        anyvalid = true;
        if(rightelm1[j] < leftelm1[j]) {
          op[out_idx_1[j]] = rightelm1[j];
          right_idx_1[j]++;
        } else {
          op[out_idx_1[j]] = leftelm1[j];
          if(!(leftelm1[j] < rightelm1[j])) {
            right_idx_1[j]++;
          }
          left_idx_1[j]++;
        }
        out_idx_1[j]++;
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_2[j] != left_idx_stop_2[j] &&
         right_idx_2[j] != right_idx_stop_2[j]) {
        anyvalid = true;
        if(rightelm2[j] < leftelm2[j]) {
          op[out_idx_2[j]] = rightelm2[j];
          right_idx_2[j]++;
        } else {
          op[out_idx_2[j]] = leftelm2[j];
          if(!(leftelm2[j] < rightelm2[j])) {
            right_idx_2[j]++;
          }
          left_idx_2[j]++;
        }
        out_idx_2[j]++;
      }
    }
  }
  for(size_t i = 0; i < SET_VLEN_EACH; i++) {
    out_idx[i] = out_idx_0[i];
    out_idx[SET_VLEN_EACH * 1 + i] = out_idx_1[i];
    out_idx[SET_VLEN_EACH * 2 + i] = out_idx_2[i];
    left_idx[i] = left_idx_0[i];
    left_idx[SET_VLEN_EACH * 1 + i] = left_idx_1[i];
    left_idx[SET_VLEN_EACH * 2 + i] = left_idx_2[i];
    right_idx[i] = right_idx_0[i];
    right_idx[SET_VLEN_EACH * 1 + i] = right_idx_1[i];
    right_idx[SET_VLEN_EACH * 2 + i] = right_idx_2[i];
  }
}

template <class T>
std::vector<T> set_union(const T* leftp, size_t left_size,
                         const T* rightp, size_t right_size) {
  if(left_size == 0) {
    std::vector<T> ret(right_size);
    auto retp = ret.data();
    for(size_t i = 0; i < right_size; i++) retp[i] = rightp[i];
    return ret;
  }
  if(right_size == 0) {
    std::vector<T> ret(left_size);
    auto retp = ret.data();
    for(size_t i = 0; i < left_size; i++) retp[i] = leftp[i];
    return ret;
  }

  size_t each = ceil_div(left_size, size_t(SET_VLEN));
  if(each % 2 == 0) each++;
  
  size_t left_idx[SET_VLEN];
  size_t right_idx[SET_VLEN];
  size_t left_idx_stop[SET_VLEN];
  size_t right_idx_stop[SET_VLEN];
  size_t out_idx[SET_VLEN];
  size_t out_idx_save[SET_VLEN];
  std::vector<T> out(left_size + right_size);
  auto outp = out.data();
  for(int i = 0; i < SET_VLEN; i++) {
    size_t pos = each * i;
    if(pos < left_size) {
      left_idx[i] = pos;
    } else {
      left_idx[i] = left_size;
    }
  }

  advance_to_split_vreg(left_idx, left_size, leftp);

  T left_start[SET_VLEN];
  for(size_t i = 0; i < SET_VLEN; i++) {
    if(left_idx[i] < left_size) left_start[i] = leftp[left_idx[i]];
    else left_start[i] = 0;
  }
  lower_bound(rightp, right_size, left_start, SET_VLEN, right_idx);
  right_idx[0] = 0;
  for(size_t i = 0; i < SET_VLEN; i++) {
    if(left_idx[i] == left_size) right_idx[i] = right_size; // left is not valid
  }
  out_idx[0] = 0;
  out_idx_save[0] = 0;
  for(int i = 1; i < SET_VLEN; i++) {
    out_idx[i] = (left_idx[i] - left_idx[i-1])
      + (right_idx[i] - right_idx[i-1])
      + out_idx[i-1];
    out_idx_save[i] = out_idx[i];
  }
  for(int i = 0; i < SET_VLEN - 1; i++) {
    left_idx_stop[i] = left_idx[i + 1];
    right_idx_stop[i] = right_idx[i + 1];
  }
  left_idx_stop[SET_VLEN-1] = left_size;
  right_idx_stop[SET_VLEN-1] = right_size;
  set_union_vreg(leftp, rightp, outp, 
                 left_idx, right_idx,
                 left_idx_stop, right_idx_stop,
                 out_idx);

  size_t total = 0;
  for(size_t i = 0; i < SET_VLEN; i++) {
    total += (out_idx[i] - out_idx_save[i]) +
      (left_idx_stop[i] - left_idx[i]) +
      (right_idx_stop[i] - right_idx[i]);
  }
  std::vector<T> ret(total);
  T* retp = &ret[0];
  size_t current = 0;
  for(size_t i = 0; i < SET_VLEN; i++) {
    for(size_t j = 0; j < out_idx[i] - out_idx_save[i]; j++) {
      retp[current + j] = outp[out_idx_save[i] + j];
    }
    current += out_idx[i] - out_idx_save[i];
    for(size_t j = 0; j < left_idx_stop[i] - left_idx[i]; j++) {
      retp[current + j] = leftp[left_idx[i] + j];
    }
    current += left_idx_stop[i] - left_idx[i];
    for(size_t j = 0; j < right_idx_stop[i] - right_idx[i]; j++) {
      retp[current + j] = rightp[right_idx[i] + j];
    }
    current += right_idx_stop[i] - right_idx[i];
  }
  return ret;
}

template <class T>
void set_difference_vreg(const T* lp, const T* rp, T* op,
                         size_t* left_idx, size_t* right_idx,
                         size_t* left_idx_stop, size_t* right_idx_stop,
                         size_t* out_idx) {
#include "set_operations.incl1"
  int anyvalid = true;
  while(anyvalid) {
    anyvalid = false;
#include "set_operations.incl2"
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_0[j] != left_idx_stop_0[j] &&
         right_idx_0[j] != right_idx_stop_0[j]) {
        anyvalid = true;
        if(leftelm0[j] < rightelm0[j]) {
          op[out_idx_0[j]++] = leftelm0[j];
          left_idx_0[j]++;
        } else {
          if(!(rightelm0[j] < leftelm0[j])) {
            left_idx_0[j]++;
          }
          right_idx_0[j]++;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_1[j] != left_idx_stop_1[j] &&
         right_idx_1[j] != right_idx_stop_1[j]) {
        anyvalid = true;
        if(leftelm1[j] < rightelm1[j]) {
          op[out_idx_1[j]++] = leftelm1[j];
          left_idx_1[j]++;
        } else {
          if(!(rightelm1[j] < leftelm1[j])) {
            left_idx_1[j]++;
          }
          right_idx_1[j]++;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_2[j] != left_idx_stop_2[j] &&
         right_idx_2[j] != right_idx_stop_2[j]) {
        anyvalid = true;
        if(leftelm2[j] < rightelm2[j]) {
          op[out_idx_2[j]++] = leftelm2[j];
          left_idx_2[j]++;
        } else {
          if(!(rightelm2[j] < leftelm2[j])) {
            left_idx_2[j]++;
          }
          right_idx_2[j]++;
        }
      }
    }
  }
  for(size_t i = 0; i < SET_VLEN_EACH; i++) {
    out_idx[i] = out_idx_0[i];
    out_idx[SET_VLEN_EACH * 1 + i] = out_idx_1[i];
    out_idx[SET_VLEN_EACH * 2 + i] = out_idx_2[i];
    left_idx[i] = left_idx_0[i];
    left_idx[SET_VLEN_EACH * 1 + i] = left_idx_1[i];
    left_idx[SET_VLEN_EACH * 2 + i] = left_idx_2[i];
  }
}

template <class T>
std::vector<T> set_difference(const T* leftp, size_t left_size,
                              const T* rightp, size_t right_size) {
  if(left_size == 0) return std::vector<T>();
  if(right_size == 0) {
    std::vector<T> ret(left_size);
    auto retp = ret.data();
    for(size_t i = 0; i < left_size; i++) retp[i] = leftp[i];
    return ret;
  }

  size_t each = ceil_div(left_size, size_t(SET_VLEN));
  if(each % 2 == 0) each++;
  
  size_t left_idx[SET_VLEN];
  size_t right_idx[SET_VLEN];
  size_t left_idx_stop[SET_VLEN];
  size_t right_idx_stop[SET_VLEN];
  size_t out_idx[SET_VLEN];
  size_t out_idx_save[SET_VLEN];
  std::vector<T> out(left_size);
  auto outp = out.data();
  for(int i = 0; i < SET_VLEN; i++) {
    size_t pos = each * i;
    if(pos < left_size) {
      left_idx[i] = pos;
    } else {
      left_idx[i] = left_size;
    }
  }

  advance_to_split_vreg(left_idx, left_size, leftp);

  for(int i = 0; i < SET_VLEN; i++) {
    out_idx[i] = left_idx[i];
    out_idx_save[i] = left_idx[i];
  }
  T left_start[SET_VLEN];
  for(size_t i = 0; i < SET_VLEN; i++) {
    if(left_idx[i] < left_size) left_start[i] = leftp[left_idx[i]];
    else left_start[i] = 0;
  }
  lower_bound(rightp, right_size, left_start, SET_VLEN, right_idx);
  right_idx[0] = 0;
  for(size_t i = 0; i < SET_VLEN; i++) {
    if(left_idx[i] == left_size) right_idx[i] = right_size; // left is not valid
  }
  for(int i = 0; i < SET_VLEN - 1; i++) {
    left_idx_stop[i] = left_idx[i + 1];
    right_idx_stop[i] = right_idx[i + 1];
  }
  left_idx_stop[SET_VLEN-1] = left_size;
  right_idx_stop[SET_VLEN-1] = right_size;
  set_difference_vreg(leftp, rightp, outp,
                      left_idx, right_idx,
                      left_idx_stop, right_idx_stop,
                      out_idx);
  size_t total = 0;
  for(size_t i = 0; i < SET_VLEN; i++) {
    total += (out_idx[i] - out_idx_save[i]) +
      (left_idx_stop[i] - left_idx[i]);
  }
  std::vector<T> ret(total);
  T* retp = &ret[0];
  size_t current = 0;
  for(size_t i = 0; i < SET_VLEN; i++) {
    for(size_t j = 0; j < out_idx[i] - out_idx_save[i]; j++) {
      retp[current + j] = outp[out_idx_save[i] + j];
    }
    current += out_idx[i] - out_idx_save[i];
    for(size_t j = 0; j < left_idx_stop[i] - left_idx[i]; j++) {
      retp[current + j] = leftp[left_idx[i] + j];
    }
    current += left_idx_stop[i] - left_idx[i];
  }
  return ret;
}

template <class T>
void set_merge_vreg(const T* lp, const T* rp, T* op, 
                    size_t* left_idx, size_t* right_idx,
                    size_t* left_idx_stop, size_t* right_idx_stop,
                    size_t* out_idx) {
#include "set_operations.incl1"
  int anyvalid = true;
  while(anyvalid) {
    anyvalid = false;
#include "set_operations.incl2"
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_0[j] != left_idx_stop_0[j] &&
         right_idx_0[j] != right_idx_stop_0[j]) {
        anyvalid = true;
        if(rightelm0[j] < leftelm0[j]) {
          op[out_idx_0[j]] = rightelm0[j];
          right_idx_0[j]++;
        } else {
          op[out_idx_0[j]] = leftelm0[j];
          left_idx_0[j]++;
        }
        out_idx_0[j]++;
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_1[j] != left_idx_stop_1[j] &&
         right_idx_1[j] != right_idx_stop_1[j]) {
        anyvalid = true;
        if(rightelm1[j] < leftelm1[j]) {
          op[out_idx_1[j]] = rightelm1[j];
          right_idx_1[j]++;
        } else {
          op[out_idx_1[j]] = leftelm1[j];
          left_idx_1[j]++;
        }
        out_idx_1[j]++;
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_2[j] != left_idx_stop_2[j] &&
         right_idx_2[j] != right_idx_stop_2[j]) {
        anyvalid = true;
        if(rightelm2[j] < leftelm2[j]) {
          op[out_idx_2[j]] = rightelm2[j];
          right_idx_2[j]++;
        } else {
          op[out_idx_2[j]] = leftelm2[j];
          left_idx_2[j]++;
        }
        out_idx_2[j]++;
      }
    }
  }
  for(size_t i = 0; i < SET_VLEN_EACH; i++) {
    out_idx[i] = out_idx_0[i];
    out_idx[SET_VLEN_EACH * 1 + i] = out_idx_1[i];
    out_idx[SET_VLEN_EACH * 2 + i] = out_idx_2[i];
    left_idx[i] = left_idx_0[i];
    left_idx[SET_VLEN_EACH * 1 + i] = left_idx_1[i];
    left_idx[SET_VLEN_EACH * 2 + i] = left_idx_2[i];
    right_idx[i] = right_idx_0[i];
    right_idx[SET_VLEN_EACH * 1 + i] = right_idx_1[i];
    right_idx[SET_VLEN_EACH * 2 + i] = right_idx_2[i];
  }
}

template <class T>
std::vector<T> set_merge(const T* leftp, size_t left_size,
                         const T* rightp, size_t right_size) {
  if(left_size == 0) {
    std::vector<T> ret(right_size);
    auto retp = ret.data();
    for(size_t i = 0; i < right_size; i++) retp[i] = rightp[i];
    return ret;
  }
  if(right_size == 0) {
    std::vector<T> ret(left_size);
    auto retp = ret.data();
    for(size_t i = 0; i < left_size; i++) retp[i] = leftp[i];
    return ret;
  }

  size_t each = ceil_div(left_size, size_t(SET_VLEN));
  if(each % 2 == 0) each++;
  
  size_t left_idx[SET_VLEN];
  size_t right_idx[SET_VLEN];
  size_t left_idx_stop[SET_VLEN];
  size_t right_idx_stop[SET_VLEN];
  size_t out_idx[SET_VLEN];
  std::vector<T> out(left_size + right_size);
  auto outp = out.data();
  for(int i = 0; i < SET_VLEN; i++) {
    size_t pos = each * i;
    if(pos < left_size) {
      left_idx[i] = pos;
    } else {
      left_idx[i] = left_size;
    }
  }

  advance_to_split_vreg(left_idx, left_size, leftp);

  T left_start[SET_VLEN];
  for(size_t i = 0; i < SET_VLEN; i++) {
    if(left_idx[i] < left_size) left_start[i] = leftp[left_idx[i]];
    else left_start[i] = 0;
  }
  lower_bound(rightp, right_size, left_start, SET_VLEN, right_idx);
  right_idx[0] = 0;
  for(size_t i = 0; i < SET_VLEN; i++) {
    if(left_idx[i] == left_size) right_idx[i] = right_size; // left is not valid
  }
  out_idx[0] = 0;
  for(int i = 1; i < SET_VLEN; i++) {
    out_idx[i] = (left_idx[i] - left_idx[i-1]) +
      (right_idx[i] - right_idx[i-1])
      + out_idx[i-1];
  }
  for(int i = 0; i < SET_VLEN - 1; i++) {
    left_idx_stop[i] = left_idx[i + 1];
    right_idx_stop[i] = right_idx[i + 1];
  }
  left_idx_stop[SET_VLEN-1] = left_size;
  right_idx_stop[SET_VLEN-1] = right_size;
  set_merge_vreg(leftp, rightp, outp, 
                 left_idx, right_idx,
                 left_idx_stop, right_idx_stop,
                 out_idx);

  for(size_t i = 0; i < SET_VLEN; i++) {
    for(size_t j = 0; j < left_idx_stop[i] - left_idx[i]; j++) {
      outp[out_idx[i] + j] = leftp[left_idx[i] + j];
    }
    for(size_t j = 0; j < right_idx_stop[i] - right_idx[i]; j++) {
      outp[out_idx[i] + j] = rightp[right_idx[i] + j];
    }
  }
  return out;
}

template <class T>
void set_merge_desc_vreg(const T* lp, const T* rp, T* op, 
                         size_t* left_idx, size_t* right_idx,
                         size_t* left_idx_stop, size_t* right_idx_stop,
                         size_t* out_idx) {
#include "set_operations.incl1"
  int anyvalid = true;
  while(anyvalid) {
    anyvalid = false;
#include "set_operations.incl2"
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_0[j] != left_idx_stop_0[j] &&
         right_idx_0[j] != right_idx_stop_0[j]) {
        anyvalid = true;
        if(leftelm0[j] < rightelm0[j]) {
          op[out_idx_0[j]] = rightelm0[j];
          right_idx_0[j]++;
        } else {
          op[out_idx_0[j]] = leftelm0[j];
          left_idx_0[j]++;
        }
        out_idx_0[j]++;
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_1[j] != left_idx_stop_1[j] &&
         right_idx_1[j] != right_idx_stop_1[j]) {
        anyvalid = true;
        if(leftelm1[j] < rightelm1[j]) {
          op[out_idx_1[j]] = rightelm1[j];
          right_idx_1[j]++;
        } else {
          op[out_idx_1[j]] = leftelm1[j];
          left_idx_1[j]++;
        }
        out_idx_1[j]++;
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_2[j] != left_idx_stop_2[j] &&
         right_idx_2[j] != right_idx_stop_2[j]) {
        anyvalid = true;
        if(leftelm2[j] < rightelm2[j]) {
          op[out_idx_2[j]] = rightelm2[j];
          right_idx_2[j]++;
        } else {
          op[out_idx_2[j]] = leftelm2[j];
          left_idx_2[j]++;
        }
        out_idx_2[j]++;
      }
    }
  }
  for(size_t i = 0; i < SET_VLEN_EACH; i++) {
    out_idx[i] = out_idx_0[i];
    out_idx[SET_VLEN_EACH * 1 + i] = out_idx_1[i];
    out_idx[SET_VLEN_EACH * 2 + i] = out_idx_2[i];
    left_idx[i] = left_idx_0[i];
    left_idx[SET_VLEN_EACH * 1 + i] = left_idx_1[i];
    left_idx[SET_VLEN_EACH * 2 + i] = left_idx_2[i];
    right_idx[i] = right_idx_0[i];
    right_idx[SET_VLEN_EACH * 1 + i] = right_idx_1[i];
    right_idx[SET_VLEN_EACH * 2 + i] = right_idx_2[i];
  }
}

template <class T>
std::vector<T> set_merge_desc(const T* leftp, size_t left_size,
                              const T* rightp, size_t right_size) {
  if(left_size == 0) {
    std::vector<T> ret(right_size);
    auto retp = ret.data();
    for(size_t i = 0; i < right_size; i++) retp[i] = rightp[i];
    return ret;
  }
  if(right_size == 0) {
    std::vector<T> ret(left_size);
    auto retp = ret.data();
    for(size_t i = 0; i < left_size; i++) retp[i] = leftp[i];
    return ret;
  }

  size_t each = ceil_div(left_size, size_t(SET_VLEN));
  if(each % 2 == 0) each++;
  
  size_t left_idx[SET_VLEN];
  size_t right_idx[SET_VLEN];
  size_t left_idx_stop[SET_VLEN];
  size_t right_idx_stop[SET_VLEN];
  size_t out_idx[SET_VLEN];
  std::vector<T> out(left_size + right_size);
  auto outp = out.data();
  for(int i = 0; i < SET_VLEN; i++) {
    size_t pos = each * i;
    if(pos < left_size) {
      left_idx[i] = pos;
    } else {
      left_idx[i] = left_size;
    }
  }

  advance_to_split_vreg(left_idx, left_size, leftp);

  T left_start[SET_VLEN];
  for(size_t i = 0; i < SET_VLEN; i++) {
    if(left_idx[i] < left_size) left_start[i] = leftp[left_idx[i]];
    else left_start[i] = 0;
  }
  lower_bound_desc(rightp, right_size, left_start, SET_VLEN, right_idx);
  right_idx[0] = 0;
  for(size_t i = 0; i < SET_VLEN; i++) {
    if(left_idx[i] == left_size) right_idx[i] = right_size; // left is not valid
  }
  out_idx[0] = 0;
  for(int i = 1; i < SET_VLEN; i++) {
    out_idx[i] = (left_idx[i] - left_idx[i-1]) +
      (right_idx[i] - right_idx[i-1])
      + out_idx[i-1];
  }
  for(int i = 0; i < SET_VLEN - 1; i++) {
    left_idx_stop[i] = left_idx[i + 1];
    right_idx_stop[i] = right_idx[i + 1];
  }
  left_idx_stop[SET_VLEN-1] = left_size;
  right_idx_stop[SET_VLEN-1] = right_size;
  set_merge_desc_vreg(leftp, rightp, outp, 
                      left_idx, right_idx,
                      left_idx_stop, right_idx_stop,
                      out_idx);

  for(size_t i = 0; i < SET_VLEN; i++) {
    for(size_t j = 0; j < left_idx_stop[i] - left_idx[i]; j++) {
      outp[out_idx[i] + j] = leftp[left_idx[i] + j];
    }
    for(size_t j = 0; j < right_idx_stop[i] - right_idx[i]; j++) {
      outp[out_idx[i] + j] = rightp[right_idx[i] + j];
    }
  }
  return out;
}

template <class T, class K>
void set_merge_pair_vreg(const T* lp, const K* lvp, const T* rp, const K* rvp,
                         T* op, K* ovp, size_t* left_idx, size_t* right_idx,
                         size_t* left_idx_stop, size_t* right_idx_stop,
                         size_t* out_idx) {
#include "set_operations.incl1"
  int anyvalid = true;
  while(anyvalid) {
    anyvalid = false;
#include "set_operations.incl2"
    K leftval0[SET_VLEN_EACH];
    K leftval1[SET_VLEN_EACH];
    K leftval2[SET_VLEN_EACH];
    K rightval0[SET_VLEN_EACH];
    K rightval1[SET_VLEN_EACH];
    K rightval2[SET_VLEN_EACH];
#pragma _NEC vreg(leftval0)
#pragma _NEC vreg(leftval1)
#pragma _NEC vreg(leftval2)
#pragma _NEC vreg(rightval0)
#pragma _NEC vreg(rightval1)
#pragma _NEC vreg(rightval2)
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_0[j] != left_idx_stop_0[j] &&
         right_idx_0[j] != right_idx_stop_0[j]) {
        leftval0[j] = lvp[left_idx_0[j]];
        rightval0[j] = rvp[right_idx_0[j]];
      }
      if(left_idx_1[j] != left_idx_stop_1[j] &&
         right_idx_1[j] != right_idx_stop_1[j]) {
        leftval1[j] = lvp[left_idx_1[j]];
        rightval1[j] = rvp[right_idx_1[j]];
      }
      if(left_idx_2[j] != left_idx_stop_2[j] &&
         right_idx_2[j] != right_idx_stop_2[j]) {
        leftval2[j] = lvp[left_idx_2[j]];
        rightval2[j] = rvp[right_idx_2[j]];
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_0[j] != left_idx_stop_0[j] &&
         right_idx_0[j] != right_idx_stop_0[j]) {
        anyvalid = true;
        if(rightelm0[j] < leftelm0[j]) {
          op[out_idx_0[j]] = rightelm0[j];
          ovp[out_idx_0[j]] = rightval0[j];
          right_idx_0[j]++;
        } else {
          op[out_idx_0[j]] = leftelm0[j];
          ovp[out_idx_0[j]] = leftval0[j];
          left_idx_0[j]++;
        }
        out_idx_0[j]++;
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_1[j] != left_idx_stop_1[j] &&
         right_idx_1[j] != right_idx_stop_1[j]) {
        anyvalid = true;
        if(rightelm1[j] < leftelm1[j]) {
          op[out_idx_1[j]] = rightelm1[j];
          ovp[out_idx_1[j]] = rightval1[j];
          right_idx_1[j]++;
        } else {
          op[out_idx_1[j]] = leftelm1[j];
          ovp[out_idx_1[j]] = leftval1[j];
          left_idx_1[j]++;
        }
        out_idx_1[j]++;
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_2[j] != left_idx_stop_2[j] &&
         right_idx_2[j] != right_idx_stop_2[j]) {
        anyvalid = true;
        if(rightelm2[j] < leftelm2[j]) {
          op[out_idx_2[j]] = rightelm2[j];
          ovp[out_idx_2[j]] = rightval2[j];
          right_idx_2[j]++;
        } else {
          op[out_idx_2[j]] = leftelm2[j];
          ovp[out_idx_2[j]] = leftval2[j];
          left_idx_2[j]++;
        }
        out_idx_2[j]++;
      }
    }
  }
  for(size_t i = 0; i < SET_VLEN_EACH; i++) {
    out_idx[i] = out_idx_0[i];
    out_idx[SET_VLEN_EACH * 1 + i] = out_idx_1[i];
    out_idx[SET_VLEN_EACH * 2 + i] = out_idx_2[i];
    left_idx[i] = left_idx_0[i];
    left_idx[SET_VLEN_EACH * 1 + i] = left_idx_1[i];
    left_idx[SET_VLEN_EACH * 2 + i] = left_idx_2[i];
    right_idx[i] = right_idx_0[i];
    right_idx[SET_VLEN_EACH * 1 + i] = right_idx_1[i];
    right_idx[SET_VLEN_EACH * 2 + i] = right_idx_2[i];
  }
}

template <class T, class K>
void set_merge_pair(const T* leftp,
                    const K* left_valp,
                    size_t left_size,
                    const T* rightp,
                    const K* right_valp,
                    size_t right_size,
                    std::vector<T>& out,
                    std::vector<K>& out_val) {
  if(left_size == 0) {
    out.resize(right_size);
    auto outp = out.data();
    for(size_t i = 0; i < right_size; i++) outp[i] = rightp[i];
    out_val.resize(right_size);
    auto out_valp = out_val.data();
    for(size_t i = 0; i < right_size; i++) out_valp[i] = right_valp[i];
    return;
  }
  if(right_size == 0) {
    out.resize(left_size);
    auto outp = out.data();
    for(size_t i = 0; i < left_size; i++) outp[i] = leftp[i];
    out_val.resize(left_size);
    auto out_valp = out_val.data();
    for(size_t i = 0; i < left_size; i++) out_valp[i] = left_valp[i];
    return;
  }
  size_t each = ceil_div(left_size, size_t(SET_VLEN));
  if(each % 2 == 0) each++;
  
  size_t left_idx[SET_VLEN];
  size_t right_idx[SET_VLEN];
  size_t left_idx_stop[SET_VLEN];
  size_t right_idx_stop[SET_VLEN];
  size_t out_idx[SET_VLEN];
  out.resize(left_size + right_size);
  out_val.resize(left_size + right_size);
  auto outp = out.data();
  auto out_valp = out_val.data();
  for(int i = 0; i < SET_VLEN; i++) {
    size_t pos = each * i;
    if(pos < left_size) {
      left_idx[i] = pos;
    } else {
      left_idx[i] = left_size;
    }
  }

  advance_to_split_vreg(left_idx, left_size, leftp);

  T left_start[SET_VLEN];
  for(size_t i = 0; i < SET_VLEN; i++) {
    if(left_idx[i] < left_size) left_start[i] = leftp[left_idx[i]];
    else left_start[i] = 0;
  }
  lower_bound(rightp, right_size, left_start, SET_VLEN, right_idx);
  right_idx[0] = 0;
  for(size_t i = 0; i < SET_VLEN; i++) {
    if(left_idx[i] == left_size) right_idx[i] = right_size; // left is not valid
  }
  out_idx[0] = 0;
  for(int i = 1; i < SET_VLEN; i++) {
    out_idx[i] =
      (left_idx[i] - left_idx[i-1]) +
      (right_idx[i] - right_idx[i-1]) +
      out_idx[i-1];
  }
  for(int i = 0; i < SET_VLEN - 1; i++) {
    left_idx_stop[i] = left_idx[i + 1];
    right_idx_stop[i] = right_idx[i + 1];
  }
  left_idx_stop[SET_VLEN-1] = left_size;
  right_idx_stop[SET_VLEN-1] = right_size;
  set_merge_pair_vreg(leftp, left_valp, rightp, right_valp, outp, out_valp,
                      left_idx, right_idx, left_idx_stop, right_idx_stop,
                      out_idx);

  for(size_t i = 0; i < SET_VLEN; i++) {
    for(size_t j = 0; j < left_idx_stop[i] - left_idx[i]; j++) {
      outp[out_idx[i] + j] = leftp[left_idx[i] + j];
      out_valp[out_idx[i] + j] = left_valp[left_idx[i] + j];
    }
    for(size_t j = 0; j < right_idx_stop[i] - right_idx[i]; j++) {
      outp[out_idx[i] + j] = rightp[right_idx[i] + j];
      out_valp[out_idx[i] + j] = right_valp[right_idx[i] + j];
    }
  }
}

template <class T, class K>
void set_merge_pair_desc_vreg(const T* lp, const K* lvp,
                              const T* rp, const K* rvp,
                              T* op, K* ovp, size_t* left_idx, size_t* right_idx,
                              size_t* left_idx_stop, size_t* right_idx_stop,
                              size_t* out_idx) {
#include "set_operations.incl1"
  int anyvalid = true;
  while(anyvalid) {
    anyvalid = false;
#include "set_operations.incl2"
    K leftval0[SET_VLEN_EACH];
    K leftval1[SET_VLEN_EACH];
    K leftval2[SET_VLEN_EACH];
    K rightval0[SET_VLEN_EACH];
    K rightval1[SET_VLEN_EACH];
    K rightval2[SET_VLEN_EACH];
#pragma _NEC vreg(leftval0)
#pragma _NEC vreg(leftval1)
#pragma _NEC vreg(leftval2)
#pragma _NEC vreg(rightval0)
#pragma _NEC vreg(rightval1)
#pragma _NEC vreg(rightval2)
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_0[j] != left_idx_stop_0[j] &&
         right_idx_0[j] != right_idx_stop_0[j]) {
        leftval0[j] = lvp[left_idx_0[j]];
        rightval0[j] = rvp[right_idx_0[j]];
      }
      if(left_idx_1[j] != left_idx_stop_1[j] &&
         right_idx_1[j] != right_idx_stop_1[j]) {
        leftval1[j] = lvp[left_idx_1[j]];
        rightval1[j] = rvp[right_idx_1[j]];
      }
      if(left_idx_2[j] != left_idx_stop_2[j] &&
         right_idx_2[j] != right_idx_stop_2[j]) {
        leftval2[j] = lvp[left_idx_2[j]];
        rightval2[j] = rvp[right_idx_2[j]];
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_0[j] != left_idx_stop_0[j] &&
         right_idx_0[j] != right_idx_stop_0[j]) {
        anyvalid = true;
        if(leftelm0[j] < rightelm0[j]) {
          op[out_idx_0[j]] = rightelm0[j];
          ovp[out_idx_0[j]] = rightval0[j];
          right_idx_0[j]++;
        } else {
          op[out_idx_0[j]] = leftelm0[j];
          ovp[out_idx_0[j]] = leftval0[j];
          left_idx_0[j]++;
        }
        out_idx_0[j]++;
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_1[j] != left_idx_stop_1[j] &&
         right_idx_1[j] != right_idx_stop_1[j]) {
        anyvalid = true;
        if(leftelm1[j] < rightelm1[j]) {
          op[out_idx_1[j]] = rightelm1[j];
          ovp[out_idx_1[j]] = rightval1[j];
          right_idx_1[j]++;
        } else {
          op[out_idx_1[j]] = leftelm1[j];
          ovp[out_idx_1[j]] = leftval1[j];
          left_idx_1[j]++;
        }
        out_idx_1[j]++;
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(left_idx_2[j] != left_idx_stop_2[j] &&
         right_idx_2[j] != right_idx_stop_2[j]) {
        anyvalid = true;
        if(leftelm2[j] < rightelm2[j]) {
          op[out_idx_2[j]] = rightelm2[j];
          ovp[out_idx_2[j]] = rightval2[j];
          right_idx_2[j]++;
        } else {
          op[out_idx_2[j]] = leftelm2[j];
          ovp[out_idx_2[j]] = leftval2[j];
          left_idx_2[j]++;
        }
        out_idx_2[j]++;
      }
    }
  }
  for(size_t i = 0; i < SET_VLEN_EACH; i++) {
    out_idx[i] = out_idx_0[i];
    out_idx[SET_VLEN_EACH * 1 + i] = out_idx_1[i];
    out_idx[SET_VLEN_EACH * 2 + i] = out_idx_2[i];
    left_idx[i] = left_idx_0[i];
    left_idx[SET_VLEN_EACH * 1 + i] = left_idx_1[i];
    left_idx[SET_VLEN_EACH * 2 + i] = left_idx_2[i];
    right_idx[i] = right_idx_0[i];
    right_idx[SET_VLEN_EACH * 1 + i] = right_idx_1[i];
    right_idx[SET_VLEN_EACH * 2 + i] = right_idx_2[i];
  }
}

template <class T, class K>
void set_merge_pair_desc(const T* leftp,
                         const K* left_valp,
                         size_t left_size,
                         const T* rightp,
                         const K* right_valp,
                         size_t right_size,
                         std::vector<T>& out,
                         std::vector<K>& out_val) {
  if(left_size == 0) {
    out.resize(right_size);
    auto outp = out.data();
    for(size_t i = 0; i < right_size; i++) outp[i] = rightp[i];
    out_val.resize(right_size);
    auto out_valp = out_val.data();
    for(size_t i = 0; i < right_size; i++) out_valp[i] = right_valp[i];
    return;
  }
  if(right_size == 0) {
    out.resize(left_size);
    auto outp = out.data();
    for(size_t i = 0; i < left_size; i++) outp[i] = leftp[i];
    out_val.resize(left_size);
    auto out_valp = out_val.data();
    for(size_t i = 0; i < left_size; i++) out_valp[i] = left_valp[i];
    return;
  }
  size_t each = ceil_div(left_size, size_t(SET_VLEN));
  if(each % 2 == 0) each++;
  
  size_t left_idx[SET_VLEN];
  size_t right_idx[SET_VLEN];
  size_t left_idx_stop[SET_VLEN];
  size_t right_idx_stop[SET_VLEN];
  size_t out_idx[SET_VLEN];
  out.resize(left_size + right_size);
  out_val.resize(left_size + right_size);
  auto outp = out.data();
  auto out_valp = out_val.data();
  for(int i = 0; i < SET_VLEN; i++) {
    size_t pos = each * i;
    if(pos < left_size) {
      left_idx[i] = pos;
    } else {
      left_idx[i] = left_size;
    }
  }

  advance_to_split_vreg(left_idx, left_size, leftp);

  T left_start[SET_VLEN];
  for(size_t i = 0; i < SET_VLEN; i++) {
    if(left_idx[i] < left_size) left_start[i] = leftp[left_idx[i]];
    else left_start[i] = 0;
  }
  lower_bound_desc(rightp, right_size, left_start, SET_VLEN, right_idx);
  right_idx[0] = 0;
  for(size_t i = 0; i < SET_VLEN; i++) {
    if(left_idx[i] == left_size) right_idx[i] = right_size; // left is not valid
  }
  out_idx[0] = 0;
  for(int i = 1; i < SET_VLEN; i++) {
    out_idx[i] =
      (left_idx[i] - left_idx[i-1]) +
      (right_idx[i] - right_idx[i-1]) +
      out_idx[i-1];
  }
  for(int i = 0; i < SET_VLEN - 1; i++) {
    left_idx_stop[i] = left_idx[i + 1];
    right_idx_stop[i] = right_idx[i + 1];
  }
  left_idx_stop[SET_VLEN-1] = left_size;
  right_idx_stop[SET_VLEN-1] = right_size;
  set_merge_pair_desc_vreg(leftp, left_valp, rightp, right_valp, outp, out_valp,
                           left_idx, right_idx, left_idx_stop, right_idx_stop,
                           out_idx);

  for(size_t i = 0; i < SET_VLEN; i++) {
    for(size_t j = 0; j < left_idx_stop[i] - left_idx[i]; j++) {
      outp[out_idx[i] + j] = leftp[left_idx[i] + j];
      out_valp[out_idx[i] + j] = left_valp[left_idx[i] + j];
    }
    for(size_t j = 0; j < right_idx_stop[i] - right_idx[i]; j++) {
      outp[out_idx[i] + j] = rightp[right_idx[i] + j];
      out_valp[out_idx[i] + j] = right_valp[right_idx[i] + j];
    }
  }
}

template <class T>
void set_separate_vreg(const T* keyp, size_t* outp, size_t* out_idx,
                       size_t size, size_t each, size_t rest, T* current_key) {

  size_t out_idx_vreg[SET_VLEN_EACH];
  T current_key_vreg[SET_VLEN_EACH];
#pragma _NEC vreg(out_idx_vreg)
#pragma _NEC vreg(current_key_vreg)
  for(size_t i = 0; i < SET_VLEN_EACH; i++){
    out_idx_vreg[i] = out_idx[i];
    current_key_vreg[i] = current_key[i];
  }

  for(size_t j = 0; j < each; j++) {
#pragma _NEC ivdep
#pragma _NEC vovertake
    for(size_t i = 0; i < SET_VLEN_EACH; i++) {
      auto loaded_key = keyp[i * each + j];
      if(loaded_key != current_key_vreg[i]) {
        outp[out_idx_vreg[i]++] = i * each + j;
        current_key_vreg[i] = loaded_key;
      }
    }
  }
  auto offset = SET_VLEN_EACH * each;
  auto out_idx_rest = out_idx[SET_VLEN_EACH];
  auto current_key_rest = current_key[SET_VLEN_EACH];
#pragma _NEC ivdep
  for(size_t i = 0; i < rest; i++) {
    auto loaded_key = keyp[offset + i];
    if(loaded_key != current_key_rest) {
      outp[out_idx_rest++] = offset + i;
      current_key_rest = loaded_key;
    }
  }
  for(size_t i = 0; i < SET_VLEN_EACH; i++) {
    out_idx[i] = out_idx_vreg[i];
  }
  out_idx[SET_VLEN_EACH] = out_idx_rest;
}

template <class T>
std::vector<size_t> set_separate(const T* keyp, size_t size) {
  if(size == 0) {return std::vector<size_t>(1);} 
  size_t each = size / SET_VLEN_EACH;
  if(each % 2 == 0 && each > 0) each--;
  size_t rest = size - each * SET_VLEN_EACH;
  std::vector<size_t> out(size);
  auto outp = out.data();
  if(each == 0) {
    auto current = keyp[0];
    outp[0] = 0;
    size_t out_idx = 1;
    for(size_t i = 1; i < size; i++) {
      auto loaded = keyp[i];
      if(loaded != current) {
        outp[out_idx++] = i;
        current = loaded;
      }
    }
    std::vector<size_t> ret(out_idx+1);
    auto retp = ret.data();
    for(size_t i = 0; i < out_idx; i++) {
      retp[i] = outp[i];
    }
    retp[out_idx] = size;
    return ret;
  } else {
    size_t out_idx[SET_VLEN_EACH+1];
    T current_key[SET_VLEN_EACH+1];
    current_key[0] = keyp[0]; // size > 0
    for(int i = 1; i < SET_VLEN_EACH+1; i++) {
      current_key[i] = keyp[each * i - 1]; // each > 0
    }
    for(int i = 0; i < SET_VLEN_EACH+1; i++) {
      out_idx[i] = each * i;
    }

    set_separate_vreg(keyp, outp, out_idx, size, each, rest, current_key);

    size_t total = 0;
    for(size_t i = 0; i < SET_VLEN_EACH+1; i++) {
      total += out_idx[i] - each * i;
    }
    std::vector<size_t> ret(total+2);
    size_t* retp = ret.data();
    size_t current = 0;
    for(size_t i = 0; i < SET_VLEN_EACH+1; i++) {
      auto crnt_size = out_idx[i] - each * i;
      for(size_t j = 0; j < crnt_size; j++) {
        retp[current + j + 1] = out[each * i + j];
      }
      current += crnt_size;
    }
    retp[current+1] = size;
    return ret;
  }
}

template <class T>
void set_unique_vreg(const T* keyp, T* outp, size_t* out_idx,
                     size_t size, size_t each, size_t rest, T* current_key) {

  size_t out_idx_vreg[SET_VLEN_EACH];
  T current_key_vreg[SET_VLEN_EACH];
#pragma _NEC vreg(out_idx_vreg)
#pragma _NEC vreg(current_key_vreg)
  for(size_t i = 0; i < SET_VLEN_EACH; i++){
    out_idx_vreg[i] = out_idx[i];
    current_key_vreg[i] = current_key[i];
  }

  for(size_t j = 0; j < each; j++) {
#pragma _NEC ivdep
#pragma _NEC vovertake
    for(size_t i = 0; i < SET_VLEN_EACH; i++) {
      auto loaded_key = keyp[i * each + j];
      if(loaded_key != current_key_vreg[i]) {
        outp[out_idx_vreg[i]++] = loaded_key;
        current_key_vreg[i] = loaded_key;
      }
    }
  }
  auto offset = SET_VLEN_EACH * each;
  auto out_idx_rest = out_idx[SET_VLEN_EACH];
  auto current_key_rest = current_key[SET_VLEN_EACH];
#pragma _NEC ivdep
  for(size_t i = 0; i < rest; i++) {
    auto loaded_key = keyp[offset + i];
    if(loaded_key != current_key_rest) {
      outp[out_idx_rest++] = loaded_key;
      current_key_rest = loaded_key;
    }
  }
  for(size_t i = 0; i < SET_VLEN_EACH; i++) {
    out_idx[i] = out_idx_vreg[i];
  }
  out_idx[SET_VLEN_EACH] = out_idx_rest;
}

template <class T>
std::vector<T> set_unique(const T* keyp, size_t size) {
  if(size == 0) {return std::vector<T>();} 
  size_t each = size / SET_VLEN_EACH;
  if(each % 2 == 0 && each > 0) each--;
  size_t rest = size - each * SET_VLEN_EACH;
  std::vector<T> out(size);
  auto outp = out.data();
  if(each == 0) {
    auto current = keyp[0];
    outp[0] = current;
    size_t out_idx = 1;
    for(size_t i = 1; i < size; i++) {
      auto loaded = keyp[i];
      if(loaded != current) {
        outp[out_idx++] = loaded;
        current = loaded;
      }
    }
    std::vector<T> ret(out_idx);
    auto retp = ret.data();
    for(size_t i = 0; i < out_idx; i++) {
      retp[i] = outp[i];
    }
    return ret;
  } else {
    size_t out_idx[SET_VLEN_EACH+1];
    T current_key[SET_VLEN_EACH+1];
    current_key[0] = keyp[0]; // size > 0
    for(int i = 1; i < SET_VLEN_EACH+1; i++) {
      current_key[i] = keyp[each * i - 1]; // each > 0
    }
    for(int i = 0; i < SET_VLEN_EACH+1; i++) {
      out_idx[i] = each * i;
    }

    set_unique_vreg(keyp, outp, out_idx, size, each, rest, current_key);

    size_t total = 0;
    for(size_t i = 0; i < SET_VLEN_EACH+1; i++) {
      total += out_idx[i] - each * i;
    }
    std::vector<T> ret(total+1);
    auto* retp = ret.data();
    retp[0] = keyp[0];
    size_t current = 0;
    for(size_t i = 0; i < SET_VLEN_EACH+1; i++) {
      auto crnt_size = out_idx[i] - each * i;
      for(size_t j = 0; j < crnt_size; j++) {
        retp[current + j + 1] = out[each * i + j];
      }
      current += crnt_size;
    }
    return ret;
  }
}

template <class T>
int set_is_unique_vreg(const T* keyp, size_t size, size_t each, size_t rest,
                       T* current_key) {
  T current_key_vreg[SET_VLEN_EACH];
#pragma _NEC vreg(current_key_vreg)
  for(size_t i = 0; i < SET_VLEN_EACH; i++){
    current_key_vreg[i] = current_key[i];
  }

  for(size_t j = 0; j < each; j++) {
#pragma _NEC ivdep
#pragma _NEC vovertake
    for(size_t i = 0; i < SET_VLEN_EACH; i++) {
      auto loaded_key = keyp[i * each + j];
      if(loaded_key == current_key_vreg[i]) {
        return false;
      } else {
        current_key_vreg[i] = loaded_key;
      }
    }
  }
  auto offset = SET_VLEN_EACH * each;
  auto current_key_rest = current_key[SET_VLEN_EACH];
#pragma _NEC ivdep
  for(size_t i = 0; i < rest; i++) {
    auto loaded_key = keyp[offset + i];
    if(loaded_key == current_key_rest) {
      return false;
    } else {
      current_key_rest = loaded_key;
    }
  }
  return true;
}

template <class T>
int set_is_unique(const T* keyp, size_t size) {
  if(size == 0) {return true;} 
  size_t size2 = size - 1;
  size_t each = size2 / SET_VLEN_EACH;
  if(each % 2 == 0 && each > 0) each--;
  size_t rest = size2 - each * SET_VLEN_EACH;
  std::vector<T> out(size);
  auto outp = out.data();
  if(each == 0) {
    auto current = keyp[0];
    outp[0] = current;
    for(size_t i = 1; i < size; i++) {
      auto loaded = keyp[i];
      if(loaded == current) {
        return false;
      } else {
        current = loaded;
      }
    }
    return true;
  } else {
    T current_key[SET_VLEN_EACH+1];
    current_key[0] = keyp[0]; // size > 0
    for(int i = 1; i < SET_VLEN_EACH+1; i++) {
      current_key[i] = keyp[each * i - 1]; // each > 0
    }
    return set_is_unique_vreg(keyp+1, size2, each, rest, current_key);
  }
}

#endif // _SX

template <class T>
std::vector<T> set_intersection(const std::vector<T>& left,
                                const std::vector<T>& right) {
  return set_intersection(left.data(), left.size(),
                          right.data(), right.size());
}

template <class T>
std::vector<T> set_union(const std::vector<T>& left,
                         const std::vector<T>& right) {
  return set_union(left.data(), left.size(),
                   right.data(), right.size());
}

template <class T>
std::vector<T> set_difference(const std::vector<T>& left,
                              const std::vector<T>& right) {
  return set_difference(left.data(), left.size(),
                        right.data(), right.size());
}

template <class T>
std::vector<T> set_merge(const std::vector<T>& left,
                         const std::vector<T>& right) {
  return set_merge(left.data(), left.size(),
                   right.data(), right.size());
}

template <class T>
std::vector<T> set_merge_desc(const std::vector<T>& left,
                              const std::vector<T>& right) {
  return set_merge_desc(left.data(), left.size(),
                        right.data(), right.size());
}

template <class T, class K>
void set_merge_pair(const std::vector<T>& left,
                    const std::vector<K>& left_val,
                    const std::vector<T>& right,
                    const std::vector<K>& right_val,
                    std::vector<T>& out,
                    std::vector<K>& out_val) {
  set_merge_pair(left.data(), left_val.data(), left.size(),
                 right.data(), right_val.data(), right.size(),
                 out, out_val);
}

template <class T, class K>
void set_merge_pair_desc(const std::vector<T>& left,
                         const std::vector<K>& left_val,
                         const std::vector<T>& right,
                         const std::vector<K>& right_val,
                         std::vector<T>& out,
                         std::vector<K>& out_val) {
  set_merge_pair_desc(left.data(), left_val.data(), left.size(),
                      right.data(), right_val.data(), right.size(),
                      out, out_val);
}

template <class T>
std::vector<size_t> set_separate(const std::vector<T>& key) {
  return set_separate(key.data(), key.size());
}

template <class T>
std::vector<T> set_unique(const std::vector<T>& key) {
  return set_unique(key.data(), key.size());
}

template <class T>
int set_is_unique(const std::vector<T>& key) {
  return set_is_unique(key.data(), key.size());
}

template <class T, class K>
void set_multimerge_pair(std::vector<std::vector<T>>& key,
                         std::vector<std::vector<K>>& val,
                         std::vector<T>& out_key,
                         std::vector<K>& out_val) {
  size_t part_size = key.size();
  if(part_size == 0) {
    out_key = std::vector<T>();
    out_val = std::vector<K>();
  } else if(part_size == 1) {
    out_key.swap(key[0]);
    out_val.swap(val[0]);
  } else if(part_size == 2) {
    set_merge_pair(key[0], val[0], key[1], val[1], out_key, out_val);
  } else {
    size_t next_part_size = ceil_div(part_size, size_t(2));
    std::vector<std::vector<T>> key_left(next_part_size);
    std::vector<std::vector<K>> val_left(next_part_size);
    for(size_t i = 0; i < next_part_size; i++) {
      key_left[i].swap(key[i]);
      val_left[i].swap(val[i]);
    }
    std::vector<T> out_key_left;
    std::vector<K> out_val_left;
    set_multimerge_pair(key_left, val_left, out_key_left, out_val_left);
    std::vector<std::vector<T>> key_right(part_size - next_part_size);
    std::vector<std::vector<K>> val_right(part_size - next_part_size);
    for(size_t i = 0; i < part_size - next_part_size; i++) {
      key_right[i].swap(key[i+next_part_size]);
      val_right[i].swap(val[i+next_part_size]);
    }
    std::vector<T> out_key_right;
    std::vector<K> out_val_right;
    set_multimerge_pair(key_right, val_right, out_key_right, out_val_right);
    set_merge_pair(out_key_left, out_val_left, out_key_right, out_val_right,
                   out_key, out_val);
                   
  }
}

template <class T, class K>
void set_multimerge_pair_desc(std::vector<std::vector<T>>& key,
                              std::vector<std::vector<K>>& val,
                              std::vector<T>& out_key,
                              std::vector<K>& out_val) {
  size_t part_size = key.size();
  if(part_size == 0) {
    out_key = std::vector<T>();
    out_val = std::vector<K>();
  } else if(part_size == 1) {
    out_key.swap(key[0]);
    out_val.swap(val[0]);
  } else if(part_size == 2) {
    set_merge_pair_desc(key[0], val[0], key[1], val[1], out_key, out_val);
  } else {
    size_t next_part_size = ceil_div(part_size, size_t(2));
    std::vector<std::vector<T>> key_left(next_part_size);
    std::vector<std::vector<K>> val_left(next_part_size);
    for(size_t i = 0; i < next_part_size; i++) {
      key_left[i].swap(key[i]);
      val_left[i].swap(val[i]);
    }
    std::vector<T> out_key_left;
    std::vector<K> out_val_left;
    set_multimerge_pair_desc(key_left, val_left, out_key_left, out_val_left);
    std::vector<std::vector<T>> key_right(part_size - next_part_size);
    std::vector<std::vector<K>> val_right(part_size - next_part_size);
    for(size_t i = 0; i < part_size - next_part_size; i++) {
      key_right[i].swap(key[i+next_part_size]);
      val_right[i].swap(val[i+next_part_size]);
    }
    std::vector<T> out_key_right;
    std::vector<K> out_val_right;
    set_multimerge_pair_desc(key_right, val_right, out_key_right, 
                             out_val_right);
    set_merge_pair_desc(out_key_left, out_val_left, out_key_right, 
                        out_val_right, out_key, out_val);
  }
}


template <class T>
void
set_multimerge(std::vector<std::vector<T>>& key,  // destructive
               std::vector<T>& out) {
  auto way = key.size();
  auto step = 1;
  while(step < way) {
    for(size_t i = 0; i < way; i+=step*2) {
      if(i + step < way)
        key[i] = set_merge(key[i], key[i+step]);
    }
    step *= 2;
  }
  out.swap(key[0]);
}

template <class T>
void
set_multimerge_desc(std::vector<std::vector<T>>& key, 
                    std::vector<T>& out) {
  auto way = key.size();
  auto step = 1;
  while(step < way) {
    for(size_t i = 0; i < way; i+=step*2) {
      if(i + step < way)
        key[i] = set_merge_desc(key[i], key[i+step]);
    }
    step *= 2;
  }
  out.swap(key[0]);
}
  
}
#endif
