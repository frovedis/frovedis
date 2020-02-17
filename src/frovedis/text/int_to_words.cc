#include "int_to_words.hpp"

namespace frovedis {

template <>
std::vector<int> get_digits_10<int>() {
  std::vector<int> digits =
    {1,
     10,
     100,
     1000,
     10000,
     100000,
     1000000,
     10000000,
     100000000,
     1000000000};
  return digits;
}
template <>
std::vector<unsigned int> get_digits_10<unsigned int>() {
  std::vector<unsigned int> digits =
    {1,
     10,
     100,
     1000,
     10000,
     100000,
     1000000,
     10000000,
     100000000,
     1000000000};
  return digits;
}
template <>
std::vector<long> get_digits_10<long>() {
  std::vector<long> digits =
    {1,
     10,
     100,
     1000,
     10000,
     100000,
     1000000,
     10000000,
     100000000,
     1000000000,
     10000000000,
     100000000000,
     1000000000000,
     10000000000000,
     100000000000000,
     1000000000000000,
     10000000000000000,
     100000000000000000,
     1000000000000000000}; // 19
  return digits;
}
template <>
std::vector<unsigned long> get_digits_10<unsigned long>() {
  std::vector<unsigned long> digits =
    {1,
     10,
     100,
     1000,
     10000,
     100000,
     1000000,
     10000000,
     100000000,
     1000000000,
     10000000000,
     100000000000,
     1000000000000,
     10000000000000,
     100000000000000,
     1000000000000000,
     10000000000000000,
     100000000000000000,
     1000000000000000000,
     10000000000000000000U}; // 20
  return digits;
}
template <>
std::vector<long long> get_digits_10<long long>() {
  std::vector<long long> digits =
    {1,
     10,
     100,
     1000,
     10000,
     100000,
     1000000,
     10000000,
     100000000,
     1000000000,
     10000000000,
     100000000000,
     1000000000000,
     10000000000000,
     100000000000000,
     1000000000000000,
     10000000000000000,
     100000000000000000,
     1000000000000000000}; // 19
  return digits;
}
template <>
std::vector<unsigned long long> get_digits_10<unsigned long long>() {
  std::vector<unsigned long long> digits =
    {1,
     10,
     100,
     1000,
     10000,
     100000,
     1000000,
     10000000,
     100000000,
     1000000000,
     10000000000,
     100000000000,
     1000000000000,
     10000000000000,
     100000000000000,
     1000000000000000,
     10000000000000000,
     100000000000000000,
     1000000000000000000,
     10000000000000000000U}; // 20
  return digits;
}
// used in float_to_words.hpp
// https://www.cc.kyoto-su.ac.jp/~yamada/programming/float.html
// 1.175494 10-38 < abs float < 3.402823 10+38
// 2.225074 10-308 < abs double < 1.797693 10+308
template <>
std::vector<double> get_digits_10<double>() {
  std::vector<double> digits(309);
  auto digitsp = digits.data();
  for(size_t i = 0; i < 309; i++) {
    digitsp[i] = pow(10.0, static_cast<double>(i)); 
  }
  return digits;
}
template <>
std::vector<float> get_digits_10<float>() {
  std::vector<float> digits(39);
  auto digitsp = digits.data();
  for(size_t i = 0; i < 39; i++) {
    digitsp[i] = powf(10.0f, static_cast<float>(i)); 
  }
  return digits;
}


}

