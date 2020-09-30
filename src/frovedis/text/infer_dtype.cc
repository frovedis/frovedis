#include "infer_dtype.hpp"

namespace frovedis {

inferred_dtype infer_dtype(const int* charsp, const size_t* org_startsp,
                           const size_t* lensp, size_t num_words) {

  std::vector<size_t> starts(num_words), stops(num_words);
  auto startsp = starts.data();
  auto stopsp = stops.data();
  for(size_t i = 0; i < num_words; i++) {
    startsp[i] = org_startsp[i];
    stopsp[i] = startsp[i] + lensp[i];
  }
#pragma _NEC ivdep
  for(size_t i = 0; i < num_words; i++) {
    if(startsp[i] != stopsp[i]) {
      auto c = charsp[startsp[i]];
      if(c == '+' || c == '-') {
        startsp[i]++;
      }
    }
  }
  int contain_zero = false;
#pragma _NEC ivdep
  for(size_t i = 0; i < num_words; i++) {
    if(startsp[i] == stopsp[i]) contain_zero = true;
  }
  if(contain_zero) return inferred_dtype::inferred_dtype_string;

  // float or int or string
  int anyvalid = true;
  while(anyvalid) {
    anyvalid = false;
#pragma _NEC ivdep
    for(size_t i = 0; i < num_words; i++) {
      if(startsp[i] != stopsp[i]) {
        auto c = charsp[startsp[i]];
        if(c >= '0' && c <= '9') {
          startsp[i]++;
          anyvalid = true;
        } 
      } 
    }
  }
  int contain_non_zero = false;
#pragma _NEC ivdep
  for(size_t i = 0; i < num_words; i++) {
    if(startsp[i] != stopsp[i]) contain_non_zero = true;
  }
  if(!contain_non_zero) return inferred_dtype::inferred_dtype_int;

  // float or string
  int nonnum = false;
#pragma _NEC ivdep
  for(size_t i = 0; i < num_words; i++) {
    if(startsp[i] != stopsp[i]) {
      auto c = charsp[startsp[i]];
      if(c == '.') {
        startsp[i]++;
      } else {
        nonnum = true;
      }
    }
  }
  if(nonnum) return inferred_dtype::inferred_dtype_string;
  anyvalid = true;
  while(anyvalid) {
    anyvalid = false;
#pragma _NEC ivdep
    for(size_t i = 0; i < num_words; i++) {
      if(startsp[i] != stopsp[i]) {
        auto c = charsp[startsp[i]];
        if(c >= '0' && c <= '9') {
          startsp[i]++;
          anyvalid = true;
        } 
      }
    }
  }
  contain_non_zero = false;
#pragma _NEC ivdep
  for(size_t i = 0; i < num_words; i++) {
    if(startsp[i] != stopsp[i]) contain_non_zero = true;
  }
  if(!contain_non_zero) return inferred_dtype::inferred_dtype_float;

  nonnum = false;
#pragma _NEC ivdep
  for(size_t i = 0; i < num_words; i++) {
    if(startsp[i] != stopsp[i]) {
      auto c = charsp[startsp[i]];
      if(c == 'E' || c == 'e') {
        startsp[i]++;
      } else {
        nonnum = true;
      }
    }
  }
  if(nonnum) return inferred_dtype::inferred_dtype_string;
#pragma _NEC ivdep
  for(size_t i = 0; i < num_words; i++) {
    if(startsp[i] != stopsp[i]) {
      auto c = charsp[startsp[i]];
      if(c == '+' || c == '-') {
        startsp[i]++;
      }
    }
  }
  anyvalid = true;
  while(anyvalid) {
    anyvalid = false;
#pragma _NEC ivdep
    for(size_t i = 0; i < num_words; i++) {
      if(startsp[i] != stopsp[i]) {
        auto c = charsp[startsp[i]];
        if(c >= '0' && c <= '9') {
          startsp[i]++;
          anyvalid = true;
        } 
      }
    }
  }
  contain_non_zero = false;
#pragma _NEC ivdep
  for(size_t i = 0; i < num_words; i++) {
    if(startsp[i] != stopsp[i]) contain_non_zero = true;
  }
  if(!contain_non_zero) return inferred_dtype::inferred_dtype_float;
  else return inferred_dtype::inferred_dtype_string;
}

inferred_dtype infer_dtype(const std::vector<int>& chars,
                           const std::vector<size_t>& org_starts,
                           const std::vector<size_t>& org_lens,
                           size_t words_to_see) {
  size_t num_words = std::min(org_starts.size(), words_to_see);
  std::vector<size_t> starts(num_words), lens(num_words);
  auto startsp = starts.data();
  auto lensp = lens.data();
  auto org_startsp = org_starts.data();
  auto org_lensp = org_lens.data();
  for(size_t i = 0; i < num_words; i++) {
    startsp[i] = org_startsp[i];
    lensp[i] = org_lensp[i];
  }
  auto inf_idx = find_condition_index(chars, starts, is_inf_word());
  auto minus_idx = find_condition_index(chars, starts, is_minus_word());
  auto minus_idxp = minus_idx.data();
  auto minus_idx_size = minus_idx.size();
  std::vector<size_t> starts_tmp(minus_idx_size);
  auto starts_tmpp = starts_tmp.data();
  for(size_t i = 0; i < minus_idx_size; i++) {
    // we assume the chars are correct data;
    // e.g. "...,-,inf,..." does not happen
    starts_tmpp[i] = startsp[minus_idxp[i]] + 1; 
  }
  auto ninf_idx_tmp = find_condition_index(chars, starts_tmp, is_inf_word());
  auto ninf_idx_size = ninf_idx_tmp.size();
  std::vector<size_t> ninf_idx(ninf_idx_size);
  auto ninf_idxp = ninf_idx.data();
  auto ninf_idx_tmpp = ninf_idx_tmp.data();
  for(size_t i = 0; i < ninf_idx_size; i++) {
    ninf_idxp[i] = minus_idxp[ninf_idx_tmpp[i]];
  }
  auto nan_idx = find_condition_index(chars, starts, is_nan_word());
  if(inf_idx.size() == 0 && ninf_idx.size() == 0 && nan_idx.size() == 0) {
    auto charsp = chars.data();
    auto startsp = starts.data();
    auto lensp = lens.data();
    return infer_dtype(charsp, startsp, lensp, num_words);
  } else {
    // should be float
    auto starts_size = starts.size();
    std::vector<size_t> iota(starts_size);
    auto iotap = iota.data();
    for(size_t i = 0; i < starts_size; i++) iotap[i] = i;
    auto inf_ninf_idx = set_merge(inf_idx, ninf_idx);
    auto inf_ninf_nan_idx = set_merge(inf_ninf_idx, nan_idx);
    auto normal_fp_idx = set_difference(iota, inf_ninf_nan_idx);
    auto normal_fp_idx_size = normal_fp_idx.size();
    auto normal_fp_idxp = normal_fp_idx.data();
    std::vector<size_t> normal_fp_starts(normal_fp_idx_size);
    std::vector<size_t> normal_fp_lens(normal_fp_idx_size);
    auto normal_fp_startsp = normal_fp_starts.data();
    auto normal_fp_lensp = normal_fp_lens.data();
    auto startsp = starts.data();
    auto lensp = lens.data();
    for(size_t i = 0; i < normal_fp_idx_size; i++) {
      normal_fp_startsp[i] = startsp[normal_fp_idxp[i]];
      normal_fp_lensp[i] = lensp[normal_fp_idxp[i]];
    }
    auto charsp = chars.data();
    auto ret_dtype = infer_dtype(charsp, normal_fp_startsp, normal_fp_lensp,
                                 normal_fp_idx_size);
    if(ret_dtype == inferred_dtype::inferred_dtype_float ||
       ret_dtype == inferred_dtype::inferred_dtype_int)
      return inferred_dtype::inferred_dtype_float;
    else return inferred_dtype::inferred_dtype_string;
  }
}


inferred_dtype infer_dtype(const words& w, size_t words_to_see) {
  return infer_dtype(w.chars, w.starts, w.lens, words_to_see);
}

}
