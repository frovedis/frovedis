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

  const size_t num_classes = 3;
  // iris dataset has no categorical feature, so leave this map empty
  std::unordered_map<size_t, size_t> categorical_features_info;
  // frovedis::tree::impurity_type::Entropy is also available
  const auto impurity = frovedis::tree::impurity_type::Gini;
  const size_t max_depth = 5;
  const size_t max_bins = 32;
  const size_t min_instances_per_node = 1;
  const double min_info_gain = 0.0;

  auto model = frovedis::decision_tree::train_classifier<double>(
    dataset, labels,
    num_classes, categorical_features_info,
    impurity, max_depth, max_bins,
    min_instances_per_node, min_info_gain
  );

  std::cout << model << std::endl;
  model.save("./model");

  auto model2 = frovedis::make_tree_by_load<double>("./model");

  auto testers = frovedis::make_rowmajor_matrix_local_load<double>("./test.mat");
  auto results = model2.predict_with_probability(testers);

  std::cout << "-------- Prediction Results:: --------" << std::endl;
  for (const auto result: results) {
    std::cout <<
      result.get_predict() << " (" <<
      result.get_probability() * 100 << "%)" <<
    std::endl;
  }

  return 0;
}
