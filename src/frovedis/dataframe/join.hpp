#ifndef JOIN_HPP
#define JOIN_HPP

#include <vector>
#if !(defined(_SX) || defined(__ve__))
#include <unordered_map>
#else
#include "../core/radix_sort.hpp"
#include "set_operations.hpp"
#include "hashtable.hpp"
#endif

namespace frovedis {

std::vector<size_t> shrink_missed(std::vector<size_t>& to_shrink, 
                                  std::vector<size_t>& missed);

#if !(defined(_SX) || defined(__ve__))
// input need not be sorted
template <class T>
void equi_join(std::vector<T>& left,
               std::vector<size_t>& left_idx, 
               std::vector<T>& right,
               std::vector<size_t>& right_idx,
               std::vector<size_t>& left_idx_out, 
               std::vector<size_t>& right_idx_out) {
  std::unordered_multimap<T,size_t> right_map;
  for(size_t i = 0; i < right.size(); i++) {
    right_map.insert(std::make_pair(right[i], right_idx[i]));
  }
  left_idx_out.clear();
  right_idx_out.clear();
  for(size_t i = 0; i < left.size(); i++) {
    auto range = right_map.equal_range(left[i]);
    for(auto it = range.first; it != range.second; ++it) {
      left_idx_out.push_back(left_idx[i]);
      right_idx_out.push_back(it->second);
    }
  }
}

// for star join
template <class T>
std::vector<size_t> unique_equi_join2(std::vector<T>& left,
                                      std::vector<size_t>& left_idx,
                                      std::vector<T>& right,
                                      std::vector<size_t>& right_idx,
                                      std::vector<size_t>& right_idx_out) {
  std::unordered_map<T,size_t> right_map;
  for(size_t i = 0; i < right.size(); i++) {
    if(right_map.find(right[i]) == right_map.end())
      right_map.insert(std::make_pair(right[i], right_idx[i]));
    else throw std::runtime_error("star_join: right column is not unique");
  }
  std::vector<size_t> missed;
  right_idx_out.resize(left.size());
  for(size_t i = 0; i < left.size(); i++) {
    auto it = right_map.find(left[i]);
    if(it == right_map.end()) {
      missed.push_back(i);
    } else {
      right_idx_out[i] = it->second;
    }
  }
  return missed;
}

template <class T>
std::vector<size_t> outer_equi_join(std::vector<T>& left,
                                    std::vector<size_t>& left_idx,
                                    std::vector<T>& right,
                                    std::vector<size_t>& right_idx,
                                    std::vector<size_t>& left_idx_out, 
                                    std::vector<size_t>& right_idx_out) {
  std::unordered_multimap<T,size_t> right_map;
  for(size_t i = 0; i < right.size(); i++) {
    right_map.insert(std::make_pair(right[i], right_idx[i]));
  }
  std::vector<size_t> only_left;
  left_idx_out.clear();
  right_idx_out.clear();
  for(size_t i = 0; i < left.size(); i++) {
    auto range = right_map.equal_range(left[i]);
    if(range.first == range.second) {
      only_left.push_back(left_idx[i]);
    } else {
      for(auto it = range.first; it != range.second; ++it) {
        left_idx_out.push_back(left_idx[i]);
        right_idx_out.push_back(it->second);
      }
    }
  }
  return only_left;
}

#else

// for star join
template <class T>
std::vector<size_t> unique_equi_join2(std::vector<T>& left,
                                      std::vector<size_t>& left_idx,
                                      std::vector<T>& right,
                                      std::vector<size_t>& right_idx,
                                      std::vector<size_t>& right_idx_out) {
  int is_unique_ok;
  unique_hashtable<T, size_t> ht(right, right_idx, is_unique_ok);
  if(!is_unique_ok)
    throw std::runtime_error("star_join: right column is not unique");
  std::vector<size_t> missed;
  right_idx_out = ht.lookup(left, missed);
  return missed;
}

template <class T>
void multi_equi_join(std::vector<size_t>& sep,
                     std::vector<T>& left,
                     std::vector<size_t>& left_val, // glob. idx
                     std::vector<T>& right,
                     std::vector<size_t>& right_val,
                     std::vector<size_t>& left_val_out_ret,
                     std::vector<size_t>& right_val_out_ret) {
  int valid[SET_VLEN];
  for(int i = 0; i < SET_VLEN; i++) valid[i] = true;
  size_t left_size = left.size();
  size_t right_size = right.size();
  if(left_size != left_val.size() || right_size != right_val.size())
    throw std::runtime_error("sizes of key and value are not the same");
  size_t each = ceil_div(left_size, size_t(SET_VLEN));
  if(each % 2 == 0) each++;

  size_t sep_size = sep.size();
  std::vector<size_t> sepdiff(sep_size - 1);
  for(size_t i = 0; i < sep_size - 1; i++) {
    sepdiff[i] = sep[i+1] - sep[i];
  }
  size_t max = 0;
  for(size_t i = 0; i < sep_size - 1; i++) {
    if(max < sepdiff[i]) max = sepdiff[i];
  }
  size_t left_idx[SET_VLEN];
  size_t right_idx[SET_VLEN];
  size_t left_idx_stop[SET_VLEN];
  size_t out_idx[SET_VLEN];
  size_t out_idx_save[SET_VLEN];
  size_t sep_idx[SET_VLEN];
  size_t* sepp = &sep[0];
  std::vector<size_t> left_val_out(left_size * max);
  std::vector<size_t> right_val_out(left_size * max);

  for(int i = 0; i < SET_VLEN; i++) {
    size_t pos = each * i;
    if(pos < left_size) {
      left_idx[i] = pos;
    } else {
      valid[i] = false;
      left_idx[i] = left_size;
    }
  }
  for(int i = 0; i < SET_VLEN; i++) {
    if(valid[i]) {
      auto it = lower_bound(right.begin(), right.end(), left[left_idx[i]]);
      if(it != right.end()) {
        auto right_idx_i = it - right.begin();
        right_idx[i] = right_idx_i;
        auto it2 = lower_bound(sep.begin(), sep.end(), right_idx_i);
        if(it2 != sep.end()) {
          sep_idx[i] = it2 - sep.begin();
        } else {
          sep_idx[i] = sep_size;
        }
      } else {
        valid[i] = false;
        right_idx[i] = right_size;
        sep_idx[i] = sep_size;
      }
    } else {
      right_idx[i] = right_size;
      sep_idx[i] = sep_size;
    }
  }
  out_idx[0] = 0;
  out_idx_save[0] = 0;
  for(int i = 1; i < SET_VLEN; i++) {
    out_idx[i] = (left_idx[i] - left_idx[i-1]) * max + out_idx[i-1];
    out_idx_save[i] = out_idx[i];
  }
  for(int i = 0; i < SET_VLEN - 1; i++) {
    left_idx_stop[i] = left_idx[i + 1];
  }
  left_idx_stop[SET_VLEN-1] = left_size;
  int valid_0[SET_VLEN_EACH];
  int valid_1[SET_VLEN_EACH];
  int valid_2[SET_VLEN_EACH];
  int valid_3[SET_VLEN_EACH];
  size_t left_idx_0[SET_VLEN_EACH];
  size_t left_idx_1[SET_VLEN_EACH];
  size_t left_idx_2[SET_VLEN_EACH];
  size_t left_idx_3[SET_VLEN_EACH];
  size_t right_idx_0[SET_VLEN_EACH];
  size_t right_idx_1[SET_VLEN_EACH];
  size_t right_idx_2[SET_VLEN_EACH];
  size_t right_idx_3[SET_VLEN_EACH];
  size_t left_idx_stop_0[SET_VLEN_EACH];
  size_t left_idx_stop_1[SET_VLEN_EACH];
  size_t left_idx_stop_2[SET_VLEN_EACH];
  size_t left_idx_stop_3[SET_VLEN_EACH];
  size_t out_idx_0[SET_VLEN_EACH];
  size_t out_idx_1[SET_VLEN_EACH];
  size_t out_idx_2[SET_VLEN_EACH];
  size_t out_idx_3[SET_VLEN_EACH];
#pragma cdir alloc_on_vreg(valid_0,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(valid_1,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(valid_2,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(valid_3,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(left_idx_stop_0,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(left_idx_stop_1,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(left_idx_stop_2,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(left_idx_stop_3,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(left_idx_0,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(left_idx_1,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(left_idx_2,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(left_idx_3,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(right_idx_0,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(right_idx_1,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(right_idx_2,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(right_idx_3,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(out_idx_0,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(out_idx_1,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(out_idx_2,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(out_idx_3,SET_VLEN_EACH)
  for(size_t i = 0; i < SET_VLEN_EACH; i++) {
    valid_0[i] = valid[i];
    valid_1[i] = valid[SET_VLEN_EACH * 1 + i];
    valid_2[i] = valid[SET_VLEN_EACH * 2 + i];
    left_idx_0[i] = left_idx[i];
    left_idx_1[i] = left_idx[SET_VLEN_EACH * 1 + i];
    left_idx_2[i] = left_idx[SET_VLEN_EACH * 2 + i];
    right_idx_0[i] = right_idx[i];
    right_idx_1[i] = right_idx[SET_VLEN_EACH * 1 + i];
    right_idx_2[i] = right_idx[SET_VLEN_EACH * 2 + i];
    left_idx_stop_0[i] = left_idx_stop[i];
    left_idx_stop_1[i] = left_idx_stop[SET_VLEN_EACH * 1 + i];
    left_idx_stop_2[i] = left_idx_stop[SET_VLEN_EACH * 2 + i];
    out_idx_0[i] = out_idx[i];
    out_idx_1[i] = out_idx[SET_VLEN_EACH * 1 + i];
    out_idx_2[i] = out_idx[SET_VLEN_EACH * 2 + i];
  }
  for(size_t i = 0; i < SET_VLEN_EACH; i++) valid_3[i] = false;
  for(size_t i = 0; i < SET_VLEN_EACH3; i++) {
    valid_3[i] = valid[SET_VLEN_EACH * 3 + i];
    left_idx_3[i] = left_idx[SET_VLEN_EACH * 3 + i];
    right_idx_3[i] = right_idx[SET_VLEN_EACH * 3 + i];
    left_idx_stop_3[i] = left_idx_stop[SET_VLEN_EACH * 3 + i];
    out_idx_3[i] = out_idx[SET_VLEN_EACH * 3 + i];
  }
  size_t sepidx0[SET_VLEN_EACH];
  size_t sepidx1[SET_VLEN_EACH];
  size_t sepidx2[SET_VLEN_EACH];
  size_t sepidx3[SET_VLEN_EACH3];
#pragma cdir alloc_on_vreg(sepidx0,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(sepidx1,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(sepidx2,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(sepidx3,SET_VLEN_EACH3)
  for(size_t i = 0; i < SET_VLEN_EACH; i++) {
    sepidx0[i] = sep_idx[i];
    sepidx1[i] = sep_idx[SET_VLEN_EACH * 1 + i];
    sepidx2[i] = sep_idx[SET_VLEN_EACH * 2 + i];
  }
  for(size_t i = 0; i < SET_VLEN_EACH3; i++) {
    sepidx3[i] = sep_idx[SET_VLEN_EACH * 3 + i];
  }
  T* lp = &left[0];
  size_t* lvp = &left_val[0];
  T* rp = &right[0];
  size_t* rvp = &right_val[0];
  size_t* lvop = &left_val_out[0];
  size_t* rvop = &right_val_out[0];
  T leftelm0[SET_VLEN_EACH];
  T leftelm1[SET_VLEN_EACH];
  T leftelm2[SET_VLEN_EACH];
  T leftelm3[SET_VLEN_EACH3];
  T rightelm0[SET_VLEN_EACH];
  T rightelm1[SET_VLEN_EACH];
  T rightelm2[SET_VLEN_EACH];
  T rightelm3[SET_VLEN_EACH3];
  T leftelm0_next[SET_VLEN_EACH];
  T leftelm1_next[SET_VLEN_EACH];
  T leftelm2_next[SET_VLEN_EACH];
  T leftelm3_next[SET_VLEN_EACH3];
#pragma cdir alloc_on_vreg(leftelm0,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(leftelm1,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(leftelm2,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(leftelm3,SET_VLEN_EACH3)
#pragma cdir alloc_on_vreg(rightelm0,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(rightelm1,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(rightelm2,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(rightelm3,SET_VLEN_EACH3)
#pragma cdir alloc_on_vreg(leftelm0_next,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(leftelm1_next,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(leftelm2_next,SET_VLEN_EACH)
#pragma cdir alloc_on_vreg(leftelm3_next,SET_VLEN_EACH3)
  for(int j = 0; j < SET_VLEN_EACH; j++) {
    if(valid_0[j]) leftelm0_next[j] = lp[left_idx_0[j]];
    if(valid_1[j]) leftelm1_next[j] = lp[left_idx_1[j]];
    if(valid_2[j]) leftelm2_next[j] = lp[left_idx_2[j]];
  }
  for(int j = 0; j < SET_VLEN_EACH3; j++) {
    if(valid_3[j]) leftelm3_next[j] = lp[left_idx_3[j]];
  }
  while(1) {
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_0[j]) leftelm0[j] = leftelm0_next[j];
      if(valid_0[j]) rightelm0[j] = rp[right_idx_0[j]];
      if(valid_1[j]) leftelm1[j] = leftelm1_next[j];
      if(valid_1[j]) rightelm1[j] = rp[right_idx_1[j]];
      if(valid_2[j]) leftelm2[j] = leftelm2_next[j];
      if(valid_2[j]) rightelm2[j] = rp[right_idx_2[j]];
    }
    for(int j = 0; j < SET_VLEN_EACH3; j++) {
      if(valid_3[j]) leftelm3[j] = leftelm3_next[j];
      if(valid_3[j]) rightelm3[j] = rp[right_idx_3[j]];
    }
    size_t leftval0[SET_VLEN_EACH];
    size_t leftval1[SET_VLEN_EACH];
    size_t leftval2[SET_VLEN_EACH];
    size_t leftval3[SET_VLEN_EACH3];
    size_t rightval0[SET_VLEN_EACH];
    size_t rightval1[SET_VLEN_EACH];
    size_t rightval2[SET_VLEN_EACH];
    size_t rightval3[SET_VLEN_EACH3];
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
        int gt = leftelm0[j] > rightelm0[j];
        int check = false;
        if(eq) {
          lvop[out_idx_0[j]] = leftval0[j];
          rvop[out_idx_0[j]++] = rightval0[j];
          right_idx_0[j]++;
        } else if(gt) {
          sepidx0[j]++;
          right_idx_0[j] = sepp[sepidx0[j]];
        } else {
          left_idx_0[j]++;
          check = true;
        }
        if(left_idx_0[j] == left_idx_stop_0[j]) {
          valid_0[j] = false;
        }
        if(valid_0[j]) leftelm0_next[j] = lp[left_idx_0[j]];
        if(valid_0[j] && check && leftelm0[j] == leftelm0_next[j]) {
          right_idx_0[j] = sepp[sepidx0[j]];
        }
        if(right_idx_0[j] == right_size) valid_0[j] = false;
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_1[j]) {
        int eq = leftelm1[j] == rightelm1[j];
        int gt = leftelm1[j] > rightelm1[j];
        int check = false;
        if(eq) {
          lvop[out_idx_1[j]] = leftval1[j];
          rvop[out_idx_1[j]++] = rightval1[j];
          right_idx_1[j]++;
        } else if(gt) {
          sepidx1[j]++;
          right_idx_1[j] = sepp[sepidx1[j]];
        } else {
          left_idx_1[j]++;
          check = true;
        }
        if(left_idx_1[j] == left_idx_stop_1[j]) {
          valid_1[j] = false;
        }
        if(valid_1[j]) leftelm1_next[j] = lp[left_idx_1[j]];
        if(valid_1[j] && check && leftelm1[j] == leftelm1_next[j]) {
          right_idx_1[j] = sepp[sepidx1[j]];
        }
        if(right_idx_1[j] == right_size) valid_1[j] = false;
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH; j++) {
      if(valid_2[j]) {
        int eq = leftelm2[j] == rightelm2[j];
        int gt = leftelm2[j] > rightelm2[j];
        int check = false;
        if(eq) {
          lvop[out_idx_2[j]] = leftval2[j];
          rvop[out_idx_2[j]++] = rightval2[j];
          right_idx_2[j]++;
        } else if(gt) {
          sepidx2[j]++;
          right_idx_2[j] = sepp[sepidx2[j]];
        } else {
          left_idx_2[j]++;
          check = true;
        }
        if(left_idx_2[j] == left_idx_stop_2[j]) {
          valid_2[j] = false;
        }
        if(valid_2[j]) leftelm2_next[j] = lp[left_idx_2[j]];
        if(valid_2[j] && check && leftelm2[j] == leftelm2_next[j]) {
          right_idx_2[j] = sepp[sepidx2[j]];
        }
        if(right_idx_2[j] == right_size) valid_2[j] = false;
      }
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < SET_VLEN_EACH3; j++) {
      if(valid_3[j]) {
        int eq = leftelm3[j] == rightelm3[j];
        int gt = leftelm3[j] > rightelm3[j];
        int check = false;
        if(eq) {
          lvop[out_idx_3[j]] = leftval3[j];
          rvop[out_idx_3[j]++] = rightval3[j];
          right_idx_3[j]++;
        } else if(gt) {
          sepidx3[j]++;
          right_idx_3[j] = sepp[sepidx3[j]];
        } else {
          left_idx_3[j]++;
          check = true;
        }
        if(left_idx_3[j] == left_idx_stop_3[j]) {
          valid_3[j] = false;
        }
        if(valid_3[j]) leftelm3_next[j] = lp[left_idx_3[j]];
        if(valid_3[j] && check && leftelm3[j] == leftelm3_next[j]) {
          right_idx_3[j] = sepp[sepidx3[j]];
        }
        if(right_idx_3[j] == right_size) valid_3[j] = false;
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
    total += out_idx[i] - out_idx_save[i];
  }
  left_val_out_ret.resize(total);
  right_val_out_ret.resize(total);
  size_t* leftretp = &left_val_out_ret[0];
  size_t* rightretp = &right_val_out_ret[0];
  size_t current = 0;
  for(size_t i = 0; i < SET_VLEN; i++) {
    for(size_t j = 0; j < out_idx[i] - out_idx_save[i]; j++) {
      auto pos = out_idx_save[i];
      leftretp[current] = lvop[pos + j];
      rightretp[current++] = rvop[pos + j];
    }
  }
}

template <class T>
void equi_join(std::vector<T>& left,
               std::vector<size_t>& left_idx,
               std::vector<T>& right,
               std::vector<size_t>& right_idx,
               std::vector<size_t>& left_idx_out,
               std::vector<size_t>& right_idx_out) {
  int is_unique_ok;
  unique_hashtable<T, size_t> ht(right, right_idx, is_unique_ok);
  if(is_unique_ok) {
    std::vector<size_t> missed;
    auto looked_up = ht.lookup(left, missed);
    left_idx_out = shrink_missed(left_idx, missed);
    right_idx_out = shrink_missed(looked_up, missed);
  } else {
    ht.clear();
    radix_sort(right, right_idx);
    auto sep = set_separate(right);
    radix_sort(left, left_idx);
    multi_equi_join(sep, left, left_idx, right, right_idx,
                    left_idx_out, right_idx_out);
  }
}

template <class T>
std::vector<size_t> outer_equi_join(std::vector<T>& left,
                                    std::vector<size_t>& left_idx,
                                    std::vector<T>& right,
                                    std::vector<size_t>& right_idx,
                                    std::vector<size_t>& left_idx_out, 
                                    std::vector<size_t>& right_idx_out) {
  int is_unique_ok;
  unique_hashtable<T, size_t> ht(right, right_idx, is_unique_ok);
  if(is_unique_ok) {
    std::vector<size_t> missed;
    auto looked_up = ht.lookup(left, missed);
    size_t missedsize = missed.size();
    left_idx_out = shrink_missed(left_idx, missed);
    right_idx_out = shrink_missed(looked_up, missed);
    size_t* left_idxp = &left_idx[0];
    size_t* missedp = &missed[0];
    std::vector<size_t> ret(missedsize);
    size_t* retp = &ret[0];
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t i = 0; i < missedsize; i++) {
      retp[i] = left_idxp[missedp[i]];
    }
    return ret;
  } else {
    radix_sort(right, right_idx);
    auto sep = set_separate(right);
    radix_sort(left, left_idx);
    multi_equi_join(sep, left, left_idx, right, right_idx,
                    left_idx_out, right_idx_out);
    size_t left_idx_out_size = left_idx_out.size();
    size_t left_idx_size = left_idx.size();
    std::vector<int> dummy(left_idx_out_size);
    auto idxhash = unique_hashtable<size_t, int>(left_idx_out, dummy);
    auto idx_exist = idxhash.check_existence(left_idx);

    std::vector<size_t> onlylefttmp(left_idx_size);
    size_t* onlylefttmpp = onlylefttmp.data();
    int* idx_existp = idx_exist.data();
    size_t* left_idxp = left_idx.data();
    size_t current = 0;
    for(size_t i = 0; i < left_idx_size; i++) {
      if(idx_existp[i] == 0) {
        onlylefttmpp[current++] = left_idxp[i];
      }
    }
    std::vector<size_t> onlyleft(current);
    size_t* onlyleftp = onlyleft.data();
    for(size_t i = 0; i < current; i++) onlyleftp[i] = onlylefttmpp[i];
    return onlyleft;
  }
}

#endif

}
#endif
