#ifndef TUPLE_SERIALIZER_HPP
#define TUPLE_SERIALIZER_HPP

#ifdef USE_BOOST_SERIALIZATION

#include <tuple>

namespace boost {
namespace serialization {

template<uint N>
struct tuple_serialize_helper {
  template<class Ar, typename... Ts>
  static void serialize(Ar & ar, std::tuple<Ts...> & t,
                        const unsigned int ver) {
      ar & std::get<N-1>(t);
      tuple_serialize_helper<N-1>::serialize(ar, t, ver);
  }
};

template<>
struct tuple_serialize_helper<0> {
  template<class Ar, typename... Ts>
  static void serialize(Ar & ar, std::tuple<Ts...> & t,
                        const unsigned int ver) {}
};

template<class Ar, typename... Ts>
void serialize(Ar & ar, std::tuple<Ts...> & t, const unsigned int ver) {
  tuple_serialize_helper<sizeof...(Ts)>::serialize(ar, t, ver);
}

}
}

#endif

#ifdef USE_CEREAL
#include <cereal/types/tuple.hpp>
#endif

#ifdef USE_YAS
#include <yas/types/std/tuple.hpp>
#endif

#endif
