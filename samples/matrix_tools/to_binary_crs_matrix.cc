#include <frovedis.hpp>
#include <frovedis/matrix/crs_matrix.hpp>

#include <boost/program_options.hpp>

using namespace std;
using namespace frovedis;

template <class T>
void to_binary_crs_matrix(string& input, string& output) {
  make_crs_matrix_load<T>(input).savebinary(output);
}

template <class T>
void to_binary_crs_matrix(string& input, string& output, size_t num_col) {
  make_crs_matrix_load<T>(input, num_col).savebinary(output);
}

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input,i", value<string>(), "input text matrix file")
    ("output,o", value<string>(), "output binary matrix directory")
    ("num_col,c", value<size_t>(), "number of colums. if not provided, max index is used")
    ("double", "assume double data")
    ("float", "assume float data")
    ("int", "assume int data");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  string input, output;
  
  if(argmap.count("help")){
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("input")){
    input = argmap["input"].as<string>();
  } else {
    cerr << "input matrix file is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("output")){
    output = argmap["output"].as<string>();
  } else {
    cerr << "output matrix directory is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("num_col")){
    size_t num_col = argmap["num_col"].as<size_t>();
    if(argmap.count("double")) {
      to_binary_crs_matrix<double>(input, output, num_col);
    } else if(argmap.count("float")) {
      to_binary_crs_matrix<float>(input, output, num_col);
    } else if(argmap.count("int")) {
      to_binary_crs_matrix<int>(input, output, num_col);
    } else {
      cerr << "data type is not specified" << endl;
      cerr << opt << endl;
      exit(1);
    }
  } else {
    if(argmap.count("double")) {
      to_binary_crs_matrix<double>(input, output);
    } else if(argmap.count("float")) {
      to_binary_crs_matrix<float>(input, output);
    } else if(argmap.count("int")) {
      to_binary_crs_matrix<int>(input, output);
    } else {
      cerr << "data type is not specified" << endl;
      cerr << opt << endl;
      exit(1);
    }
  }

}
