#ifndef __LASSO_REG__
#define __LASSO_REG__

#include <frovedis/ml/metrics.hpp>
#include <frovedis/ml/model_selection/param.hpp>
#include <frovedis/ml/glm/lasso_with_sgd.hpp>
#include <frovedis/ml/glm/lasso_with_lbfgs.hpp>

namespace frovedis {

template <class T>
struct lasso_regression {
  lasso_regression(int max_iter = 1000,
                   double alpha = 0.01,
                   const std::string& solver = "sgd",
                   double reg_param = 0.01,
                   bool fit_intercept = false,
                   double mini_batch_fraction = 1.0,
                   int hs = 10,
                   double convergence_tol = 0.001,
                   bool warm_start = false,
                   #if defined(_SX) || defined(__ve__)
                   MatType mType = HYBRID
                   #else
                   MatType mType = CRS
                   #endif
                   ) {
    this->max_iter = max_iter;
    this->alpha = alpha;
    this->solver = solver;
    this->reg_param = reg_param;
    this->fit_intercept = fit_intercept;
    this->mbf = mini_batch_fraction;
    this->hist_size = hs;
    this->tol = convergence_tol;
    this->warm_start = warm_start;
    this->mat_type = mType;
    this->is_fitted = false;
    this->n_iter_ = 0;
    this->n_features_ = 0;
  }
  lasso_regression<T>& 
  set_max_iter(int max_iter) {
    std::string msg = "expected a positive max_iter; received: " + STR(max_iter) + "\n";
    require(max_iter > 0, msg);
    this->max_iter = max_iter;
    return *this;  
  }
  lasso_regression<T>& 
  set_alpha(double alpha) {
    std::string msg = "expected a positive alpha (learning rate); received: " + STR(alpha) + "\n";
    require(alpha > 0, msg);
    this->alpha = alpha;
    return *this; 
  }
  lasso_regression<T>& 
  set_solver(const std::string& solver) {
    std::string msg = "expected sgd or lbfgs; received: " + solver + "\n";
    require(solver == "sgd" || solver == "lbfgs", msg);
    this->solver = solver;
    return *this;  
  }
  lasso_regression<T>& 
  set_reg_param(double reg_param) {
    std::string msg = "expected a positive regularization parameter; received: " + STR(reg_param) + "\n";
    require(reg_param > 0, msg);
    this->reg_param = reg_param;
    return *this;  
  }
  lasso_regression<T>& 
  set_intercept(bool is_intercept) {
    this->fit_intercept = is_intercept;
    return *this;  
  }
  lasso_regression<T>& 
  set_mini_batch_fraction(double mbf) {
    std::string msg = "expected a positive mini-batch fraction; received: " + STR(mbf) + "\n";
    require(mbf > 0, msg);
    this->mbf = mbf;
    return *this;  
  }
  lasso_regression<T>& 
  set_hist_size(int hs) {
    std::string msg = "expected a positive history size; received: " + STR(hs) + "\n";
    require(hs > 0, msg);
    this->hist_size = hs;
    return *this;  
  }
  lasso_regression<T>& 
  set_tol(double tol) {
    std::string msg = "expected a positive convergence tolerance; received: " + STR(tol) + "\n";
    require(tol > 0, msg);
    this->tol = tol;
    return *this;  
  }
  lasso_regression<T>&
  set_warm_start(bool warm_start) {
    this->warm_start = warm_start;
    return *this;
  }

  lasso_regression<T>&
  set_params(glm_config& config) {
    set_max_iter(config.numIteration);
    set_alpha(config.alpha);
    set_solver(config.solver);
    set_reg_param(config.regParam);
    set_intercept(config.isIntercept);
    set_mini_batch_fraction(config.miniBatchFraction);
    set_hist_size(config.histSize);
    set_tol(config.convergenceTol);
    set_warm_start(config.warmStart);
    return *this;
  }

  // frovedis::grid_sdearch_cv compatible setter
  lasso_regression<T>& 
  set_params(std::vector<std::pair<std::string, param_t>>& data) {
    std::string msg = "";
    for (auto& e: data) {
      auto param = e.first;
      auto val = e.second;
      if(param == "max_iter") {              
        set_max_iter(val.get<int>()); 
        msg += "max_iter: " + val.tt + "; ";
      }
      else if(param == "alpha") { 
        set_alpha(val.get<double>());
        msg += "alpha: " + val.tt + "; ";
      }
      else if(param == "mini_batch_fraction") { 
        set_mini_batch_fraction(val.get<double>());
        msg += "mini batch fraction: " + val.tt + "; ";
      }
      else if(param == "hist_size") {              
        set_hist_size(val.get<int>()); 
        msg += "hist_size: " + val.tt + "; ";
      }
      else if(param == "tol") { 
        set_tol(val.get<double>());
        msg += "tol: " + val.tt + "; ";
      }
      else if(param == "reg_param") { 
        set_reg_param(val.get<double>());
        msg += "reg param: " + val.tt + "; ";
      }
      else if(param == "solver") {
        set_solver(val.get<std::string>());
        msg += "solver: " + val.tt + "; ";
      }
      else if(param == "fit_intercept") { 
        set_intercept(val.get<bool>());
        msg += "fit intercept: " + val.tt + "; ";
      }
      else if(param == "warm_start") {
        set_warm_start(val.get<bool>());
        msg += "warm_start: " + val.tt + "; ";
      }
      else REPORT_ERROR(USER_ERROR, "[lasso_regression] Unknown parameter: '" 
                                     + param + "' is encountered!\n");
    }
    RLOG(DEBUG) << msg << "\n";
    return *this;
  }

  template <class MATRIX>
  lasso_regression&
  fit(MATRIX&& mat, dvector<T>& label) {
    std::vector<T> sample_weight;
    return fit(std::move(mat), label, sample_weight);
  }

  template <class MATRIX>
  lasso_regression&
  fit(MATRIX&& mat, dvector<T>& label,
      std::vector<T>& sample_weight) {
    return _fit(mat, label, sample_weight, true);
  }

   template <class MATRIX>
  lasso_regression&
  fit(const MATRIX& mat, dvector<T>& label) {
    std::vector<T> sample_weight;
    return fit(mat, label, sample_weight);
  }

  template <class MATRIX>
  lasso_regression&
  fit(const MATRIX& mat, dvector<T>& label,
      std::vector<T>& sample_weight) {
    return _fit(mat, label, sample_weight, false);
  }

  // MATRIX: can accept both rowmajor and colmajor matrices as for dense data; 
  //         and crs matrix as for sparse data
  template <class MATRIX>
  lasso_regression&
  _fit(MATRIX& mat, dvector<T>& label,
       std::vector<T>& sample_weight,
       bool input_movable) {
    size_t nfeatures = mat.num_col;
    if(!(warm_start && is_fitted)) {
      T intercept = fit_intercept ? 1.0 : 0.0;
      model = linear_regression_model<T>(nfeatures, intercept);
    }
    else {
      require(nfeatures == n_features_,
      "Fitted model dimension does not match with number of features in input data\n");
    }
    size_t n_iter;
    if (solver == "sgd") {
      model = lasso_with_sgd::train(
                      mat, label, model,
                      sample_weight, n_iter, max_iter, alpha, mbf,
                      reg_param, fit_intercept, tol,
                      mat_type, input_movable);
    }
    else if (solver == "lbfgs") {
      model = lasso_with_lbfgs::train(
                      mat, label, model, 
                      sample_weight, n_iter, max_iter, alpha, hist_size,
                      reg_param, fit_intercept, tol,
                      mat_type, input_movable);
    }
    else REPORT_ERROR(USER_ERROR, "Unknown solver is encountered!\n");
    this->is_fitted = true;
    this->n_iter_ = n_iter;
    this->n_features_ = nfeatures;
    return *this;
  }

  template <class MATRIX>
  std::vector<T> 
  predict(MATRIX& mat) {
    if(!is_fitted) REPORT_ERROR(USER_ERROR, 
                   "[lasso_regression] predict is called before fit\n");
    return mat.data.map(parallel_predict<typename MATRIX::local_mat_type,
                        linear_regression_model<T>, T>,
                        broadcast(model))
                   .template moveto_dvector<T>()
                   .gather();
  }

  template <class LOC_MATRIX>
  std::vector<T> 
  predict_local(LOC_MATRIX& mat) {
    if(!is_fitted) REPORT_ERROR(USER_ERROR, 
                   "[lasso_regression] predict is called before fit\n");
    return model.predict(mat);
  }

  template <class MATRIX>
  float score(MATRIX& mat, dvector<T>& label) {
    if(!is_fitted) REPORT_ERROR(USER_ERROR, 
                   "[lasso_regression] score is called before fit\n");
    auto pred_label = predict(mat);
    return r2_score(pred_label, label.gather());
  }

  size_t get_num_features() {
    return model.get_num_features();
  }

  std::vector<T> get_intercept() {
    return std::vector<T>({model.intercept});
  }

  std::vector<T> get_weight() {
    return model.weight;
  }

  void debug_print() {
    model.debug_print();
  }

  void savebinary(const std::string &inputPath) {
    model.savebinary(inputPath);
  }

  void save(const std::string &inputPath) {
    model.save(inputPath);
  }

  lasso_regression& loadbinary(const std::string &inputPath) {
    model.loadbinary(inputPath);
    n_features_ = model.get_num_features();
    is_fitted = true;
    return *this;
  }

  lasso_regression& load(const std::string &inputPath) {
    model.load(inputPath);
    n_features_ = model.get_num_features();
    is_fitted = true;
    return *this;
  }

  int max_iter, hist_size;
  double alpha, mbf, tol, reg_param;
  std::string solver;
  bool fit_intercept;
  linear_regression_model<T> model;
  bool is_fitted;
  size_t n_iter_;
  bool warm_start;
  size_t n_features_;
  MatType mat_type;
  SERIALIZE(max_iter, hist_size, alpha, mbf, tol, 
            reg_param, solver, fit_intercept, 
            model, is_fitted, n_iter_,
            warm_start, n_features_, mat_type); 
};

}
#endif
