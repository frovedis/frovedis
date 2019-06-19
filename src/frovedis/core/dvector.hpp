#ifndef DVECTOR_HPP
#define DVECTOR_HPP

#include <algorithm>
#include <iterator>
#ifdef USE_STD_RANDOM
#include <random>
#else
#include <stdlib.h>
#endif
#include <fstream>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "serfunc.hpp"
#include "DVID.hpp"
#include "node_local.hpp"
#include "serializehelper.hpp"
#include "mpihelper.hpp"
#include "type_utility.hpp"

#ifdef USE_CEREAL
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#endif
#ifdef USE_BOOST_SERIALIZATION
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#endif

namespace frovedis {

template <class K, class V> class dunordered_map;
template <class T> class dvector;
template <class T> std::vector<T> gather(dvector<T>& dv);

template <class T>
class dvector {
public:
  using value_type = T;
  // unlike normal container, do not allocate data by default ctor,
  // because it might be used by serialization (currently not used)
  dvector() : is_sizevec_valid(false), is_view(false) {} 
  dvector(const DVID<std::vector<T>>& dvid_, bool is_view_ = false) :
    dvid(dvid_), is_sizevec_valid(false), is_view(is_view_) {}
  dvector(DVID<std::vector<T>>&& dvid_, bool is_view_ = false) :
    dvid(std::move(dvid_)), is_sizevec_valid(false), is_view(is_view_) {}
  dvector(const dvector<T>& src) :
    dvid(src.dvid.copy()), 
    is_sizevec_valid(src.is_sizevec_valid), sizevec(src.sizevec),
    is_view(src.is_view) {}
  dvector(dvector<T>&& src) :
    is_sizevec_valid(src.is_sizevec_valid), sizevec(std::move(src.sizevec)), 
    is_view(src.is_view) {
    std::swap(dvid, src.dvid);
  }
  dvector<T>& operator=(const dvector<T>& src) {
    if(!is_view) dvid.delete_var();
    dvid = src.dvid.copy();
    is_sizevec_valid = src.is_sizevec_valid;
    sizevec = src.sizevec;
    is_view = src.is_view;
    return *this;
  }
  dvector<T>& operator=(dvector<T>&& src) {
    // dvid of original this will be deleted after swap if it is not view
    std::swap(dvid, src.dvid);
    is_sizevec_valid = src.is_sizevec_valid;
    std::swap(sizevec, src.sizevec);
    std::swap(is_view, src.is_view);
    return *this;
  }
  ~dvector(){if(!is_view) dvid.delete_var();}

  template <class R, class F> dvector<R> map(const F&);
  // TT: T, T&, const T&
  template <class R, class TT> dvector<R> map(R(*f)(TT)) {
    return map<R>(make_serfunc(f));
  }
  template <class R, class U, class F>
  dvector<R> map(const F&, const node_local<U>&);
  template <class R, class U, class TT, class UU>
  // TT: T, T&, const T&; UU: U, U&, const U&
  dvector<R> map(R(*f)(TT, UU), const node_local<U>& l) {
    return map<R>(make_serfunc(f),l);
  }
  template <class R, class U, class V, class F>
  dvector<R> map(const F&, const node_local<U>&, const node_local<V>&);
  template <class R, class U, class V, class TT, class UU, class VV>
  dvector<R> map(R(*f)(TT, UU, VV), const node_local<U>& l1,
                 const node_local<V>& l2) {
    return map<R>(make_serfunc(f),l1,l2);
  }
  template <class R, class U, class V, class W, class F>
  dvector<R> map(const F&, const node_local<U>&, const node_local<V>&,
                 const node_local<W>&);
  template <class R, class U, class V, class W,
            class TT, class UU, class VV, class WW>
  dvector<R> map(R(*f)(TT, UU, VV, WW), const node_local<U>& l1,
                 const node_local<V>& l2, const node_local<W>& l3) {
    return map<R>(make_serfunc(f),l1,l2,l3);
  }
  template <class R, class U, class V, class W, class X, class F>
  dvector<R> map(const F&, const node_local<U>&, const node_local<V>&,
                 const node_local<W>&, const node_local<X>&);
  template <class R, class U, class V, class W, class X,
            class TT, class UU, class VV, class WW, class XX>
  dvector<R> map(R(*f)(TT, UU, VV, WW, XX), const node_local<U>& l1,
                 const node_local<V>& l2, const node_local<W>& l3,
                 const node_local<X>& l4) {
    return map<R>(make_serfunc(f),l1,l2,l3,l4);
  }
  template <class R, class U, class V, class W, class X, class Y, class F>
  dvector<R> map(const F&, const node_local<U>&, const node_local<V>&,
                 const node_local<W>&, const node_local<X>&,
                 const node_local<Y>&);
  template <class R, class U, class V, class W, class X, class Y,
            class TT, class UU, class VV, class WW, class XX, class YY>
  dvector<R> map(R(*f)(TT, UU, VV, WW, XX, YY), const node_local<U>& l1,
                 const node_local<V>& l2, const node_local<W>& l3,
                 const node_local<X>& l4, const node_local<Y>& l5) {
    return map<R>(make_serfunc(f),l1,l2,l3,l4,l5);
  }
  template <class R, class U, class V, class W, class X, class Y, class Z, class F>
  dvector<R> map(const F&, const node_local<U>&, const node_local<V>&,
                 const node_local<W>&, const node_local<X>&,
                 const node_local<Y>&, const node_local<Z>&);
  template <class R, class U, class V, class W, class X, class Y, class Z,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ>
  dvector<R> map(R(*f)(TT, UU, VV, WW, XX, YY, ZZ), const node_local<U>& l1,
                 const node_local<V>& l2, const node_local<W>& l3,
                 const node_local<X>& l4, const node_local<Y>& l5,
                 const node_local<Z>& l6) {
    return map<R>(make_serfunc(f),l1,l2,l3,l4,l5,l6);
  }
  template <class R, class U, class V, class W, class X, class Y, class Z, class A, class F>
  dvector<R> map(const F&, const node_local<U>&, const node_local<V>&,
                 const node_local<W>&, const node_local<X>&,
                 const node_local<Y>&, const node_local<Z>&,
                 const node_local<A>&);
  template <class R, class U, class V, class W, class X, class Y, class Z, class A,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ, class AA>
  dvector<R> map(R(*f)(TT, UU, VV, WW, XX, YY, ZZ, AA), const node_local<U>& l1,
                 const node_local<V>& l2, const node_local<W>& l3,
                 const node_local<X>& l4, const node_local<Y>& l5,
                 const node_local<Z>& l6, const node_local<A>& l7) {
    return map<R>(make_serfunc(f),l1,l2,l3,l4,l5,l6,l7);
  }
  template <class R, class U, class V, class W, class X, class Y, class Z, class A, class B, class F>
  dvector<R> map(const F&, const node_local<U>&, const node_local<V>&,
                 const node_local<W>&, const node_local<X>&,
                 const node_local<Y>&, const node_local<Z>&,
                 const node_local<A>&, const node_local<B>&);
  template <class R, class U, class V, class W, class X, class Y, class Z, class A, class B,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ, class AA, class BB>
  dvector<R> map(R(*f)(TT, UU, VV, WW, XX, YY, ZZ, AA, BB), const node_local<U>& l1,
                 const node_local<V>& l2, const node_local<W>& l3,
                 const node_local<X>& l4, const node_local<Y>& l5,
                 const node_local<Z>& l6, const node_local<A>& l7,
                 const node_local<B>& l8) {
    return map<R>(make_serfunc(f),l1,l2,l3,l4,l5,l6,l7,l8);
  }

  template <typename F, typename... Types>
  dvector<map_result_t<F, T, Types...>> map(const F& f, const node_local<Types>&... ls) {
    return map<map_result_t<F, T, Types...>>(f, ls...);
  }

  template <class F> dvector<T>& mapv(const F&);
  template <class TT>
  dvector<T>& mapv(void(*f)(TT)){return mapv(make_serfunc(f));}
  template <class U, class F> dvector<T>& mapv(const F&, const node_local<U>&);
  template <class U, class TT, class UU>
  dvector<T>& mapv(void(*f)(TT,UU), const node_local<U>& l){
    return mapv(make_serfunc(f), l);
  }
  template <class U, class V, class F>
  dvector<T>& mapv(const F&, const node_local<U>&, const node_local<V>&);
  template <class U, class V, class TT, class UU, class VV>
  dvector<T>& mapv(void(*f)(TT,UU,VV), const node_local<U>& l1,
                   const node_local<V>& l2){
    return mapv(make_serfunc(f), l1, l2);
  }
  template <class U, class V, class W, class F>
  dvector<T>& mapv(const F&, const node_local<U>&, const node_local<V>&,
                   const node_local<W>&);
  template <class U, class V, class W,
            class TT, class UU, class VV, class WW>
  dvector<T>& mapv(void(*f)(TT,UU,VV,WW), const node_local<U>& l1,
                   const node_local<V>& l2, const node_local<W>& l3){
    return mapv(make_serfunc(f), l1, l2, l3);
  }
  template <class U, class V, class W, class X, class F>
  dvector<T>& mapv(const F&, const node_local<U>&, const node_local<V>&,
                   const node_local<W>&, const node_local<X>&);
  template <class U, class V, class W, class X,
            class TT, class UU, class VV, class WW, class XX>
  dvector<T>& mapv(void(*f)(TT,UU,VV,WW,XX), const node_local<U>& l1,
                   const node_local<V>& l2, const node_local<W>& l3,
                   const node_local<X>& l4){
    return mapv(make_serfunc(f), l1, l2, l3, l4);
  }
  template <class U, class V, class W, class X, class Y, class F>
  dvector<T>& mapv(const F&, const node_local<U>&, const node_local<V>&,
                   const node_local<W>&, const node_local<X>&,
                   const node_local<Y>&);
  template <class U, class V, class W, class X, class Y,
            class TT, class UU, class VV, class WW, class XX, class YY>
  dvector<T>& mapv(void(*f)(TT,UU,VV,WW,XX,YY), const node_local<U>& l1,
                   const node_local<V>& l2, const node_local<W>& l3,
                   const node_local<X>& l4, const node_local<Y>& l5){
    return mapv(make_serfunc(f), l1, l2, l3, l4, l5);
  }
  template <class U, class V, class W, class X, class Y, class Z, class F>
  dvector<T>& mapv(const F&, const node_local<U>&, const node_local<V>&,
                   const node_local<W>&, const node_local<X>&,
                   const node_local<Y>&, const node_local<Z>&);
  template <class U, class V, class W, class X, class Y, class Z,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ>
  dvector<T>& mapv(void(*f)(TT,UU,VV,WW,XX,YY,ZZ), const node_local<U>& l1,
                   const node_local<V>& l2, const node_local<W>& l3,
                   const node_local<X>& l4, const node_local<Y>& l5,
                   const node_local<Z>& l6){
    return mapv(make_serfunc(f), l1, l2, l3, l4, l5, l6);
  }
  template <class U, class V, class W, class X, class Y, class Z, class A, class F>
  dvector<T>& mapv(const F&, const node_local<U>&, const node_local<V>&,
                   const node_local<W>&, const node_local<X>&,
                   const node_local<Y>&, const node_local<Z>&,
                   const node_local<A>&);
  template <class U, class V, class W, class X, class Y, class Z, class A,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ, class AA>
  dvector<T>& mapv(void(*f)(TT,UU,VV,WW,XX,YY,ZZ,AA), const node_local<U>& l1,
                   const node_local<V>& l2, const node_local<W>& l3,
                   const node_local<X>& l4, const node_local<Y>& l5,
                   const node_local<Z>& l6, const node_local<A>& l7){
    return mapv(make_serfunc(f), l1, l2, l3, l4, l5, l6, l7);
  }
  template <class U, class V, class W, class X, class Y, class Z, class A, class B, class F>
  dvector<T>& mapv(const F&, const node_local<U>&, const node_local<V>&,
                   const node_local<W>&, const node_local<X>&,
                   const node_local<Y>&, const node_local<Z>&,
                   const node_local<A>&, const node_local<B>&);
  template <class U, class V, class W, class X, class Y, class Z, class A, class B,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ, class AA, class BB>
  dvector<T>& mapv(void(*f)(TT,UU,VV,WW,XX,YY,ZZ,AA,BB), const node_local<U>& l1,
                   const node_local<V>& l2, const node_local<W>& l3,
                   const node_local<X>& l4, const node_local<Y>& l5,
                   const node_local<Z>& l6, const node_local<A>& l7,
                   const node_local<B>& l8){
    return mapv(make_serfunc(f), l1, l2, l3, l4, l5, l6, l7, l8);
  }

  template <class R, class F> dvector<R> map_partitions(const F& f) {
    return dvector<R>(dvid.template map<std::vector<R>>(f));
  }
  template <class R, class TT>
  dvector<R> map_partitions(std::vector<R>(*f)(TT)){
    return map_partitions<R>(make_serfunc(f));
  }
  template <class R, class U, class F>
  dvector<R> map_partitions(const F& f, const node_local<U>& l) {
    return dvector<R>(dvid.template map<std::vector<R>>(f,l.get_dvid()));
  }
  template <class R, class U, class TT, class UU>
  dvector<R> map_partitions(std::vector<R>(*f)(TT, UU),
                            const node_local<U>& l) {
    return map_partitions<R>(make_serfunc(f),l);
  }
  template <class R, class U, class V, class F>
  dvector<R> map_partitions(const F& f, const node_local<U>& l1,
                            const node_local<V>& l2) {
    return dvector<R>(dvid.template map<std::vector<R>>
                      (f,l1.get_dvid(),l2.get_dvid()));
  }
  template <class R, class U, class V, class TT, class UU, class VV>
  dvector<R> map_partitions(std::vector<R>(*f)(TT, UU, VV),
                            const node_local<U>& l1, const node_local<V>& l2) {
    return map_partitions<R>(make_serfunc(f),l1,l2);
  }
  template <class R, class U, class V, class W, class F>
  dvector<R> map_partitions(const F& f, const node_local<U>& l1,
                            const node_local<V>& l2, const node_local<W>& l3) {
    return dvector<R>(dvid.template map<std::vector<R>>
                      (f,l1.get_dvid(),l2.get_dvid(),l3.get_dvid()));
  }
  template <class R, class U, class V, class W,
            class TT, class UU, class VV, class WW>
  dvector<R> map_partitions(std::vector<R>(*f)(TT, UU, VV, WW),
                            const node_local<U>& l1, const node_local<V>& l2,
                            const node_local<W>& l3) {
    return map_partitions<R>(make_serfunc(f),l1,l2,l3);
  }
  template <class R, class U, class V, class W, class X, class F>
  dvector<R> map_partitions(const F& f, const node_local<U>& l1,
                            const node_local<V>& l2, const node_local<W>& l3,
                            const node_local<X>& l4) {
    return dvector<R>(dvid.template map<std::vector<R>>
                      (f,l1.get_dvid(),l2.get_dvid(),l3.get_dvid(),
                       l4.get_dvid()));
  }
  template <class R, class U, class V, class W, class X,
            class TT, class UU, class VV, class WW, class XX>
  dvector<R> map_partitions(std::vector<R>(*f)(TT, UU, VV, WW, XX),
                            const node_local<U>& l1, const node_local<V>& l2,
                            const node_local<W>& l3, const node_local<X>& l4) {
    return map_partitions<R>(make_serfunc(f),l1,l2,l3,l4);
  }
  template <class R, class U, class V, class W, class X, class Y, class F>
  dvector<R> map_partitions(const F& f, const node_local<U>& l1,
                            const node_local<V>& l2, const node_local<W>& l3,
                            const node_local<X>& l4, const node_local<Y>& l5) {
    return dvector<R>(dvid.template map<std::vector<R>>
                      (f,l1.get_dvid(),l2.get_dvid(),l3.get_dvid(),
                       l4.get_dvid(), l5.get_dvid()));
  }
  template <class R, class U, class V, class W, class X, class Y,
            class TT, class UU, class VV, class WW, class XX, class YY>
  dvector<R> map_partitions(std::vector<R>(*f)(TT, UU, VV, WW, XX,YY),
                            const node_local<U>& l1, const node_local<V>& l2,
                            const node_local<W>& l3, const node_local<X>& l4,
                            const node_local<Y>& l5) {
    return map_partitions<R>(make_serfunc(f),l1,l2,l3,l4,l5);
  }
  template <class R, class U, class V, class W, class X, class Y, class Z, class F>
  dvector<R> map_partitions(const F& f, const node_local<U>& l1,
                            const node_local<V>& l2, const node_local<W>& l3,
                            const node_local<X>& l4, const node_local<Y>& l5,
                            const node_local<Z>& l6) {
    return dvector<R>(dvid.template map<std::vector<R>>
                      (f,l1.get_dvid(),l2.get_dvid(),l3.get_dvid(),
                       l4.get_dvid(), l5.get_dvid(), l6.get_dvid() ));
  }
  template <class R, class U, class V, class W, class X, class Y, class Z,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ>
  dvector<R> map_partitions(std::vector<R>(*f)(TT, UU, VV, WW, XX, YY, ZZ),
                            const node_local<U>& l1, const node_local<V>& l2,
                            const node_local<W>& l3, const node_local<X>& l4,
                            const node_local<Y>& l5, const node_local<Z>& l6) {
    return map_partitions<R>(make_serfunc(f),l1,l2,l3,l4,l5,l6);
  }
  template <class R, class U, class V, class W, class X, class Y, class Z, class A, class F>
  dvector<R> map_partitions(const F& f, const node_local<U>& l1,
                            const node_local<V>& l2, const node_local<W>& l3,
                            const node_local<X>& l4, const node_local<Y>& l5,
                            const node_local<Z>& l6, const node_local<A>& l7) {
    return dvector<R>(dvid.template map<std::vector<R>>
                      (f,l1.get_dvid(),l2.get_dvid(),l3.get_dvid(),
                       l4.get_dvid(), l5.get_dvid(), l6.get_dvid(),
                       l7.get_dvid() ));
  }
  template <class R, class U, class V, class W, class X, class Y, class Z, class A,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ, class AA>
  dvector<R> map_partitions(std::vector<R>(*f)(TT, UU, VV, WW, XX, YY, ZZ, AA),
                            const node_local<U>& l1, const node_local<V>& l2,
                            const node_local<W>& l3, const node_local<X>& l4,
                            const node_local<Y>& l5, const node_local<Z>& l6,
                            const node_local<A>& l7) {
    return map_partitions<R>(make_serfunc(f),l1,l2,l3,l4,l5,l6,l7);
  }
  template <class R, class U, class V, class W, class X, class Y, class Z, class A, class B, class F>
  dvector<R> map_partitions(const F& f, const node_local<U>& l1,
                            const node_local<V>& l2, const node_local<W>& l3,
                            const node_local<X>& l4, const node_local<Y>& l5,
                            const node_local<Z>& l6, const node_local<A>& l7,
                            const node_local<B>& l8) {
    return dvector<R>(dvid.template map<std::vector<R>>
                      (f,l1.get_dvid(),l2.get_dvid(),l3.get_dvid(),
                       l4.get_dvid(), l5.get_dvid(), l6.get_dvid(),
                       l7.get_dvid(), l8.get_dvid() ));
  }
  template <class R, class U, class V, class W, class X, class Y, class Z, class A, class B,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ, class AA, class BB>
  dvector<R> map_partitions(std::vector<R>(*f)(TT, UU, VV, WW, XX, YY, ZZ, AA, BB),
                            const node_local<U>& l1, const node_local<V>& l2,
                            const node_local<W>& l3, const node_local<X>& l4,
                            const node_local<Y>& l5, const node_local<Z>& l6,
                            const node_local<A>& l7, const node_local<B>& l8) {
    return map_partitions<R>(make_serfunc(f),l1,l2,l3,l4,l5,l6,l7,l8);
  }

  template <typename F, typename... Types>
  dvector<map_result_t<F, T, Types...>> map_partitions(const F& f, const node_local<Types>&... ls) {
    return map_partitions<map_result_t<F, T, Types...>>(f, ls...);
  }

  template <class F> dvector<T>& mapv_partitions(const F& f) {
    dvid.mapv(f); return *this;
  }
  template <class TT> dvector<T>&  mapv_partitions(void(*f)(TT)) {
    return mapv_partitions(make_serfunc(f));
  }
  template <class U, class F>
  dvector<T>& mapv_partitions(const F& f, const node_local<U>& l) {
    dvid.mapv(f,l.get_dvid()); return *this;
  }
  template <class U, class TT, class UU>
  dvector<T>& mapv_partitions(void(*f)(TT, UU), const node_local<U>& l) {
    return mapv_partitions(make_serfunc(f),l);
  }
  template <class U, class V, class F>
  dvector<T>& mapv_partitions(const F& f, const node_local<U>& l1,
                              const node_local<V>& l2) {
    dvid.mapv(f,l1.get_dvid(),l2.get_dvid()); return *this;
  }
  template <class U, class V, class TT, class UU, class VV>
  dvector<T>& mapv_partitions(void(*f)(TT, UU, VV), const node_local<U>& l1,
                              const node_local<V>& l2) {
    return mapv_partitions(make_serfunc(f),l1,l2);
  }
  template <class U, class V, class W, class F>
  dvector<T>& mapv_partitions(const F& f, const node_local<U>& l1,
                              const node_local<V>& l2,
                              const node_local<W>& l3) {
    dvid.mapv(f,l1.get_dvid(),l2.get_dvid(),l3.get_dvid()); return *this;
  }
  template <class U, class V, class W,
            class TT, class UU, class VV, class WW>
  dvector<T>& mapv_partitions(void(*f)(TT, UU, VV,WW), const node_local<U>& l1,
                              const node_local<V>& l2,
                              const node_local<W>& l3) {
    return mapv_partitions(make_serfunc(f),l1,l2,l3);
  }
  template <class U, class V, class W, class X, class F>
  dvector<T>& mapv_partitions(const F& f, const node_local<U>& l1,
                              const node_local<V>& l2, const node_local<W>& l3,
                              const node_local<X>& l4) {
    dvid.mapv(f,l1.get_dvid(),l2.get_dvid(),l3.get_dvid(),l4.get_dvid());
    return *this;
  }
  template <class U, class V, class W, class X,
            class TT, class UU, class VV, class WW, class XX>
  dvector<T>& mapv_partitions(void(*f)(TT, UU, VV, WW, XX),
                              const node_local<U>& l1, const node_local<V>& l2,
                              const node_local<W>& l3,
                              const node_local<X>& l4) {
    return mapv_partitions(make_serfunc(f),l1,l2,l3,l4);
  }
  template <class U, class V, class W, class X, class Y, class F>
  dvector<T>& mapv_partitions(const F& f, const node_local<U>& l1,
                              const node_local<V>& l2, const node_local<W>& l3,
                              const node_local<X>& l4,
                              const node_local<Y>& l5) {
    dvid.mapv(f,l1.get_dvid(),l2.get_dvid(),l3.get_dvid(),l4.get_dvid(),
              l5.get_dvid());
    return *this;
  }
  template <class U, class V, class W, class X, class Y,
            class TT, class UU, class VV, class WW, class XX, class YY>
  dvector<T>& mapv_partitions(void(*f)(TT, UU, VV, WW, XX, YY),
                              const node_local<U>& l1, const node_local<V>& l2,
                              const node_local<W>& l3, const node_local<X>& l4,
                              const node_local<Y>& l5) {
    return mapv_partitions(make_serfunc(f),l1,l2,l3,l4,l5);
  }
  template <class U, class V, class W, class X, class Y, class Z, class F>
  dvector<T>& mapv_partitions(const F& f, const node_local<U>& l1,
                              const node_local<V>& l2, const node_local<W>& l3,
                              const node_local<X>& l4, const node_local<Y>& l5,
                              const node_local<Z>& l6) {
    dvid.mapv(f,l1.get_dvid(),l2.get_dvid(),l3.get_dvid(),l4.get_dvid(),
              l5.get_dvid(), l6.get_dvid());
    return *this;
  }
  template <class U, class V, class W, class X, class Y, class Z,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ>
  dvector<T>& mapv_partitions(void(*f)(TT, UU, VV, WW, XX, YY, ZZ),
                              const node_local<U>& l1, const node_local<V>& l2,
                              const node_local<W>& l3, const node_local<X>& l4,
                              const node_local<Y>& l5, const node_local<Z>& l6) {
    return mapv_partitions(make_serfunc(f),l1,l2,l3,l4,l5,l6);
  }
  template <class U, class V, class W, class X, class Y, class Z, class A, class F>
  dvector<T>& mapv_partitions(const F& f, const node_local<U>& l1,
                              const node_local<V>& l2, const node_local<W>& l3,
                              const node_local<X>& l4, const node_local<Y>& l5,
                              const node_local<Z>& l6, const node_local<A>& l7 ) {
    dvid.mapv(f,l1.get_dvid(),l2.get_dvid(),l3.get_dvid(),l4.get_dvid(),
              l5.get_dvid(), l6.get_dvid(), l7.get_dvid());
    return *this;
  }
  template <class U, class V, class W, class X, class Y, class Z, class A,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ, class AA>
  dvector<T>& mapv_partitions(void(*f)(TT, UU, VV, WW, XX, YY, ZZ, AA),
                              const node_local<U>& l1, const node_local<V>& l2,
                              const node_local<W>& l3, const node_local<X>& l4,
                              const node_local<Y>& l5, const node_local<Z>& l6,
                              const node_local<A>& l7) {
    return mapv_partitions(make_serfunc(f),l1,l2,l3,l4,l5,l6,l7);
  }
  template <class U, class V, class W, class X, class Y, class Z, class A, class B, class F>
  dvector<T>& mapv_partitions(const F& f, const node_local<U>& l1,
                              const node_local<V>& l2, const node_local<W>& l3,
                              const node_local<X>& l4, const node_local<Y>& l5,
                              const node_local<Z>& l6, const node_local<A>& l7,
                              const node_local<B>& l8 ) {
    dvid.mapv(f,l1.get_dvid(),l2.get_dvid(),l3.get_dvid(),l4.get_dvid(),
              l5.get_dvid(), l6.get_dvid(), l7.get_dvid(), l8.get_dvid());
    return *this;
  }
  template <class U, class V, class W, class X, class Y, class Z, class A, class B,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ, class AA, class BB>
  dvector<T>& mapv_partitions(void(*f)(TT, UU, VV, WW, XX, YY, ZZ, AA, BB),
                              const node_local<U>& l1, const node_local<V>& l2,
                              const node_local<W>& l3, const node_local<X>& l4,
                              const node_local<Y>& l5, const node_local<Z>& l6,
                              const node_local<A>& l7, const node_local<B>& l8) {
    return mapv_partitions(make_serfunc(f),l1,l2,l3,l4,l5,l6,l7,l8);
  }

  template <class F> T reduce(const F&);
  template <class TT, class UU>
  T reduce(T(*f)(TT,UU)) {return reduce(make_serfunc(f));}
  /* do we need filter with node_local? */
  template <class F> dvector<T> filter(const F&);
  template <class TT>
  dvector<T> filter(bool(*f)(TT)){return filter(make_serfunc(f));}
  template <class F> dvector<T>& inplace_filter(const F&);
  template <class TT>
  dvector<T>& inplace_filter(bool(*f)(TT)) {
    return inplace_filter(make_serfunc(f));
  }
  /* do we need flat_map with node_local? */
  template <class R, class F> dvector<R> flat_map(const F&);
  template <class R, class TT> dvector<R> flat_map(std::vector<R>(*f)(TT)) {
    return flat_map<R>(make_serfunc(f));
  }

  void clear();

  std::vector<T> gather() {return frovedis::gather(*this);}

  const std::vector<size_t>& sizes() {
    if(is_sizevec_valid) return sizevec;
    else {
      set_sizes();
      return sizevec;
    }
  }
  void set_sizes() const;
  void set_sizes(const std::vector<size_t>& sizes_) const {
    sizevec = sizes_;
    is_sizevec_valid = true;
  }
  size_t size() const;

  dvector<T>& align_as(const std::vector<size_t>&);
  template <class U>
  dvector<T>& align_to(dvector<U>&);
  dvector<T>& align_block();

  void save(const std::string& path, const std::string& delim);
  void saveline(const std::string& path) {save(path, std::string("\n"));}

  void savebinary(const std::string& path);

  void put(size_t pos, const T& val);
  T get(size_t pos);

  // implemented in dunordered_map.hpp
  // define your own hash function in std name space, if needed
  // (giving hash function as template is not supported)
  template <class K, class V>
  dunordered_map<K,std::vector<V>> group_by_key();

  template <class K, class V, class F>
  dunordered_map<K,V> reduce_by_key(const F& f);
  template <class K, class V, class VV, class WW>
  dunordered_map<K,V> reduce_by_key(V(*f)(VV,WW)){
    return reduce_by_key<K,V>(make_serfunc(f));
  }

  node_local<std::vector<T>> as_node_local() const {
    return node_local<std::vector<T>>(dvid.copy());
  }
  node_local<std::vector<T>> moveto_node_local() {
    node_local<std::vector<T>> r(dvid);
    dvid.clear_dvid();
    return r;
  }
  node_local<std::vector<T>> viewas_node_local() {
    return node_local<std::vector<T>>(dvid, true);
  }

  /*
    if rate < 1.0, local partition candidates are randomely selected,
    which makes partition phase faster, but accuracy of partition worse
  */
  dvector<T>& sort(double rate = 1.1);

  template <class F>
  dvector<T>& sort(F f, double rate = 1.1);

  dvector<T> head(size_t size);
  dvector<T> tail(size_t size);

  DVID<std::vector<T>> get_dvid() const {return dvid;}
private:
  DVID<std::vector<T>> dvid;
  mutable bool is_sizevec_valid; 
  mutable std::vector<size_t> sizevec;
  bool is_view;

  SERIALIZE(dvid, is_sizevec_valid, sizevec, is_view)
};

template <class T>
dvector<T> make_dvector_allocate() {
  return dvector<T>(make_dvid_allocate<std::vector<T>>());
}

// TODO: direct implemeantation instead of using DVID<T>::scatter
template <class T>
dvector<T> make_dvector_scatter(const std::vector<T>& src,
                                const std::vector<size_t>& sizevec) {
  int node_size = get_nodesize();
  std::vector<std::vector<T>> src2(node_size);
  const T* srcp = &src[0];
  for(size_t i = 0; i < node_size; i++) {
    src2[i].resize(sizevec[i]);
    for(size_t j = 0; j < sizevec[i]; j++) {
      src2[i][j] = srcp[j];
    }
    srcp += sizevec[i];
  }
  auto d = make_dvid_scatter(src2);
  return dvector<T>(d);
}

template <class T>
struct make_dvector_scatter_helper2 {
  make_dvector_scatter_helper2(){}
  make_dvector_scatter_helper2(intptr_t ptr, 
                               const std::vector<size_t>& sizevec) : 
    ptr(ptr), sizevec(sizevec) {}
  void operator()(std::vector<T>& dst) {
    int self = get_selfid();
    int nodes = get_nodesize();
    dst.resize(sizevec[self]);
    T* dstp = &dst[0];
    std::vector<T>& src = *reinterpret_cast<std::vector<T>*>(ptr);
    T* srcp;
    if(self == 0) srcp = &src[0]; else srcp = 0;
    std::vector<size_t> displs(nodes);
    auto displsp = displs.data();
    auto sizevecp = sizevec.data();
#pragma _NEC novector
    for(size_t i = 1; i < nodes; i++) displsp[i] = displsp[i-1] + sizevecp[i-1];
    large_scatterv(sizeof(T), reinterpret_cast<char*>(srcp), sizevec, displs, 
                   reinterpret_cast<char*>(dstp), sizevec[self], 0, 
                   frovedis_comm_rpc);
  }
  intptr_t ptr;
  std::vector<size_t> sizevec;

  SERIALIZE(ptr, sizevec)
};

template <>
dvector<double> make_dvector_scatter(const std::vector<double>& src,
                                     const std::vector<size_t>& sizevec);

template <>
dvector<float> make_dvector_scatter(const std::vector<float>& src,
                                    const std::vector<size_t>& sizevec);

template <>
dvector<int> make_dvector_scatter(const std::vector<int>& src,
                                  const std::vector<size_t>& sizevec);

template <>
dvector<size_t> make_dvector_scatter(const std::vector<size_t>& src,
                                     const std::vector<size_t>& sizevec);

template <class T>
dvector<T> make_dvector_scatter(const std::vector<T>& src) {
  size_t total = src.size();
  int nodes = get_nodesize();
  size_t each = ceil_div(total, static_cast<size_t>(nodes));
  std::vector<size_t> sizevec(nodes);
  auto sizevecp = sizevec.data();
  size_t crnt = 0;
  size_t end = src.size();
  for(int i = 0; i < nodes; i++) {
    size_t part_end = crnt + each < end ? crnt + each : end;
    size_t part_size = part_end - crnt;
    sizevecp[i] = part_size;
    crnt = part_end;
  }
  return make_dvector_scatter(src, sizevec);
}

template <class T = std::string>
dvector<T> make_dvector_load(const std::string& path,
                             const std::string& delim);

template <>
dvector<std::string>
make_dvector_load<std::string>(const std::string& path,
                               const std::string& delim);

template <class T>
T make_dvector_load_helper(std::string& s) {
  std::istringstream iss(s);
  T r;
  iss >> r;
  return r;
}

template <class T>
dvector<T> make_dvector_load(const std::string& path,
                             const std::string& delim) {
  return make_dvector_load<std::string>(path, delim).
    map<T>(make_dvector_load_helper<T>);
}

template <class T = std::string>
dvector<T> make_dvector_loadline(const std::string& path) {
  return make_dvector_load<T>(path, std::string("\n"));
}

template <class T, class R, class F>
class map_dvector {
public:
  map_dvector(){}
  map_dvector(const F& f) : func(f) {}
  std::vector<R> operator()(std::vector<T>& vec) {
    std::vector<R> dst(vec.size());
    for(size_t i = 0; i < vec.size(); i++) {
      dst[i] = func(vec[i]);
    }
    return dst;
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class R, class F>
dvector<R> dvector<T>::map(const F& f) {
  // "template" before map is needed to tell the compiler map is a template 
  // http://stackoverflow.com/questions/1574721/g-doesnt-like-template-method-chaining-on-template-var
  dvector<R> ret(dvid.template map<std::vector<R>>(map_dvector<T,R,F>(f)));
  if(is_sizevec_valid) ret.set_sizes(sizevec);
  return ret;
}

template <class T, class R, class U, class F>
class map_dvector_nloc1 {
public:
  map_dvector_nloc1(){}
  map_dvector_nloc1(const F& f) : func(f) {}
  std::vector<R> operator()(std::vector<T>& vec, U& lv1) {
    std::vector<R> dst(vec.size());
    for(size_t i = 0; i < vec.size(); i++) {
      dst[i] = func(vec[i], lv1);
    }
    return dst;
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class R, class U, class F>
dvector<R> dvector<T>::map(const F& f, const node_local<U>& l) {
  dvector<R> ret(dvid.template map<std::vector<R>, U>
                 (map_dvector_nloc1<T,R,U,F>(f), l.get_dvid()));
  if(is_sizevec_valid) ret.set_sizes(sizevec);
  return ret;
}

template <class T, class R, class U, class V, class F>
class map_dvector_nloc2 {
public:
  map_dvector_nloc2(){}
  map_dvector_nloc2(const F& f) : func(f) {}
  std::vector<R> operator()(std::vector<T>& vec, U& lv1, V& lv2) {
    std::vector<R> dst(vec.size());
    for(size_t i = 0; i < vec.size(); i++) {
      dst[i] = func(vec[i], lv1,lv2);
    }
    return dst;
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class R, class U, class V, class F>
dvector<R> dvector<T>::map(const F& f, const node_local<U>& l1,
                           const node_local<V>& l2) {
  dvector<R> ret(dvid.template map<std::vector<R>>
                 (map_dvector_nloc2<T,R,U,V,F>(f),
                  l1.get_dvid(),
                  l2.get_dvid()));
  if(is_sizevec_valid) ret.set_sizes(sizevec);
  return ret;
}

template <class T, class R, class U, class V, class W, class F>
class map_dvector_nloc3 {
public:
  map_dvector_nloc3(){}
  map_dvector_nloc3(const F& f) : func(f) {}
  std::vector<R> operator()(std::vector<T>& vec, U& lv1, V& lv2, W& lv3) {
    std::vector<R> dst(vec.size());
    for(size_t i = 0; i < vec.size(); i++) {
      dst[i] = func(vec[i], lv1, lv2, lv3);
    }
    return dst;
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class R, class U, class V, class W, class F>
dvector<R> dvector<T>::map(const F& f, const node_local<U>& l1,
                           const node_local<V>& l2, const node_local<W>& l3) {
  dvector<R> ret(dvid.template map<std::vector<R>>
                 (map_dvector_nloc3<T,R,U,V,W,F>(f),
                  l1.get_dvid(),
                  l2.get_dvid(),
                  l3.get_dvid()));
  if(is_sizevec_valid) ret.set_sizes(sizevec);
  return ret;
}

template <class T, class R, class U, class V, class W, class X, class F>
class map_dvector_nloc4 {
public:
  map_dvector_nloc4(){}
  map_dvector_nloc4(const F& f) : func(f) {}
  std::vector<R> operator()(std::vector<T>& vec, U& lv1, V& lv2, W& lv3,
                            X& lv4) {
    std::vector<R> dst(vec.size());
    for(size_t i = 0; i < vec.size(); i++) {
      dst[i] = func(vec[i], lv1, lv2, lv3, lv4);
    }
    return dst;
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class R, class U, class V, class W, class X, class F>
dvector<R> dvector<T>::map(const F& f, const node_local<U>& l1,
                           const node_local<V>& l2, const node_local<W>& l3,
                           const node_local<X>& l4) {
  dvector<R> ret(dvid.template map<std::vector<R>>
                 (map_dvector_nloc4<T,R,U,V,W,X,F>(f),
                  l1.get_dvid(),
                  l2.get_dvid(),
                  l3.get_dvid(),
                  l4.get_dvid()));
  if(is_sizevec_valid) ret.set_sizes(sizevec);
  return ret;
}

template <class T, class R, class U, class V, class W, class X, class Y,
          class F>
class map_dvector_nloc5 {
public:
  map_dvector_nloc5(){}
  map_dvector_nloc5(const F& f) : func(f) {}
  std::vector<R> operator()(std::vector<T>& vec, U& lv1, V& lv2, W& lv3,
                            X& lv4, Y& lv5) {
    std::vector<R> dst(vec.size());
    for(size_t i = 0; i < vec.size(); i++) {
      dst[i] = func(vec[i], lv1, lv2, lv3, lv4, lv5);
    }
    return dst;
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class R, class U, class V, class W, class X, class Y, class F>
dvector<R> dvector<T>::map(const F& f, const node_local<U>& l1,
                           const node_local<V>& l2, const node_local<W>& l3,
                           const node_local<X>& l4, const node_local<Y>& l5) {
  dvector<R> ret(dvid.template map<std::vector<R>>
                 (map_dvector_nloc5<T,R,U,V,W,X,Y,F>(f),
                  l1.get_dvid(),
                  l2.get_dvid(),
                  l3.get_dvid(),
                  l4.get_dvid(),
                  l5.get_dvid()));
  if(is_sizevec_valid) ret.set_sizes(sizevec);
  return ret;
}

template <class T, class R, class U, class V, class W, class X, class Y, class Z,
          class F>
class map_dvector_nloc6 {
public:
  map_dvector_nloc6(){}
  map_dvector_nloc6(const F& f) : func(f) {}
  std::vector<R> operator()(std::vector<T>& vec, U& lv1, V& lv2, W& lv3,
                            X& lv4, Y& lv5, Z& lv6) {
    std::vector<R> dst(vec.size());
    for(size_t i = 0; i < vec.size(); i++) {
      dst[i] = func(vec[i], lv1, lv2, lv3, lv4, lv5, lv6);
    }
    return dst;
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class R, class U, class V, class W, class X, class Y, class Z, class F>
dvector<R> dvector<T>::map(const F& f, const node_local<U>& l1,
                           const node_local<V>& l2, const node_local<W>& l3,
                           const node_local<X>& l4, const node_local<Y>& l5,
                           const node_local<Z>& l6) {
  dvector<R> ret(dvid.template map<std::vector<R>>
                 (map_dvector_nloc6<T,R,U,V,W,X,Y,Z,F>(f),
                  l1.get_dvid(),
                  l2.get_dvid(),
                  l3.get_dvid(),
                  l4.get_dvid(),
                  l5.get_dvid(),
                  l6.get_dvid()));
  if(is_sizevec_valid) ret.set_sizes(sizevec);
  return ret;
}

template <class T, class R, class U, class V, class W, class X, class Y, class Z, class A,
          class F>
class map_dvector_nloc7 {
public:
  map_dvector_nloc7(){}
  map_dvector_nloc7(const F& f) : func(f) {}
  std::vector<R> operator()(std::vector<T>& vec, U& lv1, V& lv2, W& lv3,
                            X& lv4, Y& lv5, Z& lv6, A& lv7) {
    std::vector<R> dst(vec.size());
    for(size_t i = 0; i < vec.size(); i++) {
      dst[i] = func(vec[i], lv1, lv2, lv3, lv4, lv5, lv6, lv7);
    }
    return dst;
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class R, class U, class V, class W, class X, class Y, class Z, class A, class F>
dvector<R> dvector<T>::map(const F& f, const node_local<U>& l1,
                           const node_local<V>& l2, const node_local<W>& l3,
                           const node_local<X>& l4, const node_local<Y>& l5,
                           const node_local<Z>& l6, const node_local<A>& l7) {
  dvector<R> ret(dvid.template map<std::vector<R>>
                 (map_dvector_nloc7<T,R,U,V,W,X,Y,Z,A,F>(f),
                  l1.get_dvid(),
                  l2.get_dvid(),
                  l3.get_dvid(),
                  l4.get_dvid(),
                  l5.get_dvid(),
                  l6.get_dvid(),
                  l7.get_dvid()));
  if(is_sizevec_valid) ret.set_sizes(sizevec);
  return ret;
}

template <class T, class R, class U, class V, class W, class X, class Y, class Z, class A, class B,
          class F>
class map_dvector_nloc8 {
public:
  map_dvector_nloc8(){}
  map_dvector_nloc8(const F& f) : func(f) {}
  std::vector<R> operator()(std::vector<T>& vec, U& lv1, V& lv2, W& lv3,
                            X& lv4, Y& lv5, Z& lv6, A& lv7, B& lv8) {
    std::vector<R> dst(vec.size());
    for(size_t i = 0; i < vec.size(); i++) {
      dst[i] = func(vec[i], lv1, lv2, lv3, lv4, lv5, lv6, lv7, lv8);
    }
    return dst;
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class R, class U, class V, class W, class X, class Y, class Z, class A, class B, class F>
dvector<R> dvector<T>::map(const F& f, const node_local<U>& l1,
                           const node_local<V>& l2, const node_local<W>& l3,
                           const node_local<X>& l4, const node_local<Y>& l5,
                           const node_local<Z>& l6, const node_local<A>& l7,
                           const node_local<B>& l8) {
  dvector<R> ret(dvid.template map<std::vector<R>>
                 (map_dvector_nloc8<T,R,U,V,W,X,Y,Z,A,B,F>(f),
                  l1.get_dvid(),
                  l2.get_dvid(),
                  l3.get_dvid(),
                  l4.get_dvid(),
                  l5.get_dvid(),
                  l6.get_dvid(),
                  l7.get_dvid(),
                  l8.get_dvid()));
  if(is_sizevec_valid) ret.set_sizes(sizevec);
  return ret;
}

template <class T, class F>
class mapv_dvector {
public:
  mapv_dvector(){}
  mapv_dvector(const F& f) : func(f) {}
  void operator()(std::vector<T>& vec) {
    for(size_t i = 0; i < vec.size(); i++) {
      func(vec[i]);
    }
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class F>
dvector<T>& dvector<T>::mapv(const F& f) {
  dvid.mapv(mapv_dvector<T,F>(f));
  return *this;
}

template <class T, class U, class F>
class mapv_dvector_nloc1 {
public:
  mapv_dvector_nloc1(){}
  mapv_dvector_nloc1(const F& f) : func(f) {}
  void operator()(std::vector<T>& vec, U& lv1) {
    for(size_t i = 0; i < vec.size(); i++) {
      func(vec[i],lv1);
    }
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class U, class F>
dvector<T>& dvector<T>::mapv(const F& f, const node_local<U>& l) {
  dvid.mapv(mapv_dvector_nloc1<T,U,F>(f),l.get_dvid());
  return *this;
}

template <class T, class U, class V, class F>
class mapv_dvector_nloc2 {
public:
  mapv_dvector_nloc2(){}
  mapv_dvector_nloc2(const F& f) : func(f) {}
  void operator()(std::vector<T>& vec, U& lv1, V& lv2) {
    for(size_t i = 0; i < vec.size(); i++) {
      func(vec[i],lv1,lv2);
    }
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class U, class V, class F>
dvector<T>& dvector<T>::mapv(const F& f, const node_local<U>& l1,
                             const node_local<V>& l2) {
  dvid.mapv(mapv_dvector_nloc2<T,U,V,F>(f),l1.get_dvid(),l2.get_dvid());
  return *this;
}

template <class T, class U, class V, class W, class F>
class mapv_dvector_nloc3 {
public:
  mapv_dvector_nloc3(){}
  mapv_dvector_nloc3(const F& f) : func(f) {}
  void operator()(std::vector<T>& vec, U& lv1, V& lv2, W& lv3) {
    for(size_t i = 0; i < vec.size(); i++) {
      func(vec[i],lv1,lv2,lv3);
    }
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class U, class V, class W, class F>
dvector<T>& dvector<T>::mapv(const F& f, const node_local<U>& l1,
                             const node_local<V>& l2,
                             const node_local<W>& l3) {
  dvid.mapv(mapv_dvector_nloc3<T,U,V,W,F>(f),l1.get_dvid(),l2.get_dvid(),
            l3.get_dvid());
  return *this;
}

template <class T, class U, class V, class W, class X, class F>
class mapv_dvector_nloc4 {
public:
  mapv_dvector_nloc4(){}
  mapv_dvector_nloc4(const F& f) : func(f) {}
  void operator()(std::vector<T>& vec, U& lv1, V& lv2, W& lv3, X& lv4) {
    for(size_t i = 0; i < vec.size(); i++) {
      func(vec[i],lv1,lv2,lv3,lv4);
    }
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class U, class V, class W, class X, class F>
dvector<T>& dvector<T>::mapv(const F& f, const node_local<U>& l1,
                             const node_local<V>& l2, const node_local<W>& l3,
                             const node_local<X>& l4) {
  dvid.mapv(mapv_dvector_nloc4<T,U,V,W,X,F>(f),l1.get_dvid(),l2.get_dvid(),
            l3.get_dvid(), l4.get_dvid());
  return *this;
}

template <class T, class U, class V, class W, class X, class Y, class F>
class mapv_dvector_nloc5 {
public:
  mapv_dvector_nloc5(){}
  mapv_dvector_nloc5(const F& f) : func(f) {}
  void operator()(std::vector<T>& vec, U& lv1, V& lv2, W& lv3, X& lv4,
                  Y& lv5) {
    for(size_t i = 0; i < vec.size(); i++) {
      func(vec[i],lv1,lv2,lv3,lv4,lv5);
    }
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class U, class V, class W, class X, class Y, class F>
dvector<T>& dvector<T>::mapv(const F& f, const node_local<U>& l1,
                             const node_local<V>& l2, const node_local<W>& l3,
                             const node_local<X>& l4,
                             const node_local<Y>& l5) {
  dvid.mapv(mapv_dvector_nloc5<T,U,V,W,X,Y,F>(f),l1.get_dvid(),l2.get_dvid(),
            l3.get_dvid(), l4.get_dvid(), l5.get_dvid());
  return *this;
}

template <class T, class U, class V, class W, class X, class Y, class Z, class F>
class mapv_dvector_nloc6 {
public:
  mapv_dvector_nloc6(){}
  mapv_dvector_nloc6(const F& f) : func(f) {}
  void operator()(std::vector<T>& vec, U& lv1, V& lv2, W& lv3, X& lv4,
                  Y& lv5, Z& lv6) {
    for(size_t i = 0; i < vec.size(); i++) {
      func(vec[i],lv1,lv2,lv3,lv4,lv5,lv6);
    }
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class U, class V, class W, class X, class Y, class Z, class F>
dvector<T>& dvector<T>::mapv(const F& f, const node_local<U>& l1,
                             const node_local<V>& l2, const node_local<W>& l3,
                             const node_local<X>& l4, const node_local<Y>& l5,
                             const node_local<Z>& l6) {
  dvid.mapv(mapv_dvector_nloc6<T,U,V,W,X,Y,Z,F>(f),l1.get_dvid(),l2.get_dvid(),
            l3.get_dvid(), l4.get_dvid(), l5.get_dvid(), l6.get_dvid());
  return *this;
}

template <class T, class U, class V, class W, class X, class Y, class Z, class A, class F>
class mapv_dvector_nloc7 {
public:
  mapv_dvector_nloc7(){}
  mapv_dvector_nloc7(const F& f) : func(f) {}
  void operator()(std::vector<T>& vec, U& lv1, V& lv2, W& lv3, X& lv4,
                  Y& lv5, Z& lv6, A& lv7) {
    for(size_t i = 0; i < vec.size(); i++) {
      func(vec[i],lv1,lv2,lv3,lv4,lv5,lv6,lv7);
    }
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class U, class V, class W, class X, class Y, class Z, class A, class F>
dvector<T>& dvector<T>::mapv(const F& f, const node_local<U>& l1,
                             const node_local<V>& l2, const node_local<W>& l3,
                             const node_local<X>& l4, const node_local<Y>& l5,
                             const node_local<Z>& l6, const node_local<A>& l7) {
  dvid.mapv(mapv_dvector_nloc7<T,U,V,W,X,Y,Z,A,F>(f),l1.get_dvid(),l2.get_dvid(),
            l3.get_dvid(), l4.get_dvid(), l5.get_dvid(), l6.get_dvid(), l7.get_dvid());
  return *this;
}

template <class T, class U, class V, class W, class X, class Y, class Z, class A, class B, class F>
class mapv_dvector_nloc8 {
public:
  mapv_dvector_nloc8(){}
  mapv_dvector_nloc8(const F& f) : func(f) {}
  void operator()(std::vector<T>& vec, U& lv1, V& lv2, W& lv3, X& lv4,
                  Y& lv5, Z& lv6, A& lv7, B& lv8) {
    for(size_t i = 0; i < vec.size(); i++) {
      func(vec[i],lv1,lv2,lv3,lv4,lv5,lv6,lv7,lv8);
    }
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class U, class V, class W, class X, class Y, class Z, class A, class B, class F>
dvector<T>& dvector<T>::mapv(const F& f, const node_local<U>& l1,
                             const node_local<V>& l2, const node_local<W>& l3,
                             const node_local<X>& l4, const node_local<Y>& l5,
                             const node_local<Z>& l6, const node_local<A>& l7,
                             const node_local<B>& l8) {
  dvid.mapv(mapv_dvector_nloc8<T,U,V,W,X,Y,Z,A,B,F>(f),l1.get_dvid(),l2.get_dvid(),
            l3.get_dvid(), l4.get_dvid(), l5.get_dvid(), l6.get_dvid(), l7.get_dvid(), l8.get_dvid());
  return *this;
}

template <class T, class F>
class reduce_dvector {
public:
  reduce_dvector(){}
  reduce_dvector(const F& f) : func(f) {}
  T operator()(std::vector<T>& vec) {
    if(vec.size() == 0) return T();
    else {
      T current = vec[0];
      for(size_t i = 1; i < vec.size(); i++) {
        current = func(current, vec[i]);
      }
      return current;
    }
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class F>
T dvector<T>::reduce(const F& f) {
  auto tmp = dvid.template map<T>(reduce_dvector<T,F>(f));
  auto ret = tmp.reduce(f);
  tmp.delete_var();
  return ret;
}

template <class T, class F>
class filter_dvector {
public:
  filter_dvector(){}
  filter_dvector(const F& f) : func(f) {}
  std::vector<T> operator()(std::vector<T>& vec) {
    std::vector<T> dst;
    dst.reserve(vec.size());
    for(size_t i = 0; i < vec.size(); i++) {
      if(func(vec[i])) dst.push_back(vec[i]);
    }
    std::vector<T> dst2;
    dst2 = dst; // shrink memory size
    return dst2;
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class F>
dvector<T> dvector<T>::filter(const F& f) {
  return dvector<T>(dvid.template map<std::vector<T>>
                    (filter_dvector<T,F>(f)));  
}

template <class T, class F>
struct my_not {
  my_not(){}
  my_not(const F& f) : func(f) {}
  bool operator()(const T& val){return !func(val);}
  F func;
  SERIALIZE(func)
};

template <class T, class F>
class inplace_filter_dvector {
public:
  inplace_filter_dvector(){}
  inplace_filter_dvector(const F& f) : func(f) {}
  void operator()(std::vector<T>& vec) {
    vec.erase(remove_if(vec.begin(), vec.end(), my_not<T,F>(func)), vec.end());
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class F>
dvector<T>& dvector<T>::inplace_filter(const F& f) {
  dvid.mapv(inplace_filter_dvector<T,F>(f));
  is_sizevec_valid = false;
  return *this;
}

template <class T>
void clear_vector(std::vector<T>& vec){
  std::vector<T> tmp;
  vec.swap(tmp);
}

template <class T>
void dvector<T>::clear() {
  dvid.mapv(make_serfunc(clear_vector<T>));
  is_sizevec_valid = false;
}

template <class T, class R, class F>
class flatmap_dvector {
public:
  flatmap_dvector(){}
  flatmap_dvector(const F& f) : func(f) {}
  std::vector<R> operator()(std::vector<T>& vec) {
    std::vector<R> dst;
    for(size_t i = 0; i < vec.size(); i++) {
      std::vector<R> tmp = func(vec[i]);
      dst.insert(dst.end(), tmp.begin(), tmp.end());
    }
    return dst;
  }
  F func;
  SERIALIZE(func)
};

template <class T>
template <class R, class F>
dvector<R> dvector<T>::flat_map(const F& f) {
  return dvector<R>(dvid.template map<std::vector<R>>
                    (flatmap_dvector<T,R,F>(f)));
}

// TODO: direct implemeantation instead of using DVID<T>::gather
template <class T>
std::vector<T> gather(dvector<T>& dv) {
  auto tmp = dv.get_dvid().gather();
  size_t total = 0;
  for(size_t i = 0; i < tmp.size(); i++) total += tmp[i].size();
  std::vector<T> ret(total);
  T* retp = &ret[0];
  for(size_t i = 0; i < tmp.size(); i++) {
    for(size_t j = 0; j < tmp[i].size(); j++) {
      retp[j] = tmp[i][j];
    }
    retp += tmp[i].size();
  }
  return ret;
}

template <class T>
struct dvector_gather_helper2 {
  dvector_gather_helper2(){}
  dvector_gather_helper2(intptr_t ptr) : ptr(ptr) {}
  void operator()(std::vector<T>& v) {
    int self = get_selfid();
    int nodes = get_nodesize();
    std::vector<size_t> recvcounts(nodes);
    size_t vsize = v.size();
    MPI_Gather(&vsize, sizeof(size_t), MPI_CHAR, 
               reinterpret_cast<char*>(&recvcounts[0]),
               sizeof(size_t), MPI_CHAR, 0, MPI_COMM_WORLD);
    size_t total = 0;
    for(size_t i = 0; i < nodes; i++) total += recvcounts[i];
    std::vector<size_t> displs(nodes);
    if(self == 0) {
      for(size_t i = 1; i < nodes; i++) 
        displs[i] = displs[i-1] + recvcounts[i-1];
    }
    T* gathervp;
    if(self == 0) {
      std::vector<T>& gatherv = *reinterpret_cast<std::vector<T>*>(ptr);
      gatherv.resize(total);
      gathervp = &gatherv[0];
    } else gathervp = 0;
    // defined in DVID.cc
    large_gatherv(sizeof(T), reinterpret_cast<char*>(&v[0]), vsize, 
                  reinterpret_cast<char*>(gathervp), recvcounts,
                  displs, 0, MPI_COMM_WORLD);
  }
  intptr_t ptr;

  SERIALIZE(ptr)
};

template <>
std::vector<double> gather(dvector<double>& dv);
template <>
std::vector<float> gather(dvector<float>& dv);
template <>
std::vector<int> gather(dvector<int>& dv);
template <>
std::vector<size_t> gather(dvector<size_t>& dv);

template <class T>
size_t sizes_helper(DVID<T>& dvid) {
  return dvid.get_selfdata()->size();
}

template <class T>
void dvector<T>::set_sizes() const {
  sizevec = bcast_rpc(sizes_helper<std::vector<T>>, dvid);
  is_sizevec_valid = true;
}

template <class T>
size_t dvector<T>::size() const {
  if(!is_sizevec_valid) set_sizes();
  size_t total = 0;
  for(size_t i = 0; i < sizevec.size(); i++) total += sizevec[i];
  return total;
}

template <class T>
void align_as_align_pod(std::vector<T>& src, std::vector<T>& dst,
                        std::vector<std::vector<size_t>>& alltoall_sizes) {
  int node_size = get_nodesize();
  int self = get_selfid();
  std::vector<size_t> sendcounts(node_size), recvcounts(node_size);
  for(int i = 0; i < node_size; i++) {
    sendcounts[i] = alltoall_sizes[self][i];
    recvcounts[i] = alltoall_sizes[i][self];
  }
  size_t total_recv = 0;
  for(int i = 0; i < node_size; i++) total_recv += recvcounts[i];
  std::vector<size_t> sdispls(node_size), rdispls(node_size);
  for(int i = 1; i < node_size; i++) {
    sdispls[i] = sdispls[i-1] + sendcounts[i-1];
    rdispls[i] = rdispls[i-1] + recvcounts[i-1];
  }
  dst.resize(total_recv);
  large_alltoallv(sizeof(T), reinterpret_cast<char*>(&src[0]), sendcounts, sdispls,
                  reinterpret_cast<char*>(&dst[0]), recvcounts, rdispls, frovedis_comm_rpc);
}

template <class T>
void align_as_align(std::vector<T>& src, std::vector<T>& dst,
                    std::vector<std::vector<size_t>>& alltoall_sizes);

template <>
void align_as_align(std::vector<double>& src, std::vector<double>& dst,
                    std::vector<std::vector<size_t>>& alltoall_sizes);

template <>
void align_as_align(std::vector<float>& src, std::vector<float>& dst,
                    std::vector<std::vector<size_t>>& alltoall_sizes);

template <>
void align_as_align(std::vector<int>& src, std::vector<int>& dst,
                    std::vector<std::vector<size_t>>& alltoall_sizes);

template <>
void align_as_align(std::vector<size_t>& src, std::vector<size_t>& dst,
                    std::vector<std::vector<size_t>>& alltoall_sizes);

template <class T>
void align_as_align(std::vector<T>& src, std::vector<T>& dst,
                    std::vector<std::vector<size_t>>& alltoall_sizes) {
  int node_size = get_nodesize();
  int self = get_selfid();
  std::vector<std::string> srcservec(node_size);
  std::vector<size_t> ser_sendcounts(node_size);
  T* crntsrcp = &src[0];
  size_t total = 0;
  for(int i = 0; i < node_size; i++) {
    size_t tmp_size = alltoall_sizes[self][i];
    std::vector<T> tmp(tmp_size);
    for(size_t j = 0; j < tmp_size; j++) {
      tmp[j] = crntsrcp[j];
    }
    crntsrcp += tmp_size;
    std::ostringstream ss;
    my_oarchive outar(ss);
    outar << tmp;
    srcservec[i] = ss.str();
    ser_sendcounts[i] = srcservec[i].size();
    total += srcservec[i].size();
  }
  std::string srcser;
  srcser.resize(total);
  char* crntsrcserp = &srcser[0];
  for(int i = 0; i < node_size; i++) {
    memcpy(crntsrcserp, &srcservec[i][0], ser_sendcounts[i]);
    crntsrcserp += ser_sendcounts[i];
  }
  std::vector<size_t> all_sendcounts(node_size * node_size);
  MPI_Allgather(&ser_sendcounts[0], node_size * sizeof(size_t), MPI_CHAR,
                &all_sendcounts[0], node_size * sizeof(size_t), MPI_CHAR,
                frovedis_comm_rpc);
  std::vector<size_t> ser_recvcounts(node_size);
  for(int i = 0; i < node_size; i++) {
    ser_recvcounts[i] = all_sendcounts[node_size * i + self];
  }
  total = 0;
  for(int i = 0; i < node_size; i++) total += ser_recvcounts[i];
  std::string dstser;
  dstser.resize(total);
  std::vector<size_t> sdispls(node_size), rdispls(node_size);
  for(int i = 1; i < node_size; i++) {
    sdispls[i] = sdispls[i-1] + ser_sendcounts[i-1];
    rdispls[i] = rdispls[i-1] + ser_recvcounts[i-1];
  }
  large_alltoallv(sizeof(char), &srcser[0], ser_sendcounts, sdispls,
                  &dstser[0], ser_recvcounts, rdispls, frovedis_comm_rpc);
  total = 0;
  for(int i = 0; i < node_size; i++) total += alltoall_sizes[i][self];
  dst.resize(total);
  T* crntdstp = &dst[0];
  for(int i = 0; i < node_size; i++) {
    std::string sertmp;
    sertmp.resize(ser_recvcounts[i]);
    memcpy(&sertmp[0], &dstser[0] + rdispls[i], ser_recvcounts[i]);
    std::istringstream inss(sertmp);
    my_iarchive inar(inss);
    std::vector<T> tmp;
    inar >> tmp;
    size_t tmpsize = tmp.size();
    for(size_t j = 0; j < tmpsize; j++) {
      crntdstp[j] = tmp[j];
    }
    crntdstp += tmpsize;
  }
}

std::vector<size_t> 
align_as_calc_alltoall_sizes(std::vector<size_t>&, std::vector<size_t>&);

template <class T>
dvector<T>& dvector<T>::align_as(const std::vector<size_t>& dst) {
  if(is_view)
    throw std::runtime_error("currently align on view is not allowed");
  if(dst.size() != get_nodesize()) 
    throw std::runtime_error
      ("align_as: size of dst is not equal to node size");
  auto mysizes = sizes();
  size_t dsttotal = 0;
  size_t selftotal = 0;
  for(size_t i = 0; i < dst.size(); i++) dsttotal += dst[i];
  for(size_t i = 0; i < mysizes.size(); i++) selftotal += mysizes[i];
  if(dsttotal != selftotal)
    throw std::runtime_error
      ("align_as: total size of src and dst does not match");
  bool is_same = true;
  for(size_t i = 0; i < dst.size(); i++) {
    if(dst[i] != mysizes[i]) {
      is_same = false;
      break;
    }
  }
  if(is_same) return *this;
  DVID<std::vector<T>> tmp = make_dvid_allocate<std::vector<T>>();
  auto alltoall_sizes = bcast_rpc(align_as_calc_alltoall_sizes, mysizes,
                                  const_cast<std::vector<size_t>&>(dst));
  auto bcast_alltoall_sizes = make_dvid_broadcast(alltoall_sizes);
  dvid.mapv(align_as_align<T>, tmp, bcast_alltoall_sizes);
  bcast_alltoall_sizes.delete_var();
  dvid.delete_var();
  dvid = tmp.dvid;
  sizevec = dst;
  is_sizevec_valid = true;
  return *this;
}

template <class T>
template <class U>
dvector<T>& dvector<T>::align_to(dvector<U>& dvec) {
  auto sizes = dvec.sizes();
  return align_as(sizes);
}

template <class T>
dvector<T>& dvector<T>::align_block() {
  size_t sz = size();
  size_t nodesize = get_nodesize();
  std::vector<size_t> block_size(nodesize);
  size_t each = ceil_div(sz, nodesize);
  for(size_t i = 0; i < nodesize; i++) {
    if(sz > each) {
      block_size[i] = each;
      sz -= each;
    } else {
      block_size[i] = sz;
      break;
    }
  }
  return align_as(block_size);
}

template <class T>
struct dvector_save_mpi_helper {
  dvector_save_mpi_helper(){}
  dvector_save_mpi_helper(const std::string& path,
                          const std::string& delim,
                          bool is_sequential_save) :
    path(path), delim(delim), is_sequential_save(is_sequential_save) {}
  void operator()(std::vector<T>& v) {
    std::ostringstream oss;
    for(size_t i = 0; i < v.size(); i++) {
      oss << v[i] << delim;
    }
    size_t nodes = get_nodesize();
    size_t self = get_selfid();
    std::vector<size_t> sizes(nodes);
    auto result = oss.str();
    size_t mysize = result.size();
    MPI_Allgather(&mysize, sizeof(mysize), MPI_CHAR, &sizes[0], sizeof(mysize),
                  MPI_CHAR, frovedis_comm_rpc);
    size_t myoff = 0;
    for(size_t i = 0; i < self; i++) myoff += sizes[i];

    if(is_sequential_save) {
      for(size_t i = 0; i < nodes; i++) {
        if(i == self) {
          int fd = open(path.c_str(), O_RDWR);
          if(fd == -1)
            throw std::runtime_error("open " + path + " error: " +
                                     std::string(strerror(errno)));
          auto ret = lseek(fd, myoff, SEEK_SET);
          if(ret == -1)
            throw std::runtime_error("lseek " + path + " error: " +
                                     std::string(strerror(errno)));
          ssize_t to_write = mysize;
          ssize_t pos = 0;
          char* write_data = &result[0];
          while(true) {
            auto written = write(fd, write_data+pos, to_write);
            if(written == -1) {
              throw std::runtime_error("write " + path + " error: " +
                                       std::string(strerror(errno)));
            } else if(written < to_write) {
              to_write -= written;
              pos += written;
            } else break;
          }
          close(fd);
        }
        MPI_Barrier(frovedis_comm_rpc);
      }
    } else {
      MPI_File fh;
      int ret = MPI_File_open(frovedis_comm_rpc,
                              const_cast<char*>(path.c_str()),
                              MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
      if(ret != 0) throw std::runtime_error("error in MPI_File_open");
      char datarep[] = "native";
      MPI_File_set_view(fh, myoff, MPI_CHAR, MPI_CHAR, datarep, MPI_INFO_NULL);

      char* write_data = &result[0];
      size_t maxsize = 0;
      size_t* sizesp = &sizes[0];
      for(size_t i = 0; i < nodes; i++) {
        if(maxsize < sizesp[i]) maxsize = sizesp[i];
      }
      size_t numiter = ceil_div(maxsize, mpi_max_count);
      size_t to_write = mysize;
      size_t pos = 0;
      for(size_t i = 0; i < numiter; i++) {
        size_t write_size = std::min(to_write, mpi_max_count);
        MPI_Status st;
        ret = MPI_File_write_all(fh, write_data+pos, write_size, MPI_CHAR, &st);
        if(ret != 0) throw std::runtime_error("error in MPI_File_write_all");
        pos += write_size;
        if(to_write > mpi_max_count) to_write -= mpi_max_count;
        else to_write = 0;
      }
      MPI_File_close(&fh);
    }
  }
  std::string path;
  std::string delim;
  bool is_sequential_save;
  SERIALIZE(path, delim, is_sequential_save)
};

/*
  TODO: support hdfs
  in the case of hdfs, path is directory;
  otherwise, path is file, and sequentially written
*/

void save_file_checker(const std::string& path);
bool is_sequential_save();

template <class T>
void dvector<T>::save(const std::string& path, const std::string& delim) {
  auto idx = path.find(':', 0);
  if(idx == std::string::npos) {
    save_file_checker(path);
    mapv_partitions(dvector_save_mpi_helper<T>(path, delim,
                                               is_sequential_save()));
    return;
  } else {
    if(path.substr(0, idx) == "file") {
      if(path.substr(idx + 1, 2) == "//") {
        auto newpath = path.substr(idx + 3);
        save_file_checker(newpath);
        mapv_partitions(dvector_save_mpi_helper<T>(newpath, delim,
                                                   is_sequential_save()));
        return;
      } else {
        auto newpath = path.substr(idx + 1);
        save_file_checker(newpath);
        mapv_partitions(dvector_save_mpi_helper<T>(newpath, delim,
                                                   is_sequential_save()));
        return;
      } 
    } else {
      throw
        std::runtime_error("scheme " + path.substr(0, idx) +
                           " is not implemented yet.");
    }
  }
}

template <class T>
void put_dvector_helper(size_t& node, DVID<std::vector<T>>& dvid,
                        size_t& pos, intptr_t& valptr) {
  int self = get_selfid();
  if(self == 0) {
    T& val = *reinterpret_cast<T*>(valptr);
    send_data_helper(node, val);
  } else if(self == node) {
    receive_data_helper(0, (*dvid.get_selfdata())[pos]);
  }
}

template <class T>
void dvector<T>::put(size_t pos, const T& val) {
  auto v = sizes();
  if(pos < v[0]) {
    (*dvid.get_selfdata())[pos] = val;
    return;
  } else {
    pos -= v[0];
    for(size_t i = 1; i < v.size(); i++) {
      if(pos < v[i]) {
        intptr_t valptr = reinterpret_cast<intptr_t>(&val);
        bcast_rpc_oneway(put_dvector_helper<T>, i, dvid, pos, valptr);
        return;
      } else pos -= v[i];
    }      
  }
  throw std::runtime_error("put: invalid position");
}

template <class T>
void get_dvector_helper(size_t& node, DVID<std::vector<T>>& dvid,
                        size_t& pos, intptr_t& dataptr) {
  int self = get_selfid();
  if(self == 0) {
    T& data = *reinterpret_cast<T*>(dataptr);
    receive_data_helper(node, data);
  } else if(self == node) {
    send_data_helper(0, (*dvid.get_selfdata())[pos]);
  }
}

template <class T>
T dvector<T>::get(size_t pos) {
  auto v = sizes();
  if(pos < v[0]) {
    return (*dvid.get_selfdata())[pos];
  } else {
    pos -= v[0];
    for(size_t i = 1; i < v.size(); i++) {
      if(pos < v[i]) {
        T data;
        intptr_t dataptr = reinterpret_cast<intptr_t>(&data);
        bcast_rpc_oneway(get_dvector_helper<T>, i, dvid, pos, dataptr);
        return data;
      } else pos -= v[i];
    }
  }
  throw std::runtime_error("get: invalid position");
}

/*
  node_local::as_dvector
*/
template <class T>
template <class U>
dvector<U> node_local<T>::as_dvector() const {
  return dvector<U>(dvid.copy());
}

/*
  node_local::moveto_dvector
*/
template <class T>
template <class U>
dvector<U> node_local<T>::moveto_dvector() {
  dvector<U> ret(dvid);
  dvid.clear_dvid();
  return ret;
}

/*
  node_local::viewas_dvector
*/
template <class T>
template <class U>
dvector<U> node_local<T>::viewas_dvector() {
  return dvector<U>(dvid, true);
}

template <class T>
std::vector<std::string>
to_serialized_binary(std::vector<T>& v) {
  std::vector<std::string> ret(v.size());
  for(size_t i = 0; i < v.size(); i++) {
    std::ostringstream ss;
    my_oarchive outar(ss);
    outar << v[i];
    ret[i] = std::move(ss.str());
  }
  return ret;
}

struct alltoall_getsizes {
  std::vector<size_t> operator()(std::vector<std::string>& v) {
    std::vector<size_t> ret;
    for(auto& i: v) ret.push_back(i.size());
    return ret;
  }
  SERIALIZE_NONE
};

// almost same as large_alltoallv, though arguments are different
// separately implemented by historical reason...
struct call_mpi_alltoallv {
  call_mpi_alltoallv(){}
  call_mpi_alltoallv(std::vector<std::vector<size_t>> sizes) : sizes(sizes) {}
  std::vector<std::string> operator()(std::vector<std::string>& ser) {
    int nodesize = get_nodesize();
    std::vector<size_t> sendcounts(nodesize);
    for(int i = 0; i < nodesize; i++) {
      sendcounts[i] = sizes[get_selfid()][i];
    }
    std::vector<size_t> recvcounts(nodesize);
    for(int i = 0; i < nodesize; i++) {
      recvcounts[i] = sizes[i][get_selfid()];
    }
    std::vector<std::string> recv(nodesize);
    for(int i = 0; i < nodesize; i++) {
      recv[i].resize(recvcounts[i]);
    }
    std::vector<size_t> spos(nodesize);
    std::vector<size_t> rpos(nodesize);
    std::vector<int> each_sendcounts(nodesize);
    std::vector<int> each_recvcounts(nodesize);
    std::vector<int> each_sdispls(nodesize);
    std::vector<int> each_rdispls(nodesize);
    size_t local_max_count = mpi_max_count / nodesize;
    while(true) {
      for(int i = 0; i < nodesize; i++) {
        each_sendcounts[i] = std::min(sendcounts[i]-spos[i], local_max_count);
        each_recvcounts[i] = std::min(recvcounts[i]-rpos[i], local_max_count);
      }
      size_t total_send_size = 0;
      for(int i = 0; i < nodesize; i++)
        total_send_size += each_sendcounts[i];
      std::string sendbuf;
      sendbuf.resize(total_send_size);
      each_sdispls[0] = 0;
      for(int i = 1; i < nodesize; i++) {
        each_sdispls[i] = each_sdispls[i-1] + each_sendcounts[i-1];
      }
      for(int i = 0; i < nodesize; i++) {
        memcpy(&sendbuf[each_sdispls[i]], &ser[i][spos[i]],
               each_sendcounts[i]);
      }
      size_t total_recv_size = 0;
      for(int i = 0; i < nodesize; i++)
        total_recv_size += each_recvcounts[i];
      std::string recvbuf;
      recvbuf.resize(total_recv_size);
      each_rdispls[0] = 0;
      for(int i = 1; i < nodesize; i++) {
        each_rdispls[i] = each_rdispls[i-1] + each_recvcounts[i-1];
      }
      MPI_Alltoallv(&sendbuf[0], &each_sendcounts[0], &each_sdispls[0],
                    MPI_CHAR, &recvbuf[0], &each_recvcounts[0],
                    &each_rdispls[0], MPI_CHAR, frovedis_comm_rpc);
      for(int i = 0; i < nodesize; i++) {
        memcpy(&recv[i][rpos[i]], &recvbuf[each_rdispls[i]],
               each_recvcounts[i]);
      }
      for(int i = 0; i < nodesize; i++) {
        spos[i] += each_sendcounts[i];
        rpos[i] += each_recvcounts[i];
      }
      int done = 1;
      for(int i = 0; i < nodesize; i++) {
        if(sendcounts[i] != spos[i] || recvcounts[i] != rpos[i]) {
          done = 0; break;
        }
      }
      int done_all = 0;
      MPI_Allreduce(&done, &done_all, 1, MPI_INT, MPI_LAND, frovedis_comm_rpc);
      if(done_all) break;
    }
    return recv;
  }
  std::vector<std::vector<size_t>> sizes;
  SERIALIZE(sizes)
};

template <class T>
std::vector<T> deserialize_binary(std::vector<std::string>& ser) {
  std::vector<T> ret(get_nodesize());
  for(int i = 0; i < get_nodesize(); i++) {
    std::istringstream inss(ser[i]); // TODO: remove this copy
    my_iarchive inar(inss);
    inar >> ret[i];
  }
  return ret;
}

template <class TT>
DVID<std::vector<TT>>
alltoall_exchange(DVID<std::vector<TT>>& src) {
  auto serialized = src.map(to_serialized_binary<TT>);
  auto sizes = serialized.
    template map<std::vector<size_t>>(alltoall_getsizes()).gather();
  auto sent = serialized.
    template map<std::vector<std::string>>(call_mpi_alltoallv(sizes));
  serialized.delete_var();
  auto ret = sent.map(deserialize_binary<TT>);
  sent.delete_var();
  return ret;
}

template <class T>
struct alltoall_getsizes_pod {
  std::vector<size_t> operator()(std::vector<std::vector<T>>& v) {
    std::vector<size_t> ret(v.size());
    size_t* retp = &ret[0];
    for(size_t i = 0; i < v.size(); i++) retp[i] = v[i].size();
    return ret;
  }
  SERIALIZE_NONE
};

template <class T>
struct call_mpi_alltoallv_pod {
  call_mpi_alltoallv_pod (){}
  call_mpi_alltoallv_pod(const std::vector<std::vector<size_t>>& sizes) :
    alltoall_sizes(sizes) {}
  call_mpi_alltoallv_pod(std::vector<std::vector<size_t>>&& sizes) :
    alltoall_sizes(std::move(sizes)) {}
  std::vector<std::vector<T>> operator()(std::vector<std::vector<T>>& src) {
    int node_size = get_nodesize();
    int self = get_selfid();
    std::vector<size_t> sendcounts(node_size), recvcounts(node_size);
    size_t* sendcountsp = &sendcounts[0];
    size_t* recvcountsp = &recvcounts[0];
    for(int i = 0; i < node_size; i++) {
      sendcountsp[i] = alltoall_sizes[self][i];
      recvcountsp[i] = alltoall_sizes[i][self];
    }
    size_t total_recv = 0, total_send = 0;
    for(int i = 0; i < node_size; i++) total_recv += recvcountsp[i];
    for(int i = 0; i < node_size; i++) total_send += sendcountsp[i];
    std::vector<size_t> sdispls(node_size), rdispls(node_size);
    size_t* sdisplsp = &sdispls[0];
    size_t* rdisplsp = &rdispls[0];
    for(int i = 1; i < node_size; i++) {
      sdisplsp[i] = sdisplsp[i-1] + sendcountsp[i-1];
      rdisplsp[i] = rdisplsp[i-1] + recvcountsp[i-1];
    }
    std::vector<T> srcbuf(total_send);
    T* srcbufp = &srcbuf[0];
    for(size_t i = 0; i < src.size(); i++) {
      T* srcp = &src[i][0];
#pragma cdir nodep
#pragma _NEC ivdep
      for(size_t j = 0; j < sendcountsp[i]; j++) {
        srcbufp[sdisplsp[i] + j] = srcp[j];
      }
    }
    std::vector<T> dstbuf(total_recv);
    T* dstbufp = &dstbuf[0];
    large_alltoallv(sizeof(T), reinterpret_cast<char*>(srcbufp),
                    sendcounts, sdispls,
                    reinterpret_cast<char*>(dstbufp), recvcounts,
                    rdispls, frovedis_comm_rpc);
    std::vector<std::vector<T>> dst(node_size);
    for(size_t i = 0; i < dst.size(); i++) {
      dst[i].resize(recvcounts[i]);
      T* dstp = &dst[i][0];
#pragma cdir nodep
#pragma _NEC ivdep
      for(size_t j = 0; j < recvcountsp[i]; j++) {
        dstp[j] = dstbufp[rdisplsp[i] + j];
      }
    }
    return dst;
  }
  std::vector<std::vector<size_t>> alltoall_sizes;
  SERIALIZE(alltoall_sizes)
};

template <class T>
DVID<std::vector<std::vector<T>>>
alltoall_exchange_pod(DVID<std::vector<std::vector<T>>>& src) {
  auto alltoall_sizes =
    src.template map<std::vector<size_t>>(alltoall_getsizes_pod<T>()).gather();
  return
    src.template map<std::vector<std::vector<T>>>
    (call_mpi_alltoallv_pod<T>(std::move(alltoall_sizes)));
}

template <>
DVID<std::vector<std::vector<unsigned int>>> 
alltoall_exchange(DVID<std::vector<std::vector<unsigned int>>>& src);

template <>
DVID<std::vector<std::vector<int>>> 
alltoall_exchange(DVID<std::vector<std::vector<int>>>& src);

template <>
DVID<std::vector<std::vector<float>>> 
alltoall_exchange(DVID<std::vector<std::vector<float>>>& src);

template <>
DVID<std::vector<std::vector<double>>> 
alltoall_exchange(DVID<std::vector<std::vector<double>>>& src);

template <>
DVID<std::vector<std::vector<unsigned long long>>> 
alltoall_exchange(DVID<std::vector<std::vector<unsigned long long>>>& src);

template <>
DVID<std::vector<std::vector<long long>>> 
alltoall_exchange(DVID<std::vector<std::vector<long long>>>& src);

template <>
DVID<std::vector<std::vector<unsigned long>>> 
alltoall_exchange(DVID<std::vector<std::vector<unsigned long>>>& src);

template <>
DVID<std::vector<std::vector<long>>> 
alltoall_exchange(DVID<std::vector<std::vector<long>>>& src);

template <class TT>
node_local<std::vector<TT>>
alltoall_exchange(node_local<std::vector<TT>>& src) {
  auto dvid = src.get_dvid();
  return node_local<std::vector<TT>>(alltoall_exchange(dvid));
}

template <class T, class F>
struct sort_local_sort {
  sort_local_sort() {}
  sort_local_sort(F pred_, double rate_) : pred(pred_), rate(rate_) {}
  void operator()(std::vector<T>& v, std::vector<T>& lv) {
    std::sort(v.begin(), v.end(), pred);
#ifdef USE_STD_RANDOM
    std::random_device seed_gen;
    std::default_random_engine engine(seed_gen());
    std::uniform_real_distribution<> dist(0, 1.0);
    auto each = ceil_div(v.size(), get_nodesize());
    for(size_t i = 0; i < v.size(); i += each) {
      if(rate > 1.0) lv.push_back(v[i]);
      else {
        if(dist(engine) < rate) lv.push_back(v[i]);
      }
    }
#else
    auto each = ceil_div(v.size(), static_cast<size_t>(get_nodesize()));
    for(size_t i = 0; i < v.size(); i += each) {
      if(rate > 1.0) lv.push_back(v[i]);
      else {
        if(drand48() < rate) lv.push_back(v[i]);
      }
    }
#endif
  }
  F pred;
  double rate;
  SERIALIZE(pred, rate)
};

template <class T, class F>
struct sort_get_partcand {
  sort_get_partcand() {}
  sort_get_partcand(F pred_) : pred(pred_) {}
  std::vector<T> operator()(const std::vector<T>& l, const std::vector<T>& r) {
    std::vector<T> res;
    std::merge(l.begin(), l.end(), r.begin(), r.end(),
               std::back_inserter(res), pred);
    return res;
  }
  F pred;
  SERIALIZE(pred)
};

template <class T, class F>
struct sort_part {
  sort_part() {}
  sort_part(const std::vector<T>& part_,
            const DVID<std::vector<std::vector<T>>>& target_,
            F pred_) :
    part(part_), target(target_), pred(pred_) {}
  void operator()(std::vector<T>& v) {
    size_t nodesize = get_nodesize();
    target.get_selfdata()->resize(nodesize);
    for(size_t i = 0; i < nodesize; i++) {
      std::vector<T> tmp;
      if(i == 0) { // start
        if(part.size() == 0) { // no partitioner: all data go to #0
          tmp.insert(tmp.end(), v.begin(), v.end());
        } else {
          T part_r = part[i];
          auto iter_r = lower_bound(v.begin(), v.end(), part_r, pred);
          tmp.insert(tmp.end(), v.begin(), iter_r);
        }
      } else if(i == static_cast<int>(part.size())) {
        // end, part.size() >= 1 is guaranteed
        T part_l = part[i-1];
        auto iter_l = lower_bound(v.begin(), v.end(), part_l, pred);
        tmp.insert(tmp.end(), iter_l, v.end());
      } else if(i > static_cast<int>(part.size())) { // over: no use
        continue;
      } else { // middle, 0 < i < part.size() is guaranteed
        T part_r = part[i];
        T part_l = part[i-1];
        auto iter_l = lower_bound(v.begin(), v.end(), part_l, pred);
        auto iter_r = lower_bound(v.begin(), v.end(), part_r, pred);
        tmp.insert(tmp.end(), iter_l, iter_r);
      }
      (*target.get_selfdata())[i].swap(tmp);
    }
  }
  std::vector<T> part;
  DVID<std::vector<std::vector<T>>> target;
  F pred;
  SERIALIZE(part, target, pred)
};

template <class T, class F>
struct sort_multimerge {
  sort_multimerge() {};
  sort_multimerge(F pred_) : pred(pred_) {}
  std::vector<T> operator()(std::vector<std::vector<T>>& v) {
    std::vector<size_t> idx(v.size(), 1);
    std::vector<size_t> sizes;
    std::multimap<T, size_t, F> merger(pred);
    std::vector<T> r;
    for(auto& i: v) sizes.push_back(i.size());
    for(size_t i = 0; i < v.size(); i++) {
      if(v[i].size() != 0) merger.insert(std::make_pair(v[i][0], i));
    }
    while(1) {
      if(merger.empty()) break;
      else {
        r.push_back(merger.begin()->first);
        size_t from = merger.begin()->second;
        merger.erase(merger.begin());
        if(idx[from] != sizes[from])
          merger.insert(std::make_pair(v[from][idx[from]++], from));
      }
    }
    return r;
  }
  F pred;
  SERIALIZE(pred)
};

template <class T>
template <class F>
dvector<T>& dvector<T>::sort(F f, double rate) {
  if(is_view)
    throw std::runtime_error("currently sort on view is not allowed");
  auto lv = make_node_local_allocate<std::vector<T>>();
  mapv_partitions(sort_local_sort<T,F>(f, rate), lv);
  auto partcand = lv.reduce(sort_get_partcand<T,F>(f));
  std::vector<T> part;
  auto each = ceil_div(partcand.size(), static_cast<size_t>(get_nodesize()));
  for(size_t i = each; i < partcand.size(); i += each) {
    part.push_back(partcand[i]);
  }
  auto target = make_dvid_allocate<std::vector<std::vector<T>>>();
  mapv_partitions(sort_part<T,F>(part, target, f));
  auto exchanged = alltoall_exchange(target);
  target.delete_var();
  this->dvid = exchanged.template map<std::vector<T>>(sort_multimerge<T,F>(f));
  return *this;
}

template <class T>
struct sort_less_than {
  bool operator()(const T& l, const T& r) {
    return l < r;
  }
  SERIALIZE_NONE
};

template <class T>
dvector<T>& dvector<T>::sort(double rate) {
  return sort(sort_less_than<T>(), rate);
}

template <class T>
std::vector<T> convert_endian(std::vector<T>&);

template <>
std::vector<char> convert_endian(std::vector<char>& vec);

template <>
std::vector<int> convert_endian(std::vector<int>& vec);

template <>
std::vector<unsigned int> convert_endian(std::vector<unsigned int>& vec);

template <>
std::vector<long> convert_endian(std::vector<long>& vec);

template <>
std::vector<unsigned long> convert_endian(std::vector<unsigned long>& vec);

template <>
std::vector<long long> convert_endian(std::vector<long long>& vec);

template <>
std::vector<unsigned long long> convert_endian(std::vector<unsigned long long>& vec);

template <>
std::vector<float> convert_endian(std::vector<float>& vec);

template <>
std::vector<double> convert_endian(std::vector<double>& vec);

template <class T>
struct savebinary_helper {
  savebinary_helper(){}
  savebinary_helper(const std::string& path,
                    const std::vector<size_t>& sizes,
                    bool is_sequential_save) :
    path(path), sizes(sizes), is_sequential_save(is_sequential_save) {}
  void operator()(std::vector<T>& data) {
    size_t nodes = get_nodesize();
    size_t self = get_selfid();
    size_t start = 0;
    for(size_t i = 0; i < self; i++) start += sizes[i];
    size_t to_write = data.size() * sizeof(T);
    auto conv_data = convert_endian<T>(data);
    char* write_data = reinterpret_cast<char*>(&conv_data[0]);
    if(is_sequential_save) {
      size_t myoff = start * sizeof(T);
      for(size_t i = 0; i < nodes; i++) {
        if(i == self) {
          int fd = open(path.c_str(), O_RDWR);
          if(fd == -1)
            throw std::runtime_error("open " + path + " error: " +
                                     std::string(strerror(errno)));
          auto ret = lseek(fd, myoff, SEEK_SET);
          if(ret == -1)
            throw std::runtime_error("lseek " + path + " error: " +
                                     std::string(strerror(errno)));
          ssize_t pos = 0;
          while(true) {
            auto written = write(fd, write_data+pos, to_write);
            if(written == -1) {
              throw std::runtime_error("write " + path + " error: " +
                                       std::string(strerror(errno)));
            } else if(written < to_write) {
              to_write -= written;
              pos += written;
            } else break;
          }
          close(fd);
        }
        MPI_Barrier(frovedis_comm_rpc);
      }
    } else {
      MPI_File fh;
      int ret = MPI_File_open(frovedis_comm_rpc,
                              const_cast<char*>(path.c_str()),
                              MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
      if(ret != 0) throw std::runtime_error("error in MPI_File_open");
      MPI_Offset off = start * sizeof(T);
      char datarep[] = "native";
      MPI_File_set_view(fh, off, MPI_CHAR, MPI_CHAR, datarep, MPI_INFO_NULL);
      size_t maxsize = 0;
      size_t* sizesp = &sizes[0];
      for(size_t i = 0; i < sizes.size(); i++) {
        if(maxsize < sizesp[i]) maxsize = sizesp[i];
      }
      size_t numiter = ceil_div(maxsize * sizeof(T), mpi_max_count);
      size_t pos = 0;
      for(size_t i = 0; i < numiter; i++) {
        size_t write_size = std::min(to_write, mpi_max_count);
        MPI_Status st;
        ret = MPI_File_write_all(fh, write_data + pos, write_size, MPI_CHAR,
                                 &st);
        if(ret != 0) throw std::runtime_error("error in MPI_File_write_all");
        pos += write_size;
        if(to_write > mpi_max_count) to_write -= mpi_max_count;
        else to_write = 0;
      }
      MPI_File_close(&fh);
    }
  }
  std::string path;
  std::vector<size_t> sizes;
  bool is_sequential_save;
  SERIALIZE(path, sizes, is_sequential_save)
};

template <class T>
void dvector<T>::savebinary(const std::string& path) {
  save_file_checker(path);
  dvid.mapv(savebinary_helper<T>(path, sizes(), is_sequential_save()));
}

template <class T>
struct load_binary_helper {
  load_binary_helper(){}
  load_binary_helper(const std::string& path) : path(path) {}
  load_binary_helper(const std::string& path, const std::vector<size_t>& sizes)
    : path(path), sizes(sizes) {}
  void operator()(std::vector<T>& data) {
    MPI_File fh;
    int ret = MPI_File_open(frovedis_comm_rpc, const_cast<char*>(path.c_str()),
                            MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
    if(ret != 0) throw std::runtime_error("error in MPI_File_open");
    MPI_Offset file_size;
    ret = MPI_File_get_size(fh, &file_size);
    if(ret != 0) throw std::runtime_error("error in MPI_File_get_size");
    size_t num_item = static_cast<size_t>(file_size) / sizeof(T);
    if(file_size % sizeof(T) != 0)
      throw std::runtime_error("make_dvector_loadbinary: incorrect filesize");
    size_t myoff, mysize;
    if(sizes.size() == 0) {
      size_t each_size = ceil_div(num_item,
                                  static_cast<size_t>(get_nodesize()));
      myoff = std::min(each_size * get_selfid() * sizeof(T),
                       static_cast<size_t>(file_size));
      mysize = std::min(each_size, (static_cast<size_t>(file_size) -
                                    myoff) / sizeof(T));
    } else {
      size_t total = 0;
      for(size_t i = 0; i < sizes.size(); i++) total += sizes[i];
      if(num_item != total) 
        throw std::runtime_error
          ("make_dvector_loadbinary: specified sizes are incorrect");
      myoff = 0;
      for(size_t i = 0; i < get_selfid(); i++) myoff += sizes[i] * sizeof(T);
      mysize = sizes[get_selfid()];
    }
    std::vector<T> raw_data;
    raw_data.resize(mysize);
    char datarep[] = "native";
    MPI_File_set_view(fh, myoff, MPI_CHAR, MPI_CHAR, datarep, MPI_INFO_NULL);
    MPI_Status st;

    char* read_data = reinterpret_cast<char*>(&raw_data[0]);
    size_t maxsize = 0;
    if(sizeof(size_t) == 8)
      MPI_Allreduce(&mysize, &maxsize, 1, MPI_UNSIGNED_LONG_LONG, MPI_MAX,
                    frovedis_comm_rpc);
    else
      MPI_Allreduce(&mysize, &maxsize, 1, MPI_UNSIGNED_LONG, MPI_MAX,
                    frovedis_comm_rpc);
    size_t numiter = ceil_div(maxsize * sizeof(T), mpi_max_count);
    size_t to_read = raw_data.size() * sizeof(T);
    size_t pos = 0;
    for(size_t i = 0; i < numiter; i++) {
      size_t read_size = std::min(to_read, mpi_max_count);
      ret = MPI_File_read_all(fh, read_data + pos, read_size, MPI_CHAR, &st);
      if(ret != 0) throw std::runtime_error("error in MPI_File_read_all");
      pos += read_size;
      if(to_read > mpi_max_count) to_read -= mpi_max_count;
      else to_read = 0;
    }
    auto tmp = convert_endian<T>(raw_data);
    data.swap(tmp);
    MPI_File_close(&fh);
  }
  std::string path;
  std::vector<size_t> sizes;
  SERIALIZE(path, sizes)
};

template <class T>
dvector<T> make_dvector_loadbinary(const std::string& path) {
  auto ret = make_dvector_allocate<T>();
  ret.get_dvid().mapv(load_binary_helper<T>(path));
  return ret;
}

template <class T>
dvector<T> make_dvector_loadbinary(const std::string& path,
                                   std::vector<size_t>& sizes) {
  auto ret = make_dvector_allocate<T>();
  ret.get_dvid().mapv(load_binary_helper<T>(path, sizes));
  return ret;
}

template <class T>
std::vector<T> head_helper(std::vector<T>& v, size_t to_trunc) {
  if(v.size() > to_trunc) {
    std::vector<T> tmp(to_trunc);
    T* tmpp = &tmp[0];
    T* vp = &v[0];
    for(size_t i = 0; i < to_trunc; i++) tmpp[i] = vp[i];
    return tmp;
  } else return v;
}

template <class T>
dvector<T> dvector<T>::head(size_t to_trunc_size) {
  auto all_sizes = sizes();
  size_t node_size = get_nodesize();
  std::vector<size_t> trunc_sizes(node_size);
  for(size_t i = 0; i < node_size; i++) {
    trunc_sizes[i] = std::min(to_trunc_size, all_sizes[i]);
    if(to_trunc_size > all_sizes[i]) {
      to_trunc_size -= all_sizes[i];
    } else to_trunc_size = 0;
  }
  auto dtrunc_sizes = make_node_local_scatter(trunc_sizes);
  return this->viewas_node_local().
    map(head_helper<T>, dtrunc_sizes).
    template moveto_dvector<T>();
}


template <class T>
std::vector<T> tail_helper(std::vector<T>& v, size_t to_trunc) {
  if(v.size() > to_trunc) {
    std::vector<T> tmp(to_trunc);
    T* tmpp = &tmp[0];
    T* vp = &v[0] + (v.size() - to_trunc);
    for(size_t i = 0; i < to_trunc; i++) tmpp[i] = vp[i];
    return tmp;
  } else return v;
}

template <class T>
dvector<T> dvector<T>::tail(size_t to_trunc_size) {
  auto all_sizes = sizes();
  size_t node_size = get_nodesize();
  std::vector<size_t> trunc_sizes(node_size);
  for(size_t i = 0; i < node_size; i++) {
    auto current_node = node_size - i - 1;
    trunc_sizes[current_node] = std::min(to_trunc_size, all_sizes[current_node]);
    if(to_trunc_size > all_sizes[current_node]) {
      to_trunc_size -= all_sizes[current_node];
    } else to_trunc_size = 0;
  }
  auto dtrunc_sizes = make_node_local_scatter(trunc_sizes);
  return this->viewas_node_local().
    map(tail_helper<T>, dtrunc_sizes).
    template moveto_dvector<T>();
}


// utility functions
template <class T>
size_t dictionary_getid(const std::vector<T>& dic, const T& key) {
  return std::lower_bound(dic.begin(), dic.end(), key) - dic.begin();  
}

template <class T>
std::vector<T> make_dictionary_set_union(const std::vector<T>& l,
                                         const std::vector<T>& r) {
  std::vector<T> ret;
  std::set_union(l.begin(), l.end(),
                 r.begin(), r.end(),
                 inserter(ret, ret.begin()));
  return ret;
}

template <class T>
void make_dictionary_sort(std::vector<T>& v) {
  std::sort(v.begin(), v.end());
  v.erase(std::unique(v.begin(), v.end()), v.end());
}

template <class T>
std::vector<T> make_dictionary_local(dvector<T>& dv) {
  auto lv = dv.as_node_local();
  lv.mapv(make_dictionary_sort<T>);
  return lv.reduce(make_dictionary_set_union<T>);
}

template <class T>
std::vector<T> make_dictionary_local(node_local<std::vector<T>>& lv1) {
  auto lv = lv1;
  lv.mapv(make_dictionary_sort<T>);
  return lv.reduce(make_dictionary_set_union<T>);
}

template <class T>
node_local<std::vector<T>> make_dictionary(dvector<T>& dv) {
  return make_node_local_broadcast(make_dictionary_local(dv));
}

template <class T>
node_local<std::vector<T>> make_dictionary(node_local<std::vector<T>>& lv) {
  return make_node_local_broadcast(make_dictionary_local(lv));
}

}

#endif
