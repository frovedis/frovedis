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
  set_loglevel(TRACE);

  using namespace boost::program_options;
    
  vector<size_t> dims;
  double init_stdev;
  size_t iteration;
  double init_learn_rate;
  string optimizer_name;
  string input_path, label_path, output_path;
  vector<double> regulars;
  string task;
  size_t batch_size_pernode;
  bool binary_input;

  options_description opt("option");
  opt.add_options()
    ("dim", value<vector<size_t>>(&dims)->multitoken()->zero_tokens()->default_value({1,1,8}, "'1 1 8'"), 
     "k0 k1 k2: k0=use bias, k1=use 1-way interactions, k2=dim of 2-way interactions")
    ("help,h", "print help")
    ("init-stdev", value<double>(&init_stdev)->default_value(0.1, "0.1"), 
     "stdev of initialization of 2-way factors")
    ("iter", value<size_t>(&iteration)->default_value(100, "100"), "number of iteration")
    ("learn-rate", value<double>(&init_learn_rate)->default_value(0.1, "0.1"), 
     "learning rate for SGD; dafault=0.1")
    ("method", value<string>(&optimizer_name)->default_value("SGD"),
     "learning method in SGD, SGDA, ALS, MCMC")
    ("input,i", value<string>(&input_path)->required(), "filename for training data, crs matrix format")
    ("label,l", value<string>(&label_path)->required(), "filename for training label, dvector format")
    ("output,o", value<string>(&output_path)->required(), "filename for output model")
    ("regular", value<vector<double>>(&regulars)->multitoken()->default_value(vector<double>{0.,0.,0.}, "'0 0 0'"), 
     "r0,r1,r2 for SGD or ALS: r0=bias reg., r1=1-way reg., r2=2-way reg.")
    ("task,t", value<string>(&task)->default_value("r"), "r=regression, c=classification")
    ("batchsize,s", value<size_t>(&batch_size_pernode)->default_value(5000, "5000"),
     "minibatch size per node")
    ("binary", bool_switch(&binary_input)->default_value(false, "false"),
     "use binary input file")
    ;

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().run(), argmap);
  if(argmap.count("help")){
    cerr << opt << endl;
    exit(1);
  }

  notify(argmap);

  if (dims.size() != 3)  throw runtime_error("--dim is required to have 3 values.");
  bool dim_0 = dims[0];
  bool dim_1 = dims[1];
  size_t dim_2 = dims[2];
  
  fm::FmOptimizer optimizer;
  if (optimizer_name == "SGD") {
    optimizer = FmOptimizer::SGD;
  } else if (optimizer_name == "SGDA") {
    optimizer = FmOptimizer::SGDA;
  } else if (optimizer_name == "ALS") {
    optimizer = FmOptimizer::ALS;
  } else if (optimizer_name == "MCMC") {
    optimizer = FmOptimizer::MCMC;
  } else {
    throw runtime_error("optimizer specified by -method is not supported.");
  }
  
  if (regulars.size() != 3)  throw runtime_error("--regular is required to have 3 values.");
  double regular_0 = regulars[0];
  double regular_1 = regulars[1];
  double regular_2 = regulars[2];
  
  bool is_regression;
  if (task == "r") {
    is_regression = true;
  } else if (task == "c") {
    is_regression = false;
  } else {
    throw runtime_error("-task is required to be 'r' or 'c'.");
  }

  crs_matrix<double> nl_data;
  if (binary_input) {
    nl_data = make_crs_matrix_loadbinary<double>(input_path);
  } else {
    nl_data = make_crs_matrix_load<double>(input_path);
  }
  
  dvector<double> dv_label;
  if (binary_input) {
    dv_label = make_dvector_loadbinary<double>(label_path);
  } else {
    dv_label = make_dvector_loadline<double>(label_path);
  }

  auto model = fm_train(dim_0, dim_1, dim_2, 
        init_stdev, iteration, init_learn_rate, optimizer,
        regular_0, regular_1, regular_2, is_regression,
        nl_data, dv_label, batch_size_pernode); 
        
  model.save(output_path);
  return 0;
}
