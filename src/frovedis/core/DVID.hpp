#ifndef DVID_HPP
#define DVID_HPP

#include <string.h>
#include <map>

#include "rlog.hpp"
#include "utility.hpp"
#include "bcast_rpc.hpp"
#include "bcast_rpc_oneway.hpp"
#include "serfunc.hpp"
#include "mpihelper.hpp"
#include "serializehelper.hpp"

namespace frovedis {

typedef int dvid_t;
extern std::map<dvid_t, intptr_t> dvid_table;
dvid_t get_new_dvid();

template <class T> class DVID;
template <class T> std::vector<T> gather(DVID<T>& d);

template <class T>
class DVID {
public:
  DVID() : dvid(0) {}
  DVID(const DVID<T>& dvid_) : dvid(dvid_.dvid) {}
  DVID(DVID<T>&& dvid_) {std::swap(dvid, dvid_.dvid);}
  DVID(dvid_t dvid_) : dvid(dvid_) {}
  DVID<T>& operator=(const DVID<T>& dvid_) {dvid = dvid_.dvid; return *this;}
  DVID<T>& operator=(DVID<T>&& dvid_)
    {std::swap(dvid, dvid_.dvid); return *this;}
  T* get_selfdata();
  void delete_var();
  DVID<T> copy() const;
  template <class R, class F> DVID<R> map(const F&);
  template <class R, class TT>
  DVID<R> map(R(*f)(TT)) {return map<R>(make_serfunc(f));}
  template <class F> void mapv(const F&);
  template <class TT> void mapv(void(*f)(TT)){mapv(make_serfunc(f));}
  std::vector<T> gather() {return frovedis::gather(*this);}
  template<class F> T reduce(const F&);
  template <class TT, class UU> T reduce(T(*f)(TT, UU)) { 
   return reduce(make_serfunc(f));
  }
  template<class F> DVID<T> allreduce(const F&);
  template <class TT, class UU> DVID<T> allreduce(T(*f)(TT, UU)) {
    return allreduce(make_serfunc(f));
  }
  template <class R, class U, class F> DVID<R>
  map(const F&, const DVID<U>& d1);
  template <class R, class U, class TT, class UU>
  DVID<R> map(R(*f)(TT,UU), const DVID<U>& d1) {
    return map<R,U>(make_serfunc(f),d1);
  }
  template <class R, class U, class V, class F>
  DVID<R> map(const F&, const DVID<U>&, const DVID<V>&);
  template <class R, class U, class V, class TT, class UU, class VV>
  DVID<R> map(R(*f)(TT,UU,VV), const DVID<U>& d1, const DVID<V>& d2) {
    return map<R,U>(make_serfunc(f), d1, d2);
  }
  template <class R, class U, class V, class W, class F>
  DVID<R> map(const F&, const DVID<U>&, const DVID<V>&, const DVID<W>&);
  template <class R, class U, class V, class W,
            class TT, class UU, class VV, class WW>
  DVID<R> map(R(*f)(TT,UU,VV,WW), const DVID<U>& d1, const DVID<V>& d2,
              const DVID<W>& d3) {
    return map<R,U>(make_serfunc(f), d1, d2, d3);
  }
  template <class R, class U, class V, class W, class X, class F>
  DVID<R> map(const F&, const DVID<U>&, const DVID<V>&, const DVID<W>&,
              const DVID<X>&);
  template <class R, class U, class V, class W, class X,
            class TT, class UU, class VV, class WW, class XX>
  DVID<R> map(R(*f)(TT,UU,VV,WW,XX), const DVID<U>& d1, const DVID<V>& d2,
              const DVID<W>& d3, const DVID<X>& d4) {
    return map<R,U>(make_serfunc(f), d1, d2, d3, d4);
  }
  template <class R, class U, class V, class W, class X, class Y, class F>
  DVID<R> map(const F&, const DVID<U>&, const DVID<V>&, const DVID<W>&,
              const DVID<X>&, const DVID<Y>&);
  template <class R, class U, class V, class W, class X, class Y,
            class TT, class UU, class VV, class WW, class XX, class YY>
  DVID<R> map(R(*f)(TT,UU,VV,WW,XX,YY), const DVID<U>& d1, const DVID<V>& d2,
              const DVID<W>& d3, const DVID<X>& d4, const DVID<Y>& d5) {
    return map<R,U>(make_serfunc(f), d1, d2, d3, d4, d5);
  }
  template <class R, class U, class V, class W, class X, class Y, class Z, class F>
  DVID<R> map(const F&, const DVID<U>&, const DVID<V>&, const DVID<W>&,
              const DVID<X>&, const DVID<Y>&, const DVID<Z>&);
  template <class R, class U, class V, class W, class X, class Y, class Z,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ>
  DVID<R> map(R(*f)(TT,UU,VV,WW,XX,YY,ZZ), const DVID<U>& d1, const DVID<V>& d2,
              const DVID<W>& d3, const DVID<X>& d4, const DVID<Y>& d5, const DVID<Z>& d6) {
    return map<R,U>(make_serfunc(f), d1, d2, d3, d4, d5, d6);
  }
  template <class R, class U, class V, class W, class X, class Y, class Z, class A, class F>
  DVID<R> map(const F&, const DVID<U>&, const DVID<V>&, const DVID<W>&,
              const DVID<X>&, const DVID<Y>&, const DVID<Z>&, const DVID<A>&);
  template <class R, class U, class V, class W, class X, class Y, class Z, class A,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ, class AA>
  DVID<R> map(R(*f)(TT,UU,VV,WW,XX,YY,ZZ,AA), const DVID<U>& d1, const DVID<V>& d2,
              const DVID<W>& d3, const DVID<X>& d4, const DVID<Y>& d5, const DVID<Z>& d6, const DVID<A>& d7) {
    return map<R,U>(make_serfunc(f), d1, d2, d3, d4, d5, d6, d7);
  }
  template <class R, class U, class V, class W, class X, class Y, class Z, class A, class B, class F>
  DVID<R> map(const F&, const DVID<U>&, const DVID<V>&, const DVID<W>&,
              const DVID<X>&, const DVID<Y>&, const DVID<Z>&, const DVID<A>&, const DVID<B>&);
  template <class R, class U, class V, class W, class X, class Y, class Z, class A, class B,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ, class AA, class BB>
  DVID<R> map(R(*f)(TT,UU,VV,WW,XX,YY,ZZ,AA,BB), const DVID<U>& d1, const DVID<V>& d2,
              const DVID<W>& d3, const DVID<X>& d4, const DVID<Y>& d5, const DVID<Z>& d6, const DVID<A>& d7, const DVID<B>& d8) {
    return map<R,U>(make_serfunc(f), d1, d2, d3, d4, d5, d6, d7, d8);
  }

  template <class U, class F> void mapv(const F&, const DVID<U>&);
  template <class U, class TT, class UU>
  void mapv(void(*f)(TT,UU), const DVID<U>& d1){mapv(make_serfunc(f), d1);}
  template <class U, class V, class F>
  void mapv(const F&, const DVID<U>&, const DVID<V>&);
  template <class U, class V, class TT, class UU, class VV>
  void mapv(void(*f)(TT,UU,VV), const DVID<U>& d1, const DVID<V>& d2){
    mapv(make_serfunc(f), d1, d2);
  }
  template <class U, class V, class W, class F>
  void mapv(const F&, const DVID<U>&, const DVID<V>&, const DVID<W>&);
  template <class U, class V, class W,
            class TT, class UU, class VV, class WW>
  void mapv(void(*f)(TT,UU,VV,WW), const DVID<U>& d1, const DVID<V>& d2,
            const DVID<W>& d3){
    mapv(make_serfunc(f), d1, d2, d3);
  }
  template <class U, class V, class W, class X, class F>
  void mapv(const F&, const DVID<U>&, const DVID<V>&, const DVID<W>&,
            const DVID<X>&);
  template <class U, class V, class W, class X,
            class TT, class UU, class VV, class WW, class XX>
  void mapv(void(*f)(TT,UU,VV,WW,XX), const DVID<U>& d1, const DVID<V>& d2,
            const DVID<W>& d3, const DVID<X>& d4){
    mapv(make_serfunc(f), d1, d2, d3, d4);
  }
  template <class U, class V, class W, class X, class Y, class F>
  void mapv(const F&, const DVID<U>&, const DVID<V>&, const DVID<W>&,
            const DVID<X>&, const DVID<Y>&);
  template <class U, class V, class W, class X, class Y, 
            class TT, class UU, class VV, class WW, class XX, class YY>
  void mapv(void(*f)(TT,UU,VV,WW,XX,YY), const DVID<U>& d1, const DVID<V>& d2,
            const DVID<W>& d3, const DVID<X>& d4, const DVID<Y>& d5){
    mapv(make_serfunc(f), d1, d2, d3, d4, d5);
  }
  template <class U, class V, class W, class X, class Y, class Z, class F>
  void mapv(const F&, const DVID<U>&, const DVID<V>&, const DVID<W>&,
            const DVID<X>&, const DVID<Y>&, const DVID<Z>&);
  template <class U, class V, class W, class X, class Y, class Z,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ>
  void mapv(void(*f)(TT,UU,VV,WW,XX,YY,ZZ), const DVID<U>& d1, const DVID<V>& d2,
            const DVID<W>& d3, const DVID<X>& d4, const DVID<Y>& d5, const DVID<Z>& d6){
    mapv(make_serfunc(f), d1, d2, d3, d4, d5, d6);
  }
  template <class U, class V, class W, class X, class Y, class Z, class A, class F>
  void mapv(const F&, const DVID<U>&, const DVID<V>&, const DVID<W>&,
            const DVID<X>&, const DVID<Y>&, const DVID<Z>&, const DVID<A>&);
  template <class U, class V, class W, class X, class Y, class Z, class A,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ, class AA>
  void mapv(void(*f)(TT,UU,VV,WW,XX,YY,ZZ,AA), const DVID<U>& d1, const DVID<V>& d2,
            const DVID<W>& d3, const DVID<X>& d4, const DVID<Y>& d5, const DVID<Z>& d6,
            const DVID<A>& d7){
    mapv(make_serfunc(f), d1, d2, d3, d4, d5, d6, d7);
  }
  template <class U, class V, class W, class X, class Y, class Z, class A, class B, class F>
  void mapv(const F&, const DVID<U>&, const DVID<V>&, const DVID<W>&,
            const DVID<X>&, const DVID<Y>&, const DVID<Z>&, const DVID<A>&, const DVID<B>&);
  template <class U, class V, class W, class X, class Y, class Z, class A, class B,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ, class AA, class BB>
  void mapv(void(*f)(TT,UU,VV,WW,XX,YY,ZZ,AA,BB), const DVID<U>& d1, const DVID<V>& d2,
            const DVID<W>& d3, const DVID<X>& d4, const DVID<Y>& d5, const DVID<Z>& d6,
            const DVID<A>& d7, const DVID<B>& d8){
    mapv(make_serfunc(f), d1, d2, d3, d4, d5, d6, d7, d8);
  }

  T vector_sum();

  void put(int node, const T& val);
  T get(int node);

  void clear_dvid(){dvid = 0;}

  dvid_t dvid;

  SERIALIZE(dvid)
};

template <class T>
T* get_data(dvid_t dvid) {
  auto i = dvid_table.find(dvid);
  if(i == dvid_table.end())
    throw std::runtime_error("get_data(): invalid dvid");
  return reinterpret_cast<T*>(i->second);
}

template <class T>
void set_data(dvid_t dvid, T* ptr) {
  auto i = dvid_table.find(dvid);
  if(i != dvid_table.end())
    throw std::runtime_error("set_data(): dvid already exists");
  dvid_table[dvid] = reinterpret_cast<intptr_t>(ptr);
}

template <class T>
T* DVID<T>::get_selfdata() {
  return get_data<T>(dvid);
}

template <class T>
void make_dvid_allocate_helper(dvid_t& dvid) {
  T* ptr = new T();
  dvid_table[dvid] = reinterpret_cast<intptr_t>(ptr);
}

template <class T>
DVID<T> make_dvid_allocate() {
  auto new_dvid = get_new_dvid();
  bcast_rpc_oneway(make_dvid_allocate_helper<T>, new_dvid);
  return DVID<T>(new_dvid);
}

template <class T>
void delete_var_helper(dvid_t& dvid) {
  delete get_data<T>(dvid);
  dvid_table.erase(dvid);
}

template <class T>
void DVID<T>::delete_var() {
  if(dvid == 0) return; // 0 means not allocated
  else bcast_rpc_oneway(delete_var_helper<T>, dvid);
}

template <class T>
void dvid_copy_helper(dvid_t& from, dvid_t& to) {
  // SX dislikes copy ctor of std::vector
  T* tmp = new T();
  *tmp = *get_data<T>(from);
  set_data(to, tmp);
}

template <class T>
DVID<T> DVID<T>::copy() const {
  RLOG(DEBUG) << "distributed variables are copied" << std::endl;
  auto d = get_new_dvid();
  bcast_rpc_oneway(dvid_copy_helper<T>, dvid, d);
  return DVID<T>(d);
}

template <class T, class R, class F>
void dvid_map_helper(dvid_t& to, dvid_t& d, F& f) {
  T* dp = get_data<T>(d);
  R r = f(*dp);
  R* tp = new R();
  std::swap(r, *tp);
  set_data(to, tp);
}

template <class T>
template <class R, class F>
DVID<R> DVID<T>::map(const F& f) {
  auto d = get_new_dvid();
  bcast_rpc_oneway(dvid_map_helper<T,R,F>, d, dvid, const_cast<F&>(f));
  return DVID<R>(d);
}

template <class T>
void dvid_broadcast_helper(dvid_t& to, size_t& size, intptr_t& root_buf_ptr) {
  const int root = 0;
  if(get_selfid() == root) {
    char* sbuf = reinterpret_cast<char*>(root_buf_ptr);
    large_bcast(sizeof(char), sbuf, size, root, frovedis_comm_rpc);
  } else {
    std::string buf;
    buf.resize(size);
    char* rbuf = const_cast<char*>(&buf[0]);
    large_bcast(sizeof(char), rbuf, size, root, frovedis_comm_rpc);
    std::istringstream inss(buf); // TODO: remove this copy
    my_iarchive inar(inss);
    T* var = new T();
    inar >> *var;
    set_data<T>(to, var);
  }
}

template <class T>
DVID<T> make_dvid_broadcast(const T& src) {
  std::ostringstream ss;
  my_oarchive outar(ss);
  outar << src;
  std::string buf = ss.str();
  intptr_t root_buf_ptr = reinterpret_cast<intptr_t>(buf.c_str());
  size_t size = ss.str().size();
  auto d = get_new_dvid();
  bcast_rpc_oneway(dvid_broadcast_helper<T>, d, size, root_buf_ptr);
  // SX dislikes copy ctor of std::vector
  T* newsrc = new T();
  *newsrc = src;
  set_data<T>(d, newsrc);
  return DVID<T>(d);
}

template <class T>
void dvid_broadcast_helper2(dvid_t& to, size_t& size, intptr_t& root_buf_ptr) {
  const int root = 0;
  if(get_selfid() == root) {
    char* sbuf = reinterpret_cast<char*>(root_buf_ptr);
    large_bcast(sizeof(char), sbuf, size, root, frovedis_comm_rpc);
  } else {
    std::vector<T>* var = new std::vector<T>(size/sizeof(T));
    char* rbuf = reinterpret_cast<char*>(&(*var)[0]);
    large_bcast(sizeof(char), rbuf, size, root, frovedis_comm_rpc);
    set_data<std::vector<T>>(to, var);
  }
}

/*
  for vector of double/float/int/size_t, serialization is skipped
 */
template <>
DVID<std::vector<double>> make_dvid_broadcast(const std::vector<double>& src);
template <>
DVID<std::vector<float>> make_dvid_broadcast(const std::vector<float>& src);
template <>
DVID<std::vector<int>> make_dvid_broadcast(const std::vector<int>& src);
template <>
DVID<std::vector<size_t>> make_dvid_broadcast(const std::vector<size_t>& src);

template <class T, class F>
void mapv_dvars(dvid_t& d1, F& f) {
  T* d1p = get_data<T>(d1);
  f(*d1p);
}

template <class T>
template <class F>
void DVID<T>::mapv(const F& f) {
  bcast_rpc_oneway(mapv_dvars<T,F>, dvid, const_cast<F&>(f));
}

template <class T>
void dvid_scatter_helper(dvid_t& d, std::vector<size_t>& sendcounts,
                         intptr_t& bufptr) {
  const int root = 0;
  int self = get_selfid();
  if(self == root) {
    int node_size = get_nodesize();
    char* buf = reinterpret_cast<char*>(bufptr);
    std::vector<size_t> displs(node_size);
    auto displsp = displs.data();
    auto sendcountsp = sendcounts.data();
    displs[0] = 0;
    for(size_t i = 1; i < node_size; i++)
      displsp[i] = sendcountsp[i-1] + displsp[i-1];
    std::string recvbuf;
    recvbuf.resize(sendcounts[0]);
    large_scatterv(sizeof(char), buf, sendcounts, displs, &recvbuf[0],
                   sendcounts[0], root, frovedis_comm_rpc);
  } else {
    size_t recvcount = sendcounts[self];
    std::string recvbuf;
    recvbuf.resize(recvcount);
    std::vector<size_t> displs; // dummy
    large_scatterv(sizeof(char), 0, sendcounts, displs, &recvbuf[0],
                   recvcount, root, frovedis_comm_rpc);
    std::istringstream inss(recvbuf); // TODO: remove this copy
    my_iarchive inar(inss);
    T* newsrc = new T();
    inar >> *newsrc;
    set_data<T>(d, newsrc);
  }
}

template <class T>
DVID<T> make_dvid_scatter(std::vector<T>& src) {
  int node_size = get_nodesize();
  if(node_size != src.size())
    throw std::runtime_error("invalid vector size for scatter");
  std::vector<std::string> serialized_src(node_size);
  for(size_t i = 1; i < node_size; i++) { // skip root
    std::ostringstream ss;
    my_oarchive outar(ss);
    outar << src[i];
    serialized_src[i] = ss.str(); // TODO: avoid this copy
  }
  size_t total = 0;
  for(size_t i = 0; i < node_size; i++) total += serialized_src[i].size();
  std::string buf;
  buf.resize(total);
  char* bufp = &buf[0];
  std::vector<size_t> sendcounts(node_size);
  for(size_t i = 0; i < node_size; i++) {
    size_t size = serialized_src[i].size();
    sendcounts[i] = size;
    memcpy(bufp, &serialized_src[i][0], size);
    bufp += size;
  }
  auto d = get_new_dvid();
  intptr_t bufptr = reinterpret_cast<intptr_t>(&buf[0]);
  bcast_rpc_oneway(dvid_scatter_helper<T>, d, sendcounts, bufptr);
  // skip deserialization for root
  T* newsrc = new T();
  *newsrc = src[0];
  set_data<T>(d, newsrc);
  return DVID<T>(d);
}

template <class T>
void receive_data_helper(int from, T& data) {
  MPI_Status s;
  size_t recv_size;
  MPI_Recv(&recv_size, sizeof(size_t), MPI_CHAR, from, 0, frovedis_comm_rpc,
           &s);
  std::string buf;
  buf.resize(recv_size);
  large_recv(sizeof(char), &buf[0], recv_size, from, 0, frovedis_comm_rpc,
             &s);
  std::istringstream inss(buf);
  my_iarchive inar(inss);
  inar >> data;
}

template <class T>
void send_data_helper(int to, const T& data) {
  std::ostringstream ss;
  my_oarchive outar(ss);
  outar << data;
  std::string buf = ss.str();
  size_t send_size = buf.size();
  MPI_Send(&send_size, sizeof(size_t), MPI_CHAR, to, 0, frovedis_comm_rpc);
  large_send(sizeof(char), &buf[0], send_size, to, 0, frovedis_comm_rpc);
}

template <class T, class F>
void dvid_reduce_helper(dvid_t& dvid, intptr_t& ret, F& f) {
  int self = get_selfid();
  int node_size = get_nodesize();
  T self_data = *get_data<T>(dvid);
  for(int i = 0; node_size > 1; i++) {
    if(self % 2 != 0) { // sender
      int receiver = (self - 1) << i;
      send_data_helper(receiver, self_data);
      break;
    } else {
      if(self + 1 < node_size) {
        int sender = (self + 1) << i;
        T data;
        receive_data_helper(sender, data);
        self_data = f(self_data, data);
      }
      node_size = ceil_div(node_size, 2);
      self /= 2;
    }
  }
  if(self == 0) {
    T* retp = reinterpret_cast<T*>(ret);
    *retp = self_data;
  }
}

template <class T>
template <class F>
T DVID<T>::reduce(const F& f) {
  T ret;
  bcast_rpc_oneway(dvid_reduce_helper<T,F>, dvid,
                   reinterpret_cast<intptr_t>(&ret), const_cast<F&>(f));
  return ret;
}

template <class T>
template <class F>
DVID<T> DVID<T>::allreduce(const F& f) {
  return make_dvid_broadcast(reduce(f));
}

template <class T, class R, class U, class F>
void dvid_map_helper1(dvid_t& to, dvid_t& d, dvid_t& d1, F& f) {
  T* dp = get_data<T>(d);
  U* d1p = get_data<U>(d1);
  R r = f(*dp, *d1p);
  R* tp = new R();
  std::swap(r, *tp);
  set_data(to, tp);
}

template <class T>
template <class R, class U, class F>
DVID<R> DVID<T>::map(const F& f, const DVID<U>& d1) {
  auto r = get_new_dvid();
  bcast_rpc_oneway(dvid_map_helper1<T,R,U,F>, r, dvid, d1.dvid,
                   const_cast<F&>(f));
  return DVID<R>(r);
}

template <class T, class R, class U, class V, class F>
void dvid_map_helper2(dvid_t& to, dvid_t& d, dvid_t& d1, dvid_t& d2, F& f) {
  T* dp = get_data<T>(d);
  U* d1p = get_data<U>(d1);
  V* d2p = get_data<V>(d2);
  R r = f(*dp, *d1p, *d2p);
  R* tp = new R();
  std::swap(r, *tp);
  set_data(to, tp);
}

template <class T>
template <class R, class U, class V, class F>
DVID<R> DVID<T>::map(const F& f, const DVID<U>& d1, const DVID<V>& d2) {
  auto r = get_new_dvid();
  bcast_rpc_oneway(dvid_map_helper2<T,R,U,V,F>, r, dvid, d1.dvid, d2.dvid,
                   const_cast<F&>(f));
  return DVID<R>(r);
}

template <class T, class R, class U, class V, class W, class F>
void dvid_map_helper3(dvid_t& to, dvid_t& d, dvid_t& d1, dvid_t& d2,
                      dvid_t& d3, F& f) {
  T* dp = get_data<T>(d);
  U* d1p = get_data<U>(d1);
  V* d2p = get_data<V>(d2);
  W* d3p = get_data<W>(d3);
  R r = f(*dp, *d1p, *d2p, *d3p);
  R* tp = new R();
  std::swap(r, *tp);
  set_data(to, tp);
}

template <class T>
template <class R, class U, class V, class W, class F>
DVID<R> DVID<T>::map(const F& f, const DVID<U>& d1, const DVID<V>& d2,
                     const DVID<W>& d3) {
  auto r = get_new_dvid();
  bcast_rpc_oneway(dvid_map_helper3<T,R,U,V,W,F>, r, dvid, d1.dvid, d2.dvid,
                   d3.dvid, const_cast<F&>(f));
  return DVID<R>(r);
}

template <class T, class R, class U, class V, class W, class X, class F>
void dvid_map_helper4(dvid_t& to, dvid_t& d, dvid_t& d1, dvid_t& d2,
                      dvid_t& d3, dvid_t& d4, F& f) {
  T* dp = get_data<T>(d);
  U* d1p = get_data<U>(d1);
  V* d2p = get_data<V>(d2);
  W* d3p = get_data<W>(d3);
  X* d4p = get_data<X>(d4);
  R r = f(*dp, *d1p, *d2p, *d3p, *d4p);
  R* tp = new R();
  std::swap(r, *tp);
  set_data(to, tp);
}

template <class T>
template <class R, class U, class V, class W, class X, class F>
DVID<R> DVID<T>::map(const F& f, const DVID<U>& d1, const DVID<V>& d2,
                     const DVID<W>& d3, const DVID<X>& d4) {
  auto r = get_new_dvid();
  bcast_rpc_oneway(dvid_map_helper4<T,R,U,V,W,X,F>, r, dvid, d1.dvid, d2.dvid,
                   d3.dvid, d4.dvid, const_cast<F&>(f));
  return DVID<R>(r);
}

template <class T, class R, class U, class V, class W, class X,
          class Y, class F>
void dvid_map_helper5(dvid_t& to, dvid_t& d, dvid_t& d1, dvid_t& d2,
                      dvid_t& d3, dvid_t& d4, dvid_t& d5, F& f) {
  T* dp = get_data<T>(d);
  U* d1p = get_data<U>(d1);
  V* d2p = get_data<V>(d2);
  W* d3p = get_data<W>(d3);
  X* d4p = get_data<X>(d4);
  Y* d5p = get_data<Y>(d5);
  R r = f(*dp, *d1p, *d2p, *d3p, *d4p, *d5p);
  R* tp = new R();
  std::swap(r, *tp);
  set_data(to, tp);
}

template <class T>
template <class R, class U, class V, class W, class X, class Y, class F>
DVID<R> DVID<T>::map(const F& f, const DVID<U>& d1, const DVID<V>& d2,
                     const DVID<W>& d3, const DVID<X>& d4, const DVID<Y>& d5) {
  auto r = get_new_dvid();
  bcast_rpc_oneway(dvid_map_helper5<T,R,U,V,W,X,Y,F>, r, dvid, d1.dvid,
                   d2.dvid, d3.dvid, d4.dvid, d5.dvid, const_cast<F&>(f));
  return DVID<R>(r);
}

template <class T, class R, class U, class V, class W, class X,
          class Y, class Z, class F>
void dvid_map_helper6(dvid_t& to, dvid_t& d, dvid_t& d1, dvid_t& d2,
                      dvid_t& d3, dvid_t& d4, dvid_t& d5, dvid_t& d6, F& f) {
  T* dp = get_data<T>(d);
  U* d1p = get_data<U>(d1);
  V* d2p = get_data<V>(d2);
  W* d3p = get_data<W>(d3);
  X* d4p = get_data<X>(d4);
  Y* d5p = get_data<Y>(d5);
  Z* d6p = get_data<Z>(d6);
  R r = f(*dp, *d1p, *d2p, *d3p, *d4p, *d5p, *d6p);
  R* tp = new R();
  std::swap(r, *tp);
  set_data(to, tp);
}

template <class T>
template <class R, class U, class V, class W, class X, class Y, class Z, class F>
DVID<R> DVID<T>::map(const F& f, const DVID<U>& d1, const DVID<V>& d2,
                     const DVID<W>& d3, const DVID<X>& d4, const DVID<Y>& d5, const DVID<Z>& d6) {
  auto r = get_new_dvid();
  bcast_rpc_oneway(dvid_map_helper6<T,R,U,V,W,X,Y,Z,F>, r, dvid, d1.dvid,
                   d2.dvid, d3.dvid, d4.dvid, d5.dvid, d6.dvid, const_cast<F&>(f));
  return DVID<R>(r);
}

template <class T, class R, class U, class V, class W, class X,
          class Y, class Z, class A, class F>
void dvid_map_helper7(dvid_t& to, dvid_t& d, dvid_t& d1, dvid_t& d2,
                      dvid_t& d3, dvid_t& d4, dvid_t& d5, dvid_t& d6,
                      dvid_t& d7, F& f) {
  T* dp = get_data<T>(d);
  U* d1p = get_data<U>(d1);
  V* d2p = get_data<V>(d2);
  W* d3p = get_data<W>(d3);
  X* d4p = get_data<X>(d4);
  Y* d5p = get_data<Y>(d5);
  Z* d6p = get_data<Z>(d6);
  A* d7p = get_data<A>(d7);
  R r = f(*dp, *d1p, *d2p, *d3p, *d4p, *d5p, *d6p, *d7p);
  R* tp = new R();
  std::swap(r, *tp);
  set_data(to, tp);
}

template <class T>
template <class R, class U, class V, class W, class X, class Y, class Z, class A, class F>
DVID<R> DVID<T>::map(const F& f, const DVID<U>& d1, const DVID<V>& d2,
                     const DVID<W>& d3, const DVID<X>& d4, const DVID<Y>& d5, const DVID<Z>& d6, const DVID<A>& d7) {
  auto r = get_new_dvid();
  bcast_rpc_oneway(dvid_map_helper7<T,R,U,V,W,X,Y,Z,A,F>, r, dvid, d1.dvid,
                   d2.dvid, d3.dvid, d4.dvid, d5.dvid, d6.dvid, d7.dvid, const_cast<F&>(f));
  return DVID<R>(r);
}

template <class T, class R, class U, class V, class W, class X,
          class Y, class Z, class A, class B, class F>
void dvid_map_helper8(dvid_t& to, dvid_t& d, dvid_t& d1, dvid_t& d2,
                      dvid_t& d3, dvid_t& d4, dvid_t& d5, dvid_t& d6,
                      dvid_t& d7, dvid_t& d8, F& f) {
  T* dp = get_data<T>(d);
  U* d1p = get_data<U>(d1);
  V* d2p = get_data<V>(d2);
  W* d3p = get_data<W>(d3);
  X* d4p = get_data<X>(d4);
  Y* d5p = get_data<Y>(d5);
  Z* d6p = get_data<Z>(d6);
  A* d7p = get_data<A>(d7);
  B* d8p = get_data<B>(d8);
  R r = f(*dp, *d1p, *d2p, *d3p, *d4p, *d5p, *d6p, *d7p, *d8p);
  R* tp = new R();
  std::swap(r, *tp);
  set_data(to, tp);
}

template <class T>
template <class R, class U, class V, class W, class X, class Y, class Z, class A, class B, class F>
DVID<R> DVID<T>::map(const F& f, const DVID<U>& d1, const DVID<V>& d2,
                     const DVID<W>& d3, const DVID<X>& d4, const DVID<Y>& d5,
                     const DVID<Z>& d6, const DVID<A>& d7, const DVID<B>& d8) {
  auto r = get_new_dvid();
  bcast_rpc_oneway(dvid_map_helper8<T,R,U,V,W,X,Y,Z,A,B,F>, r, dvid, d1.dvid,
                   d2.dvid, d3.dvid, d4.dvid, d5.dvid, d6.dvid, d7.dvid, d8.dvid, const_cast<F&>(f));
  return DVID<R>(r);
}

template <class T, class U, class F>
void dvid_mapv_helper1(dvid_t& d, dvid_t& d1, F& f) {
  T* dp = get_data<T>(d);
  U* d1p = get_data<U>(d1);
  f(*dp, *d1p);
}

template <class T>
template <class U, class F>
void DVID<T>::mapv(const F& f, const DVID<U>& d1) {
  bcast_rpc_oneway(dvid_mapv_helper1<T,U,F>, dvid, d1.dvid,
                   const_cast<F&>(f));
}

template <class T, class U, class V, class F>
void dvid_mapv_helper2(dvid_t& d, dvid_t& d1, dvid_t& d2, F& f) {
  T* dp = get_data<T>(d);
  U* d1p = get_data<U>(d1);
  V* d2p = get_data<V>(d2);
  f(*dp, *d1p, *d2p);
}

template <class T>
template <class U, class V, class F>
void DVID<T>::mapv(const F& f, const DVID<U>& d1, const DVID<V>& d2) {
  bcast_rpc_oneway(dvid_mapv_helper2<T,U,V,F>, dvid, d1.dvid, d2.dvid,
                   const_cast<F&>(f));
}

template <class T, class U, class V, class W, class F>
void dvid_mapv_helper3(dvid_t& d, dvid_t& d1, dvid_t& d2, dvid_t& d3, F& f) {
  T* dp = get_data<T>(d);
  U* d1p = get_data<U>(d1);
  V* d2p = get_data<V>(d2);
  W* d3p = get_data<W>(d3);
  f(*dp, *d1p, *d2p, *d3p);
}

template <class T>
template <class U, class V, class W, class F>
void DVID<T>::mapv(const F& f, const DVID<U>& d1, const DVID<V>& d2,
                   const DVID<W>& d3) {
  bcast_rpc_oneway(dvid_mapv_helper3<T,U,V,W,F>, dvid, d1.dvid, d2.dvid,
                   d3.dvid, const_cast<F&>(f));
}

template <class T, class U, class V, class W, class X, class F>
void dvid_mapv_helper4(dvid_t& d, dvid_t& d1, dvid_t& d2, dvid_t& d3,
                       dvid_t& d4, F& f) {
  T* dp = get_data<T>(d);
  U* d1p = get_data<U>(d1);
  V* d2p = get_data<V>(d2);
  W* d3p = get_data<W>(d3);
  X* d4p = get_data<X>(d4);
  f(*dp, *d1p, *d2p, *d3p, *d4p);
}

template <class T>
template <class U, class V, class W, class X, class F>
void DVID<T>::mapv(const F& f, const DVID<U>& d1, const DVID<V>& d2,
                   const DVID<W>& d3, const DVID<X>& d4) {
  bcast_rpc_oneway(dvid_mapv_helper4<T,U,V,W,X,F>, dvid, d1.dvid, d2.dvid,
                   d3.dvid, d4.dvid, const_cast<F&>(f));
}

template <class T, class U, class V, class W, class X, class Y, class F>
void dvid_mapv_helper5(dvid_t& d, dvid_t& d1, dvid_t& d2, dvid_t& d3,
                       dvid_t& d4, dvid_t& d5, F& f) {
  T* dp = get_data<T>(d);
  U* d1p = get_data<U>(d1);
  V* d2p = get_data<V>(d2);
  W* d3p = get_data<W>(d3);
  X* d4p = get_data<X>(d4);
  Y* d5p = get_data<Y>(d5);
  f(*dp, *d1p, *d2p, *d3p, *d4p, *d5p);
}

template <class T>
template <class U, class V, class W, class X, class Y, class F>
void DVID<T>::mapv(const F& f, const DVID<U>& d1, const DVID<V>& d2,
                   const DVID<W>& d3, const DVID<X>& d4, const DVID<Y>& d5) {
  bcast_rpc_oneway(dvid_mapv_helper5<T,U,V,W,X,Y,F>, dvid, d1.dvid,
                   d2.dvid, d3.dvid, d4.dvid, d5.dvid, const_cast<F&>(f));
}

template <class T, class U, class V, class W, class X, class Y, class Z, class F>
void dvid_mapv_helper6(dvid_t& d, dvid_t& d1, dvid_t& d2, dvid_t& d3,
                       dvid_t& d4, dvid_t& d5, dvid_t& d6, F& f) {
  T* dp = get_data<T>(d);
  U* d1p = get_data<U>(d1);
  V* d2p = get_data<V>(d2);
  W* d3p = get_data<W>(d3);
  X* d4p = get_data<X>(d4);
  Y* d5p = get_data<Y>(d5);
  Z* d6p = get_data<Z>(d6);
  f(*dp, *d1p, *d2p, *d3p, *d4p, *d5p, *d6p);
}

template <class T>
template <class U, class V, class W, class X, class Y, class Z, class F>
void DVID<T>::mapv(const F& f, const DVID<U>& d1, const DVID<V>& d2,
                   const DVID<W>& d3, const DVID<X>& d4, const DVID<Y>& d5, const DVID<Z>& d6) {
  bcast_rpc_oneway(dvid_mapv_helper6<T,U,V,W,X,Y,Z,F>, dvid, d1.dvid,
                   d2.dvid, d3.dvid, d4.dvid, d5.dvid, d6.dvid, const_cast<F&>(f));
}

template <class T, class U, class V, class W, class X, class Y, class Z, class A, class F>
void dvid_mapv_helper7(dvid_t& d, dvid_t& d1, dvid_t& d2, dvid_t& d3,
                       dvid_t& d4, dvid_t& d5, dvid_t& d6, dvid_t& d7, F& f) {
  T* dp = get_data<T>(d);
  U* d1p = get_data<U>(d1);
  V* d2p = get_data<V>(d2);
  W* d3p = get_data<W>(d3);
  X* d4p = get_data<X>(d4);
  Y* d5p = get_data<Y>(d5);
  Z* d6p = get_data<Z>(d6);
  A* d7p = get_data<A>(d7);
  f(*dp, *d1p, *d2p, *d3p, *d4p, *d5p, *d6p, *d7p);
}

template <class T>
template <class U, class V, class W, class X, class Y, class Z, class A, class F>
void DVID<T>::mapv(const F& f, const DVID<U>& d1, const DVID<V>& d2,
                   const DVID<W>& d3, const DVID<X>& d4, const DVID<Y>& d5,
                   const DVID<Z>& d6, const DVID<A>& d7) {
  bcast_rpc_oneway(dvid_mapv_helper7<T,U,V,W,X,Y,Z,A,F>, dvid, d1.dvid,
                   d2.dvid, d3.dvid, d4.dvid, d5.dvid, d6.dvid, d7.dvid, const_cast<F&>(f));
}

template <class T, class U, class V, class W, class X, class Y, class Z, class A, class B, class F>
void dvid_mapv_helper8(dvid_t& d, dvid_t& d1, dvid_t& d2, dvid_t& d3,
                       dvid_t& d4, dvid_t& d5, dvid_t& d6, dvid_t& d7, dvid_t& d8, F& f) {
  T* dp = get_data<T>(d);
  U* d1p = get_data<U>(d1);
  V* d2p = get_data<V>(d2);
  W* d3p = get_data<W>(d3);
  X* d4p = get_data<X>(d4);
  Y* d5p = get_data<Y>(d5);
  Z* d6p = get_data<Z>(d6);
  A* d7p = get_data<A>(d7);
  B* d8p = get_data<B>(d8);
  f(*dp, *d1p, *d2p, *d3p, *d4p, *d5p, *d6p, *d7p, *d8p);
}

template <class T>
template <class U, class V, class W, class X, class Y, class Z, class A, class B, class F>
void DVID<T>::mapv(const F& f, const DVID<U>& d1, const DVID<V>& d2,
                   const DVID<W>& d3, const DVID<X>& d4, const DVID<Y>& d5,
                   const DVID<Z>& d6, const DVID<A>& d7, const DVID<B>& d8) {
  bcast_rpc_oneway(dvid_mapv_helper8<T,U,V,W,X,Y,Z,A,B,F>, dvid, d1.dvid,
                   d2.dvid, d3.dvid, d4.dvid, d5.dvid, d6.dvid, d7.dvid, d8.dvid, const_cast<F&>(f));
}

template <class V>
struct vector_sum_helper {};

template <>
struct vector_sum_helper<double> {
  vector_sum_helper(intptr_t addr) : addr(addr) {}
  vector_sum_helper() {}
  void operator()(std::vector<double>& vec) {
    if(get_selfid() == 0) {
      std::vector<double>& ret = *reinterpret_cast<std::vector<double>*>(addr);
      ret.resize(vec.size());
      MPI_Reduce(&vec[0], &ret[0], vec.size(), MPI_DOUBLE, MPI_SUM, 0,
                 frovedis_comm_rpc);
    } else {
      std::vector<double> ret(vec.size());
      MPI_Reduce(&vec[0], &ret[0], vec.size(), MPI_DOUBLE, MPI_SUM, 0,
                 frovedis_comm_rpc);
    }
  }
  intptr_t addr;

  SERIALIZE(addr)
};

template <>
struct vector_sum_helper<float> {
  vector_sum_helper(intptr_t addr) : addr(addr) {}
  vector_sum_helper() {}
  void operator()(std::vector<float>& vec) {
    if(get_selfid() == 0) {
      std::vector<float>& ret = *reinterpret_cast<std::vector<float>*>(addr);
      ret.resize(vec.size());
      MPI_Reduce(&vec[0], &ret[0], vec.size(), MPI_FLOAT, MPI_SUM, 0,
                 frovedis_comm_rpc);
    } else {
      std::vector<float> ret(vec.size());
      MPI_Reduce(&vec[0], &ret[0], vec.size(), MPI_FLOAT, MPI_SUM, 0,
                 frovedis_comm_rpc);
    }
  }
  intptr_t addr;

  SERIALIZE(addr)
};

template <>
struct vector_sum_helper<int> {
  vector_sum_helper(intptr_t addr) : addr(addr) {}
  vector_sum_helper() {}
  void operator()(std::vector<int>& vec) {
    if(get_selfid() == 0) {
      std::vector<int>& ret = *reinterpret_cast<std::vector<int>*>(addr);
      ret.resize(vec.size());
      MPI_Reduce(&vec[0], &ret[0], vec.size(), MPI_INT, MPI_SUM, 0,
                 frovedis_comm_rpc);
    } else {
      std::vector<int> ret(vec.size());
      MPI_Reduce(&vec[0], &ret[0], vec.size(), MPI_INT, MPI_SUM, 0,
                 frovedis_comm_rpc);
    }
  }
  intptr_t addr;

  SERIALIZE(addr)
};

template <>
struct vector_sum_helper<size_t> {
  vector_sum_helper(intptr_t addr) : addr(addr) {}
  vector_sum_helper() {}
  void operator()(std::vector<size_t>& vec) {
    if(get_selfid() == 0) {
      std::vector<size_t>& ret = *reinterpret_cast<std::vector<size_t>*>(addr);
      ret.resize(vec.size());
      if(sizeof(size_t) == 8)
	MPI_Reduce(&vec[0], &ret[0], vec.size(), MPI_UNSIGNED_LONG_LONG,
		   MPI_SUM, 0, frovedis_comm_rpc);
      else
	MPI_Reduce(&vec[0], &ret[0], vec.size(), MPI_UNSIGNED,
		   MPI_SUM, 0, frovedis_comm_rpc);
	
    } else {
      std::vector<size_t> ret(vec.size());
      if(sizeof(size_t) == 8)
	MPI_Reduce(&vec[0], &ret[0], vec.size(), MPI_UNSIGNED_LONG_LONG,
		   MPI_SUM, 0, frovedis_comm_rpc);
      else
	MPI_Reduce(&vec[0], &ret[0], vec.size(), MPI_UNSIGNED,
		   MPI_SUM, 0, frovedis_comm_rpc);
    }
  }
  intptr_t addr;

  SERIALIZE(addr)
};

template <class T>
T DVID<T>::vector_sum() {
  std::vector<typename T::value_type> ret;
  intptr_t addr = reinterpret_cast<intptr_t>(&ret);
  this->mapv(vector_sum_helper<typename T::value_type>(addr));
  return ret;
}

template <class T>
struct dvid_gather_helper {
  dvid_gather_helper(){}
  dvid_gather_helper(intptr_t ptr) : ptr(ptr) {}
  void operator()(T& data) {
    int self = get_selfid();
    int nodes = get_nodesize();
    std::ostringstream ss;
    my_oarchive outar(ss);
    outar << data;
    std::string sendbuf = ss.str();
    size_t send_size = sendbuf.size();

    std::vector<size_t> recvcounts(nodes);
    MPI_Gather(&send_size, sizeof(size_t), MPI_CHAR, 
               reinterpret_cast<char*>(&recvcounts[0]),
               sizeof(size_t), MPI_CHAR, 0, frovedis_comm_rpc);
    size_t total = 0;
    for(size_t i = 0; i < nodes; i++) total += recvcounts[i];
    std::vector<size_t> displs(nodes);
    if(self == 0) {
      auto displsp = displs.data();
      auto recvcountsp = recvcounts.data();
      for(size_t i = 1; i < nodes; i++) 
        displsp[i] = displsp[i-1] + recvcountsp[i-1];
    }
    std::string recvbuf;
    recvbuf.resize(total);
    char* recvbufp = &recvbuf[0];
    large_gatherv(sizeof(char), &sendbuf[0], send_size, recvbufp, recvcounts,
                  displs, 0, frovedis_comm_rpc);
    
    std::vector<T>& gatherv = *reinterpret_cast<std::vector<T>*>(ptr);
    if(self == 0) {
      for(size_t i = 0; i < nodes; i++) {
        std::string serbuf;
        serbuf.resize(recvcounts[i]);
        memcpy(&serbuf[0], recvbufp + displs[i], recvcounts[i]);
        std::istringstream inss(serbuf);
        my_iarchive inar(inss);
        inar >> gatherv[i];
      }
    }
  }
  intptr_t ptr;

  SERIALIZE(ptr)
};

template <class T>
std::vector<T> gather(DVID<T>& d) {
  std::vector<T> ret(get_nodesize());
  d.mapv(dvid_gather_helper<T>(reinterpret_cast<intptr_t>(&ret)));
  return ret;
}

template <class T>
struct dvid_gather_helper2 {
  dvid_gather_helper2(){}
  dvid_gather_helper2(intptr_t ptr) : ptr(ptr) {}
  void operator()(std::vector<T>& v) {
    int self = get_selfid();
    int nodes = get_nodesize();
    std::vector<size_t> recvcounts(nodes);
    size_t vsize = v.size();
    MPI_Gather(&vsize, sizeof(size_t), MPI_CHAR, 
               reinterpret_cast<char*>(&recvcounts[0]),
               sizeof(size_t), MPI_CHAR, 0, frovedis_comm_rpc);
    size_t total = 0;
    for(size_t i = 0; i < nodes; i++) total += recvcounts[i];
    std::vector<size_t> displs(nodes);
    if(self == 0) {
      for(size_t i = 1; i < nodes; i++) 
        displs[i] = displs[i-1] + recvcounts[i-1];
    }
    std::vector<T> tmp(total);
    T* tmpp = &tmp[0];
    large_gatherv(sizeof(T), reinterpret_cast<char*>(&v[0]), vsize, 
                  reinterpret_cast<char*>(tmpp), recvcounts,
                  displs, 0, frovedis_comm_rpc);
    
    std::vector<std::vector<T>>& gatherv = 
      *reinterpret_cast<std::vector<std::vector<T>>*>(ptr);
    if(self == 0) {
      for(size_t i = 0; i < nodes; i++) {
        gatherv[i].resize(recvcounts[i]);
        T* gathervp = &gatherv[i][0];
        for(size_t j = 0; j < recvcounts[i]; j++) {
          gathervp[j] = tmpp[j];
        }
        tmpp += recvcounts[i];
      }
    }
  }
  intptr_t ptr;

  SERIALIZE(ptr)
};

template <> 
std::vector<std::vector<double>> gather(DVID<std::vector<double>>& d);
template <> 
std::vector<std::vector<float>> gather(DVID<std::vector<float>>& d);
template <> 
std::vector<std::vector<int>> gather(DVID<std::vector<int>>& d);
template <>
std::vector<std::vector<size_t>> gather(DVID<std::vector<size_t>>& d);

template <class T>
void dvid_put_helper(dvid_t& dvid, int& node, intptr_t& valptr) {
  int self = get_selfid();
  if(self == 0) {
    T& val = *reinterpret_cast<T*>(valptr);
    send_data_helper(node, val);
  } else if(self == node) {
    T* data = get_data<T>(dvid);
    receive_data_helper(0, *data);
  }
}

template <class T>
void DVID<T>::put(int node, const T& val) {
  if(node >= get_nodesize()) throw std::runtime_error("put: invalid position");
  else if(node == 0) {
    *get_selfdata() = val;
  } else {
    intptr_t valptr = reinterpret_cast<intptr_t>(&val);
    bcast_rpc_oneway(dvid_put_helper<T>, dvid, node, valptr);
  }
}

template <class T>
void dvid_get_helper(dvid_t& dvid, int& node, intptr_t& dataptr) {
  int self = get_selfid();
  if(self == 0) {
    T& data = *reinterpret_cast<T*>(dataptr);
    receive_data_helper(node, data);
  } else if(self == node) {
    T* data = get_data<T>(dvid);
    send_data_helper(0, *data);
  }
}

template <class T>
T DVID<T>::get(int node) {
  if(node >= get_nodesize()) throw std::runtime_error("get: invalid position");
  else if(node == 0) {
    return *get_selfdata();
  } else {
    T data;
    intptr_t dataptr = reinterpret_cast<intptr_t>(&data);
    bcast_rpc_oneway(dvid_get_helper<T>, dvid, node, dataptr);
    return data;
  }
}

}
#endif
