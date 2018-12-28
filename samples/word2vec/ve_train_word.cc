
#include <iostream>

#include <frovedis.hpp>
#include <frovedis/ml/w2v/word2vec.hpp>
#include <boost/program_options.hpp>


using namespace frovedis;
using namespace boost::program_options;


int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);

  std::string encode_path, vocab_path, vocab_count_path, weight_path;
  int hidden_size, window, negative, iter, min_count, min_sync_words, full_sync_times, message_size;
  int num_threads;
  float sample, alpha, model_sync_period;
  
  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input,i", value<std::string>(&encode_path)->required(), "path for input encoded text.")
    ("vocab,v", value<std::string>(&vocab_path)->required(), "path for input binary file of vocabulary.")
    ("vocab-count,c", value<std::string>(&vocab_count_path)->required(), "path for input binary file of vocabulary count.")
    ("output,o", value<std::string>(&weight_path)->required(), "path for output file of weight matrix")
  
    ("size", value<int>(&hidden_size)->default_value(100, "100"), 
     "size of word vectors.")
    ("window", value<int>(&window)->default_value(5, "5"), 
     "max skip length between words")
    ("sample", value<float>(&sample)->default_value(1e-3f, "1e-3, useful range is (0, 1e-5)"), 
     "threshold for occurrence of words")
    ("negative", value<int>(&negative)->default_value(5, "5"), 
     "Number of negative examples")
    ("iter", value<int>(&iter)->default_value(5, "5"), 
     "Number of training iterations")
    // ("min-count", value<int>(&min_count)->default_value(5, "5"), 
    //  "This will discard words that appear less than the times")
    ("alpha", value<float>(&alpha)->default_value(0.1, "0.1"), 
     "starting learning rate")
    // ("batch-size", value<int>(&batch_size)->default_value(11, "11 (i.e., 2 * window + 1)"), 
    //  "The batch size used for mini-batch training")
    ("sync-period", value<float>(&model_sync_period)->default_value(0.1, "0.1"), 
     "Synchronize model every specified seconds")
    ("min-sync-words", value<int>(&min_sync_words)->default_value(1024, "1024"), 
     "Minimal number of words to be synced at each model sync")
    ("full-sync-times", value<int>(&full_sync_times)->default_value(0, "0"), 
     "Enforced full model sync-up times during training")
    ("message-size", value<int>(&message_size)->default_value(1024, "1024"), 
     "MPI message chunk size in MB")
  
    ("threads", value<int>(&num_threads)->default_value(0, "0"), 
     "Number of threads; '0' use as many as the number of cores.")     
    ;
  
  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().run(), argmap);
  if(argmap.count("help")){
    std::cerr << opt << std::endl;
    exit(1);
  }

  notify(argmap);

  // Alert
  int are_supposed_parameters =	   \
    hidden_size <= 512 &&	   \
    hidden_size % 2 == 0 &&	   \
    negative == 5 &&		   \
    window <= 8;
  if (!are_supposed_parameters) {
    LOG(INFO) << "===============  CAUTION  ===============" << std::endl
	      << "Optimized computation is not supported for the specified arguments so this program fallbacks to slower version." << std::endl
	      << "Recommended arguments to enable optimization are " << std::endl
	      << "    size <= 512 && " << std::endl	   
	      << "    size % 2 == 0 && " << std::endl	   
	      << "    negative == 5 && " <<	std::endl
	      << "    window <= 8" << std::endl
	      << "=========================================" << std::endl;

  }
 
  w2v::train_config config = {
    hidden_size, window, sample, negative, iter,
    alpha, model_sync_period, min_sync_words,
    full_sync_times, message_size, num_threads
  };
  
  w2v::train_each(
    encode_path, vocab_count_path, weight_path, config
  );

  return 0;
}
