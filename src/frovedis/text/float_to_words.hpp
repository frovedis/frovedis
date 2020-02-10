#ifndef FLOAT_TO_WORDS_HPP
#define FLOAT_TO_WORDS_HPP

#include "int_to_words.hpp"
#include "../core/set_operations.hpp"
#include "char_int_conv.hpp"
#include "find_condition.hpp"
#include <math.h>
#include <limits>

namespace frovedis {

template <class T>
words float_to_words_normal(const T* srcp, size_t src_size,
                            const size_t* num_of_decvp) {
  words ret;
  if(src_size == 0) return ret;

  auto digits = get_digits_10<T>();
  auto digits_size = digits.size();
  auto digitsp = digits.data();

  std::vector<int> is_minus(src_size);
  auto is_minusp = is_minus.data();
  std::vector<T> new_src(src_size);
  auto new_srcp = new_src.data();
  for(size_t i = 0; i < src_size; i++) {
    if(srcp[i] < 0) {
      is_minusp[i] = true;
      new_srcp[i] = -srcp[i];
    } else {
      is_minusp[i] = false;
      new_srcp[i] = srcp[i];
    }
  }

  ret.lens.resize(src_size);
  auto lensp = ret.lens.data();
  upper_bound(digitsp, digits_size, new_srcp, src_size, lensp);
  for(size_t i = 0; i < src_size; i++) {
    if(lensp[i] == 0) lensp[i] = 1; // for 0
  }
  for(size_t i = 0; i < src_size; i++) {
    if(is_minusp[i]) lensp[i] += (2 + num_of_decvp[i]); // '-', '.'
    else lensp[i] += 1 + num_of_decvp[i]; // '.'
  }

  size_t total = 0;
  for(size_t i = 0; i < src_size; i++) {
    total += lensp[i];
  }
  ret.chars.resize(total);
  ret.starts.resize(src_size);
  auto startsp = ret.starts.data();
  prefix_sum(lensp, startsp+1, src_size-1);
  auto charsp = ret.chars.data();
  std::vector<size_t> new_starts(src_size);
  auto new_startsp = new_starts.data();
  std::vector<size_t> new_lens(src_size);
  auto new_lensp = new_lens.data();
  for(size_t i = 0; i < src_size; i++) {
    if(is_minusp[i]) {
      charsp[startsp[i]] = '-';
      new_startsp[i] = startsp[i] + 1;
      new_lensp[i] = lensp[i] - (num_of_decvp[i] + 2);
    } else {
      new_startsp[i] = startsp[i];
      new_lensp[i] = lensp[i] - (num_of_decvp[i] + 1);
    }
  }
  int_to_words_write_digits(charsp, new_startsp, new_srcp, new_lensp,
                            src_size);

  for(size_t i = 0; i < src_size; i++) {
    new_srcp[i] *= digitsp[num_of_decvp[i]];
  }

  std::vector<size_t> after_dec_starts(src_size);
  auto after_dec_startsp = after_dec_starts.data();
  for(size_t i = 0; i < src_size-1; i++) {
    after_dec_startsp[i] = startsp[i+1] - (num_of_decvp[i] + 1);
  }
  after_dec_startsp[src_size-1] = total - (num_of_decvp[src_size-1] + 1);
  for(size_t i = 0; i < src_size; i++) {
    charsp[after_dec_startsp[i]] = '.';
    after_dec_startsp[i]++;
  }
  std::vector<size_t> after_dec_lens(src_size);
  auto after_dec_lensp = after_dec_lens.data();
  for(size_t i = 0; i < src_size; i++) {
    after_dec_lensp[i] = num_of_decvp[i];
  }
  int_to_words_write_digits(charsp, after_dec_startsp, new_srcp,
                            after_dec_lensp, src_size);
  return ret;
}

template <class T>
struct is_nan {
  int operator()(T a) const {return isnan(a);}
};
template <>
struct is_nan<double> {
  is_nan() {
    double n = std::numeric_limits<double>::quiet_NaN();
    mynan = *reinterpret_cast<uint64_t*>(&n);
  }
  int operator()(double a) const {
    return (mynan == *reinterpret_cast<uint64_t*>(&a));
  }
  uint64_t mynan;
};
template <>
struct is_nan<float> {
  is_nan() {
    float n = std::numeric_limits<float>::quiet_NaN();
    mynan = *reinterpret_cast<uint32_t*>(&n);
  }
  int operator()(float a) const {
    return (mynan == *reinterpret_cast<uint32_t*>(&a));
  }
  uint32_t mynan;
};

template <class T>
struct is_inf {
  int operator()(T a) const {return (a == std::numeric_limits<T>::infinity());}
};

template <class T>
struct is_neg_inf {
  int operator()(T a) const {return (a == -std::numeric_limits<T>::infinity());}
};

template <class T>
words float_to_words_normal(const std::vector<T>& src,
                            const std::vector<size_t> num_of_decv) {
  if(src.size() == 0) return words();
  auto nans = find_condition(src, is_nan<T>());
  auto infs = find_condition(src, is_inf<T>());
  auto ninfs = find_condition(src, is_neg_inf<T>());
  if(nans.size() == 0 && infs.size() == 0 && ninfs.size() == 0) {
    return float_to_words_normal(src.data(), src.size(), num_of_decv.data());
  } else {
    auto src_size = src.size();
    std::vector<size_t> iota(src_size);
    auto iotap = iota.data();
    for(size_t i = 0; i < src_size; i++) iotap[i] = i;
    auto nans_infs = set_merge(nans, infs);
    auto nans_infs_ninfs = set_merge(nans_infs, ninfs);
    auto normal_fp = set_difference(iota, nans_infs_ninfs);
    auto normal_fp_size = normal_fp.size();
    auto normal_fpp = normal_fp.data();
    std::vector<T> new_src(normal_fp_size);
    auto new_srcp = new_src.data();
    auto srcp = src.data();
    for(size_t i = 0; i < normal_fp_size; i++) {
      new_srcp[i] = srcp[normal_fpp[i]];
    }
    auto normal_fp_words =
      float_to_words_normal(new_srcp, normal_fp_size, num_of_decv.data());

    std::string s("nan inf -inf");
    auto w = split_to_words(char_to_int(s), " ");
    words ret;
    auto normal_fp_words_chars_size = normal_fp_words.chars.size();
    auto w_chars_size = w.chars.size();
    ret.chars.resize(normal_fp_words_chars_size + w_chars_size);
    auto ret_charsp = ret.chars.data();
    auto normal_fp_words_charsp = normal_fp_words.chars.data();
    for(size_t i = 0; i < normal_fp_words_chars_size; i++) {
      ret_charsp[i] = normal_fp_words_charsp[i];
    }
    auto crnt_ret_charsp = ret_charsp + normal_fp_words_chars_size;
    auto w_charsp = w.chars.data();
    for(size_t i = 0; i < w_chars_size; i++) {
      crnt_ret_charsp[i] = w_charsp[i];
    }
    ret.starts.resize(src_size);
    ret.lens.resize(src_size);
    auto ret_startsp = ret.starts.data();
    auto ret_lensp = ret.lens.data();
    auto normal_fp_words_startsp = normal_fp_words.starts.data();
    auto normal_fp_words_lensp = normal_fp_words.lens.data();
#pragma _NEC ivdep
    for(size_t i = 0; i < normal_fp_size; i++) {
      ret_startsp[normal_fpp[i]] = normal_fp_words_startsp[i];
      ret_lensp[normal_fpp[i]] = normal_fp_words_lensp[i];
    }
    auto nansp = nans.data();
    auto nans_size = nans.size();
    auto nan_start = w.starts[0] + normal_fp_words_chars_size;
    auto nan_len = w.lens[0];
#pragma _NEC ivdep
    for(size_t i = 0; i < nans_size; i++) {
      ret_startsp[nansp[i]] = nan_start;
      ret_lensp[nansp[i]] = nan_len;
    }
    auto infsp = infs.data();
    auto infs_size = infs.size();
    auto inf_start = w.starts[1] + normal_fp_words_chars_size;
    auto inf_len = w.lens[1];
#pragma _NEC ivdep
    for(size_t i = 0; i < infs_size; i++) {
      ret_startsp[infsp[i]] = inf_start;
      ret_lensp[infsp[i]] = inf_len;
    }
    auto ninfsp = ninfs.data();
    auto ninfs_size = ninfs.size();
    auto ninf_start = w.starts[2] + normal_fp_words_chars_size;
    auto ninf_len = w.lens[2];
#pragma _NEC ivdep
    for(size_t i = 0; i < ninfs_size; i++) {
      ret_startsp[ninfsp[i]] = ninf_start;
      ret_lensp[ninfsp[i]] = ninf_len;
    }
    return ret;
  }
}

template <class T>
words float_to_words_normal(const std::vector<T>& src, size_t num_of_dec = 6) {
  auto src_size = src.size();
  if(src_size == 0) return words();
  std::vector<size_t> num_of_decv(src_size);
  auto num_of_decvp = num_of_decv.data();
  for(size_t i = 0; i < src_size; i++) {
    num_of_decvp[i] = num_of_dec;
  }
  return float_to_words_normal(src, num_of_decv);
}

template <class T>
std::vector<T> get_digits_fp(int& to_sub);
// https://www.cc.kyoto-su.ac.jp/~yamada/programming/float.html
// 1.175494 10-38 < abs float < 3.402823 10+38
// 2.225074 10-308 < abs double < 1.797693 10+308
template <>
std::vector<double> get_digits_fp<double>(int& to_sub) {
  to_sub = 309;
  std::vector<double> digits(617);
  auto digitsp = digits.data();
  for(size_t i = 1; i < 617; i++) {
    digitsp[i] = pow(10.0, static_cast<double>(i)-308); 
  }
  return digits;
}
template <>
std::vector<float> get_digits_fp<float>(int& to_sub) {
  to_sub = 39;
  std::vector<float> digits(77);
  auto digitsp = digits.data();
  for(size_t i = 1; i < 77; i++) {
    digitsp[i] = powf(10.0, static_cast<float>(i)-38);
  }
  return digits;
}
template <class T>
std::vector<T> get_mult_fp(size_t& pad);
template <>
std::vector<double> get_mult_fp<double>(size_t& pad) {
  pad = 128 / sizeof(double);
  std::vector<double> mult(617 * pad);
  auto multp = mult.data();
  for(size_t i = 0; i < 616; i++) {
    multp[i * pad] = pow(10.0, 308-static_cast<double>(i));
  }
  multp[616 * pad] = pow(10.0, -307.0); // 1.0E-308 is out of range
  return mult;
}
template <>
std::vector<float> get_mult_fp<float>(size_t& pad) {
  pad = 128 / sizeof(float);
  std::vector<float> mult(77 * pad);
  auto multp = mult.data();
  for(size_t i = 0; i < 76; i++) {
    multp[i * pad] = pow(10.0, 38-static_cast<double>(i));
  }
  multp[76 * pad] = pow(10.0, -37.0); // 1.0E-38 is out of range
  return mult;
}

template <class T>
words float_to_words_exp(const T* srcp, size_t src_size, size_t num_of_dec) {
  words ret;
  if(src_size == 0) return ret;

  int to_sub;
  auto digits = get_digits_fp<T>(to_sub);
  auto digits_size = digits.size();
  auto digitsp = digits.data();

  std::vector<T> new_src(src_size);
  std::vector<int> is_minus(src_size);
  auto is_minusp = is_minus.data();
  auto new_srcp = new_src.data();
  for(size_t i = 0; i < src_size; i++) {
    if(srcp[i] < 0) {
      is_minusp[i] = true;
      new_srcp[i] = -srcp[i];
    } else {
      is_minusp[i] = false;
      new_srcp[i] = srcp[i];
    }
  }
  std::vector<size_t> exp_idx(src_size);
  auto exp_idxp = exp_idx.data();
  upper_bound(digitsp, digits_size, new_srcp, src_size, exp_idxp);
  std::vector<int> exps(src_size);
  auto expsp = exps.data();
  for(size_t i = 0; i < src_size; i++) {
    if(new_srcp[i] != 0.0) {
      expsp[i] = static_cast<int>(exp_idxp[i]) - to_sub;
    } else {
      expsp[i] = 0;
    }
  }

  size_t pad;
  auto to_mul = get_mult_fp<T>(pad);
  auto to_mulp = to_mul.data();
  for(size_t i = 0; i < src_size; i++) {
    if(new_srcp[i] != 0.0) {
      new_srcp[i] *= to_mulp[(exp_idxp[i] - 1) * pad];
      // 1.0 * E-38 or 1.0 * E-308 is out of range
      if(exp_idxp[i] == digits_size) { 
        new_srcp[i] *= 0.1;
      }
    }
  }

  ret.lens.resize(src_size);
  auto lensp = ret.lens.data();
  size_t each_len;
  if(sizeof(T) == 4) each_len = num_of_dec + 6; // x.yyyye+/-zz
  else each_len = num_of_dec + 7; // x.yyyye+/-zzz
  
  for(size_t i = 0; i < src_size; i++) {
    if(is_minusp[i]) {
      lensp[i] = each_len + 1;
    } else {
      lensp[i] = each_len;
    }
  }
  size_t total = 0;
  for(size_t i = 0; i < src_size; i++) {
    total += lensp[i];
  }
  ret.chars.resize(total);
  ret.starts.resize(src_size);
  auto startsp = ret.starts.data();
  prefix_sum(lensp, startsp+1, src_size-1);
  auto charsp = ret.chars.data();
  std::vector<size_t> new_starts(src_size);
  auto new_startsp = new_starts.data();
  for(size_t i = 0; i < src_size; i++) {
    if(is_minusp[i]) {
      charsp[startsp[i]] = '-';
      new_startsp[i] = startsp[i] + 1;
    } else {
      new_startsp[i] = startsp[i];
    }
  }
  T to_mul2 = pow(10, num_of_dec);
  std::vector<int> first_digit(src_size);
  auto first_digitp = first_digit.data();
  for(size_t i = 0; i < src_size; i++) {
    auto tmp = static_cast<int>(new_srcp[i]);
    // fail safe...
    if(tmp >= 10) {
      new_srcp[i] *= 0.1;
      first_digitp[i] = static_cast<int>(new_srcp[i]);
      expsp[i]++;
      new_srcp[i] -= static_cast<double>(first_digitp[i]);
      new_srcp[i] *= to_mul2;
    } else if(tmp == 0 && new_srcp[i] != 0.0) {
      new_srcp[i] *= 10.0;
      first_digitp[i] = static_cast<int>(new_srcp[i]);
      exps[i]--;
      new_srcp[i] -= static_cast<double>(first_digitp[i]);
      new_srcp[i] *= to_mul2;
    } else {
      first_digitp[i] = tmp;
      new_srcp[i] -= static_cast<double>(tmp);
      new_srcp[i] *= to_mul2;
    }
  }
#pragma _NEC ivdep
  for(size_t i = 0; i < src_size; i++) {
    charsp[new_startsp[i]] = first_digitp[i] + '0';
    charsp[new_startsp[i]+1] = '.';
    new_startsp[i] += 2;
  }
  std::vector<size_t> after_dec_lens(src_size);
  auto after_dec_lensp = after_dec_lens.data();
  for(size_t i = 0; i < src_size; i++) {
    after_dec_lensp[i] = num_of_dec;
  }
  int_to_words_write_digits(charsp, new_startsp, new_srcp,
                            after_dec_lensp, src_size);
#pragma _NEC ivdep
  for(size_t i = 0; i < src_size; i++) {
    charsp[new_startsp[i] + num_of_dec] = 'e';
    if(expsp[i] < 0) {
      charsp[new_startsp[i] + num_of_dec + 1] = '-';
      expsp[i] = -expsp[i];
    }
    else {
      charsp[new_startsp[i] + num_of_dec + 1] = '+';
    }
    new_startsp[i] += (num_of_dec + 2);
  }
  std::vector<size_t> elen(src_size);
  auto elenp = elen.data();
  auto digits10 = get_digits_10<int>();
  auto digits10_size = digits10.size();
  auto digits10p = digits10.data();
  upper_bound(digits10p, digits10_size, expsp, src_size, elenp);
  for(size_t i = 0; i < src_size; i++) {
    if(elenp[i] < 2) elenp[i] = 2; // if data is 0, elenp might become 0
  }
  int_to_words_write_digits(charsp, new_startsp, expsp, elenp, src_size);
  return ret;
}

template <class T>
words float_to_words_exp(const std::vector<T>& src, size_t num_of_dec = 6) {
  if(src.size() == 0) return words();
  auto nans = find_condition(src, is_nan<T>());
  auto infs = find_condition(src, is_inf<T>());
  auto ninfs = find_condition(src, is_neg_inf<T>());
  if(nans.size() == 0 && infs.size() == 0 && ninfs.size() == 0) {
    return float_to_words_exp(src.data(), src.size(), num_of_dec);
  } else {
    auto src_size = src.size();
    std::vector<size_t> iota(src_size);
    auto iotap = iota.data();
    for(size_t i = 0; i < src_size; i++) iotap[i] = i;
    auto nans_infs = set_merge(nans, infs);
    auto nans_infs_ninfs = set_merge(nans_infs, ninfs);
    auto normal_fp = set_difference(iota, nans_infs_ninfs);
    auto normal_fp_size = normal_fp.size();
    auto normal_fpp = normal_fp.data();
    std::vector<T> new_src(normal_fp_size);
    auto new_srcp = new_src.data();
    auto srcp = src.data();
    for(size_t i = 0; i < normal_fp_size; i++) {
      new_srcp[i] = srcp[normal_fpp[i]];
    }
    auto normal_fp_words =
      float_to_words_exp(new_srcp, normal_fp_size, num_of_dec);

    std::string s("nan inf -inf");
    auto w = split_to_words(char_to_int(s), " ");
    words ret;
    auto normal_fp_words_chars_size = normal_fp_words.chars.size();
    auto w_chars_size = w.chars.size();
    ret.chars.resize(normal_fp_words_chars_size + w_chars_size);
    auto ret_charsp = ret.chars.data();
    auto normal_fp_words_charsp = normal_fp_words.chars.data();
    for(size_t i = 0; i < normal_fp_words_chars_size; i++) {
      ret_charsp[i] = normal_fp_words_charsp[i];
    }
    auto crnt_ret_charsp = ret_charsp + normal_fp_words_chars_size;
    auto w_charsp = w.chars.data();
    for(size_t i = 0; i < w_chars_size; i++) {
      crnt_ret_charsp[i] = w_charsp[i];
    }
    ret.starts.resize(src_size);
    ret.lens.resize(src_size);
    auto ret_startsp = ret.starts.data();
    auto ret_lensp = ret.lens.data();
    auto normal_fp_words_startsp = normal_fp_words.starts.data();
    auto normal_fp_words_lensp = normal_fp_words.lens.data();
#pragma _NEC ivdep
    for(size_t i = 0; i < normal_fp_size; i++) {
      ret_startsp[normal_fpp[i]] = normal_fp_words_startsp[i];
      ret_lensp[normal_fpp[i]] = normal_fp_words_lensp[i];
    }
    auto nansp = nans.data();
    auto nans_size = nans.size();
    auto nan_start = w.starts[0] + normal_fp_words_chars_size;
    auto nan_len = w.lens[0];
#pragma _NEC ivdep
    for(size_t i = 0; i < nans_size; i++) {
      ret_startsp[nansp[i]] = nan_start;
      ret_lensp[nansp[i]] = nan_len;
    }
    auto infsp = infs.data();
    auto infs_size = infs.size();
    auto inf_start = w.starts[1] + normal_fp_words_chars_size;
    auto inf_len = w.lens[1];
#pragma _NEC ivdep
    for(size_t i = 0; i < infs_size; i++) {
      ret_startsp[infsp[i]] = inf_start;
      ret_lensp[infsp[i]] = inf_len;
    }
    auto ninfsp = ninfs.data();
    auto ninfs_size = ninfs.size();
    auto ninf_start = w.starts[2] + normal_fp_words_chars_size;
    auto ninf_len = w.lens[2];
#pragma _NEC ivdep
    for(size_t i = 0; i < ninfs_size; i++) {
      ret_startsp[ninfsp[i]] = ninf_start;
      ret_lensp[ninfsp[i]] = ninf_len;
    }
    return ret;
  }
}

struct to_use_exp {
  to_use_exp(int prec) : prec(prec) {}
  int operator()(int e) const {return (e < -4 || e >= prec);}
  int prec;
};

template <class T>
words float_to_words(const std::vector<T>& src, size_t prec = 6) {
  auto src_size = src.size();
  if(src_size == 0) return words();
  auto srcp = src.data();

  auto nans = find_condition(src, is_nan<T>());
  auto nansp = nans.data();
  auto nans_size = nans.size();
  
  int to_sub;
  auto digits = get_digits_fp<T>(to_sub);
  auto digits_size = digits.size();
  auto digitsp = digits.data();

  std::vector<T> new_src(src_size);
  auto new_srcp = new_src.data();
  for(size_t i = 0; i < src_size; i++) {
    new_srcp[i] = srcp[i];
  }
  for(size_t i = 0; i < nans_size; i++) {
    // reluctant to process nan... let exp to process nan and inf
    new_srcp[nansp[i]] = std::numeric_limits<T>::infinity();
  }
  for(size_t i = 0; i < src_size; i++) {
    if(srcp[i] < 0) {
      new_srcp[i] = -new_srcp[i];
    }
  }
  std::vector<size_t> exp_idx(src_size);
  auto exp_idxp = exp_idx.data();
  upper_bound(digitsp, digits_size, new_srcp, src_size, exp_idxp);
  std::vector<int> exps(src_size);
  auto expsp = exps.data();
  for(size_t i = 0; i < src_size; i++) {
    if(new_srcp[i] != 0.0) {
      expsp[i] = static_cast<int>(exp_idxp[i]) - to_sub;
    } else {
      expsp[i] = 0;
    }
  }
  auto exp_fp = find_condition(exps, to_use_exp(prec));
  std::vector<size_t> iota(src_size);
  auto iotap = iota.data();
  for(size_t i = 0; i < src_size; i++) iotap[i] = i;
  auto normal_fp = set_difference(iota, exp_fp);
  auto exp_fp_size = exp_fp.size();
  auto normal_fp_size = normal_fp.size();
  std::vector<T> exp_src(exp_fp_size);
  std::vector<T> normal_src(normal_fp_size);
  auto exp_srcp = exp_src.data();
  auto normal_srcp = normal_src.data();
  auto exp_fpp = exp_fp.data();
  auto normal_fpp = normal_fp.data();
  for(size_t i = 0; i < exp_fp_size; i++) {
    exp_srcp[i] = srcp[exp_fpp[i]];
  }
  auto expw = float_to_words_exp(exp_src, prec-1);
  for(size_t i = 0; i < normal_fp_size; i++) {
    normal_srcp[i] = srcp[normal_fpp[i]];
  }
  std::vector<size_t> normal_num_of_decv(normal_fp_size);
  auto normal_num_of_decvp = normal_num_of_decv.data();
  for(size_t i = 0; i < normal_fp_size; i++) {
    if(expsp[i] < 0) {
      normal_num_of_decvp[i] = prec + (-expsp[normal_fpp[i]]) - 1;
    } else {
      normal_num_of_decvp[i] = prec - expsp[normal_fpp[i]] - 1;
    }
  }
  auto normalw = float_to_words_normal(normal_src, normal_num_of_decv);
  auto ret = merge_words(expw, normalw);
  std::vector<size_t> work_starts(src_size);
  std::vector<size_t> work_lens(src_size);
  ret.starts.swap(work_starts);
  ret.lens.swap(work_lens);
  auto work_startsp = work_starts.data();
  auto work_lensp = work_lens.data();
  auto ret_startsp = ret.starts.data();
  auto ret_lensp = ret.lens.data();
  for(size_t i = 0; i < exp_fp_size; i++) {
    ret_startsp[exp_fpp[i]] = work_startsp[i];
    ret_lensp[exp_fpp[i]] = work_lensp[i];
  }
  auto crnt_startsp = work_startsp + exp_fp_size;
  auto crnt_lensp = work_lensp + exp_fp_size;
  for(size_t i = 0; i < normal_fp_size; i++) {
    ret_startsp[normal_fpp[i]] = crnt_startsp[i];
    ret_lensp[normal_fpp[i]] = crnt_lensp[i];
  }
  ret.trim_tail("0");
  ret.trim_tail("."); // should be separated for like 100.0
  return ret;
}

}
#endif
