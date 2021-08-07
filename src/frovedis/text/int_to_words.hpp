#ifndef INT_TO_WORDS_HPP
#define INT_TO_WORDS_HPP

#include "words.hpp"
#include "../core/upper_bound.hpp"
#include "../core/prefix_sum.hpp"
#include <vector>
#include <stdexcept>
#include <limits>

#if defined(__ve__) || defined(_SX)
#define INT_TO_WORDS_VLEN 256
#else
//#define INT_TO_WORDS_VLEN 1
#define INT_TO_WORDS_VLEN 4
#endif

namespace frovedis {

template <class T> std::vector<T> get_digits_10();
template <> std::vector<int> get_digits_10<int>();
template <> std::vector<unsigned int> get_digits_10<unsigned int>();
template <> std::vector<long> get_digits_10<long>();
template <> std::vector<unsigned long> get_digits_10<unsigned long>();
template <> std::vector<long long> get_digits_10<long long>();
template <> std::vector<unsigned long long> get_digits_10<unsigned long long>();
template <> std::vector<double> get_digits_10<double>();
template <> std::vector<float> get_digits_10<float>();

// T should be positive
template <class T>
void int_to_words_write_digits(int* charsp, const size_t* startsp,
                               T* srcp, const size_t* lensp,
                               size_t src_size) {
  auto digits = get_digits_10<T>();
  auto digits_size = digits.size();
  // to avoid using the same cache line for gather load
  auto tbl_pad = 128/(sizeof(T));
  auto digits_tbl_size = digits_size * tbl_pad;
  std::vector<T> digits_tbl(digits_tbl_size);
  auto digitsp = digits.data();
  auto digits_tblp = digits_tbl.data();
  for(size_t i = 0; i < digits_size; i++) {
    digits_tblp[i * tbl_pad] = digitsp[i];
  }
  auto block = src_size / INT_TO_WORDS_VLEN;
  auto rest = src_size - block * INT_TO_WORDS_VLEN;
  size_t crnt_out_vreg[INT_TO_WORDS_VLEN];
#pragma _NEC vreg(crnt_out_vreg)
  size_t crnt_lens_vreg[INT_TO_WORDS_VLEN];
#pragma _NEC vreg(crnt_lens_vreg)
  T crnt_src_vreg[INT_TO_WORDS_VLEN];
#pragma _NEC vreg(crnt_src_vreg)
  for(size_t b = 0; b < block; b++) {
    auto offset = b * INT_TO_WORDS_VLEN;
    auto crnt_startsp = startsp + offset;
    auto crnt_lensp = lensp + offset;
    auto crnt_srcp = srcp + offset;
    for(size_t i = 0; i < INT_TO_WORDS_VLEN; i++) {
      crnt_out_vreg[i] = crnt_startsp[i];
      crnt_lens_vreg[i] = crnt_lensp[i];
      crnt_src_vreg[i] = crnt_srcp[i];
    }
    size_t max = 0;
    for(size_t i = 0; i < INT_TO_WORDS_VLEN; i++) {
      if(max < crnt_lensp[i]) max = crnt_lensp[i];
    }
#pragma _NEC vob
    for(size_t m = 0; m < max; m++) {
#pragma _NEC vovertake
#pragma _NEC ivdep
      for(size_t i = 0; i < INT_TO_WORDS_VLEN; i++) {
        if(crnt_lens_vreg[i] > 0) {
          auto todiv = digits_tblp[(crnt_lens_vreg[i] - 1) * tbl_pad];
          int crnt_digit = crnt_src_vreg[i] / todiv;
          charsp[crnt_out_vreg[i]++] =  crnt_digit + '0';
          crnt_lens_vreg[i]--;
          crnt_src_vreg[i] -= crnt_digit * todiv;
        }
      }
    }
    for(size_t i = 0; i < INT_TO_WORDS_VLEN; i++) {
      crnt_srcp[i] = crnt_src_vreg[i]; // write back for float_to_words
    }
  }

  size_t crnt_out_vreg2[INT_TO_WORDS_VLEN];
//#pragma _NEC vreg(crnt_out_vreg2)
  size_t crnt_lens_vreg2[INT_TO_WORDS_VLEN];
//#pragma _NEC vreg(crnt_lens_vreg2)
  T crnt_src_vreg2[INT_TO_WORDS_VLEN];
//#pragma _NEC vreg(crnt_src_vreg2)
  auto offset = block * INT_TO_WORDS_VLEN;
  auto crnt_startsp = startsp + offset;
  auto crnt_lensp = lensp + offset;
  auto crnt_srcp = srcp + offset;
  for(size_t i = 0; i < rest; i++) {
    crnt_out_vreg2[i] = crnt_startsp[i];
    crnt_lens_vreg2[i] = crnt_lensp[i];
    crnt_src_vreg2[i] = crnt_srcp[i];
  }
  size_t max = 0;
  for(size_t i = 0; i < rest; i++) {
    if(max < crnt_lensp[i]) max = crnt_lensp[i];
  }
  for(size_t m = 0; m < max; m++) {
#pragma _NEC ivdep
    for(size_t i = 0; i < rest; i++) {
      if(crnt_lens_vreg2[i] > 0) {
        auto todiv = digits_tblp[(crnt_lens_vreg2[i] - 1) * tbl_pad];
        int crnt_digit = crnt_src_vreg2[i] / todiv;
        charsp[crnt_out_vreg2[i]++] =  crnt_digit + '0';
        crnt_lens_vreg2[i]--;
        crnt_src_vreg2[i] -= crnt_digit * todiv;
      }
    }
  }
  for(size_t i = 0; i < rest; i++) {
    crnt_srcp[i] = crnt_src_vreg2[i]; // write back for float_to_words
  }
}

template <class T, class U>
words int_to_words_signed_helper(const T* srcp, size_t src_size) {
  if(src_size == 0) return words();
  words ret;
  auto digits = get_digits_10<U>();
  auto digits_size = digits.size();
  auto digitsp = digits.data();
  ret.lens.resize(src_size);
  auto lensp = ret.lens.data();
  std::vector<U> new_src(src_size);
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
  upper_bound(digitsp, digits_size, new_srcp, src_size, lensp);
  for(size_t i = 0; i < src_size; i++) {
    if(lensp[i] == 0) lensp[i] = 1; // for 0
  }
  for(size_t i = 0; i < src_size; i++) {
    if(is_minusp[i]) lensp[i]++;
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
      new_lensp[i] = lensp[i] - 1;
    } else {
      new_startsp[i] = startsp[i];
      new_lensp[i] = lensp[i];
    }
  }
  int_to_words_write_digits(charsp, new_startsp, new_srcp, new_lensp, src_size);
  return ret;
}

template <class T>
words int_to_words_unsigned_helper(const T* srcp, size_t src_size) {
  if(src_size == 0) return words();
  words ret;
  auto digits = get_digits_10<T>();
  auto digits_size = digits.size();
  auto digitsp = digits.data();
  ret.lens.resize(src_size);
  auto lensp = ret.lens.data();
  upper_bound(digitsp, digits_size, srcp, src_size, lensp);
  for(size_t i = 0; i < src_size; i++) {
    if(lensp[i] == 0) lensp[i] = 1; // for 0
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
  std::vector<T> new_src(src_size);
  auto new_srcp = new_src.data();
  for(size_t i = 0; i < src_size; i++) new_srcp[i] = srcp[i];
  int_to_words_write_digits(charsp, startsp, new_srcp, lensp, src_size);
  return ret;
}

template <class T>
struct int_to_words_signed_struct {
  words operator()(const T* srcp, size_t src_size) {
    if(sizeof(T) == 4) {
      return int_to_words_signed_helper<T, uint32_t>(srcp, src_size);
    } else {
      return int_to_words_signed_helper<T, uint64_t>(srcp, src_size);
    }
  }
};

template <class T>
struct int_to_words_unsigned_struct {
  words operator()(const T* srcp, size_t src_size) {
    return int_to_words_unsigned_helper<T>(srcp, src_size);
  }
};

template <class T>
words int_to_words(const T* srcp, size_t src_size) {
  typename std::conditional
    <std::numeric_limits<T>::is_signed,
     int_to_words_signed_struct<T>,
     int_to_words_unsigned_struct<T>>::type int_to_words_func;
  return int_to_words_func(srcp, src_size);
}

template <class T>
words int_to_words(const std::vector<T>& src) {
  return int_to_words(src.data(), src.size());
}

}
#endif
