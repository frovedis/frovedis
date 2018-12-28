/*
 * The code is developed based on the below implementations.
 * https://code.google.com/archive/p/word2vec/
 * https://github.com/IntelLabs/pWord2Vec/
 */
 
#ifndef _WORD2VEC_HPP_
#define _WORD2VEC_HPP_

#include "word2vec_serialize.hpp"
#include "w2v_core.hpp"
#include "w2v_corpus.hpp"
#include "../../matrix/rowmajor_matrix.hpp"

namespace frovedis {
  
namespace {
  using real = w2v::real;
  //typedef w2vfloat real;
}

#if 0    
rowmajor_matrix_local<real> w2v_train(
    node_local<std::vector<int>>& _nl_train_data, std::vector<int>& _vocab_count,
    int _hidden_size, int _window, real _sample, int _negative, int _iter,
    real _alpha, real _model_sync_period, int _min_sync_words,
    int _full_sync_times, int _message_size, int _num_threads);
    
void w2v_build_vocab_and_dump(const std::string& _train_file_str, const std::string& _stream_file_str,
			      const std::string& _vocab_file_str, const std::string& _vocab_count_file_str, int _min_count);
                              
void w2v_save_model(const rowmajor_matrix_local<real>& weight,
                    const std::string& _vocab_file_str,
                    const std::string& _output_file_str);
#endif


inline rowmajor_matrix_local<real> w2v_train(
    node_local<std::vector<int>>& _nl_train_data, std::vector<int>& _vocab_count,
    int _hidden_size = 100, int _window = 5, real _sample = 1e-3f, int _negative = 5, int _iter = 5,
    real _alpha = 0.1, real _model_sync_period = 0.1, int _min_sync_words = 1024,
    int _full_sync_times = 0, int _message_size = 1024, int _num_threads = 0)
{
  w2v::train_config _config = {_hidden_size, _window,
    _sample, _negative, _iter,
    _alpha, _model_sync_period, _min_sync_words,
    _full_sync_times, _message_size, _num_threads};

  auto model_vector = _nl_train_data.map(w2v::train_each, broadcast(_vocab_count), broadcast(_config)).get(0);
  
  auto _vocab_size = _vocab_count.size();
  rowmajor_matrix_local<real> ret(std::move(model_vector));
  ret.set_local_num(_vocab_size, _hidden_size);
  return ret;
}

inline void w2v_build_vocab_and_dump(
    const std::string& _train_file_str, const std::string& _stream_file_str,
    const std::string& _vocab_file_str, const std::string& _vocab_count_file_str, int _min_count = 5)
{
  w2v::build_vocab_and_dump(_train_file_str, _stream_file_str, _vocab_file_str, _vocab_count_file_str, _min_count);
}

inline void w2v_save_model(
    const rowmajor_matrix_local<real>& weight,
    const std::string& _vocab_file_str,
    const std::string& _output_file_str,
    const bool binary,
    const int _min_count = 5)
{
  w2v::save_model(weight.val, _vocab_file_str, _output_file_str, _min_count, binary);
}


}  // namespace frovedis

#endif  // _WORD2VEC_HPP_
