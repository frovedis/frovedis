#include <string>
#include <typeinfo>

#include "type_utility.hpp"

#if defined(_SX) || defined(__ve__)
template <>
std::string frovedis::get_typename<std::type_info>(const std::type_info& info) {
  return std::string(info.name());
}
#else

#include <cxxabi.h>
#include <cstdlib>
#include <memory>

template <>
std::string frovedis::get_typename<std::type_info>(const std::type_info& info) {
  int status = 0;
  std::unique_ptr<char, decltype(&std::free)> ptr(abi::__cxa_demangle(info.name(), nullptr, nullptr, &status), std::free);

  if (!ptr) {
    switch (status) {
    case -1:
      return "memory allocation failure";
    case -2:
      return "invalid mangled name";
    case -3:
      return "invalid arguments";
    default:
      return "shouldn't reach here";
    }
  }

  return std::string(ptr.get());
}

#endif
