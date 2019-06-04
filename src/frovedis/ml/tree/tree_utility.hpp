#ifndef _TREE_UTILITY_HPP_
#define _TREE_UTILITY_HPP_

#include <algorithm>
#include <fstream>
#include <ios>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>

#include "../../../frovedis.hpp"
#include "../../core/type_utility.hpp"

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

template <typename A>
struct is_input_archive {
  static constexpr bool value = (
    std::is_same<A, cereal::BinaryInputArchive>::value ||
    std::is_same<A, cereal::JSONInputArchive>::value ||
    std::is_same<A, cereal::XMLInputArchive>::value
  );
};

template <typename A>
struct is_output_archive {
  static constexpr bool value = (
    std::is_same<A, cereal::BinaryOutputArchive>::value ||
    std::is_same<A, cereal::JSONOutputArchive>::value ||
    std::is_same<A, cereal::XMLOutputArchive>::value
  );
};

template <typename A>
struct is_text_archive {
  static constexpr bool value = (
    std::is_same<A, cereal::JSONInputArchive>::value ||
    std::is_same<A, cereal::JSONOutputArchive>::value ||
    std::is_same<A, cereal::XMLInputArchive>::value ||
    std::is_same<A, cereal::XMLOutputArchive>::value
  );
};

template <typename A>
struct is_binary_archive {
  static constexpr bool value = (
    std::is_same<A, cereal::BinaryInputArchive>::value ||
    std::is_same<A, cereal::BinaryOutputArchive>::value
  );
};

template <typename A, enable_if_t<is_input_archive<A>::value> = nullptr>
constexpr std::ios::openmode
get_rwmode() { return std::ios::in; }

template <typename A, enable_if_t<is_output_archive<A>::value> = nullptr>
constexpr std::ios::openmode
get_rwmode() { return std::ios::out; }

template <typename A, enable_if_t<is_text_archive<A>::value> = nullptr>
constexpr std::ios::openmode
get_openmode() { return get_rwmode<A>(); }

template <typename A, enable_if_t<is_binary_archive<A>::value> = nullptr>
constexpr std::ios::openmode
get_openmode() { return get_rwmode<A>() | std::ios::binary; }

template <typename Model, typename Archive>
void save_model(const Model& model, const std::string& path) {
  std::ofstream f;
  try {
    f.exceptions(std::ios::badbit | std::ios::failbit);
    f.open(path, get_openmode<Archive>());
    Archive archive(f);
    archive << model;
  } catch (const std::ios::failure& e) {
    std::ostringstream sout;
    sout << "failed to write to '" << path << "' (" << e.what() << ")";
    throw std::runtime_error(sout.str());
  }
}

template <typename Model, typename Archive>
Model load_model(const std::string& path) {
  Model model;
  std::ifstream f;

  try {
    f.exceptions(std::ios::badbit | std::ios::failbit);
    f.open(path, get_openmode<Archive>());
    // note: unset failbit not to throw an exception on eof
    f.exceptions(std::ios::badbit);
    Archive archive(f);
    archive >> model;
  } catch (const std::ios::failure& e) {
    std::ostringstream sout;
    sout << "failed to read '" << path <<"' (" << e.what() << ")";
    throw std::runtime_error(sout.str());
  }

  return model;
}

} // end namespace tree
} // end namespace frovedis

#endif
