/*
 * The code is developed based on the below implementations.
 * https://code.google.com/archive/p/word2vec/
 * https://github.com/IntelLabs/pWord2Vec/
 */
 
#ifndef _WORD2VEC_HPP_
#define _WORD2VEC_HPP_

#include "../../matrix/rowmajor_matrix.hpp"

namespace frovedis {
  
namespace {
typedef float real;
}
    
rowmajor_matrix_local<real> w2v_train(
    node_local<std::vector<int>>& _nl_train_data, std::vector<int>& _vocab_count,
    int _hidden_size, int _window, real _sample, int _negative, int _iter, int _min_count,
    real _alpha, int _batch_size, real _model_sync_period, int _min_sync_words,
    int _full_sync_times, int _message_size, int _num_threads);
    
void w2v_build_vocab_and_dump(const std::string& _train_file_str, const std::string& _stream_file_str,
                              const std::string& _vocab_file_str, const std::string& _vocab_count_file_str);
                              
void w2v_save_model(const rowmajor_matrix_local<real>& weight,
                    const std::string& _vocab_file_str,
                    const std::string& _output_file_str);
    
}  // namespace frovedis

#endif  // _WORD2VEC_HPP_
