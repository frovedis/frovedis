#ifndef _TREE_WORKER_HPP_
#define _TREE_WORKER_HPP_

#include <algorithm>
#include <cstring>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../../../frovedis.hpp"
#include "../../core/make_unique.hpp"
#include "../../matrix/rowmajor_matrix.hpp"
#include "../../matrix/colmajor_matrix.hpp"
#include "../../matrix/sliced_matrix.hpp"
#include "../../matrix/blas_wrapper.hpp"

#ifdef _USE_BVMM_
#include "bvmm.hpp"
#endif
#include "pragmas.hpp"
#include "random.hpp"
#include "tree_assert.hpp"
#include "tree_config.hpp"
#include "tree_model.hpp"

namespace frovedis {
namespace tree {

template <typename MAT>
size_t get_num_rows(const MAT& x) { return x.local_num_row; }

template <typename MAT>
size_t get_num_cols(const MAT& x) { return x.local_num_col; }

template <typename MAT>
void set_num_rows(MAT& x, const size_t num_rows) {
  x.local_num_row = num_rows;
}

template <typename MAT>
void set_num_cols(MAT& x, const size_t num_cols) {
  x.local_num_col = num_cols;
}

template <typename MAT>
void set_num(MAT& x, const size_t num_rows, const size_t num_cols) {
  x.local_num_row = num_rows;
  x.local_num_col = num_cols;
}

// ---------------------------------------------------------------------

template <typename T>
inline void iota(const size_t num, T* dst) {
  for (size_t i = 0; i < num; i++) { dst[i] = i; }
}

template <typename T>
inline std::vector<T> iota(const size_t num) {
  std::vector<T> ret(num, 0);
  iota(num, ret.data());
  return ret;
}

template <typename T>
inline void iota(const size_t num, T* dst, const T value) {
  for (size_t i = 0; i < num; i++) { dst[i] = value + i; }
}

template <typename T>
inline std::vector<T> iota(const size_t num, const T value) {
  std::vector<T> ret(num, 0);
  iota(num, ret.data(), value);
  return ret;
}

// ---------------------------------------------------------------------

template <typename T>
inline void vector_sum(const T*, T*, size_t);

template <>
inline void vector_sum(
  const int* sendbuf, int* recvbuf, size_t count
) {
  MPI_Reduce(
    const_cast<int*>(sendbuf), recvbuf, count,
    MPI_INT, MPI_SUM, 0, frovedis_comm_rpc
  );
}

template <>
inline void vector_sum(
  const float* sendbuf, float* recvbuf, size_t count
) {
  MPI_Reduce(
    const_cast<float*>(sendbuf), recvbuf, count,
    MPI_FLOAT, MPI_SUM, 0, frovedis_comm_rpc
  );
}

template <>
inline void vector_sum(
  const double* sendbuf, double* recvbuf, size_t count
) {
  MPI_Reduce(
    const_cast<double*>(sendbuf), recvbuf, count,
    MPI_DOUBLE, MPI_SUM, 0, frovedis_comm_rpc
  );
}

template <typename T>
inline void vector_allsum(const T*, T*, size_t);

template <>
inline void vector_allsum(
  const int* sendbuf, int* recvbuf, size_t count
) {
  MPI_Allreduce(
    const_cast<int*>(sendbuf), recvbuf, count,
    MPI_INT, MPI_SUM, frovedis_comm_rpc
  );
}

template <>
inline void vector_allsum(
  const float* sendbuf, float* recvbuf, size_t count
) {
  MPI_Allreduce(
    const_cast<float*>(sendbuf), recvbuf, count,
    MPI_FLOAT, MPI_SUM, frovedis_comm_rpc
  );
}

template <>
inline void vector_allsum(
  const double* sendbuf, double* recvbuf, size_t count
) {
  MPI_Allreduce(
    const_cast<double*>(sendbuf), recvbuf, count,
    MPI_DOUBLE, MPI_SUM, frovedis_comm_rpc
  );
}

// ---------------------------------------------------------------------

template <typename T>
T all_total(const colmajor_matrix_local<T>& x) {
  const size_t n = x.local_num_row * x.local_num_col;
  tree_assert(n <= x.val.size());
  const T* src = x.val.data();
  T ret = 0;
  for (size_t i = 0; i < n; i++) { ret += src[i]; }
  return ret;
}

// colmajor -> colmajor total
template <typename T>
std::vector<T> colmajor_total(const colmajor_matrix_local<T>& x) {
  const size_t num_rows = x.local_num_row;
  const size_t num_cols = x.local_num_col;

  const T* src = x.val.data();
  std::vector<T> ret(num_cols, 0);
  T* dst = ret.data();

_Pragma(__outerloop_unroll__)
  for (size_t j = 0; j < num_cols; j++) {
    for (size_t i = 0; i < num_rows; i++) {
      dst[j] += src[j * num_rows + i];
    }
  }

  return ret;
}

template <typename T>
std::pair<T, T> sumsquare_calcr(const colmajor_matrix_local<T>& x) {
  const size_t n = x.local_num_row * x.local_num_col;
  tree_assert(n <= x.val.size());
  const T* src = x.val.data();
  T sum = 0;
  T sq_sum = 0;
  for (size_t i = 0; i < n; i++) {
    sum += src[i];
    sq_sum += square(src[i]);
  }
  return std::make_pair(sum, sq_sum);
}

template <typename T, typename U>
std::pair<T, U> add_pair(
  const std::pair<T, U>& p, const std::pair<T, U>& q
) {
  return std::make_pair(p.first + q.first, p.second + q.second);
}

// ---------------------------------------------------------------------

template <typename T>
inline void compress(
  const std::vector<size_t>& indices,
  const colmajor_matrix_local<T>& src_matrix,
  colmajor_matrix_local<T>& dest_matrix
) {
  const size_t num_indices = indices.size();
  const size_t num_srcrows = src_matrix.local_num_row;
  const size_t num_columns = src_matrix.local_num_col;
  tree_assert(num_columns == dest_matrix.local_num_col);
  tree_assert(num_indices * num_columns <= dest_matrix.val.size());

  const size_t* idx = indices.data();
  const T* src = src_matrix.val.data();
  T* dst = dest_matrix.val.data();

  dest_matrix.local_num_row = num_indices;

_Pragma(__outerloop_unroll__)
  for (size_t j = 0; j < num_columns; j++) {
    for (size_t i = 0; i < num_indices; i++) {
      dst[j * num_indices + i] = src[j * num_srcrows + idx[i]];
    }
  }
}

// ---------------------------------------------------------------------

template <typename T, typename Random>
Random initialize_random_engine(const strategy<T>& str) {
  std::mt19937 tmp(str.get_seed() + get_selfid());
  return Random(tmp());
}

// ---------------------------------------------------------------------

// for classification tree
template <typename T>
colmajor_matrix_local<T> label_matrix_gtor(
  const std::vector<T>& labels, const std::vector<T>& unique_labels
) {
  const size_t num_records = labels.size();
  const size_t num_uqlabels = unique_labels.size();

  const T* labels_ptr = labels.data();
  const T* uqlabels_ptr = unique_labels.data();
  colmajor_matrix_local<T> ret(num_records, num_uqlabels);
  T* dst = ret.val.data();

_Pragma(__outerloop_unroll__)
  for (size_t l = 0; l < num_uqlabels; l++) {
    for (size_t i = 0; i < num_records; i++) {
      if (labels_ptr[i] == uqlabels_ptr[l]) {
        dst[l * num_records + i] = 1;
      }
    }
  }

  return ret;
}

// for regression tree
template <typename T>
colmajor_matrix_local<T> label_matrix_gtor(
  const std::vector<T>& labels
) {
  colmajor_matrix_local<T> ret(0, 0);
  ret.val = labels;
  ret.local_num_row = labels.size();
  ret.local_num_col = 1;
  return ret;
}

// ---------------------------------------------------------------------

template <typename MAT, typename T, typename Random>
std::vector<size_t> initial_indices_gtor(
  const MAT& x, const strategy<T>& str, Random& engine
) {
  const size_t num_records = x.local_num_row;
  if (num_records == 0) { return std::vector<size_t>(); }
  if (!str.subsampling_enabled()) { return iota<size_t>(num_records); }

  const auto ss_str = str.get_subsampling_strategy();
  const size_t num_sampling = str.get_subsampling_size(num_records);
  if (ss_str == subsampling_strategy::Enable) {
    tree_assert(num_sampling < num_records);
    return sampling_without_replacement(
      iota<size_t>(num_records), num_sampling, engine
    );
  }

  tree_assert(
    (ss_str == subsampling_strategy::Auto) ||
    (ss_str == subsampling_strategy::Bootstrap)
  );

  uniform_int_distribution<size_t> dist(0, num_records - 1);
  return dist(engine, num_sampling);
}

template <typename MAT>
void initial_dataset_gtor(
  const std::vector<size_t>& indices,
  const MAT& src_dataset, const MAT& src_labels,
  MAT& dest_dataset, MAT& dest_labels
) {
  const size_t num_records = indices.size();
  tree_assert(src_dataset.local_num_row == src_labels.local_num_row);

  dest_dataset = MAT(num_records, src_dataset.local_num_col);
  dest_labels = MAT(num_records, src_labels.local_num_col);

#ifdef _TREE_DEBUG_
  for (size_t i = 0; i < num_records; i++) {
    tree_assert(indices[i] < src_dataset.local_num_row);
  }
#endif

  compress(indices, src_dataset, dest_dataset);
  compress(indices, src_labels, dest_labels);
}

inline void check_workbench(const size_t n, const size_t orig) {
  if (n == 0) {
    std::string msg("working matrix size is too small: ");
    msg += std::to_string(orig);
    msg += " bytes";
    throw std::runtime_error(msg);
  }
}

template <typename T>
inline void check_workbench(const size_t n, const std::vector<T>& v) {
  check_workbench(n, v.size() * sizeof(T));
}

template <typename MAT>
MAT initial_workbench_gtor(const size_t bytes) {
  MAT ret(0, 0);
  using T = typename decltype(ret.val)::value_type;
  ret.val = std::vector<T>(bytes / sizeof(T), 0);
  check_workbench(ret.val.size(), bytes);
  return ret;
}

// ---------------------------------------------------------------------

template <typename T>
rowmajor_matrix_local<T> minmax_finder(
  const colmajor_matrix_local<T>& x,
  const std::vector<size_t>& feature_subset,
  const strategy<T>& str
) {
  const size_t num_records = x.local_num_row;
  if (num_records == 0) { return rowmajor_matrix_local<T>(0, 0); }

  const size_t num_features = feature_subset.size();
  const size_t* subfeats = feature_subset.data();
  tree_assert(num_features <= x.local_num_col);
  rowmajor_matrix_local<T> ret(2, num_features);

  T* mindst = ret.val.data();
  T* maxdst = mindst + num_features;

  for (size_t k = 0; k < num_features; k++) {
    const size_t j = subfeats[k];
    tree_assert(j < x.local_num_col);

    if (str.categorical_features_info.count(j)) {
      continue;
    }

#if defined(_SX) || defined(__ve__)
    const T* src = x.val.data() + j * num_records;
    T min = *src;
    T max = *src;
    for (size_t i = 0; i < num_records; i++) {
      if (src[i] < min) { min = src[i]; }
      if (max < src[i]) { max = src[i]; }
    }
    mindst[k] = min;
    maxdst[k] = max;
#else
    auto begin = std::next(x.val.cbegin(), j * num_records);
    auto end = std::next(begin, num_records);
    auto minmax_itpair = std::minmax_element(begin, end);
    mindst[k] = *(minmax_itpair.first);
    maxdst[k] = *(minmax_itpair.second);
#endif
  }

  return ret;
}

template <typename T>
rowmajor_matrix_local<T> minmax_reducer(
  rowmajor_matrix_local<T>& x, rowmajor_matrix_local<T>& y
) {
  if (x.val.empty()) { return std::move(y); }
  if (y.val.empty()) { return std::move(x); }

  const size_t num_features = x.local_num_col;
  tree_assert(num_features == y.local_num_col);
  tree_assert(x.local_num_row == 2);
  tree_assert(y.local_num_row == 2);

  T* mindst = x.val.data();
  T* maxdst = mindst + num_features;
  const T* minsrc = y.val.data();
  const T* maxsrc = minsrc + num_features;

  for (size_t j = 0; j < num_features; j++) {
    if (minsrc[j] < mindst[j]) { mindst[j] = minsrc[j]; }
    if (maxdst[j] < maxsrc[j]) { maxdst[j] = maxsrc[j]; }
  }

  return std::move(x);
}

// ---------------------------------------------------------------------

// note: combinations of categories are not considered
template <typename T>
class criteria_vectors {
  size_t num_cats, num_thresh;
  std::vector<size_t> indices;
  std::vector<T> values;
  SERIALIZE(num_cats, num_thresh, indices, values)

public:
  criteria_vectors() : num_cats(0), num_thresh(0) {}

  criteria_vectors(const size_t num_categorical_criteria) :
    num_cats(num_categorical_criteria),
    num_thresh(0),
    indices(num_categorical_criteria, 0),
    values(num_categorical_criteria, 0)
  {}

  criteria_vectors(
    const criteria_vectors<T>& categorical_criteria,
    const size_t num_thresholds,
    const size_t num_continuous_criteria
  ) :
    num_cats(categorical_criteria.num_cats),
    num_thresh(num_thresholds),
    indices(num_cats + num_continuous_criteria, 0),
    values(num_cats + num_continuous_criteria, 0)
  {
    std::memcpy(
      indices.data(), categorical_criteria.indices.data(),
      sizeof(size_t) * num_cats
    );
    std::memcpy(
      values.data(), categorical_criteria.values.data(),
      sizeof(T) * num_cats
    );
  }

  criteria_vectors(
    const criteria_vectors<T>& categorical_criteria,
    const std::vector<size_t>& feature_subset,
    const size_t num_thresholds,
    const size_t num_continuous_criteria
  ) :
    num_cats(0), num_thresh(num_thresholds),
    indices(), values()
  {
    const size_t num_allcats = categorical_criteria.num_cats;
    const size_t* srcidx = categorical_criteria.index_ptr();
    const T* srcval = categorical_criteria.value_ptr();

    const size_t num_subfeats = feature_subset.size();
    const size_t* subfeats = feature_subset.data();

    std::vector<size_t> find_result(num_allcats, num_allcats);
    size_t* find = find_result.data();

    size_t i = 0;
    if (num_allcats < num_subfeats) {
      for (size_t k = 0; k < num_allcats; k++) {
        for (size_t j = 0; j < num_subfeats; j++) {
          if (subfeats[j] == srcidx[k]) { find[i++] = k; }
        }
      }
    } else {
      for (size_t j = 0; j < num_subfeats; j++) {
        for (size_t k = 0; k < num_allcats; k++) {
          if (subfeats[j] == srcidx[k]) { find[i++] = k; }
        }
      }
    }

    num_cats = i;
    tree_assert(num_cats <= num_allcats);

    const size_t result_size = num_cats + num_continuous_criteria;
    indices = std::vector<size_t>(result_size, 0);
    values = std::vector<T>(result_size, 0);

    size_t* dstidx = indices.data();
    T* dstval = values.data();;

    for (size_t k = 0; k < num_cats; k++) {
      dstidx[k] = srcidx[find[k]];
      dstval[k] = srcval[find[k]];
    }
  }

  size_t size() const { return indices.size(); }
  size_t get_num_categorical_criteria() const { return num_cats; }
  size_t get_num_thresholds() const { return num_thresh; }

  size_t get_num_continuous_criteria() const {
    return size() - get_num_categorical_criteria();
  }

  size_t* index_ptr() & { return indices.data(); }
  T* value_ptr() & { return values.data(); }
  const size_t* index_ptr() const& { return indices.data(); }
  const T* value_ptr() const& { return values.data(); }
  void index_ptr() && = delete;
  void value_ptr() && = delete;

  // construct a single split
  std::shared_ptr<split<T>> get_criterion(const size_t j) const {
    tree_assert(j < size());
    if (j < num_cats) {
      // categorical feature
      return make_split<T>(indices[j], std::vector<T>(1, values[j]));
    } else {
      // continuous feature
      return make_split<T>(indices[j], values[j]);
    }
  }
};

template <typename T>
criteria_vectors<T> make_categorical_criteria(const strategy<T>& str) {
  size_t total_cardinality = 0;
  std::map<size_t, size_t> ordered_cf;
  auto& cf_info = str.get_categorical_features_info();
_Pragma(__novector__)
  for (const auto item: cf_info) {
    const size_t cardinality = (item.second == 2) ? 1 : item.second;
    ordered_cf.emplace(item.first, cardinality);
    total_cardinality += cardinality;
  }

  criteria_vectors<T> ret(total_cardinality);
  size_t offset = 0;
_Pragma(__novector__)
  for (const auto item: ordered_cf) {
    const size_t j = item.first;
    const size_t cardinality = item.second;

    size_t* idx = ret.index_ptr() + offset;
    T* val = ret.value_ptr() + offset;
    for (size_t k = 0; k < cardinality; k++) {
      idx[k] = j;
      val[k] = k;
    }

    offset += cardinality;
  }

  tree_assert(offset == total_cardinality);
  return ret;
}

template <typename T>
inline criteria_vectors<T> prepare_criteria_vectors(
  const criteria_vectors<T>& categorical_criteria,
  const bool feature_subset_enabled,
  const std::vector<size_t>& feature_subset,
  const size_t num_thresholds,
  const size_t num_continuous_criteria
) {
  if (feature_subset_enabled) {
    return criteria_vectors<T>(
      categorical_criteria, feature_subset,
      num_thresholds, num_continuous_criteria
    );
  } else {
    return criteria_vectors<T>(
      categorical_criteria, num_thresholds, num_continuous_criteria
    );
  }
}

template <typename T>
criteria_vectors<T> make_criteria(
  const rowmajor_matrix_local<T>& minmax_matrix,
  const criteria_vectors<T>& cat_criteria,
  const std::vector<size_t>& feature_subset,
  const size_t num_bins,
  const strategy<T>& str
) {
  const size_t num_features = minmax_matrix.local_num_col;
  const T* mins = minmax_matrix.val.data();
  const T* maxs = mins + num_features;
  tree_assert(minmax_matrix.local_num_row == 2);
  tree_assert(minmax_matrix.val.size() == num_features * 2);
  tree_assert(num_features == feature_subset.size());

  std::vector<size_t> cont_indices(num_features, 0);
  size_t* srcidx = cont_indices.data();
  size_t num_contfeats = 0;
  for (size_t j = 0; j < num_features; j++) {
    if (mins[j] < maxs[j]) { srcidx[num_contfeats++] = j; }
  }

  const size_t num_thresholds = num_bins - 1;
  const size_t num_contcriteria = num_contfeats * num_thresholds;

  auto ret = prepare_criteria_vectors<T>(
    cat_criteria,
    str.feature_subset_enabled(), feature_subset,
    num_thresholds, num_contcriteria
  );

  const size_t* subfeats = feature_subset.data();
  const T _num_bins = static_cast<T>(1) / num_bins;
  const size_t offset = ret.get_num_categorical_criteria();
  size_t* dstidx = ret.index_ptr() + offset;
  T* dstval = ret.value_ptr() + offset;

  // TODO: consider max_vlen
  if (num_contfeats <= num_thresholds) {
    // note: this novector pragma suppresses partial vectorization
_Pragma(__novector__)
    for (size_t k = 0; k < num_contfeats; k++) {
      const size_t j = srcidx[k];
      const T width = (maxs[j] - mins[j]) * _num_bins;
      for (size_t t = 0; t < num_thresholds; t++) {
        dstidx[k * num_thresholds + t] = subfeats[j];
        dstval[k * num_thresholds + t] = mins[j] + (t + 1) * width;
      }
    }
  } else {
    // gather indices/values of continuous features in advance
    std::vector<size_t> gathered_indices(num_contfeats, 0);
    size_t* g_idxs = gathered_indices.data();
    std::vector<T> gathered_values(num_contfeats * 2, 0);
    T* g_mins = gathered_values.data();
    T* widths = gathered_values.data() + num_contfeats;
    for (size_t k = 0; k < num_contfeats; k++) {
      const size_t j = srcidx[k];
      g_idxs[k] = subfeats[j];
      g_mins[k] = mins[j];
      widths[k] = (maxs[j] - mins[j]) * _num_bins;
    }

#pragma _NEC notransform
    for (size_t t = 0; t < num_thresholds; t++) {
      for (size_t k = 0; k < num_contfeats; k++) {
        dstidx[k * num_thresholds + t] = g_idxs[k];
        dstval[k * num_thresholds + t] = g_mins[k] + (t + 1) * widths[k];
      }
    }
  }

  return ret;
}

// ---------------------------------------------------------------------

template <typename T>
inline void make_split_matrix(
  colmajor_matrix_local<T>& splits,
  const colmajor_matrix_local<T>& dataset,
  const criteria_vectors<T>& criteria,
  const size_t slice_start, const size_t slice_width
) {
  const size_t num_records = dataset.local_num_row;
  const size_t num_criteria = criteria.size();
  const size_t num_categorical = criteria.get_num_categorical_criteria();
  const size_t num_thresh = criteria.get_num_thresholds();
  tree_assert(num_records > 0);
  tree_assert(slice_start < num_records);
  tree_assert(slice_width > 0);
  tree_assert(slice_start <= num_records - slice_width);
  tree_assert(slice_width * num_criteria <= splits.val.size());
  tree_assert(num_categorical <= num_criteria);

  splits.local_num_row = slice_width;
  splits.local_num_col = num_criteria;

  const T* dataset_origin = dataset.val.data() + slice_start;
  T* splits_origin = splits.val.data();
  std::memset(splits_origin, 0, sizeof(T) * slice_width * num_criteria);

  const size_t* idx = criteria.index_ptr();
  const T* val = criteria.value_ptr();

  // categorical criteria
_Pragma(__novector__)
  for (size_t j = 0; j < num_categorical; j++) {
    tree_assert(idx[j] < dataset.local_num_col);
    const T category = val[j];
    const T* src = dataset_origin + idx[j] * num_records;
    T* dst = splits_origin + j * slice_width;
    for (size_t i = 0; i < slice_width; i++) {
      if (src[i] == category) { dst[i] = 1; }
    }
  }

  // continuous criteria
_Pragma(__novector__)
  for (size_t j = num_categorical; j < num_criteria; j += num_thresh) {
    const T* thresh = val + j;
    const T* src = dataset_origin + idx[j] * num_records;
    T* dst = splits_origin + j * slice_width;
    for (size_t t = 0; t < num_thresh; t++) {
      tree_assert(idx[j] == idx[j + t]);
      for (size_t i = 0; i < slice_width; i++) {
        if (src[i] < thresh[t]) { dst[t * slice_width + i] = 1; }
      }
    }
  }
}

// ---------------------------------------------------------------------

template <typename T>
inline sliced_colmajor_matrix_local<T> make_sliced_colmajor_matrix_local(
  const T* ptr, const size_t leading_dim,
  const size_t start_row, const size_t start_col,
  const size_t num_rows, const size_t num_cols
) {
  tree_assert(ptr);
  tree_assert(num_rows > 0);
  tree_assert(num_cols > 0);
  tree_assert(leading_dim > 0);
  tree_assert(num_rows <= leading_dim);

  sliced_colmajor_matrix_local<T> ret;
  ret.data = const_cast<T*>(ptr) + start_row + start_col * leading_dim;
  ret.ldm = leading_dim;
  ret.sliced_num_row = num_rows;
  ret.sliced_num_col = num_cols;
  return ret;
}

template <typename T>
inline sliced_colmajor_matrix_local<T> make_sliced_colmajor_matrix_local(
  const colmajor_matrix_local<T>& source,
  const size_t start_row, const size_t start_col,
  const size_t num_rows, const size_t num_cols
) {
  tree_assert(
    source.local_num_row * source.local_num_col <= source.val.size()
  );
  tree_assert(start_row < source.local_num_row);
  tree_assert(start_col < source.local_num_col);
  tree_assert(num_rows <= source.local_num_row - start_row);
  tree_assert(num_cols <= source.local_num_col - start_col);

  return make_sliced_colmajor_matrix_local(
    source.val.data(), source.local_num_row,
    start_row, start_col, num_rows, num_cols
  );
}

template <typename T>
inline sliced_colmajor_matrix_local<T> make_sliced_colmajor_matrix_local(
  const colmajor_matrix_local<T>& source
) {
  return make_sliced_colmajor_matrix_local(
    source, 0, 0, source.local_num_row, source.local_num_col
  );
}

// ---------------------------------------------------------------------

// Transposed-X * Slicing-Y -> Colmajor Matrix
template <typename T, enable_if_not_fp<T> = nullptr>
inline void gemm_txsy(
  const colmajor_matrix_local<T>& x,
  const colmajor_matrix_local<T>& y,
  sliced_colmajor_matrix_local<T>& sz,
  const size_t slice_start, const size_t slice_width
) {
  const size_t num_xcols = x.local_num_col;
  const size_t num_ycols = y.local_num_col;
  const size_t xlead = x.local_num_row;
  const size_t ylead = y.local_num_row;
  const size_t zlead = sz.ldm;
  tree_assert(slice_width > 0);
  tree_assert(slice_width == x.local_num_row);
  tree_assert(slice_start < y.local_num_row);
  tree_assert(slice_width <= y.local_num_row - slice_start);
  tree_assert(num_xcols > 0);
  tree_assert(num_ycols > 0);
  tree_assert(num_xcols == sz.sliced_num_row);
  tree_assert(num_ycols == sz.sliced_num_col);
  tree_assert(zlead == sz.sliced_num_row);

  const T* xp = x.val.data();
  const T* yp = y.val.data() + slice_start;
  T* zp = sz.data;

  for (size_t k = 0; k < num_ycols; k++) {
    for (size_t j = 0; j < num_xcols; j++) {
      for (size_t i = 0; i < slice_width; i++) {
        zp[k * zlead + j] += xp[j * xlead + i] * yp[k * ylead + i];
      }
    }
  }
}

// Transposed-X * Slicing-Y (BLAS)
template <typename T, enable_if_fp<T> = nullptr>
inline void gemm_txsy(
  const colmajor_matrix_local<T>& x,
  const colmajor_matrix_local<T>& y,
  sliced_colmajor_matrix_local<T>& sz,
  const size_t slice_start, const size_t slice_width
) {
  tree_assert(slice_width > 0);
  tree_assert(slice_width == x.local_num_row);
  tree_assert(slice_start < y.local_num_row);
  tree_assert(slice_width <= y.local_num_row - slice_start);
  tree_assert(x.local_num_col > 0);
  tree_assert(y.local_num_col > 0);
  const auto sx = make_sliced_colmajor_matrix_local(x);
  const auto sy = make_sliced_colmajor_matrix_local(
    y, slice_start, 0, slice_width, y.local_num_col
  );
  tree_assert(sx.sliced_num_row == sy.sliced_num_row);
  tree_assert(sx.sliced_num_col == sz.sliced_num_row);
  tree_assert(sy.sliced_num_col == sz.sliced_num_col);
  gemm<T>(sx, sy, sz, 'T', 'N', 1, 1);
}

template <typename T>
std::vector<T> left_counter_calcr(
  colmajor_matrix_local<T>& workbench,
  const colmajor_matrix_local<T>& dataset,
  const colmajor_matrix_local<T>& labels,
  const criteria_vectors<T>& criteria,
  const strategy<T>& str
) {
  const size_t num_records = dataset.local_num_row;
  tree_assert(num_records == labels.local_num_row);
  tree_assert(dataset.local_num_col > 0);
  tree_assert(labels.local_num_col > 0);
  tree_assert(criteria.size() > 0);

  if (num_records == 0) {
    return std::vector<T>(criteria.size() * labels.local_num_col, 0);
  }

  const size_t num_criteria = criteria.size();
  const size_t num_classes = labels.local_num_col;
  const size_t slice_width = workbench.val.size() / num_criteria;
  check_workbench(slice_width, workbench.val);

  std::vector<T> ret(num_criteria * num_classes, 0);
  auto sliced_ret = make_sliced_colmajor_matrix_local(
    ret.data(), num_criteria, 0, 0, num_criteria, num_classes
  );

_Pragma(__novector__)
  for (size_t i = 0; i < num_records; i += slice_width) {
    const size_t current_width = std::min(num_records - i, slice_width);
    make_split_matrix(workbench, dataset, criteria, i, current_width);
#ifdef _USE_BVMM_
    bvmm_txsy(workbench, labels, sliced_ret, i, current_width);
#else
    gemm_txsy(workbench, labels, sliced_ret, i, current_width);
#endif
  }

  return ret;
}

template <typename T, typename F>
struct classification_impurities_calcr {
  static void calc(
    const size_t, const size_t,
    const T*, const T*, const T*, const T*,
    T*, T*, F&
  );
};

template <typename T /* if F is misclassrate_functor */>
struct classification_impurities_calcr<T, misclassrate_functor<T>> {
  static void calc(
    const size_t, const size_t,
    const T*, const T*, const T*, const T*,
    T*, T*, misclassrate_functor<T>&
  );
};

template <typename T, typename F>
inline void classification_impurities_calcr<T, F>::calc(
  const size_t num_classes, const size_t num_criteria,
  const T* l_size, const T* r_size,
  const T* l_count, const T* r_count,
  T* l_impurity, T* r_impurity,
  F& ifunc
) {
  std::vector<T> inverse_sizes(num_criteria * 2, 0);
  T* _lsize = inverse_sizes.data();
  T* _rsize = _lsize + num_criteria;
  for (size_t j = 0; j < num_criteria; j++) {
    _lsize[j] = 1 / l_size[j];
    _rsize[j] = 1 / r_size[j];
  }

_Pragma(__outerloop_unroll__)
  for (size_t k = 0; k < num_classes; k++) {
    for (size_t j = 0; j < num_criteria; j++) {
      l_impurity[j] += ifunc(l_count[k * num_criteria + j] * _lsize[j]);
      r_impurity[j] += ifunc(r_count[k * num_criteria + j] * _rsize[j]);
    }
  }
}

template <typename T /* if F is misclassrate_functor */>
inline void classification_impurities_calcr<
  T, misclassrate_functor<T>
>::calc(
  const size_t num_classes, const size_t num_criteria,
  const T* l_size, const T* r_size,
  const T* l_count, const T* r_count,
  T* l_impurity, T* r_impurity,
  misclassrate_functor<T>&
) {
_Pragma(__outerloop_unroll__)
  for (size_t k = 0; k < num_classes; k++) {
    for (size_t j = 0; j < num_criteria; j++) {
      if (l_impurity[j] < l_count[k * num_criteria + j]) {
        l_impurity[j] = l_count[k * num_criteria + j];
      }
      if (r_impurity[j] < r_count[k * num_criteria + j]) {
        r_impurity[j] = r_count[k * num_criteria + j];
      }
    }
  }

  for (size_t j = 0; j < num_criteria; j++) {
    l_impurity[j] = 1 - l_impurity[j] / l_size[j];
    r_impurity[j] = 1 - r_impurity[j] / r_size[j];
  }
}

// ---------------------------------------------------------------------

template <typename T>
class impurity_stats {
  size_t nc;
  std::vector<T> val;

public:
  impurity_stats() : nc(0), val() {}
  impurity_stats(const size_t num_criteria) :
    nc(num_criteria), val(num_criteria * num_buffers, 0)
  {}

  intptr_t get_id() { return reinterpret_cast<intptr_t>(this); }

  static std::unique_ptr<impurity_stats<T>>
  get_instance(const size_t num_criteria) {
    return std::make_unique<impurity_stats<T>>(num_criteria);
  }

  static std::unique_ptr<impurity_stats<T>>
  get_instance(intptr_t id, const size_t num_criteria) {
    auto p = reinterpret_cast<impurity_stats<T>*>(id);
    *p = impurity_stats<T>(num_criteria);
    return std::unique_ptr<impurity_stats<T>>(p);
  }

  size_t num_criteria() const { return nc; }
  T* buffer0() & { return val.data(); }
  T* buffer1() & { return val.data() + nc; }
  T* buffer2() & { return val.data() + nc * 2; }
  T* buffer3() & { return val.data() + nc * 3; }
  T* buffer4() & { return val.data() + nc * 4; }
  T* buffer5() & { return val.data() + nc * 5; }
  const T* buffer0() const& { return val.data(); }
  const T* buffer1() const& { return val.data() + nc; }
  const T* buffer2() const& { return val.data() + nc * 2; }
  const T* buffer3() const& { return val.data() + nc * 3; }
  const T* buffer4() const& { return val.data() + nc * 4; }
  const T* buffer5() const& { return val.data() + nc * 5; }
  void buffer0() && = delete;
  void buffer1() && = delete;
  void buffer2() && = delete;
  void buffer3() && = delete;
  void buffer4() && = delete;
  void buffer5() && = delete;

  constexpr static size_t num_buffers = 6;
};

template <typename T>
inline void gem1v_txsv(
  const colmajor_matrix_local<T>& x,
  const colmajor_matrix_local<T>& v,
  sliced_colmajor_matrix_local<T>& sz,
  const size_t slice_start, const size_t slice_width
) {
  const size_t num_xcols = x.local_num_col;
  const size_t xlead = x.local_num_row;
  tree_assert(v.local_num_col == 1);
  tree_assert(slice_width > 0);
  tree_assert(slice_width == x.local_num_row);
  tree_assert(slice_start < v.local_num_row);
  tree_assert(slice_width <= v.local_num_row - slice_start);
  tree_assert(num_xcols > 0);
  tree_assert(sz.sliced_num_row == num_xcols);
  tree_assert(sz.sliced_num_col == 2);
  tree_assert(sz.ldm == sz.sliced_num_row);

  const T* xp = x.val.data();
  const T* vp = v.val.data() + slice_start;
  T* z1p = sz.data;
  T* zvp = z1p + sz.ldm;

_Pragma(__outerloop_unroll__)
  for (size_t j = 0; j < num_xcols; j++) {
    for (size_t i = 0; i < slice_width; i++) {
      z1p[j] += xp[j * xlead + i];
      zvp[j] += xp[j * xlead + i] * vp[i];
    }
  }
}

template <typename T, typename F>
inline void dev2mv_txsv(
  const colmajor_matrix_local<T>& x,
  const colmajor_matrix_local<T>& v,
  impurity_stats<T>& stats, F& ifunc,
  const size_t slice_start, const size_t slice_width
) {
  const size_t num_xcols = x.local_num_col;
  const size_t xlead = x.local_num_row;
  tree_assert(v.local_num_col == 1);
  tree_assert(slice_width > 0);
  tree_assert(slice_width == x.local_num_row);
  tree_assert(slice_start < v.local_num_row);
  tree_assert(slice_width <= v.local_num_row - slice_start);
  tree_assert(num_xcols > 0);
  tree_assert(num_xcols == stats.num_criteria());

  const T* xp = x.val.data();
  const T* vp = v.val.data() + slice_start;
  const T* l_mean = stats.buffer4();
  const T* r_mean = stats.buffer5();
  T* l_dev = stats.buffer1();
  T* r_dev = stats.buffer2();

_Pragma(__outerloop_unroll__)
  for (size_t j = 0; j < num_xcols; j++) {
    for (size_t i = 0; i < slice_width; i++) {
      l_dev[j] += xp[j * xlead + i] * ifunc(vp[i], l_mean[j]);
      r_dev[j] += (xp[j * xlead + i] == 0) * ifunc(vp[i], r_mean[j]);
    }
  }
}

template <typename T>
inline void gem1vv2_txsv(
  const colmajor_matrix_local<T>& x,
  const colmajor_matrix_local<T>& v,
  sliced_colmajor_matrix_local<T>& sz,
  const size_t slice_start, const size_t slice_width
) {
  const size_t num_xcols = x.local_num_col;
  const size_t xlead = x.local_num_row;
  tree_assert(v.local_num_col == 1);
  tree_assert(slice_width > 0);
  tree_assert(slice_width == x.local_num_row);
  tree_assert(slice_start < v.local_num_row);
  tree_assert(slice_width <= v.local_num_row - slice_start);
  tree_assert(num_xcols > 0);
  tree_assert(sz.sliced_num_row == num_xcols);
  tree_assert(sz.sliced_num_col == 3);
  tree_assert(sz.ldm == sz.sliced_num_row);

  const T* xp = x.val.data();
  const T* vp = v.val.data() + slice_start;
  T* z1p = sz.data;
  T* zvp = z1p + sz.ldm;
  T* zv2p = zvp + sz.ldm;

_Pragma(__outerloop_unroll__)
  for (size_t j = 0; j < num_xcols; j++) {
    for (size_t i = 0; i < slice_width; i++) {
      z1p[j] += xp[j * xlead + i];
      zvp[j] += xp[j * xlead + i] * vp[i];
      zv2p[j] += xp[j * xlead + i] * square(vp[i]);
    }
  }
}

template <typename T, typename F>
class regression_impurity_calcr {
  T mean;
  SERIALIZE(mean)

public:
  regression_impurity_calcr() {}
  regression_impurity_calcr(const T mean) : mean(mean) {}

  T operator()(const colmajor_matrix_local<T>& x, F& ifunc) const {
    const size_t n = x.local_num_row;
    tree_assert(n <= x.val.size());
    const T mu = mean;
    const T* src = x.val.data();
    T ret = 0;
    for (size_t i = 0; i < n; i++) { ret += ifunc(src[i], mu); }
    return ret;
  }
};

template <typename T>
class regression_impurities_calcr {
  intptr_t addr;
  T current_size, current_total, current_sqtotal;
  SERIALIZE(addr, current_size, current_total, current_sqtotal);

public:
  regression_impurities_calcr() {}
  regression_impurities_calcr(
    intptr_t addr, const T current_size,
    const T current_total, const T current_sqtotal
  ) :
    addr(addr), current_size(current_size),
    current_total(current_total), current_sqtotal(current_sqtotal)
  {}

  template <typename F>
  void operator()(
    colmajor_matrix_local<T>& workbench,
    const colmajor_matrix_local<T>& dataset,
    const colmajor_matrix_local<T>& labels,
    const criteria_vectors<T>& criteria, F& ifunc
  ) const {
    tree_assert(criteria.size() > 0);
    tree_assert(dataset.local_num_row == labels.local_num_row);
    tree_assert(dataset.local_num_col > 0);
    tree_assert(labels.local_num_col == 1);
    const size_t slice_width = workbench.val.size() / criteria.size();
    check_workbench(slice_width, workbench.val);

    _OperatorParen<F>::calc(
      addr, current_size, current_total, current_sqtotal, slice_width,
      workbench, dataset, labels, criteria, ifunc
    );
  }

private:
  template <typename F, typename Void = void>
  struct _OperatorParen {
    static void calc(
      intptr_t, const T, const T, const T, const size_t,
      colmajor_matrix_local<T>&,
      const colmajor_matrix_local<T>&,
      const colmajor_matrix_local<T>&,
      const criteria_vectors<T>&, F&
    );
  };

  template </* if F is variance_functor */ typename Void>
  struct _OperatorParen<variance_functor<T>, Void> {
    static void calc(
      intptr_t, const T, const T, const T, const size_t,
      colmajor_matrix_local<T>&,
      const colmajor_matrix_local<T>&,
      const colmajor_matrix_local<T>&,
      const criteria_vectors<T>&,
      variance_functor<T>&
    );
  };

  template </* if F is friedmanvar_functor */ typename Void>
  struct _OperatorParen<friedmanvar_functor<T>, Void> {
    static void calc(
      intptr_t, const T, const T, const T, const size_t,
      colmajor_matrix_local<T>&,
      const colmajor_matrix_local<T>&,
      const colmajor_matrix_local<T>&,
      const criteria_vectors<T>&,
      friedmanvar_functor<T>&
    );
  };
};

template <typename T>
template <typename F, typename Void>
inline void regression_impurities_calcr<T>::_OperatorParen<F, Void>::calc(
  intptr_t addr, const T current_size, const T current_total, const T,
  const size_t slice_width,
  colmajor_matrix_local<T>& workbench,
  const colmajor_matrix_local<T>& dataset,
  const colmajor_matrix_local<T>& labels,
  const criteria_vectors<T>& criteria, F& ifunc
) {
  constexpr size_t num_stats = 2;
  constexpr size_t num_branches = 2;
  const size_t num_records = dataset.local_num_row;
  const size_t num_criteria = criteria.size();

  // mpi-reduce buffers
  std::unique_ptr<impurity_stats<T>> stats_ptr;
  if (get_selfid() == 0) {
    // master uses prepared address of buffers
    stats_ptr = impurity_stats<T>::get_instance(addr, num_criteria);
  } else {
    // buffers for workers
    stats_ptr = impurity_stats<T>::get_instance(num_criteria);
  }

  // --- PHASE 1 --- //
  // prepare buffers of left sizes and left summations
  T* left_stats_ptr = stats_ptr->buffer4();
  auto sliced_stats = make_sliced_colmajor_matrix_local(
    left_stats_ptr, num_criteria, 0, 0, num_criteria, num_stats
  );

  // calculate left sizes and left summations
_Pragma(__novector__)
  for (size_t i = 0; i < num_records; i += slice_width) {
    const size_t current_width = std::min(num_records - i, slice_width);
    make_split_matrix(workbench, dataset, criteria, i, current_width);
    gem1v_txsv(workbench, labels, sliced_stats, i, current_width);
  }

  // all-reduce
  T* stats_src = stats_ptr->buffer4();
  T* stats_dest = stats_ptr->buffer0();
  const size_t stats_bufsize = num_criteria * num_stats;
  vector_allsum(stats_src, stats_dest, stats_bufsize);

  // calculate right sizes, left means, and right means
  const T* l_size = stats_ptr->buffer0();
  const T* l_sum = stats_ptr->buffer1();
  T* r_size = stats_ptr->buffer3();
  T* l_mean = stats_ptr->buffer4();
  T* r_mean = stats_ptr->buffer5();

  for (size_t j = 0; j < num_criteria; j++) {
    r_size[j] = current_size - l_size[j];
    l_mean[j] = l_sum[j] / l_size[j];
    r_mean[j] = (current_total - l_sum[j]) / r_size[j];
  }

  // --- PHASE 2 --- //
  // prepare buffers of left and right deviations
  T* deviation_ptr = stats_ptr->buffer1();
  const size_t deviation_bufsize = num_criteria * num_branches;
  std::memset(deviation_ptr, 0, sizeof(T) * deviation_bufsize);

  // calculate sum of squared deviations
  if (num_records > 0) {
    size_t i = (ceil_div(num_records, slice_width) - 1) * slice_width;
    tree_assert(i % slice_width == 0);
    tree_assert(i < num_records);
    tree_assert(num_records <= i + slice_width);

    // reuse the last one chunk, not reproduce
    dev2mv_txsv(workbench, labels, *stats_ptr, ifunc, i, num_records - i);

    // in reverse order
_Pragma(__novector__)
    while (i > 0) {
      i -= slice_width;
      tree_assert(slice_width < num_records - i);
      make_split_matrix(workbench, dataset, criteria, i, slice_width);
      dev2mv_txsv(workbench, labels, *stats_ptr, ifunc, i, slice_width);
    }
  }

  // reduce
  T* deviation_src = stats_ptr->buffer1();
  T* deviation_dest = stats_ptr->buffer4();
  std::memset(deviation_dest, 0, sizeof(T) * deviation_bufsize);
  vector_sum(deviation_src, deviation_dest, deviation_bufsize);

  if (get_selfid() == 0) {
    // calculate variances
    T* l_impurity = stats_ptr->buffer4();
    T* r_impurity = stats_ptr->buffer5();

    for (size_t j = 0; j < num_criteria; j++) {
      l_impurity[j] /= l_size[j];
      r_impurity[j] /= r_size[j];
    }

    // master does not reset buffers
    stats_ptr.release();
  }
}

template <typename T>
template </* if F is variance_functor */ typename Void>
inline void regression_impurities_calcr<T>::_OperatorParen<
  variance_functor<T>, Void
>::calc(
  intptr_t addr, const T current_size,
  const T current_total, const T current_sqtotal,
  const size_t slice_width,
  colmajor_matrix_local<T>& workbench,
  const colmajor_matrix_local<T>& dataset,
  const colmajor_matrix_local<T>& labels,
  const criteria_vectors<T>& criteria,
  variance_functor<T>&
) {
  constexpr size_t num_stats = 3;
  const size_t num_records = dataset.local_num_row;
  const size_t num_criteria = criteria.size();

  // mpi-reduce buffers
  std::unique_ptr<impurity_stats<T>> stats_ptr;
  if (get_selfid() == 0) {
    // master uses prepared address of buffers
    stats_ptr = impurity_stats<T>::get_instance(addr, num_criteria);
  } else {
    // buffers for workers
    stats_ptr = impurity_stats<T>::get_instance(num_criteria);
  }

  // prepare buffers of left sizes, left summations,
  // and left squared-summations
  T* left_stats_ptr = stats_ptr->buffer3();
  auto sliced_stats = make_sliced_colmajor_matrix_local(
    left_stats_ptr, num_criteria, 0, 0, num_criteria, num_stats
  );

  // calculate left sizes, left summations, and left squared-summations
_Pragma(__novector__)
  for (size_t i = 0; i < num_records; i += slice_width) {
    const size_t current_width = std::min(num_records - i, slice_width);
    make_split_matrix(workbench, dataset, criteria, i, current_width);
    gem1vv2_txsv(workbench, labels, sliced_stats, i, current_width);
  }

  // reduce
  T* stats_src = stats_ptr->buffer3();
  T* stats_dest = stats_ptr->buffer0();
  const size_t stats_bufsize = num_criteria * num_stats;
  vector_sum(stats_src, stats_dest, stats_bufsize);

  if (get_selfid() == 0) {
    // calculate variances
    const T* l_size = stats_ptr->buffer0();
    const T* l_sum = stats_ptr->buffer1();
    const T* l_sqsum = stats_ptr->buffer2();
    T* r_size = stats_ptr->buffer3();
    T* l_impurity = stats_ptr->buffer4();
    T* r_impurity = stats_ptr->buffer5();

    for (size_t j = 0; j < num_criteria; j++) {
      r_size[j] = current_size - l_size[j];
      const T l_mean = l_sum[j] / l_size[j];
      const T l_sqmean = l_sqsum[j] / l_size[j];
      const T r_mean = (current_total - l_sum[j]) / r_size[j];
      const T r_sqmean = (current_sqtotal - l_sqsum[j]) / r_size[j];
      l_impurity[j] = l_sqmean - square(l_mean);
      r_impurity[j] = r_sqmean - square(r_mean);
    }

    // master does not reset buffers
    stats_ptr.release();
  }
}

template <typename T>
template </* if F is friedmanvar_functor */ typename Void>
inline void regression_impurities_calcr<T>::_OperatorParen<
  friedmanvar_functor<T>, Void
>::calc(
  intptr_t addr, const T current_size,
  const T current_total, const T current_sqtotal,
  const size_t slice_width,
  colmajor_matrix_local<T>& workbench,
  const colmajor_matrix_local<T>& dataset,
  const colmajor_matrix_local<T>& labels,
  const criteria_vectors<T>& criteria,
  friedmanvar_functor<T>&
) {
  constexpr size_t num_stats = 2;
  const size_t num_records = dataset.local_num_row;
  const size_t num_criteria = criteria.size();

  // mpi-reduce buffers
  std::unique_ptr<impurity_stats<T>> stats_ptr;
  if (get_selfid() == 0) {
    // master uses prepared address of buffers
    stats_ptr = impurity_stats<T>::get_instance(addr, num_criteria);
  } else {
    // buffers for workers
    stats_ptr = impurity_stats<T>::get_instance(num_criteria);
  }

  // prepare buffers of left sizes and left summations
  T* left_stats_ptr = stats_ptr->buffer4();
  auto sliced_stats = make_sliced_colmajor_matrix_local(
    left_stats_ptr, num_criteria, 0, 0, num_criteria, num_stats
  );

  // calculate left sizes and left summations
_Pragma(__novector__)
  for (size_t i = 0; i < num_records; i += slice_width) {
    const size_t current_width = std::min(num_records - i, slice_width);
    make_split_matrix(workbench, dataset, criteria, i, current_width);
    gem1v_txsv(workbench, labels, sliced_stats, i, current_width);
  }

  // reduce
  T* stats_src = stats_ptr->buffer4();
  T* stats_dest = stats_ptr->buffer0();
  const size_t stats_bufsize = num_criteria * num_stats;
  vector_sum(stats_src, stats_dest, stats_bufsize);

  if (get_selfid() == 0) {
    // calculate right sizes, left means, and right means
    const T* l_size = stats_ptr->buffer0();
    const T* l_sum = stats_ptr->buffer1();
    T* r_size = stats_ptr->buffer3();
    T* l_mean = stats_ptr->buffer4();
    T* r_mean = stats_ptr->buffer5();

    for (size_t j = 0; j < num_criteria; j++) {
      r_size[j] = current_size - l_size[j];
      l_mean[j] = l_sum[j] / l_size[j];
      r_mean[j] = (current_total - l_sum[j]) / r_size[j];
    }

    // master does not reset buffers
    stats_ptr.release();
  }
}

// ---------------------------------------------------------------------

template <typename T>
struct bestgain_stats {
  bestgain_stats() {}

  bestgain_stats(const T gain, const NID rank) :
    gain(gain), rand(0), rank(rank), local_index(0),
    left_size(0), right_size(0)
  {}

  bestgain_stats(
    const T gain, const T rand,
    const NID rank, const size_t local_index,
    const size_t left_size, const size_t right_size
  ) :
    gain(gain), rand(rand), rank(rank), local_index(local_index),
    left_size(left_size), right_size(right_size)
  {}

  bool operator<(const bestgain_stats<T>& another) const {
    const auto p = std::make_pair(this->gain, this->rand);
    const auto q = std::make_pair(another.gain, another.rand);
    return p < q;
  }

  T gain, rand;
  size_t rank, local_index, left_size, right_size;
  SERIALIZE(gain, rand, rank, local_index, left_size, right_size)
};

template <typename T>
class bestgain_finder {
  T current_size, current_impurity;
  SERIALIZE(current_size, current_impurity)

public:
  bestgain_finder() {}
  bestgain_finder(const T current_size, const T current_impurity) :
    current_size(current_size), current_impurity(current_impurity)
  {}

  // for classification
  template <typename F, typename R>
  bestgain_stats<T> operator()(
    const colmajor_matrix_local<T>&,
    const std::vector<T>&, const strategy<T>&, F&, R&
  ) const;

  // for regression
  template <typename F, typename R>
  bestgain_stats<T> operator()(
    const impurity_stats<T>& impurities,
    const strategy<T>& str, F&, R& engine
  ) const {
    if (impurities.num_criteria() > 0) {
      return _OperatorParen<F, R>::find(
        current_size, current_impurity, impurities, str, engine
      );
    } else {
      return bestgain_stats<T>(str.get_min_info_gain(), get_selfid());
    }
  }

private:
  template <typename F, typename R, typename Void = void>
  struct _OperatorParen {
    static bestgain_stats<T> find(
      const T, const T, const impurity_stats<T>&, const strategy<T>&, R&
    );
  };

  template </* if F is friedmanvar_functor */ typename R, typename Void>
  struct _OperatorParen<friedmanvar_functor<T>, R, Void> {
    static bestgain_stats<T> find(
      const T, const T, const impurity_stats<T>&, const strategy<T>&, R&
    );
  };
};

template <typename T, typename F>
struct infogain_calc_helper {
  static std::vector<T> calc(
    const T, const T, const T*, const T*, const T*, const T*,
    const size_t, const size_t
  );
};

template <typename T /* if F is friedmanvar_functor */>
struct infogain_calc_helper<T, friedmanvar_functor<T>> {
  static std::vector<T> calc(
    const T, const T, const T*, const T*, const T*, const T*,
    const size_t, const size_t
  );
};

template <typename T, typename F>
inline std::vector<T> infogain_calc_helper<T, F>::calc(
  const T csize_inv, const T current_impurity,
  const T* l_size, const T* r_size,
  const T* l_impurity, const T* r_impurity,
  const size_t num_criteria, const size_t n_min
) {
  std::vector<T> ret(num_criteria, 0);
  T* dst = ret.data();
  for (size_t j = 0; j < num_criteria; j++) {
    const T lval = csize_inv * l_size[j] * l_impurity[j];
    const T rval = csize_inv * r_size[j] * r_impurity[j];
    const T nval = (n_min <= l_size[j]) && (n_min <= r_size[j]);
    dst[j] = (current_impurity - (lval + rval)) * nval;
  }
  return ret;
}

template <typename T /* if F is friedmanvar_functor */>
inline std::vector<T> infogain_calc_helper<
  T, friedmanvar_functor<T>
>::calc(
  const T, const T,
  const T* l_size, const T* r_size,
  const T* l_mean, const T* r_mean,
  const size_t num_criteria, const size_t n_min
) {
  std::vector<T> ret(num_criteria, 0);
  T* dst = ret.data();
  for (size_t j = 0; j < num_criteria; j++) {
    const T fval = l_size[j] * r_size[j] * square(l_mean[j] - r_mean[j]);
    const T nval = (n_min <= l_size[j]) && (n_min <= r_size[j]);
    dst[j] = fval * nval;
  }
  return ret;
}

template <typename T>
inline std::vector<size_t> search_indices(
  const T target, const T* vecp, const size_t n
) {
  std::vector<size_t> ret(n, 0);
  size_t* dst = ret.data();
  size_t j = 0;

  for (size_t i = 0; i < n; i++) {
    if (vecp[i] == target) { dst[j++] = i; }
  }

  tree_assert(j <= n);
  ret.resize(j, 0);
  return ret;
}

template <typename T, typename R>
inline bestgain_stats<T> bestgain_find_helper(
  const std::vector<T>& infogains,
  const T* l_size, const T* r_size,
  const strategy<T>& str, R& engine
) {
  const size_t num_criteria = infogains.size();
  const T* ig = infogains.data();

  size_t best_index = 0;
  T best_gain = str.get_min_info_gain();
  T best_rand = 0;

  for (size_t j = 0; j < num_criteria; j++) {
    if (best_gain < ig[j]) {
      best_gain = ig[j];
      best_index = j;
    }
  }

  if (str.tie_break_enabled() && (best_gain > str.get_min_info_gain())) {
    std::uniform_real_distribution<T> dist(0, 1);
#if defined(_SX) || defined(__ve__)
    auto best_indices = search_indices(best_gain, ig, num_criteria);
    const size_t* idx = best_indices.data();
    const size_t num_bests = best_indices.size();
    tree_assert(num_bests > 0);

_Pragma(__novector__)
    for (size_t j = 0; j < num_bests; j++) {
      T temp_rand = dist(engine);
      if (best_rand < temp_rand) {
        best_rand = temp_rand;
        best_index = idx[j];
      }
    }
#else
    for (size_t j = 0; j < num_criteria; j++) {
      if (ig[j] == best_gain) {
        T temp_rand = dist(engine);
        if (best_rand < temp_rand) {
          best_rand = temp_rand;
          best_index = j;
        }
      }
    }
#endif
  }

  tree_assert(best_index < num_criteria);
  return bestgain_stats<T>(
    best_gain, best_rand, get_selfid(), best_index,
    static_cast<size_t>(l_size[best_index]),
    static_cast<size_t>(r_size[best_index])
  );
}

// for classification
template <typename T>
template <typename F, typename R>
bestgain_stats<T> bestgain_finder<T>::operator()(
  const colmajor_matrix_local<T>& left_counter,
  const std::vector<T>& current_counter,
  const strategy<T>& str, F& ifunc, R& engine
) const {
  const size_t num_criteria = left_counter.local_num_row;
  if (num_criteria == 0) {
    return bestgain_stats<T>(str.get_min_info_gain(), get_selfid());
  }

  const size_t num_uqlabels = left_counter.local_num_col;
  tree_assert(num_uqlabels == current_counter.size());

  const T* l_count = left_counter.val.data();
  const T* c_count = current_counter.data();

  std::vector<T> buffer(num_criteria * (num_uqlabels + 4), 0);
  T* r_count = buffer.data();

  T* bufptr2 = buffer.data() + num_criteria * num_uqlabels;
  T* l_size = bufptr2;
  T* r_size = bufptr2 + num_criteria;
  T* l_impurity = bufptr2 + num_criteria * 2;
  T* r_impurity = bufptr2 + num_criteria * 3;

_Pragma(__outerloop_unroll__)
  for (size_t l = 0; l < num_uqlabels; l++) {
    for (size_t j = 0; j < num_criteria; j++) {
      const size_t lj = l * num_criteria + j;
      r_count[lj] = c_count[l] - l_count[lj];
    }
  }

_Pragma(__outerloop_unroll__)
  for (size_t l = 0; l < num_uqlabels; l++) {
    for (size_t j = 0; j < num_criteria; j++) {
      l_size[j] += l_count[l * num_criteria + j];
      r_size[j] += r_count[l * num_criteria + j];
    }
  }

  classification_impurities_calcr<T, F>::calc(
    num_uqlabels, num_criteria,
    l_size, r_size, l_count, r_count,
    l_impurity, r_impurity, ifunc
  );

  return bestgain_find_helper<T, R>(
    infogain_calc_helper<T, F>::calc(
      1 / current_size, current_impurity,
      l_size, r_size, l_impurity, r_impurity,
      num_criteria, str.get_min_instances_per_node()
    ),
    l_size, r_size, str, engine
  );
}

// for regression
template <typename T>
template <typename F, typename R, typename Void>
inline bestgain_stats<T>
bestgain_finder<T>::_OperatorParen<F, R, Void>::find(
  const T current_size, const T current_impurity,
  const impurity_stats<T>& impurities,
  const strategy<T>& str, R& engine
) {
  const T* l_size = impurities.buffer0();
  const T* r_size = impurities.buffer3();
  const T* l_impurity = impurities.buffer4();
  const T* r_impurity = impurities.buffer5();

  return bestgain_find_helper<T, R>(
    infogain_calc_helper<T, F>::calc(
      1 / current_size, current_impurity,
      l_size, r_size, l_impurity, r_impurity,
      impurities.num_criteria(), str.get_min_instances_per_node()
    ),
    l_size, r_size, str, engine
  );
}

template <typename T>
template </* if F is friedmanvar_functor */ typename R, typename Void>
inline bestgain_stats<T>
bestgain_finder<T>::_OperatorParen<friedmanvar_functor<T>, R, Void>::find(
  const T current_size, const T current_impurity,
  const impurity_stats<T>& impurities,
  const strategy<T>& str, R& engine
) {
  const T* l_size = impurities.buffer0();
  const T* r_size = impurities.buffer3();
  const T* l_mean = impurities.buffer4();
  const T* r_mean = impurities.buffer5();

  auto best = bestgain_find_helper<T, R>(
    infogain_calc_helper<T, friedmanvar_functor<T>>::calc(
      0, 0, l_size, r_size, l_mean, r_mean,
      impurities.num_criteria(), str.get_min_instances_per_node()
    ),
    l_size, r_size, str, engine
  );

  best.gain /= square(current_size);
  return best;
}

template <typename T>
bestgain_stats<T> bestgain_reducer(
  const bestgain_stats<T>& a, const bestgain_stats<T>& b
) {
  return std::max(a, b);
}

// ---------------------------------------------------------------------

template <typename T>
class listvector_gtor {
  size_t criterion_index;
  SERIALIZE(criterion_index)

public:
  listvector_gtor(const size_t index = 0) : criterion_index(index) {}

  void operator()(
    const colmajor_matrix_local<T>&, const std::vector<size_t>&,
    std::vector<size_t>&, std::vector<size_t>&,
    const criteria_vectors<T>&
  ) const;
};

template <typename T>
void listvector_gtor<T>::operator()(
  const colmajor_matrix_local<T>& data,
  const std::vector<size_t>& current_indices,
  std::vector<size_t>& left_indices,
  std::vector<size_t>& right_indices,
  const criteria_vectors<T>& criteria
) const {
  const size_t num_records = data.local_num_row;
  tree_assert(num_records == current_indices.size());
  tree_assert(left_indices.empty() && right_indices.empty());
  if (num_records == 0) { return; }

#if defined(_SX) || defined(__ve__)
  left_indices = std::vector<size_t>(num_records, 0);
  right_indices = std::vector<size_t>(num_records, 0);
#else
  left_indices.resize(num_records, 0);
  right_indices.resize(num_records, 0);
#endif

  const auto criterion = criteria.get_criterion(criterion_index);
  const size_t findex = criterion->get_feature_index();
  tree_assert(findex < data.local_num_col);

  const T* src = data.val.data() + findex * num_records;
  const size_t* idx = current_indices.data();
  size_t* lidx = left_indices.data();
  size_t* ridx = right_indices.data();
  size_t l = 0, r = 0;

  if (criterion->is_categorical()) {
    std::vector<int> conditions(num_records, 0);
    int* cond = conditions.data();
    for (const T category: criterion->get_categories()) {
      for (size_t i = 0; i < num_records; i++) {
        cond[i] += static_cast<int>(src[i] == category);
      }
    }
    for (size_t i = 0; i < num_records; i++) {
      if (cond[i]) { lidx[l++] = idx[i]; } else { ridx[r++] = idx[i]; }
    }
  } else if (criterion->is_continuous()) {
    const T threshold = criterion->get_threshold();
    for (size_t i = 0; i < num_records; i++) {
      if (src[i] < threshold) {
        lidx[l++] = idx[i];
      } else {
        ridx[r++] = idx[i];
      }
    }
  } else {
    throw std::logic_error("invalid feature continuity");
  }

  tree_assert(l <= num_records);
  tree_assert(r <= num_records);
  left_indices.resize(l);
  right_indices.resize(r);
}

template <typename T>
void dataset_compressor(
  const std::vector<size_t>& indices,
  const colmajor_matrix_local<T>& src_dataset,
  const colmajor_matrix_local<T>& src_labels,
  colmajor_matrix_local<T>& dest_dataset,
  colmajor_matrix_local<T>& dest_labels
) {
  tree_assert(src_dataset.local_num_row == src_labels.local_num_row);
  tree_assert(src_dataset.local_num_col == dest_dataset.local_num_col);
  tree_assert(src_labels.local_num_col == dest_labels.local_num_col);

#ifdef _TREE_DEBUG_
  for (size_t i = 0; i < indices.size(); i++) {
    tree_assert(indices[i] < src_dataset.local_num_row);
  }
#endif

  compress(indices, src_dataset, dest_dataset);
  compress(indices, src_labels, dest_labels);
}

} // end namespace tree
} // end namespace frovedis

#endif
