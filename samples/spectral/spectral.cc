#include <frovedis.hpp>
#include <frovedis/ml/clustering/spectral_clustering.hpp>
#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;

int main(int argc, char** argv) {
  use_frovedis use(argc,argv);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
      ("help,h", "produce help message")
      ("input,i" , value<std::string>(), "input rowmajor matrix data.")
      ("n-clusters" , value<int>(), "the dimension of the projection subspace.[default = 2]")
      ("n-components" , value<int>(), "number of eigenvectors to use for the spectral embedding.[default = 2]")
      ("n-iter,n" , value<int>(), "number of times the algorithm will be run.[default = 100]")
      ("n-init" , value<int>(), "[default = 1]")
      ("seed" , value<int>(), "[default = 0]")
      ("gamma,g", value<double>(), "kernel coefficient for rbf, poly, sigmoid, laplacian and chi2 kernels.[default = 1.0]")
      ("eps,e", value<double>(), "[default = 0.01]")
      ("norm-laplacian", value<bool>(), "[default: true]")
      ("n-neighbors", value<int>(), "[default: 10]")
      ("drop-first", value<bool>(), "[default: false]")
      ("mode", value<int>(), "[default: 1]")
      ("verbose", "set loglevel to DEBUG")
      ("verbose2", "set loglevel to TRACE");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  int k = 2;
  int n_comp = 2;
  int niter = 300;
  int n_init = 1;
  int seed = 0;
  double gamma = 1.0;
  double eps = 0.1;
  bool norm_laplacian = true;
  bool drop_first = false;
  int mode = 1;
  int n_neighbors = 10;
  std::string data_p;
  std::string affinity = "rbf";

  if(argmap.count("help")){
    std::cerr << opt << std::endl;
    exit(1);
  }
  if(argmap.count("n-clusters")){
    k = argmap["n-clusters"].as<int>();
  }
  if(argmap.count("n-components")){
    n_comp = argmap["n-components"].as<int>();
  }
  if(argmap.count("n-iter")){
    niter = argmap["n-iter"].as<int>();
  }
  if(argmap.count("n-init")){
    n_init = argmap["n-init"].as<int>();
  }
  if(argmap.count("eps")){
    eps = argmap["eps"].as<double>();
  }
  if(argmap.count("seed")){
    seed = argmap["seed"].as<int>();
  }
  if(argmap.count("gamma")){
    gamma = argmap["gamma"].as<double>();
  }
  if(argmap.count("affinity")){
    affinity = argmap["affinity"].as<std::string>();
  }
  if(argmap.count("n_neighbors")){
    n_neighbors = argmap["n_neighbors"].as<int>();
  }
  if(argmap.count("norm-laplacian")){
    norm_laplacian = argmap["norm-laplacian"].as<bool>();
  }
  if(argmap.count("drop-first")){
    drop_first = argmap["drop-first"].as<bool>();
  }
  if(argmap.count("mode")){
    mode = argmap["mode"].as<int>();
  }
  if(argmap.count("input")){
    data_p = argmap["input"].as<std::string>();
  }else {
    std::cerr << "input is not specified" << std::endl;
    std::cerr << opt << std::endl;
    exit(1);
  }

  auto mat = make_rowmajor_matrix_load<double>(data_p); //./test_data

  // hyper-parameters

  time_spent train(INFO);
  train.lap_start();
  auto model = spectral_clustering_train(std::move(mat),k,n_comp,niter,n_init, 
               eps, seed, gamma, affinity, n_neighbors, norm_laplacian,drop_first, mode);  
  train.lap_stop();
  train.show_lap("training time: ");

  auto label = model.labels;
  std::cout << "\ncluster output: \n";
  for(auto e: label) std::cout << e << " "; std::cout << std::endl;
  return 0;
}

