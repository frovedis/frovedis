#ifndef BCAST_RPC_HPP
#define BCAST_RPC_HPP

#include <string>
#include <sstream>

#include "mpi_rpc.hpp"
#include "fwrapper.hpp"

namespace frovedis {

template <class R>
std::vector<R> typed_send_bcast_rpcreq(intptr_t function_addr,
                                       intptr_t wrapper_addr,
                                       const std::string& serialized_arg) {
  size_t node_size = get_nodesize();
  std::vector<std::string> serialized_result;
  send_bcast_rpcreq(rpc_type::bcast_rpc_type, function_addr, wrapper_addr,
                    serialized_arg, serialized_result);
  std::vector<R> result(node_size);
  for(size_t i = 0; i < node_size; i++) {
    std::istringstream ss(serialized_result[i]);
    my_iarchive ar(ss);
    ar >> result[i];
  }
  return result;
}

template <class R, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8, class T9, class T10>
std::vector<R> bcast_rpc(R(*f)(T1&,T2&,T3&,T4&,T5&,T6&,T7&,T8&,T9&,T10&),
                         const T1& a1, const T2& a2, const T3& a3,
                         const T4& a4, const T5& a5, const T6& a6,
                         const T7& a7, const T8& a8, const T9& a9,
                         const T10& a10) {
  std::ostringstream outss;
  my_oarchive outar(outss);
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
  return typed_send_bcast_rpcreq<R>
    (reinterpret_cast<intptr_t>(f),
     reinterpret_cast<intptr_t>(fwrapper<R,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10>),
     outss.str());
}

template <class R, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8, class T9>
std::vector<R> bcast_rpc(R(*f)(T1&,T2&,T3&,T4&,T5&,T6&,T7&,T8&,T9&),
                         const T1& a1, const T2& a2, const T3& a3,
                         const T4& a4, const T5& a5, const T6& a6,
                         const T7& a7, const T8& a8, const T9& a9) {
  std::ostringstream outss;
  my_oarchive outar(outss);
  outar << a1;
  outar << a2;
  outar << a3;
  outar << a4;
  outar << a5;
  outar << a6;
  outar << a7;
  outar << a8;
  outar << a9;
  return typed_send_bcast_rpcreq<R>
    (reinterpret_cast<intptr_t>(f),
     reinterpret_cast<intptr_t>(fwrapper<R,T1,T2,T3,T4,T5,T6,T7,T8,T9>),
     outss.str());
}

template <class R, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8>
std::vector<R> bcast_rpc(R(*f)(T1&,T2&,T3&,T4&,T5&,T6&,T7&,T8&),
                         const T1& a1, const T2& a2, const T3& a3,
                         const T4& a4, const T5& a5, const T6& a6,
                         const T7& a7, const T8& a8) {
  std::ostringstream outss;
  my_oarchive outar(outss);
  outar << a1;
  outar << a2;
  outar << a3;
  outar << a4;
  outar << a5;
  outar << a6;
  outar << a7;
  outar << a8;
  return typed_send_bcast_rpcreq<R>
    (reinterpret_cast<intptr_t>(f),
     reinterpret_cast<intptr_t>(fwrapper<R,T1,T2,T3,T4,T5,T6,T7,T8>),
     outss.str());
}

template <class R, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7>
std::vector<R> bcast_rpc(R(*f)(T1&,T2&,T3&,T4&,T5&,T6&,T7&),
                         const T1& a1, const T2& a2, const T3& a3,
                         const T4& a4, const T5& a5, const T6& a6,
                         const T7& a7) {
  std::ostringstream outss;
  my_oarchive outar(outss);
  outar << a1;
  outar << a2;
  outar << a3;
  outar << a4;
  outar << a5;
  outar << a6;
  outar << a7;
  return typed_send_bcast_rpcreq<R>
    (reinterpret_cast<intptr_t>(f),
     reinterpret_cast<intptr_t>(fwrapper<R,T1,T2,T3,T4,T5,T6,T7>),
     outss.str());
}

template <class R, class T1, class T2, class T3, class T4, class T5, class T6>
std::vector<R> bcast_rpc(R(*f)(T1&,T2&,T3&,T4&,T5&,T6&),
                         const T1& a1, const T2& a2, const T3& a3,
                         const T4& a4, const T5& a5, const T6& a6) {
  std::ostringstream outss;
  my_oarchive outar(outss);
  outar << a1;
  outar << a2;
  outar << a3;
  outar << a4;
  outar << a5;
  outar << a6;
  return typed_send_bcast_rpcreq<R>
    (reinterpret_cast<intptr_t>(f),
     reinterpret_cast<intptr_t>(fwrapper<R,T1,T2,T3,T4,T5,T6>),
     outss.str());
}

template <class R, class T1, class T2, class T3, class T4, class T5>
std::vector<R> bcast_rpc(R(*f)(T1&,T2&,T3&,T4&,T5&),
                         const T1& a1, const T2& a2, const T3& a3,
                         const T4& a4, const T5& a5) {
  std::ostringstream outss;
  my_oarchive outar(outss);
  outar << a1;
  outar << a2;
  outar << a3;
  outar << a4;
  outar << a5;
  return typed_send_bcast_rpcreq<R>
    (reinterpret_cast<intptr_t>(f),
     reinterpret_cast<intptr_t>(fwrapper<R,T1,T2,T3,T4,T5>),
     outss.str());
}

template <class R, class T1, class T2, class T3, class T4>
std::vector<R> bcast_rpc(R(*f)(T1&,T2&,T3&,T4&),
                         const T1& a1, const T2& a2, const T3& a3,
                         const T4& a4) {
  std::ostringstream outss;
  my_oarchive outar(outss);
  outar << a1;
  outar << a2;
  outar << a3;
  outar << a4;
  return typed_send_bcast_rpcreq<R>
    (reinterpret_cast<intptr_t>(f),
     reinterpret_cast<intptr_t>(fwrapper<R,T1,T2,T3,T4>),
     outss.str());
}

template <class R, class T1, class T2, class T3>
std::vector<R> bcast_rpc(R(*f)(T1&,T2&,T3&),
                         const T1& a1, const T2& a2, const T3& a3) {
  std::ostringstream outss;
  my_oarchive outar(outss);
  outar << a1;
  outar << a2;
  outar << a3;
  return typed_send_bcast_rpcreq<R>
    (reinterpret_cast<intptr_t>(f),
     reinterpret_cast<intptr_t>(fwrapper<R,T1,T2,T3>),
     outss.str());
}

template <class R, class T1, class T2>
std::vector<R> bcast_rpc(R(*f)(T1&,T2&),
                         const T1& a1, const T2& a2) {
  std::ostringstream outss;
  my_oarchive outar(outss);
  outar << a1;
  outar << a2;
  return typed_send_bcast_rpcreq<R>
    (reinterpret_cast<intptr_t>(f),
     reinterpret_cast<intptr_t>(fwrapper<R,T1,T2>),
     outss.str());
}

template <class R, class T1>
std::vector<R> bcast_rpc(R(*f)(T1&),
                         const T1& a1) {
  std::ostringstream outss;
  my_oarchive outar(outss);
  outar << a1;
  return typed_send_bcast_rpcreq<R>(reinterpret_cast<intptr_t>(f),
                                    reinterpret_cast<intptr_t>(fwrapper<R,T1>),
                                    outss.str());
}

template <class R>
std::vector<R> bcast_rpc(R(*f)()) {
  std::ostringstream outss;
  my_oarchive outar(outss);
  return typed_send_bcast_rpcreq<R>(reinterpret_cast<intptr_t>(f),
                                    reinterpret_cast<intptr_t>(fwrapper<R>),
                                    outss.str());
}

}

#endif
