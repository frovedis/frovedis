#include <chrono>
#include <string>
#include <unordered_map>

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

  const size_t num_classes = 3;
  // iris dataset has no categorical feature
  std::unordered_map<size_t, size_t> categorical_features_info;
  const auto impurity = frovedis::tree::impurity_type::Gini;
  const size_t max_depth = 4;
  const size_t max_bins = 32;

  const size_t num_trees = 3;
  // feature_subset_strategy::Sqrt is typical for classification
  const auto feats_subset = frovedis::tree::feature_subset_strategy::Sqrt;
  const unsigned int seed = seconds_since_epoch();

  auto model = frovedis::random_forest::train_classifier<double>(
    dataset, labels,
    num_classes, categorical_features_info,
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

  return 0;
}
