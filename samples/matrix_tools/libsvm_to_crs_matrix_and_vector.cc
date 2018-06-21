#include <frovedis.hpp>
#include <frovedis/matrix/crs_matrix.hpp>

#include <boost/program_options.hpp>

using namespace std;
using namespace frovedis;


template <class T>
void libsvm_to(string& input, string& output_matrix, string& output_label,
               bool binary) {
  dvector<T> lb;
  auto mat = make_crs_matrix_loadlibsvm<T>(input, lb);
  if (binary) {
    mat.savebinary(output_matrix);
    lb.savebinary(output_label);
  } else {
    mat.save(output_matrix);
    lb.saveline(output_label);
  }
}

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  string input, output_matrix, output_label;
  bool binary;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input,i", value<string>(&input)->required(), "input libsvm format (1-based) file")
    ("output-matrix,m", value<string>(&output_matrix)->required(), "output crs matrix file")
    ("output-label,l", value<string>(&output_label)->required(), "output label file")
    ("double", "assume double data")
    ("float", "assume float data")
    ("int", "assume int data")
    ("binary,b", bool_switch(&binary)->default_value(false), "use binary output")
    ;

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);

  if(argmap.count("help")){
    cerr << opt << endl;
    exit(1);
  }

  notify(argmap);

  if(argmap.count("double")) {
    libsvm_to<double>(input, output_matrix, output_label, binary);
  } else if(argmap.count("float")) {
    libsvm_to<float>(input, output_matrix, output_label, binary);
  } else if(argmap.count("int")) {
    libsvm_to<int>(input, output_matrix, output_label, binary);
  } else {
    cerr << "data type is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

}
