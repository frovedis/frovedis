#include <frovedis.hpp>
#include <frovedis/ml/clustering/shrink_kmeans.hpp>

#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;
using namespace std;

void do_kmeans(const string& input, bool dense, const string& output, int k,
               int num_iteration, double eps, bool binary) {
  if(binary) {
    if(dense) {
      time_spent t(DEBUG);
      auto mat = make_rowmajor_matrix_loadbinary<double>(input);
      t.show("load matrix: ");
      auto r = shrink::kmeans(mat, k, num_iteration, eps);
      t.show("kmeans time: ");
      r.transpose().savebinary(output);
      t.show("save centroid time: ");
    } else {
      time_spent t(DEBUG);
      auto mat = make_crs_matrix_loadbinary<double>(input);
      t.show("load matrix: ");
      auto r = shrink::kmeans(std::move(mat), k, num_iteration, eps);
      t.show("kmeans time: ");
      r.transpose().savebinary(output);
      t.show("save centroid time: ");
    }
  } else {
    if(dense) {
      time_spent t(DEBUG);
      auto mat = make_rowmajor_matrix_load<double>(input);
      t.show("load matrix: ");
      auto r = shrink::kmeans(mat, k, num_iteration, eps);
      t.show("kmeans time: ");
      r.transpose().save(output);
      t.show("save model time: ");
    } else {
      time_spent t(DEBUG);
      auto mat = make_crs_matrix_load<double>(input);
      t.show("load matrix: ");
      auto r = shrink::kmeans(std::move(mat), k, num_iteration, eps);
      t.show("kmeans time: ");
      r.transpose().save(output);
      t.show("save model time: ");
    }
  }
}

void do_assign(const string& input, bool dense, const string& input_centroid,
               const string& output, bool binary) {
  if(binary) {
    if(dense) {
      time_spent t(DEBUG);
      auto mat = make_rowmajor_matrix_local_loadbinary<double>(input);
      t.show("load matrix: ");
      auto c = make_rowmajor_matrix_local_loadbinary<double>(input_centroid);
      t.show("load centroid: ");
      auto ct = c.transpose();
      auto r = kmeans_assign_cluster(mat, ct);
      make_dvector_scatter(r).savebinary(output);    
    } else {
      time_spent t(DEBUG);
      auto mat = make_crs_matrix_local_loadbinary<double>(input);
      t.show("load matrix: ");
      auto c = make_rowmajor_matrix_local_loadbinary<double>(input_centroid);
      t.show("load centroid: ");
      auto ct = c.transpose();
      auto r = kmeans_assign_cluster(mat, ct);
      make_dvector_scatter(r).savebinary(output);    
    }
  } else {
    if(dense) {
      time_spent t(DEBUG);
      auto mat = make_rowmajor_matrix_local_load<double>(input);
      t.show("load matrix: ");
      auto c = make_rowmajor_matrix_local_load<double>(input_centroid);
      t.show("load centroid: ");
      auto ct = c.transpose();
      auto r = kmeans_assign_cluster(mat, ct);
      make_dvector_scatter(r).saveline(output);    
    } else {
      time_spent t(DEBUG);
      auto mat = make_crs_matrix_local_load<double>(input);
      t.show("load matrix: ");
      auto c = make_rowmajor_matrix_local_load<double>(input_centroid);
      t.show("load centroid: ");
      auto ct = c.transpose();
      auto r = kmeans_assign_cluster(mat, ct);
      make_dvector_scatter(r).saveline(output);    
    }
  }
}

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("assign,a", "assign data to cluster mode")
    ("input,i", value<string>(), "input matrix")
    ("centroid,c", value<string>(), "input centroid for assignment")
    ("output,o", value<string>(), "output centroids or cluster")
    ("k,k", value<int>(), "number of clusters")
    ("num-iteration,n", value<int>(), "number of max iteration")
    ("eps,e", value<double>(), "epsilon to stop the iteration")
    ("sparse,s", "use sparse matrix [default]")
    ("dense,d", "use dense matrix")
    ("verbose", "set loglevel to DEBUG")
    ("verbose2", "set loglevel to TRACE")
    ("binary,b", "use binary input/output");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  string input, output, input_centroid;
  int num_iteration = 100;
  int k;
  double eps = 0.01;
  bool assign = false;
  bool dense = false;
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

  if(argmap.count("assign")){
    assign = true;
  }

  if(argmap.count("centroid")){
    input_centroid = argmap["centroid"].as<string>();
  } else {
    if(assign == true) {
      cerr << "output is not specified" << endl;
      cerr << opt << endl;
      exit(1);
    }
  }

  if(argmap.count("k")){
    k = argmap["k"].as<int>();
  } else {
    if(assign == false) {
      cerr << "number of cluster is not specified" << endl;
      cerr << opt << endl;
      exit(1);
    }
  }

  if(argmap.count("num-iteration")){
    num_iteration = argmap["num-iteration"].as<int>();
  }

  if(argmap.count("epsilon")){
    eps = argmap["epsilon"].as<double>();
  }

  if(argmap.count("sparse")){
    dense = false;
  }

  if(argmap.count("dense")){
    dense = true;
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

  if(assign) do_assign(input, dense, input_centroid, output, binary);
  else do_kmeans(input, dense, output, k, num_iteration, eps, binary);
}
