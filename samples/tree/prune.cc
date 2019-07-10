#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include <frovedis.hpp>
#include <frovedis/ml/tree/tree_model.hpp>

namespace po = boost::program_options;
using namespace frovedis;

template <typename T>
void do_prune(
  const std::string& input_path, const std::string& output_path,
  const T min_info_gain, const bool binary_mode
) {
  time_spent timer(DEBUG);
  decision_tree_model<T> input_model, output_model;

  if (binary_mode) {
    timer.reset();
    input_model.loadbinary(input_path);
    timer.show("load model: ");
    output_model = input_model.prune(min_info_gain);
    timer.show("prune tree: ");
    output_model.savebinary(output_path);
    timer.show("save model: ");
  } else {
    timer.reset();
    input_model.load(input_path);
    timer.show("load model: ");
    output_model = input_model.prune(min_info_gain);
    timer.show("prune tree: ");
    output_model.save(output_path);
    timer.show("save model: ");
  }

  RLOG(TRACE) << "<BEFORE>" << std::endl << input_model << std::endl;
  RLOG(TRACE) << "<AFTER>" << std::endl << output_model;
}

template <typename T>
void do_prune(const po::variables_map& argmap) {
  do_prune<T>(
    argmap["input"].as<std::string>(),
    argmap["output"].as<std::string>(),
    argmap["min-infogain"].as<double>(),
    argmap.count("binary")
  );
}

po::variables_map parse(int argc, char** argv) {
  po::options_description opt_desc("");
  opt_desc.add_options()
    ("help,h", "show this help message and exit");

  po::options_description reqarg_desc("required arguments");
  reqarg_desc.add_options()
    ("input,i", po::value<std::string>(), "an input model")
    ("output,o", po::value<std::string>(), "an output model");

  po::options_description optarg_desc("optional arguments");
  optarg_desc.add_options()
    ("min-infogain,g", po::value<double>()->default_value(0.0),
     "the minimum information gain (threshold)")
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
      do_prune<double>(argmap);
    } else {
      do_prune<float>(argmap);
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    finalizefrovedis(1);
  }

  return 0;
}
