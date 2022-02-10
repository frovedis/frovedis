#include <algorithm>
#include <iterator>
#include <numeric>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../../../frovedis.hpp"
#include "tree_config.hpp"

namespace frovedis {
namespace tree {

std::unordered_map<std::string, subsampling_strategy>
get_subsampling_map() {
  std::unordered_map<std::string, subsampling_strategy> m;
  m.emplace("auto", subsampling_strategy::Auto);
  m.emplace("disable", subsampling_strategy::Disable);
  m.emplace("enable", subsampling_strategy::Enable);
  m.emplace("bootstrap", subsampling_strategy::Bootstrap);
  return m;
}

subsampling_strategy
get_subsampling_strategy(const std::string& arg) {
  static auto ssmap = get_subsampling_map();
  std::string str;
  str.resize(arg.length());
  std::transform(arg.cbegin(), arg.cend(), str.begin(), ::tolower);

  try {
    return ssmap.at(str);
  } catch (const std::out_of_range&) {
    throw std::runtime_error(
      std::string("invalid subsampling strategy: ") + arg
    );
  }
}

std::unordered_map<std::string, feature_subset_strategy>
get_feature_subset_map() {
  std::unordered_map<std::string, feature_subset_strategy> m;
  m.emplace("auto", feature_subset_strategy::Auto);
  m.emplace("all", feature_subset_strategy::All);
  m.emplace("onethird", feature_subset_strategy::OneThird);
  m.emplace("1/3", feature_subset_strategy::OneThird);
  m.emplace("sqrt", feature_subset_strategy::Sqrt);
  m.emplace("log2", feature_subset_strategy::Log2);
  m.emplace("custom", feature_subset_strategy::CustomRate);
  m.emplace("customrate", feature_subset_strategy::CustomRate);
  return m;
}

feature_subset_strategy
get_feature_subset_strategy(const std::string& arg) {
  static auto fsmap = get_feature_subset_map();
  std::string str;
  str.resize(arg.length());
  std::transform(arg.cbegin(), arg.cend(), str.begin(), ::tolower);

  try {
    return fsmap.at(str);
  } catch (const std::out_of_range&) {
    throw std::runtime_error(
      std::string("invalid feature subset strategy: ") + arg
    );
  }
}

// parse a dictionary-like string "key:value, key:value, ..."
std::unordered_map<size_t, size_t>
parse_categorical_features_info(
  const std::string& input,
  const size_t default_cardinality /* = 2 */
) {
  const auto target = trim_copy(input);

  // split by commas and colons
  std::string buffer;
  std::istringstream istream(target);
  std::vector<std::pair<std::string, std::string>> dict;

  while (std::getline(istream, buffer, ',')) {
    trim(buffer);
    const auto pos = buffer.find_first_of(':');
    if (pos == std::string::npos) {
      dict.emplace_back(buffer, "");
      continue;
    }

    std::string key, value;
    key = buffer.substr(0, pos);
    trim(key);

    if (pos + 1 < buffer.length()) {
      value = buffer.substr(pos + 1);
      trim(value);
    }

    dict.emplace_back(key, value);
  }

  // construct a map
  std::unordered_map<size_t, size_t> ret;
  for (const auto item: dict) {
    const auto key = item.first;
    const auto value = item.second;

    if (key.empty() && value.empty()) {
      RLOG(WARNING) << "WARNING: " <<
        "an empty categorical feature info has been ignored" <<
      std::endl;
      continue;
    }

    if (key.empty() && !value.empty()) {
      RLOG(WARNING) << "WARNING: " <<
        "an invalid categorical feature info has been ignored" <<
      std::endl;
      continue;
    }

    size_t key_extrapos = key.length();
    size_t index = std::stoull(key, &key_extrapos);

    size_t value_extrapos = value.length();
    size_t cardinality;
    if (value.empty()) {
      cardinality = default_cardinality;
      RLOG(TRACE) <<
        "a default cardinality has been applied"
        ": {" << index << ": " << cardinality << "}" <<
      std::endl;
    } else {
      cardinality = std::stoull(value, &value_extrapos);
    }

    if (key_extrapos < key.length() || value_extrapos < value.length()) {
      RLOG(WARNING) << "WARNING: " <<
        "extra characters has been ignored" <<
        ": {" << key << ": " << value << "} " <<
        "-> {" << index << ": " << cardinality << "}" <<
      std::endl;
    }

    auto status = ret.emplace(index, cardinality);
    if (!status.second) {
      RLOG(WARNING) << "WARNING: " <<
        "a duplicate categorical feature info has been ignored" <<
        ": {" << key << ": " << value << "}" <<
      std::endl;
    }
  }

  if (get_loglevel() <= TRACE) {
    std::ostringstream sout;
    if (!ret.empty()) {
      auto iter = ret.cbegin();
      sout << iter->first << ": " << iter->second;
      for (iter++; iter != ret.cend(); iter++) {
        sout << ", " << iter->first << ": " << iter->second;
      }
    }

    RLOG(TRACE) <<
      "categorical features info has been parsed" <<
      ": {" << sout.str() << "}" <<
    std::endl;
  }

  return ret;
}

} // end namespace tree
} // end namespace frovedis
