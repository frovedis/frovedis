#include <frovedis.hpp>
#include <frovedis/ml/glm/linear_regression_with_sgd.hpp>
#include <frovedis/ml/glm/ridge_regression_with_sgd.hpp>
#include <frovedis/ml/glm/lasso_with_sgd.hpp>
#include <boost/lexical_cast.hpp>

double parse(const std::string& s){return boost::lexical_cast<double>(s);}

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto samples = frovedis::make_crs_matrix_load<double>("./train.mat");
  auto label = frovedis::make_dvector_loadline("./train.label").map(parse);
  
  int num_iteration = 1000;
  double alpha = 0.00000001;
  double minibatch_fraction = 1.0;
  bool intercept = true;
  double convTol = 0;
  frovedis::MatType mType = frovedis::CRS;
  auto model = frovedis::linear_regression_with_sgd::
    train(samples, label, num_iteration, alpha, minibatch_fraction,
          intercept, convTol, mType);
  /*
  // L2 regularization
  double regParam = 0.001;
  auto model = frovedis::ridge_regression_with_sgd::
    train(samples, label, num_iteration, alpha, minibatch_fraction,
          regParam, intercept, convTol, mType);
  */
  /*
  // L1 regularization
  double regParam = 0.0001;
  auto model = frovedis::lasso_with_sgd::
    train(samples, label, num_iteration, alpha, minibatch_fraction,
          regParam, intercept, convTol, mType);
  */
  model.save("./model");

  frovedis::linear_regression_model<double> lm;
  lm.load("./model");
  auto test = frovedis::make_crs_matrix_local_load<double>("./test.mat");
  auto result = lm.predict(test);
  for(auto i: result) std::cout << i << std::endl;
}
