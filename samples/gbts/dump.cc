#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include <frovedis.hpp>
#include <frovedis/ml/tree/ensemble_model.hpp>

namespace po = boost::program_options;
using namespace frovedis;

// default values for program options
constexpr size_t ARGNAME_LENGTH = 24 - (2 + 1);
#define POSARG_CAPTION "positional arguments"
#define OPTARG_CAPTION "optional arguments"

template <typename T>
void do_dump(const std::string& input, const bool binary_mode) {
  time_spent timer(DEBUG);
  gradient_boosted_trees_model<T> model;

  if (binary_mode) {
    timer.reset();
    model.loadbinary(input);
    timer.show("load model: ");
  } else {
    timer.reset();
    model.load(input);
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
  // a description of positional arguments
  po::options_description posarg_desc(POSARG_CAPTION);
  posarg_desc.add_options()(
    "input", po::value<std::string>(), "an input model"
  );
  po::positional_options_description posarg_settings;
  posarg_settings.add("input", 1);

  // a description of optional arguments
  po::options_description optarg_desc(OPTARG_CAPTION);
  optarg_desc.add_options()(
    "help,h", "show this help message and exit"
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
    finalizefrovedis(1);
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

    finalizefrovedis(0);
  }

  // check required arguments
  for (const auto opt: posarg_desc.options()) {
    const std::string& name = opt->long_name();
    if (!argmap.count(name)) {
      std::cerr << "missing argument '" << name << "'" << std::endl;
      finalizefrovedis(1);
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
