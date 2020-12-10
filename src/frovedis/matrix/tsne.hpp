#ifndef _TSNE_HPP_
#define _TSNE_HPP_

#include <frovedis/matrix/tsne_impl.hpp>
#include <frovedis/ml/model_selection/param.hpp>

namespace frovedis {

template <class T>
struct TSNE {
  TSNE(double perplexity = 30.0,
       double early_exaggeration = 12.0,
       double min_grad_norm = 1e-7,
       double learning_rate = 200.0,
       size_t n_components = 2,
       size_t n_iter = 1000,
       size_t n_iter_without_progress = 300,
       const std::string& metric="euclidean",
       bool verbose = false) {
    this->perplexity = perplexity;
    this->early_exaggeration = early_exaggeration;
    this->min_grad_norm = min_grad_norm;
    this->learning_rate = learning_rate;
    this->n_components = n_components;
    this->n_iter = n_iter;
    this->n_iter_without_progress = n_iter_without_progress;
    this->metric = metric;
    this->verbose = verbose;
    this->is_fitted = false;
  }
  TSNE<T>& 
  set_perplexity(double perplexity) {
    std::string msg = "expected a positive perplexity; received: " + STR(perplexity) + "\n";
    require(perplexity >= 0, msg);
    this->perplexity = perplexity;
    return *this;
  }
  TSNE<T>& 
  set_early_exaggeration(double early_exaggeration) {
    std::string msg = "early_exaggeration must be at least 1; received: " + STR(early_exaggeration) + "\n";
    require(early_exaggeration >= 1.0, msg);
    this->early_exaggeration = early_exaggeration;
    return *this; 
  }
  TSNE<T>& 
  set_min_grad_norm(double min_grad_norm) {
    this->min_grad_norm = min_grad_norm;
    return *this;  
  }
  TSNE<T>& 
  set_learning_rate(double learning_rate) {
    std::string msg = "expected a positive learning_rate; received: " + STR(learning_rate) + "\n";
    require(learning_rate >= 0, msg);
    this->learning_rate = learning_rate;
    return *this;
  }
  TSNE<T>& 
  set_n_components(size_t n_components) {
    std::string msg = "Currently TSNE supports n_components = 2; received: " + STR(n_components) + "\n";
    require(n_components == 2, msg);
    this->n_components = n_components;
    return *this;
  }
  TSNE<T>& 
  set_n_iter(size_t n_iter) {
    std::string msg = "n_iter should be at least 250; received: " + STR(n_iter) + "\n";
    require(n_iter >= 250, msg);
    this->n_iter = n_iter;
    return *this;  
  }
  TSNE<T>& 
  set_n_iter_without_progress(size_t n_iter_without_progress) {
    this->n_iter_without_progress = n_iter_without_progress;
    return *this;  
  }
  TSNE<T>& 
  set_metric(const std::string& metric) {
    std::string msg = "expected euclidean or precomputed; received: " + metric + "\n";
    require(metric == "euclidean" || metric == "precomputed", msg);
    this->metric = metric;
    return *this;  
  }
  TSNE<T>& 
  set_verbose(bool verbose) {
    this->verbose = verbose;
    return *this;  
  }

  // frovedis::grid_sdearch_cv compatible setter
  TSNE<T>& 
  set_params(std::vector<std::pair<std::string, param_t>>& data) {
    std::string msg = "";
    for (auto& e: data) {
      auto param = e.first;
      auto val = e.second;
      if(param == "perplexity") {              
        set_perplexity(val.get<double>()); 
        msg += "perplexity: " + val.tt + "; ";
      }
      else if(param == "early_exaggeration") { 
        set_early_exaggeration(val.get<double>());
        msg += "early_exaggeration: " + val.tt + "; ";
      }
      else if(param == "min_grad_norm") { 
        set_min_grad_norm(val.get<double>());
        msg += "min_grad_norm: " + val.tt + "; ";
      }
      else if(param == "learning_rate") {              
        set_learning_rate(val.get<double>()); 
        msg += "learning_rate: " + val.tt + "; ";
      }
      else if(param == "n_components") { 
        set_n_components(val.get<size_t>());
        msg += "n_components: " + val.tt + "; ";
      }
      else if(param == "n_iter") {
        set_n_iter(val.get<size_t>());
        msg += "n_iter: " + val.tt + "; ";
      }
      else if(param == "n_iter_without_progress") {
        set_n_iter_without_progress(val.get<size_t>());
        msg += "n_iter_without_progress: " + val.tt + "; ";
      }
      else if(param == "metric") {
        set_metric(val.get<std::string>());
        msg += "metric: " + val.tt + "; ";
      }
      else if(param == "verbose") { 
        set_verbose(val.get<bool>());
        msg += "verbose: " + val.tt + "; ";
      }
      else REPORT_ERROR(USER_ERROR, "[TSNE] Unknown parameter: '" 
                                     + param + "' is encountered!\n");
    }
    RLOG(DEBUG) << msg << "\n";
    return *this;
  }

  // Getter functions for n_iter_, kl_divergence_ and embedding_
  size_t get_n_iter_() {
    if(!is_fitted) REPORT_ERROR(USER_ERROR, 
       "[TSNE] get_n_iter_ is called before fit\n");
    return this->n_iter_;
  }
  double get_kl_divergence_() {
    if(!is_fitted) REPORT_ERROR(USER_ERROR, 
       "[TSNE] get_kl_divergence_ is called before fit\n");
    return this->kl_divergence_;
  }
  rowmajor_matrix<T> get_embedding_() {
    if(!is_fitted) REPORT_ERROR(USER_ERROR, 
       "[TSNE] get_embedding_ is called before fit\n");
    return this->embedding_;
  }

  // mat can accept only row-major matrix
  TSNE<T>& 
  fit(rowmajor_matrix<T>& mat) {
    size_t iter_cnt = 0;
    double kl_div = 0.0;
    this->embedding_ = tsne(mat, perplexity, early_exaggeration, 
                    min_grad_norm, learning_rate, n_components, 
                    n_iter, n_iter_without_progress, metric, 
                    verbose, iter_cnt, kl_div);
    this->n_iter_ = iter_cnt;
    this->kl_divergence_ = kl_div;
    this->is_fitted = true;
    return *this;
  }

  rowmajor_matrix<T> 
  fit_transform(rowmajor_matrix<T>& mat) {
    fit(mat);
    return this->embedding_;
  }

  double perplexity, early_exaggeration, min_grad_norm, learning_rate, kl_divergence_;
  size_t n_components, n_iter, n_iter_without_progress, n_iter_;
  std::string metric;
  bool verbose, is_fitted;
  rowmajor_matrix<T> embedding_;
  SERIALIZE(perplexity, early_exaggeration, min_grad_norm, learning_rate, 
            n_components, n_iter, n_iter_without_progress, metric, verbose, 
            embedding_, n_iter_, kl_divergence_, is_fitted); 
};

}
#endif
