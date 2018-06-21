#include <sstream>
#include <boost/program_options.hpp>

#include <frovedis/ml/fm/fm.hpp>
#include <frovedis/matrix/crs_matrix.hpp>

using namespace boost;
using namespace frovedis;
using namespace std;


int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);

  // debug option
  // set_loglevel(TRACE);

  using namespace boost::program_options;
    
  string model_path, input_path, label_path;
  bool is_binary_model, is_binary_data;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("model,m", value<string>(&model_path)->required(), "filename for trained model, fm_model")
    ("input,i", value<string>(&input_path)->required(), "filename for test data, crs matrix format")
    ("label,l", value<string>(&label_path)->required(), "filename for test label, dvector format")
    ("binary-model", bool_switch(&is_binary_model)->default_value(false, "false"), "use binary for trained model")
    ("binary-data", bool_switch(&is_binary_data)->default_value(false, "false"), "use binary for test data and label")
    ;

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().run(), argmap);
  if(argmap.count("help")){
    cerr << opt << endl;
    exit(1);
  }

  notify(argmap);

  fm::fm_model<double> trained_model;
  if (is_binary_model) {
    throw std::runtime_error("Loading binary fm model is not implemented yet.");
  } else {
    trained_model = fm::load_fm_model<double>(model_path);
  }

  crs_matrix_local<double> test_data;
  std::vector<double> label;
  if (is_binary_data) {
    test_data = make_crs_matrix_local_loadbinary<double>(input_path);
    label = make_dvector_loadbinary<double>(label_path).gather();
  } else {
    test_data = make_crs_matrix_local_load<double>(input_path);
    label = make_dvector_loadline<double>(label_path).gather();
  }

  auto result = fm_test(trained_model, test_data, label);
  std::cout << "# of samples: " << test_data.local_num_row << std::endl;
  if (trained_model.config.is_regression) {
    std::cout << "RMSE: " << result << std::endl;
  } else {
    std::cout << "Accuracy: " << result << std::endl;
  }
  
  return 0;
}
