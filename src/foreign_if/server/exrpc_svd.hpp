// ---------------------------------------------------------------------
// NOTE: This file contains completely template-based routines.
// Based on the input argumnet type, e.g., float/double (DT1/DT2)
// sparse/dense (S_MAT1/D_MAT1) the template call will be deduced.
// thus during the support of float type or dense type data, no major
// changes need to be performed in this file.
// ---------------------------------------------------------------------

#ifndef _EXRPC_SVD_HPP_
#define _EXRPC_SVD_HPP_

#include "frovedis.hpp"
#include "frovedis/matrix/sparse_svd.hpp"
#include "../exrpc/exrpc_expose.hpp"
#include "exrpc_data_storage.hpp"
#include "scalapack_result.hpp"

using namespace frovedis;

template <class MATRIX, class T, class I = size_t>
gesvd_result frovedis_sparse_svd(exrpc_ptr_t& data_ptr, int& k, 
                                 bool& isMovableInput=false) {
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);      
  int m = mat.num_row;
  int n = mat.num_col;
  int info = 0; // currently frovedis::sparse_svd() doesn't return any return status
  colmajor_matrix<T> u, v;
  diag_matrix_local<T> s;
#if defined(_SX) || defined(__ve__)
  frovedis::sparse_svd<jds_crs_hybrid<T,I>,jds_crs_hybrid_local<T,I>>(mat,u,s,v,k);
#else
  frovedis::sparse_svd(mat,u,s,v,k);
#endif
  // if input is movable, destroying Frovedis side data after computation is done.
  if (isMovableInput)  mat.clear(); 
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
