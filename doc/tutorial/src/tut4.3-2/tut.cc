#include <frovedis.hpp>
#include <frovedis/ml/clustering/dbscan.hpp>

int main(int argc, char* argv[]) {
  frovedis::use_frovedis use(argc, argv);

  auto m = frovedis::make_rowmajor_matrix_load<double>("./train.mat");

  auto dbscan = frovedis::dbscan(0.5, 5);
  dbscan.fit(m);
  auto labels = dbscan.labels();

  for(auto j: labels) std::cout << j << " ";
  std::cout << std::endl;
}
