#ifndef _EXRPC_PBLAS_HPP_
#define _EXRPC_PBLAS_HPP_

#include "frovedis/matrix/blas_wrapper.hpp"
#include "frovedis/matrix/pblas_wrapper.hpp"
#include "../exrpc/exrpc_expose.hpp"
#include "dummy_matrix.hpp"

using namespace frovedis;

// Based on MATRIX type, BLAS/PBLAS calls will be linked
// MATRIX: colmajor_matrix_local<T> => BLAS version would be called
// MATRIX: blockcyclic_matrix<T> => PBLAS version would be called

template <class T, class MATRIX>
void frovedis_swap(exrpc_ptr_t& v_ptr1, 
                 exrpc_ptr_t& v_ptr2) {
  auto v1 = reinterpret_cast<MATRIX*>(v_ptr1); 
  auto v2 = reinterpret_cast<MATRIX*>(v_ptr2); 
  MATRIX &vv1 = *v1;
  MATRIX &vv2 = *v2;
  frovedis::swap<T>(vv1,vv2);
}

template <class T, class MATRIX>
void frovedis_copy(exrpc_ptr_t& v_ptr1, 
                 exrpc_ptr_t& v_ptr2) {
  auto v1 = reinterpret_cast<MATRIX*>(v_ptr1);
  auto v2 = reinterpret_cast<MATRIX*>(v_ptr2);
  MATRIX &vv1 = *v1;
  MATRIX &vv2 = *v2;
  frovedis::copy<T>(vv1,vv2);
}

template <class T, class MATRIX>
void frovedis_scal(exrpc_ptr_t& v_ptr, 
                 T& alpha) {
  auto v = reinterpret_cast<MATRIX*>(v_ptr);
  MATRIX &vv = *v;
  frovedis::scal<T>(vv,alpha);
}

template <class T, class MATRIX>
void frovedis_axpy(exrpc_ptr_t& v_ptr1,
                 exrpc_ptr_t& v_ptr2,
                 T& alpha) {
  auto v1 = reinterpret_cast<MATRIX*>(v_ptr1);
  auto v2 = reinterpret_cast<MATRIX*>(v_ptr2);
  MATRIX &vv1 = *v1;
  MATRIX &vv2 = *v2;
  frovedis::axpy<T>(vv1,vv2,alpha);
}

template <class T, class MATRIX>
T frovedis_dot(exrpc_ptr_t& v_ptr1,
             exrpc_ptr_t& v_ptr2) {
  auto v1 = reinterpret_cast<MATRIX*>(v_ptr1);
  auto v2 = reinterpret_cast<MATRIX*>(v_ptr2);
  MATRIX &vv1 = *v1;
  MATRIX &vv2 = *v2;
  return frovedis::dot<T>(vv1,vv2);
}

template <class T, class MATRIX>
T frovedis_nrm2(exrpc_ptr_t& v_ptr) {
  auto v = reinterpret_cast<MATRIX*>(v_ptr);
  MATRIX &vv = *v;
  return frovedis::nrm2<T>(vv);
}

template <class T, class MATRIX>
dummy_matrix
frovedis_gemv(exrpc_ptr_t& m_ptr,
            exrpc_ptr_t& v_ptr,
            bool& isTrans,
            T& alpha, T& beta) {
  auto m = reinterpret_cast<MATRIX*>(m_ptr); // MxN
  auto v = reinterpret_cast<MATRIX*>(v_ptr); // Nx1
  size_t r_nr = m->get_nrows();
  size_t r_nc = 1;
  auto ret = new MATRIX(r_nr,r_nc); //Mx1
  if(!ret) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  MATRIX &mm = *m;
  MATRIX &vv = *v;
  MATRIX &rr = *ret;
  char trans = isTrans ? 'T' : 'N';
  frovedis::gemv<T>(mm,vv,rr,trans,alpha,beta);
  auto retp = reinterpret_cast<exrpc_ptr_t>(ret);
  return dummy_matrix(retp,r_nr,r_nc);
}

template <class T, class MATRIX>
dummy_matrix
frovedis_ger(exrpc_ptr_t& v_ptr1,
           exrpc_ptr_t& v_ptr2,
           T& alpha) {
  auto v1 = reinterpret_cast<MATRIX*>(v_ptr1); // Mx1
  auto v2 = reinterpret_cast<MATRIX*>(v_ptr2); // Nx1
  size_t r_nr = v1->get_nrows();
  size_t r_nc = v2->get_nrows();
  auto ret = new MATRIX(r_nr,r_nc); //MxN
  if(!ret) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  MATRIX &vv1 = *v1;
  MATRIX &vv2 = *v2;
  MATRIX &rr = *ret;
  frovedis::ger<T>(vv1,vv2,rr,alpha);
  auto retp = reinterpret_cast<exrpc_ptr_t>(ret);
  return dummy_matrix(retp,r_nr,r_nc);
}

template <class T, class MATRIX>
dummy_matrix
frovedis_gemm(exrpc_ptr_t& m_ptr1,
            exrpc_ptr_t& m_ptr2,
            bool& isTrans1, bool& isTrans2,
            T& alpha, T& beta) {
  auto m1 = reinterpret_cast<MATRIX*>(m_ptr1); // MxK
  auto m2 = reinterpret_cast<MATRIX*>(m_ptr2); // KxN
  size_t r_nr = m1->get_nrows();
  size_t r_nc = m2->get_ncols();
  auto ret = new MATRIX(r_nr,r_nc); //MxN
  if(!ret) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  MATRIX &mm1 = *m1;
  MATRIX &mm2 = *m2;
  MATRIX &rr = *ret;
  char trans1 = isTrans1 ? 'T' : 'N';
  char trans2 = isTrans2 ? 'T' : 'N';
  frovedis::gemm<T>(mm1,mm2,rr,trans1,trans2,alpha,beta);
  auto retp = reinterpret_cast<exrpc_ptr_t>(ret);
  return dummy_matrix(retp,r_nr,r_nc);
}

// pblas only routine, MATRIX: blockcyclic-matrix
template <class T, class MATRIX>
void frovedis_geadd(exrpc_ptr_t& in_mptr,
                  exrpc_ptr_t& inout_mptr,
                  bool& isTrans,
                  T& alpha, T& beta) {
  auto m1 = reinterpret_cast<MATRIX*>(in_mptr); // MxN
  auto m2 = reinterpret_cast<MATRIX*>(inout_mptr); // MxN
  MATRIX &mm1 = *m1;
  MATRIX &mm2 = *m2;
  char trans = isTrans ? 'T' : 'N';
  frovedis::geadd<T>(mm1,mm2,trans,alpha,beta);
}

#endif
