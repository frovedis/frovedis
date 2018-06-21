#include <frovedis.hpp>
#include <frovedis/ml/fm/fm.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto data = frovedis::make_crs_matrix_load<double>("./train.mat");
  auto label = frovedis::make_dvector_loadline<double>("./train.label");

  bool dim_0 = true;
  bool dim_1 = true;
  size_t dim_2 = 8;
  double init_stdev = 0.1;
  size_t iteration = 100;
  double init_learn_rate = 0.1;
  frovedis::FmOptimizer optimizer = frovedis::FmOptimizer::SGD;
  double regular_0 = 0.;
  double regular_1 = 0.;
  double regular_2 = 0.1;
  bool is_regression = true;
  size_t batchsize_pernode = 2000;

  auto trained = frovedis::fm_train<double>(dim_0, dim_1, dim_2, 
        init_stdev, iteration, init_learn_rate, optimizer,
        regular_0, regular_1, regular_2, is_regression,
        data, label, batchsize_pernode); 

  trained.save("./model");


  auto loaded = frovedis::load_fm_model<double>("./model");
  auto testdata = frovedis::make_crs_matrix_local_load<double>("./test.mat");
  auto testlabel = frovedis::make_dvector_loadline<double>("./test.label").gather();
    
  auto rmse = frovedis::fm_test(loaded, testdata, testlabel);
  std::cout << "RMSE: " << rmse << std::endl;
  
}
