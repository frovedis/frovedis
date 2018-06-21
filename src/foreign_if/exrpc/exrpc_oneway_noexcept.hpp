#ifndef EXRPC_ONEWAY_NOEXCEPT_HPP
#define EXRPC_ONEWAY_NOEXCEPT_HPP

#include <string>
#include <sstream>

#include "exrpc.hpp"
#include "exrpc_result.hpp"
#include "exrpc_oneway.hpp"

namespace frovedis {

#define exrpc_oneway_noexcept(n, f, ...) exrpc_function_oneway_noexcept(n, #f, f, __VA_ARGS__)

#define exrpc_oneway_noexcept0(n, f) exrpc_function_oneway_noexcept(n, #f, f)

void send_exrpcreq_oneway_noexcept(exrpc_node& n, const std::string&,
                                   const std::string&);

template <class T1, class T2, class T3, class T4, class T5,
          class T6, class T7, class T8, class T9, class T10, class T11>
void exrpc_function_oneway_noexcept(exrpc_node& n, std::string fn,
                                    void(*f)(T1&,T2&,T3&,T4&,T5&,T6&,T7&,T8&,
                                             T9&,T10&,T11&),
                                    const T1& a1, const T2& a2, const T3& a3,
                                    const T4& a4, const T5& a5, const T6& a6,
                                    const T7& a7, const T8& a8, const T9& a9,
                                    const T10& a10, const T11& a11) {
  std::ostringstream outss;
  my_portable_oarchive outar(outss);
  outar << a1;
  outar << a2;
  outar << a3;
  outar << a4;
  outar << a5;
  outar << a6;
  outar << a7;
  outar << a8;
  outar << a9;
  outar << a10;
  outar << a11;
  send_exrpcreq_oneway_noexcept(n, fn, outss.str());
}

template <class T1, class T2, class T3, class T4, class T5, 
          class T6, class T7, class T8, class T9, class T10>
void exrpc_function_oneway_noexcept(exrpc_node& n, std::string fn,
                                    void(*f)(T1&,T2&,T3&,T4&,T5&,T6&,T7&,T8&,
                                             T9&,T10&),
                                    const T1& a1, const T2& a2, const T3& a3,
                                    const T4& a4, const T5& a5, const T6& a6,
                                    const T7& a7, const T8& a8, const T9& a9,
                                    const T10& a10) {
  std::ostringstream outss;
  my_portable_oarchive outar(outss);
  outar << a1;
  outar << a2;
  outar << a3;
  outar << a4;
  outar << a5;
  outar << a6;
  outar << a7;
  outar << a8;
  outar << a9;
  outar << a10;
  send_exrpcreq_oneway_noexcept(n, fn, outss.str());
}

template <class T1, class T2, class T3, class T4, class T5,
          class T6, class T7, class T8, class T9>
void exrpc_function_oneway_noexcept(exrpc_node& n, std::string fn,
                                    void(*f)(T1&,T2&,T3&,T4&,T5&,T6&,T7&,T8&,
                                             T9&),
                                    const T1& a1, const T2& a2, const T3& a3,
                                    const T4& a4, const T5& a5, const T6& a6,
                                    const T7& a7, const T8& a8, const T9& a9) {
  std::ostringstream outss;
  my_portable_oarchive outar(outss);
  outar << a1;
  outar << a2;
  outar << a3;
  outar << a4;
  outar << a5;
  outar << a6;
  outar << a7;
  outar << a8;
  outar << a9;
  send_exrpcreq_oneway_noexcept(n, fn, outss.str());
}

template <class T1, class T2, class T3, class T4, class T5, 
          class T6, class T7, class T8>
void exrpc_function_oneway_noexcept(exrpc_node& n, std::string fn,
                                    void(*f)(T1&,T2&,T3&,T4&,T5&,T6&,T7&,T8&),
                                    const T1& a1, const T2& a2, const T3& a3,
                                    const T4& a4, const T5& a5, const T6& a6,
                                    const T7& a7, const T8& a8) {
  std::ostringstream outss;
  my_portable_oarchive outar(outss);
  outar << a1;
  outar << a2;
  outar << a3;
  outar << a4;
  outar << a5;
  outar << a6;
  outar << a7;
  outar << a8;
  send_exrpcreq_oneway_noexcept(n, fn, outss.str());
}

template <class T1, class T2, class T3, class T4, class T5,
          class T6, class T7>
void exrpc_function_oneway_noexcept(exrpc_node& n, std::string fn,
                                    void(*f)(T1&,T2&,T3&,T4&,T5&,T6&,T7&),
                                    const T1& a1, const T2& a2, const T3& a3,
                                    const T4& a4, const T5& a5, const T6& a6,
                                    const T7& a7) {
  std::ostringstream outss;
  my_portable_oarchive outar(outss);
  outar << a1;
  outar << a2;
  outar << a3;
  outar << a4;
  outar << a5;
  outar << a6;
  outar << a7;
  send_exrpcreq_oneway_noexcept(n, fn, outss.str());
}

template <class T1, class T2, class T3, class T4, class T5,
          class T6>
void exrpc_function_oneway_noexcept(exrpc_node& n, std::string fn,
                                    void(*f)(T1&,T2&,T3&,T4&,T5&,T6&),
                                    const T1& a1, const T2& a2, const T3& a3,
                                    const T4& a4, const T5& a5, const T6& a6) {
  std::ostringstream outss;
  my_portable_oarchive outar(outss);
  outar << a1;
  outar << a2;
  outar << a3;
  outar << a4;
  outar << a5;
  outar << a6;
  send_exrpcreq_oneway_noexcept(n, fn, outss.str());
}

template <class T1, class T2, class T3, class T4, class T5>
void exrpc_function_oneway_noexcept(exrpc_node& n, std::string fn,
                                    void(*f)(T1&,T2&,T3&,T4&,T5&),
                                    const T1& a1, const T2& a2, const T3& a3,
                                    const T4& a4, const T5& a5) {
  std::ostringstream outss;
  my_portable_oarchive outar(outss);
  outar << a1;
  outar << a2;
  outar << a3;
  outar << a4;
  outar << a5;
  send_exrpcreq_oneway_noexcept(n, fn, outss.str());
}

template <class T1, class T2, class T3, class T4>
void exrpc_function_oneway_noexcept(exrpc_node& n, std::string fn,
                                    void(*f)(T1&,T2&,T3&,T4&),
                                    const T1& a1, const T2& a2, const T3& a3,
                                    const T4& a4) {
  std::ostringstream outss;
  my_portable_oarchive outar(outss);
  outar << a1;
  outar << a2;
  outar << a3;
  outar << a4;
  send_exrpcreq_oneway_noexcept(n, fn, outss.str());
}

template <class T1, class T2, class T3>
void exrpc_function_oneway_noexcept(exrpc_node& n, std::string fn,
                                    void(*f)(T1&,T2&,T3&),
                                    const T1& a1, const T2& a2, const T3& a3) {
  std::ostringstream outss;
  my_portable_oarchive outar(outss);
  outar << a1;
  outar << a2;
  outar << a3;
  send_exrpcreq_oneway_noexcept(n, fn, outss.str());
}

template <class T1, class T2>
void exrpc_function_oneway_noexcept(exrpc_node& n, std::string fn,
                                    void(*f)(T1&,T2&),
                                    const T1& a1, const T2& a2) {
  std::ostringstream outss;
  my_portable_oarchive outar(outss);
  outar << a1;
  outar << a2;
  send_exrpcreq_oneway_noexcept(n, fn, outss.str());
}

template <class T1>
void exrpc_function_oneway_noexcept(exrpc_node& n, std::string fn,
                                    void(*f)(T1&),
                                    const T1& a1) {
  std::ostringstream outss;
  my_portable_oarchive outar(outss);
  outar << a1;
  send_exrpcreq_oneway_noexcept(n, fn, outss.str());
}

void exrpc_function_oneway_noexcept(exrpc_node& n, std::string fn, void(*f)());

}

#endif
