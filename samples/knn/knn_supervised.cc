#include <frovedis.hpp>
#include <frovedis/ml/neighbors/knn_supervised.hpp>
#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;

template <class T>
void pretty_print(rowmajor_matrix_local<T>& mat) {
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  for(size_t i = 0; i < nrow; ++i) {
    for(size_t j = 0; j < ncol; ++j) {
      std::cout << mat.val[i * ncol + j] << " ";
    }
    std::cout << std::endl;
  }
}

int main(int argc, char** argv) {
  use_frovedis use(argc,argv);
  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input,i", value<std::string>(), "path for input dense feature data")
    ("label,l", value<std::string>(), "path for input label")
    ("target,t", value<std::string>(), "target: classification or regression")
    ("neighbors,k", value<int>(), "number of required neighbors (default: 2)")
    ("metric,m", value<std::string>(), "metric for distance calculation (default: euclidean)")
    ("algorithm,a", value<std::string>(), "algorithm for knn (default: brute)")
    ("batch_fraction,s", value<double>(), "batch fraction (0 to 1) for processing large data (default: 1 if small data, else decided based on some heuristic)")
    ("save_proba,p", value<int>(), "whether to save probability_matrix (1/0) (default: 0)")
    ("sparse,s", "use sparse matrix")
    ("dense,d", "use dense matrix (default)")
    ("verbose", "set loglevel to DEBUG")
    ("verbose2", "set loglevel to TRACE");
    
  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  std::string input, label, target;
  int k = 2;
  float chunk_size = 1.0; // not much meaningful when batch-processing will take place
  double batch_fraction = std::numeric_limits<double>::max();
  std::string metric = "euclidean";
  std::string algorithm = "brute";
  bool save_proba = false;
  bool dense = true;
  
  if(argmap.count("help")){
    std::cerr << opt << std::endl;
    exit(1);
  }

  if(argmap.count("input")){
    input = argmap["input"].as<std::string>();
  } else {
    std::cerr << "input feature data path is not specified!" << std::endl;
    std::cerr << opt << std::endl;
    exit(1);
  }
  
  if(argmap.count("label")){
    label = argmap["label"].as<std::string>();
  } else {
    std::cerr << "input label data path is not specified!" << std::endl;
    std::cerr << opt << std::endl;
    exit(1);
  }
  auto lbl = make_dvector_loadline<double>(label);
  
  if(argmap.count("target")){
    target = argmap["target"].as<std::string>();
  } else {
    std::cerr << "target: classification/regression is not specified!" << std::endl;
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

  if(argmap.count("save_proba")){
    save_proba = argmap["save_proba"].as<int>() == 1;
  }

  if(argmap.count("batch_fraction")){
    batch_fraction = argmap["batch_fraction"].as<double>();
  }

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
    if (dense) {
      auto data = make_rowmajor_matrix_load<double>(input);
      if (target == "classification") {
        calc_knn.lap_start();
        kneighbors_classifier<double, rowmajor_matrix<double>> obj(k, algorithm, metric, 
                                                                   chunk_size, batch_fraction);
        obj.fit(data, lbl);
        auto pred = obj.predict(data, save_proba);
        calc_knn.lap_stop();
        calc_knn.show_lap("total knn fit-predict time: ");
        std::cout << "prediction: "; debug_print_vector(pred.gather(), 10);   
        auto score = accuracy_score(lbl.gather(), pred.gather());
        std::cout << "score: " << score << std::endl;
      } else if (target == "regression") {
        calc_knn.lap_start();
        kneighbors_regressor<double, rowmajor_matrix<double>> obj(k, algorithm, metric, 
                                                                  chunk_size, batch_fraction);
        obj.fit(data, lbl);
        auto pred = obj.predict(data);
        calc_knn.lap_stop();
        calc_knn.show_lap("total knn fit-predict time: ");
        std::cout << "prediction: "; debug_print_vector(pred.gather(), 10);    
        auto score = r2_score(lbl.gather(), pred.gather()) ;
        std::cout << "score: " << score << std::endl;
      } else {
        std::cerr << "Unknown target: " << target << " for Supervised KNN!\n";
        std::cerr << opt << std::endl;
        exit(1);
      }
    } else {
      auto data = make_crs_matrix_load<double>(input);
      if (target == "classification") {
        calc_knn.lap_start();
        kneighbors_classifier<double, crs_matrix<double>> obj(k, algorithm, metric,
                                                              chunk_size, batch_fraction);
        obj.fit(data, lbl);
        auto pred = obj.predict(data, save_proba);
        calc_knn.lap_stop();
        calc_knn.show_lap("total knn fit-predict time: ");
        std::cout << "prediction: "; debug_print_vector(pred.gather(), 10);
        auto score = accuracy_score(lbl.gather(), pred.gather());
        std::cout << "score: " << score << std::endl;
      } else if (target == "regression") {
        calc_knn.lap_start();
        kneighbors_regressor<double, crs_matrix<double>> obj(k, algorithm, metric,
                                                             chunk_size, batch_fraction);
        obj.fit(data, lbl);
        auto pred = obj.predict(data);
        calc_knn.lap_stop();
        calc_knn.show_lap("total knn fit-predict time: ");
        std::cout << "prediction: "; debug_print_vector(pred.gather(), 10);
        auto score = r2_score(lbl.gather(), pred.gather());
        std::cout << "score: " << score << std::endl;
      } else {
        std::cerr << "Unknown target: " << target << " for Supervised KNN!\n";
        std::cerr << opt << std::endl;
        exit(1);
      }
    }
  }
  catch(std::exception& e) {
    std::cout << "exception caught: " << e.what() << std::endl;
  }

  return 0;
}

