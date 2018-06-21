#include <iterator>
#include <numeric>
#include <string>
#include <vector>

#include "tree_utility.hpp"

namespace frovedis {
namespace tree {

template <>
std::string string_join(
  const std::vector<std::string>& v, const std::string& d
) {
  if (v.empty()) { return std::string(); }
  return std::accumulate(
    std::next(v.cbegin(), 1), v.cend(), std::string(v.front()),
    [&d] (const std::string& x, const std::string& y) { return x + d + y; }
  );
}

} // end namespace tree
} // end namespace frovedis
