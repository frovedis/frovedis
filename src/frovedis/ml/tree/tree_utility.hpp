#ifndef _TREE_UTILITY_HPP_
#define _TREE_UTILITY_HPP_

#include <string>
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

} // end namespace tree
} // end namespace frovedis

#endif
