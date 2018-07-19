#ifndef _EXRPC_SCALAPACK_HPP_
#define _EXRPC_SCALAPACK_HPP_

#include "frovedis/matrix/lapack_wrapper.hpp"
#include "frovedis/matrix/scalapack_wrapper.hpp"
#include "scalapack_result.hpp"
#include "../exrpc/exrpc_expose.hpp"

using namespace frovedis;

// Based on MATRIX type, LAPACK/SCALAPACK calls will be linked
// MATRIX: colmajor_matrix_local<T> => LAPACK version would be called
// MATRIX: blockcyclic_matrix<T> => SCALAPACK version would be called

template <class T, class MATRIX, class IPIV>
getrf_result
frovedis_getrf(exrpc_ptr_t& mptr) { 
  auto m = reinterpret_cast<MATRIX*>(mptr);
  MATRIX &mm = *m;
  IPIV ff;
  auto info = frovedis::getrf<T>(mm,ff);
  auto ipivp = reinterpret_cast<exrpc_ptr_t>(new IPIV(std::move(ff)));
  return getrf_result(ipivp,info);
}

template <class T, class MATRIX, class IPIV>
int frovedis_getri(exrpc_ptr_t& mptr,
                 exrpc_ptr_t& ipiv_ptr) {
  auto m = reinterpret_cast<MATRIX*>(mptr);
  auto ipiv = reinterpret_cast<IPIV*>(ipiv_ptr);
  MATRIX &mm = *m;
  IPIV &ff = *ipiv;
  return frovedis::getri<T>(mm,ff);
}

template <class T, class MATRIX, class IPIV>
int frovedis_getrs(exrpc_ptr_t& mptrA,
                 exrpc_ptr_t& mptrB,
                 exrpc_ptr_t& ipiv_ptr,
                 bool& isTrans) {
  auto ma = reinterpret_cast<MATRIX*>(mptrA);
  auto mb = reinterpret_cast<MATRIX*>(mptrB);
  auto ipiv = reinterpret_cast<IPIV*>(ipiv_ptr);
  MATRIX &mma = *ma;
  MATRIX &mmb = *mb;
  IPIV &ff = *ipiv;
  char trans = isTrans ? 'T' : 'N';
  return frovedis::getrs<T>(mma,mmb,ff,trans);
}

template <class T, class MATRIX, class IPIV>
int frovedis_gesv(exrpc_ptr_t& mptrA,
                exrpc_ptr_t& mptrB) {
  auto ma = reinterpret_cast<MATRIX*>(mptrA);
  auto mb = reinterpret_cast<MATRIX*>(mptrB);
  MATRIX &mma = *ma;
  MATRIX &mmb = *mb;
  IPIV ff; //local IPIV, not returning
  return frovedis::gesv<T>(mma,mmb,ff);
}

template <class T, class MATRIX>
int frovedis_gels(exrpc_ptr_t& mptrA,
                exrpc_ptr_t& mptrB,
                bool& isTrans) {
  auto ma = reinterpret_cast<MATRIX*>(mptrA);
  auto mb = reinterpret_cast<MATRIX*>(mptrB);
  MATRIX &mma = *ma;
  MATRIX &mmb = *mb;
  char trans = isTrans ? 'T' : 'N';
  return frovedis::gels<T>(mma,mmb,trans);
}

template <class T, class MATRIX>
gesvd_result
frovedis_gesvd(exrpc_ptr_t& mptr,
             bool& wantU,
             bool& wantV) {
  auto mat = reinterpret_cast<MATRIX*>(mptr);
  int m = mat->get_nrows();
  int n = mat->get_ncols();
  int k = std::min(m,n);
  MATRIX *umat = NULL, *vmat = NULL;
  int info = 0;

  std::vector<T> ss; //SVAL
  MATRIX &mm = *mat;

  if(!wantU && !wantV) {
    info = frovedis::gesvd<T>(mm,ss);
  }
  else if(wantU && !wantV) {
    umat = new MATRIX(m,k);
    if(!umat) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
    MATRIX &uu = *umat;
    info = frovedis::gesvd<T>(mm,ss,uu,'L');
  }
  else if(!wantU && wantV) {
    vmat = new MATRIX(k,n);
    if(!vmat) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
    MATRIX &vv = *vmat;
    info = frovedis::gesvd<T>(mm,ss,vv,'R');
  }
  else {
    umat = new MATRIX(m,k);
    if(!umat) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
    vmat = new MATRIX(k,n);
    if(!vmat) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
    MATRIX &uu = *umat;
    MATRIX &vv = *vmat;
    info = frovedis::gesvd<T>(mm,ss,uu,vv);
  }

  if(wantV) { // need to transpose vmat, since lapack/scalapack outputs VT
    auto t_vmat = vmat;
    vmat = new MATRIX(vmat->transpose());
    delete t_vmat;
  }

  auto svecp = reinterpret_cast<exrpc_ptr_t>(new std::vector<T>(std::move(ss)));
  auto umatp = wantU ? reinterpret_cast<exrpc_ptr_t>(umat) : -1;
  auto vmatp = wantV ? reinterpret_cast<exrpc_ptr_t>(vmat) : -1;
  return gesvd_result(svecp,umatp,vmatp,m,n,k,info);
}

#endif
