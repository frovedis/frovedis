#include "exrpc_svd.hpp"
#include "exrpc_pca.hpp"
#include "exrpc_pblas.hpp"
#include "exrpc_scalapack.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"

using namespace frovedis;

void expose_frovedis_wrapper_functions() {
  // --- frovedis pca ---
  expose((frovedis_pca<R_MAT1,DT1>));              // for spark + python
  expose((frovedis_pca<R_MAT2,DT2>));              // for python 
  expose((frovedis_pca_transform<R_MAT2,R_LMAT2,DT2>));
  expose((frovedis_pca_transform<R_MAT1,R_LMAT1,DT1>));
  expose((frovedis_pca_inverse_transform<R_MAT2,R_LMAT2,DT2>));
  expose((frovedis_pca_inverse_transform<R_MAT1,R_LMAT1,DT1>));
  // --- frovedis svd ---
  expose((compute_var_sum<DT1,R_MAT1>));
  expose((compute_var_sum<DT2,R_MAT2>));
  expose((frovedis_sparse_truncated_svd<S_MAT1,DT1>));       // for spark
  expose((frovedis_sparse_truncated_svd<S_MAT14,DT1,DT4>));  // for python
  expose((frovedis_sparse_truncated_svd<S_MAT15,DT1,DT5>));  // for python
  expose((frovedis_sparse_truncated_svd<S_MAT24,DT2,DT4>));  // for python
  expose((frovedis_sparse_truncated_svd<S_MAT25,DT2,DT5>));  // for python
  expose((frovedis_dense_truncated_svd<R_MAT1,DT1>));        // for spark/python
  expose((frovedis_dense_truncated_svd<R_MAT2,DT2>));        // for python
  expose((frovedis_svd_transform<R_MAT1,DT1>));              // for python (spark: to be added)
  expose((frovedis_svd_transform<R_MAT2,DT2>));              // for python
  expose((frovedis_svd_transform<S_MAT14,DT1>));             // for python
  expose((frovedis_svd_transform<S_MAT15,DT1>));             // for python (spark: to be added)
  expose((frovedis_svd_transform<S_MAT24,DT2>));             // for python
  expose((frovedis_svd_transform<S_MAT25,DT2>));             // for python
  expose((frovedis_svd_inv_transform<R_MAT1,DT1>));          // for python (spark: to be added)
  expose((frovedis_svd_inv_transform<R_MAT2,DT2>));          // for python
  expose((frovedis_svd_inv_transform<S_MAT14,DT1>));         // for python
  expose((frovedis_svd_inv_transform<S_MAT15,DT1>));         // for python (spark: to be added)
  expose((frovedis_svd_inv_transform<S_MAT24,DT2>));         // for python
  expose((frovedis_svd_inv_transform<S_MAT25,DT2>));         // for python
  expose(load_cmm_svd_results<DT1>); //GesvdResult (arpack/lapack)
  expose(load_bcm_svd_results<DT1>); //PGesvdResult (scalapack)
  expose(load_cmm_svd_results<DT2>); //GesvdResult (arpack/lapack)
  expose(load_bcm_svd_results<DT2>); //PGesvdResult (scalapack)
  // --- frovedis blas/pblas wrappers ---
  expose((frovedis_swap<DT1,C_LMAT1>));
  expose((frovedis_swap<DT1,B_MAT1>));
  expose((frovedis_swap<DT2,B_MAT2>));
  expose((frovedis_copy<DT1,C_LMAT1>));
  expose((frovedis_copy<DT1,B_MAT1>));
  expose((frovedis_copy<DT2,B_MAT2>));
  //expose((frovedis_scal<DT1,C_LMAT1>));
  expose((frovedis_scal<DT1,B_MAT1,B_LMAT1>));
  expose((frovedis_scal<DT2,B_MAT2,B_LMAT2>));
  expose((frovedis_axpy<DT1,C_LMAT1>));
  expose((frovedis_axpy<DT1,B_MAT1>));
  expose((frovedis_axpy<DT2,B_MAT2>));
  expose((frovedis_dot<DT1,C_LMAT1>));
  expose((frovedis_dot<DT1,B_MAT1>));
  expose((frovedis_dot<DT2,B_MAT2>));
  expose((frovedis_nrm2<DT1,C_LMAT1>));
  expose((frovedis_nrm2<DT1,B_MAT1>));
  expose((frovedis_nrm2<DT2,B_MAT2>));
  expose((frovedis_gemv<DT1,C_LMAT1>));
  expose((frovedis_gemv<DT1,B_MAT1>));
  expose((frovedis_gemv<DT2,B_MAT2>));
  expose((frovedis_ger<DT1,C_LMAT1>));
  expose((frovedis_ger<DT1,B_MAT1>));
  expose((frovedis_ger<DT2,B_MAT2>));
  expose((frovedis_gemm<DT1,C_LMAT1>));
  expose((frovedis_gemm<DT1,B_MAT1>));
  expose((frovedis_gemm<DT2,B_MAT2>));
  // blas doesn't have geadd() 
  expose((frovedis_geadd<DT1,B_MAT1>));
  expose((frovedis_geadd<DT2,B_MAT2>));
  // --- frovedis lapack/scalapack wrappers ---
  expose((frovedis_getrf<DT1,C_LMAT1,std::vector<int>>));
  expose((frovedis_getrf<DT1,B_MAT1,lvec<int>>));
  expose((frovedis_getrf<DT2,B_MAT2,lvec<int>>));
  expose((frovedis_getri<DT1,C_LMAT1,std::vector<int>>));
  expose((frovedis_getri<DT1,B_MAT1,lvec<int>>));
  expose((frovedis_getri<DT2,B_MAT2,lvec<int>>));
  expose((frovedis_getrs<DT1,C_LMAT1,std::vector<int>>));
  expose((frovedis_getrs<DT1,B_MAT1,lvec<int>>));
  expose((frovedis_getrs<DT2,B_MAT2,lvec<int>>));
  expose((frovedis_gesv<DT1,C_LMAT1,std::vector<int>>));
  expose((frovedis_gesv<DT1,B_MAT1,lvec<int>>));
  expose((frovedis_gesv<DT2,B_MAT2,lvec<int>>));
  expose((frovedis_gels<DT1,C_LMAT1>));
  expose((frovedis_gels<DT1,B_MAT1>));
  expose((frovedis_gels<DT2,B_MAT2>));
  expose((frovedis_gesvd<DT1,C_LMAT1>));
  expose((frovedis_gesvd<DT1,B_MAT1>));
  expose((frovedis_gesvd<DT2,B_MAT2>));
}
