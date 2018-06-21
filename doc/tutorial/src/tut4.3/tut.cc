#include <frovedis.hpp>
#include <frovedis/ml/clustering/kmeans.hpp>
#include <boost/lexical_cast.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  set_loglevel(DEBUG);
  auto samples = frovedis::make_crs_matrix_load<double>("./train.mat");
  int num_iteration = 100;
  double eps = 0.01;
  int k = 3;
  auto centroids = frovedis::kmeans(samples, k, num_iteration, eps);

  centroids.transpose().save("./model");

  auto c = frovedis::make_rowmajor_matrix_local_load<double>("./model");
  auto ct = c.transpose();
  auto mat = frovedis::make_crs_matrix_local_load<double>("./test.mat");
  auto r = kmeans_assign_cluster(mat, ct);
  for(auto i: r) std::cout << i << std::endl;
}
