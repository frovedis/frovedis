#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include <frovedis.hpp>
#include <frovedis/ml/tree/ensemble_model.hpp>

namespace po = boost::program_options;
using namespace frovedis;

template <typename T>
void do_dump(const std::string& model_path, const bool binary_mode) {
  time_spent timer(DEBUG);
  gradient_boosted_trees_model<T> model;

  if (binary_mode) {
    timer.reset();
    model.loadbinary(model_path);
    timer.show("load model: ");
  } else {
    timer.reset();
    model.load(model_path);
    timer.show("load model: ");
  }

  timer.reset();
  std::cout << model;
  timer.show("dump model: ");
}

template <typename T>
void do_dump(const po::variables_map& argmap) {
  do_dump<T>(argmap["input"].as<std::string>(), argmap.count("binary"));
}

po::variables_map parse(int argc, char** argv) {
  po::options_description opt_desc("");
  opt_desc.add_options()
    ("help,h", "show this help message and exit");

  po::options_description reqarg_desc("required arguments");
  reqarg_desc.add_options()
    ("input,i", po::value<std::string>(), "an input model");

  po::options_description optarg_desc("optional arguments");
  optarg_desc.add_options()
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
      do_dump<double>(argmap);
    } else {
      do_dump<float>(argmap);
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    finalizefrovedis(1);
  }

  return 0;
}
