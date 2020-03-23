#include "float_to_words.hpp"

namespace frovedis {

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

template <>
words number_to_words<float>(const std::vector<float>& src, size_t prec) {
  return float_to_words<float>(src, prec);
}
template <>
words number_to_words<double>(const std::vector<double>& src, size_t prec) {
  return float_to_words<double>(src, prec);
}
template <>
words number_to_words<int>(const std::vector<int>& src, size_t prec) {
  return int_to_words<int>(src);
}
template <>
words number_to_words<long>(const std::vector<long>& src, size_t prec) {
  return int_to_words<long>(src);
}
template <>
words number_to_words<long long> (const std::vector<long long>& src,
                                  size_t prec) {
  return int_to_words<long long>(src);
}
template <>
words number_to_words<unsigned int>(const std::vector<unsigned int>& src,
                                    size_t prec) {
  return int_to_words<unsigned int>(src);
}
template <>
words number_to_words<unsigned long>(const std::vector<unsigned long>& src,
                                     size_t prec) {
  return int_to_words<unsigned long>(src);
}
template <>
words number_to_words<unsigned long long>
(const std::vector<unsigned long long>& src, size_t prec) {
  return int_to_words<unsigned long long>(src);
}

}
