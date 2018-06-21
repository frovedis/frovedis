#include <frovedis.hpp>
#include <frovedis/matrix/crs_matrix.hpp>

#include <math.h>
#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;
using namespace std;

void normalize_row(crs_matrix_local<double>& mat) {
  crs_matrix_local<double> ret;
  for(size_t r = 0; r < mat.local_num_row; r++) {
    double sq = 0;
    for(size_t c = mat.off[r]; c < mat.off[r+1]; c++) {
      sq += mat.val[c] * mat.val[c];
    }
    double root = sqrt(sq);
    for(size_t c = mat.off[r]; c < mat.off[r+1]; c++) {
      mat.val[c] /= root;
    }
  }
}

void do_normalize_row(const string& input, const string& output, bool binary) {
  if(binary) {
    time_spent t(DEBUG);
    auto mat = make_crs_matrix_loadbinary<double>(input);
    t.show("load matrix: ");
    mat.data.mapv(normalize_row);
    t.show("normalize time: ");
    mat.savebinary(output);
    t.show("save: ");
  } else {
    time_spent t(DEBUG);
    auto mat = make_crs_matrix_load<double>(input);
    t.show("load matrix: ");
    mat.data.mapv(normalize_row);
    t.show("normalize time: ");
    mat.save(output);
    t.show("save: ");
  }
}

int main(int argc, char* argv[]){
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input,i", value<string>(), "input matrix file")
    ("output,o", value<string>(), "output matrix file")
    ("binary,b", "use binary input/output");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  string input, output;
  bool binary = false;
  
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
    cerr << "output matrix file is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("binary")){
    binary = true;
  }
  do_normalize_row(input, output, binary);
}
