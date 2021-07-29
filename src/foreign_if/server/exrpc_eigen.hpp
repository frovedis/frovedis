#ifndef _EXRPC_EIGEN_HPP_
#define _EXRPC_EIGEN_HPP_

#include "frovedis.hpp"
#include "frovedis/matrix/dense_eigen.hpp"
#include "frovedis/matrix/shrink_sparse_eigen.hpp"
#include "ml_result.hpp"
#include "../exrpc/exrpc_expose.hpp"

using namespace frovedis;

template <class MATRIX, class T>
eigen_result 
frovedis_dense_eigsh(exrpc_ptr_t& data_ptr, int& k,
                     std::string& order, float& sigma,
                     int& maxiter, float& tol,
                     bool& isMovableInput = false) {
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);      
  int m = mat.num_row;
  int n = mat.num_col;
  diag_matrix_local<T> s;
  colmajor_matrix<T> u;
  auto is_standard = sigma == std::numeric_limits<float>::max();
  if (is_standard) dense_eigen_sym(mat, s, u, order, k, maxiter, tol);
  else             dense_eigen_sym(mat, s, u, order, k, (T)sigma, true, maxiter, tol);
  // if input is movable, destroying Frovedis side data after computation is done.
  if (isMovableInput)  mat.clear(); 
  auto svecp = reinterpret_cast<exrpc_ptr_t>(new std::vector<T>(std::move(s.val)));
  auto umatp = reinterpret_cast<exrpc_ptr_t>(new colmajor_matrix<T>(std::move(u)));
  return eigen_result(svecp, umatp, m, n, k);
}

template <class MATRIX, class T, class I = size_t>
eigen_result 
frovedis_sparse_eigsh(exrpc_ptr_t& data_ptr, int& k,
                      std::string& order, 
                      int& maxiter, float& tol,
                      bool& isMovableInput=false) {
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);
  int m = mat.num_row;
  int n = mat.num_col;
  colmajor_matrix<T> u;
  diag_matrix_local<T> s;
#if defined(_SX) || defined(__ve__)
  frovedis::shrink::sparse_eigen_sym<jds_crs_hybrid<T,I>,
                                     jds_crs_hybrid_local<T,I>>
                                     (mat, s, u, order, k, maxiter, tol);
#else
  frovedis::shrink::sparse_eigen_sym(mat, s, u, order, k, maxiter, tol);
#endif
  // if input is movable, destroying Frovedis side data after computation is done.
  if (isMovableInput)  mat.clear(); 
  auto svecp = reinterpret_cast<exrpc_ptr_t>(new std::vector<T>(std::move(s.val)));
  auto umatp = reinterpret_cast<exrpc_ptr_t>(new colmajor_matrix<T>(std::move(u)));
  return eigen_result(svecp, umatp, m, n, k);
}

#endif
