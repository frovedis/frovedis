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

  time_spent train(INFO);
  train.lap_start();
  auto label = spectral_clustering(mat,k,n_comp,niter,eps,
               norm_laplacian,precomputed,drop_first,gamma);
  train.lap_stop();
  train.show_lap("training time: ");

  //std::cout << "\ncluster output: \n";
  //for(auto e: label) std::cout << e << " "; std::cout << std::endl;
  return 0;
}

