#include <frovedis.hpp>
#include <frovedis/ml/clustering/spectral_clustering.hpp>

using namespace frovedis;

int main(int argc, char** argv) {
  use_frovedis use(argc,argv);
  auto mat = make_rowmajor_matrix_load<double>("./test_data");

  // hyper-parameters
  int k = 2;
  int n_comp = 2;
  int niter = 300;
  double gamma = 1.0;
  double eps = 0.1;
  bool norm_laplacian = true;
  bool precomputed = false;
  bool drop_first = false;
  int mode = 1;  

  time_spent train(INFO);
  train.lap_start();
  auto model = spectral_clustering_train(std::move(mat),k,n_comp,niter,eps,
               norm_laplacian,precomputed,gamma,mode,drop_first);  
  train.lap_stop();
  train.show_lap("training time: ");

  auto label = model.labels;
  std::cout << "\ncluster output: \n";
  for(auto e: label) std::cout << e << " "; std::cout << std::endl;
  return 0;
}

