#include <frovedis.hpp>
#include <frovedis/ml/w2v/word2vec.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  // frovedis::set_loglevel(DEBUG);

  int min_count = 5;

  const std::string train_file = "text8-10k"; 
  const std::string stream_file = "indexed-train";
  const std::string vocab_file = "vocab";
  const std::string vocab_count_file = "vocab-count";
  const std::string word_vectors_file = "vectors.txt";
  frovedis::w2v_build_vocab_and_dump(
    train_file, stream_file, vocab_file, vocab_count_file, min_count);

  auto nl_train_data = frovedis::make_dvector_loadbinary<int>(stream_file).moveto_node_local();  
  auto vocab_count = frovedis::make_dvector_loadbinary<int>(vocab_count_file).gather();
  
  int hidden_size = 100;
  int window = 5;
  float sample = 1e-3f;
  int negative = 5;
  int iter = 5;
  float alpha = 0.1;
  float model_sync_period = 0.1;
  int min_sync_words = 1024;
  int full_sync_times = 0;
  int message_size = 1024;
  int num_threads = 2;  // Carefully choose this
  
  auto weight = frovedis::w2v_train(
    nl_train_data, vocab_count, hidden_size, window, sample, negative, iter,
    alpha, model_sync_period, min_sync_words, full_sync_times, message_size, num_threads);

  bool binary = false;
  frovedis::w2v_save_model(weight, vocab_file, word_vectors_file, binary, min_count);
  if (frovedis::get_selfid() == 0) {
    std::cout << "weight is saved." << std::endl;
  }
}
