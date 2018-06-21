#include <frovedis.hpp>
#include <frovedis/matrix/crs_matrix.hpp>

#include <boost/program_options.hpp>

using namespace std;
using namespace frovedis;

template <class T>
void transpose_crs_matrix(string& input, string& output, bool binary) {
  if(binary) {
    auto m = make_crs_matrix_loadbinary<T>(input);
    m.transpose().savebinary(output);
  }
  else {
    auto m = make_crs_matrix_load<T>(input);
    m.transpose().save(output);
  }
}

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input,i", value<string>(), "input crs matrix directory")
    ("output,o", value<string>(), "output matrix file")
    ("double", "assume double data")
    ("float", "assume float data")
    ("int", "assume int data")
    ("binary,b", "binary input/putput");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  string input, output;
  bool binary  = false;
  
  if(argmap.count("help")){
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("input")){
    input = argmap["input"].as<string>();
  } else {
    cerr << "input matrix directory is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("output")){
    output = argmap["output"].as<string>();
  } else {
    cerr << "output matrix file is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("binary")){
    binary = true;
  }

  if(argmap.count("double")) {
    transpose_crs_matrix<double>(input, output, binary);
  } else if(argmap.count("float")) {
    transpose_crs_matrix<float>(input, output, binary);
  } else if(argmap.count("int")) {
    transpose_crs_matrix<int>(input, output, binary);
  } else {
    cerr << "data type is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

}
