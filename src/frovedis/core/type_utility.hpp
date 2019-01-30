#ifndef TYPE_UTILITY_HPP
#define TYPE_UTILITY_HPP

#include <cstddef>
#include <string>
#include <typeinfo>
#include <type_traits>

#include "invoke_result.hpp"

namespace frovedis {

// inference tool for a result type of map
template <typename F, typename T, typename... Others>
using map_result = frovedis::invoke_result<
  F,
  typename std::add_lvalue_reference<T>::type,
  typename std::add_lvalue_reference<Others>::type...
>;

template <typename F, typename T, typename... Others>
using map_result_t = typename map_result<F, T, Others...>::type;

// SFINAE tools
template <bool Condition, typename T = std::nullptr_t>
using enable_if = std::enable_if<Condition, T>;

template <bool Condition, typename T = std::nullptr_t>
using enable_if_t = typename enable_if<Condition, T>::type;

template <typename T>
using enable_if_fp = enable_if_t<std::is_floating_point<T>::value>;

template <typename T>
using enable_if_not_fp = enable_if_t<!std::is_floating_point<T>::value>;

template <typename T>
using enable_if_uint = enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value>;

template <typename T>
struct is_string {
  static constexpr bool value = std::is_same<std::string, typename std::remove_cv<T>::type>::value;
};

template <typename T>
using enable_if_str = enable_if_t<is_string<T>::value>;

template <typename T>
using enable_if_not_str = enable_if_t<!is_string<T>::value>;

// typename tools
template <typename T>
std::string get_typename(const T&);

template <>
std::string get_typename<std::type_info>(const std::type_info&);

template <typename T>
std::string get_typename(const T&) {
  return get_typename<std::type_info>(typeid(T));
}

} // end namespace frovedis

#endif
