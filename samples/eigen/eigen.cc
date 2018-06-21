#include <frovedis.hpp>
#include <frovedis/matrix/ccs_matrix.hpp>
#include <frovedis/matrix/colmajor_matrix.hpp>
#include <frovedis/matrix/diag_matrix.hpp>
#include <frovedis/matrix/shrink_sparse_eigen.hpp>
#include <frovedis/matrix/jds_crs_hybrid.hpp>

#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;
using namespace std;

/*
  input: m x n
  u: m x k (<-m)
  s: k(<-m) x k(<-n)
  vt: k(<-n) x n / v: n x k
 */
void do_sparse_eigen_sym(const string& input_matrix, const string& d_file,
                         const string& v_file, string mode, int k,
                         bool binary) {
  time_spent t(DEBUG);
  crs_matrix<double> matrix;
  if(binary) {
    matrix = make_crs_matrix_loadbinary<double>(input_matrix);
  } else {
    matrix = make_crs_matrix_load<double>(input_matrix);
  }
  t.show("load time: ");
  colmajor_matrix<double> v;
  diag_matrix_local<double> d;
  time_spent t2(DEBUG), t3(DEBUG);
#if defined(_SX) || defined(__ve__) 
  sparse_eigen_sym<jds_crs_hybrid<double>, jds_crs_hybrid_local<double>>
    (matrix, d, v, mode, k);
#else
  sparse_eigen_sym<double>(matrix, d, v, mode, k); 
#endif
  t2.show("sparse_eigen_sym: ");
  t3.show("total time w/o I/O: ");
  if(binary) {
    d.savebinary(d_file);
    v.to_rowmajor().savebinary(v_file);
  } else {
    d.save(d_file);
    v.to_rowmajor().save(v_file);
  }
  t2.show("save time: ");
}

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input,i", value<string>(), "input matrix")
    ("d,d", value<string>(), "eigen values")
    ("v,v", value<string>(), "eigen vectors")
    ("k,k", value<int>(), "number of eigen values to compute")
    ("mode", value<string>(), "SM: from small, LM: from large, etc. [default: SM]")
    ("verbose", "set loglevel DEBUG")
    ("verbose2", "set loglevel TRACE")
    ("binary,b", "use binary input/output");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  string input, d, v;
  int k;
  bool binary = false;

  string mode = "SM";
  
  if(argmap.count("help")){
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("input")){
    input = argmap["input"].as<string>();
  } else {
    cerr << "input is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("d")){
    d = argmap["d"].as<string>();
  } else {
    cerr << "file to store eigen value is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("v")){
    v = argmap["v"].as<string>();
  } else {
    cerr << "file to store eigen vector is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("mode")){
    mode = argmap["mode"].as<string>();
  } 

  if(argmap.count("k")){
    k = argmap["k"].as<int>();
  } else {
    cerr << "number of eigen values to compute is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("binary")){
    binary = true;
  }

  if(argmap.count("verbose")){
    set_loglevel(DEBUG);
  }

  if(argmap.count("verbose2")){
    set_loglevel(TRACE);
  }

  do_sparse_eigen_sym(input, d, v, mode, k, binary);
}
