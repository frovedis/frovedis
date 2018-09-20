#ifndef _TREE_BUILDER_HPP_
#define _TREE_BUILDER_HPP_

#include <algorithm>
#include <functional>
#include <iterator>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../../../frovedis.hpp"
#include "../../matrix/rowmajor_matrix.hpp"
#include "../../matrix/colmajor_matrix.hpp"

#include "pragmas.hpp"
#include "tree_assert.hpp"
#include "tree_ftrace.hpp"
#include "tree_impurity.hpp"
#include "tree_config.hpp"
#include "tree_model.hpp"
#include "tree_worker.hpp"
#include "tree_utility.hpp"

namespace frovedis {
namespace tree {

template <typename T, typename F>
class strategy_helper : public strategy<T> {
  F ifunc;
  node_local<F> bfunc;
  node_local<strategy<T>> bstr;

public:
  strategy_helper(
    const strategy<T>& tree_strategy,
    const F& impurity_functor
  ) :
    strategy<T>(tree_strategy),
    ifunc(impurity_functor),
    bfunc(make_node_local_broadcast(ifunc)),
    bstr(make_node_local_broadcast(tree_strategy))
  {}

  F get_impurity_functor() const& { return ifunc; }
  const node_local<F>& bcas_impurity_functor() const& { return bfunc; }
  const node_local<strategy<T>>& broadcast() const& { return bstr; }

  // disable rvalue overloads
  void get_impurity_functor() && = delete;
  void bcas_impurity_functor() && = delete;
  void broadcast() && = delete;
};

// ---------------------------------------------------------------------

template <typename T>
struct classification_policy {
  using unique_labels_t = std::vector<T>;
  using current_total_t = std::vector<T>;

  static unique_labels_t make_unique_labels(const size_t num_classes) {
    std::vector<T> ret(num_classes, 0);
    T* dst = ret.data();
    for (size_t k = 0; k < num_classes; k++) { dst[k] = k; }
    return ret;
  }

  static colmajor_matrix<T> make_label_matrix(
    dvector<T>& label_vector, const unique_labels_t& unique_labels
  ) {
    colmajor_matrix<T> ret = label_vector.viewas_node_local().map(
      label_matrix_gtor<T>,
      make_node_local_broadcast(unique_labels)
    );
    ret.set_num(label_vector.size(), unique_labels.size());
    return ret;
  }

  template <typename F>
  static current_total_t calc_current_total(
    colmajor_matrix<T>& labels, const F&
  ) {
    return labels.data.map(colmajor_total<T>).vector_sum();
  }

  static predict_pair<T> calc_current_predict(
    const T current_size,
    const current_total_t& current_counter,
    const unique_labels_t& unique_labels
  ) {
    tree_assert(current_counter.size() == unique_labels.size());
    tree_assert(
      current_size == std::accumulate(
        current_counter.cbegin(), current_counter.cend(), 0
      )
    );

#if defined(_SX) || defined(__ve__)
    const size_t num_classes = current_counter.size();
    const T* src = current_counter.data();

    T max_count = 0;
    size_t max_index = 0;

    for (size_t k = 0; k < num_classes; k++) {
      if (max_count < src[k]) {
        max_count = src[k];
        max_index = k;
      }
    }
#else
    const auto max_itr = std::max_element(
      current_counter.cbegin(), current_counter.cend()
    );
    const T max_count = *max_itr;
    const size_t max_index = std::distance(
      current_counter.cbegin(), max_itr
    );
#endif
    tree_assert(max_count > 0);
    tree_assert(max_count <= current_size);
    tree_assert(max_index < unique_labels.size());

    return predict_pair<T>(
      unique_labels[max_index], max_count / current_size
    );
  }

  template <typename F>
  static T calc_current_impurity(
    const T current_size, const current_total_t& current_counter,
    const colmajor_matrix<T>&, const T probability,
    const strategy_helper<T, F>* pstr
  ) {
    if (probability == 1) { return 0; }
    return _CalcCurrentImpurity<F>::calc(
      current_size, current_counter.size(), current_counter.data(),
      pstr->get_impurity_functor()
    );
  }

  template <typename F, typename R>
  static bestgain_stats<T> find_bestgain(
    const T current_size,
    const T current_impurity,
    const current_total_t& current_counter,
    const colmajor_matrix<T>& dataset,
    const colmajor_matrix<T>& labels,
    colmajor_matrix<T>& splits,
    const size_t num_criteria,
    const node_local<splitvector<T>>& bcas_criteria,
    const strategy_helper<T, F>* pstr,
    R& rand_engine
  ) {
    colmajor_matrix_local<T> left_counter(0, 0);
    left_counter.val = splits.data.map(
      left_counter_calcr<T>,
      dataset.data, labels.data,
      bcas_criteria, pstr->broadcast()
    ).vector_sum();
    left_counter.local_num_row = num_criteria;
    left_counter.local_num_col = labels.num_col;
    tree_assert(num_criteria * labels.num_col == left_counter.val.size());

    return bestgain_finder<T>(current_size, current_impurity)(
      left_counter, current_counter,
      *pstr, pstr->get_impurity_functor(), rand_engine
    );
  }

private:
  template <typename F, typename Void = void>
  struct _CalcCurrentImpurity {
    static T calc(
      const T current_size, const size_t num_classes, const T* src,
      const F& ifunc
    ) {
      T impurity = 0;
      const T _current_size = 1 / current_size;
      for (size_t k = 0; k < num_classes; k++) {
        impurity += ifunc(src[k] * _current_size);
      }

      return impurity;
    }
  };

  template </* if F is misclassrate_functor */ typename Void>
  struct _CalcCurrentImpurity<misclassrate_functor<T>, Void> {
    static T calc(
      const T current_size, const size_t num_classes, const T* src,
      const misclassrate_functor<T>&
    ) {
      T max_count = 0;
      for (size_t k = 0; k < num_classes; k++) {
        if (max_count < src[k]) { max_count = src[k]; }
      }

      tree_assert(max_count > 0);
      return 1 - max_count / current_size;
    }
  };
};

template <typename T>
struct regression_policy {
  using unique_labels_t = std::nullptr_t;
  using current_total_t = std::pair<T, T>;

  static unique_labels_t make_unique_labels(const size_t) {
    return unique_labels_t();
  }

  static colmajor_matrix<T> make_label_matrix(
    dvector<T>& label_vector, const unique_labels_t
  ) {
    colmajor_matrix<T> ret = label_vector.viewas_node_local().map(
      label_matrix_gtor<T>
    );
    ret.set_num(label_vector.size(), 1);
    return ret;
  }

  template <typename F>
  static current_total_t calc_current_total(
    colmajor_matrix<T>& labels, const F&
  ) {
    return _CalcCurrentTotal<F>::calc(labels);
  }

  static predict_pair<T> calc_current_predict(
    const T current_size,
    const current_total_t& current_total,
    const unique_labels_t
  ) {
    return predict_pair<T>(current_total.first / current_size);
  }

  template <typename F>
  static T calc_current_impurity(
    const T current_size, const current_total_t& current_total,
    colmajor_matrix<T>& labels, const T,
    const strategy_helper<T, F>* pstr
  ) {
    tree_assert(labels.num_col == 1);
    return _CalcCurrentImpurity<F>::calc(
      current_size, current_total, labels, pstr
    );
  }

  template <typename F, typename R>
  static bestgain_stats<T> find_bestgain(
    const T current_size,
    const T current_impurity,
    const current_total_t& current_total,
    const colmajor_matrix<T>& dataset,
    const colmajor_matrix<T>& labels,
    colmajor_matrix<T>& splits,
    const size_t num_criteria,
    const node_local<splitvector<T>>& bcas_criteria,
    const strategy_helper<T, F>* pstr,
    R& rand_engine
  ) {
    tree_assert(labels.num_col == 1);

    impurity_stats<T> impurities;
    splits.data.mapv(
      regression_impurities_calcr<T>(
        impurities.get_id(), current_size,
        current_total.first, current_total.second
      ),
      dataset.data, labels.data,
      bcas_criteria, pstr->bcas_impurity_functor()
    );
    tree_assert(num_criteria == impurities.num_criteria());

    return bestgain_finder<T>(current_size, current_impurity)(
      impurities, *pstr, pstr->get_impurity_functor(), rand_engine
    );
  }

private:
  template <typename F, typename Void = void>
  struct _CalcCurrentTotal {
    static current_total_t calc(colmajor_matrix<T>& labels) {
      const T total = labels.data.map(all_total<T>).reduce(add<T>);
      return std::make_pair(total, 0);
    }
  };

  template </* if F is variance_functor */ typename Void>
  struct _CalcCurrentTotal<variance_functor<T>, Void> {
    static current_total_t calc(colmajor_matrix<T>& labels) {
      return labels.data.map(sumsquare_calcr<T>).reduce(add_pair<T, T>);
    }
  };

  template </* if F is friedmanvar_functor */ typename Void>
  struct _CalcCurrentTotal<friedmanvar_functor<T>, Void> {
    static current_total_t calc(colmajor_matrix<T>& labels) {
      return _CalcCurrentTotal<variance_functor<T>>::calc(labels);
    }
  };

  template <typename F, typename Void = void>
  struct _CalcCurrentImpurity {
    static T calc(
      const T current_size, const current_total_t& current_total,
      colmajor_matrix<T>& labels, const strategy_helper<T, F>* pstr
    ) {
      const T mean = current_total.first / current_size;
      return labels.data.map(
        regression_impurity_calcr<T, F>(mean),
        pstr->bcas_impurity_functor()
      ).reduce(add<T>) / current_size;
    }
  };

  static T calc_variance(
    const T current_size, const current_total_t& current_total
  ) {
    const T mean = current_total.first / current_size;
    const T sq_mean = current_total.second / current_size;
    return sq_mean - square(mean);
  }

  template </* if F is variance_functor */ typename Void>
  struct _CalcCurrentImpurity<variance_functor<T>, Void> {
    static T calc(
      const T current_size, const current_total_t& current_total,
      const colmajor_matrix<T>&,
      const strategy_helper<T, variance_functor<T>>*
    ) {
      return calc_variance(current_size, current_total);
    }
  };

  template </* if F is friedmanvar_functor */ typename Void>
  struct _CalcCurrentImpurity<friedmanvar_functor<T>, Void> {
    static T calc(
      const T current_size, const current_total_t& current_total,
      const colmajor_matrix<T>&,
      const strategy_helper<T, friedmanvar_functor<T>>*
    ) {
      return calc_variance(current_size, current_total);
    }
  };
};

// ---------------------------------------------------------------------

template <typename T>
inline void compress(
  dvector<size_t>& indices,
  const colmajor_matrix<T>& src_dataset,
  const colmajor_matrix<T>& src_labels,
  colmajor_matrix<T>& dest_dataset,
  colmajor_matrix<T>& dest_labels
) {
  const size_t num_indices = indices.size();
  tree_assert(src_dataset.num_row == src_labels.num_row);
  tree_assert(num_indices <= src_dataset.num_row);

  indices.viewas_node_local().mapv(
    dataset_compressor<T>,
    src_dataset.data, src_labels.data,
    dest_dataset.data, dest_labels.data
  );

  dest_dataset.set_num(num_indices, src_dataset.num_col);
  dest_labels.set_num(num_indices, src_labels.num_col);
}

// ---------------------------------------------------------------------

template <typename T, typename F, typename AP>
class builder_impl {
  const strategy_helper<T, F>* pstr;
  typename AP::unique_labels_t unique_labels;
  colmajor_matrix<T> full_dataset, full_labels;
  colmajor_matrix<T> work_dataset, work_labels, work_splits;
  std::mt19937_64 rand_engine;

public:
  builder_impl(const strategy_helper<T, F>* strategy_ptr) :
    pstr(strategy_ptr),
    unique_labels(AP::make_unique_labels(pstr->get_num_classes())),
    full_dataset(), full_labels(),
    work_dataset(), work_labels(), work_splits(),
    rand_engine(pstr->get_seed())
  {
    RLOG(TRACE) << get_typename(*this) << std::endl;
  }

  decision_tree_model<T> build(const colmajor_matrix<T>&, dvector<T>&);

private:
  std::shared_ptr<node<T>> _build(const size_t, dvector<size_t>&);
};

template <typename T, typename F, typename AP>
decision_tree_model<T> builder_impl<T, F, AP>::build(
  const colmajor_matrix<T>& dataset, dvector<T>& labels
) {
  const ftrace_region __ftr_prepare("# prepare to build");
  tree_assert(dataset.num_row == labels.size());

  constexpr bool as_view = true;
  using node_local_t = decltype(full_dataset.data);
  full_dataset.data = node_local_t(dataset.data.get_dvid(), as_view);
  full_dataset.set_num(dataset.num_row, dataset.num_col);

  using local_matrix_t = typename node_local_t::value_type;
  labels.align_as(
    full_dataset.data.map(get_num_rows<local_matrix_t>).gather()
  );
  full_labels = AP::make_label_matrix(labels, unique_labels);

  // just copy for root
  work_dataset = full_dataset;
  work_labels = full_labels;

  // set the number of columns even for empty local matrices
  work_dataset.data.mapv(
    set_num_cols<local_matrix_t>,
    make_node_local_broadcast(dataset.num_col)
  );

  dvector<size_t> initial_indices = full_dataset.data.map(
    initial_indices_gtor<local_matrix_t>
  ).template moveto_dvector<size_t>();

  size_t bytes = pstr->get_max_working_matrix_bytes();
  if (!pstr->working_matrix_is_per_process()) {
    // use given memory size with all processes
    bytes /= get_nodesize();
  }
  work_splits = make_node_local_broadcast(bytes).map(
    initial_workbench_gtor<local_matrix_t>
  );

  __ftr_prepare.end();

  return decision_tree_model<T>(
    _build(ROOT_ID, initial_indices), pstr->get_algorithm()
  );
}

template <typename T, typename F, typename AP>
std::shared_ptr<node<T>> builder_impl<T, F, AP>::_build(
  const size_t node_index, dvector<size_t>& current_indices
) {
  const nodeid_helper id(node_index);

  const size_t num_records = work_dataset.num_row;
  const size_t num_features = work_dataset.num_col;
  const T current_size = static_cast<T>(num_records);
  tree_assert(num_records == current_indices.size());

  bool leaf = (
    (id.get_depth() == pstr->get_max_depth()) ||
    (pstr->get_min_instances_per_node() == num_records)
  );

#ifdef _TREE_DEBUG_
  using local_matrix_t = typename decltype(work_dataset.data)::value_type;

  RLOG(INFO) << "---- node #" << node_index << " ----" << std::endl;
  RLOG(INFO) << "dataset dists: {" << string_join(
    work_dataset.data.map(get_num_rows<local_matrix_t>), ", "
  ) << "}" << std::endl;

  tree_assert(
    work_dataset.data.map(
      get_num_rows<local_matrix_t>
    ).gather() == work_labels.data.map(
      get_num_rows<local_matrix_t>
    ).gather()
  );
#endif

  tree_assert(id.get_depth() <= pstr->get_max_depth());
  tree_assert(0 < num_records);
  tree_assert(pstr->get_min_instances_per_node() <= num_records);
  tree_assert(num_records == work_labels.num_row);

  const ftrace_region __ftr_total("# calc current total");
  const auto current_total = AP::calc_current_total(
    work_labels, pstr->get_impurity_functor()
  );
  __ftr_total.end();

  const ftrace_region __ftr_predict("# calc current predict");
  auto current_predict = AP::calc_current_predict(
    current_size, current_total, unique_labels
  );
  __ftr_predict.end();

  const ftrace_region __ftr_impurity("# calc current impurity");
  const T current_impurity = AP::calc_current_impurity(
    current_size, current_total, work_labels,
    current_predict.get_probability(), pstr
  );
  __ftr_impurity.end();

  // quick return
  leaf = leaf || (current_predict.get_probability() == 1);
  if (leaf) {
#ifdef _TREE_DEBUG_
    RLOG(INFO) << "node #" << node_index << " is a leaf" << std::endl;
#endif
    return make_leaf<T>(
      node_index, std::move(current_predict), current_impurity
    );
  }

  // find min/max values of each feature
  const ftrace_region __ftr_minmax("# get minmax matrix");
  rowmajor_matrix_local<T> minmax_matrix(
    work_dataset.data.map(
      minmax_finder<T>, pstr->broadcast()
    ).reduce(
      minmax_reducer<T>
    )
  );
  tree_assert(minmax_matrix.local_num_row == 2);
  tree_assert(minmax_matrix.local_num_col == num_features);
  tree_assert(num_features * 2 == minmax_matrix.val.size());

  const T* mins = minmax_matrix.val.data();
  const T* maxs = mins + minmax_matrix.local_num_col;
  __ftr_minmax.end();

  const ftrace_region __ftr_criteria("# make criteria");
  size_t num_bins = pstr->get_max_bins();
_Pragma(__novector__)
  while (num_bins > num_records) { num_bins >>= 1; }

  // construct candidates of criteria
  splitvector<T> criteria;
  criteria.reserve(num_features * num_bins);
  const auto& cats_info = pstr->get_categorical_features_info();
  for (size_t j = 0; j < num_features; j++) {
    if (cats_info.count(j)) {
      const size_t temp_cats = cats_info.at(j);
      const size_t num_categories = (temp_cats == 2) ? 1 : temp_cats;
_Pragma(__novector__)
      for (size_t k = 0; k < num_categories; k++) {
        criteria.emplace_back(j, std::vector<T>(1, k));
      }
    } else {
      const T width = (maxs[j] - mins[j]) / num_bins;
_Pragma(__novector__)
      for (size_t k = 1; k < num_bins; k++) {
        criteria.emplace_back(j, mins[j] + k * width);
      }
    }
  }
  const size_t num_criteria = criteria.size();
  __ftr_criteria.end();

#ifdef _TREE_DEBUG_
  RLOG(INFO) << "number of criteria: " << num_criteria << std::endl;
#endif

  const ftrace_region __ftr_bcas_criteria("# broadcast criteria");
  auto bcas_criteria = criteria.broadcast();
  __ftr_bcas_criteria.end();

  const ftrace_region __ftr_bestgain("# find best gain");
  const auto best = AP::find_bestgain(
    current_size, current_impurity, current_total,
    work_dataset, work_labels, work_splits,
    num_criteria, bcas_criteria, pstr, rand_engine
  );
  __ftr_bestgain.end();

  // check if the reduced gain is valid or not
  leaf = leaf || (best.gain <= pstr->get_min_info_gain());

  if (leaf) {
#ifdef _TREE_DEBUG_
    RLOG(INFO) << "node #" << node_index << " is a leaf" << std::endl;
#endif
    return make_leaf<T>(
      node_index, std::move(current_predict), current_impurity
    );
  }

  const ftrace_region __ftr_split_prepare("# prepare to split");
  const size_t best_index = best.local_index;
  tree_assert(best_index < num_criteria);
  tree_assert(best.left_size + best.right_size == num_records);
  const auto criterion = criteria.get(best_index);
  __ftr_split_prepare.end();

#ifdef _TREE_DEBUG_
{
  std::ostringstream sout;
  sout << "best split: ";
  sout << "feature[" << criterion->get_feature_index() << "] ";
  if (criterion->is_categorical()) {
    const auto& categories = criterion->get_categories();
    tree_assert(!categories.empty());
    sout << "in {" << categories[0];
    for (size_t k = 1; k < categories.size(); k++) {
      sout << ", " << categories[k];
    }
    sout << "}";
  } else if (criterion->is_continuous()) {
    sout << "< " << criterion->get_threshold();
  }
  RLOG(INFO) << sout.str() << std::endl;
  RLOG(INFO) << "best gain:  " << best.gain << std::endl;
}
#endif

  // make left/right list vectors
  const ftrace_region __ftr_list_vector("# make list vectors");
  auto left_indices = make_dvector_allocate<size_t>();
  auto right_indices = make_dvector_allocate<size_t>();
  work_dataset.data.mapv(
    listvector_gtor<T>(best_index),
    current_indices.viewas_node_local(),
    left_indices.viewas_node_local(),
    right_indices.viewas_node_local(),
    bcas_criteria
  );
  left_indices.set_sizes();
  right_indices.set_sizes();
  tree_assert(left_indices.size() == best.left_size);
  tree_assert(right_indices.size() == best.right_size);
  __ftr_list_vector.end();

  // construct next nodes
  const ftrace_region __ftr_compress_left("# compress left matrices");
  compress(
    left_indices, full_dataset, full_labels, work_dataset, work_labels
  );
  __ftr_compress_left.end();
#ifdef _TREE_DEBUG_
  tree_assert(
    best.left_size == work_dataset.data.map(
      get_num_rows<local_matrix_t>
    ).reduce(add<size_t>)
  );
  tree_assert(
    best.left_size == work_labels.data.map(
      get_num_rows<local_matrix_t>
    ).reduce(add<size_t>)
  );
#endif
  const auto left_node = _build(id.get_left_child_index(), left_indices);

  const ftrace_region __ftr_compress_right("# compress right matrices");
  compress(
    right_indices, full_dataset, full_labels, work_dataset, work_labels
  );
  __ftr_compress_right.end();
#ifdef _TREE_DEBUG_
  tree_assert(
    best.right_size == work_dataset.data.map(
      get_num_rows<local_matrix_t>
    ).reduce(add<size_t>)
  );
  tree_assert(
    best.right_size == work_labels.data.map(
      get_num_rows<local_matrix_t>
    ).reduce(add<size_t>)
  );
#endif
  const auto right_node = _build(id.get_right_child_index(), right_indices);

  const auto ig_stats = make_stats<T>(
    best.gain, current_impurity,
    left_node->get_impurity(), right_node->get_impurity(),
    left_node->get_predict(), right_node->get_predict()
  );

  return make_node<T>(
    node_index, std::move(current_predict),
    current_impurity, leaf,
    criterion, left_node, right_node, ig_stats
  );
}

// ---------------------------------------------------------------------

template <typename T, typename F, typename AP>
class build_wrapper {
  strategy_helper<T, F> str;

public:
  build_wrapper(
    const strategy<T>& tree_strategy, const F& impurity_functor = F()
  ) :
    str(tree_strategy, impurity_functor)
  {}

  // TODO: support sparse matrix
  // like: template <typename DATA>
  decision_tree_model<T> operator()(
    const colmajor_matrix<T>& dataset, dvector<T>& labels
  ) const {
    builder_impl<T, F, AP> builder(&str);
    return builder.build(dataset, labels);
  }
};

template <typename T>
using build_f = std::function<
  decision_tree_model<T>(const colmajor_matrix<T>&, dvector<T>&)
>;

} // end namespace tree

// ---------------------------------------------------------------------

// a decision tree builder interface
template <typename T>
class decision_tree_builder {
  tree::build_f<T> func;

public:
  decision_tree_builder(const tree::build_f<T>& func) : func(func) {}
  decision_tree_builder(tree::build_f<T>&& func) : func(std::move(func)) {}

  decision_tree_model<T> run(
    const colmajor_matrix<T>& dataset, dvector<T>& labels
  ) const {
    return func(dataset, labels);
  }
};

template <typename T>
decision_tree_builder<T> make_decision_tree_builder(
  const tree::strategy<T>& strategy
) {
  using builder_t = decision_tree_builder<T>;
  using namespace tree;
  using GIN = gini_functor<T>;
  using ENT = entropy_functor<T>;
  using MCR = misclassrate_functor<T>;
  using VAR = variance_functor<T>;
  using FRM = friedmanvar_functor<T>;
  using MSE = defvariance_functor<T>;
  using MAE = meanabserror_functor<T>;
  using C = classification_policy<T>;
  using R = regression_policy<T>;

  switch (strategy.algo) {
  case algorithm::Classification:
    switch (strategy.impurity) {
    case impurity_type::Default:
    case impurity_type::Gini:
      return builder_t(build_wrapper<T, GIN, C>(strategy));
    case impurity_type::Entropy:
      return builder_t(build_wrapper<T, ENT, C>(strategy));
    case impurity_type::MisclassRate:
      return builder_t(build_wrapper<T, MCR, C>(strategy));
    default:
      throw std::logic_error("invalid impurity type");
    }
  case algorithm::Regression:
    switch (strategy.impurity) {
    case impurity_type::Default:
    case impurity_type::Variance:
      return builder_t(build_wrapper<T, VAR, R>(strategy));
    case impurity_type::FriedmanVariance:
      return builder_t(build_wrapper<T, FRM, R>(strategy));
    case impurity_type::DefVariance:
      return builder_t(build_wrapper<T, MSE, R>(strategy));
    case impurity_type::MeanAbsError:
      return builder_t(build_wrapper<T, MAE, R>(strategy));
    default:
      throw std::logic_error("invalid impurity type");
    }
  default:
    throw std::logic_error("no such tree algorithm");
  }
}

} // end namespace frovedis

#endif
