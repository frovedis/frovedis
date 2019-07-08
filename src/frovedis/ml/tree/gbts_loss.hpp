#ifndef _GBTS_LOSS_HPP_
#define _GBTS_LOSS_HPP_

#include <cmath>
#include <string>

#include "../../../frovedis.hpp"

#include "tree_impurity.hpp"

namespace frovedis {
namespace tree {

enum class loss_type {
  Default,
  LogLoss,
  LeastSquaresError,
  LeastAbsoluteError,
};

loss_type get_loss_type(const std::string&);

template <typename T>
struct logloss_functor {
  T operator()(const T prediction, const T label) const {
    return 2 * std::log(1 + std::exp(-2 * label * prediction));
  }

  T gradient(const T prediction, const T label) const {
    return -4 * label / (1 + std::exp(2 * label * prediction));
  }

  SERIALIZE_NONE
};

template <typename T>
struct leastsqerror_functor {
  T operator()(const T prediction, const T label) const {
    return square(label - prediction);
  }

  T gradient(const T prediction, const T label) const {
    return -2 * (label - prediction);
  }

  SERIALIZE_NONE
};

template <typename T>
struct leastabserror_functor {
  T operator()(const T prediction, const T label) const {
    return absval(label - prediction);
  }

  T gradient(const T prediction, const T label) const {
    return (label - prediction < 0) ? 1 : -1;
  }

  SERIALIZE_NONE
};

// ---------------------------------------------------------------------

template <typename T>
constexpr T default_delta() {
#ifdef DEFAULT_DELTA
  return DEFAULT_DELTA;
#else
  return T(1) / T(1ull << 10);
#endif
}

template <typename T, typename F>
class has_gradient_func {
  template <typename G>
  static auto check(G g) -> decltype(
    g.gradient(std::declval<T>(), std::declval<T>()), std::true_type()
  );
  static auto check(...) -> decltype(std::false_type());

public:
  using type = decltype(check(std::declval<F>()));
  static constexpr bool value = type::value;
};

template <typename T>
struct always_false : public std::false_type {};
template <typename T, typename F>
using enable_if_gradfunc_t = enable_if_t<has_gradient_func<T, F>::value>;
template <typename T, typename F>
using enable_if_gradcalc_t = enable_if_t<!has_gradient_func<T, F>::value>;

template <typename T, typename F, typename = enable_if_t<true>>
class gradient_functor {
  static_assert(always_false<T>::value, "SFINAE failed");
};

template <typename T, typename F>
class gradient_functor<T, F, enable_if_gradfunc_t<T, F>> {
  F f;
  SERIALIZE(f)

public:
  gradient_functor(const T = 0) : f(F()) {}
  gradient_functor(const F& f, const T = 0) : f(f) {}

  T operator()(const T prediction, const T label) {
    return f.gradient(prediction, label);
  }
};

template <typename T, typename F>
class gradient_functor<T, F, enable_if_gradcalc_t<T, F>> {
  F f;
  T h, inv;
  SERIALIZE(f, h, inv)

public:
  gradient_functor(const T delta = default_delta<T>()) :
    gradient_functor(F(), delta)
  {}

  gradient_functor(const F& f, const T delta = default_delta<T>()) :
    f(f), h(delta), inv(1 / (2 * delta))
  {}

  T operator()(const T prediction, const T label) {
    return (f(prediction + h, label) - f(prediction - h, label)) * inv;
  }
};

} // end namespace tree
} // end namespace frovedis

#endif
