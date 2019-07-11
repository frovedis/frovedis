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

  // iris dataset has no categorical feature
  std::unordered_map<size_t, size_t> categorical_features_info;
  // use a regressional impurity
  const auto impurity = frovedis::tree::impurity_type::Variance;
  const size_t max_depth = 4;
  const size_t max_bins = 32;

  const size_t num_iterations = 3;
  const auto loss = frovedis::tree::loss_type::LogLoss;
  const double learning_rate = 0.1;

  auto model = frovedis::gradient_boosted_trees::train_classifier<double>(
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

  return 0;
}
