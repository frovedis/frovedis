#include "exrpc_pca.hpp"
#include "exrpc_svd.hpp"
#include "exrpc_pblas.hpp"
#include "exrpc_scalapack.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"

using namespace frovedis;

void expose_frovedis_wrapper_functions() {
  // --- frovedis pca ---
  expose((frovedis_pca<R_MAT1,DT1>));              // for spark
  // --- frovedis sparse svd ---
  expose((frovedis_sparse_svd<S_MAT1,DT1>));       // for spark
  expose((frovedis_sparse_svd<S_MAT14,DT1,DT4>));  // for python
  expose((frovedis_sparse_svd<S_MAT15,DT1,DT5>));  // for python
  expose((frovedis_sparse_svd<S_MAT24,DT2,DT4>));  // for python
  expose((frovedis_sparse_svd<S_MAT25,DT2,DT5>));  // for python
  expose(load_cmm_svd_results<DT1>); //GesvdResult (arpack/lapack)
  expose(load_bcm_svd_results<DT1>); //PGesvdResult (scalapack)
  // --- frovedis blas/pblas wrappers ---
  expose((frovedis_swap<DT1,C_LMAT1>));
  expose((frovedis_swap<DT1,B_MAT1>));
  expose((frovedis_copy<DT1,C_LMAT1>));
  expose((frovedis_copy<DT1,B_MAT1>));
  expose((frovedis_scal<DT1,C_LMAT1>));
  expose((frovedis_scal<DT1,B_MAT1>));
  expose((frovedis_axpy<DT1,C_LMAT1>));
  expose((frovedis_axpy<DT1,B_MAT1>));
  expose((frovedis_dot<DT1,C_LMAT1>));
  expose((frovedis_dot<DT1,B_MAT1>));
  expose((frovedis_nrm2<DT1,C_LMAT1>));
  expose((frovedis_nrm2<DT1,B_MAT1>));
  expose((frovedis_gemv<DT1,C_LMAT1>));
  expose((frovedis_gemv<DT1,B_MAT1>));
  expose((frovedis_ger<DT1,C_LMAT1>));
  expose((frovedis_ger<DT1,B_MAT1>));
  expose((frovedis_gemm<DT1,C_LMAT1>));
  expose((frovedis_gemm<DT1,B_MAT1>));
  // blas doesn't have geadd() 
  expose((frovedis_geadd<DT1,B_MAT1>));
  // --- frovedis lapack/scalapack wrappers ---
  expose((frovedis_getrf<DT1,C_LMAT1,std::vector<int>>));
  expose((frovedis_getrf<DT1,B_MAT1,lvec<int>>));
  expose((frovedis_getri<DT1,C_LMAT1,std::vector<int>>));
  expose((frovedis_getri<DT1,B_MAT1,lvec<int>>));
  expose((frovedis_getrs<DT1,C_LMAT1,std::vector<int>>));
  expose((frovedis_getrs<DT1,B_MAT1,lvec<int>>));
  expose((frovedis_gesv<DT1,C_LMAT1,std::vector<int>>));
  expose((frovedis_gesv<DT1,B_MAT1,lvec<int>>));
  expose((frovedis_gels<DT1,C_LMAT1>));
  expose((frovedis_gels<DT1,B_MAT1>));
  expose((frovedis_gesvd<DT1,C_LMAT1>));
  expose((frovedis_gesvd<DT1,B_MAT1>));
}
