#include <frovedis.hpp>
#include <frovedis/ml/glm/logistic_regression_with_sgd.hpp>
#include <frovedis/ml/glm/svm_with_sgd.hpp>
#include <boost/lexical_cast.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto samples = frovedis::make_crs_matrix_load<double>("./train.mat");
  auto label = frovedis::make_dvector_loadline("./train.label").
    map(+[](const std::string& s){return boost::lexical_cast<double>(s);});
  
  int num_iteration = 100;
  double alpha = 1.0;
  double minibatch_fraction = 1.0;
  frovedis::RegType rt = frovedis::ZERO;
  double regParam = 0.01;
  bool intercept = false;
  auto model = frovedis::logistic_regression_with_sgd::
    train(samples, label, num_iteration, alpha, minibatch_fraction, regParam,
          rt, intercept);

  /*
  double convTol = 0.001;
  frovedis::MatType mType = frovedis::CRS;
  auto model = frovedis::svm_with_sgd::
  train(samples, label, num_iteration, alpha, minibatch_fraction, regParam,
        rt, intercept, convTol, mType);
  */

  model.save("./model");

  frovedis::logistic_regression_model<double> lm;
  //frovedis::svm_model<double> lm;
  lm.load("./model");
  auto test = frovedis::make_crs_matrix_local_load<double>("./test.mat");
  auto result = lm.predict(test);
  for(auto i: result) std::cout << i << std::endl;
}
