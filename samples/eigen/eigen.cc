#include <frovedis.hpp>
#include <frovedis/matrix/dense_eigen.hpp>
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
void do_dense_eigen(const string& input_matrix, const string& d_file,
                    const string& v_file, string mode, int k,
                    bool sym, bool binary) {
  time_spent t(DEBUG);
  rowmajor_matrix<double> matrix;
  if(binary) {
    matrix = make_rowmajor_matrix_loadbinary<double>(input_matrix);
  } else {
    matrix = make_rowmajor_matrix_load<double>(input_matrix);
  }
  t.show("load time: ");
  colmajor_matrix<double> v;
  colmajor_matrix<double> vi;
  diag_matrix_local<double> d;
  diag_matrix_local<double> di;
  time_spent t2(DEBUG), t3(DEBUG);
  if(sym) dense_eigen_sym<double>(matrix, d, v, mode, k);
  else dense_eigen<double>(matrix, d, di, v, vi, mode, k);
  t2.show("dense_eigen[_sym]: ");
  t3.show("total time w/o I/O: ");
  if(binary) {
    if(sym) {
      d.savebinary(d_file);
      v.to_rowmajor().savebinary(v_file);
    } else {
      d.savebinary(d_file + "_real");
      di.savebinary(d_file + "_imag");
      v.to_rowmajor().savebinary(v_file + "_real");
      vi.to_rowmajor().savebinary(v_file + "_imag");
    }
  } else {
    if(sym) {
      d.save(d_file);
      v.to_rowmajor().save(v_file);
    } else {
      d.save(d_file + "_real");
      di.save(d_file + "_imag");
      v.to_rowmajor().save(v_file + "_real");
      vi.to_rowmajor().save(v_file + "_imag");
    }
  }
  t2.show("save time: ");
}

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input,i", value<string>(), "input dense data matrix")
    ("d,d", value<string>(), "eigen values to save")
    ("v,v", value<string>(), "eigen vectors to save")
    ("k,k", value<int>(), "number of eigen values to compute")
    ("mode", value<string>(), "SM: from small, LM: from large, etc. [default: SM]")
    ("sym,s", "input is symmetric")
    ("nonsym,n", "input is not symmetric")
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
  bool sym = true;

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

  if(argmap.count("sym")){
    sym = true;
  }

  if(argmap.count("nonsym")){
    sym = false;
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

  do_dense_eigen(input, d, v, mode, k, sym, binary);
}
