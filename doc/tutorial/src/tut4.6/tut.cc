#include <frovedis.hpp>
#include <frovedis/ml/w2v/word2vec.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  // frovedis::set_loglevel(DEBUG);

  int min_count = 5;

  frovedis::w2v_build_vocab_and_dump(
    "./text8-10k", "./indexed-train", "./vocab.bin", "./vocab-count", min_count);

  auto nl_train_data = frovedis::make_dvector_loadbinary<int>("./indexed-train").moveto_node_local();  
  auto vocab_count = frovedis::make_dvector_loadbinary<int>("./vocab-count").gather();
  
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

  frovedis::w2v_save_model(weight, "./vocab.bin", "./vectors.bin", false);
  if (frovedis::get_selfid() == 0) {
    std::cout << "weight is saved to 'vectors.bin'." << std::endl;
  }
}
