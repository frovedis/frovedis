#ifndef INVOKE_RESULT_HPP
#define INVOKE_RESULT_HPP

#include <type_traits>

namespace frovedis {
#if __cplusplus <= 201402L
  // if under C++17
  template <typename F, typename... Args>
  using invoke_result = std::result_of<F(Args...)>;
  template <typename F, typename... Args>
  using invoke_result_t = typename invoke_result<F, Args...>::type;
#else
  // C++17 or over
  template <typename F, typename... Args>
  using invoke_result = std::invoke_result<F, Args...>;
  template <typename F, typename... Args>
  using invoke_result_t = std::invoke_result_t<F, Args...>;
#endif
}

#endif
