#ifndef __GMM_HPP__
#define __GMM_HPP__

#include <frovedis/ml/model_selection/param.hpp>
#include <frovedis/ml/clustering/gmm_impl.hpp>


namespace frovedis {
    
  template <class T> 
  struct gaussian_mixture {
    gaussian_mixture(int n_components = 1,
                     const std::string& covariance_type = "full",
                     T tol = 0.001,
                     int max_iter = 100,
                     const std::string& init_params = "kmeans",
                     long random_state = 0) {
      this->n_components = n_components;
      this->covariance_type = covariance_type;
      this->tol = tol;
      this->max_iter = max_iter;
      this->init_params = init_params;
      this->random_state = random_state;      
      this->is_fitted = false;                            
    }

    gaussian_mixture<T>& 
    set_n_components(int n_components) {
      std::string msg = "expected a positive n_components; received: " 
                         + STR(n_components) + "\n";
      require(n_components > 0, msg);
      this->n_components = n_components;  
      return *this;
    }

    gaussian_mixture<T>& 
    set_covariance_type(const std::string& cov_type) {
      std::string msg = "Frovedis GMM only supports \"full\" covariance_type;\
                         received: " + cov_type + "\n";
      require(cov_type == "full", msg);
      this->covariance_type = covariance_type;  
      return *this;
    }

    gaussian_mixture<T>& 
    set_tol(T tol) { // tol >= 0.0 && tol <= 1.0
      std::string msg = "expected tol within the range of 0.0 to 1.0; received: " 
                         + STR(tol) + "\n";
      require(tol >= 0.0 && tol <= 1.0, msg);
      this->tol = tol;
      return *this;
    }

    gaussian_mixture<T>&
    set_max_iter(int max_iter) {
      std::string msg = "expected a positive max_iter; received: " + STR(max_iter) + "\n";
      require(max_iter > 0, msg);
      this->max_iter = max_iter;
      return *this;
    }

    gaussian_mixture<T>& 
    set_init_params(const std::string& init_params) {
      std::string msg = "init_params can be either \"kmeans\" or \"random\";\
                         received: " + init_params + "\n";
      require(init_params == "kmeans" || init_params == "random", msg);
      this->init_params = init_params;  
      return *this;
    }

    gaussian_mixture<T>& 
    set_random_state(long random_state) { 
      std::string msg = "expected a zero or positive value for seed; received: " 
                         + STR(random_state) + "\n";
      require(random_state >= 0, msg);
      this->random_state = random_state;
      return *this;
    }    

    gaussian_mixture<T>& 
    set_params(std::vector<std::pair<std::string, param_t>>& data) {
      std::string msg = "";
      for (auto& e: data) {
        auto param = e.first;
        auto val = e.second;
        if(param == "n_components") {
          set_n_components(val.get<int>());
          msg += "n_components: " + val.tt + "; ";
        }
        else if(param == "covariance_type") {
          set_covariance_type(val.get<std::string>());
          msg += "covariance_type: " + val.tt + "; ";
        }
        else if(param == "tol") {
          set_tol(val.get<T>());
          msg += "tol: " + val.tt + "; ";
        }
        else if(param == "max_iter") {
          set_max_iter(val.get<int>());
          msg += "max_iter: " + val.tt + "; ";
        }
        else if(param == "init_params") {
          set_init_params(val.get<std::string>());
          msg += "init_params: " + val.tt + "; ";
        }
        else if(param == "random_state") {
          set_random_state(val.get<long>());
          msg += "random_state: " + val.tt + "; ";
        }        
        else REPORT_ERROR(USER_ERROR, "[gaussian_mixture] Unknown parameter: '"
                          + param + "' is encountered!\n");
      }
      RLOG(DEBUG) << msg << "\n";
      return *this;
    }

    template <class MATRIX>
    gaussian_mixture<T>& 
    fit(const MATRIX& mat,
        const dvector<int>& label = dvector<int>()) { // ignored      
      bool switch_init = 0;
      if(init_params == "kmeans")
        switch_init = 0;            
      else if(init_params == "random")
        switch_init = 1;
      else REPORT_ERROR(USER_ERROR, "Unknown init_params type provided: '"
                        + init_params + "\n");        
      model = frovedis::gmm(mat, n_components, max_iter, tol, switch_init, random_state);      
      is_fitted = true;       
      return *this; 
    }
    
    template <class MATRIX>
    std::vector<int>
    fit_predict(const MATRIX& mat,
                const dvector<int>& label = dvector<int>()) { // ignored
      fit(mat, label);
      auto mat_local = mat.gather();  
      auto gmm_cluster = gmm_assign_cluster(mat_local, n_components, 
                                            model.model, model.covariance, 
                                            model.pi);
      return gmm_cluster.predict.val;        
    }

    template <class MATRIX>
    std::vector<int> 
    predict(MATRIX& mat) {
      if(!is_fitted) REPORT_ERROR(USER_ERROR,
       "[Gaussian Mixture] prediction is called before fit\n");
      auto mat_local = mat.gather();  
      auto gmm_pred = gmm_assign_cluster(mat_local, n_components, 
                                            model.model, model.covariance, 
                                            model.pi);           
      return gmm_pred.predict.val;
    }

    template <class MATRIX>
    rowmajor_matrix_local<T> 
    predict_proba(MATRIX& mat) { 
      if(!is_fitted) REPORT_ERROR(USER_ERROR,
       "[Gaussian Mixture] prediction is called before fit\n");
      auto mat_local = mat.gather();   
      auto gmm_pred = gmm_assign_cluster(mat_local, n_components, 
                                            model.model, model.covariance, 
                                            model.pi);  
      return gmm_pred.predict_prob;          
    }
  
    rowmajor_matrix_local<T> weights_() {
      require(is_fitted, "weights_() is called before fit()");
      return model.pi;  
    }

    rowmajor_matrix_local<T> means_() {
      require(is_fitted, "means_() is called before fit()");
      return model.model;  
    }

    rowmajor_matrix_local<T> covariances_() {
      require(is_fitted, "covariances_() is called before fit()");
      return model.covariance;  
    }

    int n_iter_() {
      require(is_fitted, "n_iter_() is called before fit()");
      return model.loops;      
    }    
 
    bool converged_() {
      require(is_fitted, "converged_() is called before fit()");
      return model.converged;      
    }

    T lower_bound_() {
      require(is_fitted, "lower_bound_() is called before fit()"); 
      return model.likelihood;
    }
      
    void debug_print() {
      std::cout << "Gaussian Mixture Model: \n";  
      model.debug_print();     
    }

    void savebinary(const std::string& fname) { 
      model.savebinary(fname);
      // TODO: save other metadata      
    }

    void save(const std::string& fname) { 
      model.save(fname); 
      // TODO: save other metadata
    }

    void loadbinary(const std::string& fname) {
      model.loadbinary(fname);    
      is_fitted = true;
      // TODO: load other metadata
    }

    void load(const std::string& fname) {
      model.load(fname);        
      is_fitted = true;      
      // TODO: load other metadata
    }

    private:
      int n_components;
      std::string covariance_type;
      T tol;
      int max_iter;
      std::string init_params;
      long random_state;      
      bool is_fitted;
      gmm_model<T> model;
      SERIALIZE(n_components, covariance_type, tol, max_iter,
                init_params, random_state, is_fitted, model);    
  };
}

#endif  
