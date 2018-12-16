#ifndef PREFIX_SUM_HPP
#define PREFIX_SUM_HPP

#include <vector>
#include "utility.hpp"

#define PREFIX_SUM_VLEN 256
#define PREFIX_SUM_VLEN_MIN 32 // minimum VLEN of 2nd part

namespace frovedis {

#if defined(_SX) || defined(__ve__)
template <class T>
void prefix_sum(const T* valp, T* outp, size_t size) {
  // If vector length for second part is small, do scalar
  if (size < PREFIX_SUM_VLEN * PREFIX_SUM_VLEN_MIN) {
    T current_val = 0;
    // no vector loop
    for(size_t j = 0; j < size; j++) {
      auto loaded_v = valp[j];
      auto next_val = loaded_v + current_val;
      outp[j] = next_val;
      current_val = next_val;
    }
    return;
  }
  // do vectorized version of prefix_sum
  size_t each = size / PREFIX_SUM_VLEN;
  if(each % 2 == 0 && each > 1) each--;
  size_t rest = size - each * PREFIX_SUM_VLEN;
  T current_val[PREFIX_SUM_VLEN];
#pragma _NEC vreg(current_val)
  for(int i = 0; i < PREFIX_SUM_VLEN; i++) {
    current_val[i] = 0;
  }
  // each should always be >= 1
  for(size_t j = 0; j < each; j++) {
    for(size_t i = 0; i < PREFIX_SUM_VLEN; i++) {
      auto loaded_v = valp[j + each * i];
      auto next_val = loaded_v + current_val[i];
      outp[j + each * i] = next_val;
      current_val[i] = next_val;
    }
  }
  size_t rest_idx_start = each * PREFIX_SUM_VLEN;
  T current_val_rest = 0;
  for(size_t j = 0; j < rest; j++) {
    auto loaded_v = valp[j + rest_idx_start];
    auto next_val = loaded_v + current_val_rest;
    outp[j + rest_idx_start] = next_val;
    current_val_rest = next_val;
  }
  for(size_t i = 1; i < PREFIX_SUM_VLEN; i++) {
    T to_add = outp[each * i - 1];
    for(size_t j = each * i; j < each * (i+1); j++) {
      outp[j] += to_add;
    }
  }
  T to_add = outp[each * PREFIX_SUM_VLEN - 1];
  for(size_t j = each * PREFIX_SUM_VLEN;
      j < each * PREFIX_SUM_VLEN + rest; j++) {
    outp[j] += to_add;
  }
  return;
}
#else
template <class T>
void prefix_sum(const T* valp, T* outp, size_t size) {
  T current_val = 0;
  for(size_t j = 0; j < size; j++) {
    auto loaded_v = valp[j];
    auto next_val = loaded_v + current_val;
    outp[j] = next_val;
    current_val = next_val;
  }
}
#endif

template <class T>
std::vector<T> prefix_sum(const std::vector<T>& val) {
  size_t size = val.size();
  if(size == 0) {return std::vector<T>();}
  std::vector<T> out(size);
  T* outp = out.data();
  const T* valp = val.data();
  prefix_sum(valp, outp, size);
  return out;
}

}
#endif
