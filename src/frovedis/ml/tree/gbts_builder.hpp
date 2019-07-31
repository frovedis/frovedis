#ifndef _GBTS_BUILDER_HPP_
#define _GBTS_BUILDER_HPP_

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

template <typename T, typename G>
void negative_gradient_calculator(
  std::vector<T>& negative_gradients,
  const std::vector<T>& predicts,
  const std::vector<T>& labels,
  G& gfunc
) {
  const size_t num_records = negative_gradients.size();
  tree_assert(predicts.size() == num_records);
  tree_assert(labels.size() == num_records);

  const T* psrc = predicts.data();
  const T* lsrc = labels.data();
  T* dst = negative_gradients.data();

  for (size_t i = 0; i < num_records; i++) {
    dst[i] = -gfunc(psrc[i], lsrc[i]);
  }
}

template <typename T>
void prediction_updater(
  std::vector<T>& predicts,
  const std::vector<T>& inferences,
  const T learning_rate
) {
  const size_t num_records = predicts.size();
  tree_assert(inferences.size() == num_records);

  const T* src = inferences.data();
  T* dst = predicts.data();

  for (size_t i = 0; i < num_records; i++) {
    dst[i] += src[i] * learning_rate;
  }
}

template <typename T>
using gbts_builder = model_builder<T, gradient_boosted_trees_model<T>>;

template <typename T, typename I, typename L>
class gbts_builder_impl : public gbts_builder<T> {
  using REGRESSION = regression_policy<T>;
  static constexpr bool INFERENCE = true;

  algorithm algo;
  unitree_builder_impl<T, REGRESSION, I, INFERENCE> builder;
  L loss_functor;

public:
  explicit gbts_builder_impl(
    strategy<T> tree_strategy,
    I impurity_functor = I(),
    L loss_functor = L()
  ) :
    algo(tree_strategy.get_algorithm()),
    builder(
      tree_strategy
        .set_ensemble_type(ensemble_type::GradientBoosting)
        .set_algorithm(algorithm::Regression)
        .move(),
      std::move(impurity_functor)
    ),
    loss_functor(std::move(loss_functor))
  {}

  gbts_builder_impl(const gbts_builder_impl<T, I, L>& src) :
    algo(src.algo), builder(src.builder), loss_functor(src.loss_functor)
  {}

  gbts_builder_impl(gbts_builder_impl<T, I, L>&& src) :
    algo(std::move(src.algo)),
    builder(std::move(src.builder)),
    loss_functor(std::move(src.loss_functor))
  {}

  gbts_builder_impl<T, I, L>&
  operator=(const gbts_builder_impl<T, I, L>& src) {
    this->algo = src.algo;
    this->builder = src.builder;
    this->loss_functor = src.loss_functor;
    return *this;
  }

  gbts_builder_impl<T, I, L>&
  operator=(gbts_builder_impl<T, I, L>&& src) {
    this->algo = std::move(src.algo);
    this->builder = std::move(src.builder);
    this->loss_functor = std::move(src.loss_functor);
    return *this;
  }

  virtual ~gbts_builder_impl() override {}

  const strategy<T>& get_strategy() const& {
    return builder.get_strategy();
  }

  strategy<T> get_strategy() && {
    return std::move(builder.get_strategy());
  }

  virtual std::unique_ptr<gbts_builder<T>> copy_ptr() const override {
    return std::make_unique<gbts_builder_impl<T, I, L>>(*this);
  }

  virtual gradient_boosted_trees_model<T>
  build(const colmajor_matrix<T>&, dvector<T>& labels) override;

  // TODO: implement run_with_validation
};

template <typename T, typename I, typename L>
gradient_boosted_trees_model<T> gbts_builder_impl<T, I, L>::build(
  const colmajor_matrix<T>& dataset, dvector<T>& labels
) {
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
  dest[0] = builder(dataset, labels);

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
  dvector<T> predicts = std::move(builder.get_inferences());
  auto predicts_view = predicts.viewas_node_local();

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
      predicts_view, labels_view, bcas_gfunc
    );

    dest[i] = builder(dataset, negative_gradients);

    predicts_view.mapv(
      prediction_updater<T>,
      builder.get_inferences().viewas_node_local(),
      bcas_learning_rate
    );
  }

  // last tree
#ifdef _TREE_DEBUG_
  RLOG(INFO) << "==== tree[" << last_iter << "] ====" << std::endl;
#endif
  negative_gradients_view.mapv(
    negative_gradient_calculator<T, G>,
    predicts_view, labels_view, bcas_gfunc
  );
  dest[last_iter] = builder(dataset, negative_gradients);

  return gradient_boosted_trees_model<T>(
    algo, std::move(trees), std::move(weights)
  );
}

template <typename T, typename I, typename L, typename... Args>
inline std::unique_ptr<gbts_builder<T>> make_gbts_builder(Args&&... args) {
  return std::make_unique<gbts_builder_impl<T, I, L>>(
    std::forward<Args>(args)...
  );
}

} // end namespace tree

template <typename T>
class gradient_boosted_trees_builder {
  std::unique_ptr<tree::gbts_builder<T>> builder_ptr;

public:
  explicit gradient_boosted_trees_builder(tree::strategy<T>);

  gradient_boosted_trees_builder(
    const gradient_boosted_trees_builder<T>& src
  ) :
    builder_ptr(src.builder_ptr->copy_ptr())
  {}

  gradient_boosted_trees_builder(
    gradient_boosted_trees_builder<T>&& src
  ) :
    builder_ptr(std::move(src.builder_ptr))
  {}

  gradient_boosted_trees_builder<T>&
  operator=(const gradient_boosted_trees_builder<T>& src) {
    this->builder_ptr = src.builder_ptr->copy_ptr();
    return *this;
  }

  gradient_boosted_trees_builder<T>&
  operator=(gradient_boosted_trees_builder<T>&& src) {
    this->builder_ptr = std::move(src.builder_ptr);
    return *this;
  }

  // ctor from a pointer of any gbts_builder_impl<...>
  explicit gradient_boosted_trees_builder(
    std::unique_ptr<tree::gbts_builder<T>>&& ptr
  ) :
    builder_ptr(std::move(ptr))
  {}

  // ctor from an object of any gbts_builder_impl<...>
  template <typename Impurity, typename Loss>
  explicit gradient_boosted_trees_builder(
    tree::gbts_builder_impl<T, Impurity, Loss> builder
  ) :
    builder_ptr(
      tree::make_gbts_builder<T, Impurity, Loss>(std::move(builder))
    )
  {}

  gradient_boosted_trees_model<T>
  run(const colmajor_matrix<T>& dataset, dvector<T>& labels) {
    return builder_ptr->build(dataset, labels);
  }

  // TODO: implement run_with_validation
};

template <typename T>
gradient_boosted_trees_builder<T>::gradient_boosted_trees_builder(
  tree::strategy<T> strategy
) : builder_ptr() {
  using namespace tree;
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
        builder_ptr = make_gbts_builder<T, VAR, LOG>(strategy.move());
        return;
      default:
        throw std::logic_error("invalid loss type");
      }
    case impurity_type::FriedmanVariance:
      switch (strategy.loss) {
      case loss_type::Default:
      case loss_type::LogLoss:
        builder_ptr = make_gbts_builder<T, FRM, LOG>(strategy.move());
        return;
      default:
        throw std::logic_error("invalid loss type");
      }
    case impurity_type::DefVariance:
      switch (strategy.loss) {
      case loss_type::Default:
      case loss_type::LogLoss:
        builder_ptr = make_gbts_builder<T, MSE, LOG>(strategy.move());
        return;
      default:
        throw std::logic_error("invalid loss type");
      }
    case impurity_type::MeanAbsError:
      switch (strategy.loss) {
      case loss_type::Default:
      case loss_type::LogLoss:
        builder_ptr = make_gbts_builder<T, MAE, LOG>(strategy.move());
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
        builder_ptr = make_gbts_builder<T, VAR, LSE>(strategy.move());
        return;
      case loss_type::LeastAbsoluteError:
        builder_ptr = make_gbts_builder<T, VAR, LAE>(strategy.move());
        return;
      default:
        throw std::logic_error("invalid loss type");
      }
    case impurity_type::FriedmanVariance:
      switch (strategy.loss) {
      case loss_type::Default:
      case loss_type::LeastSquaresError:
        builder_ptr = make_gbts_builder<T, FRM, LSE>(strategy.move());
        return;
      case loss_type::LeastAbsoluteError:
        builder_ptr = make_gbts_builder<T, FRM, LAE>(strategy.move());
        return;
      default:
        throw std::logic_error("invalid loss type");
      }
    case impurity_type::DefVariance:
      switch (strategy.loss) {
      case loss_type::Default:
      case loss_type::LeastSquaresError:
        builder_ptr = make_gbts_builder<T, MSE, LSE>(strategy.move());
        return;
      case loss_type::LeastAbsoluteError:
        builder_ptr = make_gbts_builder<T, MSE, LAE>(strategy.move());
        return;
      default:
        throw std::logic_error("invalid loss type");
      }
    case impurity_type::MeanAbsError:
      switch (strategy.loss) {
      case loss_type::Default:
      case loss_type::LeastSquaresError:
        builder_ptr = make_gbts_builder<T, MAE, LSE>(strategy.move());
        return;
      case loss_type::LeastAbsoluteError:
        builder_ptr = make_gbts_builder<T, MAE, LAE>(strategy.move());
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
