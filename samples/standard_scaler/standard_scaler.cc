#include <boost/program_options.hpp>
#include <frovedis.hpp>
#include <frovedis/matrix/standard_scaler.hpp>

using namespace boost;
using namespace frovedis;
using namespace std;

template <class T>
void do_scale(const string& input, const string& output, 
              bool with_mean, bool with_std, bool sample_stddev, 
              bool binary) {
  rowmajor_matrix<T> mat;
  time_spent t(DEBUG);  
  if (binary) {
    mat = make_rowmajor_matrix_loadbinary<T>(input);
  } else {
    mat = make_rowmajor_matrix_load<T>(input);
  }
  t.show("load matrix: ");
  auto ss = frovedis::standard_scaler<T>(with_mean, with_std, sample_stddev);
  ss.fit(mat);
  t.show("fit matrix: ");  
  auto scaled_matrix = ss.transform(mat);
  t.show("transform matrix: ");  
  binary ? scaled_matrix.savebinary(output) : scaled_matrix.save(output);
  t.show("save scaled matrix: ");  
}

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  options_description opt("option");
  // clang-format off
  opt.add_options()("help,h", "print help")
    ("input,i", value<string>(),"input matrix")
    ("output,o", value<string>(), "output scaled matrix")
    ("double","for double type input (default)")      
    ("binary,b", "use binary input/output");
  // clang-format on
  variables_map argmap;
  store(command_line_parser(argc, argv).options(opt).allow_unregistered().run(),
        argmap);
  notify(argmap);

  string input, output;
  bool with_mean = true;
  bool with_std = true;
  bool sample_stddev = false;
  bool binary = false;

  if (argmap.count("help")) {
    cerr << opt << endl;
    exit(1);
  }

  if (argmap.count("input")) {
    input = argmap["input"].as<string>();
  } else {
    cerr << "input is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if (argmap.count("output")) {
    output = argmap["output"].as<string>();
  } else {
    cerr << "output is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("double"))
    do_scale<double>(input, output, with_mean, with_std, sample_stddev, binary);
  else 
    do_scale<float>(input, output, with_mean, with_std, sample_stddev, binary);
}
