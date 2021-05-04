#include <frovedis.hpp>
#include <frovedis/ml/clustering/gmm.hpp>

int main(int argc, char* argv[]) {
  frovedis::use_frovedis use(argc, argv);

  auto Data = frovedis::make_rowmajor_matrix_load<double>("./train.mat");
  int num_iteration = 100;
  double eps = 0.01;
  int k = 3;
  auto gmm_model = frovedis::gmm(Data, k, num_iteration, eps);
  const std::string& path = "./model";
  gmm_model.save(path);
  auto mat = frovedis::make_rowmajor_matrix_local_load<double>("./test.mat");
  auto gmm_cluster = frovedis::gmm_assign_cluster(mat, k, gmm_model.model,
                                                  gmm_model.covariance,
                                                  gmm_model.pi);
  gmm_cluster.save("./result");
}
