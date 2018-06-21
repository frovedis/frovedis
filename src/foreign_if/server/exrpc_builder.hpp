// ---------------------------------------------------------------------
// NOTE: This file contains completely template-based routines.
// Based on the input argumnet type, e.g., float/double (DT1/DT2)
// sparse/dense (S_MAT1/D_MAT1) the template call will be deduced.
// thus during the support of float type or dense type data, no major
// changes need to be performed in this file.
// ---------------------------------------------------------------------

#ifndef _EXRPC_BUILDER_HPP_
#define _EXRPC_BUILDER_HPP_

#include <vector>
#include "frovedis.hpp"
#include "frovedis/matrix/crs_matrix.hpp"
#include "frovedis/matrix/diag_matrix.hpp"
#include "frovedis/matrix/jds_crs_hybrid.hpp"
#include "frovedis/matrix/blockcyclic_matrix.hpp"
#include "frovedis/matrix/sparse_svd.hpp"
#include "frovedis/ml/glm/logistic_regression_with_sgd.hpp"
#include "frovedis/ml/glm/logistic_regression_with_lbfgs.hpp"
#include "frovedis/ml/glm/svm_with_sgd.hpp"
#include "frovedis/ml/glm/svm_with_lbfgs.hpp"
#include "frovedis/ml/glm/linear_regression_with_sgd.hpp"
#include "frovedis/ml/glm/linear_regression_with_lbfgs.hpp"
#include "frovedis/ml/glm/lasso_with_sgd.hpp"
#include "frovedis/ml/glm/lasso_with_lbfgs.hpp"
#include "frovedis/ml/glm/ridge_regression_with_sgd.hpp"
#include "frovedis/ml/glm/ridge_regression_with_lbfgs.hpp"
#include "frovedis/ml/recommendation/als.hpp"
#include "frovedis/ml/clustering/kmeans.hpp"
#include "../exrpc/exrpc_expose.hpp"
#include "frovedis_mem_pair.hpp"
#include "dummy_model.hpp"
#include "dummy_matrix.hpp"
#include "scalapack_result.hpp"
#include "short_hand_type.hpp"
#include "model_tracker.hpp"

using namespace frovedis;
using namespace std;

void cleanup_frovedis_server() {
  frovedis::finalize_model_table();
  frovedis::finalize_trackers();
}

// --- (A) Functions to enable ML data transfer and handling the same ---
template <class MATRIX>
void set_matrix_data(MATRIX& mat,
                     std::vector<exrpc_ptr_t>& eps) {
  size_t iam = get_selfid();
  auto mem_ptr = reinterpret_cast<MATRIX*>(eps[iam]);
  mat = *mem_ptr;
}

template <class T>
void set_vector_data(std::vector<T>& vec,
                     std::vector<exrpc_ptr_t>& eps,
                     bool is_to_be_moved = false) {
  size_t iam = get_selfid();
  auto mem_ptr = reinterpret_cast<std::vector<T>*>(eps[iam]);
  if(is_to_be_moved) vec.swap(*mem_ptr);
  else vec = *mem_ptr;
}

template <class T>
std::vector<size_t> 
set_dvector_data(std::vector<T>& vec,
                 std::vector<exrpc_ptr_t>& eps) {
  size_t iam = get_selfid();
  auto mem_ptr = reinterpret_cast<std::vector<T>*>(eps[iam]);
  vec = *mem_ptr;
  // map_partitions needs to return std::vector
  std::vector<size_t> ret(1);
  ret[0] = vec.size();
  return ret;
}

// after the loading, input data will be destroyed...
template <class MATRIX>
exrpc_ptr_t load_local_data(MATRIX& mat) {
#ifdef _EXRPC_DEBUG_
  std::cout << "connected to worker[" << frovedis::get_selfid()
            << "]: local data loading started.\n";
#endif
  auto mat_local_head = new MATRIX(std::move(mat));
  if(!mat_local_head) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  auto mat_ = reinterpret_cast<exrpc_ptr_t>(mat_local_head);
  return mat_;
}

// after the loading, input data will be destroyed...
template <class T, class MATRIX>
frovedis_mem_pair load_local_glm_data(MATRIX& mat, std::vector<T>& vec) {
  auto mat_ = load_local_data<MATRIX>(mat);
  auto dvec_ = load_local_data<std::vector<T>>(vec);
  return frovedis_mem_pair(mat_,dvec_);
}

// prints frovedis glm data for debugging purpose
template <class T>
void show_dvector(exrpc_ptr_t& vptr) {
  auto vecp = reinterpret_cast<dvector<T>*>(vptr);
  std::cout << "dvector(size: " << vecp->size() << "):\n";
  auto vv = vecp->gather();
  for(auto& v : vv) std::cout << " " << v; std::cout << std::endl;
}

// prints frovedis data for debugging purpose
template <class MATRIX>
void show_data(exrpc_ptr_t& dptr) {
  auto matp = reinterpret_cast<MATRIX*>(dptr);
  std::cout << "matrix:\n"; matp->debug_print();
}

template <class T, class MATRIX>
void show_glm_data(frovedis_mem_pair& mp) {
  auto mptr = mp.first();
  auto dptr = mp.second();
  show_data<MATRIX>(mptr);
  show_dvector<T>(dptr);
}

// returns a memptr pointing to the head of created dvector
template <class T>
exrpc_ptr_t create_and_set_dvector(std::vector<exrpc_ptr_t>& dvec_eps) {
  auto vecp = new dvector<T>(make_dvector_allocate<T>());
  if(!vecp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed!\n");
  auto dvec_ep_bcast = broadcast(dvec_eps);
  auto ss = vecp->map_partitions(set_dvector_data<T>, dvec_ep_bcast).gather();
  vecp->set_sizes(ss);
  return reinterpret_cast<exrpc_ptr_t>(vecp);
}

// returns a memptr pointing to the head of created matrix
template <class MATRIX, class L_MATRIX>
exrpc_ptr_t
create_and_set_data(std::vector<exrpc_ptr_t>& mat_eps,
                    size_t& nrows, size_t& ncols) {
  auto matp = new MATRIX(make_node_local_allocate<L_MATRIX>());
  if(!matp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed!\n");
  auto mat_ep_bcast = broadcast(mat_eps);
  matp->data.mapv(set_matrix_data<L_MATRIX>, mat_ep_bcast);
  matp->set_num(nrows,ncols);
  auto matp_ = reinterpret_cast<exrpc_ptr_t>(matp);
#ifdef _EXRPC_DEBUG_
  show_data<MATRIX>(matp_);
#endif
  return matp_;
}

// returns a memory pair containing the heads of created crs_matrix and dvector
template <class T, class MATRIX, class L_MATRIX>
frovedis_mem_pair 
create_and_set_glm_data(std::vector<exrpc_ptr_t>& mat_eps,
                        std::vector<exrpc_ptr_t>& dvec_eps, 
                        size_t& nrows, size_t& ncols) {
  auto matp_ = create_and_set_data<MATRIX,L_MATRIX>(mat_eps,nrows,ncols);
  auto vecp_ = create_and_set_dvector<T>(dvec_eps);
  auto mp = frovedis_mem_pair(matp_,vecp_);
#ifdef _EXRPC_DEBUG_
  show_glm_data<T,MATRIX>(mp);
#endif
  return mp;
}

// input arr[exrpc::crs_matrix_local<T>] <= loaded from spark worker data
// returns exrpc::crs_matrix<T> from array of exrpc::crs_matrix_local<T>
template <class T>
exrpc_ptr_t
create_crs_data(std::vector<exrpc_ptr_t>& mat_eps,
                size_t& nrows, size_t& ncols) {
  return create_and_set_data<crs_matrix<T>,crs_matrix_local<T>>(mat_eps,nrows,ncols);
}

// input arr[exrpc::rowmajor_matrix_local<T>] <= loaded from spark worker data
// returns exrpc::rowmajor_matrix<T> from array of exrpc::rowmajor_matrix_local<T>
template <class T>
exrpc_ptr_t
create_rmm_data(std::vector<exrpc_ptr_t>& mat_eps,
                size_t& nrows, size_t& ncols) {
  auto matp = new rowmajor_matrix<T>(
              make_node_local_allocate<rowmajor_matrix_local<T>>());
  if(!matp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed!\n");
  auto mat_ep_bcast = broadcast(mat_eps);
  matp->data.mapv(set_matrix_data<rowmajor_matrix_local<T>>, mat_ep_bcast);
  matp->set_num(nrows,ncols);
  matp->align_block(); // align as per frovedis no. of worker nodes
  auto matp_ = reinterpret_cast<exrpc_ptr_t>(matp);
#ifdef _EXRPC_DEBUG_
  show_data<rowmajor_matrix<T>>(matp_);
#endif
  return matp_;
}

// input arr[exrpc::rowmajor_matrix_local<T>] <= loaded from spark worker data
// returns exrpc::colmajor_matrix<T> from array of exrpc::rowmajor_matrix_local<T>
template <class T>
exrpc_ptr_t
create_cmm_data(std::vector<exrpc_ptr_t>& mat_eps,
                size_t& nrows, size_t& ncols) {
  auto rmatp_ = create_rmm_data<T>(mat_eps,nrows,ncols);
  auto rmatp = reinterpret_cast<rowmajor_matrix<T>*>(rmatp_);
  auto &rmat = *rmatp;
  auto matp = new colmajor_matrix<T>(rmat);
  if(!matp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed!\n");
  delete rmatp; // deleting the intermediate rowmajor matrix
  auto matp_ = reinterpret_cast<exrpc_ptr_t>(matp);
#ifdef _EXRPC_DEBUG_
  show_data<colmajor_matrix<T>>(matp_);
#endif
  return matp_;
}

// input arr[exrpc::rowmajor_matrix_local<T>] <= loaded from spark worker data
// returns exrpc::blockcyclic_matrix<T> from array of exrpc::rowmajor_matrix<T>
template <class T>
exrpc_ptr_t
create_bcm_data(std::vector<exrpc_ptr_t>& mat_eps,
                size_t& nrows, size_t& ncols) {
  auto cmatp_ = create_cmm_data<T>(mat_eps,nrows,ncols);
  auto cmatp = reinterpret_cast<colmajor_matrix<T>*>(cmatp_);
  auto &cmat = *cmatp;
  auto matp = new blockcyclic_matrix<T>(cmat);
  if(!matp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed!\n");
  delete cmatp; // deleting the intermediate colmajor matrix
  auto matp_ = reinterpret_cast<exrpc_ptr_t>(matp);
#ifdef _EXRPC_DEBUG_
  show_data<blockcyclic_matrix<T>>(matp_);
#endif
  return matp_;
}

// saves exrpc::matrix in specified file/dir
template <class MATRIX>
void save_matrix(exrpc_ptr_t& mptr, 
                 std::string& path, bool& isbinary) {
  auto matp = reinterpret_cast<MATRIX*>(mptr);
  isbinary ? matp->savebinary(path) : matp->save(path);
}

// converts exrpc::matrix to exrpc::dummy_matrix
template <class MATRIX>
dummy_matrix to_dummy_matrix(MATRIX* mptr) {
  size_t nr = mptr->num_row;
  size_t nc = mptr->num_col;
  auto mptr_ = reinterpret_cast<exrpc_ptr_t>(mptr);
#ifdef _EXRPC_DEBUG_
  show_data<MATRIX>(mptr_);
#endif
  return dummy_matrix(mptr_,nr,nc);
}

// creates crs_matrix from node local coo vector strings
template <class T>
dummy_matrix 
create_crs_from_local_coo_string_vectors(std::vector<exrpc_ptr_t>& vec_eps) {
  auto lvs = make_node_local_allocate<std::vector<std::string>>();
  auto vec_ep_bcast = broadcast(vec_eps);
  // after crs_matrix creation local string vectors are to be freed
  auto is_to_be_moved = broadcast(true); 
  lvs.mapv(set_vector_data<std::string>,vec_ep_bcast,is_to_be_moved);
  bool zero_origin = false; // spark user_id starts with 1
  auto matp = new crs_matrix<T>(make_crs_matrix_loadcoo<T>(lvs,zero_origin));
  if(!matp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed!\n");
  return to_dummy_matrix<crs_matrix<T>>(matp);
}

// loads data from specified file/dir and creates a exrpc::crs_matrix<T>
template <class T>
dummy_matrix load_crs_matrix(std::string& path, bool& isbinary) {
  crs_matrix<T> *matp = NULL;
  if(isbinary)
    matp = new crs_matrix<T>(make_crs_matrix_loadbinary<T>(path)); //rvalue
  else
    matp = new crs_matrix<T>(make_crs_matrix_load<T>(path)); //rvalue
  if(!matp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed!\n");
  return to_dummy_matrix<crs_matrix<T>>(matp);
}
 
// loads data from specified file/dir and creates a exrpc::rowmajor_matrix<T>
template <class T> 
dummy_matrix load_rmm_matrix(std::string& path, bool& isbinary) {
  rowmajor_matrix<T> *matp = NULL;
  if(isbinary)
    matp = new rowmajor_matrix<T>(make_rowmajor_matrix_loadbinary<T>(path)); //rvalue
  else
    matp = new rowmajor_matrix<T>(make_rowmajor_matrix_load<T>(path)); //rvalue
  if(!matp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed!\n");
  return to_dummy_matrix<rowmajor_matrix<T>>(matp);
}
 
// loads data from specified file/dir and creates a exrpc::colmajor_matrix<T>
template <class T> 
dummy_matrix load_cmm_matrix(std::string& path, bool& isbinary) {
  colmajor_matrix<T> *matp = NULL;
  // (rvalue) rowmajor => colmajor
  if(isbinary)
    matp = new colmajor_matrix<T>(make_rowmajor_matrix_loadbinary<T>(path)); 
  else
    matp = new colmajor_matrix<T>(make_rowmajor_matrix_load<T>(path));
  if(!matp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed!\n");
  return to_dummy_matrix<colmajor_matrix<T>>(matp);
}
 
// loads data from specified file/dir and creates a exrpc::blockcyclic_matrix<T>
template <class T> 
dummy_matrix load_bcm_matrix(std::string& path, bool& isbinary) {
  colmajor_matrix<T> cmat;
  if(isbinary)
    cmat = colmajor_matrix<T>(make_rowmajor_matrix_loadbinary<T>(path));
  else
    cmat = colmajor_matrix<T>(make_rowmajor_matrix_load<T>(path));
  // (rvalue) colmajor => blockcyclic
  auto matp = new blockcyclic_matrix<T>(std::move(cmat));
  if(!matp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed!\n");
  return to_dummy_matrix<blockcyclic_matrix<T>>(matp);
}
 
// releases the frovedis dvector from the heap
template <class T>
void release_dvector(exrpc_ptr_t& vptr) {
  delete reinterpret_cast<dvector<T>*>(vptr);
}

// releases the frovedis data from the heap
template <class DATA>
void release_data(exrpc_ptr_t& dptr) {
  delete reinterpret_cast<DATA*>(dptr);
}

// releases the frovedis glm data from the heap
template <class T, class MATRIX>
void release_glm_data(frovedis_mem_pair& mp) {
  checkAssumption(mp.mx && mp.my);
  auto mptr = mp.first();
  auto dptr = mp.second();
  release_data<MATRIX>(mptr);
  release_dvector<T>(dptr);
  mp.mx = mp.my = 0;
}

// returns local array of the input local exrpc::matrix
template <class T, class LOC_MATRIX>
std::vector<T> get_local_array(exrpc_ptr_t& d_ptr) {
  auto matp = reinterpret_cast<LOC_MATRIX*>(d_ptr);
  return matp->val;
}

// returns local array of the input local matrix
template <class T, class LOC_MATRIX>
std::vector<T> get_local_array_from_local_matrix(LOC_MATRIX& mat) {
  return mat.val;
}

// returns array from exrpc::std::vector<T>
template <class T>
std::vector<T> vec_to_array(exrpc_ptr_t& vptr) {
  auto vecp = reinterpret_cast<std::vector<T>*>(vptr);
  std::vector<T> &vec = *vecp;
  return std::vector<T>(vec);
}

// converts MATRIX to exrpc::rowmajor_matrix<T>
template <class T, class MATRIX>
dummy_matrix to_rowmajor_matrix(exrpc_ptr_t& d_ptr) {
  auto matp = reinterpret_cast<MATRIX*>(d_ptr);
  // MATRIX class should have to_rowmajor() method defined
  auto rmat = matp->to_rowmajor(); 
  auto rmatp = new rowmajor_matrix<T>(std::move(rmat));
  if(!rmatp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed!\n");
  return to_dummy_matrix<rowmajor_matrix<T>>(rmatp);
}

// reurns the exrpc::pointer of the input local matrix
template <class LOC_MATRIX>
exrpc_ptr_t get_local_pointer(LOC_MATRIX& lm) {
  auto matp = &lm;
  return reinterpret_cast<exrpc_ptr_t>(matp);
}

// returns all the local data pointers of the input MATRIX data
template <class MATRIX, class LOC_MATRIX>
std::vector<exrpc_ptr_t>
get_all_local_pointers(exrpc_ptr_t& d_ptr) {
  auto matp = reinterpret_cast<MATRIX*>(d_ptr);
  return matp->data.map(get_local_pointer<LOC_MATRIX>).gather();
}

// converts MATRIX to rowmajor_matrix<T> and returns all its local data pointers
template <class T, class MATRIX>
std::vector<exrpc_ptr_t>
convert_and_get_all_rml_pointers(exrpc_ptr_t& d_ptr) {
  auto rmat = to_rowmajor_matrix<T,MATRIX>(d_ptr);
  return get_all_local_pointers<rowmajor_matrix<T>,
                                rowmajor_matrix_local<T>>(rmat.mptr);
}

// saves exrpc::std::vector<T> as diag_matrix_local<T>
template <class T>
void save_as_diag_matrix_local(exrpc_ptr_t& vptr,
                               std::string& path, bool& isbinary) {
  diag_matrix_local<T> dmat;
  dmat.val = vec_to_array<T>(vptr);
  isbinary ? dmat.savebinary(path) : dmat.save(path);
}

// converts a MATRIX data into LOC_MATRIX value array form
template <class T, class MATRIX, class LOC_MATRIX>
std::vector<T> matrix_to_array(exrpc_ptr_t& d_ptr) {
  auto matp = reinterpret_cast<MATRIX*>(d_ptr);
  return matp->data.map(get_local_array_from_local_matrix<T,LOC_MATRIX>)
                   .template moveto_dvector<T>().gather();
}

// converts MATRIX data into rowmajor array form
template <class T, class MATRIX>
std::vector<T> 
to_rowmajor_array(exrpc_ptr_t& d_ptr) {
  auto rmat = to_rowmajor_matrix<T,MATRIX>(d_ptr); // ret -> dummy_matrix
  auto ret = matrix_to_array<T,rowmajor_matrix<T>,
                             rowmajor_matrix_local<T>>(rmat.mptr);
  delete reinterpret_cast<rowmajor_matrix<T>*>(rmat.mptr);
  return ret;
}

// converts exrpc::blockcyclic_matrix<T> to exrpc::colmajor_matrix<T>
template <class T>
dummy_matrix blockcyclic_to_colmajor_matrix(exrpc_ptr_t& d_ptr) {
  auto bmatp = reinterpret_cast<blockcyclic_matrix<T>*>(d_ptr);
  auto cmatp = new colmajor_matrix<T>(bmatp->to_colmajor());
  if(!cmatp) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
  return to_dummy_matrix<colmajor_matrix<T>>(cmatp); 
}

// converts exrpc::rowmajor_matrix<T> to exrpc::colmajor_matrix<T>
template <class T>
dummy_matrix rowmajor_to_colmajor_matrix(exrpc_ptr_t& d_ptr) {
  auto rmatp = reinterpret_cast<rowmajor_matrix<T>*>(d_ptr);
  auto &rmat = *rmatp;
  auto cmatp = new colmajor_matrix<T>(rmat); 
  if(!cmatp) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
  return to_dummy_matrix<colmajor_matrix<T>>(cmatp); 
}

// returns a colmajor array from a exrpc::rowmajor_matrix<T> data
// gather() -> transpose() -> return val
template <class T>
std::vector<T> rowmajor_to_colmajor_array(exrpc_ptr_t& d_ptr) {
  auto &rmat = *reinterpret_cast<rowmajor_matrix<T>*>(d_ptr);
  return rmat.gather().transpose().val;
}

// returns a colmajor array from a exrpc::colmajor_matrix<T> data
template <class T>
std::vector<T> colmajor_to_colmajor_array(exrpc_ptr_t& d_ptr) {
  auto rmat = to_rowmajor_matrix<T,colmajor_matrix<T>>(d_ptr); //dummy_mat 
  auto ret = rowmajor_to_colmajor_array<T>(rmat.mptr);
  delete reinterpret_cast<rowmajor_matrix<T>*>(rmat.mptr);
  return ret;
}

// returns a colmajor array from a exrpc::blockcyclic_matrix<T> data
template <class T>
std::vector<T> blockcyclic_to_colmajor_array(exrpc_ptr_t& d_ptr) {
  auto rmat = to_rowmajor_matrix<T,blockcyclic_matrix<T>>(d_ptr); //dummy_mat 
  auto ret = rowmajor_to_colmajor_array<T>(rmat.mptr);
  delete reinterpret_cast<rowmajor_matrix<T>*>(rmat.mptr);
  return ret;
}

template <class MATRIX>
dummy_matrix transpose_matrix(exrpc_ptr_t& d_ptr) {
  auto matp = reinterpret_cast<MATRIX*>(d_ptr);
  // MATRIX should have the transpose() method...
  auto retp = new MATRIX(matp->transpose());
  if(!retp) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
  return to_dummy_matrix<MATRIX>(retp); 
}

// MATRIX should have copy-constructor implemented for deepcopy
template <class MATRIX>
dummy_matrix copy_matrix(exrpc_ptr_t& d_ptr) {
  auto matp = reinterpret_cast<MATRIX*>(d_ptr);
  MATRIX &mat = *matp;
  auto retp = new MATRIX(mat); // copy constructor
  if(!retp) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
  return to_dummy_matrix<MATRIX>(retp); 
}

// --- (B) Frovedis Models Handling (print, delete, set threshold) ---
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

// --- (C) Broadcasted Model Handling ---
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

// --- (C) Prediction related functions on glm ---
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

// --- (D) Prediction related functions on mfm ---
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

// --- (E) Prediction related functions on kmeans model ---
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

// --- (F) Load and Save Models ---
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

// --- (G) Handling of Training input/output ---
template <class T>
inline void clear_mat_data(crs_matrix_local<T>& data) {
  std::vector<T> tmp1;      tmp1.swap(data.val);
  std::vector<size_t> tmp2; tmp2.swap(data.idx);
  std::vector<size_t> tmp3; tmp3.swap(data.off);
}

template <class T>
inline void clear_lbl_data(std::vector<T>& data) {
  std::vector<T> tmp; tmp.swap(data);
}

template <class MODEL>
inline void handle_trained_model(int mid, MODEL_KIND mkind, MODEL& model) {
#ifdef _EXRPC_DEBUG_
  std::cout << "training request [" << mid << "] is processed at Frovedis side.\n";
#endif
  auto mptr = new MODEL(std::move(model));
  if(!mptr) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
  auto mptr_ = reinterpret_cast<exrpc_ptr_t>(mptr);
  register_model(mid, mkind, mptr_); // registering the trained model in model_table
  unregister_from_train(mid); // and unregistering it from "under training" state
}

// --- (H) Frovedis ML Trainer Calls ---
template <class T>
void transform_output_label(std::vector<T>& val) {
  size_t sz = val.size();
  T* valp = &val[0];
  for(size_t i=0; i<sz; ++i) valp[i] = (valp[i] == (T)0) ? -1 : valp[i];
}

template <class T, class MATRIX>
void frovedis_lr_sgd(frovedis_mem_pair& mp, int& numIter, T& stepSize, 
                   T& mbf, int& regType, T& regParam, 
                   bool& isIntercept, double& tol, int& verbose, int& mid, 
                   bool& isMovableInput=false) {
  register_for_train(mid);  // mark model 'mid' as "under training"
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T> lbl = *reinterpret_cast<dvector<T>*>(mp.second()); // copying 
  lbl.mapv_partitions(transform_output_label<T>); // transforming copied dvector
  logistic_regression_model<T> m;      // trained output model holder

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  frovedis::RegType rtype = ZERO;
  if (regType == 1) rtype = L1;
  else if (regType == 2) rtype = L2;

  if (isMovableInput) {
    m = logistic_regression_with_sgd::train(std::move(mat),lbl,numIter,stepSize,
                                            mbf,regParam,rtype,isIntercept,tol);
    lbl.mapv_partitions(clear_lbl_data<T>); 
  }
  else m = logistic_regression_with_sgd::train(mat,lbl,numIter,stepSize,mbf,
                                               regParam,rtype,isIntercept,tol);
  frovedis::set_loglevel(old_level);
  handle_trained_model<logistic_regression_model<T>>(mid, LRM, m);
}

template <class T, class MATRIX>
void frovedis_lr_lbfgs(frovedis_mem_pair& mp, int& numIter, T& stepSize, 
                     int& histSize, int& regType, T& regParam, 
                     bool& isIntercept, double& tol, int& verbose, int& mid,
                     bool& isMovableInput=false) {
  register_for_train(mid);  // mark model 'mid' as "under training"
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T> lbl = *reinterpret_cast<dvector<T>*>(mp.second()); // copying 
  lbl.mapv_partitions(transform_output_label<T>); // transforming copied dvector
  logistic_regression_model<T> m;                // trained output model holder

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  frovedis::RegType rtype = ZERO;
  if (regType == 1) rtype = L1;
  else if (regType == 2) rtype = L2;

  if (isMovableInput) {
    m = logistic_regression_with_lbfgs::train(std::move(mat),lbl,numIter,stepSize,
                                           histSize,regParam,rtype,
                                           isIntercept,tol);
    mat.data.mapv(clear_mat_data<T>); // is a no-operation, if already freed.
    lbl.mapv_partitions(clear_lbl_data<T>);
  }
  else m = logistic_regression_with_lbfgs::train(mat,lbl,numIter,stepSize,
                                              histSize,regParam,rtype,
                                              isIntercept,tol);
  frovedis::set_loglevel(old_level);
  handle_trained_model<logistic_regression_model<T>>(mid, LRM, m);
}

template <class T, class MATRIX>
void frovedis_svm_sgd(frovedis_mem_pair& mp, int& numIter, T& stepSize, 
                    T& mbf, int& regType, T& regParam, 
                    bool& isIntercept, double& tol, int& verbose, int& mid,
                    bool& isMovableInput=false) {
  register_for_train(mid);  // mark model 'mid' as "under training"
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T> lbl = *reinterpret_cast<dvector<T>*>(mp.second()); // copying 
  lbl.mapv_partitions(transform_output_label<T>); // transforming copied dvector
  svm_model<T> m;  // trained output model holder

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  frovedis::RegType rtype = ZERO;
  if (regType == 1) rtype = L1;
  else if (regType == 2) rtype = L2;

  if (isMovableInput) {
    m = svm_with_sgd::train(std::move(mat),lbl,numIter,stepSize,
                            mbf,regParam,rtype,isIntercept,tol);
    lbl.mapv_partitions(clear_lbl_data<T>); 
  }
  else m = svm_with_sgd::train(mat,lbl,numIter,stepSize,
                               mbf,regParam,rtype,isIntercept,tol);
  frovedis::set_loglevel(old_level);
  handle_trained_model<svm_model<T>>(mid, SVM, m);
}

template <class T, class MATRIX>
void frovedis_svm_lbfgs(frovedis_mem_pair& mp, int& numIter, T& stepSize, 
                      int& histSize, int& regType, T& regParam, 
                      bool& isIntercept, double& tol, int& verbose, int& mid,
                      bool& isMovableInput=false) {
  register_for_train(mid);  // mark model 'mid' as "under training"
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T> lbl = *reinterpret_cast<dvector<T>*>(mp.second()); // copying 
  lbl.mapv_partitions(transform_output_label<T>); // transforming copied dvector
  svm_model<T> m;  // trained output model holder

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  frovedis::RegType rtype = ZERO;
  if (regType == 1) rtype = L1;
  else if (regType == 2) rtype = L2;

  if (isMovableInput) {
    m = svm_with_lbfgs::train(std::move(mat),lbl,numIter,stepSize,
                              histSize,regParam,rtype,isIntercept,tol);
    mat.data.mapv(clear_mat_data<T>); // is a no-operation, if already freed.
    lbl.mapv_partitions(clear_lbl_data<T>);
  }
  else m = svm_with_lbfgs::train(mat,lbl,numIter,stepSize,
                                 histSize,regParam,rtype,isIntercept,tol);
  frovedis::set_loglevel(old_level);
  handle_trained_model<svm_model<T>>(mid, SVM, m);
}

template <class T, class MATRIX>
void frovedis_lnr_sgd(frovedis_mem_pair& mp, int& numIter, T& stepSize, 
                    T& mbf,
                    bool& isIntercept, int& verbose, int& mid, 
                    bool& isMovableInput=false) {
  register_for_train(mid);  // mark model 'mid' as "under training"
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T> lbl = *reinterpret_cast<dvector<T>*>(mp.second()); // copying 
  lbl.mapv_partitions(transform_output_label<T>); // transforming copied dvector
  linear_regression_model<T> m;   // trained output model holder

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  if (isMovableInput) {
    m = linear_regression_with_sgd::train(std::move(mat),lbl,numIter,stepSize,
                                          mbf,isIntercept);
    lbl.mapv_partitions(clear_lbl_data<T>); 
  }
  else m = linear_regression_with_sgd::train(mat,lbl,numIter,stepSize,
                                             mbf,isIntercept);
  frovedis::set_loglevel(old_level);
  handle_trained_model<linear_regression_model<T>>(mid, LNRM, m);
}

template <class T, class MATRIX>
void frovedis_lnr_lbfgs(frovedis_mem_pair& mp, int& numIter, T& stepSize, 
                      int& histSize,
                      bool& isIntercept, int& verbose, int& mid, 
                      bool& isMovableInput=false) {
  register_for_train(mid);  // mark model 'mid' as "under training"
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T> lbl = *reinterpret_cast<dvector<T>*>(mp.second()); // copying 
  lbl.mapv_partitions(transform_output_label<T>); // transforming copied dvector
  linear_regression_model<T> m;   // trained output model holder

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  if (isMovableInput) {
    m = linear_regression_with_lbfgs::train(std::move(mat),lbl,numIter,stepSize,
                                            histSize,isIntercept);
    mat.data.mapv(clear_mat_data<T>); // is a no-operation, if already freed.
    lbl.mapv_partitions(clear_lbl_data<T>);
  }
  else m = linear_regression_with_lbfgs::train(mat,lbl,numIter,stepSize,
                                               histSize,isIntercept);
  frovedis::set_loglevel(old_level);
  handle_trained_model<linear_regression_model<T>>(mid, LNRM, m);
}

template <class T, class MATRIX>
void frovedis_lasso_sgd(frovedis_mem_pair& mp, int& numIter, T& stepSize, 
                      T& mbf, T& regParam,
                      bool& isIntercept, double& tol, int& verbose, int& mid, 
                      bool& isMovableInput=false) {
  register_for_train(mid);  // mark model 'mid' as "under training"
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T> lbl = *reinterpret_cast<dvector<T>*>(mp.second()); // copying 
  lbl.mapv_partitions(transform_output_label<T>); // transforming copied dvector
  linear_regression_model<T> m;   // trained output model holder

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  if (isMovableInput) {
    m = lasso_with_sgd::train(std::move(mat),lbl,numIter,stepSize,
                              mbf,regParam,isIntercept,tol);
    lbl.mapv_partitions(clear_lbl_data<T>); 
  }
  else m = lasso_with_sgd::train(mat,lbl,numIter,stepSize,
                                 mbf,regParam,isIntercept,tol);
  frovedis::set_loglevel(old_level);
  handle_trained_model<linear_regression_model<T>>(mid, LNRM, m);
}

template <class T, class MATRIX>
void frovedis_lasso_lbfgs(frovedis_mem_pair& mp, int& numIter, T& stepSize, 
                        int& histSize, T& regParam,
                        bool& isIntercept, double& tol, int& verbose, int& mid, 
                        bool& isMovableInput=false) {
  register_for_train(mid);  // mark model 'mid' as "under training"
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T> lbl = *reinterpret_cast<dvector<T>*>(mp.second()); // copying 
  lbl.mapv_partitions(transform_output_label<T>); // transforming copied dvector
  linear_regression_model<T> m;   // trained output model holder

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  if (isMovableInput) {
    m = lasso_with_lbfgs::train(std::move(mat),lbl,numIter,stepSize,
                                histSize,regParam,isIntercept,tol);
    mat.data.mapv(clear_mat_data<T>); // is a no-operation, if already freed.
    lbl.mapv_partitions(clear_lbl_data<T>);
  }
  else m = lasso_with_lbfgs::train(mat,lbl,numIter,stepSize,
                                   histSize,regParam,isIntercept,tol);
  frovedis::set_loglevel(old_level);
  handle_trained_model<linear_regression_model<T>>(mid, LNRM, m);
}

template <class T, class MATRIX>
void frovedis_ridge_sgd(frovedis_mem_pair& mp, int& numIter, T& stepSize, 
                      T& mbf, T& regParam, 
                      bool& isIntercept, double& tol, int& verbose, int& mid,
                      bool& isMovableInput=false) {
  register_for_train(mid);  // mark model 'mid' as "under training"
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T> lbl = *reinterpret_cast<dvector<T>*>(mp.second()); // copying 
  lbl.mapv_partitions(transform_output_label<T>); // transforming copied dvector
  linear_regression_model<T> m;   // trained output model holder

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  if (isMovableInput) {
    m = ridge_regression_with_sgd::train(std::move(mat),lbl,numIter,stepSize,
                                         mbf,regParam,isIntercept,tol);
    lbl.mapv_partitions(clear_lbl_data<T>); 
  }
  else m = ridge_regression_with_sgd::train(mat,lbl,numIter,stepSize,
                                            mbf,regParam,isIntercept,tol);
  frovedis::set_loglevel(old_level);
  handle_trained_model<linear_regression_model<T>>(mid, LNRM, m);
}

template <class T, class MATRIX>
void frovedis_ridge_lbfgs(frovedis_mem_pair& mp, int& numIter, T& stepSize, 
                        int& histSize, T& regParam,
                        bool& isIntercept, double& tol, int& verbose, int& mid,
                        bool& isMovableInput=false) {
  register_for_train(mid);  // mark model 'mid' as "under training"
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T> lbl = *reinterpret_cast<dvector<T>*>(mp.second()); // copying 
  lbl.mapv_partitions(transform_output_label<T>); // transforming copied dvector
  linear_regression_model<T> m;   // trained output model holder

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  if (isMovableInput) {
    m = ridge_regression_with_lbfgs::train(std::move(mat),lbl,numIter,stepSize,
                                           histSize,regParam,isIntercept,tol);
    mat.data.mapv(clear_mat_data<T>); // is a no-operation, if already freed.
    lbl.mapv_partitions(clear_lbl_data<T>);
  }
  else m = ridge_regression_with_lbfgs::train(mat,lbl,numIter,stepSize,
                                              histSize,regParam,isIntercept,tol);
  frovedis::set_loglevel(old_level);
  handle_trained_model<linear_regression_model<T>>(mid, LNRM, m);
}

template <class T, class MATRIX>
void frovedis_mf_als(exrpc_ptr_t& data_ptr, int& rank, int& numIter, T& alpha,
                   T& regParam, long& seed, int& verbose, int& mid, 
                   bool& isMovableInput=false) {
  register_for_train(mid);   // mark model 'mid' as "under training"
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);   // training input data holder
  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  auto m = matrix_factorization_using_als::train(mat,rank,numIter,
                                                 alpha,regParam,seed);
  // if input is movable, destroying Frovedis side data after training is done.
  if (isMovableInput)  mat.data.mapv(clear_mat_data<T>); 
  frovedis::set_loglevel(old_level);
  handle_trained_model<matrix_factorization_model<T>>(mid, MFM, m);
}

template <class T, class MATRIX>
void frovedis_kmeans(exrpc_ptr_t& data_ptr, int& k, int& numIter, long& seed,
                   T& epsilon, int& verbose, int& mid, 
                   bool& isMovableInput=false) {
  register_for_train(mid);   // mark model 'mid' as "under training"
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);  // training input data holder
  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  auto m = frovedis::kmeans(mat,k,numIter,epsilon,seed);
  // if input is movable, destroying Frovedis side data after training is done.
  if (isMovableInput)  mat.data.mapv(clear_mat_data<T>); 
  frovedis::set_loglevel(old_level);
  handle_trained_model<rowmajor_matrix_local<T>>(mid, KMEANS, m);
}

template <class T, class MATRIX>
gesvd_result frovedis_sparse_svd(exrpc_ptr_t& data_ptr, int& k, 
                               bool& isMovableInput=false) {
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);      
  int m = mat.num_row;
  int n = mat.num_col;
  int info = 0; // currently frovedis::sparse_svd() doesn't return any return status
  colmajor_matrix<T> u, v;
  diag_matrix_local<T> s;
#if defined(_SX) || defined(__ve__)
  frovedis::sparse_svd<jds_crs_hybrid<T>,jds_crs_hybrid_local<T>>(mat,u,s,v,k);
#else
  frovedis::sparse_svd<T>(mat,u,s,v,k);
#endif
  // if input is movable, destroying Frovedis side data after computation is done.
  if (isMovableInput)  mat.data.mapv(clear_mat_data<T>); 
  auto svecp = reinterpret_cast<exrpc_ptr_t>(new std::vector<T>(std::move(s.val)));
  auto umatp = reinterpret_cast<exrpc_ptr_t>(new colmajor_matrix<T>(std::move(u)));
  auto vmatp = reinterpret_cast<exrpc_ptr_t>(new colmajor_matrix<T>(std::move(v)));
  return gesvd_result(svecp,umatp,vmatp,m,n,k,info);
}

// loads svd result from files
// u_file/v_file is loaded as colmajor_matrix<T> (lapack/arpack)
// if wantU/wantV is true, else they are ignored
template <class T>
gesvd_result load_cmm_svd_results(std::string& s_file,
                                  std::string& u_file,
                                  std::string& v_file,
                                  bool& wantU, bool& wantV,
                                  bool& isbinary) {
  diag_matrix_local<T> s;
  if(isbinary) s = make_diag_matrix_local_loadbinary<T>(s_file);
  else s = make_diag_matrix_local_load<T>(s_file);
  int m = 0, n = 0; // (ignored at client side if wantU/wantV is false)
  int k = s.local_num();
  auto svecp_ = reinterpret_cast<exrpc_ptr_t>(new std::vector<T>(std::move(s.val)));
  exrpc_ptr_t umatp_ = -1, vmatp_ = -1; 
  if(wantU) {
    auto tmp = load_cmm_matrix<T>(u_file, isbinary);
    umatp_ = tmp.mptr;
    m = tmp.nrow;
  }
  if(wantV) {
    auto tmp = load_cmm_matrix<T>(v_file, isbinary);
    vmatp_ = tmp.mptr;
    n = tmp.nrow;  
  }
  // "info"(0) is insignificant while loading svd result
  return gesvd_result(svecp_,umatp_,vmatp_,m,n,k,0);
}

// loads svd result from files
// u_file/v_file is loaded as blockcyclic_matrix<T>(scalapack)
// if wantU/wantV is true, else they are ignored
template <class T>
gesvd_result load_bcm_svd_results(std::string& s_file,
                                  std::string& u_file,
                                  std::string& v_file,
                                  bool& wantU, bool& wantV,
                                  bool& isbinary) {
  diag_matrix_local<T> s;
  if(isbinary) s = make_diag_matrix_local_loadbinary<T>(s_file);
  else s = make_diag_matrix_local_load<T>(s_file);
  int m = 0, n = 0; // (ignored at client side if wantU/wantV is false)
  int k = s.local_num();
  auto svecp_ = reinterpret_cast<exrpc_ptr_t>(new std::vector<T>(std::move(s.val)));
  exrpc_ptr_t umatp_ = -1, vmatp_ = -1;
  if(wantU) {
    auto tmp = load_bcm_matrix<T>(u_file, isbinary);
    umatp_ = tmp.mptr;
    m = tmp.nrow;
  }
  if(wantV) {
    auto tmp = load_bcm_matrix<T>(v_file, isbinary);
    vmatp_ = tmp.mptr;
    n = tmp.nrow; 
  }
  // "info"(0) is insignificant while loading svd result
  return gesvd_result(svecp_,umatp_,vmatp_,m,n,k,0);
}

#endif
