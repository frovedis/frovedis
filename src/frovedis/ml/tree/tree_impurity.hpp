#ifndef _TREE_IMPURITY_HPP_
#define _TREE_IMPURITY_HPP_

#include <cmath>
#include <type_traits>

#include "../../../frovedis.hpp"

namespace frovedis {
namespace tree {

enum class impurity_type {
  Default,
  Gini,
  Entropy,
  Variance,
};

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
inline T square(const T value) { return value * value; }

template <typename T>
struct variance_functor {
  variance_functor() : mean(0), num_data(0) {}

#if defined(_SX) || defined(__ve__)
  variance_functor(const T mean, const T num_data) :
    mean(mean), num_data(1 / num_data)
  {}

  T operator()(const T value) const {
    return square(value - mean) * num_data;
  }
#else
  variance_functor(const T mean, const T num_data) :
    mean(mean), num_data(num_data)
  {}

  T operator()(const T value) const {
    return square(value - mean) / num_data;
  }
#endif

  T mean, num_data;
  SERIALIZE(mean, num_data)
};

// for specializing variance_functor
template <typename T, typename F>
struct is_variance {
  static constexpr bool value = std::is_same<
    variance_functor<T>, typename std::remove_cv<F>::type
  >::value;
};

template <typename T, typename F>
using enable_if_variance = enable_if_t<is_variance<T, F>::value>;
template <typename T, typename F>
using enable_if_not_variance = enable_if_t<!is_variance<T, F>::value>;

template <typename T>
struct sumsqdev_functor {
  sumsqdev_functor() : mean(0) {}
  sumsqdev_functor(const T mean) : mean(mean) {}

  T operator()(const T value) const {
    return square(value - mean);
  }

  T mean;
  SERIALIZE(mean)
};

} // end namespace tree
} // end namespace frovedis

#endif
