#include <string>
#include <unordered_map>

#include <boost/lexical_cast.hpp>

#include <frovedis.hpp>
#include <frovedis/ml/tree/tree.hpp>

double parse(const std::string& line) {
  return boost::lexical_cast<double>(line);
}

int main(int argc, char** argv) {
  frovedis::use_frovedis use(argc, argv);

  auto dataset = frovedis::make_rowmajor_matrix_load<double>("./train.mat");
  auto labels = frovedis::make_dvector_loadline("./train.label").map(parse);

  // housing dataset has one categorical feature: {3: binary}
  std::unordered_map<size_t, size_t> categorical_features_info;
  categorical_features_info.emplace(3, 2);

  const auto impurity = frovedis::tree::impurity_type::Variance;
  const size_t max_depth = 4;
  const size_t max_bins = 32;

  const size_t num_iterations = 3;
  const auto loss = frovedis::tree::loss_type::LeastSquaresError;
  const double learning_rate = 0.1;

  auto model = frovedis::gradient_boosted_trees::train_regressor<double>(
    dataset, labels,
    categorical_features_info,
    impurity, max_depth, max_bins,
    num_iterations, loss, learning_rate
  );

  std::cout << model << std::endl;
  model.save("./model");

  auto model2 = frovedis::make_gbts_by_load<double>("./model");

  auto testers = frovedis::make_rowmajor_matrix_local_load<double>("./test.mat");
  auto results = model2.predict(testers);

  std::cout << "-------- Prediction Results:: --------" << std::endl;
  for (const auto result: results) {
    std::cout << result << std::endl;
  }
  std::cout << std::endl;

  auto answers = frovedis::make_dvector_loadline("./test.label").map(parse);
  auto dresults = frovedis::make_dvector_scatter(results);
  auto zipped = frovedis::zip(dresults, answers);

  const double mae = zipped.map(
    +[] (const double predict, const double answer) -> double {
      return std::abs(predict - answer);
    }
  ).reduce(frovedis::add<double>) / results.size();
  const double mse = zipped.map(
    +[] (const double predict, const double answer) -> double {
      const double error = predict - answer;
      return error * error;
    }
  ).reduce(frovedis::add<double>) / results.size();

  std::cout << "MAE:  " << mae << std::endl;
  std::cout << "MSE:  " << mse << std::endl;
  std::cout << "RMSE: " << std::sqrt(mse) << std::endl;

  return 0;
}
