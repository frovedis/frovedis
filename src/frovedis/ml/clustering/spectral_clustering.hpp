#ifndef _SPECTRAL_CLUSTER_
#define _SPECTRAL_CLUSTER_

#include <frovedis/ml/model_selection/param.hpp>
#include <frovedis/ml/clustering/spectral_clustering_impl.hpp>

namespace frovedis {
    
  template <class T>
  struct spectral_clustering { 
    spectral_clustering(int n_clusters = 2,
                        int n_comp = 2,
                        int n_iter = 300,
                        double gamma = 1.0,
                        double eps = 0.1,
                        bool norm_laplacian = true,
                        bool precomputed = false,
                        bool drop_first = false,
                        int mode = 1) {

      this->n_clusters = n_clusters;
      this->n_comp = n_comp;
      this->n_iter = n_iter;
      this->gamma = gamma;
      this->eps = eps;
      this->norm_laplacian = norm_laplacian;
      this->precomputed = precomputed;
      this->drop_first = drop_first;
      this->mode = mode;
      this->is_fitted = false;
    }   

    spectral_clustering<T>& 
    set_n_clusters(int n_clusters) {
      std::string msg = "expected a positive n_clusters; received: " +
                         STR(n_clusters) + "\n";
      require(n_clusters > 0, msg);
      this->n_clusters = n_clusters;  
      return *this;
    }

    spectral_clustering<T>& 
    set_n_comp(int n_comp) {
      std::string msg = "expected a positive n_comp; received: " + STR(n_comp) + "\n";
      require(n_comp > 0, msg);
      this->n_comp = n_comp;    
      return *this;
    }

    spectral_clustering<T>& 
    set_n_iter(int n_iter) {
      std::string msg = "expected a positive n_iter; received: " + STR(n_iter) + "\n";
      require(n_iter > 0, msg);
      this->n_iter = n_iter;    
      return *this;
    }
    
    spectral_clustering<T>& 
    set_gamma(double gamma) {
      this->gamma = gamma;      
      return *this;
    }
    
    spectral_clustering<T>& 
    set_eps(double eps) {
      std::string msg = "expected eps within the range of 0.0 to 1.0; received: " +
                         STR(eps) + "\n";
      require(eps >= 0.0 && eps <= 1.0, msg);
      this->eps = eps;
      return *this;
    }

    spectral_clustering<T>& 
    set_norm_laplacian(bool norm_laplacian) {
      this->norm_laplacian = norm_laplacian;    
      return *this;
    }

    spectral_clustering<T>& 
    set_precomputed(bool precomputed) {
      this->precomputed = precomputed;      
      return *this;
    }
    
    spectral_clustering<T>& 
    set_drop_first(bool drop_first) {
      this->drop_first = drop_first;    
      return *this;
    }

    spectral_clustering<T>& 
    set_mode(int mode) {
      std::string msg = "supported modes are either 1 or 3; received: " +
                         STR(mode) + "\n";
      require(mode == 1 || mode == 3, msg);
      this->mode = mode;    
      return *this;
    } 

    spectral_clustering<T>& 
    set_params(std::vector<std::pair<std::string, param_t>>& data) {
      std::string msg = "";
      for (auto& e: data) {
        auto param = e.first;
        auto val = e.second;
        if(param == "n_clusters") {
          set_n_clusters(val.get<int>());
          msg += "n_clusters: " + val.tt + "; ";
        }
        else if(param == "n_comp") {
          set_n_comp(val.get<int>());
          msg += "n_comp: " + val.tt + "; ";
        }
        else if(param == "n_iter") {
          set_n_iter(val.get<int>());
          msg += "n_iter: " + val.tt + "; ";
        }       
        else if(param == "eps") {
          set_eps(val.get<double>());
          msg += "eps: " + val.tt + "; ";
        }
        else if(param == "gamma") {
          set_gamma(val.get<double>());
          msg += "gamma: " + val.tt + "; ";
        }
        else if(param == "norm_laplacian") {
          set_norm_laplacian(val.get<bool>());
          msg += "norm_laplacian: " + val.tt + "; ";
        }
        else if(param == "precomputed") {
          set_precomputed(val.get<bool>());
          msg += "precomputed: " + val.tt + "; ";
        }
        else if(param == "drop_first") {
          set_drop_first(val.get<bool>());
          msg += "drop_first: " + val.tt + "; ";
        }
        else if(param == "mode") {
          set_mode(val.get<int>());
          msg += "mode: " + val.tt + "; ";
        }        
        else REPORT_ERROR(USER_ERROR, "[spectral_clustering] Unknown parameter: '"
                          + param + "' is encountered!\n");
      }
      RLOG(DEBUG) << msg << "\n";
      return *this;
    }

    template <class MATRIX>
    spectral_clustering<T>& 
    fit(const MATRIX& mat,
      const dvector<int>& label = dvector<int>()) { // ignored
      bool movable = false;
      model = spectral_clustering_impl(mat, n_clusters, n_comp, n_iter, eps,
                norm_laplacian, precomputed, drop_first, gamma, mode, movable);
      is_fitted = true;
      return *this;           
    }

    template <class MATRIX>
    spectral_clustering<T>& 
    fit(MATRIX&& mat,
      const dvector<int>& label = dvector<int>()) { // ignored
      bool movable = true;
      model = spectral_clustering_impl(mat, n_clusters, n_comp, n_iter, eps,
                norm_laplacian, precomputed, drop_first, gamma, mode, movable);
      is_fitted = true;
      return *this;           
    }   

    template <class MATRIX>
    std::vector<int>
    fit_predict(const MATRIX& mat,
      const dvector<int>& label = dvector<int>()) { // ignored
      fit(mat, label);
      return model.labels;
    }

    template <class MATRIX>
    std::vector<int>
    fit_predict(MATRIX&& mat,
      const dvector<int>& label = dvector<int>()) { // ignored
      fit(std::move(mat), label);
      return model.labels;
    }
        
     template <class MATRIX>
     float score(const MATRIX& mat, dvector<int>& true_label) {
       auto pred_label = fit_predict(mat);
       return homogeneity_score(true_label.gather(), pred_label);
     }

     template <class MATRIX>
     float score(MATRIX&& mat, dvector<int>& true_label) {
       auto pred_label = fit_predict(std::move(mat));
       return homogeneity_score(true_label.gather(), pred_label);
     }      

    rowmajor_matrix<T> affinity_matrix_() {
      require(is_fitted, "affinity_matrix_() is called before fit()");       
      return model.affinity_matrix; 
    } 
    
    std::vector<int> labels_() {
      require(is_fitted, "labels_() is called before fit()");              
      return model.labels; 
    }


    void debug_print(size_t limit = 0) {
      std::cout << "spectral clustering model: \n";
      model.debug_print(limit);
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
      int n_clusters;
      int n_comp;
      int n_iter;
      double eps;
      bool norm_laplacian, precomputed;
      bool drop_first, is_fitted;
      double gamma;
      int mode;
      spectral_clustering_model<T> model;    
      SERIALIZE(n_clusters, n_comp, n_iter, eps, norm_laplacian, 
                 precomputed, drop_first, gamma, mode, model)     
      typedef int predict_type;
  };
}

#endif
