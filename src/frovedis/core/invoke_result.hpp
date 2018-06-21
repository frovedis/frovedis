#ifndef INVOKE_RESULT_HPP
#define INVOKE_RESULT_HPP

#include <type_traits>

// if under C++17
#if __cplusplus <= 201402L

namespace std {
  template <typename F, typename... Args>
  using invoke_result = result_of<F(Args...)>;

  template <typename F, typename... Args>
  using invoke_result_t = typename invoke_result<F, Args...>::type;
}

#endif

#endif
