// -----------------------------------------------------------------------------
// NOTE: As per current implementation only double(DT1) version is supported.
// To support, float(DT2) version, you just need to register 
// all these exposed functions in expose_table with 
// "DT2" mentioned in the explicit templates parameters.
// Similarly, only SPARSE (crs_matrix[_local]<T>) data is supported
// for the training and prediction purposes. If the DENSE version support is 
// provided in Frovedis side, then the same can also be configurable
// just exposing the dense wrapper routines with template args "D_MAT1/D_LMAT1"
// ------------------------------------------------------------------------------

#include "exrpc_builder.hpp"
#include "exrpc_pblas.hpp"
#include "exrpc_scalapack.hpp"
#include "model_tracker.hpp"

using namespace frovedis;
using namespace std;

int main(int argc, char* argv[]) {
  frovedis::use_frovedis use(argc, argv);
  expose(get_nodesize);
  expose(cleanup_frovedis_server);
  // --- frovedis dvector ---
  expose((load_local_data<std::vector<DT1>>));
  expose(create_and_set_dvector<DT1>);
  expose(show_dvector<DT1>);
  expose(release_dvector<DT1>);
  // --- frovedis glm data ---
  expose((load_local_glm_data<DT1,S_LMAT1>));
  expose((create_and_set_glm_data<DT1,S_MAT1,S_LMAT1>));
  expose((release_glm_data<DT1,S_MAT1>));
  expose((show_glm_data<DT1,S_MAT1>));
  // --- frovedis generic data ---
  expose(load_local_data<std::vector<std::string>>);
  expose(load_local_data<S_LMAT1>);
  expose(load_local_data<R_LMAT1>);
  expose((create_and_set_data<S_MAT1,S_LMAT1>));
  expose(create_crs_from_local_coo_string_vectors<DT1>);
  expose(create_crs_data<DT1>);
  expose(create_rmm_data<DT1>);
  expose(create_cmm_data<DT1>);
  expose(create_bcm_data<DT1>);
  expose(load_crs_matrix<DT1>);
  expose(load_rmm_matrix<DT1>);
  expose(load_cmm_matrix<DT1>);
  expose(load_bcm_matrix<DT1>);
  expose(release_data<S_MAT1>);
  expose(release_data<R_MAT1>);
  expose(release_data<C_MAT1>);
  expose(release_data<B_MAT1>);
  expose((release_data<std::vector<double>>)); //SVAL
  expose((release_data<std::vector<int>>));    //LAPACK(IPIV)
  expose((release_data<lvec<int>>));           //ScaLAPACK(IPIV)
  expose(show_data<S_MAT1>);
  expose(show_data<R_MAT1>);
  expose(show_data<C_MAT1>);
  expose(show_data<B_MAT1>);
  expose(save_matrix<S_MAT1>);
  expose(save_matrix<R_MAT1>);
  expose(save_matrix<C_MAT1>);
  expose(save_matrix<B_MAT1>);
  expose(transpose_matrix<R_MAT1>);
  //expose(transpose_matrix<C_MAT1>); // not yet supported
  expose(transpose_matrix<B_MAT1>);
  //expose(copy_matrix<R_MAT1>); // not yet supported
  //expose(copy_matrix<C_MAT1>); // not yet supported
  expose(copy_matrix<B_MAT1>);
  expose((to_rowmajor_matrix<DT1,C_MAT1>));
  expose((to_rowmajor_matrix<DT1,B_MAT1>));
  expose((to_rowmajor_array<DT1,C_MAT1>));
  expose((to_rowmajor_array<DT1,B_MAT1>));
  expose(rowmajor_to_colmajor_array<DT1>);
  expose(colmajor_to_colmajor_array<DT1>);
  expose(blockcyclic_to_colmajor_array<DT1>);
  expose((matrix_to_array<DT1,R_MAT1,R_LMAT1>));
  expose((matrix_to_array<DT1,C_MAT1,C_LMAT1>));
  expose((matrix_to_array<DT1,B_MAT1,B_LMAT1>));
  expose((convert_and_get_all_rml_pointers<DT1,C_MAT1>));
  expose((convert_and_get_all_rml_pointers<DT1,B_MAT1>));
  expose((get_all_local_pointers<R_MAT1,R_LMAT1>));
  expose((get_all_local_pointers<C_MAT1,C_LMAT1>));
  expose((get_all_local_pointers<B_MAT1,B_LMAT1>));
  expose((get_local_array<DT1,R_LMAT1>));
  expose((get_local_array<DT1,C_LMAT1>));
  expose((get_local_array<DT1,B_LMAT1>));
  expose(vec_to_array<DT1>);
  expose(save_as_diag_matrix_local<DT1>);
  // --- frovedis ML trainers ---
  expose((frovedis_lr_sgd<DT1,S_MAT1>));
  expose((frovedis_lr_lbfgs<DT1,S_MAT1>));
  expose((frovedis_svm_sgd<DT1,S_MAT1>));
  expose((frovedis_svm_lbfgs<DT1,S_MAT1>));
  expose((frovedis_lnr_sgd<DT1,S_MAT1>));
  expose((frovedis_lnr_lbfgs<DT1,S_MAT1>));
  expose((frovedis_lasso_sgd<DT1,S_MAT1>));
  expose((frovedis_lasso_lbfgs<DT1,S_MAT1>));
  expose((frovedis_ridge_sgd<DT1,S_MAT1>));
  expose((frovedis_ridge_lbfgs<DT1,S_MAT1>));
  expose((frovedis_mf_als<DT1,S_MAT1>));
  expose((frovedis_kmeans<DT1,S_MAT1>));
  // --- frovedis LogisticRegressionModel ---
  expose(show_model<LRM1>);
  expose(release_model<LRM1>);
  expose((set_glm_threshold<DT1,LRM1>));
  expose(save_model<LRM1>);
  expose((bcast_model_to_workers<DT1,LRM1>));
  expose(load_glm<LRM1>);
  expose((parallel_glm_predict<DT1,S_LMAT1,LRM1>));
  expose((pgp2<DT1,S_MAT1,S_LMAT1,LRM1>));
  expose((single_glm_predict<DT1,S_LMAT1,LRM1>));
  // --- frovedis LinearRegressionModel ---
  expose(show_model<LNRM1>);
  expose(release_model<LNRM1>);
  expose(save_model<LNRM1>);
  expose((bcast_model_to_workers<DT1,LNRM1>));
  expose(load_lnrm<DT1>);
  expose((parallel_lnrm_predict<DT1,S_LMAT1>));
  expose((p_lnrm_p2<DT1,S_MAT1,S_LMAT1>));
  expose((single_lnrm_predict<DT1,S_LMAT1>));
  // --- frovedis SVMModel ---
  expose(show_model<SVM1>);
  expose(release_model<SVM1>);
  expose(save_model<SVM1>);
  expose((bcast_model_to_workers<DT1,SVM1>));
  expose((set_glm_threshold<DT1,SVM1>));
  expose(load_glm<SVM1>);
  expose((parallel_glm_predict<DT1,S_LMAT1,SVM1>));
  expose((pgp2<DT1,S_MAT1,S_LMAT1,SVM1>));
  expose((single_glm_predict<DT1,S_LMAT1,SVM1>));
  // --- frovedis MatrixFactorizationModel ---
  expose(show_model<MFM1>);
  expose(release_model<MFM1>);
  expose(save_model<MFM1>);
  expose((bcast_model_to_workers<DT1,MFM1>));
  expose(load_mfm<DT1>);
  expose(parallel_mfm_predict<DT1>);
  expose(single_mfm_predict<DT1>);
  expose(frovedis_als_predict<DT1>);
  expose(recommend_users<DT1>);
  expose(recommend_products<DT1>);
  // --- frovedis kmeans model (rowmajor_matrix_local<T>) ---
  expose(show_model<KMM1>);
  expose(release_model<KMM1>);
  expose(save_model<KMM1>);
  expose((bcast_model_to_workers<DT1,KMM1>));
  expose(load_kmm<DT1>);
  expose((parallel_kmm_predict<S_LMAT1,KMM1>));
  expose((pkp2<S_MAT1,S_LMAT1,KMM1>));
  expose((single_kmm_predict<S_LMAT1,KMM1>));
  // --- frovedis sparse svd ---
  expose((frovedis_sparse_svd<DT1,S_MAT1>));
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

  frovedis::init_frovedis_server(argc, argv);
  return 0;
}
