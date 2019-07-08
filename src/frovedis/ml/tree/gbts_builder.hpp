#ifndef _GBTS_BUILDER_HPP_
#define _GBTS_BUILDER_HPP_

#include <functional>
#include <utility>
#include <vector>

#include "../../../frovedis.hpp"
#include "../../matrix/colmajor_matrix.hpp"

#include "tree_builder.hpp"
#include "tree_config.hpp"

#include "ensemble_model.hpp"

namespace frovedis {

namespace tree {

template <typename T, typename G>
void negative_gradient_calculator(
  std::vector<T>& negative_gradients,
  const std::vector<T>& predictions,
  const std::vector<T>& labels,
  G& gfunc
) {
  const size_t num_records = negative_gradients.size();
  tree_assert(predictions.size() == num_records);
  tree_assert(labels.size() == num_records);

  const T* pre = predictions.data();
  const T* lab = labels.data();
  T* dst = negative_gradients.data();

  for (size_t i = 0; i < num_records; i++) {
    dst[i] = -gfunc(pre[i], lab[i]);
  }
}

template <typename T>
void prediction_updater(
  std::vector<T>& predictions,
  const std::vector<T>& inferences,
  const T learning_rate
) {
  const size_t num_records = predictions.size();
  tree_assert(inferences.size() == num_records);

  const T* src = inferences.data();
  T* dst = predictions.data();

  for (size_t i = 0; i < num_records; i++) {
    dst[i] += src[i] * learning_rate;
  }
}

template <typename T, typename A, typename I, typename L>
class gbts_builder_impl {
  static constexpr bool INFERENCE = true;

  algorithm algo;
  builder_impl<T, A, I, INFERENCE> tree_builder;
  L loss_functor;

public:
  gbts_builder_impl(
    strategy<T> tree_strategy,
    I impurity_functor = I(),
    L loss_functor = L()
  ) :
    algo(tree_strategy.get_algorithm()),
    tree_builder(
      tree_strategy
        .set_ensemble_type(ensemble_type::GradientBoosting)
        .set_algorithm(algorithm::Regression)
        .move(),
      std::move(impurity_functor)
    ),
    loss_functor(std::move(loss_functor))
  {}

  const strategy<T>& get_strategy() const& {
    return tree_builder.get_strategy();
  }

  strategy<T> get_strategy() && {
    return std::move(tree_builder.get_strategy());
  }

  gradient_boosted_trees_model<T>
  operator()(const colmajor_matrix<T>& dataset, dvector<T>& labels) {
    const size_t num_iters = get_strategy().get_num_iterations();
    const T learning_rate = get_strategy().get_learning_rate();
    std::vector<decision_tree_model<T>> trees(num_iters);
    std::vector<T> weights(num_iters, learning_rate);

    if (num_iters == 0) {
      return gradient_boosted_trees_model<T>(
        algo, std::move(trees), std::move(weights)
      );
    }

    tree_assert(num_iters >= 1);
    decision_tree_model<T>* dest = trees.data();

    // first tree
#ifdef _TREE_DEBUG_
    RLOG(INFO) << "==== tree[0] ====" << std::endl;
#endif
    weights[0] = 1;
    dest[0] = tree_builder(dataset, labels);

    if (num_iters == 1) {
      return gradient_boosted_trees_model<T>(
        algo, std::move(trees), std::move(weights)
      );
    }

    tree_assert(num_iters >= 2);
    const size_t last_iter = num_iters - 1;
    auto labels_view = labels.viewas_node_local();

    using G = gradient_functor<T, L>;
    auto bcas_gfunc = make_node_local_broadcast(
      G(loss_functor, get_strategy().get_delta())
    );
    auto bcas_learning_rate = make_node_local_broadcast(learning_rate);

    // get initial predictions by moving inferences
    dvector<T> predictions = std::move(tree_builder.get_inferences());
    auto predictions_view = predictions.viewas_node_local();

    // negative gradients keeper
    dvector<T> negative_gradients = labels;
    auto negative_gradients_view = negative_gradients.viewas_node_local();

    // boosting [1, num_iters - 1]
_Pragma(__novector__)
    for (size_t i = 1; i < last_iter; i++) {
#ifdef _TREE_DEBUG_
      RLOG(INFO) << "==== tree[" << i << "] ====" << std::endl;
#endif
      negative_gradients_view.mapv(
        negative_gradient_calculator<T, G>,
        predictions_view, labels_view, bcas_gfunc
      );

      dest[i] = tree_builder(dataset, negative_gradients);

      predictions_view.mapv(
        prediction_updater<T>,
        tree_builder.get_inferences().viewas_node_local(),
        bcas_learning_rate
      );
    }

    // last tree
#ifdef _TREE_DEBUG_
    RLOG(INFO) << "==== tree[" << last_iter << "] ====" << std::endl;
#endif
    negative_gradients_view.mapv(
      negative_gradient_calculator<T, G>,
      predictions_view, labels_view, bcas_gfunc
    );
    dest[last_iter] = tree_builder(dataset, negative_gradients);

    return gradient_boosted_trees_model<T>(
      algo, std::move(trees), std::move(weights)
    );
  }

  // TODO: implement run_with_validation
};

} // end namespace tree

template <typename T>
class gradient_boosted_trees_builder {
  std::function<
    gradient_boosted_trees_model<T>(const colmajor_matrix<T>&, dvector<T>&)
  > wrapper;

public:
  gradient_boosted_trees_builder(tree::strategy<T>);

  template <typename Policy, typename Impurity, typename Loss>
  gradient_boosted_trees_builder(
    tree::gbts_builder_impl<T, Policy, Impurity, Loss> builder
  ) :
    wrapper(std::move(builder))
  {}

  gradient_boosted_trees_model<T>
  run(const colmajor_matrix<T>& dataset, dvector<T>& labels) {
    return wrapper(dataset, labels);
  }

  // TODO: implement run_with_validation
};

// TODO: simplify
template <typename T>
gradient_boosted_trees_builder<T>::gradient_boosted_trees_builder(
  tree::strategy<T> strategy
) : wrapper() {
  using namespace tree;
  using REG = regression_policy<T>;
  using VAR = variance_functor<T>;
  using FRM = friedmanvar_functor<T>;
  using MSE = defvariance_functor<T>;
  using MAE = meanabserror_functor<T>;
  using LOG = logloss_functor<T>;
  using LSE = leastsqerror_functor<T>;
  using LAE = leastabserror_functor<T>;

  switch (strategy.algo) {
  case algorithm::Classification:
    switch (strategy.impurity) {
    case impurity_type::Default:
    case impurity_type::Variance:
      switch (strategy.loss) {
      case loss_type::Default:
      case loss_type::LogLoss:
        wrapper = gbts_builder_impl<T, REG, VAR, LOG>(strategy.move());
        return;
      default:
        throw std::logic_error("invalid loss type");
      }
    case impurity_type::FriedmanVariance:
      switch (strategy.loss) {
      case loss_type::Default:
      case loss_type::LogLoss:
        wrapper = gbts_builder_impl<T, REG, FRM, LOG>(strategy.move());
        return;
      default:
        throw std::logic_error("invalid loss type");
      }
    case impurity_type::DefVariance:
      switch (strategy.loss) {
      case loss_type::Default:
      case loss_type::LogLoss:
        wrapper = gbts_builder_impl<T, REG, MSE, LOG>(strategy.move());
        return;
      default:
        throw std::logic_error("invalid loss type");
      }
    case impurity_type::MeanAbsError:
      switch (strategy.loss) {
      case loss_type::Default:
      case loss_type::LogLoss:
        wrapper = gbts_builder_impl<T, REG, MAE, LOG>(strategy.move());
        return;
      default:
        throw std::logic_error("invalid loss type");
      }
    default:
      throw std::logic_error("invalid impurity type");
    }
  case algorithm::Regression:
    switch (strategy.impurity) {
    case impurity_type::Default:
    case impurity_type::Variance:
      switch (strategy.loss) {
      case loss_type::Default:
      case loss_type::LeastSquaresError:
        wrapper = gbts_builder_impl<T, REG, VAR, LSE>(strategy.move());
        return;
      case loss_type::LeastAbsoluteError:
        wrapper = gbts_builder_impl<T, REG, VAR, LAE>(strategy.move());
        return;
      default:
        throw std::logic_error("invalid loss type");
      }
    case impurity_type::FriedmanVariance:
      switch (strategy.loss) {
      case loss_type::Default:
      case loss_type::LeastSquaresError:
        wrapper = gbts_builder_impl<T, REG, FRM, LSE>(strategy.move());
        return;
      case loss_type::LeastAbsoluteError:
        wrapper = gbts_builder_impl<T, REG, FRM, LAE>(strategy.move());
        return;
      default:
        throw std::logic_error("invalid loss type");
      }
    case impurity_type::DefVariance:
      switch (strategy.loss) {
      case loss_type::Default:
      case loss_type::LeastSquaresError:
        wrapper = gbts_builder_impl<T, REG, MSE, LSE>(strategy.move());
        return;
      case loss_type::LeastAbsoluteError:
        wrapper = gbts_builder_impl<T, REG, MSE, LAE>(strategy.move());
        return;
      default:
        throw std::logic_error("invalid loss type");
      }
    case impurity_type::MeanAbsError:
      switch (strategy.loss) {
      case loss_type::Default:
      case loss_type::LeastSquaresError:
        wrapper = gbts_builder_impl<T, REG, MAE, LSE>(strategy.move());
        return;
      case loss_type::LeastAbsoluteError:
        wrapper = gbts_builder_impl<T, REG, MAE, LAE>(strategy.move());
        return;
      default:
        throw std::logic_error("invalid loss type");
      }
    default:
      throw std::logic_error("invalid impurity type");
    }
  default:
    throw std::logic_error("invalid tree algorithm");
  }
}

template <typename T>
inline gradient_boosted_trees_builder<T>
make_gradient_boosted_trees_builder(tree::strategy<T> strategy) {
  return gradient_boosted_trees_builder<T>(strategy.move());
}

} // end namespace frovedis

#endif
