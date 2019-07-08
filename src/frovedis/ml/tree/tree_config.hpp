#ifndef _TREE_CONFIG_HPP_
#define _TREE_CONFIG_HPP_

#include <cmath>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

// explicit include for SX
#include "../../core/unordered_map_serialize.hpp"

#include "gbts_loss.hpp"
#include "random.hpp"
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

enum class ensemble_type {
  None,
  RandomForest,
  GradientBoosting,
};

enum class subsampling_strategy {
  Auto,
  Disable,
  Enable,
  Bootstrap,
};

enum class feature_subset_strategy {
  Auto,
  All,
  OneThird,
  Sqrt,
  Log2,
  CustomRate,
};

subsampling_strategy get_subsampling_strategy(const std::string&);
feature_subset_strategy get_feature_subset_strategy(const std::string&);

template <typename T>
struct sampling_strategy {
  sampling_strategy(
    const subsampling_strategy ss_str = subsampling_strategy::Auto,
    const T subsampling_rate = 1,
    const feature_subset_strategy fs_str = feature_subset_strategy::Auto,
    const T feature_subset_rate = 1
  ) :
    ss_str(ss_str), ss_rate(subsampling_rate),
    fs_str(fs_str), fs_rate(feature_subset_rate)
  {}

  sampling_strategy<T> move() { return std::move(*this); }
  sampling_strategy<T> submit_by_move() { return move(); }

  sampling_strategy<T>&
  set_subsampling_strategy(const subsampling_strategy ss_str) {
    this->ss_str = ss_str;
    return *this;
  }

  sampling_strategy<T>&
  set_subsampling_strategy(const std::string& str) {
    this->ss_str = get_subsampling_strategy(str);
    return *this;
  }

  sampling_strategy<T>& set_subsampling_rate(const T rate) {
    this->ss_rate = rate;
    return *this;
  }

  sampling_strategy<T>&
  set_feature_subset_strategy(const feature_subset_strategy fs_str) {
    this->fs_str = fs_str;
    return *this;
  }

  sampling_strategy<T>&
  set_feature_subset_strategy(const std::string& str) {
    this->fs_str = get_feature_subset_strategy(str);
    return *this;
  }

  sampling_strategy<T>& set_feature_subset_rate(const T rate) {
    this->fs_rate = rate;
    return *this;
  }

  subsampling_strategy
  get_subsampling_strategy() const { return ss_str; }
  T get_subsampling_rate() const { return ss_rate; }

  feature_subset_strategy
  get_feature_subset_strategy() const { return fs_str; }
  T get_feature_subset_rate() const { return fs_rate; }

  subsampling_strategy ss_str;
  T ss_rate;
  feature_subset_strategy fs_str;
  T fs_rate;

  SERIALIZE(ss_str, ss_rate, fs_str, fs_rate)
};

// to avoid a bug of optimizer
template <typename R, typename T>
#ifdef __ve__
__attribute__((noinline))
#else
inline
#endif
R explicit_cast(const T value) { return static_cast<R>(value); }

// TODO: implement validation_tol
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
    const T min_info_gain = 0,
    const size_t num_trees = 1,
    const ensemble_type ensemble = ensemble_type::None,
    const sampling_strategy<T>& sample_str = sampling_strategy<T>(),
    const loss_type loss = loss_type::Default,
    const T learning_rate = 0.1,
    //const T validation_tol = 0.001,
    const T delta = default_delta,
    const bool tie_break = false,
    const size_t max_working_matrix_megabytes = 512,
    const bool working_matrix_per_process = false,
    const size_t matmul_threshold = 256,
    const size_t seed = default_seed,
    const bool seed_from_random_device = false
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
    num_trees(num_trees),
    ensemble(ensemble),
    sample_str(sample_str),
    loss(loss),
    learning_rate(learning_rate),
    //validation_tol(validation_tol),
    delta(delta),
    tie_break(tie_break),
    max_working_matrix_megabytes(max_working_matrix_megabytes),
    working_matrix_per_process(working_matrix_per_process),
    matmul_threshold(matmul_threshold),
    seed(seed)
  {
    if (seed_from_random_device) { set_seed_from_random_device(); }
  }

  strategy<T> move() { return std::move(*this); }
  strategy<T> submit_by_move() { return move(); }

  strategy<T>& set_algorithm(const algorithm algo) {
    this->algo = algo;
    return *this;
  }

  strategy<T>& set_impurity_type(const impurity_type impurity) {
    this->impurity = impurity;
    return *this;
  }

  strategy<T>& set_impurity_type(const std::string& str) {
    this->impurity = get_impurity_type(str);
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

  strategy<T>&
  set_quantile_strategy(const quantile_strategy qstrategy) {
    this->qstrategy = qstrategy;
    return *this;
  }

  strategy<T>&
  set_categorize_strategy(const categorize_strategy cstrategy) {
    this->cstrategy = cstrategy;
    return *this;
  }

  strategy<T>& set_categorical_features_info(
    std::unordered_map<size_t, size_t> categorical_features_info
  ) {
    this->categorical_features_info = std::move(categorical_features_info);
    return *this;
  }

  strategy<T>&
  set_min_instances_per_node(const size_t min_instances_per_node) {
    this->min_instances_per_node = min_instances_per_node;
    return *this;
  }

  strategy<T>& set_min_info_gain(const T min_info_gain) {
    this->min_info_gain = min_info_gain;
    return *this;
  }

  strategy<T>& set_num_trees(const size_t num_trees) {
    this->num_trees = num_trees;
    return *this;
  }

  strategy<T>& set_num_iterations(const size_t num_iterations) {
    this->num_trees = num_iterations;
    return *this;
  }

  strategy<T>& set_ensemble_type(const ensemble_type ensemble) {
    this->ensemble = ensemble;
    return *this;
  }

  strategy<T>&
  set_sampling_strategy(const sampling_strategy<T>& sample_str) {
    this->sample_str = sample_str;
    return *this;
  }

  strategy<T>&
  set_sampling_strategy(sampling_strategy<T>&& sample_str) {
    this->sample_str = std::move(sample_str);
    return *this;
  }

  strategy<T>&
  set_subsampling_strategy(const subsampling_strategy ss_str) {
    this->sample_str.ss_str = ss_str;
    return *this;
  }

  strategy<T>& set_subsampling_strategy(const std::string& str) {
    this->sample_str.set_subsampling_strategy(str);
    return *this;
  }

  strategy<T>& set_subsampling_rate(const T subsampling_rate) {
    this->sample_str.ss_rate = subsampling_rate;
    return *this;
  }

  strategy<T>&
  set_feature_subset_strategy(const feature_subset_strategy fs_str) {
    this->sample_str.fs_str = fs_str;
    return *this;
  }

  strategy<T>& set_feature_subset_strategy(const std::string& str) {
    this->sample_str.set_feature_subset_strategy(str);
    return *this;
  }

  strategy<T>& set_feature_subset_rate(const T feature_subset_rate) {
    this->sample_str.fs_rate = feature_subset_rate;
    return *this;
  }

  strategy<T>& set_loss_type(const loss_type loss) {
    this->loss = loss;
    return *this;
  }

  strategy<T>& set_loss_type(const std::string& str) {
    this->loss = get_loss_type(str);
    return *this;
  }

  strategy<T>& set_learning_rate(const T learning_rate) {
    this->learning_rate = learning_rate;
    return *this;
  }

/*
  strategy<T>& set_validation_tol(const T validation_tol) {
    this->validation_tol = validation_tol;
    return *this;
  }
*/

  strategy<T>& set_delta(const T delta) {
    this->delta = delta;
    return *this;
  }

  strategy<T>& set_tie_break(const bool yes) {
    this->tie_break = yes;
    return *this;
  }

  strategy<T>& set_max_working_matrix_megabytes(
    const size_t max_working_matrix_megabytes
  ) {
    this->max_working_matrix_megabytes = max_working_matrix_megabytes;
    return *this;
  }

  strategy<T>& set_working_matrix_per_process(const bool yes) {
    this->working_matrix_per_process = yes;
    return *this;
  }

  strategy<T>& set_matmul_threshold(const size_t matmul_threshold) {
    this->matmul_threshold = matmul_threshold;
    return *this;
  }

  strategy<T>& set_seed(const size_t seed = default_seed) {
    this->seed = seed;
    return *this;
  }

  strategy<T>& set_seed_from_random_device() {
    this->seed = get_random_device()();
    return *this;
  }

  strategy<T>& set_seed(const size_t seed, const bool from_random_device) {
    if (from_random_device) {
      set_seed_from_random_device();
    } else {
      set_seed(seed);
    }
    return *this;
  }

  algorithm get_algorithm() const { return algo; }
  impurity_type get_impurity_type() const { return impurity; }
  size_t get_max_depth() const { return max_depth; }
  size_t get_num_classes() const { return num_classes; }
  size_t get_max_bins() const { return max_bins; }

  quantile_strategy
  get_quantile_strategy() const { return qstrategy; }
  categorize_strategy
  get_categorize_strategy() const { return cstrategy; }

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
  size_t get_num_trees() const { return num_trees; }
  size_t get_num_iterations() const { return get_num_trees(); }
  ensemble_type get_ensemble_type() const { return ensemble; }

  const sampling_strategy<T>&
  get_sampling_strategy() const& { return sample_str; }

  sampling_strategy<T>
  get_sampling_strategy() && { return std::move(sample_str); }

  subsampling_strategy
  get_subsampling_strategy() const { return sample_str.ss_str; }
  T get_subsampling_rate() const { return sample_str.ss_rate; }

  feature_subset_strategy
  get_feature_subset_strategy() const { return sample_str.fs_str; }
  T get_feature_subset_rate() const { return sample_str.fs_rate; }

  bool subsampling_enabled() const {
    switch (sample_str.ss_str) {
    case subsampling_strategy::Auto:
      return (ensemble == ensemble_type::RandomForest);
    case subsampling_strategy::Disable:
      return false;
    case subsampling_strategy::Enable:
      if ((sample_str.ss_rate <= 0) || (1 < sample_str.ss_rate)) {
        throw std::runtime_error("invalid subsampling rate");
      }
      return (sample_str.ss_rate < 1);
    case subsampling_strategy::Bootstrap:
      if (sample_str.ss_rate <= 0) {
        throw std::runtime_error("invalid subsampling rate");
      }
      return true;
    default:
      throw std::logic_error("invalid subsampling strategy");
    }
  }

  size_t get_subsampling_size(const size_t num_records) const {
    return static_cast<size_t>(sample_str.ss_rate * num_records);
  }

  bool feature_subset_enabled() const {
    switch (sample_str.fs_str) {
    case feature_subset_strategy::Auto:
      return (ensemble == ensemble_type::RandomForest);
    case feature_subset_strategy::All:
      return false;
    case feature_subset_strategy::CustomRate:
      if ((sample_str.fs_rate <= 0) || (1 < sample_str.fs_rate)) {
        throw std::runtime_error("invalid feature subset rate");
      }
      return (sample_str.fs_rate < 1);
    default:
      return true;
    }

    throw std::logic_error("SHOULDN'T REACH HERE");
  }

  size_t get_feature_subset_size(const size_t num_features) const {
    if (!feature_subset_enabled()) { return num_features; }

    constexpr T three(3);
    const T num_feats = static_cast<T>(num_features);

    // use explicit_cast to avoid a bug of optimizer
    switch (sample_str.fs_str) {
    case feature_subset_strategy::Auto:
      switch (algo) {
      case algorithm::Classification:
        return explicit_cast<size_t>(std::sqrt(num_feats));
      case algorithm::Regression:
        return explicit_cast<size_t>(num_feats / three);
      default:
        throw std::logic_error("invalid tree algorithm");
      }
      break;
    case feature_subset_strategy::OneThird:
      return explicit_cast<size_t>(num_feats / three);
    case feature_subset_strategy::Sqrt:
      return explicit_cast<size_t>(std::sqrt(num_feats));
    case feature_subset_strategy::Log2:
      return explicit_cast<size_t>(std::log2(num_feats));
    case feature_subset_strategy::CustomRate:
      return explicit_cast<size_t>(num_feats * sample_str.fs_rate);
    default:
      throw std::logic_error("invalid feature subset strategy");
    }
  }

  loss_type get_loss_type() const { return loss; }
  T get_learning_rate() const { return learning_rate; }
  //T get_validation_tol() const { return validation_tol; }
  T get_delta() const { return delta; }

  bool tie_break_enabled() const { return tie_break; }

  size_t get_max_working_matrix_megabytes() const {
    return max_working_matrix_megabytes;
  }

  size_t get_max_working_matrix_bytes() const {
    constexpr size_t mega = 1ull << 20;
    return max_working_matrix_megabytes * mega;
  }

  bool working_matrix_is_per_process() const {
    return working_matrix_per_process;
  }

  size_t get_matmul_threshold() const { return matmul_threshold; }
  size_t get_seed() const { return seed; }

  static random_device& get_random_device() {
    static random_device dev;
    return dev;
  }

  algorithm algo;
  impurity_type impurity;
  quantile_strategy qstrategy;
  categorize_strategy cstrategy;
  std::unordered_map<size_t, size_t> categorical_features_info;
  size_t max_depth, num_classes, max_bins, min_instances_per_node;
  T min_info_gain;
  size_t num_trees;
  ensemble_type ensemble;
  sampling_strategy<T> sample_str;
  loss_type loss;
  T learning_rate, /*validation_tol, */delta;
  bool tie_break;
  size_t max_working_matrix_megabytes;
  bool working_matrix_per_process;
  size_t matmul_threshold, seed;

  static constexpr T default_delta = tree::default_delta<T>();
  static constexpr size_t default_seed = mt19937::default_seed;

  SERIALIZE(
    algo, impurity, qstrategy, cstrategy,
    categorical_features_info,
    max_depth, num_classes, max_bins,
    min_instances_per_node, min_info_gain,
    num_trees, ensemble, sample_str, loss, learning_rate, /*validation_tol,*/
    delta, tie_break,
    max_working_matrix_megabytes, working_matrix_per_process,
    matmul_threshold, seed
  )
};

// a definition for static member
template <typename T>
constexpr T strategy<T>::default_delta;
template <typename T>
constexpr size_t strategy<T>::default_seed;

// shortcut functions
template <typename T>
inline strategy<T> make_classification_strategy() {
  return strategy<T>(algorithm::Classification);
}
template <typename T>
inline strategy<T> make_regression_strategy() {
  return strategy<T>(algorithm::Regression);
}

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
