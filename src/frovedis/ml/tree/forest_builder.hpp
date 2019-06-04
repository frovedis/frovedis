#ifndef _FOREST_BUILDER_HPP_
#define _FOREST_BUILDER_HPP_

#include <functional>
#include <utility>
#include <vector>

#include "../../../frovedis.hpp"
#include "../../matrix/colmajor_matrix.hpp"

#include "tree_builder.hpp"
#include "tree_config.hpp"

#include "forest_model.hpp"

namespace frovedis {

namespace tree {

template <typename T, typename A, typename F>
class forest_builder_impl {
  builder_impl<T, A, F> tree_builder;

public:
  forest_builder_impl(
    strategy<T> tree_strategy, F impurity_functor = F()
  ) :
    tree_builder(
      tree_strategy.set_ensemble_type(ensemble_type::RandomForest).move(),
      std::move(impurity_functor)
    )
  {}

  const strategy<T>& get_strategy() const& {
    return tree_builder.get_strategy();
  }

  strategy<T> get_strategy() && {
    return std::move(tree_builder.get_strategy());
  }

  random_forest_model<T>
  operator()(const colmajor_matrix<T>& dataset, dvector<T>& labels) {
    const size_t num_trees = get_strategy().get_num_trees();
    std::vector<decision_tree_model<T>> trees(num_trees);
    decision_tree_model<T>* dest = trees.data();

_Pragma(__novector__)
    for (size_t i = 0; i < num_trees; i++) {
      dest[i] = tree_builder(dataset, labels);
    }

    return random_forest_model<T>(
      std::move(trees), get_strategy().get_algorithm()
    );
  }
};

} // end namespace tree

template <typename T>
class random_forest_builder {
  std::function<
    random_forest_model<T>(const colmajor_matrix<T>&, dvector<T>&)
  > wrapper;

public:
  random_forest_builder(tree::strategy<T>);

  template <typename AlgorithmPolicy, typename ImpurityFunctor>
  random_forest_builder(
    tree::forest_builder_impl<T, AlgorithmPolicy, ImpurityFunctor> builder
  ) :
    wrapper(std::move(builder))
  {}

  random_forest_model<T>
  run(const colmajor_matrix<T>& dataset, dvector<T>& labels) {
    return wrapper(dataset, labels);
  }
};

template <typename T>
random_forest_builder<T>::random_forest_builder(
  tree::strategy<T> strategy
) : wrapper() {
  using namespace tree;
  using CLA = classification_policy<T>;
  using REG = regression_policy<T>;
  using GIN = gini_functor<T>;
  using ENT = entropy_functor<T>;
  using MCR = misclassrate_functor<T>;
  using VAR = variance_functor<T>;
  using FRM = friedmanvar_functor<T>;
  using MSE = defvariance_functor<T>;
  using MAE = meanabserror_functor<T>;

  switch (strategy.algo) {
  case algorithm::Classification:
    switch (strategy.impurity) {
    case impurity_type::Default:
    case impurity_type::Gini:
      wrapper = forest_builder_impl<T, CLA, GIN>(strategy.move());
      return;
    case impurity_type::Entropy:
      wrapper = forest_builder_impl<T, CLA, ENT>(strategy.move());
      return;
    case impurity_type::MisclassRate:
      wrapper = forest_builder_impl<T, CLA, MCR>(strategy.move());
      return;
    default:
      throw std::logic_error("invalid impurity type");
    }
  case algorithm::Regression:
    switch (strategy.impurity) {
    case impurity_type::Default:
    case impurity_type::Variance:
      wrapper = forest_builder_impl<T, REG, VAR>(strategy.move());
      return;
    case impurity_type::FriedmanVariance:
      wrapper = forest_builder_impl<T, REG, FRM>(strategy.move());
      return;
    case impurity_type::DefVariance:
      wrapper = forest_builder_impl<T, REG, MSE>(strategy.move());
      return;
    case impurity_type::MeanAbsError:
      wrapper = forest_builder_impl<T, REG, MAE>(strategy.move());
      return;
    default:
      throw std::logic_error("invalid impurity type");
    }
  default:
    throw std::logic_error("invalid tree algorithm");
  }
}

template <typename T>
inline random_forest_builder<T>
make_random_forest_builder(tree::strategy<T> strategy) {
  return random_forest_builder<T>(strategy.move());
}

} // end namespace frovedis

#endif
