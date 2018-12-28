#ifndef _WORD2VEC_UTILITY_
#define _WORD2VEC_UTILITY_

#include "w2v_common.hpp"
#include <string>

namespace w2v {

void build_vocab_and_dump(
  const std::string& _train_file_str, 
  const std::string& _stream_file_str,
  const std::string& _vocab_file_str, 
  const std::string& _vocab_count_file_str,
  int min_count
);
                          
void save_model(
  const std::string& _weight_file_str,
  const std::string& _vocab_file_str,
  const std::string& _output_file_str,
  const int min_count,
  bool binary
);

void save_model(
  const std::vector<real>& _weight,
  const std::string& _vocab_file_str,
  const std::string& _output_file_str,
  const int min_count,
  bool binary
);

}  // namespace w2v

#endif  // _WORD2VEC_UTILITY_
