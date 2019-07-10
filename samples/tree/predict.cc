#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#include <frovedis.hpp>
#include <frovedis/core/dvector.hpp>
#include <frovedis/core/zipped_dvectors.hpp>
#include <frovedis/matrix/rowmajor_matrix.hpp>
#include <frovedis/ml/tree/tree_model.hpp>

namespace po = boost::program_options;
using namespace frovedis;

template <typename T>
void show_classification_error_rate(
  zipped_dvectors<T, T>& zipped, const T num_records
) {
  std::cout << "Error rate: " << zipped.map(
    +[] (const T predict, const T label) -> T {
      return static_cast<T>(label != predict);
    }
  ).reduce(add<T>) / num_records * 100 << "%" << std::endl;
}

template <typename T>
void show_regression_errors(
  zipped_dvectors<T, T>& zipped, const T num_records
) {
  const T mae = zipped.map(
    +[] (const T predict, const T label) -> T {
      return std::abs(label - predict);
    }
  ).reduce(add<T>) / num_records;

  const T mse = zipped.map(
    +[] (const T predict, const T label) -> T {
      const T error = label - predict;
      return error * error;
    }
  ).reduce(add<T>) / num_records;

  std::cout << "MAE:  " << mae << std::endl;
  std::cout << "MSE:  " << mse << std::endl;
  std::cout << "RMSE: " << std::sqrt(mse) << std::endl;
}

template <typename T>
void do_validate(
  const std::vector<T>& predicts, dvector<T>& labels,
  const tree::algorithm algo
) {
  const T num_records = static_cast<T>(predicts.size());
  auto dpredicts = make_dvector_scatter(predicts);
  auto zipped = zip(dpredicts, labels);

  switch (algo) {
  case tree::algorithm::Classification:
    show_classification_error_rate(zipped, num_records);
    return;
  case tree::algorithm::Regression:
    show_regression_errors(zipped, num_records);
    return;
  default:
    throw std::logic_error("invalid tree algorithm");
  }
}

template <typename T>
void do_predict(
  const std::string& data_path,
  const std::string& model_path,
  const std::string& output_path,
  const std::string& label_path,
  const bool binary_mode
) {
  time_spent timer(DEBUG);
  decision_tree_model<T> model;

  if (binary_mode) {
    timer.reset();
    auto dataset = make_rowmajor_matrix_local_loadbinary<T>(data_path);
    timer.show("load matrix: ");
    model.loadbinary(model_path);
    timer.show("load model:  ");
    auto results = model.predict(dataset);
    timer.show("predict:     ");
    make_dvector_scatter(results).savebinary(output_path);
    timer.show("save result: ");

    if (!label_path.empty()) {
      timer.reset();
      auto labels = make_dvector_loadbinary<T>(label_path);
      timer.show("load labels: ");
      do_validate(results, labels, model.get_algo());
      timer.show("validate:    ");
    }
  } else {
    timer.reset();
    auto dataset = make_rowmajor_matrix_local_load<T>(data_path);
    timer.show("load matrix: ");
    model.load(model_path);
    timer.show("load model:  ");
    auto results = model.predict(dataset);
    timer.show("predict:     ");
    make_dvector_scatter(results).saveline(output_path);
    timer.show("save result: ");

    if (!label_path.empty()) {
      timer.reset();
      auto labels = make_dvector_loadline(label_path).map(
        +[] (const std::string& line) -> T {
          return boost::lexical_cast<T>(line);
        }
      );
      timer.show("load labels: ");
      do_validate(results, labels, model.get_algo());
      timer.show("validate:    ");
    }
  }
}

template <typename T>
void do_predict(const po::variables_map& argmap) {
  do_predict<T>(
    argmap["input"].as<std::string>(),
    argmap["model"].as<std::string>(),
    argmap["output"].as<std::string>(),
    argmap["label"].as<std::string>(),
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
    ("model,m", po::value<std::string>(), "an input model")
    ("output,o", po::value<std::string>(), "an output prediction result");

  po::options_description optarg_desc("optional arguments");
  optarg_desc.add_options()
    ("label,l", po::value<std::string>()->default_value(""),
     "a correct label for validation")
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
      do_predict<double>(argmap);
    } else {
      do_predict<float>(argmap);
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    finalizefrovedis(1);
  }

  return 0;
}
