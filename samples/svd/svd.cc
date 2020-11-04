#include <frovedis.hpp>
#include <frovedis/matrix/jds_crs_hybrid.hpp>
#include <frovedis/matrix/sparse_svd.hpp>

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
void do_sparse_svd(const string& input_matrix, const string& u_file,
                   const string& s_file, const string& v_file, int k,
                   bool binary) {
  time_spent t(DEBUG);
  crs_matrix<double> matrix;
  if(binary) {
    matrix = make_crs_matrix_loadbinary<double>(input_matrix);
  } else {
    matrix = make_crs_matrix_load<double>(input_matrix);
  }
  t.show("load time: ");
  colmajor_matrix<double> u, v;
  diag_matrix_local<double> s;
  time_spent t2(DEBUG), t3(DEBUG);
#if defined(_SX) || defined(__ve__)
  sparse_svd<jds_crs_hybrid<double>, jds_crs_hybrid_local<double>>
    (std::move(matrix), u, s, v, k);
  t2.show("sparse_svd: ");
#else
  sparse_svd<double>(matrix, u, s, v, k); 
  t2.show("sparse_svd: ");
#endif
  t3.show("total time w/o I/O: ");
  if(binary) {
    u.to_rowmajor().savebinary(u_file);
    s.savebinary(s_file);
    v.to_rowmajor().savebinary(v_file);
  } else {
    u.to_rowmajor().save(u_file);
    s.save(s_file);
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
    ("input,i", value<string>(), "input sparse data matrix")
    ("u,u", value<string>(), "left singular vectors to save")
    ("s,s", value<string>(), "singular values to save")
    ("v,v", value<string>(), "right singular vectors to save")
    ("k,k", value<int>(), "number of singular values to compute")
    ("verbose", "set loglevel DEBUG")
    ("verbose2", "set loglevel TRACE")
    ("binary,b", "use binary input/output");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  string input, s, u, v;
  int k;
  bool binary = false;
  
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

  if(argmap.count("s")){
    s = argmap["s"].as<string>();
  } else {
    cerr << "file to store singular value is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("v")){
    v = argmap["v"].as<string>();
  } else {
    cerr << "file to store left singular vector is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("u")){
    u = argmap["u"].as<string>();
  } else {
    cerr << "file to store right singular vector is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("k")){
    k = argmap["k"].as<int>();
  } else {
    cerr << "number of singular value to compute is not specified" << endl;
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

  do_sparse_svd(input, u, s, v, k, binary);
}
