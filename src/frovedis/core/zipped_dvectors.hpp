#ifndef ZIPPEDDVECTORS_HPP
#define ZIPPEDDVECTORS_HPP

#include "dvector.hpp"

namespace frovedis {

template <class V1, class V2>
class zipped_dvectors {
public:
  zipped_dvectors(): vec1(nullptr), vec2(nullptr) {}
  zipped_dvectors(dvector<V1>& vec1_, dvector<V2>& vec2_) :
    vec1(&vec1_), vec2(&vec2_) {
    if(vec1->size() != vec2->size()) {
      throw std::runtime_error("Cannot construct zipped_dvectors: sizes of two vectors are different");
    } else
      vec2->align_to(*vec1);
  }
  template <class R, class F> dvector<R> map(const F&);
  template <class R, class VV1, class VV2> dvector<R> map(R(*f)(VV1,VV2)) {
    return map<R>(make_serfunc(f));
  }
  template <class F> zipped_dvectors<V1,V2>& mapv(const F&);
  template <class VV1, class VV2>
  zipped_dvectors<V1,V2>& mapv(void(*f)(VV1,VV2)) {
    return mapv(make_serfunc(f));
  }
/*
  template <class R, class U, class F> dvector<R> map(const F&, node_local<U>&);
  template <class R, class U, class VV1, class VV2, class UU>
  dvector<R> map(R(*f)(VV1, VV2, UU), node_local<U>& l) {
    return map<R,U>(make_serfunc(f),l);
  }
  template <class R, class U, class V, class F>
  dvector<R> map(const F&, node_local<U>&, node_local<V>&);
  template <class R,class U,class V,class VV1,class VV2,class UU,class VV>
  dvector<R> map(R(*f)(VV1, VV2, UU, VV), node_local<U>& l1, node_local<V>& l2) {
    return map<R,U,V>(make_serfunc(f),l1,l2);
  }
*/
  /*
    mapv, 
    mapv w/ node_local
    flatmap?
   */
private:
  dvector<V1>* vec1;
  dvector<V2>* vec2;
  SERIALIZE(vec1, vec2)
};

template <class V1, class V2>
zipped_dvectors<V1, V2> zip(dvector<V1>& v1, dvector<V2>& v2) {
  return zipped_dvectors<V1,V2>(v1,v2);
}

template <class V1, class V2, class R, class F>
class map_zippeddvectors {
public:
  map_zippeddvectors(){}
  map_zippeddvectors(const F& f) : func(f) {}
  std::vector<R> operator()(std::vector<V1>& vec, std::vector<V2>& v2) {
    auto size = vec.size();
    std::vector<R> dst(size);
    auto dstp = dst.data();
    auto vecp = vec.data();
    auto v2p = v2.data();
    for(size_t i = 0; i < size; i++) {
      dstp[i] = func(vecp[i], v2p[i]);
    }
    return dst;
  }
  F func;
  SERIALIZE(func)
};

template <class V1, class V2>
template <class R, class F>
dvector<R> zipped_dvectors<V1,V2>::map(const F& f) {
  dvector<R> ret(vec1->get_dvid().template map<std::vector<R>>
                 (map_zippeddvectors<V1,V2,R,F>(f), vec2->get_dvid()));
  ret.set_sizes(vec1->sizes());
  return ret;
}

template <class V1, class V2, class F>
class mapv_zippeddvectors {
public:
  mapv_zippeddvectors(){}
  mapv_zippeddvectors(const F& f) : func(f) {}
  void operator()(std::vector<V1>& vec, std::vector<V2>& v2) {
    auto size = vec.size();
    auto vecp = vec.data();
    auto v2p = v2.data();
    for(size_t i = 0; i < size; i++) {
      func(vecp[i],v2p[i]);
    }
  }
  F func;
  SERIALIZE(func)
};

template <class V1, class V2>
template <class F>
zipped_dvectors<V1,V2>& zipped_dvectors<V1,V2>::mapv(const F& f) {
  vec1->get_dvid().mapv(mapv_zippeddvectors<V1,V2,F>(f), vec2->get_dvid());
  return *this;
}

}

#endif
