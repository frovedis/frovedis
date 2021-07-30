#include <frovedis.hpp>
#include <frovedis/ml/model_selection/grid_search_cv.hpp>
#include <frovedis/ml/glm/logistic_regression.hpp>
#include <boost/program_options.hpp>


using namespace frovedis;

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);
  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
      ("help,h", "produce help message")
      ("input-data,i" , value<std::string>(), "input matrix data.")
      ("input-labels,l" , value<std::string>(), "input label data.")
      ("niter" , value<int>(), "[default = 50]")
      ("solver" , value<std::string>(), "[default = sgd]") 
      ("reg-param" , value<double>(), "[default = 0.0001]")
      ("tol" , value<double>(), "[default = 1e-4]")
      ("nsplits" , value<int>(), "[default = 5]")
      ("min-proc-per-group" , value<int>(), "[default = 2]")
      ("refit" , value<bool>(), "[default = true]")
      ("verbose", "set loglevel to DEBUG")
      ("verbose2", "set loglevel to TRACE");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  std::string data_p, data_l, solver = "sgd";
  int niter = 50, nsplits = 5, min_proc_pg = 2;
  double reg_param = 0.0001, tol = 1e-4;
  bool refit = true;

  if(argmap.count("input-data")){
    data_p = argmap["input-data"].as<std::string>();
  }else {
    std::cerr << "input-data is not specified" << std::endl;
    std::cerr << opt << std::endl;
    exit(1);
  }
  if(argmap.count("input-labels")){
    data_l = argmap["input-labels"].as<std::string>();
  }else {
    std::cerr << "input-labels is not specified" << std::endl;
    std::cerr << opt << std::endl;
    exit(1);
  }
  if(argmap.count("solver")){
    solver = argmap["solver"].as<std::string>();
  }
  if(argmap.count("min-proc-per-group")){
    min_proc_pg = argmap["min-proc-per-group"].as<int>();
  }
  if(argmap.count("reg-param")){
    reg_param = argmap["reg-param"].as<double>();
  }
  if(argmap.count("tol")){
    tol = argmap["tol"].as<double>();
  }
  if(argmap.count("niter")){
    niter = argmap["niter"].as<int>();
  }
  if(argmap.count("nsplits")){
    nsplits = argmap["nsplits"].as<int>();
  }
  if(argmap.count("refir")){
    refit = argmap["refit"].as<bool>();
  }
  try {
    auto mat = make_crs_matrix_load<double>(data_p); // for sparse data
    //auto mat = make_rowmajor_matrix_load<double>(data_p); // for dense data
    auto label = make_dvector_loadline<double>(data_l);

    // constructing estimator for which parameter search will be performed
    logistic_regression<double> lr;
    lr.set_max_iter(niter)
      .set_solver(solver)
      .set_reg_param(reg_param)
      .set_tol(tol);     

    // key of 'params' has to be the known parameter to target ESTIMATOR
    std::map<std::string, std::vector<param_t>> params;
    params["max_iter"] = {100, 500, 1000};
    params["alpha"] = {0.1, 0.01, 0.001};
    params["reg_type"] = {"ZERO", "L1", "L2"};
    params["fit_intercept"] = {true, false};

    // frovedis::grid_search_cv demo for logistic regression
    auto cv = make_grid_search_cv(lr, params, nsplits, min_proc_pg, refit);
    cv.fit(mat, label);

    std::cout << "grid search cv result: "; 
    cv.debug_print();
    std::cout << "prediction on best param: "; 
    debug_print_vector(cv.predict(mat), 5);
  }
  catch (std::exception& e) {
    REPORT_ERROR(USER_ERROR, "exception caught: " + std::string(e.what()) + "\n");
  }
  return 0; 
}
