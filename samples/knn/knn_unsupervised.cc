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
    ("chunk_size,s", value<float>(), "chunk size in MB for controlling runtime memory requirement (default: 1.0)");
    
  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  std::string input, output;
  int k = 2;
  float radius = 1.0;
  float chunk_size = 1.0;
  std::string metric = "euclidean";
  std::string algorithm = "brute";
  bool need_distance = true;
  
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
  auto data = make_rowmajor_matrix_load<double>(input);
  
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
  
  if(argmap.count("chunk_size")){
    chunk_size = argmap["chunk_size"].as<float>();
  }

  if(argmap.count("radius")){
    radius = argmap["radius"].as<float>();
  }

  std::string mode = need_distance ? "distance" : "connectivity";

  time_spent calc_knn(INFO);
  calc_knn.lap_start();
  nearest_neighbors<double, rowmajor_matrix<double>> obj(k, radius, algorithm, metric, chunk_size);
  obj.fit(data);
  auto knn_graph = obj.kneighbors_graph(data, k, mode);
  calc_knn.lap_stop();
  calc_knn.show_lap("total knn + graph creation time: ");
 
  knn_graph.save(output + "/knn_graph");

  auto radius_graph = obj.radius_neighbors_graph(data, radius, mode);
  radius_graph.save(output + "/radius_graph");

  return 0;
}

