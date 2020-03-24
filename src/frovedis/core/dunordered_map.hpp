#ifndef DUNORDERED_MAP_HPP
#define DUNORDERED_MAP_HPP

#include "dvector.hpp"

#ifdef USE_YAS
#ifdef USE_ORDERED_MAP
#include <yas/types/std/map.hpp>
#include <yas/types/std/pair.hpp>
#else
#include <yas/types/std/unordered_map.hpp>
#include <yas/types/std/pair.hpp>
#endif
#endif

#ifdef USE_CEREAL
#ifdef USE_ORDERED_MAP
#include <cereal/types/map.hpp>
#include <cereal/types/utility.hpp>
#else
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/utility.hpp>
#endif
#endif

#ifdef USE_BOOST_SERIALIZATION
#ifdef USE_ORDERED_MAP
#include <boost/serialization/map.hpp>
#include <boost/serialization/utility.hpp>
#else
#include "unordered_map_serialize.hpp"
#include <boost/serialization/utility.hpp>
#endif
#endif

#ifdef USE_BOOST_HASH
#include <boost/functional/hash.hpp>
#endif

namespace frovedis {

#ifdef USE_ORDERED_MAP
template <class K, class V>
using my_map = std::map<K,V>;
#else
template <class K, class V>
using my_map = std::unordered_map<K,V>;
#endif

#ifdef USE_BOOST_HASH
template <class K>
using my_hash = boost::hash<K>;
#else
template <class K>
using my_hash = std::hash<K>;
#endif

template <class K, class V>
class dunordered_map {
public:
  using key_type = K;
  using value_type = V;
  dunordered_map() : is_view(false) {}
  dunordered_map(const DVID<my_map<K,V>>& dvid_, bool is_view_ = false) :
    dvid(dvid_), is_view(is_view_) {}
  dunordered_map(DVID<my_map<K,V>>&& dvid_, bool is_view_ = false) :
    dvid(std::move(dvid_)), is_view(is_view_) {}
  dunordered_map(const dunordered_map<K,V>& src) :
    dvid(src.dvid.copy()), is_view(false) {}
  dunordered_map(dunordered_map<K,V>&& src) : is_view(src.is_view) {
    std::swap(dvid, src.dvid);
  }
  dunordered_map<K,V>& operator=(const dunordered_map<K,V>& src) {
    if(!is_view) dvid.delete_var();
    dvid = src.dvid.copy();
    is_view = src.is_view;
    return *this;
  }
  dunordered_map<K,V>& operator=(dunordered_map<K,V>&& src) {
    // dvid of original this will be deleted after swap if it is not view
    std::swap(dvid, src.dvid);
    std::swap(is_view, src.is_view);
    return *this;
  }
  ~dunordered_map(){if(!is_view) dvid.delete_var();}

  template <class R, class F>
  dunordered_map<K,R> map_values(const F&);
  // KK: K, const K&; VV: V, V&, const V&
  template <class R, class KK, class VV>
  dunordered_map<K,R> map_values(R(*f)(KK,VV)) {
    return map_values<R>(make_serfunc(f));
  }
  template <class R, class U, class F>
  dunordered_map<K,R> map_values(const F&, const node_local<U>& l);
  template <class R, class U, class KK, class VV, class UU>
  dunordered_map<K,R> map_values(R(*f)(KK,VV,UU), const node_local<U>& l) {
    return map_values<R,U>(make_serfunc(f), l);
  }
  template <class R, class U, class W, class F>
  dunordered_map<K,R> map_values(const F&, const node_local<U>& l1,
                                 const node_local<W>& l2);
  template <class R, class U, class W, class KK, class VV, class UU, class WW>
  dunordered_map<K,R> map_values(R(*f)(KK,VV,UU,WW), const node_local<U>& l1,
                                 const node_local<W>& l2) {
    return map_values<R>(make_serfunc(f), l1, l2);
  }
  template <class R, class U, class W, class X, class F>
  dunordered_map<K,R> map_values(const F&, const node_local<U>& l1,
                                 const node_local<W>& l2,
                                 const node_local<X>& l3);
  template <class R, class U, class W, class X,
            class KK, class VV, class UU, class WW, class XX>
  dunordered_map<K,R> map_values(R(*f)(KK,VV,UU,WW,XX),
                                 const node_local<U>& l1, 
                                 const node_local<W>& l2,
                                 const node_local<X>& l3) {
    return map_values<R>(make_serfunc(f), l1, l2, l3);
  }
  template <class R, class U, class W, class X, class Y, class F>
  dunordered_map<K,R> map_values(const F&, const node_local<U>& l1,
                                 const node_local<W>& l2,
                                 const node_local<X>& l3,
                                 const node_local<Y>& l4);
  template <class R, class U, class W, class X, class Y,
            class KK, class VV, class UU, class WW, class XX, class YY>
  dunordered_map<K,R> map_values(R(*f)(KK,VV,UU,WW,XX,YY),
                                 const node_local<U>& l1,
                                 const node_local<W>& l2,
                                 const node_local<X>& l3,
                                 const node_local<Y>& l4) {
    return map_values<R>(make_serfunc(f), l1, l2, l3, l4);
  }
  template <class R, class U, class W, class X, class Y, class Z, class F>
  dunordered_map<K,R> map_values(const F&, const node_local<U>& l1,
                                 const node_local<W>& l2,
                                 const node_local<X>& l3,
                                 const node_local<Y>& l4,
                                 const node_local<Z>& l5);
  template <class R, class U, class W, class X, class Y, class Z,
            class KK, class VV, class UU, class WW, class XX, class YY,
            class ZZ>
  dunordered_map<K,R> map_values(R(*f)(KK,VV,UU,WW,XX,YY,ZZ),
                                 const node_local<U>& l1,
                                 const node_local<W>& l2,
                                 const node_local<X>& l3,
                                 const node_local<Y>& l4,
                                 const node_local<Z>& l5) {
    return map_values<R>(make_serfunc(f), l1, l2, l3, l4, l5);
  }
  template <class F> dunordered_map<K,V>& mapv(const F&);
  template <class KK, class VV>
  dunordered_map<K,V>& mapv(void(*f)(KK,VV)){return mapv(make_serfunc(f));}
  template <class U, class F>
  dunordered_map<K,V>& mapv(const F&, const node_local<U>& l);
  template <class U, class KK, class VV, class UU>
  dunordered_map<K,V>& mapv(void(*f)(KK,VV,UU), const node_local<U>& l) {
    return mapv(make_serfunc(f),l);
  }
  template <class U, class W, class F>
  dunordered_map<K,V>& mapv(const F&, const node_local<U>& l1,
                            const node_local<W>& l2);
  template <class U, class W, class KK, class VV, class UU, class WW>
  dunordered_map<K,V>& mapv(void(*f)(KK,VV,UU,WW), const node_local<U>& l1,
                            const node_local<W>& l2) {
    return mapv(make_serfunc(f),l1,l2);
  }
  template <class U, class W, class X, class F>
  dunordered_map<K,V>& mapv(const F&, const node_local<U>& l1,
                            const node_local<W>& l2, const node_local<X>& l3);
  template <class U, class W, class X,
            class KK, class VV, class UU, class WW, class XX>
  dunordered_map<K,V>& mapv(void(*f)(KK,VV,UU,WW,XX), const node_local<U>& l1,
                            const node_local<W>& l2, const node_local<X>& l3) {
    return mapv(make_serfunc(f),l1,l2,l3);
  }
  template <class U, class W, class X, class Y, class F>
  dunordered_map<K,V>& mapv(const F&, const node_local<U>& l1,
                            const node_local<W>& l2, const node_local<X>& l3,
                            const node_local<Y>& l4);
  template <class U, class W, class X, class Y,
            class KK, class VV, class UU, class WW, class XX, class YY>
  dunordered_map<K,V>& mapv(void(*f)(KK,VV,UU,WW,XX,YY),
                            const node_local<U>& l1, const node_local<W>& l2,
                            const node_local<X>& l3,
                            const node_local<Y>& l4) {
    return mapv(make_serfunc(f),l1,l2,l3,l4);
  }
  template <class U, class W, class X, class Y, class Z, class F>
  dunordered_map<K,V>& mapv(const F&, const node_local<U>& l1,
                            const node_local<W>& l2, const node_local<X>& l3,
                            const node_local<Y>& l4,
                            const node_local<Z>& l5);
  template <class U, class W, class X, class Y, class Z,
            class KK, class VV, class UU, class WW, class XX, class YY,
            class ZZ>
  dunordered_map<K,V>& mapv(void(*f)(KK,VV,UU,WW,XX,YY,ZZ),
                            const node_local<U>& l1, const node_local<W>& l2,
                            const node_local<X>& l3, const node_local<Y>& l4,
                            const node_local<Z>& l5) {
    return mapv(make_serfunc(f),l1,l2,l3,l4,l5);
  }
  template <class F> dunordered_map<K,V> filter(const F&);
  template <class KK, class VV>
  dunordered_map<K,V> filter(bool(*f)(KK,VV)){return filter(make_serfunc(f));}
  template <class F> dunordered_map<K,V>& inplace_filter(const F&);
  template <class KK, class VV>
  dunordered_map<K,V>& inplace_filter(bool(*f)(KK,VV)) {
    return inplace_filter(make_serfunc(f));
  }
  dvector<std::pair<K,V>> as_dvector();

  void put(const K& key, const V& val);
  V get(const K& key);
  V get(const K& key, bool& found);

  void clear();
  size_t size();

  node_local<my_map<K,V>> as_node_local() {
    return node_local<my_map<K,V>>(dvid.copy());
  }
  node_local<my_map<K,V>> moveto_node_local() {
    node_local<my_map<K,V>> r(std::move(dvid));
    dvid.clear_dvid();
    return r;
  }
  node_local<my_map<K,V>> viewas_node_local() {
    return node_local<my_map<K,V>>(dvid, true);
  }

  DVID<my_map<K,V>> get_dvid(){return dvid;}
private:
  DVID<my_map<K,V>> dvid;
  bool is_view;

  SERIALIZE(dvid, is_view)
};

template <class K, class V>
dunordered_map<K,V> make_dunordered_map_allocate() {
  return dunordered_map<K,V>(make_dvid_allocate<my_map<K,V>>());
}

template <class K, class V, class R, class F>
struct mapvalues_dunordered_map {
  mapvalues_dunordered_map() {}
  mapvalues_dunordered_map(const F& f) : func(f) {}
  my_map<K,R> operator()(my_map<K,V>& m) {
    my_map<K,R> dst;
    for(auto it = m.begin(); it != m.end(); ++it) {
      dst.insert(std::make_pair(it->first,func(it->first, it->second)));
    }
    return dst;
  }
  F func;
  SERIALIZE(func)
};

template <class K, class V>
template <class R, class F>
dunordered_map<K,R> dunordered_map<K,V>::map_values(const F& f) {
  // "template" before map is needed to tell the compiler map is a template 
  // http://stackoverflow.com/questions/1574721/g-doesnt-like-template-method-chaining-on-template-var
  return dunordered_map<K,R>
    (dvid.template map<my_map<K,R>>(mapvalues_dunordered_map<K,V,R,F>(f)));
}

template <class K, class V, class R, class U, class F>
struct mapvalues_dunordered_map_nloc1 {
  mapvalues_dunordered_map_nloc1() {}
  mapvalues_dunordered_map_nloc1(const F& f) : func(f) {}
  my_map<K,R> operator()(my_map<K,V>& m, U& lv1) {
    my_map<K,R> dst;
    for(auto it = m.begin(); it != m.end(); ++it) {
      dst.insert(std::make_pair(it->first,func(it->first, it->second, lv1)));
    }
    return dst;
  }
  F func;
  SERIALIZE(func)
};

template <class K, class V>
template <class R, class U, class F>
dunordered_map<K,R> dunordered_map<K,V>::map_values(const F& f,
                                                    const node_local<U>& l) {
  return dunordered_map<K,R>
    (dvid.template map<my_map<K,R>>
     (mapvalues_dunordered_map_nloc1<K,V,R,U,F>(f),l.get_dvid()));
}

template <class K, class V, class R, class U, class W, class F>
struct mapvalues_dunordered_map_nloc2 {
  mapvalues_dunordered_map_nloc2() {}
  mapvalues_dunordered_map_nloc2(const F& f) : func(f) {}
  my_map<K,R> operator()(my_map<K,V>& m, U& lv1, W& lv2) {
    my_map<K,R> dst;
    for(auto it = m.begin(); it != m.end(); ++it) {
      dst.insert(std::make_pair(it->first,func(it->first, it->second, lv1,
                                               lv2)));
    }
    return dst;
  }
  F func;
  SERIALIZE(func)
};

template <class K, class V>
template <class R, class U, class W, class F>
dunordered_map<K,R> dunordered_map<K,V>::map_values(const F& f,
                                                    const node_local<U>& l1,
                                                    const node_local<W>& l2) {

  return dunordered_map<K,R>
    (dvid.template map<my_map<K,R>>
     (mapvalues_dunordered_map_nloc2<K,V,R,U,W,F>(f),
      l1.get_dvid(), l2.get_dvid()));
}

template <class K, class V, class R, class U, class W, class X, class F>
struct mapvalues_dunordered_map_nloc3 {
  mapvalues_dunordered_map_nloc3() {}
  mapvalues_dunordered_map_nloc3(const F& f) : func(f) {}
  my_map<K,R> operator()(my_map<K,V>& m, U& lv1, W& lv2, X& lv3) {
    my_map<K,R> dst;
    for(auto it = m.begin(); it != m.end(); ++it) {
      dst.insert(std::make_pair(it->first,func(it->first, it->second, lv1,
                                               lv2, lv3)));
    }
    return dst;
  }
  F func;
  SERIALIZE(func)
};

template <class K, class V>
template <class R, class U, class W, class X, class F>
dunordered_map<K,R> dunordered_map<K,V>::map_values(const F& f,
                                                    const node_local<U>& l1,
                                                    const node_local<W>& l2,
                                                    const node_local<X>& l3) {

  return dunordered_map<K,R>
    (dvid.template map<my_map<K,R>>
     (mapvalues_dunordered_map_nloc3<K,V,R,U,W,X,F>(f),
      l1.get_dvid(), l2.get_dvid(), l3.get_dvid()));
}

template <class K, class V, class R, class U, class W, class X, class Y,
          class F>
struct mapvalues_dunordered_map_nloc4 {
  mapvalues_dunordered_map_nloc4() {}
  mapvalues_dunordered_map_nloc4(const F& f) : func(f) {}
  my_map<K,R> operator()(my_map<K,V>& m, U& lv1, W& lv2, X& lv3, Y& lv4) {
    my_map<K,R> dst;
    for(auto it = m.begin(); it != m.end(); ++it) {
      dst.insert(std::make_pair(it->first,func(it->first, it->second, lv1,
                                               lv2, lv3, lv4)));
    }
    return dst;
  }
  F func;
  SERIALIZE(func)
};

template <class K, class V>
template <class R, class U, class W, class X, class Y, class F>
dunordered_map<K,R> dunordered_map<K,V>::map_values(const F& f,
                                                    const node_local<U>& l1,
                                                    const node_local<W>& l2,
                                                    const node_local<X>& l3,
                                                    const node_local<Y>& l4) {

  return dunordered_map<K,R>
    (dvid.template map<my_map<K,R>>
     (mapvalues_dunordered_map_nloc4<K,V,R,U,W,X,Y,F>(f),
      l1.get_dvid(), l2.get_dvid(), l3.get_dvid(), l4.get_dvid()));
}

template <class K, class V, class R, class U, class W, class X, class Y,
          class Z, class F>
struct mapvalues_dunordered_map_nloc5 {
  mapvalues_dunordered_map_nloc5() {}
  mapvalues_dunordered_map_nloc5(const F& f) : func(f) {}
  my_map<K,R> operator()(my_map<K,V>& m, U& lv1, W& lv2, X& lv3, Y& lv4,
                         Z& lv5) {
    my_map<K,R> dst;
    for(auto it = m.begin(); it != m.end(); ++it) {
      dst.insert(std::make_pair(it->first,func(it->first, it->second, lv1,
                                               lv2, lv3, lv4, lv5)));
    }
    return dst;
  }
  F func;
  SERIALIZE(func)
};

template <class K, class V>
template <class R, class U, class W, class X, class Y, class Z, class F>
dunordered_map<K,R> dunordered_map<K,V>::map_values(const F& f,
                                                    const node_local<U>& l1,
                                                    const node_local<W>& l2,
                                                    const node_local<X>& l3,
                                                    const node_local<Y>& l4,
                                                    const node_local<Z>& l5) {

  return dunordered_map<K,R>
    (dvid.template map<my_map<K,R>>
     (mapvalues_dunordered_map_nloc5<K,V,R,U,W,X,Y,Z,F>(f),
      l1.get_dvid(), l2.get_dvid(), l3.get_dvid(), l4.get_dvid(),
      l5.get_dvid()));
}

template <class K, class V, class F>
class mapv_dunorderd_map {
public:
  mapv_dunorderd_map(){}
  mapv_dunorderd_map(const F& f) : func(f) {}
  void operator()(my_map<K,V>& m) {
    for(auto it = m.begin(); it != m.end(); ++it) {
      func(it->first, it->second);
    }
  }
  F func;
  SERIALIZE(func)
};

template <class K, class V>
template <class F>
dunordered_map<K,V>& dunordered_map<K,V>::mapv(const F& f) {
  dvid.mapv(mapv_dunorderd_map<K,V,F>(f));
  return *this;
}

template <class K, class V, class U, class F>
class mapv_dunorderd_map_nloc1 {
public:
  mapv_dunorderd_map_nloc1(){}
  mapv_dunorderd_map_nloc1(const F& f) : func(f) {}
  void operator()(my_map<K,V>& m, U& lv1) {
    for(auto it = m.begin(); it != m.end(); ++it) {
      func(it->first, it->second, lv1);
    }
  }
  F func;
  SERIALIZE(func)
};

template <class K, class V>
template <class U, class F>
dunordered_map<K,V>& dunordered_map<K,V>::mapv(const F& f,
                                               const node_local<U>& l) {
  dvid.mapv(mapv_dunorderd_map_nloc1<K,V,U,F>(f),l.get_dvid());
  return *this;
}

template <class K, class V, class U, class W, class F>
class mapv_dunorderd_map_nloc2 {
public:
  mapv_dunorderd_map_nloc2(){}
  mapv_dunorderd_map_nloc2(const F& f) : func(f) {}
  void operator()(my_map<K,V>& m, U& lv1, W& lv2) {
    for(auto it = m.begin(); it != m.end(); ++it) {
      func(it->first, it->second, lv1, lv2);
    }
  }
  F func;
  SERIALIZE(func)
};

template <class K, class V>
template <class U, class W, class F>
dunordered_map<K,V>& dunordered_map<K,V>::mapv(const F& f,
                                               const node_local<U>& l1,
                                               const node_local<W>& l2) {
  dvid.mapv(mapv_dunorderd_map_nloc2<K,V,U,W,F>(f),
            l1.get_dvid(),l2.get_dvid());
  return *this;
}

template <class K, class V, class U, class W, class X, class F>
class mapv_dunorderd_map_nloc3 {
public:
  mapv_dunorderd_map_nloc3(){}
  mapv_dunorderd_map_nloc3(const F& f) : func(f) {}
  void operator()(my_map<K,V>& m, U& lv1, W& lv2, X& lv3) {
    for(auto it = m.begin(); it != m.end(); ++it) {
      func(it->first, it->second, lv1, lv2, lv3);
    }
  }
  F func;
  SERIALIZE(func)
};

template <class K, class V>
template <class U, class W, class X, class F>
dunordered_map<K,V>& dunordered_map<K,V>::mapv(const F& f,
                                               const node_local<U>& l1,
                                               const node_local<W>& l2,
                                               const node_local<X>& l3) {
  dvid.mapv(mapv_dunorderd_map_nloc3<K,V,U,W,X,F>(f),
            l1.get_dvid(),l2.get_dvid(), l3.get_dvid());
  return *this;
}

template <class K, class V, class U, class W, class X, class Y, class F>
class mapv_dunorderd_map_nloc4 {
public:
  mapv_dunorderd_map_nloc4(){}
  mapv_dunorderd_map_nloc4(const F& f) : func(f) {}
  void operator()(my_map<K,V>& m, U& lv1, W& lv2, X& lv3, Y& lv4) {
    for(auto it = m.begin(); it != m.end(); ++it) {
      func(it->first, it->second, lv1, lv2, lv3, lv4);
    }
  }
  F func;
  SERIALIZE(func)
};

template <class K, class V>
template <class U, class W, class X, class Y, class F>
dunordered_map<K,V>& dunordered_map<K,V>::mapv(const F& f,
                                               const node_local<U>& l1,
                                               const node_local<W>& l2,
                                               const node_local<X>& l3,
                                               const node_local<Y>& l4) {
  dvid.mapv(mapv_dunorderd_map_nloc4<K,V,U,W,X,Y,F>(f),
            l1.get_dvid(),l2.get_dvid(), l3.get_dvid(), l4.get_dvid());
  return *this;
}

template <class K, class V, class U, class W, class X, class Y, class Z,
          class F>
class mapv_dunorderd_map_nloc5 {
public:
  mapv_dunorderd_map_nloc5(){}
  mapv_dunorderd_map_nloc5(const F& f) : func(f) {}
  void operator()(my_map<K,V>& m, U& lv1, W& lv2, X& lv3, Y& lv4, Z& lv5) {
    for(auto it = m.begin(); it != m.end(); ++it) {
      func(it->first, it->second, lv1, lv2, lv3, lv4, lv5);
    }
  }
  F func;
  SERIALIZE(func)
};

template <class K, class V>
template <class U, class W, class X, class Y, class Z, class F>
dunordered_map<K,V>& dunordered_map<K,V>::mapv(const F& f,
                                               const node_local<U>& l1,
                                               const node_local<W>& l2,
                                               const node_local<X>& l3,
                                               const node_local<Y>& l4,
                                               const node_local<Z>& l5) {
  dvid.mapv(mapv_dunorderd_map_nloc5<K,V,U,W,X,Y,Z,F>(f),
            l1.get_dvid(),l2.get_dvid(), l3.get_dvid(), l4.get_dvid(),
            l5.get_dvid());
  return *this;
}

template <class K, class V, class F>
class filter_dunordered_map {
public:
  filter_dunordered_map(){}
  filter_dunordered_map(const F& f) : func(f) {}
  my_map<K,V> operator()(my_map<K,V>& src) {
    my_map<K,V> dst;
    for(auto it = src.begin(); it != src.end(); ++it) {
      if(func(it->first, it->second)) dst.insert(*it);
    }
    return dst;
  }
  F func;
  SERIALIZE(func)
};

template <class K, class V>
template <class F>
dunordered_map<K,V> dunordered_map<K,V>::filter(const F& f) {
  return dunordered_map<K,V>(dvid.template map<my_map<K,V>>
                             (filter_dunordered_map<K,V,F>(f)));  
}

template <class K, class V, class F>
class inplace_filter_dunordered_map {
public:
  inplace_filter_dunordered_map(){}
  inplace_filter_dunordered_map(const F& f) : func(f) {}
  void operator()(my_map<K,V>& m) {
    for(auto it = m.begin(); it != m.end();){
      if(func(it->first, it->second)) {
        ++it;
      } else {
        it = m.erase(it);
      }
    }
  }
  F func;
  SERIALIZE(func)
};

template <class K, class V>
template <class F>
dunordered_map<K,V>& dunordered_map<K,V>::inplace_filter(const F& f) {
  dvid.mapv(inplace_filter_dunordered_map<K,V,F>(f));
  return *this;
}

template <class K, class V>
void put_dunordered_map_helper(DVID<my_map<K,V>>& dvid, int& n,
                               intptr_t& keyptr, intptr_t& valptr) {
  int self = get_selfid();
  if(self == 0) {
    K& key = *reinterpret_cast<K*>(keyptr);
    V& val = *reinterpret_cast<V*>(valptr);
    send_data_helper(n, key);
    send_data_helper(n, val);
  } else if(self == n) {
    K key;
    V val;
    receive_data_helper(0, key);
    receive_data_helper(0, val);
    (*dvid.get_selfdata())[key] = val;
  }
}

template <class K, class V>
void dunordered_map<K,V>::put(const K& key, const V& val) {
  size_t nodes = get_nodesize();
  int n = my_hash<K>()(key) % nodes;
  if(n == 0) {
    (*dvid.get_selfdata())[key] = val;
  } else {
    intptr_t keyptr = reinterpret_cast<intptr_t>(&key);
    intptr_t valptr = reinterpret_cast<intptr_t>(&val);    
    bcast_rpc_oneway(put_dunordered_map_helper<K,V>, dvid, n, keyptr, valptr);
  }
  return;
}

template <class K, class V>
void get_dunordered_map_helper(DVID<my_map<K,V>>& dvid, int& n, 
                               intptr_t& keyptr, intptr_t& valptr) {
  int self = get_selfid();
  if(self == 0) {
    K& key = *reinterpret_cast<K*>(keyptr);
    V& val = *reinterpret_cast<V*>(valptr);
    send_data_helper(n, key);
    receive_data_helper(n, val);
  } else if(self == n) {
    K key;
    receive_data_helper(0, key);
    send_data_helper(0, (*dvid.get_selfdata())[key]);
  }
}

template <class K, class V>
V dunordered_map<K,V>::get(const K& key) {
  size_t nodes = get_nodesize();
  int n = my_hash<K>()(key) % nodes;
  if(n == 0) {
    return (*dvid.get_selfdata())[key];
  } else {
    V val;
    intptr_t keyptr = reinterpret_cast<intptr_t>(&key);
    intptr_t valptr = reinterpret_cast<intptr_t>(&val);    
    bcast_rpc_oneway(get_dunordered_map_helper<K,V>, dvid, n, keyptr, valptr);
    return val;
  }
}

template <class K, class V>
void get_dunordered_map_helper2(DVID<my_map<K,V>>& dvid, int& n,
                               intptr_t& keyptr, intptr_t& valfoundptr) {
  int self = get_selfid();
  if(self == 0) {
    K& key = *reinterpret_cast<K*>(keyptr);
    std::pair<V,bool>& val_found = 
      *reinterpret_cast<std::pair<V,bool>*>(valfoundptr);
    send_data_helper(n, key);
    receive_data_helper(n, val_found);
  } else if(n == get_selfid()) {
    K key;
    V val;
    bool found;
    receive_data_helper(0, key);
    auto to_check = *dvid.get_selfdata();
    auto it = to_check.find(key);
    if(it == to_check.end()) {
      found = false;
      val = V();
    } else {
      found = true;
      val = it->second;
    }
    send_data_helper(0, std::make_pair(val,found));
  }
}

template <class K, class V>
V dunordered_map<K,V>::get(const K& key, bool& found) {
  size_t nodes = get_nodesize();
  int n = my_hash<K>()(key) % nodes;
  if(n == 0) {
    auto to_check = *dvid.get_selfdata();
    auto it = to_check.find(key);
    if(it == to_check.end()) {
      found = false;
      return V();
    } else {
      found = true;
      return it->second;
    }
  } else {
    std::pair<V,bool> val_found;
    intptr_t keyptr = reinterpret_cast<intptr_t>(&key);
    intptr_t valfoundptr = reinterpret_cast<intptr_t>(&val_found);
    bcast_rpc_oneway(get_dunordered_map_helper2<K,V>, dvid, n, keyptr,
                     valfoundptr);
    found = val_found.second;
    return val_found.first;
  }
}

template <class K, class V>
std::vector<std::pair<K,V>>
  asdvector_dunordered_map(my_map<K,V>& src) {
  std::vector<std::pair<K,V>> ret;
  for(auto it = src.begin(); it != src.end(); ++it) {
    ret.push_back(*it);
  }
  return ret;
}

template <class K, class V>
dvector<std::pair<K,V>> dunordered_map<K,V>::as_dvector() {
  return dvector<std::pair<K,V>>
    (dvid.template map<std::vector<std::pair<K,V>>>
     (asdvector_dunordered_map<K,V>));
}

template <class K, class V>
void clear_unordered_map(my_map<K,V>& m){my_map<K,V> tmp; tmp.swap(m);}

template <class K, class V>
void dunordered_map<K,V>::clear() {
  dvid.mapv(clear_unordered_map<K,V>);
}

template <class T>
std::vector<size_t> get_container_size(const T& c){
  return std::vector<size_t>({c.size()});
}

template <class K, class V>
size_t dunordered_map<K,V>::size() {
  dvector<size_t> s(dvid.template map<std::vector<size_t>>
                    (get_container_size<my_map<K,V>>));
  return s.reduce(add<size_t>);
}

/*
  dvector::group_by_key
*/
template <class K, class V>
my_map<K, std::vector<V>>
  group_by_key_merge(std::vector<my_map<K, std::vector<V>>>& src) {
  my_map<K, std::vector<V>> ret = std::move(src[0]);
  for(size_t i = 1; i < src.size(); i++) {
    for(auto it = src[i].begin(); it != src[i].end(); ++it) {
      auto jt = ret.find(it->first);
      if(jt != ret.end()) 
        jt->second.insert(jt->second.end(),
                          it->second.begin(), it->second.end());
      else
        ret.insert(std::move(*it));
    }
  }
  return ret;
}

template <class K, class V>
struct group_by_key_local {
  group_by_key_local(){}
  group_by_key_local
  (const DVID<std::vector<my_map<K,std::vector<V>>>>& dvid_) : dvid(dvid_) {}
  void operator()(std::vector<std::pair<K,V>>& v) {
    size_t nodes = get_nodesize();
    dvid.get_selfdata()->resize(nodes);
    std::vector<my_map<K,std::vector<V>>>& dic = *dvid.get_selfdata();
    for(auto it = v.begin(); it != v.end(); ++it) {
      size_t n = my_hash<K>()(it->first) % nodes;
      auto jt = dic[n].find(it->first);
      if(jt != dic[n].end()) 
        jt->second.push_back(it->second);
      else {
        std::vector<V> v;
        v.push_back(it->second);
        dic[n].insert(std::make_pair(it->first, v));
      }
    }
  }
  DVID<std::vector<my_map<K,std::vector<V>>>> dvid;  
  SERIALIZE(dvid)
};

/*
  non-member function for type inference
  (in this case, types need not be supplied)
*/
template <class K, class V>
dunordered_map<K,std::vector<V>> 
group_by_key(dvector<std::pair<K,V>>& dvector) {
  auto tmp = make_dvid_allocate<std::vector<my_map<K,std::vector<V>>>>();
  dvector.get_dvid().mapv(group_by_key_local<K,V>(tmp));
  auto tmp2 = alltoall_exchange(tmp);
  tmp.delete_var();
  auto tmp3 = tmp2
    .template map<my_map<K,std::vector<V>>>(group_by_key_merge<K,V>);
  tmp2.delete_var();
  return dunordered_map<K,std::vector<V>>(std::move(tmp3));
}
/*
  member function: <K,V> need to be supplied as template arguments...
*/
template <class T>
template <class K, class V>
dunordered_map<K,std::vector<V>> dvector<T>::group_by_key() {
  return frovedis::group_by_key<K,V>(*this);
}

/*
  dvector::reduce_by_key
*/
template <class K, class V, class F>
struct reduce_by_key_local {
  reduce_by_key_local(){}
  reduce_by_key_local(const F& f) : func(f) {}
  void operator()(const std::pair<K,V>& kv,
                  my_map<K,V>& dic) {
    auto it = dic.find(kv.first);
    if(it == dic.end()) {
      dic.insert(kv);
    } else {
      it->second = func(it->second, kv.second);
    }
  }
  F func;
  SERIALIZE(func)
};

template <class K, class V, class F>
struct vector_reduce {
  vector_reduce(){}
  vector_reduce(const F& f) : func(f) {}
  V operator()(const K key, const std::vector<V>& v) {
    // v.size() never be zero
    V ret = v[0];
    for(size_t i = 1; i < v.size(); i++)
      ret = func(ret, v[i]);
    return ret;
  }
  F func;
  SERIALIZE(func)
};

template <class K, class V>
struct maptovector {
  maptovector(){}
  std::vector<std::pair<K,V>> operator()(my_map<K,V>& dic) {
    std::vector<std::pair<K,V>> ret;
    for(auto it = dic.begin(); it != dic.end(); ++it) {
      ret.push_back(*it);
    }
    return ret;
  }
  SERIALIZE_NONE
};

template <class K, class V, class F>
dunordered_map<K,V>
reduce_by_key(dvector<std::pair<K,V>>& dvector, const F& f) {
  auto dic = make_node_local_allocate<my_map<K,V>>();
  dvector.mapv(reduce_by_key_local<K,V,F>(f), dic);
  auto vdic = dic.
    template map<std::vector<std::pair<K,V>>>
    (maptovector<K,V>()).template moveto_dvector<std::pair<K,V>>();
  return vdic.template group_by_key<K,V>().
    template map_values<V>(vector_reduce<K,V,F>(f));
}


template <class K, class V, class VV, class WW>
dunordered_map<K,V> reduce_by_key(dvector<std::pair<K,V>>& dv, 
                                  V(*f)(VV,WW)){
  return frovedis::reduce_by_key<K,V>(dv,make_serfunc(f));
}

template <class T>
template <class K, class V, class F>
dunordered_map<K,V> dvector<T>::reduce_by_key(const F& f) {
  return frovedis::reduce_by_key<K,V>(*this, f);
}

}

#endif
