
#include <iostream>
#include <frovedis.hpp>
#include <frovedis/ml/w2v/word2vec.hpp>
#include <boost/program_options.hpp>


using namespace frovedis;
using namespace boost::program_options;

int main(int argc, char* argv[]) {
  
  use_frovedis use(argc, argv);
  
  std::string weight_path, vocab_path, output_path;
  int  min_count;
  bool binary_output;
  
  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("weight,w", value<std::string>(&weight_path)->required(), "path for input binary file of trained weight matrix")
    ("vocab,v", value<std::string>(&vocab_path)->required(), "path for input binary file of vocabulary.")
    ("output,o", value<std::string>(&output_path)->required(), "path for output file of word vectors whose format is same to original.") 
    ("min-count", value<int>(&min_count)->default_value(5, "5"), "This will discard words that appear less than the times")
    ("binary", bool_switch(&binary_output)->default_value(false, "false"),
     "save output as binary")
    ;

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().run(), argmap);
  if(argmap.count("help")){
    std::cerr << opt << std::endl;
    exit(1);
  }

  notify(argmap);
  
  w2v::save_model(weight_path, vocab_path, output_path, min_count, binary_output);
  
  return 0;
}
