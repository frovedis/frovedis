#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "tree_impurity.hpp"

namespace frovedis {
namespace tree {

std::unordered_map<std::string, impurity_type> get_impurity_map() {
  std::unordered_map<std::string, impurity_type> m;
  m.emplace("default", impurity_type::Default);
  m.emplace("gini", impurity_type::Gini);
  m.emplace("entropy", impurity_type::Entropy);
  m.emplace("misclassification", impurity_type::MisclassRate);
  m.emplace("variance", impurity_type::Variance);
  m.emplace("mse", impurity_type::Variance);
  m.emplace("friedman_mse", impurity_type::FriedmanVariance);
  m.emplace("definitional-variance", impurity_type::DefVariance);
  m.emplace("mae", impurity_type::MeanAbsError);
  return m;
}

impurity_type get_impurity_type(const std::string& arg) {
  static auto imap = get_impurity_map();
  std::string str;
  str.resize(arg.length());
  std::transform(arg.cbegin(), arg.cend(), str.begin(), ::tolower);

  try {
    return imap.at(str);
  } catch (const std::out_of_range&) {
    throw std::runtime_error(std::string("invalid impurity type: ") + arg);
  }
}

} // end namespace tree
} // end namespace frovedis
