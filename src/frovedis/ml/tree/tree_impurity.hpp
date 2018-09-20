#ifndef _TREE_IMPURITY_HPP_
#define _TREE_IMPURITY_HPP_

#include <cmath>
#include <string>

#include "../../../frovedis.hpp"

namespace frovedis {
namespace tree {

enum class impurity_type {
  Default,
  Gini,
  Entropy,
  MisclassRate,
  Variance,
  FriedmanVariance,
  DefVariance,
  MeanAbsError,
};

impurity_type get_impurity_type(const std::string&);

template <typename T>
struct gini_functor {
  T operator()(const T freq) const {
    return freq * (1 - freq);
  }

  SERIALIZE_NONE
};

template <typename T>
struct entropy_functor {
  T operator()(const T freq) const {
#if defined(_SX) || defined(__ve__)
    return -freq * std::log(freq);
#else
    return (freq > 0) ? -freq * std::log(freq) : 0;
#endif
  }

  SERIALIZE_NONE
};

template <typename T>
struct misclassrate_functor {
  void operator()(const T) const = delete;
  SERIALIZE_NONE
};

template <typename T>
inline T square(const T value) { return value * value; }

template <typename T>
struct variance_functor {
  T operator()(const T, const T) const = delete;
  SERIALIZE_NONE
};

template <typename T>
struct friedmanvar_functor {
  T operator()(const T, const T) const = delete;
  SERIALIZE_NONE
};

template <typename T>
struct defvariance_functor {
  T operator()(const T value, const T mean) const {
    return square(value - mean);
  }

  SERIALIZE_NONE
};

template <typename T>
struct meanabserror_functor {
  T operator()(const T value, const T mean) const {
    return std::abs(value - mean);
  }

  SERIALIZE_NONE
};

} // end namespace tree
} // end namespace frovedis

#endif
