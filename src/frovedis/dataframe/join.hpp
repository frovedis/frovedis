#ifndef JOIN_HPP
#define JOIN_HPP

#include <vector>
#if !(defined(_SX) || defined(__ve__))
#include <unordered_map>
#else
#include "../core/radix_sort.hpp"
#include "../core/set_operations.hpp"
#include "../core/lower_bound.hpp"
#include "../core/upper_bound.hpp"
#include "hashtable.hpp"
#include <limits>
#endif

#define JOIN_VLEN 256

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
void multi_equi_join_vreg(int* valid, size_t* left_idx, size_t* right_idx,
                          size_t* left_idx_stop, size_t* out_idx,
                          size_t* sep_idx, T* lp, size_t* lvp, T* rp,
                          size_t* rvp, size_t* lvop, size_t* rvop,
                          size_t* sepp, size_t right_size) {
  T leftelm[JOIN_VLEN];
  T rightelm[JOIN_VLEN];
  T leftelm_next[JOIN_VLEN];
#pragma _NEC vreg(leftelm)
#pragma _NEC vreg(rightelm)
#pragma _NEC vreg(leftelm_next)
  for(int j = 0; j < JOIN_VLEN; j++) {
    if(valid[j]) leftelm_next[j] = lp[left_idx[j]];
  }
  T rightmax = std::numeric_limits<T>::max();
  int valid_vreg[JOIN_VLEN];
  size_t left_idx_vreg[JOIN_VLEN];
  size_t right_idx_vreg[JOIN_VLEN];
  size_t left_idx_stop_vreg[JOIN_VLEN];
  size_t out_idx_vreg[JOIN_VLEN];
  size_t sep_idx_vreg[JOIN_VLEN];
#pragma _NEC vreg(valid_vreg)
#pragma _NEC vreg(left_idx_vreg)
#pragma _NEC vreg(right_idx_vreg)
#pragma _NEC vreg(left_idx_stop_vreg)
#pragma _NEC vreg(out_idx_vreg)
#pragma _NEC vreg(sep_idx_vreg)
  for(size_t i = 0; i < JOIN_VLEN; i++) {
    valid_vreg[i] = valid[i];
    left_idx_vreg[i] = left_idx[i];
    right_idx_vreg[i] = right_idx[i];
    left_idx_stop_vreg[i] = left_idx_stop[i];
    out_idx_vreg[i] = out_idx[i];
    sep_idx_vreg[i] = sep_idx[i];
  }
  while(1) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(int j = 0; j < JOIN_VLEN; j++) {
      if(valid_vreg[j]) {
        leftelm[j] = leftelm_next[j];
        if(right_idx_vreg[j] == right_size) rightelm[j] = rightmax;
        else rightelm[j] = rp[right_idx_vreg[j]];
        if(leftelm[j] == rightelm[j]) {
          lvop[out_idx_vreg[j]] = lvp[left_idx_vreg[j]];
          rvop[out_idx_vreg[j]++] = rvp[right_idx_vreg[j]];
          right_idx_vreg[j]++;
        } else if(leftelm[j] > rightelm[j]) {
          sep_idx_vreg[j]++;
          right_idx_vreg[j] = sepp[sep_idx_vreg[j]];
        } else {
          left_idx_vreg[j]++;
          if(left_idx_vreg[j] == left_idx_stop_vreg[j]) {
            valid_vreg[j] = false;
          } else {
            leftelm_next[j] = lp[left_idx_vreg[j]];
            if(leftelm[j] == leftelm_next[j]) {
              right_idx_vreg[j] = sepp[sep_idx_vreg[j]];
            }
          }
        }
      }
    }
    int any_valid = false;
    for(int i = 0; i < JOIN_VLEN; i++) {
      if(valid_vreg[i]) any_valid = true;
    }
    if(any_valid == false) break;
  }
  for(size_t i = 0; i < JOIN_VLEN; i++) {
    out_idx[i] = out_idx_vreg[i];
  }
}


template <class T>
void multi_equi_join(std::vector<size_t>& sep,
                     std::vector<T>& left,
                     std::vector<size_t>& left_val, // glob. idx
                     std::vector<T>& right,
                     std::vector<size_t>& right_val,
                     std::vector<size_t>& left_val_out_ret,
                     std::vector<size_t>& right_val_out_ret) {
  int valid[JOIN_VLEN];
  for(int i = 0; i < JOIN_VLEN; i++) valid[i] = true;
  size_t left_size = left.size();
  size_t right_size = right.size();
  if(left_size != left_val.size() || right_size != right_val.size())
    throw std::runtime_error("sizes of key and value are not the same");
  size_t each = ceil_div(left_size, size_t(JOIN_VLEN));
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
  size_t left_idx[JOIN_VLEN];
  size_t right_idx[JOIN_VLEN];
  size_t left_idx_stop[JOIN_VLEN];
  size_t out_idx[JOIN_VLEN];
  size_t out_idx_save[JOIN_VLEN];
  size_t sep_idx[JOIN_VLEN];
  size_t* sepp = &sep[0];
  std::vector<size_t> left_val_out(left_size * max);
  std::vector<size_t> right_val_out(left_size * max);

  for(int i = 0; i < JOIN_VLEN; i++) {
    size_t pos = each * i;
    if(pos < left_size) {
      left_idx[i] = pos;
    } else {
      valid[i] = false;
      left_idx[i] = left_size;
    }
  }
  for(int i = 0; i < JOIN_VLEN; i++) {
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
  for(int i = 1; i < JOIN_VLEN; i++) {
    out_idx[i] = (left_idx[i] - left_idx[i-1]) * max + out_idx[i-1];
    out_idx_save[i] = out_idx[i];
  }
  for(int i = 0; i < JOIN_VLEN - 1; i++) {
    left_idx_stop[i] = left_idx[i + 1];
  }
  left_idx_stop[JOIN_VLEN-1] = left_size;
  T* lp = &left[0];
  size_t* lvp = &left_val[0];
  T* rp = &right[0];
  size_t* rvp = &right_val[0];
  size_t* lvop = &left_val_out[0];
  size_t* rvop = &right_val_out[0];

  multi_equi_join_vreg(valid, left_idx, right_idx, left_idx_stop,
                       out_idx, sep_idx, lp, lvp, rp, rvp, lvop, rvop,
                       sepp, right_size);

  size_t total = 0;
  for(size_t i = 0; i < JOIN_VLEN; i++) {
    total += out_idx[i] - out_idx_save[i];
  }
  left_val_out_ret.resize(total);
  right_val_out_ret.resize(total);
  size_t* leftretp = &left_val_out_ret[0];
  size_t* rightretp = &right_val_out_ret[0];
  size_t current = 0;
  for(size_t i = 0; i < JOIN_VLEN; i++) {
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

// left, left_idx, right, right_idx are destructed
template <class T>
void lt_join(std::vector<T>& left,
             std::vector<size_t>& left_idx,
             std::vector<T>& right,
             std::vector<size_t>& right_idx,
             std::vector<size_t>& left_idx_out,
             std::vector<size_t>& right_idx_out) {
  radix_sort(right, right_idx);
  auto sep = upper_bound(right, left);
  auto sep_size = sep.size();
  std::vector<size_t> sizes(sep_size);
  auto sepp = sep.data();
  auto sizesp = sizes.data();
  auto right_size = right.size();
  for(size_t i = 0; i < sep_size; i++) {
    sizesp[i] = right_size - sepp[i];
  }
  size_t total = 0;
  for(size_t i = 0; i < sep_size; i++) {
    total += sizesp[i];
  }
  left_idx_out.resize(total);
  right_idx_out.resize(total);
  auto left_idx_outp = left_idx_out.data();
  auto right_idx_outp = right_idx_out.data();
  auto left_idxp = left_idx.data();
  auto right_idxp = right_idx.data();
  auto left_size = left.size();
  auto crnt_left_idx_outp = left_idx_outp;
  auto crnt_right_idx_outp = right_idx_outp;
  for(size_t i = 0; i < left_size; i++) {
    auto crnt_right_idxp = right_idxp + sepp[i];
    for(size_t j = 0; j < sizes[i]; j++) {
      crnt_left_idx_outp[j] = left_idxp[i];
      crnt_right_idx_outp[j] = crnt_right_idxp[j];
    }
    crnt_left_idx_outp += sizesp[i];
    crnt_right_idx_outp += sizesp[i];
  }
}

template <class T>
void le_join(std::vector<T>& left,
             std::vector<size_t>& left_idx,
             std::vector<T>& right,
             std::vector<size_t>& right_idx,
             std::vector<size_t>& left_idx_out,
             std::vector<size_t>& right_idx_out) {
  radix_sort(right, right_idx);
  auto sep = lower_bound(right, left);
  auto sep_size = sep.size();
  std::vector<size_t> sizes(sep_size);
  auto sepp = sep.data();
  auto sizesp = sizes.data();
  auto right_size = right.size();
  for(size_t i = 0; i < sep_size; i++) {
    sizesp[i] = right_size - sepp[i];
  }
  size_t total = 0;
  for(size_t i = 0; i < sep_size; i++) {
    total += sizesp[i];
  }
  left_idx_out.resize(total);
  right_idx_out.resize(total);
  auto left_idx_outp = left_idx_out.data();
  auto right_idx_outp = right_idx_out.data();
  auto left_idxp = left_idx.data();
  auto right_idxp = right_idx.data();
  auto left_size = left.size();
  auto crnt_left_idx_outp = left_idx_outp;
  auto crnt_right_idx_outp = right_idx_outp;
  for(size_t i = 0; i < left_size; i++) {
    auto crnt_right_idxp = right_idxp + sepp[i];
    for(size_t j = 0; j < sizes[i]; j++) {
      crnt_left_idx_outp[j] = left_idxp[i];
      crnt_right_idx_outp[j] = crnt_right_idxp[j];
    }
    crnt_left_idx_outp += sizesp[i];
    crnt_right_idx_outp += sizesp[i];
  }
}

template <class T>
void gt_join(std::vector<T>& left,
             std::vector<size_t>& left_idx,
             std::vector<T>& right,
             std::vector<size_t>& right_idx,
             std::vector<size_t>& left_idx_out,
             std::vector<size_t>& right_idx_out) {
  radix_sort(right, right_idx);
  auto sep = lower_bound(right, left);
  auto sep_size = sep.size();
  auto sepp = sep.data();
  size_t total = 0;
  for(size_t i = 0; i < sep_size; i++) {
    total += sepp[i];
  }
  left_idx_out.resize(total);
  right_idx_out.resize(total);
  auto left_idx_outp = left_idx_out.data();
  auto right_idx_outp = right_idx_out.data();
  auto left_idxp = left_idx.data();
  auto right_idxp = right_idx.data();
  auto left_size = left.size();
  auto crnt_left_idx_outp = left_idx_outp;
  auto crnt_right_idx_outp = right_idx_outp;
  for(size_t i = 0; i < left_size; i++) {
    for(size_t j = 0; j < sepp[i]; j++) {
      crnt_left_idx_outp[j] = left_idxp[i];
      crnt_right_idx_outp[j] = right_idxp[j];
    }
    crnt_left_idx_outp += sepp[i];
    crnt_right_idx_outp += sepp[i];
  }
}

template <class T>
void ge_join(std::vector<T>& left,
             std::vector<size_t>& left_idx,
             std::vector<T>& right,
             std::vector<size_t>& right_idx,
             std::vector<size_t>& left_idx_out,
             std::vector<size_t>& right_idx_out) {
  radix_sort(right, right_idx);
  auto sep = upper_bound(right, left);
  auto sep_size = sep.size();
  auto sepp = sep.data();
  size_t total = 0;
  for(size_t i = 0; i < sep_size; i++) {
    total += sepp[i];
  }
  left_idx_out.resize(total);
  right_idx_out.resize(total);
  auto left_idx_outp = left_idx_out.data();
  auto right_idx_outp = right_idx_out.data();
  auto left_idxp = left_idx.data();
  auto right_idxp = right_idx.data();
  auto left_size = left.size();
  auto crnt_left_idx_outp = left_idx_outp;
  auto crnt_right_idx_outp = right_idx_outp;
  for(size_t i = 0; i < left_size; i++) {
    for(size_t j = 0; j < sepp[i]; j++) {
      crnt_left_idx_outp[j] = left_idxp[i];
      crnt_right_idx_outp[j] = right_idxp[j];
    }
    crnt_left_idx_outp += sepp[i];
    crnt_right_idx_outp += sepp[i];
  }
}

}
#endif
