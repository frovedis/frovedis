#ifndef _SCALAPACK_RESULT_HPP_
#define _SCALAPACK_RESULT_HPP_

namespace frovedis {
struct getrf_result {
  getrf_result() {}
  getrf_result(exrpc_ptr_t ptr, int stat): ipiv_ptr(ptr), info(stat) {}
  
  exrpc_ptr_t ipiv_ptr;
  int info;
  SERIALIZE(ipiv_ptr, info)
};

struct gesvd_result {
  gesvd_result() {}
  gesvd_result(exrpc_ptr_t sptr, exrpc_ptr_t uptr, exrpc_ptr_t vptr,
               int mm, int nn, int kk, int stat) :
    svec_ptr(sptr), umat_ptr(uptr), vmat_ptr(vptr),
    m(mm), n(nn), k(kk), info(stat) {}

  exrpc_ptr_t svec_ptr, umat_ptr, vmat_ptr;
  int m, n, k, info;
  SERIALIZE(svec_ptr,umat_ptr,vmat_ptr,m,n,k,info)
};
}

#endif
