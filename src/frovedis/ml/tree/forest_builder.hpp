#ifndef _FOREST_BUILDER_HPP_
#define _FOREST_BUILDER_HPP_

#include <memory>
#include <utility>
#include <vector>

#include "../../../frovedis.hpp"
#include "../../matrix/colmajor_matrix.hpp"

#include "tree_builder.hpp"
#include "tree_config.hpp"

#include "ensemble_model.hpp"

namespace frovedis {

namespace tree {

template <typename T>
using forest_builder = model_builder<T, random_forest_model<T>>;

template <typename T, typename A, typename F>
class forest_builder_impl : public forest_builder<T> {
  unitree_builder_impl<T, A, F> builder;

public:
  explicit forest_builder_impl(
    strategy<T> tree_strategy, F impurity_functor = F()
  ) :
    builder(
      tree_strategy.set_ensemble_type(ensemble_type::RandomForest).move(),
      std::move(impurity_functor)
    )
  {}

  forest_builder_impl(const forest_builder_impl<T, A, F>& src) :
    builder(src.builder)
  {}

  forest_builder_impl(forest_builder_impl<T, A, F>&& src) :
    builder(std::move(src.builder))
  {}

  forest_builder_impl<T, A, F>&
  operator=(const forest_builder_impl<T, A, F>& src) {
    this->builder = src.builder;
    return *this;
  }

  forest_builder_impl<T, A, F>&
  operator=(forest_builder_impl<T, A, F>&& src) {
    this->builder = std::move(src.builder);
    return *this;
  }

  virtual ~forest_builder_impl() override {}

  const strategy<T>& get_strategy() const& {
    return builder.get_strategy();
  }

  strategy<T> get_strategy() && {
    return std::move(builder.get_strategy());
  }

  virtual std::unique_ptr<forest_builder<T>> copy_ptr() const override {
    return std::make_unique<forest_builder_impl<T, A, F>>(*this);
  }

  virtual random_forest_model<T>
  build(const colmajor_matrix<T>& dataset, dvector<T>& labels) override {
    const size_t num_trees = get_strategy().get_num_trees();
    std::vector<decision_tree_model<T>> trees(num_trees);
    decision_tree_model<T>* dest = trees.data();

_Pragma(__novector__)
    for (size_t i = 0; i < num_trees; i++) {
      dest[i] = builder(dataset, labels);
    }

    return random_forest_model<T>(
      get_strategy().get_algorithm(), std::move(trees)
    );
  }
};

template <typename T, typename A, typename F, typename... Args>
inline std::unique_ptr<forest_builder<T>>
make_forest_builder(Args&&... args) {
  return std::make_unique<forest_builder_impl<T, A, F>>(
    std::forward<Args>(args)...
  );
}

} // end namespace tree

template <typename T>
class random_forest_builder {
  std::unique_ptr<tree::forest_builder<T>> builder_ptr;

public:
  explicit random_forest_builder(tree::strategy<T>);

  random_forest_builder(const random_forest_builder<T>& src) :
    builder_ptr(src.builder_ptr->copy_ptr())
  {}

  random_forest_builder(random_forest_builder<T>&& src) :
    builder_ptr(std::move(src.builder_ptr))
  {}

  random_forest_builder<T>&
  operator=(const random_forest_builder<T>& src) {
    this->builder_ptr = src.builder_ptr->copy_ptr();
    return *this;
  }

  random_forest_builder<T>&
  operator=(random_forest_builder<T>&& src) {
    this->builder_ptr = std::move(src.builder_ptr);
    return *this;
  }

  // ctor from a pointer of any forest_builder_impl<...>
  explicit random_forest_builder(
    std::unique_ptr<tree::forest_builder<T>>&& ptr
  ) :
    builder_ptr(std::move(ptr))
  {}

  // ctor from an object of any forest_builder_impl<...>
  template <typename Policy, typename Impurity>
  explicit random_forest_builder(
    tree::forest_builder_impl<T, Policy, Impurity> builder
  ) :
    builder_ptr(
      tree::make_forest_builder<T, Policy, Impurity>(std::move(builder))
    )
  {}

  random_forest_model<T>
  run(const colmajor_matrix<T>& dataset, dvector<T>& labels) {
    return builder_ptr->build(dataset, labels);
  }
};

template <typename T>
random_forest_builder<T>::random_forest_builder(
  tree::strategy<T> strategy
) : builder_ptr() {
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
      builder_ptr = make_forest_builder<T, CLA, GIN>(strategy.move());
      return;
    case impurity_type::Entropy:
      builder_ptr = make_forest_builder<T, CLA, ENT>(strategy.move());
      return;
    case impurity_type::MisclassRate:
      builder_ptr = make_forest_builder<T, CLA, MCR>(strategy.move());
      return;
    default:
      throw std::logic_error("invalid impurity type");
    }
  case algorithm::Regression:
    switch (strategy.impurity) {
    case impurity_type::Default:
    case impurity_type::Variance:
      builder_ptr = make_forest_builder<T, REG, VAR>(strategy.move());
      return;
    case impurity_type::FriedmanVariance:
      builder_ptr = make_forest_builder<T, REG, FRM>(strategy.move());
      return;
    case impurity_type::DefVariance:
      builder_ptr = make_forest_builder<T, REG, MSE>(strategy.move());
      return;
    case impurity_type::MeanAbsError:
      builder_ptr = make_forest_builder<T, REG, MAE>(strategy.move());
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
