#include <frovedis.hpp>
#include <fstream>
#include <iostream>
#include <frovedis/ml/clustering/dbscan.hpp>

#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;
using namespace std;

template <class T>
void do_dbscan(const string& input, const string& output,
               double eps, int min_pts, bool binary) {
  if (binary) {
    time_spent t(DEBUG);
    auto m = frovedis::make_rowmajor_matrix_loadbinary<T>(input);
    t.show("load matrix data: ");
    auto est = frovedis::dbscan<T>(eps, min_pts);
    est.fit(m);
    t.show("total excution time: ");
    auto labels = est.labels();
    make_dvector_scatter(labels).savebinary(output);
    t.show("save labels data: ");
  } else {
    time_spent t(DEBUG);
    auto m = frovedis::make_rowmajor_matrix_load<T>(input);
    t.show("load matrix data: ");
    auto est = frovedis::dbscan<T>(eps, min_pts);
    est.fit(m);
    t.show("total excution time: ");
    auto labels = est.labels();
    make_dvector_scatter(labels).saveline(output);
    t.show("save labels data: ");
  }
}

int main(int argc, char* argv[]) {
  frovedis::use_frovedis use(argc, argv);
  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input,i", value<string>(), "input matrix")
    ("output,o", value<string>(), "output")
    ("eps,e", value<double>(), "distance that can be determined to be reachable [default: 0.5]")
    ("min_pts,m", value<int>(), "minimum number of core [default: 5]")
    ("float,f", "for float type input")
    ("double,d","for double type input(default input type is double)")
    ("verbose", "set loglevel to DEBUG")
    ("binary,b", "use binary input/output");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  string input, output;
  double eps = 0.5;
  int min_pts = 5;
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

  if(argmap.count("output")){
    output = argmap["output"].as<string>();
  } else {
    cerr << "output is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("eps")){
    eps = argmap["eps"].as<double>();
  } 

  if(argmap.count("min_pts")){
    min_pts = argmap["min_pts"].as<int>();
  } 

  if(argmap.count("binary")){
    binary = true;
  }

  if(argmap.count("verbose")){
    set_loglevel(DEBUG);
  }

  if(argmap.count("float")) 
    do_dbscan<float>(input, output, eps, min_pts, binary);
  else
    do_dbscan<double>(input, output, eps, min_pts, binary);
}
