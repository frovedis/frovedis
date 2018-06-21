#ifndef _TREE_HPP_
#define _TREE_HPP_

#include <unordered_map>
#include <utility>

#include "tree_impurity.hpp"
#include "tree_config.hpp"
#include "tree_model.hpp"
#include "tree_builder.hpp"

namespace frovedis {

struct decision_tree {
  template <typename T>
  static decision_tree_model<T> train_classifier(
    const colmajor_matrix<T>& data, dvector<T>& labels,
    const size_t num_classes = 2,
    std::unordered_map<size_t, size_t>
    categorical_features_info = std::unordered_map<size_t, size_t>(),
    const tree::impurity_type impurity = tree::impurity_type::Default,
    const size_t max_depth = 5,
    const size_t max_bins = 32,
    const size_t min_instances_per_node = 1,
    const T min_info_gain = 0.0
  ) {
    tree::strategy<T> strategy(
      tree::algorithm::Classification,
      impurity, max_depth, num_classes, max_bins,
      tree::quantile_strategy::ApproxHist,
      tree::categorize_strategy::Single,
      std::move(categorical_features_info),
      min_instances_per_node, min_info_gain
    );
    auto builder = make_decision_tree_builder<T>(std::move(strategy));
    return builder.run(data, labels);
  }

  template <typename T>
  static decision_tree_model<T> train_regressor(
    const colmajor_matrix<T>& data, dvector<T>& labels,
    std::unordered_map<size_t, size_t>
    categorical_features_info = std::unordered_map<size_t, size_t>(),
    const tree::impurity_type impurity = tree::impurity_type::Default,
    const size_t max_depth = 5,
    const size_t max_bins = 32,
    const size_t min_instances_per_node = 1,
    const T min_info_gain = 0.0
  ) {
    tree::strategy<T> strategy(
      tree::algorithm::Regression,
      impurity, max_depth, 0, max_bins,
      tree::quantile_strategy::ApproxHist,
      tree::categorize_strategy::Single,
      std::move(categorical_features_info),
      min_instances_per_node, min_info_gain
    );
    auto builder = make_decision_tree_builder<T>(std::move(strategy));
    return builder.run(data, labels);
  }
};

} // end namespace frovedis

#endif
