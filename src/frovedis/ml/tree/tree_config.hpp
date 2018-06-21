#ifndef _TREE_CONFIG_HPP_
#define _TREE_CONFIG_HPP_

#include <string>
#include <unordered_map>
#include <utility>

// explicit include for SX
#include "../../core/unordered_map_serialize.hpp"

#include "tree_impurity.hpp"

namespace frovedis {
namespace tree {

enum class algorithm {
  Classification,
  Regression,
};

enum class continuity {
  Categorical,
  Continuous,
};

// for compatibility
using feature_type = continuity;

enum class quantile_strategy {
  ApproxHist,
  //MinMax,
  //Sort,
};

enum class categorize_strategy {
  Single,
  //Multiple,
};

constexpr size_t NORANDOM = 0;

template <typename T>
struct strategy {
  strategy() {}
  strategy(
    const algorithm algo,
    const impurity_type impurity = impurity_type::Default,
    const size_t max_depth = 5,
    const size_t num_classes = 2,
    const size_t max_bins = 32,
    const quantile_strategy qstrategy = quantile_strategy::ApproxHist,
    const categorize_strategy cstrategy = categorize_strategy::Single,
    std::unordered_map<size_t, size_t>
    categorical_features_info = std::unordered_map<size_t, size_t>(),
    const size_t min_instances_per_node = 1,
    const T min_info_gain = 0.0,
    const size_t matmul_threshold = 256,
    const size_t seed = NORANDOM
  ) :
    algo(algo),
    impurity(impurity),
    qstrategy(qstrategy),
    cstrategy(cstrategy),
    categorical_features_info(std::move(categorical_features_info)),
    max_depth(max_depth),
    num_classes(num_classes),
    max_bins(max_bins),
    min_instances_per_node(min_instances_per_node),
    min_info_gain(min_info_gain),
    matmul_threshold(matmul_threshold),
    seed(seed)
  {}

  strategy<T>& set_algorithm(const algorithm algo) {
    this->algo = algo;
    return *this;
  }

  strategy<T>& set_impurity_type(const impurity_type impurity) {
    this->impurity = impurity;
    return *this;
  }

  strategy<T>& set_max_depth(const size_t max_depth) {
    this->max_depth = max_depth;
    return *this;
  }

  strategy<T>& set_num_classes(const size_t num_classes) {
    this->num_classes = num_classes;
    return *this;
  }

  strategy<T>& set_max_bins(const size_t max_bins) {
    this->max_bins = max_bins;
    return *this;
  }

  strategy<T>& set_quantile_strategy(
    const quantile_strategy qstrategy
  ) {
    this->qstrategy = qstrategy;
    return *this;
  }

  strategy<T>& set_categorize_strategy(
    const categorize_strategy cstrategy
  ) {
    this->cstrategy = cstrategy;
    return *this;
  }

  strategy<T>& set_categorical_features_info(
    std::unordered_map<size_t, size_t> categorical_features_info
  ) {
    this->categorical_features_info = std::move(categorical_features_info);
    return *this;
  }

  strategy<T>& set_min_instances_per_node(
    const size_t min_instances_per_node
  ) {
    this->min_instances_per_node = min_instances_per_node;
    return *this;
  }

  strategy<T>& set_min_info_gain(const T min_info_gain) {
    this->min_info_gain = min_info_gain;
    return *this;
  }

  strategy<T>& set_matmul_threshold(const size_t matmul_threshold) {
    this->matmul_threshold = matmul_threshold;
    return *this;
  }

  strategy<T>& set_seed(const size_t seed) {
    this->seed = seed;
    return *this;
  }

  algorithm get_algorithm() const { return algo; }
  impurity_type get_impurity_type() const { return impurity; }
  size_t get_max_depth() const { return max_depth; }
  size_t get_num_classes() const { return num_classes; }
  size_t get_max_bins() const { return max_bins; }

  quantile_strategy get_quantile_strategy() const {
    return qstrategy;
  }
  categorize_strategy get_categorize_strategy() const {
    return cstrategy;
  }

  const std::unordered_map<size_t, size_t>&
  get_categorical_features_info() const& {
    return categorical_features_info;
  }

  std::unordered_map<size_t, size_t>
  get_categorical_features_info() && { // rvalue overload
    return std::move(categorical_features_info);
  }

  size_t get_min_instances_per_node() const {
    return min_instances_per_node;
  }

  T get_min_info_gain() const { return min_info_gain; }

  size_t get_matmul_threshold() const { return matmul_threshold; }
  size_t get_seed() const { return seed; }
  bool random_enabled() const { return seed != NORANDOM; }

  algorithm algo;
  impurity_type impurity;
  quantile_strategy qstrategy;
  categorize_strategy cstrategy;
  std::unordered_map<size_t, size_t> categorical_features_info;
  size_t max_depth, num_classes, max_bins, min_instances_per_node;
  T min_info_gain;
  size_t matmul_threshold, seed;

  SERIALIZE(
    algo, impurity, qstrategy, cstrategy,
    categorical_features_info,
    max_depth, num_classes, max_bins,
    min_instances_per_node, min_info_gain,
    matmul_threshold, seed
  )
};

inline void ltrim(std::string& s) {
  s.erase(
    s.begin(), std::find_if(
      s.begin(), s.end(), [](int c) { return !std::isspace(c); }
    )
  );
}

inline void rtrim(std::string& s) {
  s.erase(
    std::find_if(
      s.rbegin(), s.rend(), [](int c) { return !std::isspace(c); }
    ).base(), s.end()
  );
}

inline void trim(std::string& s) {
  ltrim(s);
  rtrim(s);
}

inline std::string trim_copy(std::string s) {
  trim(s);
  return s;
}

std::unordered_map<size_t, size_t>
parse_categorical_features_info(const std::string&, const size_t = 2);

} // end namespace tree
} // end namespace frovedis

#endif
