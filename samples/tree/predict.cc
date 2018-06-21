#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#include <frovedis.hpp>
#include <frovedis/ml/tree/tree_model.hpp>
#include <frovedis/matrix/rowmajor_matrix.hpp>

namespace po = boost::program_options;
using namespace frovedis;

// default values for program options
constexpr size_t ARGNAME_LENGTH = 24 - (2 + 1);
#define POSARG_CAPTION "positional arguments"
#define OPTARG_CAPTION "optional arguments"

template <typename T>
void show_classification_error_rate(
  zipped_dvectors<T, T>& zipped, const T num_records
) {
  std::cout << "Error rate: " << zipped.map(
    +[] (const T predict, const T answer) -> T {
      return static_cast<T>(predict != answer);
    }
  ).reduce(add<T>) / num_records * 100 << "%" << std::endl;
}

template <typename T>
void show_regression_errors(
  zipped_dvectors<T, T>& zipped, const T num_records
) {
  const T mae = zipped.map(
    +[] (const T predict, const T answer) -> T {
      return std::abs(predict - answer);
    }
  ).reduce(add<T>) / num_records;

  const T mse = zipped.map(
    +[] (const T predict, const T answer) -> T {
      const T error = predict - answer;
      return error * error;
    }
  ).reduce(add<T>) / num_records;

  std::cout << "MAE:  " << mae << std::endl;
  std::cout << "MSE:  " << mse << std::endl;
  std::cout << "RMSE: " << std::sqrt(mse) << std::endl;
}

template <typename T>
void do_validate(
  const std::vector<T>& predicts, dvector<T>& answers,
  const tree::algorithm algo
) {
  const T num_records = static_cast<T>(predicts.size());
  auto dpredicts = make_dvector_scatter(predicts);
  auto zipped = zip(dpredicts, answers);

  switch (algo) {
  case tree::algorithm::Classification:
    show_classification_error_rate(zipped, num_records);
    return;
  case tree::algorithm::Regression:
    show_regression_errors(zipped, num_records);
    return;
  default:
    std::logic_error("no such algorithm");
  }
}

template <typename T>
void do_predict(
  const std::string& input,
  const std::string& saved_model,
  const std::string& output,
  const std::string& corrected,
  const bool binary_mode
) {
  time_spent timer(DEBUG);
  decision_tree_model<T> model;

  if (binary_mode) {
    timer.reset();
    auto dataset = make_rowmajor_matrix_local_loadbinary<T>(input);
    timer.show("load matrix: ");
    model.loadbinary(saved_model);
    timer.show("load model:  ");
    auto results = model.predict(dataset);
    timer.show("predict:     ");
    make_dvector_scatter(results).savebinary(output);
    timer.show("save result: ");

    if (!corrected.empty()) {
      timer.reset();
      auto answers = make_dvector_loadbinary<T>(corrected);
      timer.show("load answer: ");
      do_validate(results, answers, model.get_algo());
      timer.show("validate:    ");
    }
  } else {
    timer.reset();
    auto dataset = make_rowmajor_matrix_local_load<T>(input);
    timer.show("load matrix: ");
    model.load(saved_model);
    timer.show("load model:  ");
    auto results = model.predict(dataset);
    timer.show("predict:     ");
    make_dvector_scatter(results).saveline(output);
    timer.show("save result: ");

    if (!corrected.empty()) {
      timer.reset();
      auto answers = make_dvector_loadline(corrected).map(
        +[] (const std::string& line) -> T {
          return boost::lexical_cast<T>(line);
        }
      );
      timer.show("load answer: ");
      do_validate(results, answers, model.get_algo());
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
    argmap["corrected"].as<std::string>(),
    argmap.count("binary")
  );
}

void abort(const int code = 0) {
  std::vector<std::string> dummy;
  send_bcast_rpcreq(rpc_type::finalize_type, 0, 0, "", dummy);
  MPI_Finalize();
  exit(code);
}

po::variables_map parse(int argc, char** argv) {
  // a description of positional arguments
  po::options_description posarg_desc(POSARG_CAPTION);
  posarg_desc.add_options()(
    "input", po::value<std::string>(), "an input matrix"
  )(
    "model", po::value<std::string>(), "an input model"
  )(
    "output", po::value<std::string>(), "an output predict result"
  );
  po::positional_options_description posarg_settings;
  posarg_settings.add("input", 1).add("model", 1).add("output", 1);

  // a description of optional arguments
  po::options_description optarg_desc(OPTARG_CAPTION);
  optarg_desc.add_options()(
    "help,h", "show this help message and exit"
  )(
    "corrected", po::value<std::string>()->default_value(""),
    "corrected labels for validation"
  )(
    "binary", "use binary input/output"
  )(
    "double", "use 64-bit float type (default: 32-bit float)"
  )(
    "verbose", "set log-level to DEBUG"
  )(
    "trace", "set log-level to TRACE"
  );

  // merge descriptions of all arguments
  po::options_description arg_desc;
  arg_desc.add(posarg_desc).add(optarg_desc);

  po::variables_map argmap;
  try {
    auto parsed = po::command_line_parser(argc, argv)
      .options(arg_desc)
      .positional(posarg_settings)
//    .allow_unregistered()
      .run();

    // make sure there are no `optional-style` positional arguments
    for (const auto parsed_opt: parsed.options) {
      if (parsed_opt.position_key < 0) {
        for (const auto opt: posarg_desc.options()) {
          if (parsed_opt.string_key == opt->long_name()) {
            throw po::unknown_option(parsed_opt.string_key);
          }
        }
      }
    }

    po::store(parsed, argmap);
    po::notify(argmap);
  } catch (const po::error_with_option_name& e) {
    std::cerr << e.what() << std::endl;
    abort(1);
  }

  // help message
  if (argmap.count("help")) {
    size_t max_length = ARGNAME_LENGTH;
    for (const auto opt: posarg_desc.options()) {
      const size_t temp = opt->long_name().length();
      if (max_length < temp) { max_length = temp; }
    }
    for (const auto opt: optarg_desc.options()) {
      const size_t namelen = opt->format_name().length();
      const size_t paramlen = opt->format_parameter().length();
      const size_t temp = namelen + paramlen + !!(paramlen > 0);
      if (max_length < temp) { max_length = temp; }
    }

    std::cerr << POSARG_CAPTION << ":" << std::endl;
    for (const auto opt: posarg_desc.options()) {
      const auto name = opt->long_name();
      std::cerr <<
        "  " << name <<
        std::string(max_length - name.length(), ' ') << " " <<
        opt->description() <<
      std::endl;
    }
    std::cerr << optarg_desc;

    abort(0);
  }

  // check required arguments
  for (const auto opt: posarg_desc.options()) {
    const std::string& name = opt->long_name();
    if (!argmap.count(name)) {
      std::cerr << "missing argument '" << name << "'" << std::endl;
      abort(1);
    }
  }

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
    abort(1);
  }

  return 0;
}
