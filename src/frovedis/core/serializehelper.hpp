#ifndef SERIALIZEHELPER_HPP
#define SERIALIZEHELPER_HPP

#include "config.hpp"
#include <boost/preprocessor.hpp>

// the idea is borrowed from http://yodancpp.blogspot.jp/2011/11/serialize.html
#define PP_VA_ARGS_SIZE(...)                    \
  PP_VA_ARGS_SIZE_I((__VA_ARGS__,               \
                     48,47,46,45,44,43,42,41,   \
                     40,39,38,37,36,35,34,33,   \
                     32,31,30,29,28,27,26,25,   \
                     24,23,22,21,20,19,18,17,   \
                     16,15,14,13,12,11,10,9,    \
                     8,7,6,5,4,3,2,1))

#define PP_VA_ARGS_SIZE_I(tuple) PP_VA_ARGS_SIZE_II tuple

#define PP_VA_ARGS_SIZE_II(_1,_2,_3,_4,_5,_6,_7,_8,             \
                           _9,_10,_11,_12,_13,_14,_15,_16,      \
                           _17,_18,_19,_20,_21,_22,_23,_24,     \
                           _25,_26,_27,_28,_29,_30,_31,_32,     \
                           _33,_34,_35,_36,_37,_38,_39,_40,     \
                           _41,_42,_43,_44,_45,_46,_47,_48,     \
                           size,...) size

#define PP_SERIALIZATION(z,count,array)         \
  & BOOST_PP_ARRAY_ELEM(count,array)

#if defined(USE_BOOST_SERIALIZATION)
#define SERIALIZE(...)                          \
  public:                                       \
  template<class Ar>                            \
  void serialize(Ar & ar,const unsigned int){   \
    ar SERIALIZE_I((__VA_ARGS__));              \
  }                                             \
  template<class Ar>                            \
  void serialize(Ar & ar){                      \
    ar SERIALIZE_I((__VA_ARGS__));              \
  }
#else
#define SERIALIZE(...)                          \
  public:                                       \
  template<class Ar>                            \
  void serialize(Ar & ar){                      \
    ar SERIALIZE_I((__VA_ARGS__));              \
  }
#endif

#define SERIALIZE_I(tuple)                              \
  SERIALIZE_II(( PP_VA_ARGS_SIZE tuple,                 \
                 PP_SERIALIZATION,                      \
                 (PP_VA_ARGS_SIZE tuple,tuple) ))

#define SERIALIZE_II(tuple) BOOST_PP_REPEAT tuple

#if defined(USE_BOOST_SERIALIZATION)
#define SERIALIZE_NONE                          \
  public:                                       \
  template<class Ar>                            \
  void serialize(Ar & ar,const unsigned int){}  \
  template<class Ar>                            \
  void serialize(Ar & ar){}            
#else
#define SERIALIZE_NONE                          \
  public:                                       \
  template<class Ar>                            \
  void serialize(Ar & ar){}            
#endif

#endif
