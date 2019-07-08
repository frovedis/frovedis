#ifndef _TREE_HPP_
#define _TREE_HPP_

#include <unordered_map>
#include <utility>

#include "tree_impurity.hpp"
#include "tree_config.hpp"
#include "tree_model.hpp"
#include "tree_builder.hpp"

#include "ensemble_model.hpp"
#include "forest_builder.hpp"
#include "gbts_builder.hpp"

namespace frovedis {

struct decision_tree {
  template <typename T>
  static decision_tree_model<T> train_classifier(
    const colmajor_matrix<T>& dataset, dvector<T>& labels,
    const size_t num_classes = 2,
    std::unordered_map<size_t, size_t>
    categorical_features_info = std::unordered_map<size_t, size_t>(),
    const tree::impurity_type impurity = tree::impurity_type::Default,
    const size_t max_depth = 5,
    const size_t max_bins = 32
  ) {
    auto strategy = tree::make_classification_strategy<T>()
      .set_num_classes(num_classes)
      .set_categorical_features_info(std::move(categorical_features_info))
      .set_impurity_type(impurity)
      .set_max_depth(max_depth)
      .set_max_bins(max_bins)
      .move();
    decision_tree_builder<T> builder(strategy.move());
    return builder.run(dataset, labels);
  }

  template <typename T>
  static decision_tree_model<T> train_regressor(
    const colmajor_matrix<T>& dataset, dvector<T>& labels,
    std::unordered_map<size_t, size_t>
    categorical_features_info = std::unordered_map<size_t, size_t>(),
    const tree::impurity_type impurity = tree::impurity_type::Default,
    const size_t max_depth = 5,
    const size_t max_bins = 32
  ) {
    auto strategy = tree::make_regression_strategy<T>()
      .set_categorical_features_info(std::move(categorical_features_info))
      .set_impurity_type(impurity)
      .set_max_depth(max_depth)
      .set_max_bins(max_bins)
      .move();
    decision_tree_builder<T> builder(strategy.move());
    return builder.run(dataset, labels);
  }
};

struct random_forest {
  template <typename T>
  static random_forest_model<T> train_classifier(
    const colmajor_matrix<T>& dataset, dvector<T>& labels,
    const size_t num_classes = 2,
    std::unordered_map<size_t, size_t>
    categorical_features_info = std::unordered_map<size_t, size_t>(),
    const size_t num_trees = 10,
    const tree::feature_subset_strategy
    feature_subset_strategy = tree::feature_subset_strategy::Auto,
    const tree::impurity_type impurity = tree::impurity_type::Default,
    const size_t max_depth = 4,
    const size_t max_bins = 32,
    const size_t seed = tree::strategy<T>::default_seed
  ) {
    auto strategy = tree::make_classification_strategy<T>()
      .set_num_classes(num_classes)
      .set_categorical_features_info(std::move(categorical_features_info))
      .set_num_trees(num_trees)
      .set_feature_subset_strategy(feature_subset_strategy)
      .set_impurity_type(impurity)
      .set_max_depth(max_depth)
      .set_max_bins(max_bins)
      .set_seed(seed)
      .move();
    random_forest_builder<T> builder(strategy.move());
    return builder.run(dataset, labels);
  }

  template <typename T>
  static random_forest_model<T> train_regressor(
    const colmajor_matrix<T>& dataset, dvector<T>& labels,
    std::unordered_map<size_t, size_t>
    categorical_features_info = std::unordered_map<size_t, size_t>(),
    const size_t num_trees = 10,
    const tree::feature_subset_strategy
    feature_subset_strategy = tree::feature_subset_strategy::Auto,
    const tree::impurity_type impurity = tree::impurity_type::Default,
    const size_t max_depth = 4,
    const size_t max_bins = 32,
    const size_t seed = tree::strategy<T>::default_seed
  ) {
    auto strategy = tree::make_regression_strategy<T>()
      .set_categorical_features_info(std::move(categorical_features_info))
      .set_num_trees(num_trees)
      .set_feature_subset_strategy(feature_subset_strategy)
      .set_impurity_type(impurity)
      .set_max_depth(max_depth)
      .set_max_bins(max_bins)
      .set_seed(seed)
      .move();
    random_forest_builder<T> builder(strategy.move());
    return builder.run(dataset, labels);
  }
};

struct gradient_boosted_trees {
  template <typename T>
  static gradient_boosted_trees_model<T> train_classifier(
    const colmajor_matrix<T>& dataset, dvector<T>& labels,
    //const size_t num_classes = 2,
    std::unordered_map<size_t, size_t>
    categorical_features_info = std::unordered_map<size_t, size_t>(),
    const tree::impurity_type impurity = tree::impurity_type::Default,
    const size_t max_depth = 5,
    const size_t max_bins = 32,
    const size_t num_iterations = 10,
    const tree::loss_type loss = tree::loss_type::Default,
    const T learning_rate = 0.1
  ) {
    constexpr size_t __num_classes__ = 2;
    auto strategy = tree::make_classification_strategy<T>()
      .set_num_classes(__num_classes__)
      .set_categorical_features_info(std::move(categorical_features_info))
      .set_impurity_type(impurity)
      .set_max_depth(max_depth)
      .set_max_bins(max_bins)
      .set_num_iterations(num_iterations)
      .set_loss_type(loss)
      .set_learning_rate(learning_rate)
      .move();
    gradient_boosted_trees_builder<T> builder(strategy.move());
    return builder.run(dataset, labels);
  }

  template <typename T>
  static gradient_boosted_trees_model<T> train_regressor(
    const colmajor_matrix<T>& dataset, dvector<T>& labels,
    std::unordered_map<size_t, size_t>
    categorical_features_info = std::unordered_map<size_t, size_t>(),
    const tree::impurity_type impurity = tree::impurity_type::Default,
    const size_t max_depth = 5,
    const size_t max_bins = 32,
    const size_t num_iterations = 10,
    const tree::loss_type loss = tree::loss_type::Default,
    const T learning_rate = 0.1
  ) {
    auto strategy = tree::make_regression_strategy<T>()
      .set_categorical_features_info(std::move(categorical_features_info))
      .set_impurity_type(impurity)
      .set_max_depth(max_depth)
      .set_max_bins(max_bins)
      .set_num_iterations(num_iterations)
      .set_loss_type(loss)
      .set_learning_rate(learning_rate)
      .move();
    gradient_boosted_trees_builder<T> builder(strategy.move());
    return builder.run(dataset, labels);
  }
};

} // end namespace frovedis

#endif
