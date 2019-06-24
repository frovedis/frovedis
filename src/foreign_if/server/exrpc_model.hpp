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
#include "frovedis/core/exceptions.hpp"
#include "frovedis/ml/glm/linear_model.hpp"
#include "frovedis/ml/glm/multinomial_logistic_regression_model.hpp"
#include "frovedis/ml/recommendation/matrix_factorization_model.hpp"
#include "frovedis/ml/clustering/kmeans.hpp"
#include "frovedis/ml/clustering/agglomerative.hpp"
#include "frovedis/ml/clustering/spectral_clustering_model.hpp"
#include "frovedis/ml/clustering/spectral_embedding_model.hpp"
#include "frovedis/ml/tree/tree_model.hpp"
#include "frovedis/ml/fm/model.hpp"
#include "frovedis/ml/nb/nb_model.hpp"
#include "frovedis/ml/fpm/fp_growth_model.hpp"
#include "frovedis/ml/w2v/word2vec.hpp"
#include "../exrpc/exrpc_expose.hpp"
#include "dummy_model.hpp"
#include "dummy_matrix.hpp"
#include "model_tracker.hpp"

using namespace frovedis;
   
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
template <class T, class MODEL> // TODO: T not required, delete it
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

// broadcasts the fm_model from master node to all worker nodes
// and returns the vector containing all model heads at worker nodes
template <class T>
std::vector<exrpc_ptr_t>
bcast_fmm_to_workers(int& mid) {
  auto& model = *get_model_ptr<fm::fm_model<T>>(mid);
  auto bcast_model = frovedis::broadcast(model);
  //auto bcast_model = model.broadcast(); // TODO: support as a member function, for performance
  auto ret = bcast_model.map(get_each_model<fm::fm_model<T>>);
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
  auto thr = mptr->get_threshold();
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
  auto thr = mptr->get_threshold();
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
  auto ncls = mptr->get_num_classes();
  auto thr  = mptr->get_threshold();
  auto mptr_ = reinterpret_cast<exrpc_ptr_t>(mptr);
  register_model(mid,mkind,mptr_);
  return dummy_glm(mid,mkind,nftr,ncls,thr);
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
  auto mptr_ = reinterpret_cast<exrpc_ptr_t>(mptr);
  register_model(mid,mkind,mptr_);
  return dummy_glm(mid,mkind,nftr,0,1.0);
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

// generic model loading for dftdfp_growth_model.
template <class MODEL>
void load_model(int& mid, MODEL_KIND& mkind, std::string& path) {
  auto mptr = new MODEL();
  if(!mptr) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
  mptr->loadbinary(path); // for faster loading
  auto mptr_ = reinterpret_cast<exrpc_ptr_t>(mptr);
  register_model(mid,mkind,mptr_);
}

// load for naive_bayes_model returns a string
template <class T>
std::string load_nbm(int& mid, MODEL_KIND& mkind, std::string& path) {
  auto mptr = new naive_bayes_model<T>();
  if(!mptr) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
  mptr->loadbinary(path); // for faster loading
  auto mtype = mptr->model_type;
  auto mptr_ = reinterpret_cast<exrpc_ptr_t>(mptr);
  register_model(mid,mkind,mptr_);
  return mtype;
}

// load for agglomerative_model (rowmajor_matrix_local<T>)
template <class T>
int load_acm(int& mid, std::string& path) {
  auto mptr = new rowmajor_matrix_local<T>();
  if(!mptr) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
  *mptr = make_rowmajor_matrix_local_loadbinary<T>(path); // for faster loading
  auto nsamples = mptr->local_num_row + 1;
  auto mptr_ = reinterpret_cast<exrpc_ptr_t>(mptr);
  register_model(mid,ACM,mptr_);
  return nsamples;
}

// load for spectral_clustering_model returns an std::vector<int>
template <class T>
std::vector<int> load_scm(int& mid, std::string& path) {
  auto mptr = new spectral_clustering_model<T>();
  if(!mptr) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
  mptr->loadbinary(path); // for faster loading
  auto lbl = mptr->labels;
  auto mptr_ = reinterpret_cast<exrpc_ptr_t>(mptr);
  register_model(mid,SCM,mptr_);
  return lbl;
}

// saves the frovedis model to the specified file
template <class MODEL>
void save_model(int& mid, std::string& path) {
  auto mptr = get_model_ptr<MODEL>(mid);
  mptr->savebinary(path); // for faster saving
}

// saves the frovedis fm_model to the specified file
template <class T>
void save_fmm(int& mid, std::string& path) {
  auto mptr = get_model_ptr<fm::fm_model<T>>(mid);
  mptr->save(path); // savebinary not implemented for factorization machine model
}

template <class T>
std::vector<int>
frovedis_acm_pred(int& mid, int& ncluster) {
  auto& tree = *get_model_ptr<rowmajor_matrix_local<T>>(mid);
  return agglomerative_assign_cluster(tree, ncluster);
}

template <class T, class MATRIX>
T single_nbm_predict(MATRIX& data, int& mid) {
  auto mptr = get_model_ptr<naive_bayes_model<T>>(mid);
  return mptr->predict(data)[0]; // TODO: support: direct vector data from client side
}

template <class T>
dummy_matrix get_scm_affinity_matrix(int& mid) {
  auto& model = *get_model_ptr<spectral_clustering_model<T>>(mid);
  auto aff_mat = model.affinity_matrix;
  auto nrow = aff_mat.num_row;
  auto ncol = aff_mat.num_col;
  auto retp = new rowmajor_matrix<T>(std::move(aff_mat)); // stack to heap
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  auto retp_ = reinterpret_cast<exrpc_ptr_t>(retp);
  return dummy_matrix(retp_,nrow,ncol);
}

template <class T>
dummy_matrix get_sem_affinity_matrix(int& mid) {
  auto& model = *get_model_ptr<spectral_embedding_model<T>>(mid);
  auto aff_mat = model.affinity_matrix;
  auto nrow = aff_mat.num_row;
  auto ncol = aff_mat.num_col;
  auto retp = new rowmajor_matrix<T>(std::move(aff_mat)); // stack to heap
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  auto retp_ = reinterpret_cast<exrpc_ptr_t>(retp);
  return dummy_matrix(retp_,nrow,ncol);
}

template <class T>
dummy_matrix get_sem_embedding_matrix(int& mid) {
  auto& model = *get_model_ptr<spectral_embedding_model<T>>(mid);
  auto embed_mat = model.embed_matrix;
  auto nrow = embed_mat.num_row;
  auto ncol = embed_mat.num_col;
  auto retp = new rowmajor_matrix<T>(std::move(embed_mat)); // stack to heap
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  auto retp_ = reinterpret_cast<exrpc_ptr_t>(retp);
  return dummy_matrix(retp_,nrow,ncol);
}

// only for spark
// it should be executed by each worker.
// model should have broadcasted beforehand.
template <class T, class MATRIX>
std::vector<T> parallel_nbm_predict(MATRIX& data, exrpc_ptr_t& mp) {
  auto mptr = reinterpret_cast<naive_bayes_model<T>*>(mp);
  std::vector<T> ret = mptr->predict(data);
  // after prediction broadcasted models will be freed from worker nodes.
  delete mptr;
  return ret;
}

template <class T, class MATRIX>
std::vector<T> 
nbm_predict_at_worker(MATRIX& data, 
                      naive_bayes_model<T>& model, bool& prob) {
  //return (prob ? model.predict(data): model.predict_proba(data));
  return model.predict(data);
}

// the below call is for scikit-learn. 
// in that case model would be broadcasted first at master, 
// and then predict call would be mapped on each worker with probability value
template <class T, class MATRIX, class L_MATRIX>
std::vector<T> 
parallel_nbm_predict_with_broadcast(exrpc_ptr_t& mat_ptr, int& mid, bool& prob) { 
  MATRIX& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  auto& model = *get_model_ptr<naive_bayes_model<T>>(mid);
  auto bmodel = model.broadcast(); // for performance
  return mat.data.map(nbm_predict_at_worker<T,L_MATRIX>,bmodel,broadcast(prob))
                 .template moveto_dvector<T>().gather();
}

template <class T, class MATRIX>
T single_dtm_predict(MATRIX& data, int& mid) {
  auto mptr = get_model_ptr<decision_tree_model<T>>(mid);
  return mptr->predict(data)[0]; // TODO: support: direct vector data from client side
}

// only for spark
// it should be executed by each worker.
// model should have broadcasted beforehand.
template <class T, class MATRIX>
std::vector<T> parallel_dtm_predict(MATRIX& data, exrpc_ptr_t& mp) {
  auto mptr = reinterpret_cast<decision_tree_model<T>*>(mp);
  std::vector<T> ret = mptr->predict(data);
  // after prediction broadcasted models will be freed from worker nodes.
  delete mptr;
  return ret;
}

template <class T, class MATRIX>
std::vector<T> 
dtm_predict_at_worker(MATRIX& data, 
                      decision_tree_model<T>& model, bool& prob) {
  if(!prob) return model.predict(data);
  else {
    auto tmp = model.predict_with_probability(data);
    std::vector<T> ret(tmp.size());
    for(size_t i=0; i<tmp.size(); ++i) ret[i] = tmp[i].get_probability();
    return ret;
  }
}

// the below call is for scikit-learn. 
// in that case model would be broadcasted first at master, 
// and then predict call would be mapped on each worker with probability value
template <class T, class MATRIX, class L_MATRIX>
std::vector<T> 
parallel_dtm_predict_with_broadcast(exrpc_ptr_t& mat_ptr, int& mid, bool& prob) { 
  MATRIX& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  auto& model = *get_model_ptr<decision_tree_model<T>>(mid);
  auto bmodel = model.broadcast(); // for performance
  return mat.data.map(dtm_predict_at_worker<T,L_MATRIX>,bmodel,broadcast(prob))
                 .template moveto_dvector<T>().gather();
}

template <class T, class MATRIX>
T single_fmm_predict(MATRIX& data, int& mid) {
  auto mptr = get_model_ptr<fm::fm_model<T>>(mid);
  return mptr->predict(data)[0]; // TODO: support: direct vector data from client side
}

// only for spark
// it should be executed by each worker.
// model should have broadcasted beforehand.
template <class T, class MATRIX>
std::vector<T> parallel_fmm_predict(MATRIX& data, exrpc_ptr_t& mp) {
  auto mptr = reinterpret_cast<fm::fm_model<T>*>(mp);
  std::vector<T> ret = mptr->predict(data);
  // after prediction broadcasted models will be freed from worker nodes.
  delete mptr;
  return ret;
}

template <class T, class MATRIX>
std::vector<T> 
fmm_predict_at_worker(MATRIX& data, fm::fm_model<T>& model) {
  return model.predict(data);
}

// the below call is for scikit-learn. 
// in that case model would be broadcasted first at master, 
// and then predict call would be mapped on each worker with probability value
template <class T, class MATRIX, class L_MATRIX>
std::vector<T> 
parallel_fmm_predict_with_broadcast(exrpc_ptr_t& mat_ptr, int& mid) { 
  MATRIX& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  auto& model = *get_model_ptr<fm::fm_model<T>>(mid);
  auto bmodel = frovedis::broadcast(model); 
  //auto bmodel = model.broadcast(); // support as a member function, for performance
  return mat.data.map(fmm_predict_at_worker<T,L_MATRIX>,bmodel)
                 .template moveto_dvector<T>().gather();
}

template <class T>
dummy_matrix get_w2v_weight_ptr(int& mid) {
  auto& model = *get_model_ptr<rowmajor_matrix_local<T>>(mid);
  auto nrow = model.local_num_row;
  auto ncol = model.local_num_col;
  auto retp = new rowmajor_matrix<T>(make_rowmajor_matrix_scatter<T>(model));
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  auto retp_ = reinterpret_cast<exrpc_ptr_t>(retp);
  return dummy_matrix(retp_,nrow,ncol);
}

template <class T>
std::vector<T> get_w2v_weight_vector(int& mid) {
  auto& model = *get_model_ptr<rowmajor_matrix_local<T>>(mid);
  return model.val;
}

template <class T>
void show_w2v_weight(int& mid) {
  auto& model = *get_model_ptr<rowmajor_matrix_local<T>>(mid);
  model.debug_print();
}

// ref: w2v/w2v_corpus.cc: saveModel()
template <class T>
void save_w2v_model(int& mid, std::vector<std::string>& vocab,
                    std::string& path) {
  auto& model = *get_model_ptr<rowmajor_matrix_local<T>>(mid);
  int vocab_size = model.local_num_row;
  int hidden_size = model.local_num_col;
  const w2v::real* _weight_ptr = model.val.data();
  auto Wih = const_cast<w2v::real*>(_weight_ptr);
  FILE *fo = fopen(path.c_str(), "wb");
  // Save the word vectors
  fprintf(fo, "%d %d\n", vocab_size, hidden_size);
  for (int a = 0; a < vocab_size; a++) {
    fprintf(fo, "%s ", vocab[a].c_str());
    for (int b = 0; b < hidden_size; b++) {
      //fwrite(&Wih[a * hidden_size + b], sizeof(w2v::real), 1, fo); //binary
      fprintf(fo, "%f ", Wih[a * hidden_size + b]);
    }
    fprintf(fo, "\n");
  }
  fclose(fo);
}

#endif 
