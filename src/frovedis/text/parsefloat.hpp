#ifndef PARSEFLOAT_HPP
#define PARSEFLOAT_HPP

#include "words.hpp"
#include "../core/find_condition.hpp"
#include "../core/set_operations.hpp"
#include <stdexcept>
#include <limits>
#include <math.h>

#if defined(__ve__) || defined(_SX)
#define PARSEFLOAT_VLEN 256
#else
//#define PARSEFLOAT_VLEN 1
#define PARSEFLOAT_VLEN 4
#endif


namespace frovedis {

template <class T>
void parsefloat(const int* charsp, const size_t* startsp,
                const size_t* lensp, size_t num_words, T* retp) {
  
  auto block = num_words / PARSEFLOAT_VLEN;
  auto rest = num_words - block * PARSEFLOAT_VLEN;

  size_t starts_buf[PARSEFLOAT_VLEN];
#pragma _NEC vreg(starts_buf)
  size_t stops_buf[PARSEFLOAT_VLEN];
#pragma _NEC vreg(stops_buf)
  T res_buf[PARSEFLOAT_VLEN];
#pragma _NEC vreg(res_buf)
  T sign_buf[PARSEFLOAT_VLEN];
#pragma _NEC vreg(sign_buf)
  T e_buf[PARSEFLOAT_VLEN];
#pragma _NEC vreg(e_buf)
  int esign_buf[PARSEFLOAT_VLEN];
//#pragma _NEC vreg(esign_buf)
  int ispoint[PARSEFLOAT_VLEN];
//#pragma _NEC vreg(ispoint)
  int isexp[PARSEFLOAT_VLEN];
//#pragma _NEC vreg(isexp)
  T e_val_buf[PARSEFLOAT_VLEN];
#pragma _NEC vreg(e_val_buf)

  // to surpress "may be used uninitialized" warning  
  for(int i = 0; i < PARSEFLOAT_VLEN; i++) {
    starts_buf[i] = 0;
    stops_buf[i] = 0;
    res_buf[i] = 0;
    sign_buf[i] = 0;
    e_buf[i] = 0;
    esign_buf[i] = 0;
    ispoint[i] = 0;
    isexp[i] = 0;
    e_val_buf[i] = 0;
  }

  int anyvalid = true;

  for(size_t b = 0; b < block; b++) {
    auto crnt_startsp = startsp + b * PARSEFLOAT_VLEN;
    auto crnt_lensp = lensp + b * PARSEFLOAT_VLEN;
    auto crnt_retp = retp + b * PARSEFLOAT_VLEN;
    for(size_t i = 0; i < PARSEFLOAT_VLEN; i++) {
      starts_buf[i] = crnt_startsp[i];
      stops_buf[i] = starts_buf[i] + crnt_lensp[i];
      sign_buf[i] = 1;
      res_buf[i] = 0;
      e_buf[i] = 0;
      esign_buf[i] = 1;
      e_val_buf[i] = 0;
    }
    for(size_t i = 0; i < PARSEFLOAT_VLEN; i++) {
      if(starts_buf[i] != stops_buf[i]) {
        auto c = charsp[starts_buf[i]];
        if(c == '+') {
          starts_buf[i]++;
        } else if (c == '-') {
          starts_buf[i]++;
          sign_buf[i] = -1;
        }
      }
    }
    anyvalid = true;
    while(anyvalid) {
      anyvalid = false;
#pragma _NEC ivdep
      for(size_t i = 0; i < PARSEFLOAT_VLEN; i++) {
        if(starts_buf[i] != stops_buf[i]) {
          auto c = charsp[starts_buf[i]];
          if(c >= '0' && c <= '9') {
            res_buf[i] = res_buf[i] * 10 + c - '0';
            starts_buf[i]++;
            anyvalid = true;
          }
        } 
      }
    }
#pragma _NEC ivdep
    for(size_t i = 0; i < PARSEFLOAT_VLEN; i++) {
      if(starts_buf[i] != stops_buf[i]) {
        auto c = charsp[starts_buf[i]];
        if(c == '.') {
          ispoint[i] = true;
          starts_buf[i]++;
        } else {
          ispoint[i] = false;
        }
      }
    }
    anyvalid = true;
    while(anyvalid) {
      anyvalid = false;
#pragma _NEC ivdep
      for(size_t i = 0; i < PARSEFLOAT_VLEN; i++) {
        if(starts_buf[i] != stops_buf[i] && ispoint[i]) {
          auto c = charsp[starts_buf[i]];
          if(c >= '0' && c <= '9') {
            res_buf[i] = res_buf[i] * 10 + c - '0';
            starts_buf[i]++;
            e_buf[i]--;
            anyvalid = true;
          }
        }
      }
    }
#pragma _NEC ivdep
    for(size_t i = 0; i < PARSEFLOAT_VLEN; i++) {
      if(starts_buf[i] != stops_buf[i]) {
        auto c = charsp[starts_buf[i]];
        if(c == 'E' || c == 'e') {
          isexp[i] = true;
          starts_buf[i]++;
        } else {
          isexp[i] = false;
        }
      }
    }
#pragma _NEC ivdep
    for(size_t i = 0; i < PARSEFLOAT_VLEN; i++) {
      if(starts_buf[i] != stops_buf[i] && isexp[i]) {
        auto c = charsp[starts_buf[i]];
        if(c == '+') {
          starts_buf[i]++;
        } else if (c == '-') {
          esign_buf[i] = -1;
          starts_buf[i]++;
        }
      }
    }
    anyvalid = true;
    while(anyvalid) {
      anyvalid = false;
#pragma _NEC ivdep
      for(size_t i = 0; i < PARSEFLOAT_VLEN; i++) {
        if(starts_buf[i] != stops_buf[i] && isexp[i]) {
          auto c = charsp[starts_buf[i]];
          if(c >= '0' && c <= '9') {
            e_val_buf[i] = e_val_buf[i] * 10 + c - '0';
            starts_buf[i]++;
            anyvalid = true;
          } 
        }
      }
    }
    for(size_t i = 0; i < PARSEFLOAT_VLEN; i++) {
      e_buf[i] += e_val_buf[i] * esign_buf[i];
    }
    if(sizeof(T) == 8) {
      for(size_t i = 0; i < PARSEFLOAT_VLEN; i++) {
        crnt_retp[i] = pow(10, e_buf[i]) * res_buf[i] * sign_buf[i];
      }
    } else {
      for(size_t i = 0; i < PARSEFLOAT_VLEN; i++) {
        crnt_retp[i] = powf(10, e_buf[i]) * res_buf[i] * sign_buf[i];
      }
    }
  }

  // rest; need to separate to use vreg
  size_t starts_buf2[PARSEFLOAT_VLEN];
//#pragma _NEC vreg(starts_buf2)
  size_t stops_buf2[PARSEFLOAT_VLEN];
//#pragma _NEC vreg(stops_buf2)
  T res_buf2[PARSEFLOAT_VLEN];
//#pragma _NEC vreg(res_buf2)
  T sign_buf2[PARSEFLOAT_VLEN];
//#pragma _NEC vreg(sign_buf2)
  T e_buf2[PARSEFLOAT_VLEN];
//#pragma _NEC vreg(e_buf2)
  int esign_buf2[PARSEFLOAT_VLEN];
//#pragma _NEC vreg(esign_buf2)
  int ispoint2[PARSEFLOAT_VLEN];
//#pragma _NEC vreg(ispoint2)
  int isexp2[PARSEFLOAT_VLEN];
//#pragma _NEC vreg(isexp2)
  T e_val_buf2[PARSEFLOAT_VLEN];
//#pragma _NEC vreg(e_val_buf2)

  // to surpress "may be used uninitialized" warning  
  for(int i = 0; i < PARSEFLOAT_VLEN; i++) {
    starts_buf2[i] = 0;
    stops_buf2[i] = 0;
    res_buf2[i] = 0;
    sign_buf2[i] = 0;
    e_buf2[i] = 0;
    esign_buf2[i] = 0;
    ispoint2[i] = 0;
    isexp2[i] = 0;
    e_val_buf2[i] = 0;
  }

  auto crnt_startsp = startsp + block * PARSEFLOAT_VLEN;
  auto crnt_lensp = lensp + block * PARSEFLOAT_VLEN;
  auto crnt_retp = retp + block * PARSEFLOAT_VLEN;

  for(size_t i = 0; i < rest; i++) {
    starts_buf2[i] = crnt_startsp[i];
    stops_buf2[i] = starts_buf2[i] + crnt_lensp[i];
    sign_buf2[i] = 1;
    res_buf2[i] = 0;
    e_buf2[i] = 0;
    esign_buf2[i] = 1;
    e_val_buf2[i] = 0;
  }
  for(size_t i = 0; i < rest; i++) {
    if(starts_buf2[i] != stops_buf2[i]) {
      auto c = charsp[starts_buf2[i]];
      if(c == '+') {
        starts_buf2[i]++;
      } else if (c == '-') {
        starts_buf2[i]++;
        sign_buf2[i] = -1;
      }
    }
  }
  anyvalid = true;
  while(anyvalid) {
    anyvalid = false;
#pragma _NEC ivdep
    for(size_t i = 0; i < rest; i++) {
      if(starts_buf2[i] != stops_buf2[i]) {
        auto c = charsp[starts_buf2[i]];
        if(c >= '0' && c <= '9') {
          res_buf2[i] = res_buf2[i] * 10 + c - '0';
          starts_buf2[i]++;
          anyvalid = true;
        }
      } 
    }
  }
#pragma _NEC ivdep
  for(size_t i = 0; i < rest; i++) {
    if(starts_buf2[i] != stops_buf2[i]) {
      auto c = charsp[starts_buf2[i]];
      if(c == '.') {
        ispoint2[i] = true;
        starts_buf2[i]++;
      } else {
        ispoint2[i] = false;
      }
    }
  }
  anyvalid = true;
  while(anyvalid) {
    anyvalid = false;
#pragma _NEC ivdep
    for(size_t i = 0; i < rest; i++) {
      if(starts_buf2[i] != stops_buf2[i] && ispoint2[i]) {
        auto c = charsp[starts_buf2[i]];
        if(c >= '0' && c <= '9') {
          res_buf2[i] = res_buf2[i] * 10 + c - '0';
          starts_buf2[i]++;
          e_buf2[i]--;
          anyvalid = true;
        }
      }
    }
  }
#pragma _NEC ivdep
  for(size_t i = 0; i < rest; i++) {
    if(starts_buf2[i] != stops_buf2[i]) {
      auto c = charsp[starts_buf2[i]];
      if(c == 'E' || c == 'e') {
        isexp2[i] = true;
        starts_buf2[i]++;
      } else {
        isexp2[i] = false;
      }
    }
  }
#pragma _NEC ivdep
  for(size_t i = 0; i < rest; i++) {
    if(starts_buf2[i] != stops_buf2[i] && isexp2[i]) {
      auto c = charsp[starts_buf2[i]];
      if(c == '+') {
        starts_buf2[i]++;
      } else if (c == '-') {
        esign_buf2[i] = -1;
        starts_buf2[i]++;
      }
    }
  }
  anyvalid = true;
  while(anyvalid) {
    anyvalid = false;
#pragma _NEC ivdep
    for(size_t i = 0; i < rest; i++) {
      if(starts_buf2[i] != stops_buf2[i] && isexp2[i]) {
        auto c = charsp[starts_buf2[i]];
        if(c >= '0' && c <= '9') {
          e_val_buf2[i] = e_val_buf2[i] * 10 + c - '0';
          starts_buf2[i]++;
          anyvalid = true;
        } 
      }
    }
  }
  for(size_t i = 0; i < rest; i++) {
    e_buf2[i] += e_val_buf2[i] * esign_buf2[i];
  }
  if(sizeof(T) == 8) {
    for(size_t i = 0; i < rest; i++) {
      crnt_retp[i] = pow(10, e_buf2[i]) * res_buf2[i] * sign_buf2[i];
    }
  } else {
    for(size_t i = 0; i < rest; i++) {
      crnt_retp[i] = powf(10, e_buf2[i]) * res_buf2[i] * sign_buf2[i];
    }
  }
}

struct is_inf_word {
  int operator()(int c) const {return (c == 'i' || c == 'I');}
};

struct is_minus_word {
  int operator()(int c) const {return (c == '-');}
};

struct is_nan_word {
  int operator()(int c) const {return (c == 'n' || c == 'N');}
};

template <class T>
std::vector<T> parsefloat(const std::vector<int>& chars,
                          const std::vector<size_t>& starts,
                          const std::vector<size_t>& lens) {
  if(std::numeric_limits<T>::is_integer)
    throw std::runtime_error("not float type");
  // w.trim(" "); // at this moment, commented out
  auto inf_idx = find_condition_index(chars, starts, is_inf_word());
  auto minus_idx = find_condition_index(chars, starts, is_minus_word());
  auto minus_idxp = minus_idx.data();
  auto minus_idx_size = minus_idx.size();
  std::vector<size_t> starts_tmp(minus_idx_size);
  auto starts_tmpp = starts_tmp.data();
  auto startsp = starts.data();
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
    auto num_words = starts.size();
    std::vector<T> ret(num_words);
    parsefloat(charsp, startsp, lensp, num_words, ret.data());
    return ret;
  } else {
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
    std::vector<T> rettmp(normal_fp_idx_size);
    parsefloat(charsp, normal_fp_startsp, normal_fp_lensp, normal_fp_idx_size,
               rettmp.data());
    auto num_words = starts.size();
    std::vector<T> ret(num_words);
    auto retp = ret.data();
#pragma _NEC ivdep
    for(size_t i = 0; i < normal_fp_idx_size; i++) {
      retp[normal_fp_idxp[i]] = rettmp[i];
    }
    auto inf_idxp = inf_idx.data();
    auto inf_idx_size = inf_idx.size();
#pragma _NEC ivdep
    for(size_t i = 0; i < inf_idx_size; i++) {
      retp[inf_idxp[i]] = std::numeric_limits<T>::infinity();
    }
    auto ninf_idxp = ninf_idx.data();
    auto ninf_idx_size = ninf_idx.size();
#pragma _NEC ivdep
    for(size_t i = 0; i < ninf_idx_size; i++) {
      retp[ninf_idxp[i]] = -std::numeric_limits<T>::infinity();
    }
    auto nan_idxp = nan_idx.data();
    auto nan_idx_size = nan_idx.size();
    T mynan = std::numeric_limits<T>::quiet_NaN();
#pragma _NEC ivdep
    for(size_t i = 0; i < nan_idx_size; i++) {
      retp[nan_idxp[i]] = mynan;
    }
    return ret;
  }
}

template <class T>
std::vector<T> parsefloat(const words& w) {
  return parsefloat<T>(w.chars, w.starts, w.lens);
}

template <class T> std::vector<T> parsenumber(const words& w);
template <> std::vector<float> parsenumber<float>(const words& w);
template <> std::vector<double> parsenumber<double>(const words& w);
template <> std::vector<int> parsenumber<int>(const words& w);
template <> std::vector<long> parsenumber<long>(const words& w);
template <> std::vector<long long> parsenumber<long long>(const words& w);
template <> std::vector<unsigned int> parsenumber<unsigned int>(const words& w);
template <> std::vector<unsigned long>
parsenumber<unsigned long>(const words& w);
template <> std::vector<unsigned long long>
parsenumber<unsigned long long>(const words& w);

template <class T>
std::vector<T> parsenumber(const std::vector<int>& chars,
                           const std::vector<size_t>& starts,
                           const std::vector<size_t>& lens);
template <>
std::vector<float> parsenumber<float>(const std::vector<int>& chars,
                                      const std::vector<size_t>& starts,
                                      const std::vector<size_t>& lens);
template <>
std::vector<double> parsenumber<double>(const std::vector<int>& chars,
                                        const std::vector<size_t>& starts,
                                        const std::vector<size_t>& lens);
template <>
std::vector<int> parsenumber<int>(const std::vector<int>& chars,
                                  const std::vector<size_t>& starts,
                                  const std::vector<size_t>& lens);
template <>
std::vector<long> parsenumber<long>(const std::vector<int>& chars,
                                    const std::vector<size_t>& starts,
                                    const std::vector<size_t>& lens);
template <>
std::vector<long long> parsenumber<long long>(const std::vector<int>& chars,
                                              const std::vector<size_t>& starts,
                                              const std::vector<size_t>& lens);
template <>
std::vector<unsigned int>
parsenumber<unsigned int>(const std::vector<int>& chars,
                          const std::vector<size_t>& starts,
                          const std::vector<size_t>& lens);
template <> std::vector<unsigned long>
parsenumber<unsigned long>(const std::vector<int>& chars,
                           const std::vector<size_t>& starts,
                           const std::vector<size_t>& lens);
template <> std::vector<unsigned long long>
parsenumber<unsigned long long>(const std::vector<int>& chars,
                                const std::vector<size_t>& starts,
                                const std::vector<size_t>& lens);

}
#endif
