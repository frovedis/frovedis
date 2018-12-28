
#include <iostream>

#include <frovedis.hpp>
#include <frovedis/ml/w2v/word2vec.hpp>
#include <boost/program_options.hpp>

using namespace frovedis;
using namespace boost::program_options;    


int main(int argc, char* argv[]) {

  use_frovedis use(argc, argv);

  std::string text_path, encode_path, vocab_path, vocab_count_path;
  int min_count;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input,i", value<std::string>(&text_path)->required(), "path for input raw text on which vocabulary is built.")
    ("encode,e", value<std::string>(&encode_path)->required(), "path for output file created by encoding input text.")
    ("vocab,v", value<std::string>(&vocab_path)->required(), "path for output binary file of vocabulary.")
    ("vocab-count,c", value<std::string>(&vocab_count_path)->required(), "path for output binary file of vocabulary counts.") 
    ("min-count", value<int>(&min_count)->default_value(5, "5"),
     "This will discard words that appear less than the times")
    ;

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().run(), argmap);
  if(argmap.count("help")){
    std::cerr << opt << std::endl;
    exit(1);
  }

  notify(argmap);

  w2v::build_vocab_and_dump(
    text_path, encode_path, vocab_path, vocab_count_path, min_count
  );
  
  return 0;
}
