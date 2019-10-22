// very simple converter; does not work all the mtx files!
// create "./tmp.coo" as intermediate file (COO format)
#include <frovedis.hpp>
#include <frovedis/matrix/crs_matrix.hpp>

#include <boost/program_options.hpp>

using namespace std;
using namespace frovedis;

template <class T>
void mtx_to_crs(string& input, string& output_matrix, bool binary) {
  std::ifstream ifs(input.c_str());
  std::ofstream ofs("./tmp.coo");
  std::string line;
  bool is_symmetric = false;
  bool is_pattern = false;
  std::getline(ifs,line);
  if(!(line.find("symmetric") == string::npos)) is_symmetric = true;
  if(!(line.find("pattern") == string::npos)) is_pattern = true;
  while(std::getline(ifs,line)) {
    if(line[0] == '%') continue;
    else break;
  }
  size_t num_row, num_col, nnz;
  istringstream iss(line);
  iss >> num_row >> num_col >> nnz;
  while(std::getline(ifs,line)) {
    if(is_pattern) {
      ofs << line << " " << 1 << endl;
      if(is_symmetric) {
        istringstream iss2(line);
        size_t row, col;
        iss2 >> row >> col;
        if(row != col) ofs << col << " " << row << " " << 1 << endl;
      }
    } else {
      ofs << line << endl;
      if(is_symmetric) {
        istringstream iss2(line);
        size_t row, col;
        string val; // for arbitrary precision
        iss2 >> row >> col >> val;
        if(row != col) ofs << col << " " << row << " " << val << endl;
      }
    }
  }
  auto crs = make_crs_matrix_loadcoo<T>("./tmp.coo");
  crs.data.mapv(+[](crs_matrix_local<T>& m, size_t num_col){m.local_num_col = num_col;},
                broadcast(num_col));
  crs.num_col = num_col;
  if(binary) crs.savebinary(output_matrix);
  else {
    crs.save(output_matrix);
  }
}

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  string input, output_matrix;
  bool binary;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input,i", value<string>(&input)->required(), "input libsvm format (1-based) file")
    ("output-matrix,o", value<string>(&output_matrix)->required(), "output crs matrix file")
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
    mtx_to_crs<double>(input, output_matrix, binary);
  } else if(argmap.count("float")) {
    mtx_to_crs<float>(input, output_matrix, binary);
  } else if(argmap.count("int")) {
    mtx_to_crs<int>(input, output_matrix, binary);
  } else {
    cerr << "data type is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

}
