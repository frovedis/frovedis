#include <frovedis.hpp>
#include <fstream>
#include <iostream>
#include <frovedis/ml/clustering/dbscan.hpp>

#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;
using namespace std;

void do_dbscan(const string& input, const string& output,
               double eps, int min_pts, bool binary) {
  if (binary) {
    time_spent t(DEBUG);
    auto m = frovedis::make_rowmajor_matrix_loadbinary<double>(input);
    t.show("load matrix data: ");
    auto dbscan = frovedis::dbscan(eps, min_pts);
    dbscan.fit(m);
    t.show("total excution time: ");
    auto labels = dbscan.labels();
    make_dvector_scatter(labels).savebinary(output);
    t.show("save labels data: ");
  } else {
    time_spent t(DEBUG);
    auto m = frovedis::make_rowmajor_matrix_load<double>(input);
    t.show("load matrix data: ");
    auto dbscan = frovedis::dbscan(eps, min_pts);
    dbscan.fit(m);
    t.show("total excution time: ");
    auto labels = dbscan.labels();
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
    ("eps,e", value<double>(), "distance that can be determined to be reachable")
    ("min_pts,m", value<int>(), "minimum number of core")
    ("verbose", "set loglevel to DEBUG")
    ("binary,b", "use binary input/output");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  string input, output;
  double eps;
  int min_pts;
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
  } else {
    cerr << "eps is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("min_pts")){
    min_pts = argmap["min_pts"].as<int>();
  } else {
    cerr << "min_pts is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("binary")){
    binary = true;
  }

  if(argmap.count("verbose")){
    set_loglevel(DEBUG);
  }

  do_dbscan(input, output, eps, min_pts, binary);
}
