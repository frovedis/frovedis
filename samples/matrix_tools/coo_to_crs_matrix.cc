#include <frovedis.hpp>
#include <frovedis/matrix/crs_matrix.hpp>

#include <boost/program_options.hpp>

using namespace std;
using namespace frovedis;

template <class T>
void coo_to_crs_matrix(string& input, string& output, bool zero_based,
                       bool binary) {
  auto m = make_crs_matrix_loadcoo<T>(input, zero_based);
  if(binary) m.savebinary(output);
  else m.save(output);
}

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input,i", value<string>(), "input coo text matrix file")
    ("output,o", value<string>(), "output crs matrix file")
    ("double", "assume double data")
    ("float", "assume float data")
    ("int", "assume int data")
    ("zero_based,z", "assume zero based index")
    ("one_based,n", "assume one based index [default]")
    ("binary,b", "use binary output");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  string input, output;
  bool binary = false;
  bool zero_based = false;
  
  if(argmap.count("help")){
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("input")){
    input = argmap["input"].as<string>();
  } else {
    cerr << "input coo matrix file is not specified" << endl;
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

  if(argmap.count("zero_based")){
    if(argmap.count("one_based")) {
      cerr << "both zero_based and one_based is specified" << endl;
      cerr << opt << endl;
      exit(1);
    } else
      zero_based = true;
  }

  if(argmap.count("one_based")){
    if(argmap.count("zero_based")) {
      cerr << "both zero_based and one_based is specified" << endl;
      cerr << opt << endl;
      exit(1);
    } else
      zero_based = false;
  }

  if(argmap.count("binary")){
    binary = true;
  }

  if(argmap.count("double")) {
    coo_to_crs_matrix<double>(input, output, zero_based, binary);
  } else if(argmap.count("float")) {
    coo_to_crs_matrix<float>(input, output, zero_based, binary);
  } else if(argmap.count("int")) {
    coo_to_crs_matrix<int>(input, output, zero_based, binary);
  } else {
    cerr << "data type is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

}
