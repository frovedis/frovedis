#ifndef DBSCAN_HPP
#define DBSCAN_HPP

#include <frovedis/ml/model_selection/param.hpp>
#include "frovedis/ml/clustering/dbscan_impl.hpp"

namespace frovedis {

template <typename T>  
struct dbscan { 
  dbscan(double eps = 0.5, int min_pts = 5, 
         double batch_fraction = std::numeric_limits<double>::max()): 
    eps(eps), min_pts(min_pts), batch_fraction(batch_fraction), is_fitted(false) {}

  dbscan<T>& 
  set_eps(double eps) { 
    std::string msg = "expected eps within the range of 0.0 to 1.0; received: " + 
                       STR(eps) + "\n";
    require(eps >= 0.0 && eps <= 1.0, msg);
    this->eps = eps;
    return *this;
  }

  dbscan<T>&
  set_min_pts(int min_pts) {
    std::string msg = "min_pts should be a positive value \n";  
    require(min_pts > 0, msg);  
    this->min_pts = min_pts;
    return *this;
  }

  dbscan<T>&
  set_batch_fraction(double batch_fraction) {
    std::string msg = "expected batch_fraction within the range of 0.0 to 1.0; received: " + 
                       STR(batch_fraction) + "\n";
    require(batch_fraction >= 0.0 && batch_fraction <= 1.0, msg);
    this->batch_fraction = batch_fraction;
    return *this;
  }    
    
  dbscan<T>& 
  set_params(std::vector<std::pair<std::string, param_t>>& data) {
    std::string msg = "";
    for (auto& e: data) {
      auto param = e.first;
      auto val = e.second;
      if(param == "eps") {
        set_eps(val.get<double>());
        msg += "eps: " + val.tt + "; ";
      }
      else if(param == "min_pts") {
        set_min_pts(val.get<int>());
        msg += "seed: " + val.tt + "; ";
      }
      else if(param == "batch_fraction") {
        set_batch_fraction(val.get<double>());
        msg += "batch_fraction: " + val.tt + "; ";
      }        
      else REPORT_ERROR(USER_ERROR, "[DBSCAN] Unknown parameter: '"
                        + param + "' is encountered!\n");
    }
    RLOG(DEBUG) << msg << "\n";
    return *this;
  }

  template <class MATRIX>    
  dbscan<T>& 
  fit(MATRIX& mat, 
      const dvector<int>& label = dvector<int>()) { //ignored
    est = dbscan_impl<T>(eps, min_pts, batch_fraction);
    est.fit(mat);
    is_fitted = true;       
    return *this;
  }
    
  template <class MATRIX>    
  dbscan<T>& 
  fit(MATRIX& mat,
      std::vector<T>& sample_weight, 
      const dvector<int>& label = dvector<int>()) { //ignored
    est = dbscan_impl<T>(eps, min_pts, batch_fraction);
    est.fit(mat, sample_weight);
    is_fitted = true;       
    return *this;
  }

  template <class MATRIX>    
  std::vector<int> 
  fit_predict(MATRIX& mat, 
             const dvector<int>& label = dvector<int>()) { //ignored
    fit(mat, label);     
    return est.labels();
  }   

  template <class MATRIX>    
  std::vector<int> 
  fit_predict(MATRIX& mat, 
              std::vector<T>& sample_weight,
              const dvector<int>& label = dvector<int>()) { //ignored
    fit(mat, sample_weight, label);     
    return est.labels();
  }   

  template <class MATRIX>    
  float score(MATRIX& mat, dvector<int>& true_label) {
    auto pred_label = fit_predict(mat);
    return homogeneity_score(true_label.gather(), pred_label);
  }
    
  std::vector<int> labels() {
    require(is_fitted, "labels_() is called before fit()");
    return est.labels(); 
  }

  std::vector<size_t> core_sample_indices_() {
    require(is_fitted, "components__() is called before fit()");
    return est.core_sample_indices_(); 
  }

  rowmajor_matrix<T> components_() { 
    require(is_fitted, "components_() is called before fit()");
    return est.components_();
  }

  private:
    dbscan_impl<T> est;
    double eps;
    int min_pts;
    double batch_fraction;
    bool is_fitted;
    SERIALIZE(est, eps, min_pts, batch_fraction, is_fitted);
    typedef int predict_type;
};

}
#endif
