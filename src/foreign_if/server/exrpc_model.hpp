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
#include "frovedis/ml/tree/ensemble_model.hpp"
#include "frovedis/ml/fm/model.hpp"
#include "frovedis/ml/nb/nb_model.hpp"
#include "frovedis/ml/fpm/fp_growth_model.hpp"
#include "frovedis/ml/w2v/word2vec.hpp"
#include "frovedis/ml/clustering/dbscan.hpp"
#include "frovedis/ml/neighbors/knn_unsupervised.hpp"
#include "frovedis/ml/neighbors/knn_supervised.hpp"
#include "frovedis/ml/lda/lda_cgs.hpp"
#include "frovedis/ml/tree/ensemble_model.hpp"
#include "frovedis/ml/kernel/kernel_svm.hpp"
#include "../exrpc/exrpc_expose.hpp"
#include "dummy_model.hpp"
#include "dummy_matrix.hpp"
#include "model_tracker.hpp"
#include "ml_result.hpp"
#include "lda_result.hpp"
#include "frovedis_mem_pair.hpp"

#include <typeinfo>
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

// --- Prediction related functions on Generic Models ---
// For: LNRM, NBM, FMM, DTM, RFM, GBT (spark only)
template <class T, class MATRIX, class MODEL>
T single_generic_predict(exrpc_ptr_t& f_dptr, int& mid) {
  auto matptr = reinterpret_cast<MATRIX*>(f_dptr);
  MATRIX& mat = *matptr;
  MODEL& model = *get_model_ptr<MODEL>(mid);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connected with master node to perform single generic prediction with model:\n";
  model.debug_print();
  std::cout << "with test data: \n";
  mat.debug_print();
#endif
  // predict returns std::vector<T>. 
  // In single input prediction case, a vector of single 'T' type data only [0]
  auto ret = model.predict(mat)[0];
  delete matptr; // it is internally created at spark side predict(Vector)
  return ret;
}

template <class T, class MATRIX, class MODEL>
std::vector<T> 
parallel_predict_at_worker(MATRIX& data, MODEL& model) {
  if(data.local_num_row < 1) return std::vector<T>();
  return model.predict(data);
}

template <class T, class MATRIX, class MODEL>
std::vector<T>
parallel_predict_proba_at_worker(MATRIX& data, MODEL& model) {
  if(data.local_num_row < 1) return std::vector<T>();
  return model.compute_probability_matrix(data).val;
}

// multi-input prediction in parallel: For NBM 
// first, model is broadcasted at rank #0, 
// and then predict call would be mapped on each worker with probability value
template <class T, class MATRIX, class L_MATRIX, class MODEL>
std::vector<T> 
parallel_generic_predict(exrpc_ptr_t& mat_ptr, int& mid, bool& prob) { 
  MATRIX& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  MODEL& model = *get_model_ptr<MODEL>(mid);
  auto bmodel = model.broadcast();
  auto loc_ret = make_node_local_allocate<std::vector<T>>();
  if(!prob) loc_ret = mat.data.map(parallel_predict_at_worker<T,L_MATRIX,MODEL>,
                                  bmodel);
  else loc_ret = mat.data.map(parallel_predict_proba_at_worker<T,L_MATRIX,MODEL>,
                              bmodel);
  return loc_ret.template moveto_dvector<T>().gather();
}

// --- Prediction related functions on glm ---
template <class T, class L_MATRIX, class MODEL>
std::vector<T> 
predict_lrm_at_worker(L_MATRIX& mat, MODEL& model, bool& prob) {
  if (mat.local_num_row < 1) return std::vector<T>();
  if(prob) return model.compute_probability_matrix(mat).val;
  else {
    // for spark: if threshold is cleared, it will always return raw probability values
    auto thr = model.get_threshold();
    if (thr == NONE) return model.predict_probability(mat);
    else             return model.predict(mat);
  }
}

// multiple inputs: prediction done in parallel in worker nodes
template <class T, class MATRIX, class L_MATRIX, class MODEL>
std::vector<T> 
parallel_lrm_predict(exrpc_ptr_t& mat_ptr, int& mid, bool& prob) {  
  MATRIX& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  MODEL& model = *get_model_ptr<MODEL>(mid);
  auto bmodel = model.broadcast(); 
  return mat.data.map(predict_lrm_at_worker<T,L_MATRIX,MODEL>,
                      bmodel, broadcast(prob))
                 .template moveto_dvector<T>().gather();
}

template <class T, class MATRIX, class L_MATRIX>
std::vector<T>
parallel_lnrm_predict(exrpc_ptr_t& mat_ptr, int& mid, bool& prob) {
  MATRIX& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  auto& model = *get_model_ptr<linear_regression_model<T>>(mid);
  auto bmodel = model.broadcast();
  std::vector<T> ret;
  if(prob) REPORT_ERROR(USER_ERROR, "predict_proba: not applicable for regression model!\n");
  else ret = mat.data.map(parallel_predict_at_worker<T,L_MATRIX,
                          linear_regression_model<T>>,
                          bmodel)
                     .template moveto_dvector<T>().gather();
  return ret;
}

template <class T, class MATRIX, class L_MATRIX>
std::vector<T>
parallel_svm_predict(exrpc_ptr_t& mat_ptr, int& mid, bool& prob) {
  MATRIX& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  auto& model = *get_model_ptr<svm_model<T>>(mid);
  auto bmodel = model.broadcast();
  std::vector<T> ret;
  if(prob) REPORT_ERROR(USER_ERROR, "predict_proba: not applicable for svm model!\n");
  else ret = mat.data.map(parallel_predict_at_worker<T,L_MATRIX,
                          svm_model<T>>,
                          bmodel)
                     .template moveto_dvector<T>().gather();
  return ret;
}

// single input: prediction done in master node (spark only)
template <class T, class MATRIX, class MODEL>
T single_glm_predict(exrpc_ptr_t& f_dptr, int& mid) {
  auto matptr = reinterpret_cast<MATRIX*>(f_dptr);
  MATRIX& mat = *matptr;
  MODEL& model = *get_model_ptr<MODEL>(mid);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connected with master node to perform single glm prediction with model:\n";
  model.debug_print();
  std::cout << "with test data: \n";
  mat.debug_print();
#endif
  auto thr = model.get_threshold();
  // predict/predictProbability returns std::vector<T>. 
  // In single input prediction case, a vector of single 'T' type data only [0]
  auto ret = (thr == NONE) ? model.predict_probability(mat)[0] : model.predict(mat)[0];
  delete matptr; // it is internally created at spark side predict(Vector)
  return ret;
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
  auto total = ids.size();
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
template <class MATRIX, class MODEL>
std::vector<int> 
frovedis_kmeans_predict(exrpc_ptr_t& mat_ptr, int& mid) {
  MATRIX& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  MODEL& est = *get_model_ptr<MODEL>(mid);
  return est.predict(mat);
}

template <class MATRIX, class MODEL>
float frovedis_kmeans_score(exrpc_ptr_t& mat_ptr, int& mid) {
  MATRIX& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  MODEL& est = *get_model_ptr<MODEL>(mid);
  return est.score(mat);
}

template <class T, class MATRIX, class MODEL>
dummy_matrix 
frovedis_kmeans_transform(exrpc_ptr_t& mat_ptr, int& mid) {
  MATRIX& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  MODEL& est = *get_model_ptr<MODEL>(mid);
  auto retp = new rowmajor_matrix<T>(est.transform(mat));
  return to_dummy_matrix<rowmajor_matrix<T>,
                         rowmajor_matrix_local<T>>(retp);
}

template <class T, class MODEL>
std::vector<T> 
frovedis_kmeans_centroid(int& mid) {
  MODEL& est = *get_model_ptr<MODEL>(mid);
  return est.cluster_centers_().transpose().val;
}

// single input: prediction done in master node
template <class MATRIX, class MODEL>
int single_kmm_predict(exrpc_ptr_t& f_dptr, int& mid) {
  MATRIX& data = *reinterpret_cast<MATRIX*>(f_dptr);
  MODEL& est = *get_model_ptr<MODEL>(mid);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connected with master node to perform single prediction with model:\n";
  est.debug_print(10);
  std::cout << "with test data: \n";
  data.debug_print(10);
#endif
  // frovedis::kmeans_assign_cluster returns std::vector<int>.
  // In single input prediction case, a vector of single 'int' type data only [0]
  auto centroid = est.cluster_centers_();
  return kmeans_assign_cluster(data, centroid)[0];
}

// --- KNN related functions ---
template <class T, class I, class MATRIX, class ALGO>
knn_result frovedis_kneighbors(exrpc_ptr_t& mat_ptr, int& mid, 
                               int& k, bool& need_distance) {
  auto& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  auto& obj = *get_model_ptr<ALGO>(mid);
  auto model = obj.template kneighbors<I>(mat, k, need_distance); // knn model is returned
  auto indices = new rowmajor_matrix<I>(std::move(model.indices));
  auto distances = new rowmajor_matrix<T>(std::move(model.distances));
  auto indices_ptr = reinterpret_cast<exrpc_ptr_t>(indices);
  auto distances_ptr = reinterpret_cast<exrpc_ptr_t>(distances);
  return knn_result(k, 
                    indices_ptr, indices->num_row, indices->num_col,
                    distances_ptr, distances->num_row, distances->num_col);
}

template <class R, class T> 
rowmajor_matrix_local<R> 
change_datatype_helper(rowmajor_matrix_local<T>& mat){
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  rowmajor_matrix_local<R> res(nrow, ncol);
  auto datap = mat.val.data();
  auto resdatap = res.val.data();
  for(size_t i = 0; i < nrow * ncol; i++) {
    resdatap[i] = static_cast<R> (datap[i]);
  }
  return res;
}

template <class R, class T> 
rowmajor_matrix<R> 
change_datatype(rowmajor_matrix<T>& mat) {
  rowmajor_matrix<R> res;
  res.num_row = mat.num_row;
  res.num_col = mat.num_col; 
  res.data = mat.data.map(change_datatype_helper<R,T>);
  return res;
}

// for spark: indices mat wil be typecasted rmm<int> => rmm<double>
template <class T, class I, class MATRIX, class ALGO>
knn_result frovedis_kneighbors_spark(exrpc_ptr_t& mat_ptr, int& mid, 
                                     int& k, bool& need_distance) {
  auto& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  auto& obj = *get_model_ptr<ALGO>(mid);
  auto model = obj.template kneighbors<I>(mat, k, need_distance); // knn model is returned
  auto indices = new rowmajor_matrix<double>(change_datatype<double>(model.indices)); 
  auto distances = new rowmajor_matrix<T>(std::move(model.distances));
  auto indices_ptr = reinterpret_cast<exrpc_ptr_t>(indices);
  auto distances_ptr = reinterpret_cast<exrpc_ptr_t>(distances);
  return knn_result(k, 
                    indices_ptr, indices->num_row, indices->num_col,
                    distances_ptr, distances->num_row, distances->num_col);
}

template <class I, class MATRIX, class ALGO, 
          class OUTMAT, class OUTMAT_LOC>
dummy_matrix frovedis_kneighbors_graph(exrpc_ptr_t& mat_ptr, int& mid, 
                                       int& k, std::string& mode) {
  auto& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  auto& obj = *get_model_ptr<ALGO>(mid);
  auto ret = new OUTMAT(obj.template kneighbors_graph<I>(mat, k, mode)); 
  return to_dummy_matrix<OUTMAT,OUTMAT_LOC>(ret);
}

template <class I, class MATRIX, class ALGO,
          class OUTMAT, class OUTMAT_LOC>
dummy_matrix frovedis_radius_neighbors(exrpc_ptr_t& mat_ptr, int& mid, 
                                       float& radius) { 
  auto& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  auto& obj = *get_model_ptr<ALGO>(mid);
  auto ret = new OUTMAT(obj.template radius_neighbors<I>(mat, radius));
  return to_dummy_matrix<OUTMAT,OUTMAT_LOC>(ret);
}

template <class I, class MATRIX, class ALGO,
          class OUTMAT, class OUTMAT_LOC>
dummy_matrix frovedis_radius_neighbors_graph(exrpc_ptr_t& mat_ptr, int& mid, 
                                             float& radius, std::string& mode) {
  auto& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  auto& obj = *get_model_ptr<ALGO>(mid);
  auto ret = new OUTMAT(obj.template radius_neighbors_graph<I>(mat, radius, mode));  
  return to_dummy_matrix<OUTMAT,OUTMAT_LOC>(ret);
}

// predict for knc
template <class T, class I, class MATRIX, class ALGO>
std::vector<T> 
frovedis_knc_predict(exrpc_ptr_t& mat_ptr, int& mid, bool& save_proba){
  auto& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  auto& obj = *get_model_ptr<ALGO>(mid);
  return obj.template predict<I>(mat, save_proba).gather();
}

// predict for knr
template <class T, class I, class MATRIX, class ALGO>
std::vector<T> 
frovedis_knr_predict(exrpc_ptr_t& mat_ptr, int& mid){
  auto& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  auto& obj = *get_model_ptr<ALGO>(mid);
  return obj.template predict<I>(mat).gather();
}

// predict_proba for knc 
template <class I, class MATRIX, class ALGO,
          class OUTMAT, class OUTMAT_LOC>
dummy_matrix frovedis_knc_predict_proba(exrpc_ptr_t& mat_ptr, int& mid){
  auto& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  auto& obj = *get_model_ptr<ALGO>(mid);
  auto ret = new OUTMAT(obj.template predict_probability<I>(mat)); // rmm 
  return to_dummy_matrix<OUTMAT,OUTMAT_LOC>(ret);
}

// score for knr 
template <class T, class I, class MATRIX, class ALGO>
float frovedis_model_score(exrpc_ptr_t& mptr, exrpc_ptr_t& lblptr, int& mid){
  auto& mat = *reinterpret_cast<MATRIX*>(mptr);
  dvector<T>& lbl = *reinterpret_cast<dvector<T>*>(lblptr); 
  auto& obj = *get_model_ptr<ALGO>(mid);
  return obj.template score<I>(mat, lbl);
}

// --- Prediction related functions for factorization machine model ---
template <class T, class MATRIX, class L_MATRIX>
std::vector<T>
parallel_fmm_predict(exrpc_ptr_t& mat_ptr, int& mid, bool& prob) {
  MATRIX& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  auto& model = *get_model_ptr<fm::fm_model<T>>(mid);
  //auto bmodel = model.broadcast(); // not supported
  auto bmodel = broadcast(model);
  std::vector<T> ret;
  if(prob) REPORT_ERROR(USER_ERROR,"Frovedis currently doesn't support predict_proba for Factorization machine!\n");
  else ret = mat.data.map(parallel_predict_at_worker<T,L_MATRIX,
                          fm::fm_model<T>>, bmodel)
                     .template moveto_dvector<T>().gather();
  return ret;
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
size_t load_kmm(int& mid, MODEL_KIND& mkind, 
                std::string& path) {
  auto est = new KMeans<T>(2); // k default 2, TODO: load from metadata
  est->loadbinary(path);
  if(!est) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
#ifdef _EXRPC_DEBUG_
  std::cout << "[load_kmm]: model loading finished...printing:\n";
  est->debug_print();
#endif
  auto k = est->cluster_centers_().local_num_col; // second dim of centroid
  auto estptr_ = reinterpret_cast<exrpc_ptr_t>(est);
  register_model(mid, mkind, estptr_);
  return k;
}

// generic model loading 
template <class MODEL>
void load_model(int& mid, MODEL_KIND& mkind, std::string& path) {
  auto mptr = new MODEL();
  if(!mptr) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
  mptr->loadbinary(path); // for faster loading
  auto mptr_ = reinterpret_cast<exrpc_ptr_t>(mptr);
  register_model(mid,mkind,mptr_);
}

// load for fp_growth_model returns FIS count
template <class MODEL>
int load_fpm(int& mid, MODEL_KIND& mkind, std::string& path) {
  auto mptr = new MODEL();
  if(!mptr) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
  mptr->loadbinary(path); // for faster loading
  auto fis_cnt = mptr->get_count();
  auto mptr_ = reinterpret_cast<exrpc_ptr_t>(mptr);
  register_model(mid,mkind,mptr_);
  return fis_cnt;
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

template <class MODEL>
std::vector<int> load_acm(int& mid, std::string& path) {
  auto mptr = new MODEL();
  if(!mptr) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
  mptr->loadbinary(path); // for faster loading
  auto labels = mptr->labels_();
  auto mptr_ = reinterpret_cast<exrpc_ptr_t>(mptr);
  register_model(mid,ACM,mptr_);
  return labels;
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
frovedis_acm_reassign(int& mid, int& ncluster) {
  auto& agg_est = *get_model_ptr<agglomerative_clustering<T>>(mid);
  return agg_est.reassign(ncluster);
}

template <class T>
std::vector<size_t>
get_acm_children(int& mid) {
  auto& agg_est = *get_model_ptr<agglomerative_clustering<T>>(mid);
  return agg_est.children_().val;
}

template <class T>
std::vector<T>
get_acm_distances(int& mid) {
  auto& agg_est = *get_model_ptr<agglomerative_clustering<T>>(mid);
  return agg_est.distances_();
}

template <class T>
int get_acm_n_components(int& mid) {
  auto& agg_est = *get_model_ptr<agglomerative_clustering<T>>(mid);
  return agg_est.n_connected_components_();
}

template <class T>
int get_acm_n_clusters(int& mid) {
  auto& agg_est = *get_model_ptr<agglomerative_clustering<T>>(mid);
  return agg_est.n_clusters_();
}

template <class T>
dummy_matrix get_scm_affinity_matrix(int& mid) {
  auto& model = *get_model_ptr<spectral_clustering_model<T>>(mid);
  auto aff_mat = model.affinity_matrix;
  auto retp = new rowmajor_matrix<T>(std::move(aff_mat)); // stack to heap
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return to_dummy_matrix<rowmajor_matrix<T>, 
                         rowmajor_matrix_local<T>>(retp);
}

template <class T>
dummy_matrix get_sem_affinity_matrix(int& mid) {
  auto& model = *get_model_ptr<spectral_embedding_model<T>>(mid);
  auto aff_mat = model.affinity_matrix;
  auto retp = new rowmajor_matrix<T>(std::move(aff_mat)); // stack to heap
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return to_dummy_matrix<rowmajor_matrix<T>, 
                         rowmajor_matrix_local<T>>(retp);
}

template <class T>
dummy_matrix get_sem_embedding_matrix(int& mid) {
  auto& model = *get_model_ptr<spectral_embedding_model<T>>(mid);
  auto embed_mat = model.embed_matrix;
  auto retp = new rowmajor_matrix<T>(std::move(embed_mat)); // stack to heap
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return to_dummy_matrix<rowmajor_matrix<T>, 
                         rowmajor_matrix_local<T>>(retp);
}

// --- Prediction related functions on Tree models ---
template <class T, class MATRIX>
std::vector<T> 
dtm_predict_at_worker(MATRIX& data, 
                      decision_tree_model<T>& model, bool& prob) {
  if (data.local_num_row <= 0) return std::vector<T>();
  if(!prob) return model.predict(data);
  else {
    auto tmp = model.predict_with_probability(data);
    auto nsamples = tmp.size();
    auto nclasses = 2; // supports only binary classification 
    // TODO: raise exception if nclasses > 2
    auto retsz =  nsamples * nclasses; 
    std::vector<T> ret(retsz);
    auto rptr = ret.data();
    auto tptr = tmp.data();
    for(size_t i = 0; i < nsamples; ++i) {
      auto probability = tptr[i].get_probability();
      auto pred_id = static_cast<int>(tptr[i].get_predict()); // 0 or 1
      auto non_pred_id = !pred_id; // 1 or 0
      rptr[i * 2 + pred_id] = probability;
      rptr[i * 2 + non_pred_id] = static_cast<T>(1.0) - probability;
    }
    return ret;
  }
}

template <class T, class MATRIX, class L_MATRIX>
std::vector<T> 
parallel_dtm_predict(exrpc_ptr_t& mat_ptr, int& mid, bool& prob) { 
  MATRIX& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  auto& model = *get_model_ptr<decision_tree_model<T>>(mid);
  auto bmodel = model.broadcast(); 
  return mat.data.map(dtm_predict_at_worker<T,L_MATRIX>,
                      bmodel, broadcast(prob))
                 .template moveto_dvector<T>().gather();
}

template <class T, class MATRIX, class L_MATRIX>
std::vector<T> 
parallel_rfm_predict(exrpc_ptr_t& mat_ptr, int& mid, bool& prob) { 
  MATRIX& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  auto& model = *get_model_ptr<random_forest_model<T>>(mid);
  auto bmodel = model.broadcast();
  std::vector<T> ret; 
  if(prob) REPORT_ERROR(USER_ERROR,"Frovedis currently doesn't support predict_proba for Random Forest!\n");
  else ret = mat.data.map(parallel_predict_at_worker<T,L_MATRIX,
                          random_forest_model<T>>, bmodel)
                     .template moveto_dvector<T>().gather();
  return ret;
}

template <class T, class MATRIX, class L_MATRIX>
std::vector<T> 
parallel_gbt_predict(exrpc_ptr_t& mat_ptr, int& mid, bool& prob) {
  MATRIX& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  auto& model = *get_model_ptr<gradient_boosted_trees_model<T>>(mid);
  auto bmodel = model.broadcast(); 
  std::vector<T> ret; 
  if(prob) REPORT_ERROR(USER_ERROR,"Frovedis currently doesn't support predict_proba for GBTree!\n"); 
  else ret = mat.data.map(parallel_predict_at_worker<T,L_MATRIX,
                          gradient_boosted_trees_model<T>>,
                          bmodel)
                     .template moveto_dvector<T>().gather();
  return ret;
}

// GBT spark related getters
template <class MODEL>
size_t
frovedis_ensemble_get_num_trees(int& mid) {
  auto& model = *get_model_ptr<MODEL>(mid);
  return model.get_num_trees();
}

template <class MODEL>
size_t
frovedis_ensemble_get_total_num_nodes(int& mid) {
  auto& model = *get_model_ptr<MODEL>(mid);
  return model.get_total_num_nodes();
}

template <class MODEL, class T>
std::vector<T>
frovedis_ensemble_get_tree_weights(int& mid) {
  auto& model = *get_model_ptr<MODEL>(mid);
  return model.get_tree_weights();
}

template <class MODEL>
std::string
frovedis_ensemble_to_string(int& mid) {
  auto& model = *get_model_ptr<MODEL>(mid);
  return model.to_string();
}

template <class T, class MODEL>
std::vector<T> get_weight_vector(int& mid) {
  auto& model = *get_model_ptr<MODEL>(mid);
  std::vector<T> ret;
  return model.weight;
}

template <class T, class MODEL>
std::vector<T> get_weight_as_vector(int& mid) {
  auto& model = *get_model_ptr<MODEL>(mid);
  return model.weight.val;
}

template <class T, class MODEL>
std::vector<T> get_intercept_as_vector(int& mid) {
  auto& model = *get_model_ptr<MODEL>(mid);
  return std::vector<T>({model.intercept}); // intercept is a scalar quantity for other linear models
}

template <class T, class MODEL>
std::vector<T> get_intercept_vector(int& mid) {
  auto& model = *get_model_ptr<MODEL>(mid);
  return model.intercept; 
}

template <class T, class MODEL>
std::vector<T> get_pi_vector(int& mid) {
  auto& model = *get_model_ptr<MODEL>(mid);
  return model.pi;
}

template <class T, class MODEL>
std::vector<T> get_feature_count(int& mid) {
  auto& model = *get_model_ptr<MODEL>(mid);
  return model.feature_count;
}

template <class T, class MODEL>
std::vector<T> get_theta_vector(int& mid) {
  auto& model = *get_model_ptr<MODEL>(mid);
  return model.theta.val;
}

template <class T, class MODEL>
std::vector<T> get_cls_counts_vector(int& mid) {
  auto& model = *get_model_ptr<MODEL>(mid);
  return model.cls_counts;
}

template <class T>
dummy_matrix get_w2v_weight_ptr(int& mid) {
  auto& model = *get_model_ptr<rowmajor_matrix_local<T>>(mid);
  auto retp = new rowmajor_matrix<T>(make_rowmajor_matrix_scatter<T>(model));
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return to_dummy_matrix<rowmajor_matrix<T>, 
                         rowmajor_matrix_local<T>>(retp);
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

template <class MODEL>
dummy_matrix get_lda_component(int& mid) {
  auto& model = *get_model_ptr<MODEL>(mid);
  auto sum = sum_of_rows(model.word_topic_count); // computing sum_of_rows for word_topic_count
  int axis = 0;
  auto dist = get_distribution_matrix_local(model.word_topic_count, sum, axis);
  auto distptr = new rowmajor_matrix<double>(make_rowmajor_matrix_scatter(dist));
  return to_dummy_matrix<rowmajor_matrix<double>,
                         rowmajor_matrix_local<double>>(distptr);
}

template <class TD, class MATRIX, class MODEL>
dummy_lda_result
frovedis_lda_transform(exrpc_ptr_t& dptr, double& alpha,
                       double& beta, int& num_iter,
                       std::string& algorithm,
                       int& num_explore_iter, int& mid) {
  auto& model = *get_model_ptr<MODEL>(mid);
  MATRIX& mat = *reinterpret_cast<MATRIX*>(dptr);
  std::vector<double> likelihood, perplexity;
  auto doc_topic_count = lda_test(mat, alpha, beta, num_iter, algorithm,
                                  num_explore_iter, model, perplexity,
                                  likelihood);
  auto retptr = new rowmajor_matrix<double>(
                get_distribution_matrix<TD>(doc_topic_count));
  auto dmat = to_dummy_matrix<rowmajor_matrix<double>,
                              rowmajor_matrix_local<double>>(retptr);
  return dummy_lda_result(dmat, perplexity.back(), likelihood.back());
}

template <class TD, class MATRIX, class MODEL>
rowmajor_matrix<TD>
spark_lda_test(exrpc_ptr_t& dptr, double& alpha,
               double& beta, int& num_iter,
               std::string& algorithm,
               int& num_explore_iter, int& mid,
               double& ppl, double& llh) {
  auto& t_mod = *get_model_ptr<MODEL>(mid);
  MATRIX& mat = *reinterpret_cast<MATRIX*>(dptr);
  auto mod_mat = mat.template change_datatype<TD>();
  std::vector<double> likelihood, perplexity;
  auto doc_topic_count = lda_test(mod_mat, alpha, beta, num_iter, algorithm,
                                  num_explore_iter, t_mod.model, perplexity,
                                  likelihood);
  // --- outputs ---
  ppl = perplexity.back();
  llh = likelihood.back();
  return doc_topic_count;
}

template <class TD, class MATRIX, class MODEL>
dummy_lda_result
frovedis_lda_transform_for_spark(exrpc_ptr_t& dptr, double& alpha,
                                 double& beta, int& num_iter,
                                 std::string& algorithm,
                                 int& num_explore_iter, int& mid) {
  double ppl, llh;
  auto doc_topic_count = spark_lda_test<TD,MATRIX,MODEL>(dptr, alpha, 
                                        beta, num_iter, 
                                        algorithm, num_explore_iter, mid,
                                        ppl, llh);
  auto retptr = new rowmajor_matrix<double>(
                get_distribution_matrix<TD>(doc_topic_count));
  auto dmat = to_dummy_matrix<rowmajor_matrix<double>,
                              rowmajor_matrix_local<double>>(retptr);
  return dummy_lda_result(dmat, ppl, llh);
}

template <class TC, class MODEL>
dummy_matrix get_topics_matrix(int& mid) {
  auto& t_mod = *get_model_ptr<MODEL>(mid);
  auto topic_mat = t_mod.model.word_topic_count; // COMMENT: OK or compute distribution same as lda_component?
  auto topic_mat_ptr = new rowmajor_matrix<TC>(make_rowmajor_matrix_scatter(topic_mat));
  return to_dummy_matrix<rowmajor_matrix<TC>,
                         rowmajor_matrix_local<TC>>(topic_mat_ptr);
}

template <class MODEL>
size_t get_vocabulary_size(int& mid) {
  auto& t_mod = *get_model_ptr<MODEL>(mid);
  return t_mod.model.word_topic_count.local_num_row;
}

template <class MODEL>
size_t get_num_topics(int& mid) {
  auto& t_mod = *get_model_ptr<MODEL>(mid);
  return t_mod.model.word_topic_count.local_num_col;
}

template <class I, class T>
rowmajor_matrix_local<I>
arg_partition_local(rowmajor_matrix_local<T>& mat, // to resolve overloaded version  
                    size_t max_terms) {
  return frovedis::arg_partition<T,I>(mat, max_terms);
}

template <class I, class T>
rowmajor_matrix<I>
arg_partition_global(rowmajor_matrix<T>& mat, size_t max_terms) {
  auto nrow = mat.num_row;
  auto ncol = mat.num_col;
  rowmajor_matrix<I> ret(mat.data.map(arg_partition_local<I,T>, 
                         broadcast(max_terms)));
  ret.num_row = nrow;
  ret.num_col = ncol;
  return ret;
}

template <class I>
void decode_doc_ids(rowmajor_matrix_local<I>& matrix,
                    std::vector<I> logic) {
  auto mptr = matrix.val.data();
  auto idptr = logic.data();
  for(size_t i = 0; i < matrix.val.size(); i++) mptr[i] = idptr[mptr[i]];
}

template <class I, class T>
distMatrix<I,T>
extract_top_k_of_each_row(rowmajor_matrix<T>& mat, 
                          int& k) {
  auto index = arg_partition_global<I,T>(mat, k);
  auto key = extract_k_cols(mat, k).gather();
  auto value = extract_k_cols(index, k).gather();
  radix_sort_desc(key.val.data(), value.val.data(), key.val.size());
  return distMatrix<I,T>(std::move(value), std::move(key));
}

// compute distribution of words (vocab) per topic
template <class MODEL>
dummy_matrix get_topic_word_distribution(int& mid) {
  auto& model = *get_model_ptr<MODEL>(mid);
  auto topic_word_count_local = model.model.word_topic_count.transpose();
  auto topic_word_count = make_rowmajor_matrix_scatter(topic_word_count_local);
  auto topic_word_dist = new rowmajor_matrix<double>(
                         get_distribution_matrix(topic_word_count));
  return to_dummy_matrix<rowmajor_matrix<double>, 
                         rowmajor_matrix_local<double>>(topic_word_dist);
}

// extract: top words (vocab) per topic (describeTopics(max))
template <class I, class T>
distMatrix<I,T> 
extract_sorted_topic_word_distribution(exrpc_ptr_t& dptr,
                                       int& max_terms) {
  auto& mat = *reinterpret_cast<rowmajor_matrix<T>*>(dptr);
  return extract_top_k_of_each_row<I,T>(mat, max_terms);
}

// compute distribution of documents per topic
template <class MODEL>
dummy_matrix get_topic_doc_distribution(int& mid) { 
  auto& model = *get_model_ptr<MODEL>(mid);
  auto topic_doc_count = model.doc_topic_count.transpose(); 
  auto topic_doc_dist = new rowmajor_matrix<double>(
                        get_distribution_matrix(topic_doc_count));
  return to_dummy_matrix<rowmajor_matrix<double>,
                         rowmajor_matrix_local<double>>(topic_doc_dist);
}  

// extract: top documents per topic
template <class MODEL, class I, class T>
distMatrix<I,T> 
extract_sorted_topic_doc_distribution(exrpc_ptr_t& dptr,
                                      int& mid,
                                      int& maxDocumentsPerTopic) { 
  auto& mat = *reinterpret_cast<rowmajor_matrix<T>*>(dptr);
  auto& model = *get_model_ptr<MODEL>(mid);
  auto ret = extract_top_k_of_each_row<I,T>(mat, maxDocumentsPerTopic);
  decode_doc_ids(ret.indices, model.orig_doc_id); // decode ids in-place
  return ret;
}

// compute distribution of topics per document
template <class MODEL>
dummy_matrix get_doc_topic_distribution(int& mid) {
  auto& model = *get_model_ptr<MODEL>(mid);
  auto doc_topic_count = model.doc_topic_count;
  auto doc_topic_dist = new rowmajor_matrix<double>(
                        get_distribution_matrix(doc_topic_count));
  return to_dummy_matrix<rowmajor_matrix<double>,
                         rowmajor_matrix_local<double>>(doc_topic_dist);
}

template <class MODEL>
std::vector<long> get_doc_id(int& mid) {
  auto& model = *get_model_ptr<MODEL>(mid);
  return model.orig_doc_id;
}

// extract: top topics per document
template <class I, class T>
distMatrix<I,T> 
extract_sorted_doc_topic_distribution(exrpc_ptr_t& dptr,
                                      int& k) { 
  auto& mat = *reinterpret_cast<rowmajor_matrix<T>*>(dptr);
  return extract_top_k_of_each_row<I,T>(mat, k); 
}

template <class TD, class MATRIX, class MODEL, 
          class I, class T>
distMatrix<I,T>
get_top_documents_per_topic(exrpc_ptr_t& dptr, std::vector<I>& orig_doc_id,
                            double& alpha, double& beta, 
                            int& num_iter, std::string& algorithm,
                            int& num_explore_iter, int& mid,
                            int& maxDocumentsPerTopic) { 
  // lda_test(dptr, alpha, ...) + sort(extract(part(distribution(topic_doc_count))), maxDocumentsPerTopic)
  double ppl, llh;
  auto doc_topic_count = spark_lda_test<TD,MATRIX,MODEL>(dptr, alpha,
                                        beta, num_iter,
                                        algorithm, num_explore_iter, mid,
                                        ppl, llh);
  auto topic_doc_count = doc_topic_count.transpose();
  auto topic_doc_dist = get_distribution_matrix(topic_doc_count);
  auto ret = extract_top_k_of_each_row<I,T>(topic_doc_dist, maxDocumentsPerTopic); 
  decode_doc_ids(ret.indices, orig_doc_id); // decode ids in-place
  return ret;
}

template <class TD, class MATRIX, class MODEL, 
          class I, class T>
distMatrix<I,T>
get_top_topics_per_document(exrpc_ptr_t& dptr,
                            double& alpha,
                            double& beta, int& num_iter,
                            std::string& algorithm,
                            int& num_explore_iter, int& mid,
                            int& k) {
  // lda_test(dptr, alpha, ...) + sort(extract(part(distribution(doc_topic_count))), k)
  double ppl, llh;
  auto doc_topic_count = spark_lda_test<TD,MATRIX,MODEL>(dptr, alpha,
                                        beta, num_iter,
                                        algorithm, num_explore_iter, mid,
                                        ppl, llh);
  auto doc_topic_dist = get_distribution_matrix(doc_topic_count);
  return extract_top_k_of_each_row<I,T>(doc_topic_dist, k); 
}

template <class MODEL>
dummy_lda_model load_lda_model(int& mid, std::string& path) {
  auto spark_lda_model_kind = LDASP;
  load_model<MODEL>(mid, spark_lda_model_kind, path);
  auto& model_ = *get_model_ptr<MODEL>(mid);
  return dummy_lda_model(model_.num_docs, model_.num_topics, 
                         model_.model.word_topic_count.local_num_row);
}

//predict for svm kernel
template <class T, class MATRIX, class MODEL>
std::vector<T>
ksvm_predict(exrpc_ptr_t& mat_ptr, int& mid, bool& prob) {
  MATRIX& mat = *reinterpret_cast<MATRIX*> (mat_ptr);
  auto loc_mat = mat.gather();
  MODEL& model = *get_model_ptr<MODEL>(mid);
  if(!prob) return model.predict(loc_mat);
  else return model.compute_probability_matrix(loc_mat).val;
}

template <class T, class MODEL>
std::vector<T> get_support_vector(int& mid) {
  auto& model = *get_model_ptr<MODEL>(mid);
  return model.sv.val;
}

template <class MODEL>
std::vector<size_t> get_support_idx(int& mid) {
  auto& model = *get_model_ptr<MODEL>(mid);
  return model.sv_index;
}

template <class MODEL>
std::vector<size_t> get_dbscan_core_sample_indices(int& mid) {
  auto& model = *get_model_ptr<MODEL>(mid);
  return model.core_sample_indices_();
}

template <class MODEL, class T>
dummy_matrix get_dbscan_components(int& mid) {
  auto& model = *get_model_ptr<MODEL>(mid);
  auto retptr = new rowmajor_matrix<T>(model.components_());
  return to_dummy_matrix<rowmajor_matrix<T>,
                         rowmajor_matrix_local<T>>(retptr);
}

#endif 
