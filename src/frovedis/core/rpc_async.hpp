#ifndef RPC_ASYNC_HPP
#define RPC_ASYNC_HPP

#include <string>
#include <sstream>

#include "mpi_rpc.hpp"
#include "rpc_result.hpp"
#include "fwrapper.hpp"

namespace frovedis {

template <class R>
rpc_result<R> send_rpcreq_async(NID n, 
                                intptr_t function_addr,
                                intptr_t wrapper_addr,
                                const std::string& serialized_arg) {
  int tag = send_rpcreq(rpc_type::rpc_async_type, n, function_addr,
                        wrapper_addr, serialized_arg);
  return rpc_result<R>(tag, n);
}

template <class R, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8, class T9, class T10>
rpc_result<R> rpc_async(NID n, R(*f)(T1&,T2&,T3&,T4&,T5&,T6&,T7&,T8&,T9&,T10&),
                        const T1& a1, const T2& a2, const T3& a3, const T4& a4,
                        const T5& a5, const T6& a6, const T7& a7, const T8& a8,
                        const T9& a9, const T10& a10) {
  my_ostream outss;
  my_oarchive outar(outss);
  outar & a1;
  outar & a2;
  outar & a3;
  outar & a4;
  outar & a5;
  outar & a6;
  outar & a7;
  outar & a8;
  outar & a9;
  outar & a10;
  OSTREAM_TO_STRING(outss, str);
  return send_rpcreq_async<R>(n, reinterpret_cast<intptr_t>(f),
                              reinterpret_cast<intptr_t>
                              (fwrapper<R,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10>),
                              str);
}

template <class R, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8, class T9>
rpc_result<R> rpc_async(NID n, R(*f)(T1&,T2&,T3&,T4&,T5&,T6&,T7&,T8&,T9&),
                        const T1& a1, const T2& a2, const T3& a3, const T4& a4,
                        const T5& a5, const T6& a6, const T7& a7, const T8& a8,
                        const T9& a9) {
  my_ostream outss;
  my_oarchive outar(outss);
  outar & a1;
  outar & a2;
  outar & a3;
  outar & a4;
  outar & a5;
  outar & a6;
  outar & a7;
  outar & a8;
  outar & a9;
  OSTREAM_TO_STRING(outss, str);
  return send_rpcreq_async<R>(n, reinterpret_cast<intptr_t>(f),
                              reinterpret_cast<intptr_t>
                              (fwrapper<R,T1,T2,T3,T4,T5,T6,T7,T8,T9>),
                              str);
}

template <class R, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8>
rpc_result<R> rpc_async(NID n, R(*f)(T1&,T2&,T3&,T4&,T5&,T6&,T7&,T8&),
                        const T1& a1, const T2& a2, const T3& a3, const T4& a4,
                        const T5& a5, const T6& a6, const T7& a7,
                        const T8& a8){
  my_ostream outss;
  my_oarchive outar(outss);
  outar & a1;
  outar & a2;
  outar & a3;
  outar & a4;
  outar & a5;
  outar & a6;
  outar & a7;
  outar & a8;
  OSTREAM_TO_STRING(outss, str);
  return send_rpcreq_async<R>(n, reinterpret_cast<intptr_t>(f),
                              reinterpret_cast<intptr_t>
                              (fwrapper<R,T1,T2,T3,T4,T5,T6,T7,T8>),
                              str);
}

template <class R, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7>
rpc_result<R> rpc_async(NID n, R(*f)(T1&,T2&,T3&,T4&,T5&,T6&,T7&),
                        const T1& a1, const T2& a2, const T3& a3, const T4& a4,
                        const T5& a5, const T6& a6, const T7& a7) {
  my_ostream outss;
  my_oarchive outar(outss);
  outar & a1;
  outar & a2;
  outar & a3;
  outar & a4;
  outar & a5;
  outar & a6;
  outar & a7;
  OSTREAM_TO_STRING(outss, str);
  return send_rpcreq_async<R>(n, reinterpret_cast<intptr_t>(f),
                              reinterpret_cast<intptr_t>
                              (fwrapper<R,T1,T2,T3,T4,T5,T6,T7>),
                              str);
}

template <class R, class T1, class T2, class T3, class T4, class T5, class T6>
rpc_result<R> rpc_async(NID n, R(*f)(T1&,T2&,T3&,T4&,T5&,T6&),
                        const T1& a1, const T2& a2, const T3& a3, const T4& a4,
                        const T5& a5, const T6& a6) {
  my_ostream outss;
  my_oarchive outar(outss);
  outar & a1;
  outar & a2;
  outar & a3;
  outar & a4;
  outar & a5;
  outar & a6;
  OSTREAM_TO_STRING(outss, str);
  return send_rpcreq_async<R>(n, reinterpret_cast<intptr_t>(f),
                              reinterpret_cast<intptr_t>
                              (fwrapper<R,T1,T2,T3,T4,T5,T6>),
                              str);
}

template <class R, class T1, class T2, class T3, class T4, class T5>
rpc_result<R> rpc_async(NID n, R(*f)(T1&,T2&,T3&,T4&,T5&),
                        const T1& a1, const T2& a2, const T3& a3, const T4& a4,
                        const T5& a5) {
  my_ostream outss;
  my_oarchive outar(outss);
  outar & a1;
  outar & a2;
  outar & a3;
  outar & a4;
  outar & a5;
  OSTREAM_TO_STRING(outss, str);
  return send_rpcreq_async<R>(n, reinterpret_cast<intptr_t>(f),
                              reinterpret_cast<intptr_t>
                              (fwrapper<R,T1,T2,T3,T4,T5>),
                              str);
}

template <class R, class T1, class T2, class T3, class T4>
rpc_result<R> rpc_async(NID n, R(*f)(T1&,T2&,T3&,T4&),
                        const T1& a1, const T2& a2, const T3& a3,
                        const T4& a4) {
  my_ostream outss;
  my_oarchive outar(outss);
  outar & a1;
  outar & a2;
  outar & a3;
  outar & a4;
  OSTREAM_TO_STRING(outss, str);
  return send_rpcreq_async<R>(n, reinterpret_cast<intptr_t>(f),
                              reinterpret_cast<intptr_t>
                              (fwrapper<R,T1,T2,T3,T4>),
                              str);
}

template <class R, class T1, class T2, class T3>
rpc_result<R> rpc_async(NID n, R(*f)(T1&,T2&,T3&),
                        const T1& a1, const T2& a2, const T3& a3) {
  my_ostream outss;
  my_oarchive outar(outss);
  outar & a1;
  outar & a2;
  outar & a3;
  OSTREAM_TO_STRING(outss, str);
  return send_rpcreq_async<R>(n, reinterpret_cast<intptr_t>(f),
                              reinterpret_cast<intptr_t>(fwrapper<R,T1,T2,T3>),
                              str);
}

template <class R, class T1, class T2>
rpc_result<R> rpc_async(NID n, R(*f)(T1&,T2&),
                        const T1& a1, const T2& a2) {
  my_ostream outss;
  my_oarchive outar(outss);
  outar & a1;
  outar & a2;
  OSTREAM_TO_STRING(outss, str);
  return send_rpcreq_async<R>(n, reinterpret_cast<intptr_t>(f),
                              reinterpret_cast<intptr_t>(fwrapper<R,T1,T2>),
                              str);
}

template <class R, class T1>
rpc_result<R> rpc_async(NID n, R(*f)(T1&),
                        const T1& a1) {
  my_ostream outss;
  my_oarchive outar(outss);
  outar & a1;
  OSTREAM_TO_STRING(outss, str);
  return send_rpcreq_async<R>(n, reinterpret_cast<intptr_t>(f),
                              reinterpret_cast<intptr_t>(fwrapper<R,T1>),
                              str);
}

template <class R>
rpc_result<R> rpc_async(NID n, R(*f)()) {
  my_ostream outss;
  my_oarchive outar(outss);
  OSTREAM_TO_STRING(outss, str);
  return send_rpcreq_async<R>(n, reinterpret_cast<intptr_t>(f),
                              reinterpret_cast<intptr_t>(fwrapper<R>),
                              str);
}

}

#endif
