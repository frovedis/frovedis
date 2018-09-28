#ifndef ZIPPEDDUNORDERED_MAP_HPP
#define ZIPPEDDUNORDERED_MAP_HPP

#include "dunordered_map.hpp"
#ifdef USE_CEREAL_FOR_RPC
// copied from https://github.com/USCiLab/cereal/blob/de79ac166b33057b85ec54207930fc67ea7d6cc6/include/cereal/types/boost_optional.hpp
// replaced _CEREAL_NVP to CEREAL_NVP_ to support current version
#include "boost_optional.hpp"
#else
#include <boost/serialization/optional.hpp>
#endif

namespace frovedis {

template <class K, class V1, class V2>
class zipped_dunordered_maps {
public:
  zipped_dunordered_maps(): map1(nullptr), map2(nullptr) {}
  zipped_dunordered_maps(dunordered_map<K,V1>& map1_,
                        dunordered_map<K,V2>& map2_) :
    map1(&map1_), map2(&map2_) {}
  template <class R, class F>
  dunordered_map<K,R> map_values(const F&);
  template <class R, class KK, class VV1, class VV2>
  dunordered_map<K,R> map_values(R(*f)(KK,VV1,VV2)) {
    return map_values<R>(make_serfunc(f));
  }
  template <class R, class F>
  dunordered_map<K,R> leftouter_map_values(const F&);
  template <class R, class KK, class VV1, class VV2>
  dunordered_map<K,R> leftouter_map_values(R(*f)(KK,VV1,VV2)) {
    return leftouter_map_values<R>(make_serfunc(f));
  }

  template <class F>
  zipped_dunordered_maps<K,V1,V2>& mapv(const F&);
  template <class KK, class VV1, class VV2>
  zipped_dunordered_maps<K,V1,V2>& mapv(void(*f)(KK,VV1,VV2)) {
    return mapv(make_serfunc(f));
  }
  template <class F>
  zipped_dunordered_maps<K,V1,V2>& leftouter_mapv(const F&);
  template <class KK, class VV1, class VV2>
  zipped_dunordered_maps<K,V1,V2>& leftouter_mapv(void(*f)(KK,VV1,VV2)) {
    return leftouter_mapv(make_serfunc(f));
  }

private:
  dunordered_map<K,V1>* map1;
  dunordered_map<K,V2>* map2;
  SERIALIZE(map1, map2)
};

template <class K, class V1, class V2>
zipped_dunordered_maps<K, V1, V2> zip(dunordered_map<K,V1>& m1,
                                     dunordered_map<K,V2>& m2) {
  return zipped_dunordered_maps<K,V1,V2>(m1,m2);
}

template <class K, class V1, class V2, class R, class F>
class mapvalues_zippeddunordered_maps {
public:
  mapvalues_zippeddunordered_maps(){}
  mapvalues_zippeddunordered_maps(const F& f) : func(f) {}
  my_map<K,R> operator()(my_map<K,V1>& m1,
                         my_map<K,V2>& m2) {
    my_map<K,R> dst;
    for(auto it = m1.begin(); it != m1.end(); ++it) {
      auto jt = m2.find(it->first);
      if(jt != m2.end())
        dst.insert(std::make_pair(it->first,
                                  func(it->first, it->second, jt->second)));
    }
    return dst;
  }
  F func;
  SERIALIZE(func)
};

template <class K, class V1, class V2>
template <class R, class F>
dunordered_map<K,R> zipped_dunordered_maps<K,V1,V2>::map_values(const F& f) {
  return dunordered_map<K,R>
    (map1->get_dvid().template map<my_map<K,R>>
     (mapvalues_zippeddunordered_maps<K,V1,V2,R,F>(f),
      map2->get_dvid()));
}

template <class K, class V1, class V2, class R, class F>
class leftoutermapvalues_zippeddunordered_maps {
public:
  leftoutermapvalues_zippeddunordered_maps(){}
  leftoutermapvalues_zippeddunordered_maps(const F& f) : func(f) {}
  my_map<K,R> operator()(my_map<K,V1>& m1,
                         my_map<K,V2>& m2) {
    my_map<K,R> dst;
    for(auto it = m1.begin(); it != m1.end(); ++it) {
      auto jt = m2.find(it->first);
      if(jt != m2.end()) {
        dst.insert(std::make_pair(it->first,
                                  func(it->first, it->second,
                                       boost::optional<V2>(jt->second))));
      } else {
        dst.insert(std::make_pair(it->first,
                                  func(it->first, it->second,
                                       boost::optional<V2>(boost::none))));
      }
    }
    return dst;
  }
  F func;
  SERIALIZE(func)
};

template <class K, class V1, class V2>
template <class R, class F>
dunordered_map<K,R>
zipped_dunordered_maps<K,V1,V2>::leftouter_map_values(const F& f) {
  return dunordered_map<K,R>
    (map1->get_dvid().template map<my_map<K,R>>
     (leftoutermapvalues_zippeddunordered_maps<K,V1,V2,R,F>(f),
      map2->get_dvid()));
}


template <class K, class V1, class V2, class F>
class mapv_zippeddunordered_maps {
public:
  mapv_zippeddunordered_maps(){}
  mapv_zippeddunordered_maps(const F& f) : func(f) {}
  void operator()(my_map<K,V1>& m1,
                  my_map<K,V2>& m2) {
    for(auto it = m1.begin(); it != m1.end(); ++it) {
      auto jt = m2.find(it->first);
      if(jt != m2.end())
        func(it->first, it->second, jt->second);
    }
  }
  F func;
  SERIALIZE(func)
};

template <class K, class V1, class V2>
template <class F>
zipped_dunordered_maps<K,V1,V2>&
zipped_dunordered_maps<K,V1,V2>::mapv(const F& f) {
  map1->get_dvid().mapv(mapv_zippeddunordered_maps<K,V1,V2,F>(f),
                        map2->get_dvid());
  return *this;
}

template <class K, class V1, class V2, class F>
class leftoutermapv_zippeddunordered_maps {
public:
  leftoutermapv_zippeddunordered_maps(){}
  leftoutermapv_zippeddunordered_maps(const F& f) : func(f) {}
  void operator()(my_map<K,V1>& m1,
                  my_map<K,V2>& m2) {
    for(auto it = m1.begin(); it != m1.end(); ++it) {
      auto jt = m2.find(it->first);
      if(jt != m2.end()) {
        func(it->first, it->second, boost::optional<V2>(jt->second));
      } else {
        func(it->first, it->second, boost::optional<V2>(boost::none));
      }
    }
  }
  F func;
  SERIALIZE(func)
};

template <class K, class V1, class V2>
template <class F>
zipped_dunordered_maps<K,V1,V2>&
zipped_dunordered_maps<K,V1,V2>::leftouter_mapv(const F& f) {
  map1->get_dvid().mapv(leftoutermapv_zippeddunordered_maps<K,V1,V2,F>(f),
                        map2->get_dvid());
  return *this;
}

}

#endif
