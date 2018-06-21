#include <frovedis.hpp>
#include <frovedis/ml/recommendation/als.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto data = frovedis::make_crs_matrix_load<double>("./train.mat");

  int num_iteration = 100;
  double alpha = 1.0;
  double regParam = 0.001;
  int seed = 0;
  int factor = 20;
  auto model = frovedis::matrix_factorization_using_als::
    train(data, factor, num_iteration, alpha, regParam, seed);

  model.save("./model");

  frovedis::matrix_factorization_model<double> mfm;
  mfm.load("./model");
  std::vector<size_t> pids;
  pids.push_back(6);
  pids.push_back(10);
  pids.push_back(12);
  pids.push_back(14);
  pids.push_back(17);
  pids.push_back(20);
  pids.push_back(23);
  pids.push_back(24);
  pids.push_back(27);
  pids.push_back(31);
  for(size_t i = 0; i < pids.size(); i++) {
    auto p = mfm.predict(0, pids[i] - 1); // 0 origin
    std::cout << "user = 1, movie = " << pids[i] << ": " << p << std::endl;
  }
}
