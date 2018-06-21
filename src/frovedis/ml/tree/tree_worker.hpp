#ifndef _TREE_WORKER_HPP_
#define _TREE_WORKER_HPP_

#include <algorithm>
#include <cstring>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../../../frovedis.hpp"
#include "../../core/type_utility.hpp"
#include "../../matrix/rowmajor_matrix.hpp"
#include "../../matrix/colmajor_matrix.hpp"
#include "../../matrix/sliced_matrix.hpp"
#include "../../matrix/blas_wrapper.hpp"

#include "tree_assert.hpp"
#include "tree_config.hpp"
#include "tree_vector.hpp"
#include "tree_random.hpp"

namespace frovedis {
namespace tree {

template <typename MATRIX_LOCAL>
size_t get_num_rows(const MATRIX_LOCAL& x) { return x.local_num_row; }

template <typename MATRIX_LOCAL>
size_t get_num_cols(const MATRIX_LOCAL& x) { return x.local_num_col; }

// ---------------------------------------------------------------------

template <typename T>
std::vector<T> increment(
  const colmajor_matrix_local<T>& splits,
  const colmajor_matrix_local<T>& labels
) {
  const size_t num_records = splits.local_num_row;
  const size_t num_criteria = splits.local_num_col;
  const size_t num_uqlabels = labels.local_num_col;
  tree_assert(num_records == labels.local_num_row);
  tree_assert(num_criteria > 0);
  tree_assert(num_uqlabels > 0);

  const T* splits_ptr = splits.val.data();
  const T* labels_ptr = labels.val.data();

  std::vector<T> ret(num_criteria * num_uqlabels, 0);
  T* dst = ret.data();
  for (size_t j = 0; j < num_criteria; j++) {
    for (size_t l = 0; l < num_uqlabels; l++) {
      for (size_t i = 0; i < num_records; i++) {
        if (
          (splits_ptr[j * num_records + i] != 0) &&
          (labels_ptr[l * num_records + i] != 0)
        ) {
          dst[j * num_uqlabels + l] += 1;
        }
      }
    }
  }

  return ret;
}

// ---------------------------------------------------------------------

// Transposed-X * V -> Vector
template <typename T, enable_if_not_fp<T> = nullptr>
std::vector<T> gemv_txv(
  const colmajor_matrix_local<T>& x,
  const colmajor_matrix_local<T>& v
) {
  const size_t num_records = x.local_num_row;
  const size_t num_attrs = x.local_num_col;
  tree_assert(num_records == v.local_num_row);
  tree_assert(num_attrs > 0);
  tree_assert(v.local_num_col == 1);

  const T* xp = x.val.data();
  const T* vp = v.val.data();

  std::vector<T> ret(num_attrs, 0);
  T* dst = ret.data();
  for (size_t j = 0; j < num_attrs; j++) {
    for (size_t i = 0; i < num_records; i++) {
      dst[j] += xp[j * num_records + i] * vp[i];
    }
  }

  return ret;
}

// Transposed-X * V (BLAS)
template <typename T, enable_if_fp<T> = nullptr>
std::vector<T> gemv_txv(
  const colmajor_matrix_local<T>& x,
  const colmajor_matrix_local<T>& v
) {
  tree_assert(x.local_num_row == v.local_num_row);
  tree_assert(x.local_num_col > 0);
  tree_assert(v.local_num_col == 1);

  if (x.local_num_row == 0) {
    return std::vector<T>(x.local_num_col, 0);
  }

  const sliced_colmajor_matrix_local<T> sx(x);
  const sliced_colmajor_vector_local<T> sv(v);
  tree_assert(sx.is_valid() && sv.is_valid());
  tree_assert(sx.sliced_num_row == sv.size);
  std::vector<T> ret(sx.sliced_num_col, 0);
  gemv<T>(sx, sv, ret, 'T', 1.0, 0.0);
  return ret;
}

// ---------------------------------------------------------------------

// Transposed-X * Y -> Rowmajor Vector
template <typename T, enable_if_not_fp<T> = nullptr>
std::vector<T> gemm_txy(
  const colmajor_matrix_local<T>& x,
  const colmajor_matrix_local<T>& y
) {
  const size_t num_records = x.local_num_row;
  const size_t num_xattrs = x.local_num_col;
  const size_t num_yattrs = y.local_num_col;
  tree_assert(num_records == y.local_num_row);
  tree_assert(num_xattrs > 0);
  tree_assert(num_yattrs > 0);

  const T* xp = x.val.data();
  const T* yp = y.val.data();

  std::vector<T> ret(num_xattrs * num_yattrs, 0);
  T* dst = ret.data();
  for (size_t j = 0; j < num_xattrs; j++) {
    for (size_t l = 0; l < num_yattrs; l++) {
      for (size_t i = 0; i < num_records; i++) {
        dst[j * num_yattrs + l] += xp[j * num_records + i] * yp[l * num_records + i];
      }
    }
  }

  return ret;
}

// Transposed-X * Y (BLAS)
template <typename T, enable_if_fp<T> = nullptr>
std::vector<T> gemm_txy(
  const colmajor_matrix_local<T>& x,
  const colmajor_matrix_local<T>& y
) {
  tree_assert(x.local_num_row == y.local_num_row);
  tree_assert(x.local_num_col > 0);
  tree_assert(y.local_num_col > 0);

  if (x.local_num_row == 0) {
    return std::vector<T>(x.local_num_col * y.local_num_col, 0);
  }

  // note: transpose(TX * Y) == TY * X
  const sliced_colmajor_matrix_local<T> sx(x);
  const sliced_colmajor_matrix_local<T> sy(y);
  tree_assert(sx.is_valid() && sy.is_valid());
  tree_assert(sx.sliced_num_row == sy.sliced_num_row);
  colmajor_matrix_local<T> ret(sy.sliced_num_col, sx.sliced_num_col);
  gemm<T>(sy, sx, ret, 'T', 'N', 1.0, 0.0);
  return ret.val;
}

template <typename T, typename U = void>
std::vector<T> bimm_txy(
  const colmajor_matrix_local<T>& x,
  const colmajor_matrix_local<T>& y,
  const strategy<T>& str
) {
  tree_assert(x.local_num_row == y.local_num_row);
  return gemm_txy<T>(x, y);
  // TODO: implement bvmm_txy
/*
  if (x.local_num_row < str.matmul_threshold) {
    return gemm_txy<T>(x, y);
  } else {
    return bvmm_txy<T, U>(x, y);
  }
*/
}

// ---------------------------------------------------------------------

template <typename T>
T all_total(const colmajor_matrix_local<T>& x) {
  const size_t n = x.local_num_row * x.local_num_col;
  tree_assert(n <= x.val.size());
  const T* src = x.val.data();
  T ret = 0;
#pragma cdir nodep
#pragma _NEC ivdep
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

  for (size_t j = 0; j < num_cols; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for (size_t i = 0; i < num_rows; i++) {
      dst[j] += src[j * num_rows + i];
    }
  }

  return ret;
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

  for (size_t l = 0; l < num_uqlabels; l++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for (size_t i = 0; i < num_records; i++) {
      dst[l * num_records + i] = static_cast<T>(
        labels_ptr[i] == uqlabels_ptr[l]
      );
    }
  }

  return ret;
}

// for regression tree
template <typename T>
colmajor_matrix_local<T> label_matrix_gtor(
  const std::vector<T>& labels
) {
  rowmajor_matrix_local<T> temp(labels);
  tree_assert(temp.local_num_row == labels.size());
  tree_assert(temp.local_num_col == 1);
  return colmajor_matrix_local<T>(std::move(temp));
}

// ---------------------------------------------------------------------

template <typename T>
void split_matrix_gtor(
  colmajor_matrix_local<T>& splits,
  const colmajor_matrix_local<T>& data,
  const splitvector<T>& criteria
) {
  const size_t num_records = data.local_num_row;
  const size_t num_criteria = criteria.size();
  tree_assert(num_records * num_criteria <= splits.val.size());

  splits.local_num_row = num_records;
  splits.local_num_col = num_criteria;
  if (num_records == 0) { return; }

  const T* data_origin = data.val.data();
  T* split_origin = splits.val.data();
  std::memset(split_origin, 0, sizeof(T) * num_records * num_criteria);

  for (size_t j = 0; j < num_criteria; j++) {
    const size_t findex = criteria.get_feature_index(j);
    tree_assert(findex < data.local_num_col);

    const T* src = data_origin + findex * num_records;
    T* dst = split_origin + j * num_records;

    if (criteria.is_categorical(j)) {
      for (const T category: criteria.get_categories(j)) {
#pragma cdir nodep
#pragma _NEC ivdep
        for (size_t i = 0; i < num_records; i++) {
          dst[i] = static_cast<T>(dst[i] || src[i] == category);
        }
      }
    } else if (criteria.is_continuous(j)) {
      const T threshold = criteria.get_threshold(j);
#pragma cdir nodep
#pragma _NEC ivdep
      for (size_t i = 0; i < num_records; i++) {
        dst[i] = static_cast<T>(src[i] < threshold);
      }
    } else {
      throw std::logic_error("no such feature continuity");
    }
  }
}

// ---------------------------------------------------------------------

template <typename T>
rowmajor_matrix_local<T> minmax_finder(
  const colmajor_matrix_local<T>& x,
  const strategy<T>& str
) {
  const size_t num_records = x.local_num_row;
  if (num_records == 0) { return rowmajor_matrix_local<T>(0, 0); }

  const size_t num_features = x.local_num_col;
  rowmajor_matrix_local<T> ret(2, num_features);

  T* mindst = ret.val.data();
  T* maxdst = mindst + num_features;

  for (size_t j = 0; j < num_features; j++) {
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
    mindst[j] = min;
    maxdst[j] = max;
#else
    auto begin = std::next(x.val.cbegin(), j * num_records);
    auto end = std::next(begin, num_records);
    auto minmax_itpair = std::minmax_element(begin, end);
    mindst[j] = *(minmax_itpair.first);
    maxdst[j] = *(minmax_itpair.second);
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

#pragma cdir nodep
#pragma _NEC ivdep
  for (size_t j = 0; j < num_features; j++) {
    if (minsrc[j] < mindst[j]) { mindst[j] = minsrc[j]; }
    if (maxdst[j] < maxsrc[j]) { maxdst[j] = maxsrc[j]; }
  }

  return std::move(x);
}

// ---------------------------------------------------------------------

template <typename T, typename F>
class regression_impurity_calcr {
  F ifunc;
  SERIALIZE(ifunc)

public:
  regression_impurity_calcr() {}
  regression_impurity_calcr(const F& ifunc) : ifunc(ifunc) {}

  T operator()(const colmajor_matrix_local<T>& x) const {
    const size_t n = x.val.size();
    const T* src = x.val.data();
    T ret = 0;
#pragma cdir nodep
#pragma _NEC ivdep
    for (size_t i = 0; i < n; i++) { ret += ifunc(src[i]); }
    return ret;
  }
};

template <typename T, typename F, enable_if_variance<T, F> = nullptr>
std::vector<T> regression_impurities_calcr(
  const colmajor_matrix_local<T>& splits,
  const colmajor_matrix_local<T>& labels,
  const rowmajor_matrix_local<T>& label_stats
) {
  const size_t num_records = splits.local_num_row;
  const size_t num_criteria = splits.local_num_col;
  tree_assert(num_records == labels.local_num_row);
  tree_assert(num_criteria > 0);
  tree_assert(labels.local_num_col == 1);
  tree_assert(label_stats.local_num_row == 4);
  tree_assert(num_criteria == label_stats.local_num_col);

  const T* lsiz = label_stats.val.data();
  const T* rsiz = lsiz + num_criteria;
  const T* lavg = rsiz + num_criteria;
  const T* ravg = lavg + num_criteria;
  tree_assert(ravg + num_criteria == lsiz + label_stats.val.size());

  std::vector<T> impurities(num_criteria * 2, 0);
  T* limp = impurities.data();
  T* rimp = limp + num_criteria;
  tree_assert(rimp + num_criteria == limp + impurities.size());

  const T* sp = splits.val.data();
  const T* lp = labels.val.data();

  for (size_t j = 0; j < num_criteria; j++) {
    const sumsqdev_functor<T> lfn(lavg[j]), rfn(ravg[j]);
    for (size_t i = 0; i < num_records; i++) {
      limp[j] += sp[j * num_records + i] * lfn(lp[i]);
      rimp[j] += static_cast<T>(sp[j * num_records + i] == 0) * rfn(lp[i]);
    }
    limp[j] /= lsiz[j];
    rimp[j] /= rsiz[j];
  }

  return impurities;
}

template <typename T, typename F, enable_if_not_variance<T, F> = nullptr>
std::vector<T> regression_impurities_calcr(
  const colmajor_matrix_local<T>& splits,
  const colmajor_matrix_local<T>& labels,
  const rowmajor_matrix_local<T>& label_stats
) {
  const size_t num_records = splits.local_num_row;
  const size_t num_criteria = splits.local_num_col;
  tree_assert(num_records == labels.local_num_row);
  tree_assert(num_criteria > 0);
  tree_assert(labels.local_num_col == 1);
  tree_assert(label_stats.local_num_row == 4);
  tree_assert(num_criteria == label_stats.local_num_col);

  const T* lsiz = label_stats.val.data();
  const T* rsiz = lsiz + num_criteria;
  const T* lavg = rsiz + num_criteria;
  const T* ravg = lavg + num_criteria;
  tree_assert(ravg + num_criteria == lsiz + label_stats.val.size());

  std::vector<T> impurities(num_criteria * 2, 0);
  T* limp = impurities.data();
  T* rimp = limp + num_criteria;
  tree_assert(rimp + num_criteria == limp + impurities.size());

  const T* sp = splits.val.data();
  const T* lp = labels.val.data();

  for (size_t j = 0; j < num_criteria; j++) {
    F lfn(lavg[j], lsiz[j]), rfn(ravg[j], rsiz[j]);
    for (size_t i = 0; i < num_records; i++) {
      limp[j] += sp[j * num_records + i] * lfn(lp[i]);
      rimp[j] += static_cast<T>(sp[j * num_records + i] == 0) * rfn(lp[i]);
    }
  }

  return impurities;
}

// ---------------------------------------------------------------------

#if defined(_SX) || defined(__ve__)
template <typename T>
void random_initializer(const strategy<T>& str) {
  init_random(str.seed + get_selfid());
}
#endif

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
  T parent_size, parent_impurity;
  SERIALIZE(parent_size, parent_impurity)

public:
  bestgain_finder() {}
  bestgain_finder(const T parent_size, const T parent_impurity) :
    parent_size(parent_size), parent_impurity(parent_impurity)
  {}

  // for classification
  template <typename F>
  bestgain_stats<T> operator()(
    const colmajor_matrix_local<T>&,
    const std::vector<T>&, const strategy<T>&, const F&
  ) const;

  // for regression
  bestgain_stats<T> operator()(
    const colmajor_matrix_local<T>&, const strategy<T>&
  ) const;
};

template <typename T>
inline bestgain_stats<T> bestgain_find_helper(
  const T parent_size, const T parent_impurity,
  const size_t num_criteria,
  const T* left_sizes, const T* right_sizes,
  const T* left_impurities, const T* right_impurities,
  const strategy<T>& str
) {
  const T _psiz = 1 / parent_size;
  const size_t imin = str.min_instances_per_node;

  std::vector<T> infogains(num_criteria, parent_impurity);
  T* ig = infogains.data();
#pragma cdir nodep
#pragma _NEC ivdep
  for (size_t j = 0; j < num_criteria; j++) {
    const T left_impurity = left_sizes[j] * _psiz * left_impurities[j];
    const T right_impurity = right_sizes[j] * _psiz * right_impurities[j];
    ig[j] -= left_impurity + right_impurity;
    ig[j] *= static_cast<T>(
      (imin <= left_sizes[j]) && (imin <= right_sizes[j])
    );
  }

  const size_t randomize = str.random_enabled();
#if defined(_SX) || defined(__ve__)
  T best_gain = str.min_info_gain;
  size_t best_index = 0;
  for (size_t j = 0; j < num_criteria; j++) {
    if (best_gain < ig[j]) {
      best_gain = ig[j];
      best_index = j;
    }
  }
  T best_rand = 0;
#else
  static std::mt19937_64 rand_engine(str.seed + get_selfid());
  static std::uniform_real_distribution<T> dist(0, 1);

  size_t best_index = 0;
  T best_gain = str.min_info_gain;
  T temp_rand = randomize * dist(rand_engine);
  T best_rand = 0;

  for (size_t j = 0; j < num_criteria; j++) {
    if (
      (best_gain < ig[j]) ||
      (
        (randomize) && (best_gain == ig[j]) &&
        (best_rand < (temp_rand = dist(rand_engine)))
      )
    ) {
      best_gain = ig[j];
      best_rand = temp_rand;
      best_index = j;
    }
  }
#endif

  return bestgain_stats<T>(
    best_gain, best_rand, get_selfid(), best_index,
    static_cast<size_t>(left_sizes[best_index]),
    static_cast<size_t>(right_sizes[best_index])
  );
}

// for classification
template <typename T>
template <typename F>
bestgain_stats<T> bestgain_finder<T>::operator()(
  const colmajor_matrix_local<T>& left_counter,
  const std::vector<T>& parent_counter,
  const strategy<T>& str, const F& ifunc
) const {
  const size_t num_criteria = left_counter.local_num_row;
  if (num_criteria == 0) {
    return bestgain_stats<T>(str.min_info_gain, get_selfid());
  }

  const size_t num_uqlabels = left_counter.local_num_col;
  tree_assert(num_uqlabels == parent_counter.size());

  const T* lcnt = left_counter.val.data();
  const T* pcnt = parent_counter.data();

  std::vector<T> right_counter(num_criteria * num_uqlabels, 0);
  T* rcnt = right_counter.data();
  for (size_t l = 0; l < num_uqlabels; l++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for (size_t j = 0; j < num_criteria; j++) {
      rcnt[l * num_criteria + j] = pcnt[l] - lcnt[l * num_criteria + j];
    }
  }

  std::vector<T> left_sizes(num_criteria, 0);
  std::vector<T> right_sizes(num_criteria, 0);
  T* lsiz = left_sizes.data();
  T* rsiz = right_sizes.data();
  for (size_t l = 0; l < num_uqlabels; l++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for (size_t j = 0; j < num_criteria; j++) {
      lsiz[j] += lcnt[l * num_criteria + j];
      rsiz[j] += rcnt[l * num_criteria + j];
    }
  }

  std::vector<T> left_sizes_inv(left_sizes);
  std::vector<T> right_sizes_inv(right_sizes);
  T* _lsiz = left_sizes_inv.data();
  T* _rsiz = right_sizes_inv.data();
#pragma cdir nodep
#pragma _NEC ivdep
  for (size_t j = 0; j < num_criteria; j++) {
    _lsiz[j] = 1 / _lsiz[j];
    _rsiz[j] = 1 / _rsiz[j];
  }

  std::vector<T> left_impurities(num_criteria, 0);
  std::vector<T> right_impurities(num_criteria, 0);
  T* limp = left_impurities.data();
  T* rimp = right_impurities.data();
  for (size_t l = 0; l < num_uqlabels; l++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for (size_t j = 0; j < num_criteria; j++) {
      limp[j] += ifunc(lcnt[l * num_criteria + j] * _lsiz[j]);
      rimp[j] += ifunc(rcnt[l * num_criteria + j] * _rsiz[j]);
    }
  }

  return bestgain_find_helper<T>(
    parent_size, parent_impurity, num_criteria,
    lsiz, rsiz, limp, rimp, str
  );
}

// for regression
template <typename T>
bestgain_stats<T> bestgain_finder<T>::operator()(
  const colmajor_matrix_local<T>& impurities,
  const strategy<T>& str
) const {
  const size_t num_criteria = impurities.local_num_row;
  if (num_criteria == 0) {
    return bestgain_stats<T>(str.min_info_gain, get_selfid());
  }

  tree_assert(impurities.local_num_col == 4);

  const T* lsiz = impurities.val.data();
  const T* rsiz = lsiz + num_criteria;
  const T* limp = rsiz + num_criteria;
  const T* rimp = limp + num_criteria;
  tree_assert(rimp + num_criteria == lsiz + impurities.val.size());

  return bestgain_find_helper<T>(
    parent_size, parent_impurity, num_criteria,
    lsiz, rsiz, limp, rimp, str
  );
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
    const splitvector<T>&
  ) const;
};

template <typename T>
void listvector_gtor<T>::operator()(
  const colmajor_matrix_local<T>& data,
  const std::vector<size_t>& current_indices,
  std::vector<size_t>& left_indices,
  std::vector<size_t>& right_indices,
  const splitvector<T>& criteria
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

  const auto criterion = criteria.get(criterion_index);
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
    throw std::logic_error("no such feature continuity");
  }

  tree_assert(l <= num_records);
  tree_assert(r <= num_records);
  left_indices.resize(l);
  right_indices.resize(r);
}

template <typename T>
inline void compress_helper(
  const std::vector<size_t>& indices,
  const colmajor_matrix_local<T>& src_matrix,
  colmajor_matrix_local<T>& dest_matrix
) {
  const size_t num_indices = indices.size();
  const size_t num_srcrows = src_matrix.local_num_row;
  const size_t num_columns = dest_matrix.local_num_col;
  tree_assert(num_srcrows * num_columns == dest_matrix.val.size());

  const size_t* idx = indices.data();
  const T* src = src_matrix.val.data();
  T* dst = dest_matrix.val.data();

  dest_matrix.local_num_row = num_indices;

  for (size_t j = 0; j < num_columns; j++) {
    for (size_t i = 0; i < num_indices; i++) {
      dst[j * num_indices + i] = src[j * num_srcrows + idx[i]];
    }
  }
}

template <typename T>
void dataset_compressor(
  const std::vector<size_t>& indices,
  const colmajor_matrix_local<T>& src_dataset,
  const colmajor_matrix_local<T>& src_labels,
  colmajor_matrix_local<T>& dest_dataset,
  colmajor_matrix_local<T>& dest_labels
) {
  tree_assert(indices.size() <= src_dataset.local_num_row);
  tree_assert(src_dataset.local_num_row == src_labels.local_num_row);

#ifdef _TREE_DEBUG_
  for (size_t i = 0; i < indices.size(); i++) {
    tree_assert(indices[i] < src_dataset.local_num_row);
  }
#endif

  compress_helper(indices, src_dataset, dest_dataset);
  compress_helper(indices, src_labels, dest_labels);
}

} // end namespace tree
} // end namespace frovedis

#endif
