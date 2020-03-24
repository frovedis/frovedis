#ifndef RPC_ONEWAY_HPP
#define RPC_ONEWAY_HPP

#include <string>
#include <sstream>
#include "mpi_rpc.hpp"
#include "fwrapper_oneway.hpp"

namespace frovedis {

template <class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8, class T9, class T10>
void rpc_oneway(NID n, void(*f)(T1&,T2&,T3&,T4&,T5&,T6&,T7&,T8&,T9&,T10&), 
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
  send_rpcreq(rpc_type::rpc_oneway_type, n,
              reinterpret_cast<intptr_t>(f),
              reinterpret_cast<intptr_t>
              (fwrapper_oneway<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10>),
              str);
}

template <class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8, class T9>
void rpc_oneway(NID n, void(*f)(T1&,T2&,T3&,T4&,T5&,T6&,T7&,T8&,T9&), 
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
  send_rpcreq(rpc_type::rpc_oneway_type, n,
              reinterpret_cast<intptr_t>(f),
              reinterpret_cast<intptr_t>
              (fwrapper_oneway<T1,T2,T3,T4,T5,T6,T7,T8,T9>),
              str);
}

template <class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8>
void rpc_oneway(NID n, void(*f)(T1&,T2&,T3&,T4&,T5&,T6&,T7&,T8&), 
                const T1& a1, const T2& a2, const T3& a3, const T4& a4,
                const T5& a5, const T6& a6, const T7& a7, const T8& a8) {
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
  send_rpcreq(rpc_type::rpc_oneway_type, n,
              reinterpret_cast<intptr_t>(f),
              reinterpret_cast<intptr_t>
              (fwrapper_oneway<T1,T2,T3,T4,T5,T6,T7,T8>),
              str);
}

template <class T1, class T2, class T3, class T4, class T5, class T6,
          class T7>
void rpc_oneway(NID n, void(*f)(T1&,T2&,T3&,T4&,T5&,T6&,T7&), 
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
  send_rpcreq(rpc_type::rpc_oneway_type, n,
              reinterpret_cast<intptr_t>(f),
              reinterpret_cast<intptr_t>
              (fwrapper_oneway<T1,T2,T3,T4,T5,T6,T7>),
              str);
}

template <class T1, class T2, class T3, class T4, class T5, class T6>
void rpc_oneway(NID n, void(*f)(T1&,T2&,T3&,T4&,T5&,T6&), 
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
  send_rpcreq(rpc_type::rpc_oneway_type, n,
              reinterpret_cast<intptr_t>(f),
              reinterpret_cast<intptr_t>(fwrapper_oneway<T1,T2,T3,T4,T5,T6>),
              str);
}

template <class T1, class T2, class T3, class T4, class T5>
void rpc_oneway(NID n, void(*f)(T1&,T2&,T3&,T4&,T5&), 
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
  send_rpcreq(rpc_type::rpc_oneway_type, n,
              reinterpret_cast<intptr_t>(f),
              reinterpret_cast<intptr_t>(fwrapper_oneway<T1,T2,T3,T4,T5>),
              str);
}

template <class T1, class T2, class T3, class T4>
void rpc_oneway(NID n, void(*f)(T1&,T2&,T3&,T4&), 
                const T1& a1, const T2& a2, const T3& a3, const T4& a4) {
  my_ostream outss;
  my_oarchive outar(outss);
  outar & a1;
  outar & a2;
  outar & a3;
  outar & a4;
  OSTREAM_TO_STRING(outss, str);
  send_rpcreq(rpc_type::rpc_oneway_type, n,
              reinterpret_cast<intptr_t>(f),
              reinterpret_cast<intptr_t>(fwrapper_oneway<T1,T2,T3,T4>),
              str);
}

template <class T1, class T2, class T3>
void rpc_oneway(NID n, void(*f)(T1&,T2&,T3&), 
                const T1& a1, const T2& a2, const T3& a3) {
  my_ostream outss;
  my_oarchive outar(outss);
  outar & a1;
  outar & a2;
  outar & a3;
  OSTREAM_TO_STRING(outss, str);
  send_rpcreq(rpc_type::rpc_oneway_type, n,
              reinterpret_cast<intptr_t>(f),
              reinterpret_cast<intptr_t>(fwrapper_oneway<T1,T2,T3>),
              str);
}

template <class T1, class T2>
void rpc_oneway(NID n, void(*f)(T1&,T2&), 
                const T1& a1, const T2& a2) {
  my_ostream outss;
  my_oarchive outar(outss);
  outar & a1;
  outar & a2;
  OSTREAM_TO_STRING(outss, str);
  send_rpcreq(rpc_type::rpc_oneway_type, n,
              reinterpret_cast<intptr_t>(f),
              reinterpret_cast<intptr_t>(fwrapper_oneway<T1,T2>),
              str);
}

template <class T1>
void rpc_oneway(NID n, void(*f)(T1&), 
                const T1& a1) {
  my_ostream outss;
  my_oarchive outar(outss);
  outar & a1;
  OSTREAM_TO_STRING(outss, str);
  send_rpcreq(rpc_type::rpc_oneway_type, n,
              reinterpret_cast<intptr_t>(f),
              reinterpret_cast<intptr_t>(fwrapper_oneway<T1>),
              str);
}

void rpc_oneway(NID n, void(*f)());

}

#endif
