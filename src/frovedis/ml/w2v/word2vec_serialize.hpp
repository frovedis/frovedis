/*
 * The code is developed based on the below implementations.
 * https://code.google.com/archive/p/word2vec/
 * https://github.com/IntelLabs/pWord2Vec/
 */
 
#ifndef _WORD2VEC_SERIALIZE_HPP_
#define _WORD2VEC_SERIALIZE_HPP_


#include "frovedis.hpp"
#include "w2v_core.hpp"
#include "../../core/config.hpp"


#if defined(USE_CEREAL) 
namespace cereal {
template <class Archive>
void serialize(Archive& ar, w2v::train_config& m)
{
  ar(
     m.hidden_size,
     m.window,
     m.sample,
     m.negative,
     m.iter,
     m.alpha,
     m.model_sync_period,
     m.min_sync_words,
     m.full_sync_times,
     m.message_size,
     m.num_threads
  );
}
}  // namespace cereal 
#endif


#if defined(USE_BOOST_SERIALIZATION)
namespace boost {
namespace serialization {
template <class Archive>
void serialize(Archive& ar, w2v::train_config& m, const unsigned int version)
{
  ar & m.hidden_size;
  ar &  m.window;
  ar &  m.sample;
  ar &  m.negative;
  ar &  m.iter;
  ar &  m.alpha;
  ar & m.model_sync_period;
  ar & m.min_sync_words;
  ar & m.full_sync_times;
  ar & m.message_size;
  ar & m.num_threads;  
}
}  // namespace serialization
}  // namespace boost
#endif


#endif  // _WORD2VEC_SERIALIZE_HPP_
