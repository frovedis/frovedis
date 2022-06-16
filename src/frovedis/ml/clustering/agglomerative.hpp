#ifndef __AGGLOMERATIVE_HPP__
#define __AGGLOMERATIVE_HPP__

#include <frovedis/ml/model_selection/param.hpp>
#include <frovedis/ml/clustering/agglomerative_impl.hpp>

namespace frovedis {
    
  template <class T> 
  struct agglomerative_clustering {
    agglomerative_clustering(int n_clusters = 2,
                             const std::string& linkage = "average",
                             T threshold = 0) {
      this->n_clusters = n_clusters;
      this->linkage = linkage;
      this->threshold = threshold;  
      this->is_fitted = false;                            
    }
    
    agglomerative_clustering<T>& 
    set_n_clusters(int n_clusters) {
      std::string msg = "expected a positive n_clusters; received: " 
                         + STR(n_clusters) + "\n";
      require(n_clusters > 0, msg);
      this->n_clusters = n_clusters;  
      return *this;
    }

    agglomerative_clustering<T>& 
    set_linkage(const std::string& linkage) {
      std::string msg = "linkage must be \"complete\", \"average\" or \"single\";\
                         received: " + linkage + "\n";
      require(linkage == "complete" || linkage == "average" ||
              linkage == "single", msg);
      this->linkage = linkage;      
      return *this;
    }

    agglomerative_clustering<T>& 
    set_threshold(T threshold) { 
      std::string msg = "threshold cannot be negative: " + STR(threshold) + "\n";
      require(threshold >= 0, msg);
      this->threshold = threshold;  
      return *this;
    }      
    
    agglomerative_clustering<T>& 
    set_params(std::vector<std::pair<std::string, param_t>>& data) {
      std::string msg = "";
      for (auto& e: data) {
        auto param = e.first;
        auto val = e.second;
        if(param == "n_clusters") {
          set_n_clusters(val.get<int>());
          msg += "n_clusters: " + val.tt + "; ";
        }
        else if(param == "linkage") {
          set_linkage(val.get<std::string>());
          msg += "linkage: " + val.tt + "; ";
        }
        else if(param == "threshold") {
          set_threshold(val.get<T>());
          msg += "threshold: " + val.tt + "; ";
        }
        else REPORT_ERROR(USER_ERROR, "[agglomerative_clustering] Unknown parameter: '"
                          + param + "' is encountered!\n");
      }
      RLOG(DEBUG) << msg << "\n";
      return *this;
    }
    
    template <class MATRIX>
    agglomerative_clustering<T>& 
    fit(const MATRIX& mat,
      const dvector<int>& label = dvector<int>()) { // ignored  
      return _fit(mat, false);
    }

    template <class MATRIX>
    agglomerative_clustering<T>& 
    fit(MATRIX&& mat,
      const dvector<int>& label = dvector<int>()) { // ignored 
      return _fit(mat, true);
    }   
    
    template <class MATRIX>
    agglomerative_clustering<T>& 
    _fit(MATRIX& mat, bool input_movable) {
      auto link = this->linkage; 
      nleaves_ = mat.num_row;  
      model = input_movable ? agglomerative_training<T>(std::move(mat), link)
                            : agglomerative_training<T>(mat, link);
      is_fitted = true;
      labels = agglomerative_assign_cluster(model, n_clusters, threshold, nclusters_);
      return *this; 
    }
 
    template <class MATRIX>
    std::vector<int>
    fit_predict(const MATRIX& mat,
      const dvector<int>& label = dvector<int>()) { // ignored
      _fit(mat, false);
      return labels;        
    }

    template <class MATRIX>
    std::vector<int>
    fit_predict(MATRIX&& mat,
      const dvector<int>& label = dvector<int>()) { // ignored
      _fit(mat, true);
      return labels;        
    }
      
    std::vector<int>
    reassign(int n_clusters) {
      require(is_fitted, "reassign() is called before fit()");
      if(n_clusters == this->n_clusters)
        return labels;
      else
        return agglomerative_assign_cluster(model, n_clusters, threshold, nclusters_);
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

    void debug_print(size_t limit = 5) { 
      auto nrow = model.local_num_row;
      auto vp = model.val.data();
      std::cout << "--- dendrogram --- \n";
      //model.debug_print(limit); 
      std::cout << "\tX \tY \tdistance \tsize\n";
      if(limit == 0 || nrow <= 2 * limit) {
        for(size_t i = 0; i < nrow; ++i) {
          std::cout << nrow + i + 1 << ":\t";
          for(size_t j = 0; j < 3; ++j) std::cout << vp[i * 4 + j] << "\t";
          std::cout << vp[i * 4 + 3] << std::endl;
        }
      }
      else {
        for(size_t i = 0; i < limit; ++i) {
          std::cout << nrow + i + 1 << ":\t";
          for(size_t j = 0; j < 3; ++j) std::cout << vp[i * 4 + j] << "\t";
          std::cout << vp[i * 4 + 3] << std::endl;
        }
        std::cout << ":\n:\n";
        for(size_t i = nrow - limit; i < nrow; ++i) {
          std::cout << nrow + i + 1 << ":\t";
          for(size_t j = 0; j < 3; ++j) std::cout << vp[i * 4 + j] << "\t";
          std::cout << vp[i * 4 + 3] << std::endl;
        }
      }
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
      model = make_rowmajor_matrix_local_loadbinary<T>(fname);
      labels = agglomerative_assign_cluster(model, n_clusters, threshold, nclusters_);      
      nleaves_ = model.local_num_row + 1;    
      is_fitted = true;
      // TODO: load other metadata
    }

    void load(const std::string& fname) {
      model = make_rowmajor_matrix_local_load<T>(fname);
      labels = agglomerative_assign_cluster(model, n_clusters, threshold, nclusters_);      
      nleaves_ = model.local_num_row + 1;  
      is_fitted = true;      
      // TODO: load other metadata
    }      
            
    std::vector<int> labels_() {
      require(is_fitted, "labels_() is called before fit()"); 
      return labels; 
    }
    
    int n_clusters_() {
      require(is_fitted, "n_clusters_() is called before fit()");    
      return nclusters_; 
    }

    size_t n_leaves_() {
      require(is_fitted, "n_leaves_() is called before fit()");
      return nleaves_;  
    }      
      
    rowmajor_matrix_local<size_t> children_() {
      require(is_fitted, "children_() is called before fit()");
      return extract_cols(model, 0, 2).template astype<size_t>();  
    }

    std::vector<T> distances_() {
      require(is_fitted, "distances_() is called before fit()");
      return extract_cols(model, 2, 3).val;  
    }

    int n_connected_components_() { 
      require(is_fitted, "n_connected_components_() is called before fit()");
      return 1;    
    }      

    private:
      int n_clusters;
      std::string linkage;
      bool is_fitted;
      std::vector<int> labels;
      rowmajor_matrix_local<T> model;
      int nclusters_;
      size_t nleaves_;
      T threshold;      
      SERIALIZE(n_clusters, linkage, is_fitted, labels, model, nclusters_, threshold);
      typedef int predict_type;
  };
}

#endif
