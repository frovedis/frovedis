#include <frovedis.hpp>
#include <frovedis/ml/neighbors/knn_unsupervised.hpp>
#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;

int main(int argc, char** argv) {
  use_frovedis use(argc,argv);
  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input,i", value<std::string>(), "path for input dense matrix")
    ("output,o", value<std::string>(), "path for output graphs")
    ("neighbors,k", value<int>(), "number of required neighbors (default: 2)")
    ("metric,m", value<std::string>(), "metric for distance calculation (default: euclidean)")
    ("algorithm,a", value<std::string>(), "algorithm for knn (default: brute)")
    ("radius,r", value<float>(), "radius for finding nearest neighbors (default: 1.0)")
    ("need_distance,d", value<int>(), "need distance as output (1/0) (default: 1)")
    ("batch_fraction,s", value<double>(), "batch fraction (0 to 1) for processing large data (default: 1 if small data, else decided based on some heuristic)")
    ("sparse,s", "use sparse matrix")
    ("dense,d", "use dense matrix (default) ")
    ("verbose", "set loglevel to DEBUG")
    ("verbose2", "set loglevel to TRACE");
    
  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  std::string input, output;
  int k = 2;
  float radius = 1.0;
  float chunk_size = 1.0; // not much meaningful when batch-processing will take place
  double batch_fraction = std::numeric_limits<double>::max();
  std::string metric = "euclidean";
  std::string algorithm = "brute";
  bool need_distance = true;
  bool dense = true;
  
  if(argmap.count("help")){
    std::cerr << opt << std::endl;
    exit(1);
  }

  if(argmap.count("input")){
    input = argmap["input"].as<std::string>();
  } else {
    std::cerr << "input path is not specified" << std::endl;
    std::cerr << opt << std::endl;
    exit(1);
  }
  
  if(argmap.count("output")){
    output = argmap["output"].as<std::string>();
  } else {
    std::cerr << "output path is not specified" << std::endl;
    std::cerr << opt << std::endl;
    exit(1);
  }

  if(argmap.count("neighbors")){
    k = argmap["neighbors"].as<int>();
  }
  
  if(argmap.count("metric")){
    metric = argmap["metric"].as<std::string>();
  }

  if(argmap.count("algorithm")){
    algorithm = argmap["algorithm"].as<std::string>();
  }

  if(argmap.count("need_distance")){
    need_distance = argmap["need_distance"].as<int>() == 1;
  }
  
  if(argmap.count("batch_fraction")){
    batch_fraction = argmap["batch_fraction"].as<double>();
  }

  if(argmap.count("radius")){
    radius = argmap["radius"].as<float>();
  }

  std::string mode = need_distance ? "distance" : "connectivity";

  if(argmap.count("sparse")){
    dense = false;
  }

  if(argmap.count("dense")){
    dense = true;
  }

  if(argmap.count("verbose")){
    set_loglevel(DEBUG);
  }

  if(argmap.count("verbose2")){
    set_loglevel(TRACE);
  }

  time_spent calc_knn(INFO);
  try {
    if(dense) {
      auto data = make_rowmajor_matrix_load<double>(input);
      calc_knn.lap_start();
      nearest_neighbors<double, rowmajor_matrix<double>> obj(k, radius, algorithm, metric, 
                                                             chunk_size, batch_fraction);
      obj.fit(data);
      auto knn_graph = obj.kneighbors_graph(data, k, mode);
      calc_knn.lap_stop();
      calc_knn.show_lap("total knn + graph creation time: ");
      knn_graph.save(output + "/knn_graph");
      auto radius_graph = obj.radius_neighbors_graph(data, radius, mode);
      radius_graph.save(output + "/radius_graph");
    } else {
      auto data = make_crs_matrix_load<double>(input);
      calc_knn.lap_start();
      nearest_neighbors<double, crs_matrix<double>> obj(k, radius, algorithm, metric, 
                                                        chunk_size, batch_fraction);
      obj.fit(data);
      auto knn_graph = obj.kneighbors_graph(data, k, mode);
      calc_knn.lap_stop();
      calc_knn.show_lap("total knn + graph creation time: ");
      knn_graph.save(output + "/knn_graph");
      auto radius_graph = obj.radius_neighbors_graph(data, radius, mode);
      radius_graph.save(output + "/radius_graph");
    }
  }
  catch(std::exception& e) {
    std::cout << "exception caught: " << e.what() << std::endl;
  }

  return 0;
}

