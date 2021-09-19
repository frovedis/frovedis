#ifndef _SPECTRAL_CLUSTER_
#define _SPECTRAL_CLUSTER_

#include <frovedis/ml/model_selection/param.hpp>
#include <frovedis/ml/clustering/spectral_clustering_impl.hpp>

namespace frovedis {
    
  template <class T>
  struct spectral_clustering { 
    spectral_clustering(int ncluster = 2,
                        int n_comp = 2,
                        double gamma = 1.0,
                        const std::string& affinity = "rbf",
                        int n_neighbors = 10,
                        int max_iter = 300,
                        int n_init = 1,
                        int seed = 0,
                        double eps = 0.001,
                        bool norm_laplacian = true,
                        bool drop_first = true,
                        int mode = 3) {
      this->assign.set_k(ncluster)
                  .set_max_iter(max_iter)
                  .set_n_init(n_init)
                  .set_seed(seed)
                  .set_eps(eps);
      this->n_comp = n_comp;
      this->gamma = gamma;
      this->affinity = affinity;
      this->n_neighbors = n_neighbors;
      this->norm_laplacian = norm_laplacian;
      this->drop_first = drop_first;
      this->mode = mode;
      this->is_fitted = false;
    }   

    spectral_clustering<T>& 
    set_n_component(int n_comp) {
      std::string msg = "expected a positive n_comp; received: " + STR(n_comp) + "\n";
      require(n_comp > 0, msg);
      this->n_comp = n_comp;    
      return *this;
    }

    spectral_clustering<T>& 
    set_gamma(double gamma) {
      this->gamma = gamma;      
      return *this;
    }
    
    spectral_clustering<T>& 
    set_affinity(const std::string& aff) {
      std::string msg = "expected 'rbf', 'precomputed' or 'nearest_neighbors'; received: " + aff + "\n";
      require(aff == "rbf" || aff == "nearest_neighbors" || aff == "precomputed", msg);
      this->affinity = aff;    
      return *this;
    }
    
    spectral_clustering<T>& 
    set_n_neighbors(int n_nb) {
      std::string msg = "expected a positive n_neighbors; received: " + STR(n_nb) + "\n";
      require(n_nb > 0, msg);
      this->n_neighbors = n_nb;    
      return *this;
    }
    
    spectral_clustering<T>& 
    set_assign(KMeans<T>& obj) {
      this->assign = obj;   
      return *this;
    }
    
    spectral_clustering<T>& 
    set_norm_laplacian(bool norm_laplacian) {
      this->norm_laplacian = norm_laplacian;    
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
          assign.set_k(val.get<int>());
          msg += "n_clusters: " + val.tt + "; ";
        }
        else if(param == "n_iter") {
          assign.set_max_iter(val.get<int>());
          msg += "n_iter: " + val.tt + "; ";
        }       
        else if(param == "n_init") {
          assign.set_n_init(val.get<int>());
          msg += "n_init: " + val.tt + "; ";
        }       
        else if(param == "seed") {
          assign.set_seed(val.get<int>());
          msg += "n_seed: " + val.tt + "; ";
        }       
        else if(param == "eps") {
          assign.set_eps(val.get<double>());
          msg += "eps: " + val.tt + "; ";
        }
        else if(param == "n_comp") {
          set_n_component(val.get<int>());
          msg += "n_comp: " + val.tt + "; ";
        }
        else if(param == "gamma") {
          set_gamma(val.get<double>());
          msg += "gamma: " + val.tt + "; ";
        }
        else if(param == "affinity") {
          set_affinity(val.get<std::string>());
          msg += "affinity: " + val.tt + "; ";
        }
        else if(param == "n_neighbors") {
          set_n_neighbors(val.get<int>());
          msg += "n_neighbors: " + val.tt + "; ";
        }
        else if(param == "norm_laplacian") {
          set_norm_laplacian(val.get<bool>());
          msg += "norm_laplacian: " + val.tt + "; ";
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
      model = spectral_clustering_impl<T>(mat, assign, n_comp, gamma, affinity, n_neighbors,
                                       norm_laplacian, drop_first, mode, movable);
      is_fitted = true;
      return *this;           
    }

    template <class MATRIX>
    spectral_clustering<T>& 
    fit(MATRIX&& mat,
      const dvector<int>& label = dvector<int>()) { // ignored
      bool movable = true;
      model = spectral_clustering_impl<T>(mat, assign, n_comp, gamma, affinity, n_neighbors,
                                       norm_laplacian, drop_first, mode, movable);
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

    rowmajor_matrix<T> dense_affinity_matrix_() {
      require(is_fitted, "dense_affinity_matrix_() is called before fit()");       
      return model.is_dense_affinity ? model.dense_affinity_matrix 
                                     : model.sparse_affinity_matrix.to_rowmajor(); 
    } 

    crs_matrix<T> sparse_affinity_matrix_() {
      require(is_fitted, "sparse_affinity_matrix_() is called before fit()");       
      return model.is_dense_affinity ? model.dense_affinity_matrix.to_crs() 
                                     : model.sparse_affinity_matrix;
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

    int n_comp;
    double gamma;
    std::string affinity;
    int n_neighbors;
    KMeans<T> assign;
    bool norm_laplacian, drop_first, is_fitted;
    int mode;
    spectral_clustering_model<T> model;    
    SERIALIZE(n_comp, gamma, affinity, n_neighbors, 
              assign,
              norm_laplacian, drop_first, mode, 
              is_fitted, model)     
    typedef int predict_type;
  };
}

#endif
