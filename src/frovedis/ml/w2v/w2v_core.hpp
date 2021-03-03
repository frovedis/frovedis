  /*
 * Copyright 2018 NEC Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * The code is developed based on the below implementations.
 * https://code.google.com/archive/p/word2vec/
 * https://github.com/IntelLabs/pWord2Vec/
 */
 
#ifndef _W2V_CORE_HPP_
#define _W2V_CORE_HPP_

//#include "../w2v_common.hpp"
#include <string>
#include <vector>

namespace w2v {
  
namespace {
typedef float real;
}
      
class train_config {
public:
  int hidden_size;
  int window;
  real sample;
  int negative;
  int iter;
  //int min_count;
  real alpha;
  //int batch_size;
  real model_sync_period;
  int min_sync_words;
  int full_sync_times;
  int message_size;
  int num_threads;
};

std::vector<float> train_each(
    std::vector<int> &_proc_train_data,
    std::vector<int> &_vocab_count,
    train_config &_config
);

void train_each(
    std::string& _train_file_str, 
    std::string& _vocab_count_file_str,
    std::string& _weight_file_str,
    train_config& _config
);

std::vector<float> train_each_impl(
    std::string& _train_file_str,
    std::string& _vocab_count_file_str,
    train_config& _config
);
    
}  // namespace w2v

#endif  // _W2V_CORE_HPP_
