#include <frovedis.hpp>
#include <frovedis/ml/model_selection/grid_search_cv.hpp>
#include <frovedis/ml/glm/logistic_regression.hpp>

using namespace frovedis;

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);
  //set_loglevel(DEBUG);
  try {
    if(argc < 3) 
      throw std::runtime_error("Insufficient args! Syntax: ./search_cv <train-data> <train-label>\n");

    auto mat = make_crs_matrix_load<double>(argv[1]); // for sparse data
    //auto mat = make_rowmajor_matrix_load<double>(argv[1]); // for dense data
    auto label = make_dvector_loadline<double>(argv[2]);

    // constructing estimator for which parameter search will be performed
    logistic_regression<double> lr;
    lr.set_max_iter(50)
      .set_solver("sgd")
      .set_reg_param(0.0001)
      .set_tol(1e-4);     

    // key of 'params' has to be the known parameter to target ESTIMATOR
    std::map<std::string, std::vector<param_t>> params;
    params["max_iter"] = {100, 500, 1000};
    params["alpha"] = {0.1, 0.01, 0.001};
    params["reg_type"] = {"zero", "l1", "l2"};
    params["fit_intercept"] = {true, false};

    // frovedis::grid_search_cv demo for logistic regression
    int nsplits = 3;
    int min_proc_per_group = 2;
    bool refit = true;
    auto cv = make_grid_search_cv(lr, params, nsplits, min_proc_per_group, refit);
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
