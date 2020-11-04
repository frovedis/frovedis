// ---------------------------------------------------------------------
// NOTE: This file contains completely template-based routines.
// Based on the input argumnet type, e.g., float/double (DT1/DT2)
// sparse/dense (S_MAT1/D_MAT1) the template call will be deduced.
// thus during the support of float type or dense type data, no major
// changes need to be performed in this file.
// ---------------------------------------------------------------------

#ifndef _EXRPC_DS_HPP_
#define _EXRPC_DS_HPP_

#include "frovedis.hpp"
#include "frovedis/matrix/crs_matrix.hpp"
#include "frovedis/matrix/diag_matrix.hpp"
#include "frovedis/matrix/blockcyclic_matrix.hpp"
#include "frovedis/dataframe/dftable.hpp"
#include "frovedis/dataframe/dfoperator.hpp"
#include "../exrpc/exrpc_expose.hpp"
#include "frovedis_mem_pair.hpp"
#include "dummy_matrix.hpp"

using namespace frovedis;

// --- Functions to enable ML data transfer and handling the same ---
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
void do_sort(std::vector<T>& vec) {
  radix_sort(vec, false);
}

template <class T>
std::vector<T>
get_unique(dvector<T>& dvec) {
  return dvec.as_node_local()
             .mapv(do_sort<T>)
             .map(set_unique<T>)
             .reduce(set_union<T>);
}

template <class T>
int count_distinct(exrpc_ptr_t& dptr) {
  auto& dvec = *reinterpret_cast<dvector<T>*>(dptr);
  return get_unique(dvec).size();
}

template <class T>
std::vector<T> get_distinct_elements(exrpc_ptr_t& dptr) {
  auto& dvec = *reinterpret_cast<dvector<T>*>(dptr);
  return get_unique(dvec);
}

template <class T>
exrpc_ptr_t get_encoded_dvector(exrpc_ptr_t& dptr, 
                                std::vector<T>& src,
                                std::vector<T>& target) {
  auto& dvec = *reinterpret_cast<dvector<T>*>(dptr);
  // assumption: 'src' vector should contain all unique elements in dvector 'dvec'
  dftable left, right;
  left.append_column("labels", dvec);
  right.append_column("src",make_dvector_scatter(src)); // original data, like {10, 20, 30, 40}
  right.append_column("target",make_dvector_scatter(target)); // target data, like {0, 1, 2, 3}
  auto joined = left.bcast_join(right, eq("labels", "src")); // encoding by joining
  auto encoded = new dvector<T>(joined.as_dvector<T>("target")); // from stack to heap
  return reinterpret_cast<exrpc_ptr_t>(encoded); // making a handle
}

template <class T, class R>
std::vector<R> cast_type(std::vector<T>& vec) {
  std::vector<R> ret(vec.size());
  auto vptr = vec.data();
  auto rptr = ret.data();
  for(size_t i = 0; i < vec.size(); ++i) {
    rptr[i] = static_cast<R>(vptr[i]);
  }
  return ret;
} 

// sorted unique elements in input dvector will be encodes by {0, 1, 2, ...}
template <class T>
exrpc_ptr_t get_encoded_dvector_zero_based(exrpc_ptr_t& dptr) { 
  auto& dvec = *reinterpret_cast<dvector<T>*>(dptr);
  dftable left;
  left.append_column("labels", dvec);
  std::vector<std::string> target = {std::string("labels")};
  auto right = left.group_by(target).select(target)
                   .rename("labels", "src")
                   .sort("src")
                   .append_rowid("target"); // zero based ids (col type: size_t)
  auto joined = left.bcast_join(right, eq("labels", "src")); // encoding by joining
  auto encoded = joined.as_dvector<size_t>("target"); 
  // size_t type is casted as T and move casted data to heap from stack
  auto casted_encoded_ptr = new dvector<T>(encoded.map_partitions(cast_type<size_t, T>));
  return reinterpret_cast<exrpc_ptr_t>(casted_encoded_ptr); // making a handle
}

template <class T>
dummy_vector create_frovedis_vector(std::vector<T>& vec, short& dtype) {
  auto vecp = new std::vector<T>(std::move(vec)); 
  auto vecp_ = reinterpret_cast<exrpc_ptr_t>(vecp);
  return dummy_vector(vecp_, vecp->size(), dtype);
}

template <class T>
void save_frovedis_vector(exrpc_ptr_t& vecptr,
                          std::string& filename,
                          bool& isBinary) {
  auto& vec = *reinterpret_cast<std::vector<T>*>(vecptr);
  if(isBinary) make_dvector_scatter(vec).savebinary(filename);
  else make_dvector_scatter(vec).saveline(filename);
}

template <class T>
dummy_vector load_frovedis_vector(std::string& filename,
                                  bool& isBinary, 
                                  short& dtype) {
  dvector<T> dv;
  if(isBinary) dv = make_dvector_loadbinary<T>(filename);
  else dv = make_dvector_loadline<T>(filename);
  auto vecp = new std::vector<T>(dv.gather());
  auto vecp_ = reinterpret_cast<exrpc_ptr_t>(vecp);
  return dummy_vector(vecp_, vecp->size(), dtype);
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

// input arr[exrpc::crs_matrix_local<T,I,O>] <= loaded from spark worker data
// returns exrpc::crs_matrix<T,I,O> from array of exrpc::crs_matrix_local<T,I,O>
template <class T, class I=size_t, class O=size_t>
exrpc_ptr_t
create_crs_data(std::vector<exrpc_ptr_t>& mat_eps,
                size_t& nrows, size_t& ncols) {
  return create_and_set_data<crs_matrix<T,I,O>,
         crs_matrix_local<T,I,O>>(mat_eps,nrows,ncols);
}

// spmv
template <class T, class I=size_t, class O=size_t>
std::vector<T> exrpc_spmv(crs_matrix_local<T,I,O>& mat, 
                          std::vector<T>& vec) {
  return mat * vec;
}

template <class T, class I=size_t, class O=size_t>
dummy_vector get_computed_spmv(exrpc_ptr_t& dptr, 
                               exrpc_ptr_t& vptr,
                               short& dtype) {
  auto& mat = *reinterpret_cast<crs_matrix<T,I,O>*>(dptr);
  auto& vec = *reinterpret_cast<std::vector<T>*>(vptr);
  auto res = mat.data.map(exrpc_spmv<T,I,O>, broadcast(vec))
                .template moveto_dvector<T>().gather(); // stack
  auto ret = new std::vector<T>(std::move(res)); // stack to heap
  auto ret_ = reinterpret_cast<exrpc_ptr_t>(ret);
  return dummy_vector(ret_, ret->size(), dtype);
}

// to_scipy_matrix()
template <class T, class I=size_t, class O=size_t>
crs_matrix_local<T,I,O> get_crs_matrix_local(exrpc_ptr_t& d_ptr) {
  auto matp = reinterpret_cast<crs_matrix_local<T,I,O>*>(d_ptr);
  return *matp;
}

// reurns the exrpc::pointer of the input local matrix
template <class T, class I=size_t, class O=size_t>
exrpc_ptr_t get_crs_matrix_local_pointer(crs_matrix_local<T,I,O>& lm) {
  auto matp = &lm;
  return reinterpret_cast<exrpc_ptr_t>(matp);
}

// returns all the local data pointers of the input MATRIX data
template <class T, class I=size_t, class O=size_t>
std::vector<exrpc_ptr_t>
get_all_crs_matrix_local_pointers(exrpc_ptr_t& d_ptr) {
  auto matp = reinterpret_cast<crs_matrix<T,I,O>*>(d_ptr);
  return matp->data.map(get_crs_matrix_local_pointer<T,I,O>).gather();
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

#if 0
template <class L_MATRIX>
size_t get_local_val_size(L_MATRIX& lmat) { return lmat.val.size(); }

template <class T>
T sum_size(T& x, T& y) { return x + y; }

// converts exrpc::matrix to exrpc::dummy_matrix
template <class MATRIX, class L_MATRIX>
dummy_matrix to_dummy_matrix(MATRIX* mptr) {
  size_t nr = mptr->num_row;
  size_t nc = mptr->num_col;
  size_t n_nz = mptr->data.map(get_local_val_size<L_MATRIX>).reduce(sum_size);
  auto mptr_ = reinterpret_cast<exrpc_ptr_t>(mptr);
#ifdef _EXRPC_DEBUG_
  show_data<MATRIX>(mptr_);
#endif
  //return dummy_matrix(mptr_,nr,nc);
  return dummy_matrix(mptr_,nr,nc,n_nz);
}
#endif

// creates crs_matrix from node local coo vector strings
template <class T, class I=size_t, class O=size_t>
dummy_matrix 
create_crs_from_local_coo_string_vectors(std::vector<exrpc_ptr_t>& vec_eps) {
  auto lvs = make_node_local_allocate<std::vector<std::string>>();
  auto vec_ep_bcast = broadcast(vec_eps);
  // after crs_matrix creation local string vectors are to be freed
  auto is_to_be_moved = broadcast(true); 
  lvs.mapv(set_vector_data<std::string>,vec_ep_bcast,is_to_be_moved);
  bool zero_origin = false; // spark user_id starts with 1
  auto matp = new crs_matrix<T,I,O>(make_crs_matrix_loadcoo<T,I,O>(lvs,zero_origin));
  if(!matp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed!\n");
  return to_dummy_matrix<crs_matrix<T,I,O>, crs_matrix_local<T,I,O>>(matp);
}

// loads data from specified file/dir and creates a exrpc::crs_matrix<T,I,O>
template <class T, class I=size_t, class O=size_t>
dummy_matrix load_crs_matrix(std::string& path, bool& isbinary) {
  crs_matrix<T,I,O> *matp = NULL;
  if(isbinary)
    matp = new crs_matrix<T,I,O>(make_crs_matrix_loadbinary<T,I,O>(path)); //rvalue
  else
    matp = new crs_matrix<T,I,O>(make_crs_matrix_load<T,I,O>(path)); //rvalue
  if(!matp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed!\n");
  return to_dummy_matrix<crs_matrix<T,I,O>, crs_matrix_local<T,I,O>>(matp);
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
  return to_dummy_matrix<rowmajor_matrix<T>, rowmajor_matrix_local<T>>(matp);
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
  return to_dummy_matrix<colmajor_matrix<T>, colmajor_matrix_local<T>>(matp);
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
  return to_dummy_matrix<blockcyclic_matrix<T>, blockcyclic_matrix_local<T>>(matp);
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
  return to_dummy_matrix<rowmajor_matrix<T>, rowmajor_matrix_local<T>>(rmatp);
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
  return to_dummy_matrix<colmajor_matrix<T>, rowmajor_matrix_local<T>>(cmatp); 
}

// converts exrpc::rowmajor_matrix<T> to exrpc::colmajor_matrix<T>
template <class T>
dummy_matrix rowmajor_to_colmajor_matrix(exrpc_ptr_t& d_ptr) {
  auto rmatp = reinterpret_cast<rowmajor_matrix<T>*>(d_ptr);
  auto &rmat = *rmatp;
  auto cmatp = new colmajor_matrix<T>(rmat); 
  if(!cmatp) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
  return to_dummy_matrix<colmajor_matrix<T>, colmajor_matrix_local<T>>(cmatp); 
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

template <class MATRIX, class L_MATRIX>
dummy_matrix transpose_matrix(exrpc_ptr_t& d_ptr) {
  auto matp = reinterpret_cast<MATRIX*>(d_ptr);
  // MATRIX should have the transpose() method...
  auto retp = new MATRIX(matp->transpose());
  if(!retp) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
  return to_dummy_matrix<MATRIX, L_MATRIX>(retp); 
}

// MATRIX should have copy-constructor implemented for deepcopy
template <class MATRIX, class L_MATRIX>
dummy_matrix copy_matrix(exrpc_ptr_t& d_ptr) {
  auto matp = reinterpret_cast<MATRIX*>(d_ptr);
  MATRIX &mat = *matp;
  auto retp = new MATRIX(mat); // copy constructor
  if(!retp) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
  return to_dummy_matrix<MATRIX, L_MATRIX>(retp); 
}

template <class MATRIX, class L_MATRIX>
L_MATRIX get_global_data(exrpc_ptr_t& dptr) {
  auto& mat = *reinterpret_cast<MATRIX*>(dptr);  
  return mat.gather();
}

// returns the non-zero elements of the input local crs matrix
template <class T, class I=size_t, class O=size_t>
size_t get_local_nnz(crs_matrix_local<T,I,O>& l_mat) { 
  return l_mat.val.size(); 
}

// returns vector of non-zero elements of the input local crs matrix
template <class T, class I=size_t, class O=size_t>
std::vector<size_t>
get_all_nnz(exrpc_ptr_t& d_ptr) {
  auto& mat = *reinterpret_cast<crs_matrix<T,I,O>*>(d_ptr);
  return mat.data.map(get_local_nnz<T,I,O>).gather();
}

// returns vector of local_num_row of the input local crs matrix
template <class T, class I=size_t, class O=size_t>
std::vector<size_t>
get_all_nrow(exrpc_ptr_t& d_ptr) {
  auto& mat = *reinterpret_cast<crs_matrix<T,I,O>*>(d_ptr);
  return mat.get_local_num_rows();
}

#endif
