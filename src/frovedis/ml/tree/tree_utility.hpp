#ifndef _TREE_UTILITY_HPP_
#define _TREE_UTILITY_HPP_

#include <array>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "../../../frovedis.hpp"

namespace frovedis {
namespace tree {

template <typename T>
std::string string_join(
  const std::vector<T>&, const std::string& = std::string()
);

template <>
std::string string_join(
  const std::vector<std::string>&, const std::string&
);

template <typename T>
std::string string_join(const std::vector<T>& v, const std::string& d) {
  std::vector<std::string> result;
  std::transform(
    v.cbegin(), v.cend(), std::back_inserter(result),
    [] (const T& x) { return std::to_string(x); }
  );
  return string_join(result, d);
}

template <typename T>
std::string string_join(
  const dvector<T>& dv, const std::string& d = std::string()
) {
  return string_join(const_cast<dvector<T>&>(dv).gather(), d);
}

template <typename T>
std::string string_join(
  const node_local<T>& lc, const std::string& d = std::string()
) {
  return string_join(const_cast<node_local<T>&>(lc).gather(), d);
}

union union32_t {
  float f;
  uint32_t uint;
};

union union64_t {
  double f;
  uint64_t uint;
};

inline uint32_t f2uint(float value) {
  return reinterpret_cast<union32_t*>(&value)->uint;
}

inline uint64_t f2uint(double value) {
  return reinterpret_cast<union64_t*>(&value)->uint;
}

inline uint32_t* fp2uintp(float* ptr) {
  return &reinterpret_cast<union32_t*>(ptr)->uint;
}

inline uint64_t* fp2uintp(double* ptr) {
  return &reinterpret_cast<union64_t*>(ptr)->uint;
}

inline const uint32_t* fp2uintp(const float* ptr) {
  return &reinterpret_cast<const union32_t*>(ptr)->uint;
}

inline const uint64_t* fp2uintp(const double* ptr) {
  return &reinterpret_cast<const union64_t*>(ptr)->uint;
}

template <typename T, typename... Args>
inline std::array<T, sizeof...(Args)> make_array(Args&&... args) {
  return std::array<T, sizeof...(Args)>{ std::forward<Args>(args)... };
}

} // end namespace tree
} // end namespace frovedis

#endif
