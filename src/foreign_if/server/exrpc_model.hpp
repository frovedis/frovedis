// ---------------------------------------------------------------------
// NOTE: This file contains completely template-based routines.
// Based on the input argumnet type, e.g., float/double (DT1/DT2)
// sparse/dense (S_MAT1/D_MAT1) the template call will be deduced.
// thus during the support of float type or dense type data, no major
// changes need to be performed in this file.
// ---------------------------------------------------------------------

#ifndef _EXRPC_MODEL_HPP_
#define _EXRPC_MODEL_HPP_

#include "frovedis.hpp"
#include "frovedis/ml/glm/linear_model.hpp"
#include "frovedis/ml/recommendation/matrix_factorization_model.hpp"
#include "frovedis/ml/clustering/kmeans.hpp"
#include "../exrpc/exrpc_expose.hpp"
#include "dummy_model.hpp"
#include "model_tracker.hpp"

using namespace frovedis;
   
void cleanup_frovedis_server() {
  frovedis::finalize_model_table();
  frovedis::finalize_trackers();
}

// --- Frovedis Models Handling (print, delete, set threshold) ---
// prints registered Frovedis Model
template <class MODEL>
void show_model(int& mid) {
  auto mptr = get_model_ptr<MODEL>(mid);
  mptr->debug_print();
}

// releases registered Frovedis Model from the heap (if not already released)
template <class MODEL>
void release_model(int& mid) {
  if(!is_deleted(mid)) {               // if not yet deleted, then
    deleted_model_tracker.insert(mid); // mark as 'deleted'
    delete get_model_ptr<MODEL>(mid);  // get the model pointer and release it
    model_table.erase(mid);            // remove it's entry from model_table
  }
  else std::cout << "[warning] Request for already deleted model[" << mid << "].\n";
}

// resets the threshold value of the specified model
template <class T, class MODEL>
void set_glm_threshold(int& mid, T& thr) {
  auto mptr = get_model_ptr<MODEL>(mid);
  mptr->set_threshold(thr);
}

// ---  Broadcasted Model Handling ---
// gets the model heads from worker node for broadcasted model
template <class MODEL>
exrpc_ptr_t get_each_model(MODEL& m) {
  auto model = new MODEL(std::move(m));
  if(!model) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
#ifdef _EXRPC_DEBUG_
  std::cout << "(get_each_model): Connected with worker[" << get_selfid() << "]: model:\n";
  model->debug_print();
#endif
  return reinterpret_cast<exrpc_ptr_t>(model);
}

// prints broadcasted models for debugging purpose
template <class MODEL>
void print_each_model(exrpc_ptr_t& mp) {
  auto mptr = reinterpret_cast<MODEL*>(mp);
  std::cout << "(print_each_model): worker[" << get_selfid() << "]: printing broadcasted model:\n";
  mptr->debug_print();
}

// broadcasts the registered model from master node to all worker nodes
// and returns the vector containing all model heads at worker nodes
template <class T, class MODEL>
std::vector<exrpc_ptr_t> 
bcast_model_to_workers(int& mid) {
  auto mptr = get_model_ptr<MODEL>(mid);
  MODEL& model = *mptr;
#ifdef _EXRPC_DEBUG_
  std::cout << "[master] broadcasting model:\n";
  model.debug_print();
#endif
  //auto bcast_model = broadcast(model); 
  auto bcast_model = model.broadcast(); // for performance
  auto ret = bcast_model.map(get_each_model<MODEL>);
#ifdef _EXRPC_DEBUG_
  ret.mapv(print_each_model<MODEL>);
#endif
  return ret.gather();
}

// --- Prediction related functions on glm ---
// multiple inputs (crs_matrix_local): prediction done in parallel in worker nodes
template <class T, class MATRIX, class MODEL>
std::vector<T> 
parallel_glm_predict(MATRIX& data, exrpc_ptr_t& mp) {
  auto mptr = reinterpret_cast<MODEL*>(mp);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connected with worker[" << get_selfid()
            << "] to perform parallel prediction with model:\n";
  mptr->debug_print();
  std::cout << "with test data: \n";
  data.debug_print();
#endif
  auto thr = mptr->threshold;
  std::vector<T> ret = (thr == NONE) ? mptr->predict_probability(data) : mptr->predict(data);
#ifdef _EXRPC_DEBUG_
  std::cout << "[worker" << get_selfid()
            << "]: prediction done...deleting local model.\n";
#endif
  // after prediction broadcasted models will be freed from worker nodes.
  delete mptr;
  return ret;
}

template <class T, class MATRIX>
std::vector<T> 
parallel_lnrm_predict(MATRIX& data, exrpc_ptr_t& mp) {
  auto mptr = reinterpret_cast<linear_regression_model<T>*>(mp);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connected with worker[" << get_selfid()
            << "] to perform parallel prediction with model:\n";
  mptr->debug_print();
  std::cout << "with test data: \n";
  data.debug_print();
#endif
  std::vector<T> ret = mptr->predict(data);
#ifdef _EXRPC_DEBUG_
  std::cout << "[worker" << get_selfid()
            << "]: prediction done...deleting local model.\n";
#endif
  // after prediction broadcasted models will be freed from worker nodes.
  delete mptr;
  return ret;
}


template <class T, class L_MATRIX, class MODEL>
std::vector<T> predict_glm_by_worker(L_MATRIX& mat, MODEL& model) {
  return model.predict(mat);
}

template <class T, class L_MATRIX, class MODEL>
std::vector<T> predict_proba_glm_by_worker(L_MATRIX& mat, MODEL& model) {
  return model.predict_probability(mat);
}

// multiple inputs (crs_matrix): prediction done in parallel in worker nodes
template <class T, class MATRIX, class L_MATRIX, class MODEL>
std::vector<T> pgp2(exrpc_ptr_t& mat_ptr, int& mid, bool& prob) { // sklearn
  auto mat_ptr_ = reinterpret_cast<MATRIX*> (mat_ptr);
  auto model_ptr = get_model_ptr<MODEL>(mid);
  MATRIX& mat = *mat_ptr_;
  MODEL& model = *model_ptr;
  //auto bmodel = broadcast(model);
  auto bmodel = model.broadcast(); // for performance
  if (prob) return mat.data.map(predict_proba_glm_by_worker<T,L_MATRIX,MODEL>,bmodel)
                           .template moveto_dvector<T>().gather();
  else return mat.data.map(predict_glm_by_worker<T,L_MATRIX,MODEL>,bmodel)
                      .template moveto_dvector<T>().gather();
}

template <class T, class MATRIX, class L_MATRIX>
std::vector<T> p_lnrm_p2(exrpc_ptr_t& mat_ptr, int& mid) { // sklearn
  auto mat_ptr_ = reinterpret_cast<MATRIX*> (mat_ptr);
  auto model_ptr = get_model_ptr<linear_regression_model<T>>(mid);
  MATRIX& mat = *mat_ptr_;
  linear_regression_model<T>& model = *model_ptr;
  //auto bmodel = broadcast(model);
  auto bmodel = model.broadcast(); // for performance
  return mat.data.map(predict_glm_by_worker<T,L_MATRIX,linear_regression_model<T>>,
                      bmodel).template moveto_dvector<T>().gather();
}

// single input (crs_vector_local): prediction done in master node
template <class T, class MATRIX, class MODEL>
T single_glm_predict(MATRIX& data, int& mid) {
  auto mptr = get_model_ptr<MODEL>(mid);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connected with master node to perform single prediction with model:\n";
  mptr->debug_print();
  std::cout << "with test data: \n";
  data.debug_print();
#endif
  auto thr = mptr->threshold;
  // predict/predictProbability returns std::vector<T>. 
  // In single input prediction case, a vector of single 'T' type data only [0]
  return (thr == NONE) ? mptr->predict_probability(data)[0] : mptr->predict(data)[0];
}

template <class T, class MATRIX>
T single_lnrm_predict(MATRIX& data, int& mid) {
  auto mptr = get_model_ptr<linear_regression_model<T>>(mid);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connected with master node to perform single prediction with model:\n";
  mptr->debug_print();
  std::cout << "with test data: \n";
  data.debug_print();
#endif
  // predict returns std::vector<T>. 
  // In single input prediction case, a vector of single 'T' type data only [0]
  return mptr->predict(data)[0];
}

// --- Prediction related functions on mfm ---
// (uid, pid): single prediction done in master node
template <class T>
T single_mfm_predict(int& mid, int& uid, int& pid) {
  auto mptr = get_model_ptr<matrix_factorization_model<T>>(mid);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connected with master node to perform single prediction with model:\n";
  mptr->debug_print();
  std::cout << "with test data: \n";
  std::cout << "uid: " << uid << ", pid: " << pid << "\n";
#endif
  return mptr->predict(uid, pid);
}

// (Array[uid], Array[pid]): parallel prediction done in parallel in worker nodes
template <class T>
std::vector<T> 
parallel_mfm_predict(exrpc_ptr_t& mp,  
                     std::vector<std::pair<size_t,size_t>>& ids) {
  auto mptr = reinterpret_cast<matrix_factorization_model<T>*>(mp);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connected with worker node " << get_selfid()
            << " to perform parallel prediction with model:\n";
  mptr->debug_print();
  std::cout << "with test data: \n [";
  for(int i=0; i<total; ++i) 
    std::cout << "(" << ids[i].first << "," << ids[i].second << "), ";
  std::cout << "]\n";
#endif
  return mptr->predict_all(ids);
}

template <class T>
std::vector<T>
frovedis_als_predict(int& mid, std::vector<std::pair<size_t,size_t>>& ids) {
  auto mptr = get_model_ptr<matrix_factorization_model<T>>(mid);
  return mptr->predict_all(ids);
}

template <class T>
std::vector<std::pair<size_t,T>>
recommend_users(int& mid, int& pid, int& num) {
  auto mptr = get_model_ptr<matrix_factorization_model<T>>(mid);
  return mptr->recommend_users(pid,num);
}

template <class T>
std::vector<std::pair<size_t,T>>
recommend_products(int& mid, int& uid, int& num) {
  auto mptr = get_model_ptr<matrix_factorization_model<T>>(mid);
  return mptr->recommend_products(uid,num);
}

// --- Prediction related functions on kmeans model ---
// multiple inputs (crs_matrix_local): prediction done in parallel in worker nodes
template <class MATRIX, class MODEL>
std::vector<int> 
parallel_kmm_predict(MATRIX& data, exrpc_ptr_t& mp) {
  auto mptr = reinterpret_cast<MODEL*>(mp);
  MODEL& kmeans_model = *mptr; // centroid
#ifdef _EXRPC_DEBUG_
  std::cout << "Connected with worker[" << get_selfid()
            << "] to perform parallel prediction with model:\n";
  mptr->debug_print();
  std::cout << "with test data: \n";
  data.debug_print();
#endif
  auto ret = frovedis::kmeans_assign_cluster(data,kmeans_model);
#ifdef _EXRPC_DEBUG_
  std::cout << "[worker" << get_selfid()
            << "]: prediction done...deleting local model.\n";
#endif
  // after prediction broadcasted models will be freed from worker nodes.
  delete mptr;
  return ret;
}

template <class L_MATRIX, class MODEL>
std::vector<int> 
predict_kmm_by_worker(L_MATRIX& mat, MODEL& model) {
  return frovedis::kmeans_assign_cluster(mat,model);
}

template <class MATRIX, class L_MATRIX, class MODEL>
std::vector<int> pkp2(exrpc_ptr_t& mat_ptr, int& mid) {
  auto mat_ptr_ = reinterpret_cast<MATRIX*> (mat_ptr);
  auto model_ptr = get_model_ptr<MODEL>(mid);
  MATRIX& mat = *mat_ptr_;
  MODEL& model = *model_ptr;
  //auto bmodel = broadcast(model);
  auto bmodel = model.broadcast(); // for performance
  return mat.data.map(predict_kmm_by_worker<L_MATRIX,MODEL>,bmodel)
                      .template moveto_dvector<int>().gather();
}

// single input (crs_vector_local): prediction done in master node
template <class MATRIX, class MODEL>
int single_kmm_predict(MATRIX& data, int& mid) {
  auto mptr = get_model_ptr<MODEL>(mid);
  MODEL& kmeans_model = *mptr; // centroid
#ifdef _EXRPC_DEBUG_
  std::cout << "Connected with master node to perform single prediction with model:\n";
  mptr->debug_print();
  std::cout << "with test data: \n";
  data.debug_print();
#endif
  // frovedis::kmeans_assign_cluster returns std::vector<int>. 
  // In single input prediction case, a vector of single 'int' type data only [0]
  return frovedis::kmeans_assign_cluster(data,kmeans_model)[0];
}

// --- Load and Save Models ---
// loads a frovedis glm from the specified file
template <class MODEL>
dummy_glm load_glm(int& mid, MODEL_KIND& mkind, std::string& path) {
  auto mptr = new MODEL();
  if(!mptr) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
  mptr->loadbinary(path); // for faster loading
#ifdef _EXRPC_DEBUG_
  std::cout << "[load_glm]: model loading finished...printing:\n";
  mptr->debug_print();
#endif
  auto nftr = mptr->get_num_features();
  auto icpt = mptr->intercept;
  auto ncls = 2; // supports only binary classification problem 
  auto thr  = mptr->threshold;
  auto mptr_ = reinterpret_cast<exrpc_ptr_t>(mptr);
  register_model(mid,mkind,mptr_);
  return dummy_glm(mid,mkind,nftr,ncls,icpt,thr);
}

template <class T>
dummy_glm load_lnrm(int& mid, MODEL_KIND& mkind, std::string& path) {
  auto mptr = new linear_regression_model<T>();
  if(!mptr) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
  mptr->loadbinary(path); // for faster loading
#ifdef _EXRPC_DEBUG_
  std::cout << "[load_lnrm]: model loading finished...printing:\n";
  mptr->debug_print();
#endif
  auto nftr = mptr->get_num_features();
  auto icpt = mptr->intercept;
  auto mptr_ = reinterpret_cast<exrpc_ptr_t>(mptr);
  register_model(mid,mkind,mptr_);
  return dummy_glm(mid,mkind,nftr,0,icpt,1.0);
}

// loads a frovedis mfm from the specified file
template <class T>
dummy_mfm load_mfm(int& mid, MODEL_KIND& mkind, std::string& path) {
  auto mptr = new matrix_factorization_model<T>();
  if(!mptr) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
  mptr->loadbinary(path); // for faster loading
#ifdef _EXRPC_DEBUG_
  std::cout << "[load_mfm]: model loading finished...printing:\n";
  mptr->debug_print();
#endif
  auto rank = mptr->factor;
  auto nrow = mptr->numRows;
  auto ncol = mptr->numCols;
  auto mptr_ = reinterpret_cast<exrpc_ptr_t>(mptr);
  register_model(mid,mkind,mptr_);
  return dummy_mfm(mid,rank,nrow,ncol);
}

// loads frovedis kmeans model (rowmajor_matrix_local<T>) from the specified file
template <class T>
size_t load_kmm(int& mid, MODEL_KIND& mkind, std::string& path) {
  auto mat = frovedis::make_rowmajor_matrix_local_loadbinary<T>(path); 
  auto mptr = new rowmajor_matrix_local<T>(std::move(mat));
  if(!mptr) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
#ifdef _EXRPC_DEBUG_
  std::cout << "[load_kmm]: model loading finished...printing:\n";
  mptr->debug_print();
#endif
  auto k = mptr->local_num_col; // second dim of centroid
  auto mptr_ = reinterpret_cast<exrpc_ptr_t>(mptr);
  register_model(mid,mkind,mptr_);
  return k;
}

// saves the frovedis model to the specified file
template <class MODEL>
void save_model(int& mid, std::string& path) {
  auto mptr = get_model_ptr<MODEL>(mid);
  mptr->savebinary(path); // for faster saving
}

#endif 
