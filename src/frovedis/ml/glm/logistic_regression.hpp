#ifndef __LOG_REG__
#define __LOG_REG__

#include <frovedis/ml/metrics.hpp>
#include <frovedis/ml/model_selection/param.hpp>
#include <frovedis/ml/glm/logistic_regression_with_sgd.hpp>
#include <frovedis/ml/glm/shrink_logistic_regression_with_sgd.hpp>
#include <frovedis/ml/glm/logistic_regression_with_lbfgs.hpp>
#include <frovedis/ml/glm/multinomial_logistic_regression.hpp>

namespace frovedis {

template <class T>
struct logistic_regression {
  logistic_regression(int max_iter = 1000,
                      double alpha = 0.01,
                      const std::string& solver = "sgd",
                      double reg_param = 0.01,
                      const std::string& reg_type= "ZERO",
                      bool fit_intercept = false,
                      double mini_batch_fraction = 1.0,
                      int hs = 10,
                      double convergence_tol = 0.001,
                      bool is_mult = false,
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
    this->reg_type = reg_type;
    this->fit_intercept = fit_intercept;
    this->mbf = mini_batch_fraction;
    this->hist_size = hs;
    this->tol = convergence_tol;
    this->is_mult = is_mult;
    this->warm_start = warm_start;
    this->mat_type = mType;
    this->is_fitted = false;
    this->n_iter_ = 0;
    this->n_features_ = 0;
    this->n_classes_ = 0;
  }
  logistic_regression<T>& 
  set_max_iter(int max_iter) {
    std::string msg = "expected a positive max_iter; received: " + STR(max_iter) + "\n";
    require(max_iter > 0, msg);
    this->max_iter = max_iter;
    return *this;  
  }
  logistic_regression<T>& 
  set_alpha(double alpha) {
    std::string msg = "expected a positive alpha (learning rate); received: " + STR(alpha) + "\n";
    require(alpha > 0, msg);
    this->alpha = alpha;
    return *this;  
  }
  logistic_regression<T>& 
  set_solver(const std::string& solver) {
    std::string msg = "expected sgd, shrink-sgd or lbfgs; received: " + solver + "\n";
    require(solver == "sgd" || solver == "shrink-sgd" || solver == "lbfgs", msg);
    this->solver = solver;
    return *this;  
  }
  logistic_regression<T>& 
  set_reg_param(double reg_param) {
    std::string msg = "expected a positive regularization parameter; received: " + STR(reg_param) + "\n";
    require(reg_param > 0, msg);
    this->reg_param = reg_param;
    return *this;  
  }
  logistic_regression<T>& 
  set_reg_type(const std::string& reg_type) {
    std::string msg = "expected ZERO, L1 or L2; received: " + reg_type + "\n";
    require(reg_type == "ZERO" || reg_type == "L1" || reg_type == "L2", msg);
    this->reg_type = reg_type;
    return *this;  
  }
  logistic_regression<T>& 
  set_intercept(bool is_intercept) {
    this->fit_intercept = is_intercept;
    return *this;  
  }
  logistic_regression<T>& 
  set_mini_batch_fraction(double mbf) {
    std::string msg = "expected a positive mini-batch fraction; received: " + STR(mbf) + "\n";
    require(mbf > 0, msg);
    this->mbf = mbf;
    return *this;  
  }
  logistic_regression<T>& 
  set_hist_size(int hs) {
    std::string msg = "expected a positive history size; received: " + STR(hs) + "\n";
    require(hs > 0, msg);
    this->hist_size = hs;
    return *this;  
  }
  logistic_regression<T>& 
  set_tol(double tol) {
    std::string msg = "expected a positive convergence tolerance; received: " + STR(tol) + "\n";
    require(tol > 0, msg);
    this->tol = tol;
    return *this;  
  }
  logistic_regression<T>& 
  set_is_mult(bool is_mult) {
    this->is_mult = is_mult;
    return *this;  
  }
  logistic_regression<T>&
  set_warm_start(bool warm_start) {
    this->warm_start = warm_start;
    return *this;
  }

  logistic_regression<T>&
  set_params(glm_config& config) {
    set_max_iter(config.numIteration);
    set_alpha(config.alpha);
    set_solver(config.solver);
    set_reg_param(config.regParam);
    set_reg_type(config.regType);
    set_intercept(config.isIntercept);
    set_mini_batch_fraction(config.miniBatchFraction);
    set_hist_size(config.histSize);
    set_tol(config.convergenceTol);
    set_is_mult(config.isMult);
    set_warm_start(config.warmStart);
    return *this;
  }

  // frovedis::grid_sdearch_cv compatible setter
  logistic_regression<T>& 
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
      else if(param == "reg_type") {
        set_reg_type(val.get<std::string>());
        msg += "reg type: " + val.tt + "; ";
      }
      else if(param == "solver") {
        set_solver(val.get<std::string>());
        msg += "solver: " + val.tt + "; ";
      }
      else if(param == "fit_intercept") { 
        set_intercept(val.get<bool>());
        msg += "fit intercept: " + val.tt + "; ";
      }
      else if(param == "is_mult") { 
        set_is_mult(val.get<bool>());
        msg += "is_mult: " + val.tt + "; ";
      }
      else if(param == "warm_start") {
        set_warm_start(val.get<bool>());
        msg += "warm_start: " + val.tt + "; ";
      }
      else REPORT_ERROR(USER_ERROR, "[logistic_regression] Unknown parameter: '" 
                                     + param + "' is encountered!\n");
    }
    RLOG(DEBUG) << msg << "\n";
    return *this;
  }

  RegType get_regularizer() {
    RegType regularizer = ZERO;
    if (reg_type == "ZERO") regularizer = ZERO;
    else if (reg_type == "L1") regularizer = L1;
    else if (reg_type == "L2") regularizer = L2;
    else REPORT_ERROR(USER_ERROR, "[logistic_regression] Unknown regularizer '" 
                                   + reg_type + "' is encountered!\n");
    return regularizer;
  }

  template <class MATRIX>
  logistic_regression& 
  fit(MATRIX&& mat, dvector<T>& label) {
    std::vector<T> sample_weight;
    return fit(std::move(mat), label, sample_weight);
  }

  template <class MATRIX>
  logistic_regression& 
  fit(MATRIX&& mat, dvector<T>& label,
      std::vector<T>& sample_weight) {
    return _fit(mat, label, sample_weight, true);
  }

   template <class MATRIX>
  logistic_regression& 
  fit(const MATRIX& mat, dvector<T>& label) {
    std::vector<T> sample_weight;
    return fit(mat, label, sample_weight);
  }

  template <class MATRIX>
  logistic_regression& 
  fit(const MATRIX& mat, dvector<T>& label,
      std::vector<T>& sample_weight) {
    return _fit(mat, label, sample_weight, false);
  }

  // MATRIX: can accept both rowmajor and colmajor matrices as for dense data; 
  //         and crs matrix as for sparse data
  template <class MATRIX>
  logistic_regression& 
  _fit(MATRIX& mat, dvector<T>& label,
      std::vector<T>& sample_weight, bool input_movable) {
     size_t nclasses = compute_nclasses(label);
     // multinomial problem detected, implicitly setting is_mult as true
     if (nclasses > 2) is_mult = true;
     size_t nfeatures = mat.num_col;
     if(!(warm_start && is_fitted)) {
       if(is_mult) {
        model_mult = multinomial_logistic_regression_model<T>
                           (nfeatures,nclasses,fit_intercept);
      }
      else {
        T intercept = fit_intercept ? 1.0 : 0.0;
        model_bin = logistic_regression_model<T>(nfeatures, intercept);
      }
     }
     else {
       require(nfeatures == n_features_, 
       "Fitted model dimension does not match with number of features in input data\n");
       require(nclasses == n_classes_, 
       "Fitted model dimension does not match with number of labels in input data\n");
     }
     size_t n_iter = 0;
     if(is_mult) {
       if(solver == "sgd") {
         model_mult = multinomial_logistic_regression::train(
                          mat, label, model_mult,
                          sample_weight, n_iter, max_iter, alpha,
                          reg_param, get_regularizer(), 
                          fit_intercept, tol, mat_type, input_movable);
       }
       else REPORT_ERROR(USER_ERROR, 
            "Multinomil classification is only supported for sgd solver\n");
     }
     else {
       if (solver == "sgd") {
         model_bin = logistic_regression_with_sgd::train(
                           mat, label, model_bin,
                           sample_weight, n_iter, max_iter, alpha, mbf,
                           reg_param, get_regularizer(), 
                           fit_intercept, tol, mat_type, input_movable);
       }
       else if (solver == "shrink-sgd") {
         model_bin = shrink::logistic_regression_with_sgd::train(
                           mat, label, model_bin,
                           sample_weight, n_iter, max_iter, alpha, mbf,
                           reg_param, get_regularizer(), 
                           fit_intercept, tol, mat_type, input_movable);
       }
       else if (solver == "lbfgs") {
         model_bin = logistic_regression_with_lbfgs::train(
                           mat, label, model_bin,
                           sample_weight, n_iter, max_iter, alpha, hist_size,
                           reg_param, get_regularizer(), 
                           fit_intercept, tol, mat_type, input_movable);
       }
       else REPORT_ERROR(USER_ERROR, "Unknown solver is encountered!\n");
     }
     this->is_fitted = true;
     this->n_iter_ = n_iter;
     this->n_features_ = nfeatures;
     this->n_classes_ = nclasses;
     return *this;
   }

  template <class MATRIX>
  std::vector<T> 
  predict(MATRIX& mat) {
    if(!is_fitted) REPORT_ERROR(USER_ERROR, 
                   "[logistic_regression] predict is called before fit\n");
    if(is_mult) {
      return mat.data.map(parallel_predict<typename MATRIX::local_mat_type,
                        multinomial_logistic_regression_model<T>, T>,
                        model_mult.broadcast())
                     .template moveto_dvector<T>()
                     .gather();
    }
    else {
      return mat.data.map(parallel_predict<typename MATRIX::local_mat_type,
                        logistic_regression_model<T>, T>,
                        model_bin.broadcast())
                     .template moveto_dvector<T>()
                     .gather();
    }
  }

  template <class LOC_MATRIX>
  std::vector<T> 
  predict_local(LOC_MATRIX& mat) {
    if(!is_fitted) REPORT_ERROR(USER_ERROR, 
                   "[logistic_regression] predict is called before fit\n");
    if(is_mult) return model_mult.predict(mat);
    else        return model_bin.predict(mat);
  }

  template <class MATRIX>
  float score(MATRIX& mat, dvector<T>& label) { 
    if(!is_fitted) REPORT_ERROR(USER_ERROR, 
                   "[logistic_regression] score is called before fit\n");
    auto pred_label = predict(mat);
    return accuracy_score(pred_label, label.gather());
  }

  template <class MATRIX>
  std::vector<T>
  predict_probability(MATRIX& mat, bool use_score=false) {
    if(!is_fitted) REPORT_ERROR(USER_ERROR,
      "[logistic_regression] predict_probability is called before fit\n");
    if(is_mult) {
      return mat.data.map(parallel_predict_probability
                        <typename MATRIX::local_mat_type,
                        multinomial_logistic_regression_model<T>, T>,
                        model_mult.broadcast(), broadcast(use_score))
                     .template moveto_dvector<T>()
                     .gather();
    }
    else {
      return mat.data.map(parallel_predict_probability
                        <typename MATRIX::local_mat_type,
                        logistic_regression_model<T>, T>,
                        model_bin.broadcast(), broadcast(use_score))
                     .template moveto_dvector<T>()
                     .gather();
    }
  }

  template <class MATRIX>
  rowmajor_matrix_local<T>
  compute_probability_matrix(MATRIX& mat) {
    if(!is_fitted) REPORT_ERROR(USER_ERROR,
      "[logistic_regression] compute_probability_matrix is called before fit\n");
    if(is_mult) {
      return merge(mat.data.map(parallel_compute_probability_matrix
                        <typename MATRIX::local_mat_type,
                        multinomial_logistic_regression_model<T>, T>,
                        model_mult.broadcast())
                      .gather());
    }
    else {
      return merge(mat.data.map(parallel_compute_probability_matrix
                        <typename MATRIX::local_mat_type,
                        logistic_regression_model<T>, T>,
                        model_bin.broadcast())
                      .gather());
    }
  }

  void set_threshold(T thr) {
    is_mult ? model_mult.set_threshold(thr) : model_bin.set_threshold(thr);
  }

  size_t get_num_features() {
    return is_mult ? model_mult.get_num_features() 
                   : model_bin.get_num_features();
  }

  size_t get_num_classes() {
    return is_mult ? model_mult.get_num_classes() 
                   : model_bin.get_num_classes();
  }

  T get_threshold() {
    return is_mult ? model_mult.get_threshold() : model_bin.get_threshold();
  }

  std::vector<T> get_intercept() {
    return is_mult ? model_mult.intercept 
                   : std::vector<T>({model_bin.intercept});
  }
  
  std::vector<T> get_weight() {
    return is_mult ? model_mult.weight.val : model_bin.weight;
  }
 
  void debug_print() {
    is_mult ? model_mult.debug_print() : model_bin.debug_print();
  }

  void savebinary(const std::string &inputPath) {
    make_directory(inputPath);
    std::string model_file = inputPath + "/model";
    is_mult ? model_mult.savebinary(model_file) 
            : model_bin.savebinary(model_file);
    std::string type_file = inputPath + "/type";
    std::ofstream type_str;
    type_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    type_str.open(type_file.c_str()); 
    std::string model_type = is_mult ? "multinomial" : "binomial";
    type_str << model_type << std::endl;
  }

  void save(const std::string &inputPath) {
    make_directory(inputPath);
    std::string model_file = inputPath + "/model";
    is_mult ? model_mult.save(model_file) 
            : model_bin.save(model_file);
    std::string type_file = inputPath + "/type";
    std::ofstream type_str;
    type_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    type_str.open(type_file.c_str()); 
    std::string model_type = is_mult ? "multinomial" : "binomial";
    type_str << model_type << std::endl;
  }

  logistic_regression& loadbinary(const std::string &inputPath) {
    std::string type_file = inputPath + "/type";
    std::string model_type;
    std::ifstream type_str(type_file.c_str()); type_str >> model_type;
    is_mult = (model_type == "multinomial");
    std::string model_file = inputPath + "/model";
    is_mult ? model_mult.loadbinary(model_file)
            : model_bin.loadbinary(model_file);
    n_classes_ = get_num_classes();
    n_features_ = get_num_features();
    is_fitted = true;
    return *this;
  }

  logistic_regression& load(const std::string &inputPath) {
    std::string type_file = inputPath + "/type";
    std::string model_type;
    std::ifstream type_str(type_file.c_str()); type_str >> model_type;
    is_mult = (model_type == "multinomial");
    std::string model_file = inputPath + "/model";
    is_mult ? model_mult.load(model_file)
            : model_bin.load(model_file);
    n_classes_ = get_num_classes();
    n_features_ = get_num_features();
    is_fitted = true;
    return *this;
  }

  int max_iter, hist_size;
  double alpha, mbf, tol, reg_param;
  std::string reg_type, solver;
  bool fit_intercept;
  logistic_regression_model<T> model_bin;
  multinomial_logistic_regression_model<T> model_mult;
  bool is_fitted;
  size_t n_iter_;
  bool is_mult;
  bool warm_start;
  size_t n_features_;
  size_t n_classes_;
  MatType mat_type;
  SERIALIZE(max_iter, hist_size, alpha, mbf, tol, 
            reg_param, reg_type, solver,
            fit_intercept, model_bin, model_mult, 
            is_fitted, n_iter_, is_mult, warm_start,
            n_features_, n_classes_, mat_type); 
};

}
#endif
