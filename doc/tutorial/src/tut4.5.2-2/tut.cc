#include <chrono>
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/lexical_cast.hpp>

#include <frovedis.hpp>
#include <frovedis/ml/tree/tree.hpp>

double parse(const std::string& line) {
  return boost::lexical_cast<double>(line);
}

unsigned int seconds_since_epoch() {
  auto ep_point = std::chrono::system_clock::now();
  auto ep_dur = ep_point.time_since_epoch();
  auto ep_sec = std::chrono::duration_cast<std::chrono::seconds>(ep_dur);
  return ep_sec.count();
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

  const size_t num_trees = 3;
  // feature_subset_strategy::OneThird is typical for regression
  const auto feats_subset = frovedis::tree::feature_subset_strategy::OneThird;
  const unsigned int seed = seconds_since_epoch();

  auto model = frovedis::random_forest::train_regressor<double>(
    dataset, labels,
    categorical_features_info,
    num_trees, feats_subset,
    impurity, max_depth, max_bins,
    seed
  );

  std::cout << model << std::endl;
  model.save("./model");

  auto model2 = frovedis::make_forest_by_load<double>("./model");

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
