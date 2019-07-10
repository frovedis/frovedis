#include <algorithm>
#include <exception>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#include <frovedis.hpp>
#include <frovedis/core/dvector.hpp>
#include <frovedis/matrix/rowmajor_matrix.hpp>
#include <frovedis/ml/tree/tree.hpp>

namespace po = boost::program_options;
using namespace frovedis;

template <typename T>
void do_train(
  const std::string& data_path,
  const std::string& label_path,
  const std::string& output_path,
  const size_t num_classes,
  const std::unordered_map<size_t, size_t>& categorical_features_info,
  const size_t num_trees,
  const tree::feature_subset_strategy feature_subset_strategy,
  const T feature_subset_rate,
  const tree::impurity_type impurity_type,
  const size_t max_depth,
  const size_t max_bins,
  const unsigned int seed,
  const bool binary_mode
) {
  time_spent timer(DEBUG);
  random_forest_model<T> model;

  auto strategy = tree::make_classification_strategy<T>()
    .set_num_classes(num_classes)
    .set_categorical_features_info(categorical_features_info)
    .set_num_trees(num_trees)
    .set_feature_subset_strategy(feature_subset_strategy)
    .set_feature_subset_rate(feature_subset_rate)
    .set_impurity_type(impurity_type)
    .set_max_depth(max_depth)
    .set_max_bins(max_bins)
    .set_seed(seed);
  auto builder = make_random_forest_builder(strategy);

  if (binary_mode) {
    timer.reset();
    auto dataset = make_rowmajor_matrix_loadbinary<T>(data_path);
    timer.show("load matrix: ");
    auto labels = make_dvector_loadbinary<T>(label_path);
    timer.show("load labels: ");
    model = builder.run(dataset, labels);
    timer.show("train model: ");
    model.savebinary(output_path);
    timer.show("save model:  ");
  } else {
    timer.reset();
    auto dataset = make_rowmajor_matrix_load<T>(data_path);
    timer.show("load matrix: ");
    auto labels = make_dvector_loadline(label_path).map(
      +[] (const std::string& line) -> T {
        return boost::lexical_cast<T>(line);
      }
    );
    timer.show("load labels: ");
    model = builder.run(dataset, labels);
    timer.show("train model: ");
    model.save(output_path);
    timer.show("save model:  ");
  }

  RLOG(TRACE) << std::endl << model;
}

template <typename T>
void do_train(const po::variables_map& argmap) {
  do_train<T>(
    argmap["input"].as<std::string>(),
    argmap["label"].as<std::string>(),
    argmap["output"].as<std::string>(),
    argmap["class"].as<size_t>(),
    tree::parse_categorical_features_info(
      argmap["category"].as<std::string>(),
      argmap["default-cardinality"].as<size_t>()
    ),
    argmap["trees"].as<size_t>(),
    tree::get_feature_subset_strategy(
      argmap["feature-subset-strategy"].as<std::string>()
    ),
    argmap["feature-subset-rate"].as<double>(),
    tree::get_impurity_type(argmap["impurity"].as<std::string>()),
    argmap["depth"].as<size_t>(),
    argmap["bins"].as<size_t>(),
    argmap["seed"].as<unsigned int>(),
    argmap.count("binary")
  );
}

po::variables_map parse(int argc, char** argv) {
  po::options_description opt_desc("");
  opt_desc.add_options()
    ("help,h", "show this help message and exit");

  po::options_description reqarg_desc("required arguments");
  reqarg_desc.add_options()
    ("input,i", po::value<std::string>(), "an input matrix")
    ("label,l", po::value<std::string>(), "an input label")
    ("output,o", po::value<std::string>(), "an output model");

  po::options_description optarg_desc("optional arguments");
  optarg_desc.add_options()
    ("class,c", po::value<size_t>()->default_value(2),
     "the number of classes")
    ("category", po::value<std::string>()->default_value(""),
     "categorical features' information (index: cardinality, ...)")
    ("default-cardinality", po::value<size_t>()->default_value(2),
     "a default cardinality of categorical features")
    ("trees,n", po::value<size_t>()->default_value(10),
     "the number of trees")
    ("feature-subset-strategy",
     po::value<std::string>()->default_value("auto"),
     "a feature subset strategy")
    ("feature-subset-rate", po::value<double>()->default_value(1.0),
     "a feature subset custom rate")
    ("impurity", po::value<std::string>()->default_value("default"),
     "an impurity function")
    ("depth,d", po::value<size_t>()->default_value(4),
     "the maximum depth")
    ("bins,b", po::value<size_t>()->default_value(32),
     "the maximum number of bins")
    ("seed", po::value<unsigned int>()->default_value(0), "a random seed")
    ("binary", "use binary input/output")
    ("double", "use double precision")
    ("verbose", "set log-level to DEBUG")
    ("trace", "set log-level to TRACE");

  opt_desc.add(reqarg_desc).add(optarg_desc);
  po::variables_map argmap;
  try {
    po::store(po::command_line_parser(argc, argv)
                .options(opt_desc)
//              .allow_unregistered()
                .run(),
              argmap);
    po::notify(argmap);
  } catch (const po::error_with_option_name& e) {
    std::cerr << e.what() << std::endl;
    finalizefrovedis(1);
  }

  // help message
  if (argmap.count("help")) {
    std::cerr << opt_desc;
    finalizefrovedis(0);
  }

  // check required arguments
  bool missing = false;
  for (const auto opt: reqarg_desc.options()) {
    const std::string& name = opt->long_name();
    if (!argmap.count(name)) {
      std::cerr << "option '--" << name << "' is required" << std::endl;
      missing = true;
    }
  }
  if (missing) { finalizefrovedis(1); }

  return argmap;
}

int main(int argc, char** argv) {
  use_frovedis use(argc, argv);

  const auto argmap = parse(argc, argv);
  if (argmap.count("verbose")) { set_loglevel(DEBUG); }
  if (argmap.count("trace")) { set_loglevel(TRACE); }

  try {
    if (argmap.count("double")) {
      do_train<double>(argmap);
    } else {
      do_train<float>(argmap);
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    finalizefrovedis(1);
  }

  return 0;
}
