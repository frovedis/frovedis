#ifndef NODE_LOCAL_HPP
#define NODE_LOCAL_HPP

#include "DVID.hpp"
#include "type_utility.hpp"

namespace frovedis {

template <class T> class node_local;
template <class T> node_local<T> make_node_local_broadcast(const T& var);
template <class T> class dvector;

template <class T>
class node_local {
public:
  using value_type = T;
  node_local() : is_view(false) {}
  node_local(const DVID<T>& dvid_, bool is_view_ = false) : 
    dvid(dvid_), is_view(is_view_) {}
  node_local(DVID<T>&& dvid_, bool is_view_ = false) :
    dvid(std::move(dvid_)), is_view(is_view_)  {}
  node_local(const node_local<T>& src) :
    dvid(src.dvid.copy()), is_view(false)  {}
  node_local(node_local<T>&& src) : is_view(false) {
    std::swap(dvid, src.dvid);
    std::swap(is_view, src.is_view);
  }
  node_local<T>& operator=(const node_local<T>& src) {
    if(!is_view) dvid.delete_var();
    dvid = src.dvid.copy();
    is_view = false;
    return *this;
  }
  node_local<T>& operator=(node_local<T>&& src) {
    // dvid of original this will be deleted after swap if it is not view
    std::swap(dvid, src.dvid);
    std::swap(is_view, src.is_view);
    return *this;
  }
  ~node_local(){if(!is_view) dvid.delete_var();}

  template<class F> T reduce(const F& f) {
    return dvid.reduce(f);
  }
  template <class TT, class UU>
  T reduce(T(*f)(TT,UU)) {return reduce(make_serfunc(f));}
  template<class F>
  node_local<T> allreduce(const F& f) {
    return node_local<T>(dvid.allreduce(f));
  }
  template <class TT, class UU>
  node_local<T> allreduce(T(*f)(TT,UU)) {return allreduce(make_serfunc(f));}
  std::vector<T> gather(){return dvid.gather();}
  template <class R, class F> node_local<R> map(const F& f) {
    return node_local<R>(dvid.template map<R>(f));
  }
  template <class R, class TT> node_local<R> map(R(*f)(TT)) {
    return map<R>(make_serfunc(f));
  }
  template <class R, class U, class F>
  node_local<R> map(const F& f, const node_local<U>& l) {
    return node_local<R>(dvid.template map<R>(f, l.get_dvid()));
  }
  template <class R, class U, class TT, class UU>
  node_local<R> map(R(*f)(TT,UU), const node_local<U>& l) {
    return map<R>(make_serfunc(f),l);
  }
  template <class R, class U, class V, class F>
  node_local<R> map(const F& f, const node_local<U>& l,
                    const node_local<V>& l2) {
    return node_local<R>(dvid.template map<R>(f, l.get_dvid(), l2.get_dvid()));
  }
  template <class R, class U, class V, class TT, class UU, class VV>
  node_local<R> map(R(*f)(TT,UU,VV), const node_local<U>& l,
                    const node_local<V>& l2) {
    return map<R>(make_serfunc(f),l,l2);
  }
  template <class R, class U, class V, class W, class F>
  node_local<R> map(const F& f, const node_local<U>& l,
                    const node_local<V>& l2, const node_local<W>& l3) {
    return node_local<R>(dvid.template map<R>(f, l.get_dvid(), l2.get_dvid(), 
                                              l3.get_dvid()));
  }
  template <class R, class U, class V, class W,
            class TT, class UU, class VV, class WW>
  node_local<R> map(R(*f)(TT,UU,VV,WW), const node_local<U>& l,
                    const node_local<V>& l2, const node_local<W>& l3) {
    return map<R>(make_serfunc(f),l,l2,l3);
  }
  template <class R, class U, class V, class W, class X, class F>
  node_local<R> map(const F& f, const node_local<U>& l,
                    const node_local<V>& l2, const node_local<W>& l3,
                    const node_local<X>& l4) {
    return node_local<R>(dvid.template map<R>(f, l.get_dvid(), l2.get_dvid(), 
                                              l3.get_dvid(), l4.get_dvid()));
  }
  template <class R, class U, class V, class W, class X,
            class TT, class UU, class VV, class WW, class XX>
  node_local<R> map(R(*f)(TT,UU,VV,WW,XX), const node_local<U>& l,
                    const node_local<V>& l2, const node_local<W>& l3,
                    const node_local<X>& l4) {
    return map<R>(make_serfunc(f),l,l2,l3,l4);
  }
  template <class R, class U, class V, class W, class X, class Y, class F>
  node_local<R> map(const F& f, const node_local<U>& l,
                    const node_local<V>& l2, const node_local<W>& l3,
                    const node_local<X>& l4, const node_local<Y>& l5) {
    return node_local<R>(dvid.template map<R>(f, l.get_dvid(), l2.get_dvid(), 
                                              l3.get_dvid(), l4.get_dvid(), 
                                              l5.get_dvid()));
  }
  template <class R, class U, class V, class W, class X, class Y,
            class TT, class UU, class VV, class WW, class XX, class YY>
  node_local<R> map(R(*f)(TT,UU,VV,WW,XX,YY), const node_local<U>& l,
                    const node_local<V>& l2, const node_local<W>& l3,
                    const node_local<X>& l4, const node_local<Y>& l5) {
    return map<R>(make_serfunc(f),l,l2,l3,l4,l5);
  }
  template <class R, class U, class V, class W, class X, class Y, class Z, class F>
  node_local<R> map(const F& f, const node_local<U>& l,
                    const node_local<V>& l2, const node_local<W>& l3,
                    const node_local<X>& l4, const node_local<Y>& l5,
                    const node_local<Z>& l6 ) {
    return node_local<R>(dvid.template map<R>(f, l.get_dvid(), l2.get_dvid(),
                                              l3.get_dvid(), l4.get_dvid(),
                                              l5.get_dvid(), l6.get_dvid() ));
  }
  template <class R, class U, class V, class W, class X, class Y, class Z,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ>
  node_local<R> map(R(*f)(TT,UU,VV,WW,XX,YY,ZZ), const node_local<U>& l,
                    const node_local<V>& l2, const node_local<W>& l3,
                    const node_local<X>& l4, const node_local<Y>& l5,
                    const node_local<Z>& l6) {
    return map<R>(make_serfunc(f),l,l2,l3,l4,l5,l6);
  }
  template <class R, class U, class V, class W, class X, class Y, class Z, class A, class F>
  node_local<R> map(const F& f, const node_local<U>& l,
                    const node_local<V>& l2, const node_local<W>& l3,
                    const node_local<X>& l4, const node_local<Y>& l5,
                    const node_local<Z>& l6, const node_local<A>& l7 ) {
    return node_local<R>(dvid.template map<R>(f, l.get_dvid(), l2.get_dvid(),
                                              l3.get_dvid(), l4.get_dvid(),
                                              l5.get_dvid(), l6.get_dvid(),
                                              l7.get_dvid() ));
  }
  template <class R, class U, class V, class W, class X, class Y, class Z, class A,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ, class AA>
  node_local<R> map(R(*f)(TT,UU,VV,WW,XX,YY,ZZ,AA), const node_local<U>& l,
                    const node_local<V>& l2, const node_local<W>& l3,
                    const node_local<X>& l4, const node_local<Y>& l5,
                    const node_local<Z>& l6, const node_local<A>& l7) {
    return map<R>(make_serfunc(f),l,l2,l3,l4,l5,l6,l7);
  }
  template <class R, class U, class V, class W, class X, class Y, class Z, class A, class B, class F>
  node_local<R> map(const F& f, const node_local<U>& l,
                    const node_local<V>& l2, const node_local<W>& l3,
                    const node_local<X>& l4, const node_local<Y>& l5,
                    const node_local<Z>& l6, const node_local<A>& l7,
                    const node_local<B>& l8 ) {
    return node_local<R>(dvid.template map<R>(f, l.get_dvid(), l2.get_dvid(),
                                              l3.get_dvid(), l4.get_dvid(),
                                              l5.get_dvid(), l6.get_dvid(),
                                              l7.get_dvid(), l8.get_dvid() ));
  }
  template <class R, class U, class V, class W, class X, class Y, class Z, class A, class B,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ, class AA, class BB>
  node_local<R> map(R(*f)(TT,UU,VV,WW,XX,YY,ZZ,AA,BB), const node_local<U>& l,
                    const node_local<V>& l2, const node_local<W>& l3,
                    const node_local<X>& l4, const node_local<Y>& l5,
                    const node_local<Z>& l6, const node_local<A>& l7,
                    const node_local<B>& l8) {
    return map<R>(make_serfunc(f),l,l2,l3,l4,l5,l6,l7,l8);
  }

  template <typename F, typename... Types>
  node_local<map_result_t<F, T, Types...>> map(const F& f, const node_local<Types>&... ls) {
    return map<map_result_t<F, T, Types...>>(f, ls...);
  }

  template <class F> node_local<T>& mapv(const F& f){
    dvid.mapv(f);
    return *this;
  }
  template <class TT>
  node_local<T>& mapv(void(*f)(TT)){return mapv(make_serfunc(f));}
  template <class U, class F>
  node_local<T>& mapv(const F& f, const node_local<U>& l) {
    dvid.mapv(f, l.get_dvid());
    return *this;
  }
  template <class U, class V, class F>
  node_local<T>& mapv(const F& f, const node_local<U>& l,
                      const node_local<V>& l2){
    dvid.mapv(f, l.get_dvid(), l2.get_dvid());
    return *this;
  }
  template <class U, class V, class TT, class UU, class VV>
  node_local<T>& mapv(void(*f)(TT,UU,VV), const node_local<U>& l,
                      const node_local<V>& l2){
    return mapv(make_serfunc(f),l,l2);
  }
  template <class U, class V, class W, class F>
  node_local<T>& mapv(const F& f, const node_local<U>& l,
                      const node_local<V>& l2, const node_local<W>& l3) {
    dvid.mapv(f, l.get_dvid(), l2.get_dvid(), l3.get_dvid());
    return *this;
  }
  template <class U, class V, class W,
            class TT, class UU, class VV, class WW>
  node_local<T>& mapv(void(*f)(TT,UU,VV,WW), const node_local<U>& l,
                      const node_local<V>& l2, const node_local<W>& l3) {
    return mapv(make_serfunc(f),l,l2,l3);
  }
  template <class U, class V, class W, class X, class F>
  node_local<T>& mapv(const F& f, const node_local<U>& l,
                      const node_local<V>& l2, const node_local<W>& l3,
                      const node_local<X>& l4){
    dvid.mapv(f, l.get_dvid(), l2.get_dvid(), l3.get_dvid(), l4.get_dvid());
    return *this;
  }
  template <class U, class V, class W, class X,
            class TT, class UU, class VV, class WW, class XX>
  node_local<T>& mapv(void(*f)(TT,UU,VV,WW,XX), const node_local<U>& l,
                      const node_local<V>& l2, const node_local<W>& l3,
                      const node_local<X>& l4) {
    return mapv(make_serfunc(f),l,l2,l3,l4);
  }
  template <class U, class V, class W, class X, class Y, class F>
  node_local<T>& mapv(const F& f, const node_local<U>& l,
                      const node_local<V>& l2, const node_local<W>& l3,
                      const node_local<X>& l4, const node_local<Y>& l5) {
    dvid.mapv(f, l.get_dvid(), l2.get_dvid(), l3.get_dvid(), l4.get_dvid(),
              l5.get_dvid());
    return *this;
  }
  template <class U, class V, class W, class X, class Y,
            class TT, class UU, class VV, class WW, class XX, class YY>
  node_local<T>& mapv(void(*f)(TT,UU,VV,WW,XX), const node_local<U>& l,
                      const node_local<V>& l2, const node_local<W>& l3,
                      const node_local<X>& l4, const node_local<Y>& l5) {
    return mapv(make_serfunc(f),l,l2,l3,l4,l5);
  }
  template <class U, class V, class W, class X, class Y, class Z, class F>
  node_local<T>& mapv(const F& f, const node_local<U>& l,
                      const node_local<V>& l2, const node_local<W>& l3,
                      const node_local<X>& l4, const node_local<Y>& l5,
                      const node_local<Z>& l6) {
    dvid.mapv(f, l.get_dvid(), l2.get_dvid(), l3.get_dvid(), l4.get_dvid(),
              l5.get_dvid(), l6.get_dvid());
    return *this;
  }
  template <class U, class V, class W, class X, class Y, class Z,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ>
  node_local<T>& mapv(void(*f)(TT,UU,VV,WW,XX,YY,ZZ), const node_local<U>& l,
                      const node_local<V>& l2, const node_local<W>& l3,
                      const node_local<X>& l4, const node_local<Y>& l5,
                      const node_local<Z>& l6) {
    return mapv(make_serfunc(f),l,l2,l3,l4,l5,l6);
  }
  template <class U, class V, class W, class X, class Y, class Z, class A, class F>
  node_local<T>& mapv(const F& f, const node_local<U>& l,
                      const node_local<V>& l2, const node_local<W>& l3,
                      const node_local<X>& l4, const node_local<Y>& l5,
                      const node_local<Z>& l6, const node_local<A>& l7) {
    dvid.mapv(f, l.get_dvid(), l2.get_dvid(), l3.get_dvid(), l4.get_dvid(),
              l5.get_dvid(), l6.get_dvid(), l7.get_dvid());
    return *this;
  }
  template <class U, class V, class W, class X, class Y, class Z, class A,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ, class AA>
  node_local<T>& mapv(void(*f)(TT,UU,VV,WW,XX,YY,ZZ,AA), const node_local<U>& l,
                      const node_local<V>& l2, const node_local<W>& l3,
                      const node_local<X>& l4, const node_local<Y>& l5,
                      const node_local<Z>& l6, const node_local<A>& l7) {
    return mapv(make_serfunc(f),l,l2,l3,l4,l5,l6,l7);
  }
  template <class U, class V, class W, class X, class Y, class Z, class A, class B, class F>
  node_local<T>& mapv(const F& f, const node_local<U>& l,
                      const node_local<V>& l2, const node_local<W>& l3,
                      const node_local<X>& l4, const node_local<Y>& l5,
                      const node_local<Z>& l6, const node_local<A>& l7,
                      const node_local<B>& l8) {
    dvid.mapv(f, l.get_dvid(), l2.get_dvid(), l3.get_dvid(), l4.get_dvid(),
              l5.get_dvid(), l6.get_dvid(), l7.get_dvid(), l8.get_dvid());
    return *this;
  }
  template <class U, class V, class W, class X, class Y, class Z, class A, class B,
            class TT, class UU, class VV, class WW, class XX, class YY, class ZZ, class AA, class BB>
  node_local<T>& mapv(void(*f)(TT,UU,VV,WW,XX,YY,ZZ,AA,BB), const node_local<U>& l,
                      const node_local<V>& l2, const node_local<W>& l3,
                      const node_local<X>& l4, const node_local<Y>& l5,
                      const node_local<Z>& l6, const node_local<A>& l7,
                      const node_local<B>& l8) {
    return mapv(make_serfunc(f),l,l2,l3,l4,l5,l6,l7,l8);
  }

  T vector_sum() {return dvid.vector_sum();}

  template <class U>
  dvector<U> as_dvector() const;

  template <class U>
  dvector<U> moveto_dvector();

  template <class U>
  dvector<U> viewas_dvector();

  void put(int node, const T& val) {dvid.put(node,val);}
  T get(int node) {return dvid.get(node);}

  DVID<T> get_dvid() const {return dvid;}
private:
  DVID<T> dvid;
  bool is_view;

  SERIALIZE(dvid, is_view)
};

// shortcut definition
template <class T>
using lvec = frovedis::node_local<std::vector<T>>;

template <class T>
node_local<T> make_node_local_broadcast(const T& var) {
  return node_local<T>(make_dvid_broadcast<T>(var));
}

// shortcut expression
template <class T>
node_local<T> broadcast(const T& var) {
  return make_node_local_broadcast<T>(var);
}

template <class T>
node_local<T> make_node_local_allocate() {
  return node_local<T>(make_dvid_allocate<T>());
}

template <class T>
node_local<T> make_node_local_scatter(const std::vector<T>& v) {
  return node_local<T>(make_dvid_scatter<T>(v));
}

}
#endif
