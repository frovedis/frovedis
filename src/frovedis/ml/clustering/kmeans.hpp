#ifndef __KMEANS_HPP__
#define __KMEANS_HPP__

#include <frovedis/matrix/rowmajor_matrix.hpp>
#include <frovedis/ml/metrics.hpp>
#include <frovedis/ml/model_selection/param.hpp>
#include <frovedis/ml/clustering/kmeans_impl.hpp>
#include <frovedis/ml/clustering/shrink_kmeans.hpp>

namespace frovedis {

  template <class MATRIX, class T>
  std::vector<int>
  invoke_kmeans_assign_cluster(
    MATRIX& mat, 
    rowmajor_matrix_local<T>& centroid,
    T& inertia,
    bool need_inertia) {
    auto lbl = kmeans_assign_cluster(mat, centroid, inertia, need_inertia); 
    return lbl;
  }

  template <class T>
  struct KMeans { 
    KMeans(int k,
	   int max_iter = 100,
	   double eps = 0.01, 
           long seed = 0,
           bool shrink = false){
      this->k = k;
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
   set_eps(double eps) { // eps >= 0.0 && eps <= 1.0
     std::string msg = "expected eps within the range of 0.0 to 1.0; received: " + STR(eps) + "\n";
     require(eps >= 0.0 && eps <= 1.0, msg);
     this->eps = eps;
     return *this;
   }

   KMeans<T>& 
   set_seed(long seed) { 
     std::string msg = "expected a zero or positive value for seed; received: " + STR(seed) + "\n";
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
   std::vector<int> 
   label_inertia(MATRIX& mat, T& inertia, bool need_inertia) {
     auto l_inertia = make_node_local_allocate<T>();
     auto lbl =  mat.data.map(invoke_kmeans_assign_cluster
                              <typename MATRIX::local_mat_type, T>,
		              frovedis::broadcast(centroid), 
                              l_inertia,
                              frovedis::broadcast(need_inertia))
	                 .template moveto_dvector<int>()
                         .gather();
     inertia = l_inertia.reduce(add<T>);
     return lbl;
   }

   template <class MATRIX>
   KMeans<T>& 
   fit(MATRIX& mat,
     const dvector<T>& label = dvector<T>()) { // ignored
     n_iter = 0;
     if(use_shrink) 
       centroid = frovedis::shrink::kmeans(mat, k, max_iter, eps, seed, n_iter);
     else 
       centroid = frovedis::kmeans(mat, k, max_iter, eps, seed, n_iter);
     is_fitted = true;
     labels = label_inertia(mat, inertia, true);
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
     if(!is_fitted) REPORT_ERROR(USER_ERROR,
		    "[KMeans] prediction is called before fit\n");
     T inertia_score = 0.0;
     return label_inertia(mat, inertia_score, false);
   }
  
   template <class MATRIX>
   float score(MATRIX& mat, 
     const dvector<T>& label = dvector<T>()) { // ignored
     if(!is_fitted) REPORT_ERROR(USER_ERROR,
         	   "[KMeans] score is called before fit\n");
     T inertia_score = 0.0;
     label_inertia(mat, inertia_score, true);
     return static_cast<float>(inertia_score);
   }
  
  rowmajor_matrix_local<T> cluster_centers_() { return centroid; } 

  std::vector<int> labels_() { return labels; }   

  T inertia_() { return inertia; }

  int n_iter_() { return n_iter; }

  void debug_print(size_t limit = 0) { 
    std::cout << "centroid: \n";
    centroid.debug_print(limit); 
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
  typedef rowmajor_matrix_local<T> model_type;

  private:
    int k;
    int max_iter;
    double eps;
    long seed;
    bool is_fitted, use_shrink;
    std::vector<int> labels;
    T inertia;
    int n_iter;
    rowmajor_matrix_local<T> centroid;
    SERIALIZE(k, max_iter, eps, seed, is_fitted, use_shrink, 
              labels, inertia, n_iter, centroid);
};

}
#endif
