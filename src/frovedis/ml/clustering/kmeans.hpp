#ifndef __KMEANS_HPP__
#define __KMEANS_HPP__

#include <frovedis/matrix/rowmajor_matrix.hpp>
#include <frovedis/ml/metrics.hpp>
#include <frovedis/ml/model_selection/param.hpp>
#include <frovedis/ml/clustering/kmeans_impl.hpp>
#include <frovedis/ml/clustering/shrink_kmeans.hpp>

namespace frovedis {

template <class T>
struct KMeans { 
  KMeans(int k,
         int n_init = 1,
         int max_iter = 300,
         double eps = 0.0001,
         long seed = 0,
         bool shrink = false){
    this->k = k;
    this->n_init = n_init;
    this->max_iter = max_iter;
    this->eps = eps;
    this->seed = seed;
    this->use_shrink = shrink;
    this->is_fitted = false;
  }

  KMeans<T>& 
  set_k(int k) {
    std::string msg = "expected a positive k; received: " + STR(k) + "\n";
    require(k > 0, msg);
    this->k = k;
    return *this;
  }
    
  KMeans<T>&
  set_max_iter(int max_iter) {
    std::string msg = "expected a positive max_iter; received: " + STR(max_iter) + "\n";
    require(max_iter > 0, msg);
    this->max_iter = max_iter;
    return *this;
  }

  KMeans<T>&
  set_n_init(int n_init) {
    std::string msg = "expected a positive n_init; received: " + STR(n_init) + "\n";
    require(n_init > 0, msg);
    this->n_init = n_init;
    return *this;
  }

  KMeans<T>& 
  set_eps(double eps) { // eps >= 0.0 && eps <= 1.0
    std::string msg = "expected eps within the range of 0.0 to 1.0; received: " + STR(eps) + "\n";
    require(eps >= 0.0 && eps <= 1.0, msg);
    this->eps = eps;
    return *this;
  }

  KMeans<T>& 
  set_seed(long seed) { 
    std::string msg = "expected zero or a positive seed; received: " + STR(seed) + "\n";
    require(seed >= 0, msg);
    this->seed = seed;
    return *this;
  }

  KMeans<T>&
  set_use_shrink(bool shrink) {
    this->use_shrink = shrink;
    return *this;
  }

  KMeans<T>& 
  set_params(std::vector<std::pair<std::string, param_t>>& data) {
    std::string msg = "";
    for (auto& e: data) {
      auto param = e.first;
      auto val = e.second;
      if(param == "k") {
        set_k(val.get<int>());
        msg += "k: " + val.tt + "; ";
      }
      else if(param == "n_init") {
        set_n_init(val.get<int>());
        msg += "n_init: " + val.tt + "; ";
      }
      else if(param == "max_iter") {
        set_max_iter(val.get<int>());
        msg += "max_iter: " + val.tt + "; ";
      }
      else if(param == "eps") {
        set_eps(val.get<double>());
        msg += "eps: " + val.tt + "; ";
      }
      else if(param == "seed") {
        set_seed(val.get<long>());
        msg += "seed: " + val.tt + "; ";
      }
      else if(param == "use_shrink") {
        set_use_shrink(val.get<bool>());
        msg += "use_shrink: " + val.tt + "; ";
      }
      else REPORT_ERROR(USER_ERROR, "[KMeans] Unknown parameter: '"
                        + param + "' is encountered!\n");
    }
    RLOG(DEBUG) << msg << "\n";
    return *this;
  }
   
  template <class MATRIX>
  KMeans<T>& 
  fit(MATRIX& mat,
    const dvector<T>& label = dvector<T>()) { // ignored
    if(use_shrink) { 
      centroid = frovedis::shrink::kmeans(mat, k, max_iter, eps, seed, n_init,
                                          n_iter, inertia, labels);
    }
    else {
      centroid = frovedis::kmeans(mat, k, max_iter, eps, seed, n_init, 
                                  n_iter, inertia, labels);
    }
    is_fitted = true;
    return *this;           
  }

  template <class MATRIX>
  std::vector<int>
  fit_predict(MATRIX& mat,
    const dvector<T>& label = dvector<T>()) { // ignored
    fit(mat, label);
    return labels;
  }

  template <class MATRIX>
  std::vector<int> 
  predict(MATRIX& mat) {
    require(is_fitted, "[KMeans] predict(): is called before fit\n");
    return parallel_kmeans_assign_cluster(mat, centroid);
  }
  
  template <class MATRIX>
  float score(MATRIX& mat, 
    const dvector<T>& label = dvector<T>()) { // ignored
    require(is_fitted, "[KMeans] score(): is called before fit\n");
    float inertia_score = 0.0;
    parallel_kmeans_assign_cluster(mat, centroid, inertia_score);
    return -1.0f * inertia_score;
  }

  template <class MATRIX>
  rowmajor_matrix<T>
  transform(MATRIX& mat) {
    require(is_fitted, "[KMeans] transform(): is called before fit\n");
    return parallel_kmeans_transform(mat, centroid);
  }

  template <class MATRIX>
  rowmajor_matrix<T>
  fit_transform(MATRIX& mat,
    const dvector<T>& label = dvector<T>()) { // ignored
    fit(mat, label);
    return parallel_kmeans_transform(mat, centroid);
  }
  
  void debug_print(size_t limit = 0) { 
    std::cout << "centroid: \n";
    centroid.transpose().debug_print(limit); 
  }

  void savebinary(const std::string& fname) { 
    centroid.savebinary(fname); 
    // TODO: save other metadata
  }

  void save(const std::string& fname) { 
    centroid.save(fname); 
    // TODO: save other metadata
  }

  void loadbinary(const std::string& fname) {
    centroid = make_rowmajor_matrix_local_loadbinary<T>(fname);
    is_fitted = true;
    // TODO: load other metadata
  }

  void load(const std::string& fname) {
    centroid = make_rowmajor_matrix_local_load<T>(fname);
    is_fitted = true;
    // TODO: load other metadata
  }
  
  typedef T value_type;
  typedef int predict_value_type;
  typedef rowmajor_matrix_local<T> model_type;

  // attributes
  rowmajor_matrix_local<T> cluster_centers_() { 
    require(is_fitted, "cluster_centers_: attribute can be obtained after fit!\n");
    return centroid; 
  } 

  std::vector<int> labels_() { 
    require(is_fitted, "labels_: attribute can be obtained after fit!\n");
    return labels; 
  }   

  float inertia_() { 
    require(is_fitted, "inertia_: attribute can be obtained after fit!\n");
    return inertia; 
  }

  int n_iter_() { 
    require(is_fitted, "n_iter_: attribute can be obtained after fit!\n");
    return n_iter;
   }

  int n_clusters_() {
    require(is_fitted, "n_clusters_: attribute can be obtained after fit!\n");
    return centroid.local_num_col;
  }

  private:
    int k;
    int n_init, max_iter;
    double eps;
    long seed;
    bool is_fitted, use_shrink;
    std::vector<int> labels;
    float inertia;
    int n_iter;
    rowmajor_matrix_local<T> centroid;
    SERIALIZE(k, n_init, max_iter, eps, seed, is_fitted, use_shrink,
              labels, inertia, n_iter, centroid);
};

}
#endif
