#ifndef _TREE_BUILDER_HPP_
#define _TREE_BUILDER_HPP_

#include <algorithm>
#include <functional>
#include <iterator>
#include <iostream>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../../../frovedis.hpp"
#include "../../core/invoke_result.hpp"
#include "../../core/make_unique.hpp"
#include "../../core/type_utility.hpp"
#include "../../matrix/rowmajor_matrix.hpp"
#include "../../matrix/colmajor_matrix.hpp"

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
  void bcas_impurity_ifunctor() && = delete;
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

  static current_total_t calc_current_total(colmajor_matrix<T>& labels) {
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
    const colmajor_matrix<T>&, const F& ifunctor, const T probability
  ) {
    if (probability == 1) { return 0; }

    const size_t num_classes = current_counter.size();
    const T* src = current_counter.data();

    T impurity = 0;
    const T _current_size = 1 / current_size;
    for (size_t k = 0; k < num_classes; k++) {
      impurity += ifunctor(src[k] * _current_size);
    }

    return impurity;
  }

  template <typename F>
  static bestgain_stats<T> find_bestgain(
    const T current_size,
    const T current_impurity,
    const current_total_t& current_counter,
    colmajor_matrix<T>& splits,
    colmajor_matrix<T>& labels,
    const strategy_helper<T, F>* pstr
  ) {
    // get group/label-wise counters
    rowmajor_matrix_local<T> temp_counter(
      splits.num_col, labels.num_col,
      splits.data.map(
        bimm_txy<T>, labels.data, pstr->broadcast()
      ).vector_sum().data()
    );

    // re-distribute (group-parallel)
    colmajor_matrix<T> left_counter(
      make_rowmajor_matrix_scatter(temp_counter)
    );
    tree_assert(splits.num_col == left_counter.num_row);
    tree_assert(labels.num_col == left_counter.num_col);

#ifdef _TREE_DEBUG_
    RLOG(INFO) << "split dists: {" << string_join(
      left_counter.data.map(get_num_rows<colmajor_matrix_local<T>>), ", "
    ) << "}" << std::endl;
#endif

    return bestgain_stats<T>(
      left_counter.data.map(
        bestgain_finder<T>(current_size, current_impurity),
        make_node_local_broadcast(current_counter),
        pstr->broadcast(), pstr->bcas_impurity_functor()
      ).reduce(
        bestgain_reducer<T>
      )
    );
  }
};

template <typename T>
struct regression_policy {
  using unique_labels_t = std::nullptr_t;
  using current_total_t = T;

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

  static current_total_t calc_current_total(colmajor_matrix<T>& labels) {
    return labels.data.map(all_total<T>).reduce(add<T>);
  }

  static predict_pair<T> calc_current_predict(
    const T current_size,
    const current_total_t current_total,
    const unique_labels_t
  ) {
    return predict_pair<T>(current_total / current_size);
  }

  template <typename F, enable_if_variance<T, F> = nullptr>
  static T calc_current_impurity(
    const T current_size, const current_total_t current_total,
    colmajor_matrix<T>& labels, const F&, const T
  ) {
    tree_assert(labels.num_col == 1);
    return labels.data.map(
      regression_impurity_calcr<T, sumsqdev_functor<T>>(
        sumsqdev_functor<T>(current_total / current_size)
      )
    ).reduce(add<T>) / current_size;
  }

  template <typename F, enable_if_not_variance<T, F> = nullptr>
  static T calc_current_impurity(
    const T current_size, const current_total_t current_total,
    colmajor_matrix<T>& labels, const F&, const T
  ) {
    tree_assert(labels.num_col == 1);
    return labels.data.map(
      regression_impurity_calcr<T, F>(
        F(current_total / current_size, current_size)
      )
    ).reduce(add<T>);
  }

  template <typename F>
  static bestgain_stats<T> find_bestgain(
    const T current_size,
    const T current_impurity,
    const current_total_t current_total,
    colmajor_matrix<T>& splits,
    colmajor_matrix<T>& labels,
    const strategy_helper<T, F>* pstr
  ) {
    const size_t num_criteria = splits.num_col;
    tree_assert(labels.num_col == 1);

    rowmajor_matrix_local<T> label_stats(4, num_criteria);
    T* lsiz = label_stats.val.data();
    T* rsiz = lsiz + num_criteria;
    T* lavg = rsiz + num_criteria;
    T* ravg = lavg + num_criteria;
    tree_assert(ravg + num_criteria == lsiz + label_stats.val.size());

    std::vector<T> temp_sizes = splits.data.map(
      colmajor_total<T>
    ).vector_sum();
    tree_assert(temp_sizes.size() == num_criteria);

    std::vector<T> temp_totals = splits.data.map(
      gemv_txv<T>, labels.data
    ).vector_sum();
    tree_assert(temp_totals.size() == num_criteria);

    const T* tsiz = temp_sizes.data();
    const T* tsum = temp_totals.data();

    for (size_t j = 0; j < num_criteria; j++) {
      lsiz[j] = tsiz[j];
      rsiz[j] = current_size - tsiz[j];
      lavg[j] = tsum[j] / lsiz[j];
      ravg[j] = (current_total - tsum[j]) / rsiz[j];
    }

    std::vector<T> impurities = splits.data.map(
      regression_impurities_calcr<T, F>,
      labels.data, label_stats.broadcast()
    ).vector_sum();
    tree_assert(impurities.size() == num_criteria * 2);

    const size_t num_impurities = impurities.size();
    const T* isrc = impurities.data();
    T* idst = lavg;
    tree_assert(num_impurities == num_criteria * 2);

    for (size_t j = 0; j < num_impurities; j++) {
      idst[j] = isrc[j];
    }

    // re-distribute (group-parallel)
    rowmajor_matrix_local<T> transposed = label_stats.transpose();
    colmajor_matrix<T> dimpurities(
      make_rowmajor_matrix_scatter(transposed)
    );
    tree_assert(dimpurities.num_row == label_stats.local_num_col);
    tree_assert(dimpurities.num_col == label_stats.local_num_row);

#ifdef _TREE_DEBUG_
    RLOG(INFO) << "impurity dists: {" << string_join(
      dimpurities.data.map(get_num_rows<colmajor_matrix_local<T>>), ", "
    ) << "}" << std::endl;
#endif

    return bestgain_stats<T>(
      dimpurities.data.map(
        bestgain_finder<T>(current_size, current_impurity),
        pstr->broadcast()
      ).reduce(
        bestgain_reducer<T>
      )
    );
  }
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

public:
  builder_impl(const strategy_helper<T, F>* strategy_ptr) :
    pstr(strategy_ptr),
    unique_labels(AP::make_unique_labels(pstr->get_num_classes())),
    full_dataset(), full_labels(),
    work_dataset(), work_labels(), work_splits()
  {
    RLOG(TRACE) << get_typename(*this) << std::endl;
#if defined(_SX) || defined(__ve__)
    const_cast<node_local<strategy<T>>&>(
      pstr->broadcast()
    ).mapv(random_initializer<T>);
#endif
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
    +[] (colmajor_matrix_local<T>& x, const size_t num_cols) {
      tree_assert(x.val.size() == x.local_num_row * num_cols);
      x.local_num_col = num_cols;
    },
    make_node_local_broadcast(dataset.num_col)
  );

  dvector<size_t> initial_indices = full_dataset.data.map(
    +[] (const colmajor_matrix_local<T>& x) -> std::vector<size_t> {
      const size_t num_records = x.local_num_row;
      std::vector<size_t> ret(num_records, 0);
      size_t* dst = ret.data();
      for (size_t i = 0; i < num_records; i++) { dst[i] = i; }
      return ret;
    }
  ).template moveto_dvector<size_t>();

  const auto& cats_info = pstr->get_categorical_features_info();
  const size_t num_categorical = cats_info.size();
  const size_t num_continuous = full_dataset.num_col - num_categorical;
  size_t max_criteria = num_continuous * pstr->get_max_bins();
  for (const auto kv: cats_info) {
    const size_t cardinality = kv.second;
    max_criteria += (cardinality == 2) ? 1 : cardinality;
  }

  work_splits = full_dataset.data.map(
    +[] (
      const colmajor_matrix_local<T>& x, const size_t max_criteria
    ) -> colmajor_matrix_local<T> {
      return colmajor_matrix_local<T>(x.local_num_row, max_criteria);
    },
    make_node_local_broadcast(max_criteria)
  );
  work_splits.set_num(full_dataset.num_row, max_criteria);

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
  const auto current_total = AP::calc_current_total(work_labels);
  __ftr_total.end();

  const ftrace_region __ftr_predict("# calc current predict");
  auto current_predict = AP::calc_current_predict(
    current_size, current_total, unique_labels
  );
  __ftr_predict.end();

  const ftrace_region __ftr_impurity("# calc current impurity");
  const T current_impurity = AP::calc_current_impurity(
    current_size, current_total, work_labels,
    pstr->get_impurity_functor(), current_predict.get_probability()
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
#pragma cdir novector
#pragma _NEC novector
  while (num_bins > num_records) { num_bins >>= 1; }

  // construct candidates of criteria
  splitvector<T> criteria;
  criteria.reserve(num_features * num_bins);
  const auto& cats_info = pstr->get_categorical_features_info();
  for (size_t j = 0; j < num_features; j++) {
    if (cats_info.count(j)) {
      const size_t temp_cats = cats_info.at(j);
      const size_t num_categories = (temp_cats == 2) ? 1 : temp_cats;
      for (size_t k = 0; k < num_categories; k++) {
        criteria.emplace_back(j, std::vector<T>(1, k));
      }
    } else {
      const T width = (maxs[j] - mins[j]) / num_bins;
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

  const ftrace_region __ftr_split_matrix("# make split matrix");
  work_splits.data.mapv(
    split_matrix_gtor<T>, work_dataset.data, bcas_criteria
  );
  work_splits.set_num(num_records, num_criteria);
  __ftr_split_matrix.end();

  const ftrace_region __ftr_bestgain("# find best gain");
  const auto best = AP::find_bestgain(
    current_size, current_impurity, current_total,
    work_splits, work_labels, pstr
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
  const std::vector<size_t> block_sizes(get_block_sizes(num_criteria));
  tree_assert(best.rank < block_sizes.size());
  tree_assert(block_sizes.size() == get_nodesize());

#if defined(_SX) || defined(__ve__)
  size_t best_index = best.local_index;
  const size_t rank = best.rank;
  const size_t* bs = block_sizes.data();
  for (size_t r = 0; r < rank; r++) {
    best_index += bs[r];
  }
#else
  const auto bs_head = block_sizes.cbegin();
  const auto bs_tail = std::next(bs_head, best.rank);
  const size_t best_index = std::accumulate(
    bs_head, bs_tail, best.local_index
  );
#endif
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
    sout << "in {";
    if (!categories.empty()) {
      sout << categories.front();
      for (size_t k = 1; k < categories.size(); k++) {
        sout << ", " << categories[k];
      }
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
using builder_t = std::function<
  decision_tree_model<T>(const colmajor_matrix<T>&, dvector<T>&)
>;

} // end namespace tree

// ---------------------------------------------------------------------

// a decision tree builder interface
template <typename T>
class decision_tree_builder {
  tree::builder_t<T> build_functor;

public:
  decision_tree_builder(tree::builder_t<T> build_functor) :
    build_functor(std::move(build_functor))
  {}

  decision_tree_model<T> run(
    const colmajor_matrix<T>& dataset, dvector<T>& labels
  ) const {
    return build_functor(dataset, labels);
  }
};

template <typename T>
decision_tree_builder<T> make_decision_tree_builder(
  const tree::strategy<T>& strategy
) {
  switch (strategy.algo) {
  case tree::algorithm::Classification:
    switch (strategy.impurity) {
    case tree::impurity_type::Default:
    case tree::impurity_type::Gini:
      return decision_tree_builder<T>(
        tree::build_wrapper<
          T, tree::gini_functor<T>, tree::classification_policy<T>
        >(strategy)
      );
    case tree::impurity_type::Entropy:
      return decision_tree_builder<T>(
        tree::build_wrapper<
          T, tree::entropy_functor<T>, tree::classification_policy<T>
        >(strategy)
      );
    default:
      throw std::logic_error("invalid impurity type");
    }
  case tree::algorithm::Regression:
    switch (strategy.impurity) {
    case tree::impurity_type::Default:
    case tree::impurity_type::Variance:
      return decision_tree_builder<T>(
        tree::build_wrapper<
          T, tree::variance_functor<T>, tree::regression_policy<T>
        >(strategy)
      );
    default:
      throw std::logic_error("invalid impurity type");
    }
  default:
    throw std::logic_error("no such tree algorithm");
  }
}

} // end namespace frovedis

#endif
