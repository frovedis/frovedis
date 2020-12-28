#include <frovedis.hpp>
#include <frovedis/ml/clustering/kmeans.hpp>
#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;
using namespace std;

void do_kmeans(const string& input, bool dense, 
               const string& output, int k,
               int num_iteration, double eps, long seed, 
               bool binary, bool shrink) {

  time_spent t(DEBUG);
  auto km = KMeans<double>(k).set_max_iter(num_iteration)
                             .set_eps(eps)
                             .set_seed(seed)
                             .set_use_shrink(shrink);
  if(binary) {
    if(dense) {
      auto mat = make_rowmajor_matrix_loadbinary<double>(input);
      t.show("load matrix: ");
      km.fit(mat);
      t.show("kmeans time: ");
    } else {
      auto mat = make_crs_matrix_loadbinary<double>(input);
      t.show("load matrix: ");
      km.fit(mat);
      t.show("kmeans time: ");
    }
    km.cluster_centers_().transpose().savebinary(output + "_centroid");
    make_dvector_scatter(km.labels_()).savebinary(output + "_labels");
    t.show("save result time: ");
  } else {
    if(dense) {
      auto mat = make_rowmajor_matrix_load<double>(input);
      t.show("load matrix: ");
      km.fit(mat);
      t.show("kmeans time: ");
    } else {
      time_spent t(DEBUG);
      auto mat = make_crs_matrix_load<double>(input);
      t.show("load matrix: ");
      km.fit(mat);
      t.show("kmeans time: ");
    }
    km.cluster_centers_().transpose().save(output + "_centroid");
    make_dvector_scatter(km.labels_()).saveline(output + "_labels");
    t.show("save result time: ");
  }
}

void do_assign(const string& input, bool dense, 
               const string& input_centroid,
               const string& output, bool binary) {
  double score = 0.0;
  if(binary) {
    if(dense) {
      time_spent t(DEBUG);
      auto mat = make_rowmajor_matrix_local_loadbinary<double>(input);
      t.show("load matrix: ");
      auto c = make_rowmajor_matrix_local_loadbinary<double>(input_centroid);
      t.show("load centroid: ");
      auto ct = c.transpose();
      auto r = kmeans_assign_cluster(mat, ct, score);
      t.show("assign time: ");
      make_dvector_scatter(r).savebinary(output);    
      t.show("prediction save time: ");
    } else {
      time_spent t(DEBUG);
      auto mat = make_crs_matrix_local_loadbinary<double>(input);
      t.show("load matrix: ");
      auto c = make_rowmajor_matrix_local_loadbinary<double>(input_centroid);
      t.show("load centroid: ");
      auto ct = c.transpose();
      auto r = kmeans_assign_cluster(mat, ct, score);
      t.show("assign time: ");
      make_dvector_scatter(r).savebinary(output);    
      t.show("prediction save time: ");
    }
  } else {
    if(dense) {
      time_spent t(DEBUG);
      auto mat = make_rowmajor_matrix_local_load<double>(input);
      t.show("load matrix: ");
      auto c = make_rowmajor_matrix_local_load<double>(input_centroid);
      t.show("load centroid: ");
      auto ct = c.transpose();
      auto r = kmeans_assign_cluster(mat, ct, score);
      t.show("assign time: ");
      make_dvector_scatter(r).saveline(output);    
      t.show("prediction save time: ");
    } else {
      time_spent t(DEBUG);
      auto mat = make_crs_matrix_local_load<double>(input);
      t.show("load matrix: ");
      auto c = make_rowmajor_matrix_local_load<double>(input_centroid);
      t.show("load centroid: ");
      auto ct = c.transpose();
      auto r = kmeans_assign_cluster(mat, ct, score);
      t.show("assign time: ");
      make_dvector_scatter(r).saveline(output);    
      t.show("prediction save time: ");
    }
  }
  std::cout << "prediction score: " << score << std::endl;
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
    ("seed,r", value<long>(), "seed for init randomizer")
    ("sparse,s", "use sparse matrix [default]")
    ("dense,d", "use dense matrix")
    ("use-shrink", "whether to use shrinking for sparse data [default: false]")
    ("verbose", "set loglevel to DEBUG")
    ("verbose2", "set loglevel to TRACE")
    ("binary,b", "use binary input/output");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  string input, output, input_centroid;
  int k = 0;
  int num_iteration = 100;
  double eps = 0.01;
  long seed = 0;
  bool shrink = false;
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

  if(argmap.count("seed")){
    seed = argmap["seed"].as<long>();
  }

  if(argmap.count("use-shrink")){
    shrink = true;
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
  else do_kmeans(input, dense, output, k, num_iteration, eps, seed, binary, shrink);
}
