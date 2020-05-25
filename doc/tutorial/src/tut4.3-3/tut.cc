#include <frovedis.hpp>
#include <frovedis/matrix/crs_matrix.hpp>
#include <frovedis/ml/clustering/art2a.hpp>


int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  set_loglevel(DEBUG);
  
  auto data = frovedis::make_crs_matrix_load<double>("./train.mat").to_rowmajor();
  size_t weight_size = 2;
  size_t max_iter = 100;
  bool random_shuffle = true;
  size_t random_seed = 1022;
  double learning_rate = 0.5;
  double vigilance = 0.01;

  auto result = frovedis::art2a(data, weight_size, max_iter, random_shuffle, random_seed,
				learning_rate, vigilance);
  
  auto cluster_weight = std::get<0>(result);
  size_t cluster_size = cluster_weight.num_row;
  auto cluster_idx = std::get<1>(result);
  
  std::vector<std::vector<size_t>> cluster_group(cluster_size);
  for (size_t i = 0; i < cluster_idx.size(); i++) {
    size_t ci = cluster_idx[i];
    cluster_group[ci].push_back(i);
  }

  std::cout << "Number of cluster = " << cluster_idx.size() << std::endl;
  std::cout << "cluster id : data id" << std::endl;
  for (size_t ci = 0; ci < cluster_size; ci++) {
    std::cout << ci << " : ";
    for (auto i: cluster_group[ci]) std::cout << i << " ";
    std::cout << std::endl;
  }
  
}
