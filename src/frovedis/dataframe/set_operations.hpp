#ifndef SET_OPERATIONS_HPP
#define SET_OPERATIONS_HPP

#include <vector>
#include <algorithm>
#include <utility>
#include "../core/utility.hpp"
#include "../core/radix_sort.hpp" // for reverse

#define SET_VLEN 1023
#define SET_VLEN_EACH 256
#define SET_VLEN_EACH3 255

namespace frovedis {

#if !(defined(_SX) || defined(__ve__))
template <class T>
std::vector<T> set_intersection(std::vector<T>& left, std::vector<T>& right) {
  std::vector<T> ret;
  std::set_intersection(left.begin(), left.end(),
                        right.begin(), right.end(),
                        std::back_inserter(ret));
  return ret;
}

template <class T>
std::vector<T> set_union(std::vector<T>& left, std::vector<T>& right) {
  std::vector<T> ret;
  std::set_union(left.begin(), left.end(),
                 right.begin(), right.end(),
                 std::back_inserter(ret));
  return ret;
}

template <class T>
std::vector<T> set_difference(std::vector<T>& left, std::vector<T>& right) {
  std::vector<T> ret;
  std::set_difference(left.begin(), left.end(),
                      right.begin(), right.end(),
                      std::back_inserter(ret));
  return ret;
}

template <class T>
std::vector<T> set_merge(std::vector<T>& left, std::vector<T>& right) {
  std::vector<T> ret;
  std::merge(left.begin(), left.end(),
             right.begin(), right.end(),
             std::back_inserter(ret));
  return ret;
}

template <class T>
std::vector<T> set_merge_desc(std::vector<T>& left, std::vector<T>& right) {
  std::vector<T> ret;
  std::merge(left.begin(), left.end(),
             right.begin(), right.end(),
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
void set_merge_pair(std::vector<T>& left, std::vector<K>& left_val,
                    std::vector<T>& right, std::vector<K>& right_val,
                    std::vector<T>& out, std::vector<K>& out_val) {
  size_t left_size = left.size();
  size_t right_size = right.size();
  std::vector<std::pair<T,K>> left_pair(left_size);
  std::vector<std::pair<T,K>> right_pair(right_size);
  std::vector<std::pair<T,K>> out_pair;
  for(size_t i = 0; i < left_size; i++) {
    left_pair[i].first = left[i];
    left_pair[i].second = left_val[i];
  }
  for(size_t i = 0; i < right_size; i++) {
    right_pair[i].first = right[i];
    right_pair[i].second = right_val[i];
  }
  std::merge(left_pair.begin(), left_pair.end(),
             right_pair.begin(), right_pair.end(),
             std::back_inserter(out_pair),set_merge_pair_helper<T,K>());
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
void set_merge_pair_desc(std::vector<T>& left, std::vector<K>& left_val,
                         std::vector<T>& right, std::vector<K>& right_val,
                         std::vector<T>& out, std::vector<K>& out_val) {
  size_t left_size = left.size();
  size_t right_size = right.size();
  std::vector<std::pair<T,K>> left_pair(left_size);
  std::vector<std::pair<T,K>> right_pair(right_size);
  std::vector<std::pair<T,K>> out_pair;
  for(size_t i = 0; i < left_size; i++) {
    left_pair[i].first = left[i];
    left_pair[i].second = left_val[i];
  }
  for(size_t i = 0; i < right_size; i++) {
    right_pair[i].first = right[i];
    right_pair[i].second = right_val[i];
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
std::vector<size_t> set_separate(const std::vector<T>& key) {
  size_t size = key.size();
  std::vector<size_t> ret;
  if(size == 0) {
    ret.push_back(0);
    return ret;
  } else {
    T current = key[0];
    ret.push_back(0);
    for(size_t i = 1; i < size; i++) {
      if(key[i] != current) {
        current = key[i];
        ret.push_back(i);
      }
    }
    ret.push_back(size);
    return ret;
  }
}

template <class T>
std::vector<T> set_unique(const std::vector<T>& key) {
  size_t size = key.size();
  std::vector<T> ret;
  if(size == 0) {
    return ret;
  } else {
    T current = key[0];
    ret.push_back(current);
    for(size_t i = 1; i < size; i++) {
      if(key[i] != current) {
        current = key[i];
        ret.push_back(current);
      }
    }
    return ret;
  }
}

template <class T>
int set_is_unique(const std::vector<T>& key) {
  size_t size = key.size();
  std::vector<size_t> ret;
  if(size == 0) {
    return true;
  } else {
    T current = key[0];
    for(size_t i = 1; i < size; i++) {
      if(key[i] != current) current = key[i];
      else return false;
    }
    return true;
  }
}

#else // SX

template <class T>
std::vector<T> set_intersection(std::vector<T>& left, std::vector<T>& right) {
  int valid[SET_VLEN];
  for(int i = 0; i < SET_VLEN; i++) valid[i] = true;

  size_t left_size = left.size();
  size_t right_size = right.size();
  if(left_size == 0 || right_size == 0) return std::vector<T>();

  size_t each = ceil_div(left_size, size_t(SET_VLEN));
  if(each % 2 == 0) each++;
  
  size_t left_idx[SET_VLEN];
  size_t right_idx[SET_VLEN];
  size_t left_idx_stop[SET_VLEN];
  size_t right_idx_stop[SET_VLEN];
  size_t out_idx[SET_VLEN];
  size_t out_idx_save[SET_VLEN];
  std::vector<T> out;
  out.resize(left_size);
  for(int i = 0; i < SET_VLEN; i++) {
    size_t pos = each * i;
    if(pos < left_size) {
      left_idx[i] = pos;
      out_idx[i] = pos;
      out_idx_save[i] = pos;
    } else {
      valid[i] = false;
      left_idx[i] = left_size;
      out_idx[i] = left_size;
      out_idx_save[i] = left_size;
    }
  }
  for(int i = 0; i < SET_VLEN; i++) {
    if(valid[i]) {
      auto it = lower_bound(right.begin(), right.end(), left[left_idx[i]]);
      if(it != right.end()) {
        right_idx[i] = it - right.begin();
      } else {
        valid[i] = false;
        right_idx[i] = right_size;
      }
    } else {
      right_idx[i] = right_size;
    }
  }
  for(int i = 0; i < SET_VLEN-1; i++) {
    if(valid[i]) {
      auto it = upper_bound(right.begin(), right.end(), left[left_idx[i+1]-1]);
      if(it != right.end()) {
        right_idx_stop[i] = it - right.begin();
      } else {
        right_idx_stop[i] = right_size;
      }
    } else {
      right_idx_stop[i] = right_size;
    }
  }
  if(valid[SET_VLEN-1]) {
    auto it = upper_bound(right.begin(), right.end(), left[left_size-1]);
    if(it != right.end()) {
      right_idx_stop[SET_VLEN-1] = it - right.begin();
    } else {
      valid[SET_VLEN-1] = false;
      right_idx_stop[SET_VLEN-1] = right_size;
    }
  } else {
    right_idx_stop[SET_VLEN-1] = right_size;
  }
  for(int i = 0; i < SET_VLEN - 1; i++) {
    left_idx_stop[i] = left_idx[i + 1];
  }
  left_idx_stop[SET_VLEN-1] = left_size;
  right_idx_stop[SET_VLEN-1] = right_size;
  for(int i = 0; i < SET_VLEN; i++) {
    if(right_idx[i] == right_idx_stop[i]) valid[i] = false;
  }
#include "set_operations.incl1" // decl/init vregs
  T* lp = &left[0];
  T* rp = &right[0];
  T* op = &out[0];
  while(1) {
#include "set_operations.incl2" // decl/load left/rightelm
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_0[j]) {
        int eq = leftelm0[j] == rightelm0[j];
        int lt = leftelm0[j] < rightelm0[j];
        if(eq) {
          op[out_idx_0[j]++] = leftelm0[j];
        }
        if(eq || lt) {
          left_idx_0[j]++;
        }
        if(eq || !lt) {
          right_idx_0[j]++;
        }
        if(left_idx_0[j] == left_idx_stop_0[j] ||
           right_idx_0[j] == right_idx_stop_0[j]) {
          valid_0[j] = false;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_1[j]) {
        int eq = leftelm1[j] == rightelm1[j];
        int lt = leftelm1[j] < rightelm1[j];
        if(eq) {
          op[out_idx_1[j]++] = leftelm1[j];
        }
        if(eq || lt) {
          left_idx_1[j]++;
        }
        if(eq || !lt) {
          right_idx_1[j]++;
        }
        if(left_idx_1[j] == left_idx_stop_1[j] ||
           right_idx_1[j] == right_idx_stop_1[j]) {
          valid_1[j] = false;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_2[j]) {
        int eq = leftelm2[j] == rightelm2[j];
        int lt = leftelm2[j] < rightelm2[j];
        if(eq) {
          op[out_idx_2[j]++] = leftelm2[j];
        }
        if(eq || lt) {
          left_idx_2[j]++;
        }
        if(eq || !lt) {
          right_idx_2[j]++;
        }
        if(left_idx_2[j] == left_idx_stop_2[j] ||
           right_idx_2[j] == right_idx_stop_2[j]) {
          valid_2[j] = false;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH3; j++) {
      if(valid_3[j]) {
        int eq = leftelm3[j] == rightelm3[j];
        int lt = leftelm3[j] < rightelm3[j];
        if(eq) {
          op[out_idx_3[j]++] = leftelm3[j];
        }
        if(eq || lt) {
          left_idx_3[j]++;
        }
        if(eq || !lt) {
          right_idx_3[j]++;
        }
        if(left_idx_3[j] == left_idx_stop_3[j] ||
           right_idx_3[j] == right_idx_stop_3[j]) {
          valid_3[j] = false;
        }
      }
    }
    int any_valid = false;
    for(int i = 0; i < SET_VLEN_EACH; i++) {
      if(valid_0[i] || valid_1[i] || valid_2[i] || valid_3[i])
        any_valid = true;
    }
    if(any_valid == false) break;
  }
  for(size_t i = 0; i < SET_VLEN_EACH; i++) {
    out_idx[i] = out_idx_0[i];
    out_idx[SET_VLEN_EACH * 1 + i] = out_idx_1[i];
    out_idx[SET_VLEN_EACH * 2 + i] = out_idx_2[i];
  }
  for(size_t i = 0; i < SET_VLEN_EACH3; i++) {
    out_idx[SET_VLEN_EACH * 3 + i] = out_idx_3[i];
  }
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
      retp[current + j] = out[out_idx_save[i] + j];
    }
    current += out_idx[i] - out_idx_save[i];
  }
  return ret;
}

template <class T>
std::vector<T> set_union(std::vector<T>& left, std::vector<T>& right) {
  int valid[SET_VLEN];
  for(int i = 0; i < SET_VLEN; i++) valid[i] = true;

  size_t left_size = left.size();
  size_t right_size = right.size();
  if(left_size == 0) return right;
  if(right_size == 0) return left;

  size_t each = ceil_div(left_size, size_t(SET_VLEN));
  if(each % 2 == 0) each++;
  
  size_t left_idx[SET_VLEN];
  size_t right_idx[SET_VLEN];
  size_t left_idx_stop[SET_VLEN];
  size_t right_idx_stop[SET_VLEN];
  size_t out_idx[SET_VLEN];
  size_t out_idx_save[SET_VLEN];
  std::vector<T> out;
  out.resize(left_size + right_size);
  for(int i = 0; i < SET_VLEN; i++) {
    size_t pos = each * i;
    if(pos < left_size) {
      left_idx[i] = pos;
    } else {
      valid[i] = false;
      left_idx[i] = left_size;
    }
  }
  // need to split by different data for union
  for(int i = 0; i < SET_VLEN-1; i++) {
    if(valid[i]) {
      T last = left[left_idx[i+1]-1];
      while(true) {
        if(left_idx[i+1] < left_size - 1 && left[left_idx[i+1]] == last) {
          left_idx[i+1]++;
        } else break;
      }
    }
  }
  right_idx[0] = 0;
  for(int i = 1; i < SET_VLEN; i++) {
    if(valid[i]) {
      auto it = lower_bound(right.begin(), right.end(), left[left_idx[i]]);
      if(it != right.end()) {
        right_idx[i] = it - right.begin();
      } else {
        valid[i] = false;
        right_idx[i] = right_size;
      }
    } else {
      right_idx[i] = right_size;
    }
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
  for(int i = 0; i < SET_VLEN; i++) {
    if(left_idx[i] == left_idx_stop[i] || right_idx[i] == right_idx_stop[i]) 
      valid[i] = false;
  }
#include "set_operations.incl1"
  T* lp = &left[0];
  T* rp = &right[0];
  T* op = &out[0];
  while(1) {
#include "set_operations.incl2"
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_0[j]) {
        int eq = leftelm0[j] == rightelm0[j];
        int lt = leftelm0[j] < rightelm0[j];
        if(eq || lt) {
          op[out_idx_0[j]++] = leftelm0[j];
          left_idx_0[j]++;
        } else {
          op[out_idx_0[j]++] = rightelm0[j];
          right_idx_0[j]++;
        }
        if(eq) {
          right_idx_0[j]++;
        }
        if(left_idx_0[j] == left_idx_stop_0[j] ||
           right_idx_0[j] == right_idx_stop_0[j]) {
          valid_0[j] = false;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_1[j]) {
        int eq = leftelm1[j] == rightelm1[j];
        int lt = leftelm1[j] < rightelm1[j];
        if(eq || lt) {
          op[out_idx_1[j]++] = leftelm1[j];
          left_idx_1[j]++;
        } else {
          op[out_idx_1[j]++] = rightelm1[j];
          right_idx_1[j]++;
        }
        if(eq) {
          right_idx_1[j]++;
        }
        if(left_idx_1[j] == left_idx_stop_1[j] ||
           right_idx_1[j] == right_idx_stop_1[j]) {
          valid_1[j] = false;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_2[j]) {
        int eq = leftelm2[j] == rightelm2[j];
        int lt = leftelm2[j] < rightelm2[j];
        if(eq || lt) {
          op[out_idx_2[j]++] = leftelm2[j];
          left_idx_2[j]++;
        } else {
          op[out_idx_2[j]++] = rightelm2[j];
          right_idx_2[j]++;
        }
        if(eq) {
          right_idx_2[j]++;
        }
        if(left_idx_2[j] == left_idx_stop_2[j] ||
           right_idx_2[j] == right_idx_stop_2[j]) {
          valid_2[j] = false;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH3; j++) {
      if(valid_3[j]) {
        int eq = leftelm3[j] == rightelm3[j];
        int lt = leftelm3[j] < rightelm3[j];
        if(eq || lt) {
          op[out_idx_3[j]++] = leftelm3[j];
          left_idx_3[j]++;
        } else {
          op[out_idx_3[j]++] = rightelm3[j];
          right_idx_3[j]++;
        }
        if(eq) {
          right_idx_3[j]++;
        }
        if(left_idx_3[j] == left_idx_stop_3[j] ||
           right_idx_3[j] == right_idx_stop_3[j]) {
          valid_3[j] = false;
        }
      }
    }
    int any_valid = false;
    for(int i = 0; i < SET_VLEN_EACH; i++) {
      if(valid_0[i] || valid_1[i] || valid_2[i] || valid_3[i])
        any_valid = true;
    }
    if(any_valid == false) break;
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
  for(size_t i = 0; i < SET_VLEN_EACH3; i++) {
    out_idx[SET_VLEN_EACH * 3 + i] = out_idx_3[i];
    left_idx[SET_VLEN_EACH * 3 + i] = left_idx_3[i];
    right_idx[SET_VLEN_EACH * 3 + i] = right_idx_3[i];
  }
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
      retp[current + j] = out[out_idx_save[i] + j];
    }
    current += out_idx[i] - out_idx_save[i];
    for(size_t j = 0; j < left_idx_stop[i] - left_idx[i]; j++) {
      retp[current + j] = lp[left_idx[i] + j];
    }
    current += left_idx_stop[i] - left_idx[i];
    for(size_t j = 0; j < right_idx_stop[i] - right_idx[i]; j++) {
      retp[current + j] = rp[right_idx[i] + j];
    }
    current += right_idx_stop[i] - right_idx[i];
  }
  return ret;
}

template <class T>
std::vector<T> set_difference(std::vector<T>& left, std::vector<T>& right) {
  int valid[SET_VLEN];
  for(int i = 0; i < SET_VLEN; i++) valid[i] = true;

  size_t left_size = left.size();
  size_t right_size = right.size();
  if(left_size == 0) return std::vector<T>();
  if(right_size == 0) return left;

  size_t each = ceil_div(left_size, size_t(SET_VLEN));
  if(each % 2 == 0) each++;
  
  size_t left_idx[SET_VLEN];
  size_t right_idx[SET_VLEN];
  size_t left_idx_stop[SET_VLEN];
  size_t right_idx_stop[SET_VLEN];
  size_t out_idx[SET_VLEN];
  size_t out_idx_save[SET_VLEN];
  std::vector<T> out;
  out.resize(left_size);
  for(int i = 0; i < SET_VLEN; i++) {
    size_t pos = each * i;
    if(pos < left_size) {
      left_idx[i] = pos;
    } else {
      valid[i] = false;
      left_idx[i] = left_size;
    }
  }
  // need to split by different data for difference
  for(int i = 0; i < SET_VLEN-1; i++) {
    if(valid[i]) {
      T last = left[left_idx[i+1]-1];
      while(true) {
        if(left[left_idx[i+1]] == last && left_idx[i+1] < left_size - 1) {
          left_idx[i+1]++;
        } else break;
      }
    }
  }
  for(int i = 0; i < SET_VLEN; i++) {
    out_idx[i] = left_idx[i];
    out_idx_save[i] = left_idx[i];
  }
  right_idx[0] = 0;
  for(int i = 1; i < SET_VLEN; i++) {
    if(valid[i]) {
      auto it = lower_bound(right.begin(), right.end(), left[left_idx[i]]);
      if(it != right.end()) {
        right_idx[i] = it - right.begin();
      } else {
        valid[i] = false;
        right_idx[i] = right_size;
      }
    } else {
      right_idx[i] = right_size;
    }
  }
  for(int i = 0; i < SET_VLEN - 1; i++) {
    left_idx_stop[i] = left_idx[i + 1];
    right_idx_stop[i] = right_idx[i + 1];
  }
  left_idx_stop[SET_VLEN-1] = left_size;
  right_idx_stop[SET_VLEN-1] = right_size;
  for(int i = 0; i < SET_VLEN; i++) {
    if(left_idx[i] == left_idx_stop[i] || right_idx[i] == right_idx_stop[i]) 
      valid[i] = false;
  }
#include "set_operations.incl1"
  T* lp = &left[0];
  T* rp = &right[0];
  T* op = &out[0];
  while(1) {
#include "set_operations.incl2"
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_0[j]) {
        int eq = leftelm0[j] == rightelm0[j];
        int lt = leftelm0[j] < rightelm0[j];
        if(eq) {
          left_idx_0[j]++;
          right_idx_0[j]++;
        } else if(lt) {
          op[out_idx_0[j]++] = leftelm0[j];
          left_idx_0[j]++;
        } else {
          right_idx_0[j]++;
        }
        if(left_idx_0[j] == left_idx_stop_0[j] ||
           right_idx_0[j] == right_idx_stop_0[j]) {
          valid_0[j] = false;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_1[j]) {
        int eq = leftelm1[j] == rightelm1[j];
        int lt = leftelm1[j] < rightelm1[j];
        if(eq) {
          left_idx_1[j]++;
          right_idx_1[j]++;
        } else if(lt) {
          op[out_idx_1[j]++] = leftelm1[j];
          left_idx_1[j]++;
        } else {
          right_idx_1[j]++;
        }
        if(left_idx_1[j] == left_idx_stop_1[j] ||
           right_idx_1[j] == right_idx_stop_1[j]) {
          valid_1[j] = false;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_2[j]) {
        int eq = leftelm2[j] == rightelm2[j];
        int lt = leftelm2[j] < rightelm2[j];
        if(eq) {
          left_idx_2[j]++;
          right_idx_2[j]++;
        } else if(lt) {
          op[out_idx_2[j]++] = leftelm2[j];
          left_idx_2[j]++;
        } else {
          right_idx_2[j]++;
        }
        if(left_idx_2[j] == left_idx_stop_2[j] ||
           right_idx_2[j] == right_idx_stop_2[j]) {
          valid_2[j] = false;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH3; j++) {
      if(valid_3[j]) {
        int eq = leftelm3[j] == rightelm3[j];
        int lt = leftelm3[j] < rightelm3[j];
        if(eq) {
          left_idx_3[j]++;
          right_idx_3[j]++;
        } else if(lt) {
          op[out_idx_3[j]++] = leftelm3[j];
          left_idx_3[j]++;
        } else {
          right_idx_3[j]++;
        }
        if(left_idx_3[j] == left_idx_stop_3[j] ||
           right_idx_3[j] == right_idx_stop_3[j]) {
          valid_3[j] = false;
        }
      }
    }
    int any_valid = false;
    for(int i = 0; i < SET_VLEN_EACH; i++) {
      if(valid_0[i] || valid_1[i] || valid_2[i] || valid_3[i])
        any_valid = true;
    }
    if(any_valid == false) break;
  }
  for(size_t i = 0; i < SET_VLEN_EACH; i++) {
    out_idx[i] = out_idx_0[i];
    out_idx[SET_VLEN_EACH * 1 + i] = out_idx_1[i];
    out_idx[SET_VLEN_EACH * 2 + i] = out_idx_2[i];
    left_idx[i] = left_idx_0[i];
    left_idx[SET_VLEN_EACH * 1 + i] = left_idx_1[i];
    left_idx[SET_VLEN_EACH * 2 + i] = left_idx_2[i];
  }
  for(size_t i = 0; i < SET_VLEN_EACH3; i++) {
    out_idx[SET_VLEN_EACH * 3 + i] = out_idx_3[i];
    left_idx[SET_VLEN_EACH * 3 + i] = left_idx_3[i];
  }
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
      retp[current + j] = out[out_idx_save[i] + j];
    }
    current += out_idx[i] - out_idx_save[i];
    for(size_t j = 0; j < left_idx_stop[i] - left_idx[i]; j++) {
      retp[current + j] = lp[left_idx[i] + j];
    }
    current += left_idx_stop[i] - left_idx[i];
  }
  return ret;
}

template <class T>
std::vector<T> set_merge(std::vector<T>& left, std::vector<T>& right) {
  int valid[SET_VLEN];
  for(int i = 0; i < SET_VLEN; i++) valid[i] = true;

  size_t left_size = left.size();
  size_t right_size = right.size();
  if(left_size == 0) return right;
  if(right_size == 0) return left;

  size_t each = ceil_div(left_size, size_t(SET_VLEN));
  if(each % 2 == 0) each++;
  
  size_t left_idx[SET_VLEN];
  size_t right_idx[SET_VLEN];
  size_t left_idx_stop[SET_VLEN];
  size_t right_idx_stop[SET_VLEN];
  size_t out_idx[SET_VLEN];
  std::vector<T> out;
  out.resize(left_size + right_size);
  for(int i = 0; i < SET_VLEN; i++) {
    size_t pos = each * i;
    if(pos < left_size) {
      left_idx[i] = pos;
    } else {
      valid[i] = false;
      left_idx[i] = left_size;
    }
  }
  right_idx[0] = 0;
  for(int i = 1; i < SET_VLEN; i++) {
    if(valid[i]) {
      auto it = lower_bound(right.begin(), right.end(), left[left_idx[i]]);
      if(it != right.end()) {
        right_idx[i] = it - right.begin();
      } else {
        valid[i] = false;
        right_idx[i] = right_size;
      }
    } else {
      right_idx[i] = right_size;
    }
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
  for(int i = 0; i < SET_VLEN; i++) {
    if(right_idx[i] == right_idx_stop[i]) 
      valid[i] = false;
  }
#include "set_operations.incl1"
  T* lp = &left[0];
  T* rp = &right[0];
  T* op = &out[0];
  while(1) {
#include "set_operations.incl2"
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_0[j]) {
        int eq = leftelm0[j] == rightelm0[j];
        int lt = leftelm0[j] < rightelm0[j];
        if(eq) {
          op[out_idx_0[j]++] = leftelm0[j];
          op[out_idx_0[j]++] = rightelm0[j];
          left_idx_0[j]++;
          right_idx_0[j]++;
        } else if(lt) {
          op[out_idx_0[j]++] = leftelm0[j];
          left_idx_0[j]++;
        } else{
          op[out_idx_0[j]++] = rightelm0[j];
          right_idx_0[j]++;
        }
        if(left_idx_0[j] == left_idx_stop_0[j] ||
           right_idx_0[j] == right_idx_stop_0[j]) {
          valid_0[j] = false;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_1[j]) {
        int eq = leftelm1[j] == rightelm1[j];
        int lt = leftelm1[j] < rightelm1[j];
        if(eq) {
          op[out_idx_1[j]++] = leftelm1[j];
          op[out_idx_1[j]++] = rightelm1[j];
          left_idx_1[j]++;
          right_idx_1[j]++;
        } else if(lt) {
          op[out_idx_1[j]++] = leftelm1[j];
          left_idx_1[j]++;
        } else{
          op[out_idx_1[j]++] = rightelm1[j];
          right_idx_1[j]++;
        }
        if(left_idx_1[j] == left_idx_stop_1[j] ||
           right_idx_1[j] == right_idx_stop_1[j]) {
          valid_1[j] = false;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_2[j]) {
        int eq = leftelm2[j] == rightelm2[j];
        int lt = leftelm2[j] < rightelm2[j];
        if(eq) {
          op[out_idx_2[j]++] = leftelm2[j];
          op[out_idx_2[j]++] = rightelm2[j];
          left_idx_2[j]++;
          right_idx_2[j]++;
        } else if(lt) {
          op[out_idx_2[j]++] = leftelm2[j];
          left_idx_2[j]++;
        } else{
          op[out_idx_2[j]++] = rightelm2[j];
          right_idx_2[j]++;
        }
        if(left_idx_2[j] == left_idx_stop_2[j] ||
           right_idx_2[j] == right_idx_stop_2[j]) {
          valid_2[j] = false;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH3; j++) {
      if(valid_3[j]) {
        int eq = leftelm3[j] == rightelm3[j];
        int lt = leftelm3[j] < rightelm3[j];
        if(eq) {
          op[out_idx_3[j]++] = leftelm3[j];
          op[out_idx_3[j]++] = rightelm3[j];
          left_idx_3[j]++;
          right_idx_3[j]++;
        } else if(lt) {
          op[out_idx_3[j]++] = leftelm3[j];
          left_idx_3[j]++;
        } else{
          op[out_idx_3[j]++] = rightelm3[j];
          right_idx_3[j]++;
        }
        if(left_idx_3[j] == left_idx_stop_3[j] ||
           right_idx_3[j] == right_idx_stop_3[j]) {
          valid_3[j] = false;
        }
      }
    }

    int any_valid = false;
    for(int i = 0; i < SET_VLEN_EACH; i++) {
      if(valid_0[i] || valid_1[i] || valid_2[i] || valid_3[i])
        any_valid = true;
    }
    if(any_valid == false) break;
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
  for(size_t i = 0; i < SET_VLEN_EACH3; i++) {
    out_idx[SET_VLEN_EACH * 3 + i] = out_idx_3[i];
    left_idx[SET_VLEN_EACH * 3 + i] = left_idx_3[i];
    right_idx[SET_VLEN_EACH * 3 + i] = right_idx_3[i];
  }
  for(size_t i = 0; i < SET_VLEN; i++) {
    for(size_t j = 0; j < left_idx_stop[i] - left_idx[i]; j++) {
      op[out_idx[i] + j] = lp[left_idx[i] + j];
    }
    for(size_t j = 0; j < right_idx_stop[i] - right_idx[i]; j++) {
      op[out_idx[i] + j] = rp[right_idx[i] + j];
    }
  }
  return out;
}

template <class T>
std::vector<T> set_merge_desc(std::vector<T>& left, std::vector<T>& right) {
  int valid[SET_VLEN];
  for(int i = 0; i < SET_VLEN; i++) valid[i] = true;

  size_t left_size = left.size();
  size_t right_size = right.size();
  if(left_size == 0) return right;
  if(right_size == 0) return left;
  size_t each = ceil_div(left_size, size_t(SET_VLEN));
  if(each % 2 == 0) each++;
  
  size_t left_idx[SET_VLEN];
  size_t right_idx[SET_VLEN];
  size_t left_idx_stop[SET_VLEN];
  size_t right_idx_stop[SET_VLEN];
  size_t out_idx[SET_VLEN];
  std::vector<T> out;
  out.resize(left_size + right_size);
  for(int i = 0; i < SET_VLEN; i++) {
    size_t pos = each * i;
    if(pos < left_size) {
      left_idx[i] = pos;
    } else {
      valid[i] = false;
      left_idx[i] = left_size;
    }
  }
  right_idx[0] = 0;
  for(int i = 1; i < SET_VLEN; i++) {
    if(valid[i]) {
      auto it = lower_bound(right.begin(), right.end(), left[left_idx[i]],
                            std::greater<T>());
      if(it != right.end()) {
        right_idx[i] = it - right.begin();
      } else {
        valid[i] = false;
        right_idx[i] = right_size;
      }
    } else {
      right_idx[i] = right_size;
    }
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
  for(int i = 0; i < SET_VLEN; i++) {
    if(right_idx[i] == right_idx_stop[i]) 
      valid[i] = false;
  }
#include "set_operations.incl1"
  T* lp = &left[0];
  T* rp = &right[0];
  T* op = &out[0];
  while(1) {
#include "set_operations.incl2"
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_0[j]) {
        int eq = leftelm0[j] == rightelm0[j];
        int lt = leftelm0[j] > rightelm0[j]; // desc
        if(eq) {
          op[out_idx_0[j]++] = leftelm0[j];
          op[out_idx_0[j]++] = rightelm0[j];
          left_idx_0[j]++;
          right_idx_0[j]++;
        } else if(lt) {
          op[out_idx_0[j]++] = leftelm0[j];
          left_idx_0[j]++;
        } else{
          op[out_idx_0[j]++] = rightelm0[j];
          right_idx_0[j]++;
        }
        if(left_idx_0[j] == left_idx_stop_0[j] ||
           right_idx_0[j] == right_idx_stop_0[j]) {
          valid_0[j] = false;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_1[j]) {
        int eq = leftelm1[j] == rightelm1[j];
        int lt = leftelm1[j] > rightelm1[j]; // desc
        if(eq) {
          op[out_idx_1[j]++] = leftelm1[j];
          op[out_idx_1[j]++] = rightelm1[j];
          left_idx_1[j]++;
          right_idx_1[j]++;
        } else if(lt) {
          op[out_idx_1[j]++] = leftelm1[j];
          left_idx_1[j]++;
        } else{
          op[out_idx_1[j]++] = rightelm1[j];
          right_idx_1[j]++;
        }
        if(left_idx_1[j] == left_idx_stop_1[j] ||
           right_idx_1[j] == right_idx_stop_1[j]) {
          valid_1[j] = false;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_2[j]) {
        int eq = leftelm2[j] == rightelm2[j];
        int lt = leftelm2[j] > rightelm2[j]; // desc
        if(eq) {
          op[out_idx_2[j]++] = leftelm2[j];
          op[out_idx_2[j]++] = rightelm2[j];
          left_idx_2[j]++;
          right_idx_2[j]++;
        } else if(lt) {
          op[out_idx_2[j]++] = leftelm2[j];
          left_idx_2[j]++;
        } else{
          op[out_idx_2[j]++] = rightelm2[j];
          right_idx_2[j]++;
        }
        if(left_idx_2[j] == left_idx_stop_2[j] ||
           right_idx_2[j] == right_idx_stop_2[j]) {
          valid_2[j] = false;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH3; j++) {
      if(valid_3[j]) {
        int eq = leftelm3[j] == rightelm3[j];
        int lt = leftelm3[j] > rightelm3[j]; // desc
        if(eq) {
          op[out_idx_3[j]++] = leftelm3[j];
          op[out_idx_3[j]++] = rightelm3[j];
          left_idx_3[j]++;
          right_idx_3[j]++;
        } else if(lt) {
          op[out_idx_3[j]++] = leftelm3[j];
          left_idx_3[j]++;
        } else{
          op[out_idx_3[j]++] = rightelm3[j];
          right_idx_3[j]++;
        }
        if(left_idx_3[j] == left_idx_stop_3[j] ||
           right_idx_3[j] == right_idx_stop_3[j]) {
          valid_3[j] = false;
        }
      }
    }

    int any_valid = false;
    for(int i = 0; i < SET_VLEN_EACH; i++) {
      if(valid_0[i] || valid_1[i] || valid_2[i] || valid_3[i])
        any_valid = true;
    }
    if(any_valid == false) break;
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
  for(size_t i = 0; i < SET_VLEN_EACH3; i++) {
    out_idx[SET_VLEN_EACH * 3 + i] = out_idx_3[i];
    left_idx[SET_VLEN_EACH * 3 + i] = left_idx_3[i];
    right_idx[SET_VLEN_EACH * 3 + i] = right_idx_3[i];
  }
  for(size_t i = 0; i < SET_VLEN; i++) {
    for(size_t j = 0; j < left_idx_stop[i] - left_idx[i]; j++) {
      op[out_idx[i] + j] = lp[left_idx[i] + j];
    }
    for(size_t j = 0; j < right_idx_stop[i] - right_idx[i]; j++) {
      op[out_idx[i] + j] = rp[right_idx[i] + j];
    }
  }
  return out;
}

template <class T, class K>
void set_merge_pair(std::vector<T>& left, std::vector<K>& left_val,
                    std::vector<T>& right, std::vector<K>& right_val,
                    std::vector<T>& out, std::vector<K>& out_val) {
  int valid[SET_VLEN];
  for(int i = 0; i < SET_VLEN; i++) valid[i] = true;
  size_t left_size = left.size();
  size_t right_size = right.size();
  if(left_size == 0) {
    out = right;
    out_val = right_val;
    return;
  }
  if(right_size == 0) {
    out = left;
    out_val = left_val;
    return;
  }
  if(left_size != left_val.size() || right_size != right_val.size())
    throw std::runtime_error("sizes of key and value are not the same");
  size_t each = ceil_div(left_size, size_t(SET_VLEN));
  if(each % 2 == 0) each++;
  
  size_t left_idx[SET_VLEN];
  size_t right_idx[SET_VLEN];
  size_t left_idx_stop[SET_VLEN];
  size_t right_idx_stop[SET_VLEN];
  size_t out_idx[SET_VLEN];
  out.resize(left_size + right_size);
  out_val.resize(left_size + right_size);
  for(int i = 0; i < SET_VLEN; i++) {
    size_t pos = each * i;
    if(pos < left_size) {
      left_idx[i] = pos;
    } else {
      valid[i] = false;
      left_idx[i] = left_size;
    }
  }
  right_idx[0] = 0;
  for(int i = 1; i < SET_VLEN; i++) {
    if(valid[i]) {
      auto it = lower_bound(right.begin(), right.end(), left[left_idx[i]]);
      if(it != right.end()) {
        right_idx[i] = it - right.begin();
      } else {
        valid[i] = false;
        right_idx[i] = right_size;
      }
    } else {
      right_idx[i] = right_size;
    }
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
  for(int i = 0; i < SET_VLEN; i++) {
    if(right_idx[i] == right_idx_stop[i]) 
      valid[i] = false;
  }
#include "set_operations.incl1"
  T* lp = &left[0];
  K* lvp = &left_val[0];
  T* rp = &right[0];
  K* rvp = &right_val[0];
  T* op = &out[0];
  K* ovp = &out_val[0];
  while(1) {
#include "set_operations.incl2"
  K leftval0[SET_VLEN_EACH];
  K leftval1[SET_VLEN_EACH];
  K leftval2[SET_VLEN_EACH];
  K leftval3[SET_VLEN_EACH3];
  K rightval0[SET_VLEN_EACH];
  K rightval1[SET_VLEN_EACH];
  K rightval2[SET_VLEN_EACH];
  K rightval3[SET_VLEN_EACH3];
#pragma cdir alloc_on_vreg(leftval0,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(leftval1,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(leftval2,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(leftval3,SET_VLEN_EACH3)
#pragma cdir alloc_on_vreg(rightval0,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(rightval1,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(rightval2,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(rightval3,SET_VLEN_EACH3)
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_0[j]) leftval0[j] = lvp[left_idx_0[j]];
      if(valid_0[j]) rightval0[j] = rvp[right_idx_0[j]];
      if(valid_1[j]) leftval1[j] = lvp[left_idx_1[j]];
      if(valid_1[j]) rightval1[j] = rvp[right_idx_1[j]];
      if(valid_2[j]) leftval2[j] = lvp[left_idx_2[j]];
      if(valid_2[j]) rightval2[j] = rvp[right_idx_2[j]];
    }
    for(int j = 0; j < SET_VLEN_EACH3; j++) {
      if(valid_3[j]) leftval3[j] = lvp[left_idx_3[j]];
      if(valid_3[j]) rightval3[j] = rvp[right_idx_3[j]];
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_0[j]) {
        int eq = leftelm0[j] == rightelm0[j];
        int lt = leftelm0[j] < rightelm0[j];
        if(eq) {
          op[out_idx_0[j]] = leftelm0[j];
          ovp[out_idx_0[j]++] = leftval0[j];
          op[out_idx_0[j]] = rightelm0[j];
          ovp[out_idx_0[j]++] = rightval0[j];
          left_idx_0[j]++;
          right_idx_0[j]++;
        } else if(lt) {
          op[out_idx_0[j]] = leftelm0[j];
          ovp[out_idx_0[j]++] = leftval0[j];
          left_idx_0[j]++;
        } else{
          op[out_idx_0[j]] = rightelm0[j];
          ovp[out_idx_0[j]++] = rightval0[j];
          right_idx_0[j]++;
        }
        if(left_idx_0[j] == left_idx_stop_0[j] ||
           right_idx_0[j] == right_idx_stop_0[j]) {
          valid_0[j] = false;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_1[j]) {
        int eq = leftelm1[j] == rightelm1[j];
        int lt = leftelm1[j] < rightelm1[j];
        if(eq) {
          op[out_idx_1[j]] = leftelm1[j];
          ovp[out_idx_1[j]++] = leftval1[j];
          op[out_idx_1[j]] = rightelm1[j];
          ovp[out_idx_1[j]++] = rightval1[j];
          left_idx_1[j]++;
          right_idx_1[j]++;
        } else if(lt) {
          op[out_idx_1[j]] = leftelm1[j];
          ovp[out_idx_1[j]++] = leftval1[j];
          left_idx_1[j]++;
        } else{
          op[out_idx_1[j]] = rightelm1[j];
          ovp[out_idx_1[j]++] = rightval1[j];
          right_idx_1[j]++;
        }
        if(left_idx_1[j] == left_idx_stop_1[j] ||
           right_idx_1[j] == right_idx_stop_1[j]) {
          valid_1[j] = false;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_2[j]) {
        int eq = leftelm2[j] == rightelm2[j];
        int lt = leftelm2[j] < rightelm2[j];
        if(eq) {
          op[out_idx_2[j]] = leftelm2[j];
          ovp[out_idx_2[j]++] = leftval2[j];
          op[out_idx_2[j]] = rightelm2[j];
          ovp[out_idx_2[j]++] = rightval2[j];
          left_idx_2[j]++;
          right_idx_2[j]++;
        } else if(lt) {
          op[out_idx_2[j]] = leftelm2[j];
          ovp[out_idx_2[j]++] = leftval2[j];
          left_idx_2[j]++;
        } else{
          op[out_idx_2[j]] = rightelm2[j];
          ovp[out_idx_2[j]++] = rightval2[j];
          right_idx_2[j]++;
        }
        if(left_idx_2[j] == left_idx_stop_2[j] ||
           right_idx_2[j] == right_idx_stop_2[j]) {
          valid_2[j] = false;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH3; j++) {
      if(valid_3[j]) {
        int eq = leftelm3[j] == rightelm3[j];
        int lt = leftelm3[j] < rightelm3[j];
        if(eq) {
          op[out_idx_3[j]] = leftelm3[j];
          ovp[out_idx_3[j]++] = leftval3[j];
          op[out_idx_3[j]] = rightelm3[j];
          ovp[out_idx_3[j]++] = rightval3[j];
          left_idx_3[j]++;
          right_idx_3[j]++;
        } else if(lt) {
          op[out_idx_3[j]] = leftelm3[j];
          ovp[out_idx_3[j]++] = leftval3[j];
          left_idx_3[j]++;
        } else{
          op[out_idx_3[j]] = rightelm3[j];
          ovp[out_idx_3[j]++] = rightval3[j];
          right_idx_3[j]++;
        }
        if(left_idx_3[j] == left_idx_stop_3[j] ||
           right_idx_3[j] == right_idx_stop_3[j]) {
          valid_3[j] = false;
        }
      }
    }

    int any_valid = false;
    for(int i = 0; i < SET_VLEN_EACH; i++) {
      if(valid_0[i] || valid_1[i] || valid_2[i] || valid_3[i])
        any_valid = true;
    }
    if(any_valid == false) break;
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
  for(size_t i = 0; i < SET_VLEN_EACH3; i++) {
    out_idx[SET_VLEN_EACH * 3 + i] = out_idx_3[i];
    left_idx[SET_VLEN_EACH * 3 + i] = left_idx_3[i];
    right_idx[SET_VLEN_EACH * 3 + i] = right_idx_3[i];
  }
  for(size_t i = 0; i < SET_VLEN; i++) {
    for(size_t j = 0; j < left_idx_stop[i] - left_idx[i]; j++) {
      op[out_idx[i] + j] = lp[left_idx[i] + j];
      ovp[out_idx[i] + j] = lvp[left_idx[i] + j];
    }
    for(size_t j = 0; j < right_idx_stop[i] - right_idx[i]; j++) {
      op[out_idx[i] + j] = rp[right_idx[i] + j];
      ovp[out_idx[i] + j] = rvp[right_idx[i] + j];
    }
  }
}

template <class T, class K>
void set_merge_pair_desc(std::vector<T>& left, std::vector<K>& left_val,
                         std::vector<T>& right, std::vector<K>& right_val,
                         std::vector<T>& out, std::vector<K>& out_val) {
                    
  int valid[SET_VLEN];
  for(int i = 0; i < SET_VLEN; i++) valid[i] = true;
  size_t left_size = left.size();
  size_t right_size = right.size();
  if(left_size == 0) {
    out = right;
    out_val = right_val;
    return;
  }
  if(right_size == 0) {
    out = left;
    out_val = left_val;
    return;
  }
  if(left_size != left_val.size() || right_size != right_val.size())
    throw std::runtime_error("sizes of key and value are not the same");
  size_t each = ceil_div(left_size, size_t(SET_VLEN));
  if(each % 2 == 0) each++;
  
  size_t left_idx[SET_VLEN];
  size_t right_idx[SET_VLEN];
  size_t left_idx_stop[SET_VLEN];
  size_t right_idx_stop[SET_VLEN];
  size_t out_idx[SET_VLEN];
  out.resize(left_size + right_size);
  out_val.resize(left_size + right_size);
  for(int i = 0; i < SET_VLEN; i++) {
    size_t pos = each * i;
    if(pos < left_size) {
      left_idx[i] = pos;
    } else {
      valid[i] = false;
      left_idx[i] = left_size;
    }
  }
  right_idx[0] = 0;
  for(int i = 1; i < SET_VLEN; i++) {
    if(valid[i]) {
      auto it = lower_bound(right.begin(), right.end(), left[left_idx[i]],
                            std::greater<T>()); // desc
      if(it != right.end()) {
        right_idx[i] = it - right.begin();
      } else {
        valid[i] = false;
        right_idx[i] = right_size;
      }
    } else {
      right_idx[i] = right_size;
    }
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
  for(int i = 0; i < SET_VLEN; i++) {
    if(right_idx[i] == right_idx_stop[i]) 
      valid[i] = false;
  }
#include "set_operations.incl1"
  T* lp = &left[0];
  K* lvp = &left_val[0];
  T* rp = &right[0];
  K* rvp = &right_val[0];
  T* op = &out[0];
  K* ovp = &out_val[0];
  while(1) {
#include "set_operations.incl2"
  K leftval0[SET_VLEN_EACH];
  K leftval1[SET_VLEN_EACH];
  K leftval2[SET_VLEN_EACH];
  K leftval3[SET_VLEN_EACH3];
  K rightval0[SET_VLEN_EACH];
  K rightval1[SET_VLEN_EACH];
  K rightval2[SET_VLEN_EACH];
  K rightval3[SET_VLEN_EACH3];
#pragma cdir alloc_on_vreg(leftval0,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(leftval1,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(leftval2,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(leftval3,SET_VLEN_EACH3)
#pragma cdir alloc_on_vreg(rightval0,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(rightval1,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(rightval2,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(rightval3,SET_VLEN_EACH3)
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_0[j]) leftval0[j] = lvp[left_idx_0[j]];
      if(valid_0[j]) rightval0[j] = rvp[right_idx_0[j]];
      if(valid_1[j]) leftval1[j] = lvp[left_idx_1[j]];
      if(valid_1[j]) rightval1[j] = rvp[right_idx_1[j]];
      if(valid_2[j]) leftval2[j] = lvp[left_idx_2[j]];
      if(valid_2[j]) rightval2[j] = rvp[right_idx_2[j]];
    }
    for(int j = 0; j < SET_VLEN_EACH3; j++) {
      if(valid_3[j]) leftval3[j] = lvp[left_idx_3[j]];
      if(valid_3[j]) rightval3[j] = rvp[right_idx_3[j]];
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_0[j]) {
        int eq = leftelm0[j] == rightelm0[j];
        int lt = leftelm0[j] > rightelm0[j]; // desc
        if(eq) {
          op[out_idx_0[j]] = leftelm0[j];
          ovp[out_idx_0[j]++] = leftval0[j];
          op[out_idx_0[j]] = rightelm0[j];
          ovp[out_idx_0[j]++] = rightval0[j];
          left_idx_0[j]++;
          right_idx_0[j]++;
        } else if(lt) {
          op[out_idx_0[j]] = leftelm0[j];
          ovp[out_idx_0[j]++] = leftval0[j];
          left_idx_0[j]++;
        } else{
          op[out_idx_0[j]] = rightelm0[j];
          ovp[out_idx_0[j]++] = rightval0[j];
          right_idx_0[j]++;
        }
        if(left_idx_0[j] == left_idx_stop_0[j] ||
           right_idx_0[j] == right_idx_stop_0[j]) {
          valid_0[j] = false;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_1[j]) {
        int eq = leftelm1[j] == rightelm1[j];
        int lt = leftelm1[j] > rightelm1[j]; // desc
        if(eq) {
          op[out_idx_1[j]] = leftelm1[j];
          ovp[out_idx_1[j]++] = leftval1[j];
          op[out_idx_1[j]] = rightelm1[j];
          ovp[out_idx_1[j]++] = rightval1[j];
          left_idx_1[j]++;
          right_idx_1[j]++;
        } else if(lt) {
          op[out_idx_1[j]] = leftelm1[j];
          ovp[out_idx_1[j]++] = leftval1[j];
          left_idx_1[j]++;
        } else{
          op[out_idx_1[j]] = rightelm1[j];
          ovp[out_idx_1[j]++] = rightval1[j];
          right_idx_1[j]++;
        }
        if(left_idx_1[j] == left_idx_stop_1[j] ||
           right_idx_1[j] == right_idx_stop_1[j]) {
          valid_1[j] = false;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_2[j]) {
        int eq = leftelm2[j] == rightelm2[j];
        int lt = leftelm2[j] > rightelm2[j]; // desc
        if(eq) {
          op[out_idx_2[j]] = leftelm2[j];
          ovp[out_idx_2[j]++] = leftval2[j];
          op[out_idx_2[j]] = rightelm2[j];
          ovp[out_idx_2[j]++] = rightval2[j];
          left_idx_2[j]++;
          right_idx_2[j]++;
        } else if(lt) {
          op[out_idx_2[j]] = leftelm2[j];
          ovp[out_idx_2[j]++] = leftval2[j];
          left_idx_2[j]++;
        } else{
          op[out_idx_2[j]] = rightelm2[j];
          ovp[out_idx_2[j]++] = rightval2[j];
          right_idx_2[j]++;
        }
        if(left_idx_2[j] == left_idx_stop_2[j] ||
           right_idx_2[j] == right_idx_stop_2[j]) {
          valid_2[j] = false;
        }
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH3; j++) {
      if(valid_3[j]) {
        int eq = leftelm3[j] == rightelm3[j];
        int lt = leftelm3[j] > rightelm3[j]; // desc
        if(eq) {
          op[out_idx_3[j]] = leftelm3[j];
          ovp[out_idx_3[j]++] = leftval3[j];
          op[out_idx_3[j]] = rightelm3[j];
          ovp[out_idx_3[j]++] = rightval3[j];
          left_idx_3[j]++;
          right_idx_3[j]++;
        } else if(lt) {
          op[out_idx_3[j]] = leftelm3[j];
          ovp[out_idx_3[j]++] = leftval3[j];
          left_idx_3[j]++;
        } else{
          op[out_idx_3[j]] = rightelm3[j];
          ovp[out_idx_3[j]++] = rightval3[j];
          right_idx_3[j]++;
        }
        if(left_idx_3[j] == left_idx_stop_3[j] ||
           right_idx_3[j] == right_idx_stop_3[j]) {
          valid_3[j] = false;
        }
      }
    }

    int any_valid = false;
    for(int i = 0; i < SET_VLEN_EACH; i++) {
      if(valid_0[i] || valid_1[i] || valid_2[i] || valid_3[i])
        any_valid = true;
    }
    if(any_valid == false) break;
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
  for(size_t i = 0; i < SET_VLEN_EACH3; i++) {
    out_idx[SET_VLEN_EACH * 3 + i] = out_idx_3[i];
    left_idx[SET_VLEN_EACH * 3 + i] = left_idx_3[i];
    right_idx[SET_VLEN_EACH * 3 + i] = right_idx_3[i];
  }
  for(size_t i = 0; i < SET_VLEN; i++) {
    for(size_t j = 0; j < left_idx_stop[i] - left_idx[i]; j++) {
      op[out_idx[i] + j] = lp[left_idx[i] + j];
      ovp[out_idx[i] + j] = lvp[left_idx[i] + j];
    }
    for(size_t j = 0; j < right_idx_stop[i] - right_idx[i]; j++) {
      op[out_idx[i] + j] = rp[right_idx[i] + j];
      ovp[out_idx[i] + j] = rvp[right_idx[i] + j];
    }
  }
}

// TODO: modify to use alloc_on_vreg
template <class T>
std::vector<size_t> set_separate(const std::vector<T>& key) {
  size_t size = key.size();
  if(size == 0) {return std::vector<size_t>(1);} 
  int valid[SET_VLEN];
  for(int i = 0; i < SET_VLEN; i++) valid[i] = true;
  size_t each = ceil_div(size, size_t(SET_VLEN));
  if(each % 2 == 0) each++;
  size_t key_idx[SET_VLEN];
  size_t key_idx_stop[SET_VLEN];
  size_t out_idx[SET_VLEN];
  size_t out_idx_save[SET_VLEN];
  T current_key[SET_VLEN];
  std::vector<size_t> out;
  out.resize(size);
  size_t* outp = &out[0];
  const T* keyp = &key[0];
  if(size > 0) {
    key_idx[0] = 1;
    outp[0] = 0;
    out_idx[0] = 1;
    out_idx_save[0] = 0;
    current_key[0] = keyp[0];
  } else {
    valid[0] = false;
    key_idx[0] = size;
    out_idx[0] = size;
    out_idx_save[0] = size;
  }
  for(int i = 1; i < SET_VLEN; i++) {
    size_t pos = each * i;
    if(pos < size) {
      key_idx[i] = pos;
      out_idx[i] = pos;
      out_idx_save[i] = pos;
      current_key[i] = keyp[pos-1];
    } else {
      valid[i] = false;
      key_idx[i] = size;
      out_idx[i] = size;
      out_idx_save[i] = size;
    }
  }
  for(int i = 0; i < SET_VLEN - 1; i++) {
    key_idx_stop[i] = key_idx[i + 1];
  }
  key_idx_stop[SET_VLEN-1] = size;
  // idx 0 is manually advanced; need to be checked
  if(key_idx[0] == key_idx_stop[0]) valid[0] = false; 
  while(1) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(int i = 0; i < SET_VLEN; i++) {
      if(valid[i]) {
        auto keyval = keyp[key_idx[i]];
        if(keyval != current_key[i]) {
          outp[out_idx[i]++] = key_idx[i];
          current_key[i] = keyval;
        }
        key_idx[i]++;
        if(key_idx[i] == key_idx_stop[i]) {valid[i] = false;}
      }
    }
    int any_valid = false;
    for(int i = 0; i < SET_VLEN; i++) if(valid[i]) any_valid = true;
    if(any_valid == false) break;
  }
  size_t total = 0;
  for(size_t i = 0; i < SET_VLEN; i++) {
    total += out_idx[i] - out_idx_save[i];
  }
  std::vector<size_t> ret(total+1);
  size_t* retp = &ret[0];
  size_t current = 0;
  for(size_t i = 0; i < SET_VLEN; i++) {
    for(size_t j = 0; j < out_idx[i] - out_idx_save[i]; j++) {
      retp[current + j] = out[out_idx_save[i] + j];
    }
    current += out_idx[i] - out_idx_save[i];
  }
  retp[current] = size;
  return ret;
}

template <class T>
std::vector<T> set_unique(const std::vector<T>& key) {
  size_t size = key.size();
  if(size == 0) {return std::vector<T>();} 
  int valid[SET_VLEN];
  for(int i = 0; i < SET_VLEN; i++) valid[i] = true;
  size_t each = ceil_div(size, size_t(SET_VLEN));
  if(each % 2 == 0) each++;
  size_t key_idx[SET_VLEN];
  size_t key_idx_stop[SET_VLEN];
  size_t out_idx[SET_VLEN];
  size_t out_idx_save[SET_VLEN];
  T current_key[SET_VLEN];
  std::vector<T> out;
  out.resize(size);
  T* outp = &out[0];
  const T* keyp = &key[0];
  if(size > 0) {
    key_idx[0] = 1;
    outp[0] = keyp[0];
    out_idx[0] = 1;
    out_idx_save[0] = 0;
    current_key[0] = keyp[0];
  } else {
    valid[0] = false;
    key_idx[0] = size;
    out_idx[0] = size;
    out_idx_save[0] = size;
  }
  for(int i = 1; i < SET_VLEN; i++) {
    size_t pos = each * i;
    if(pos < size) {
      key_idx[i] = pos;
      out_idx[i] = pos;
      out_idx_save[i] = pos;
      current_key[i] = keyp[pos-1];
    } else {
      valid[i] = false;
      key_idx[i] = size;
      out_idx[i] = size;
      out_idx_save[i] = size;
    }
  }
  for(int i = 0; i < SET_VLEN - 1; i++) {
    key_idx_stop[i] = key_idx[i + 1];
  }
  key_idx_stop[SET_VLEN-1] = size;
  // idx 0 is manually advanced; need to be checked
  if(key_idx[0] == key_idx_stop[0]) valid[0] = false; 
  while(1) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(int i = 0; i < SET_VLEN; i++) {
      if(valid[i]) {
        auto keyval = keyp[key_idx[i]];
        if(keyval != current_key[i]) {
          outp[out_idx[i]++] = keyval;
          current_key[i] = keyval;
        }
        key_idx[i]++;
        if(key_idx[i] == key_idx_stop[i]) {valid[i] = false;}
      }
    }
    int any_valid = false;
    for(int i = 0; i < SET_VLEN; i++) if(valid[i]) any_valid = true;
    if(any_valid == false) break;
  }
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
      retp[current + j] = out[out_idx_save[i] + j];
    }
    current += out_idx[i] - out_idx_save[i];
  }
  return ret;
}

// TODO: modify to use alloc_on_vreg
template <class T>
int set_is_unique(const std::vector<T>& key) {
  size_t size = key.size();
  if(size == 0) return true;
  int valid[SET_VLEN];
  int is_unique[SET_VLEN];
  for(int i = 0; i < SET_VLEN; i++) {
    valid[i] = true;
    is_unique[i] = true;
  }
  size_t each = ceil_div(size, size_t(SET_VLEN));
  if(each % 2 == 0) each++;
  size_t key_idx[SET_VLEN];
  size_t key_idx_stop[SET_VLEN];
  T current_key[SET_VLEN];
  const T* keyp = &key[0];
  if(size > 0) {
    key_idx[0] = 1;
    current_key[0] = keyp[0];
  } else {
    valid[0] = false;
    key_idx[0] = size;
  }
  for(int i = 1; i < SET_VLEN; i++) {
    size_t pos = each * i;
    if(pos < size) {
      key_idx[i] = pos;
      current_key[i] = keyp[pos-1];
    } else {
      valid[i] = false;
      key_idx[i] = size;
    }
  }
  for(int i = 0; i < SET_VLEN - 1; i++) {
    key_idx_stop[i] = key_idx[i + 1];
  }
  key_idx_stop[SET_VLEN-1] = size;
  // idx 0 is manually advanced; need to be checked
  if(key_idx[0] == key_idx_stop[0]) valid[0] = false; 
  int any_unique = true;
  while(1) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(int i = 0; i < SET_VLEN; i++) {
      if(valid[i]) {
        auto keyval = keyp[key_idx[i]];
        if(keyval != current_key[i]) current_key[i] = keyval;
        else is_unique[i] = false;
        key_idx[i]++;
        if(key_idx[i] == key_idx_stop[i]) {valid[i] = false;}
      }
    }
    for(int i = 0; i < SET_VLEN; i++) if(!is_unique[i]) any_unique = false;
    int any_valid = false;
    for(int i = 0; i < SET_VLEN; i++) if(valid[i]) any_valid = true;
    if(any_valid == false || any_unique == false) break;
  }
  return any_unique;
}


#endif // _SX

template <class T, class K>
void set_multimerge_pair(std::vector<std::vector<T>>& key,
                         std::vector<std::vector<K>>& val,
                         std::vector<T>& out_key,
                         std::vector<K>& out_val) {
  size_t part_size = key.size();
  if(part_size == 1) {
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
  if(part_size == 1) {
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
  
}
#endif
