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

#include "exrpc_request_headers.hpp"

using namespace frovedis;

int main(int argc, char* argv[]) {
  frovedis::use_frovedis use(argc, argv);
  expose(get_nodesize);
  expose(cleanup_frovedis_server);
  // --- frovedis dvector for labels ---
  expose((load_local_data<std::vector<DT1>>));
  expose(create_and_set_dvector<DT1>);
  expose(show_dvector<DT1>);
  expose(release_dvector<DT1>);
  // --- frovedis typed dvector for dataframes ---
  expose((load_local_data<std::vector<int>>));
  expose((load_local_data<std::vector<long>>));
  expose((load_local_data<std::vector<float>>));
  expose((load_local_data<std::vector<double>>));
  expose((load_local_data<std::vector<std::string>>));
  expose(create_and_set_dvector<int>);
  expose(create_and_set_dvector<long>);
  expose(create_and_set_dvector<float>);
  expose(create_and_set_dvector<double>);
  expose(create_and_set_dvector<std::string>);
  expose(show_dvector<int>);
  expose(show_dvector<long>);
  expose(show_dvector<float>);
  expose(show_dvector<double>);
  expose(show_dvector<std::string>);
  expose(release_dvector<int>);
  expose(release_dvector<long>);
  expose(release_dvector<float>);
  expose(release_dvector<double>);
  expose(release_dvector<std::string>);
  // --- frovedis glm data ---
  expose((load_local_glm_data<DT1,S_LMAT1>));
  expose((create_and_set_glm_data<DT1,S_MAT1,S_LMAT1>));
  expose((release_glm_data<DT1,S_MAT1>));
  expose((release_glm_data<DT1,D_MAT1>));
  expose((show_glm_data<DT1,S_MAT1>));
  expose((show_glm_data<DT1,D_MAT1>));
  // --- frovedis generic data ---
  expose(load_local_data<std::vector<std::string>>);
  expose((release_data<std::vector<double>>)); //SVAL
  expose((release_data<std::vector<int>>));    //LAPACK(IPIV)
  expose((release_data<lvec<int>>));           //ScaLAPACK(IPIV)
  // --- frovedis crs data ---
  expose(load_local_data<S_LMAT1>); //spark
  expose((create_and_set_data<S_MAT1,S_LMAT1>)); //spark
  expose(create_crs_from_local_coo_string_vectors<DT1>); //spark
  expose(create_crs_data<DT1>); //spark
  expose(load_crs_matrix<DT1>); //spark
  expose(release_data<S_MAT1>); //spark
  expose(show_data<S_MAT1>); //spark
  expose(save_matrix<S_MAT1>); //spark
  expose(load_local_data<S_LMAT14>); 
  expose(load_local_data<S_LMAT15>); 
  expose(load_local_data<S_LMAT24>); 
  expose(load_local_data<S_LMAT25>); 
  expose(load_local_data<S_LMAT34>); 
  expose(load_local_data<S_LMAT35>); 
  expose(load_local_data<S_LMAT44>); 
  expose(load_local_data<S_LMAT45>); 
  expose((create_crs_data<DT1,DT4,DT5>));
  expose((create_crs_data<DT1,DT5,DT5>));
  expose((create_crs_data<DT2,DT4,DT5>));
  expose((create_crs_data<DT2,DT5,DT5>));
  expose((create_crs_data<DT3,DT4,DT5>));
  expose((create_crs_data<DT3,DT5,DT5>));
  expose((create_crs_data<DT4,DT4,DT5>));
  expose((create_crs_data<DT4,DT5,DT5>));
  expose((load_crs_matrix<DT1,DT4,DT5>));
  expose((load_crs_matrix<DT1,DT5,DT5>));
  expose((load_crs_matrix<DT2,DT4,DT5>));
  expose((load_crs_matrix<DT2,DT5,DT5>));
  //expose((load_crs_matrix<DT3,DT4,DT5>)); // not supported
  //expose((load_crs_matrix<DT3,DT5,DT5>)); // not supported
  expose((load_crs_matrix<DT4,DT4,DT5>));
  expose((load_crs_matrix<DT4,DT5,DT5>));
  expose(release_data<S_MAT14>);
  expose(release_data<S_MAT15>);
  expose(release_data<S_MAT24>);
  expose(release_data<S_MAT25>);
  expose(release_data<S_MAT34>);
  expose(release_data<S_MAT35>);
  expose(release_data<S_MAT44>);
  expose(release_data<S_MAT45>);
  expose(show_data<S_MAT14>);
  expose(show_data<S_MAT15>);
  expose(show_data<S_MAT24>);
  expose(show_data<S_MAT25>);
  expose(show_data<S_MAT34>);
  expose(show_data<S_MAT35>);
  expose(show_data<S_MAT44>);
  expose(show_data<S_MAT45>);
  expose(save_matrix<S_MAT14>);
  expose(save_matrix<S_MAT15>);
  expose(save_matrix<S_MAT24>);
  expose(save_matrix<S_MAT25>);
  expose(save_matrix<S_MAT34>);
  expose(save_matrix<S_MAT35>);
  expose(save_matrix<S_MAT44>);
  expose(save_matrix<S_MAT45>);
  // --- frovedis dense data ---
  expose(load_local_data<R_LMAT1>);
  expose(load_local_data<R_LMAT2>);
  expose(load_local_data<R_LMAT3>);
  expose(load_local_data<R_LMAT4>);
  expose(create_rmm_data<DT1>);
  expose(create_rmm_data<DT2>);
  expose(create_rmm_data<DT3>);
  expose(create_rmm_data<DT4>);
  expose(create_cmm_data<DT1>);
  expose(create_cmm_data<DT2>);
  expose(create_cmm_data<DT3>);
  expose(create_cmm_data<DT4>);
  expose(create_bcm_data<DT1>);
  expose(create_bcm_data<DT2>);
  expose(load_rmm_matrix<DT1>);
  expose(load_rmm_matrix<DT2>);
  //expose(load_rmm_matrix<DT3>); // not supported
  expose(load_rmm_matrix<DT4>);
  expose(load_cmm_matrix<DT1>);
  expose(load_cmm_matrix<DT2>);
  //expose(load_cmm_matrix<DT3>); // not supported
  expose(load_cmm_matrix<DT4>);
  expose(load_bcm_matrix<DT1>);
  expose(load_bcm_matrix<DT2>);
  expose(release_data<R_MAT2>);
  expose(release_data<R_MAT3>);
  expose(release_data<R_MAT4>);
  expose(release_data<R_MAT1>);
  expose(release_data<C_MAT1>);
  expose(release_data<C_MAT2>);
  expose(release_data<C_MAT3>);
  expose(release_data<C_MAT4>);
  expose(release_data<B_MAT1>);
  expose(release_data<B_MAT2>);
  expose(show_data<R_MAT1>);
  expose(show_data<R_MAT2>);
  expose(show_data<R_MAT3>);
  expose(show_data<R_MAT4>);
  expose(show_data<C_MAT1>);
  expose(show_data<C_MAT2>);
  expose(show_data<C_MAT3>);
  expose(show_data<C_MAT4>);
  expose(show_data<B_MAT1>);
  expose(show_data<B_MAT2>);
  expose(save_matrix<R_MAT1>);
  expose(save_matrix<R_MAT2>);
  expose(save_matrix<R_MAT3>);
  expose(save_matrix<R_MAT4>);
  expose(save_matrix<C_MAT1>);
  expose(save_matrix<C_MAT2>);
  expose(save_matrix<C_MAT3>); 
  expose(save_matrix<C_MAT4>);
  expose(save_matrix<B_MAT1>);
  expose(save_matrix<B_MAT2>);
  expose(transpose_matrix<R_MAT1>);
  expose(transpose_matrix<R_MAT2>);
  expose(transpose_matrix<R_MAT3>);
  expose(transpose_matrix<R_MAT4>);
  //expose(transpose_matrix<C_MAT1>); // not yet supported
  //expose(transpose_matrix<C_MAT2>); // not yet supported
  //expose(transpose_matrix<C_MAT3>); // not yet supported
  //expose(transpose_matrix<C_MAT4>); // not yet supported
  expose(transpose_matrix<B_MAT1>);
  expose(transpose_matrix<B_MAT2>);
  //expose(copy_matrix<R_MAT1>); // not yet supported
  //expose(copy_matrix<C_MAT1>); // not yet supported
  expose(copy_matrix<B_MAT1>);
  expose(copy_matrix<B_MAT2>);
  expose((to_rowmajor_matrix<DT1,C_MAT1>));
  expose((to_rowmajor_matrix<DT2,C_MAT2>));
  expose((to_rowmajor_matrix<DT3,C_MAT3>));
  expose((to_rowmajor_matrix<DT4,C_MAT4>));
  expose((to_rowmajor_matrix<DT1,B_MAT1>));
  expose((to_rowmajor_matrix<DT2,B_MAT2>));
  expose((to_rowmajor_array<DT1,C_MAT1>));
  expose((to_rowmajor_array<DT1,B_MAT1>));
  expose(rowmajor_to_colmajor_array<DT1>);
  expose(colmajor_to_colmajor_array<DT1>);
  expose(blockcyclic_to_colmajor_array<DT1>);
  expose((matrix_to_array<DT1,R_MAT1,R_LMAT1>));
  expose((matrix_to_array<DT1,C_MAT1,C_LMAT1>));
  expose((matrix_to_array<DT1,B_MAT1,B_LMAT1>));
  expose((convert_and_get_all_rml_pointers<DT1,C_MAT1>));
  expose((convert_and_get_all_rml_pointers<DT2,C_MAT2>));
  expose((convert_and_get_all_rml_pointers<DT3,C_MAT3>));
  expose((convert_and_get_all_rml_pointers<DT4,C_MAT4>));
  expose((convert_and_get_all_rml_pointers<DT1,B_MAT1>));
  expose((convert_and_get_all_rml_pointers<DT2,B_MAT2>));
  expose((get_all_local_pointers<R_MAT1,R_LMAT1>));
  expose((get_all_local_pointers<R_MAT2,R_LMAT2>));
  expose((get_all_local_pointers<R_MAT3,R_LMAT3>));
  expose((get_all_local_pointers<R_MAT4,R_LMAT4>));
  expose((get_all_local_pointers<C_MAT1,C_LMAT1>));
  expose((get_all_local_pointers<B_MAT1,B_LMAT1>));
  expose((get_local_array<DT1,R_LMAT1>));
  expose((get_local_array<DT1,C_LMAT1>));
  expose((get_local_array<DT1,B_LMAT1>));
  expose(vec_to_array<DT1>);
  expose(save_as_diag_matrix_local<DT1>);
  // --- frovedis ML trainers ---
  // (1) logistic regression
  expose((frovedis_lr_sgd<DT1,D_MAT1>));    // both spark and python case
  expose((frovedis_lr_sgd<DT2,D_MAT2>));    // python case
  expose((frovedis_lr_sgd<DT1,S_MAT1>));    // spark case
  expose((frovedis_lr_sgd<DT1,S_MAT14>));   // python case
  expose((frovedis_lr_sgd<DT1,S_MAT15>));   // python case
  expose((frovedis_lr_sgd<DT2,S_MAT24>));   // python case
  expose((frovedis_lr_sgd<DT2,S_MAT25>));   // python case
  expose((frovedis_lr_lbfgs<DT1,D_MAT1>));  // both spark and python case
  expose((frovedis_lr_lbfgs<DT2,D_MAT2>));  // python case
  expose((frovedis_lr_lbfgs<DT1,S_MAT1>));  // spark case
  expose((frovedis_lr_lbfgs<DT1,S_MAT14>)); // python case
  expose((frovedis_lr_lbfgs<DT1,S_MAT15>)); // python case
  expose((frovedis_lr_lbfgs<DT2,S_MAT24>)); // python case
  expose((frovedis_lr_lbfgs<DT2,S_MAT25>)); // python case
  // (2) linear svm
  expose((frovedis_svm_sgd<DT1,D_MAT1>));    // both spark and python case
  expose((frovedis_svm_sgd<DT2,D_MAT2>));    // python case
  expose((frovedis_svm_sgd<DT1,S_MAT1>));    // spark case
  expose((frovedis_svm_sgd<DT1,S_MAT14>));   // python case
  expose((frovedis_svm_sgd<DT1,S_MAT15>));   // python case
  expose((frovedis_svm_sgd<DT2,S_MAT24>));   // python case
  expose((frovedis_svm_sgd<DT2,S_MAT25>));   // python case
  expose((frovedis_svm_lbfgs<DT1,D_MAT1>));  // both spark and python case
  expose((frovedis_svm_lbfgs<DT2,D_MAT2>));  // python case
  expose((frovedis_svm_lbfgs<DT1,S_MAT1>));  // spark case
  expose((frovedis_svm_lbfgs<DT1,S_MAT14>)); // python case
  expose((frovedis_svm_lbfgs<DT1,S_MAT15>)); // python case
  expose((frovedis_svm_lbfgs<DT2,S_MAT24>)); // python case
  expose((frovedis_svm_lbfgs<DT2,S_MAT25>)); // python case
  // (3) decision tree
  //expose((frovedis_dt<DT1,S_MAT1>)); // not supported currently
  expose((frovedis_dt<DT1,D_MAT1>));   // both spark and python case
  expose((frovedis_dt<DT2,D_MAT2>));   // python case
  // (4) naive bayes
  expose((frovedis_nb<DT1,D_MAT1,D_LMAT1>));   // both spark and python case
  expose((frovedis_nb<DT2,D_MAT2,D_LMAT2>));   // python case
  expose((frovedis_nb<DT1,S_MAT1,S_LMAT1>));   // spark case
  expose((frovedis_nb<DT1,S_MAT14,S_LMAT14>)); // python case
  expose((frovedis_nb<DT1,S_MAT15,S_LMAT15>)); // python case
  expose((frovedis_nb<DT2,S_MAT24,S_LMAT24>)); // python case
  expose((frovedis_nb<DT2,S_MAT25,S_LMAT25>)); // python case
  // (5) linear regression
  expose((frovedis_lnr_lbfgs<DT1,D_MAT1>));    // both spark and python case
  expose((frovedis_lnr_lbfgs<DT2,D_MAT2>));    // python case
  expose((frovedis_lnr_lbfgs<DT1,S_MAT1>));    // spark case
  expose((frovedis_lnr_lbfgs<DT1,S_MAT14>));   // python case
  expose((frovedis_lnr_lbfgs<DT1,S_MAT15>));   // python case
  expose((frovedis_lnr_lbfgs<DT2,S_MAT24>));   // python case
  expose((frovedis_lnr_lbfgs<DT2,S_MAT25>));   // python case
  expose((frovedis_lnr_sgd<DT1,D_MAT1>));      // both spark and python case
  expose((frovedis_lnr_sgd<DT2,D_MAT2>));      // python case
  expose((frovedis_lnr_sgd<DT1,S_MAT1>));      // spark case
  expose((frovedis_lnr_sgd<DT1,S_MAT14>));     // python case
  expose((frovedis_lnr_sgd<DT1,S_MAT15>));     // python case
  expose((frovedis_lnr_sgd<DT2,S_MAT24>));     // python case
  expose((frovedis_lnr_sgd<DT2,S_MAT25>));     // python case
  // (6) lasso regression
  expose((frovedis_lasso_sgd<DT1,D_MAT1>));    // both spark and python case
  expose((frovedis_lasso_sgd<DT2,D_MAT2>));    // python case
  expose((frovedis_lasso_sgd<DT1,S_MAT1>));    // spark case
  expose((frovedis_lasso_sgd<DT1,S_MAT14>));   // python case
  expose((frovedis_lasso_sgd<DT1,S_MAT15>));   // python case
  expose((frovedis_lasso_sgd<DT2,S_MAT24>));   // python case
  expose((frovedis_lasso_sgd<DT2,S_MAT25>));   // python case
  expose((frovedis_lasso_lbfgs<DT1,D_MAT1>));  // both spark and python case
  expose((frovedis_lasso_lbfgs<DT2,D_MAT2>));  // python case
  expose((frovedis_lasso_lbfgs<DT1,S_MAT1>));  // spark case
  expose((frovedis_lasso_lbfgs<DT1,S_MAT14>)); // python case
  expose((frovedis_lasso_lbfgs<DT1,S_MAT15>)); // python case
  expose((frovedis_lasso_lbfgs<DT2,S_MAT24>)); // python case
  expose((frovedis_lasso_lbfgs<DT2,S_MAT25>)); // python case
  // (7) ridge regression
  expose((frovedis_ridge_sgd<DT1,D_MAT1>));    // both spark and python case
  expose((frovedis_ridge_sgd<DT2,D_MAT2>));    // python case
  expose((frovedis_ridge_sgd<DT1,S_MAT1>));    // spark case
  expose((frovedis_ridge_sgd<DT1,S_MAT14>));   // python case
  expose((frovedis_ridge_sgd<DT1,S_MAT15>));   // python case
  expose((frovedis_ridge_sgd<DT2,S_MAT24>));   // python case
  expose((frovedis_ridge_sgd<DT2,S_MAT25>));   // python case
  expose((frovedis_ridge_lbfgs<DT1,D_MAT1>));  // both spark and python case
  expose((frovedis_ridge_lbfgs<DT2,D_MAT2>));  // python case
  expose((frovedis_ridge_lbfgs<DT1,S_MAT1>));  // spark case
  expose((frovedis_ridge_lbfgs<DT1,S_MAT14>)); // python case
  expose((frovedis_ridge_lbfgs<DT1,S_MAT15>)); // python case
  expose((frovedis_ridge_lbfgs<DT2,S_MAT24>)); // python case
  expose((frovedis_ridge_lbfgs<DT2,S_MAT25>)); // python case
  // (8) kmeans clustering
  // as for dense input, kmeans supports only rowmajor_matrix<T> for dense data
  expose((frovedis_kmeans<DT1,R_MAT1>));  // both spark and python case
  expose((frovedis_kmeans<DT2,R_MAT2>));  // python case
  expose((frovedis_kmeans<DT1,S_MAT1>));  // spark case
  expose((frovedis_kmeans<DT1,S_MAT14>)); // python case
  expose((frovedis_kmeans<DT1,S_MAT15>)); // python case
  expose((frovedis_kmeans<DT2,S_MAT24>)); // python case
  expose((frovedis_kmeans<DT2,S_MAT25>)); // python case
  // (9) matrix factorization using als
  expose((frovedis_mf_als<DT1,S_MAT1>));  // spark case
  expose((frovedis_mf_als<DT1,S_MAT14>)); // python case
  expose((frovedis_mf_als<DT1,S_MAT15>)); // python case
  expose((frovedis_mf_als<DT2,S_MAT24>)); // python case
  expose((frovedis_mf_als<DT2,S_MAT25>)); // python case
  // (10) factorization machine
  expose((frovedis_fm<DT1,S_MAT1>));      // spark case
  expose((frovedis_fm<DT1,S_MAT14>));     // python case
  expose((frovedis_fm<DT1,S_MAT15>));     // python case
  expose((frovedis_fm<DT2,S_MAT24>));     // python case
  expose((frovedis_fm<DT2,S_MAT25>));     // python case
  // --- frovedis DecisionTreeModel ---
  expose(show_model<DTM1>);
  expose(release_model<DTM1>);
  expose(save_model<DTM1>);
  expose(load_model<DTM1>);    // void returning
  expose(show_model<DTM2>);    // for python
  expose(release_model<DTM2>); // for python
  expose(save_model<DTM2>);    // for python
  expose(load_model<DTM2>);    // for python (void returning)
  expose((bcast_model_to_workers<DT1,DTM1>));
  expose((single_dtm_predict<DT1,S_LMAT1>));
  expose((parallel_dtm_predict<DT1,S_LMAT1>));
  expose((parallel_dtm_predict_with_broadcast<DT1,R_MAT1,R_LMAT1>));   // for python
  expose((parallel_dtm_predict_with_broadcast<DT2,R_MAT2,R_LMAT2>));   // for python
  expose((parallel_dtm_predict_with_broadcast<DT1,S_MAT14,S_LMAT14>)); // for python
  expose((parallel_dtm_predict_with_broadcast<DT1,S_MAT15,S_LMAT15>)); // for python
  expose((parallel_dtm_predict_with_broadcast<DT2,S_MAT24,S_LMAT24>)); // for python
  expose((parallel_dtm_predict_with_broadcast<DT2,S_MAT25,S_LMAT25>)); // for python
  // --- frovedis NBModel ---
  expose(show_model<NBM1>);
  expose(release_model<NBM1>);
  expose(save_model<NBM1>);
  expose(load_nbm<DT1>);       // returns string
  expose(show_model<NBM2>);    // for python
  expose(release_model<NBM2>); // for python
  expose(save_model<NBM2>);    // for python
  expose(load_nbm<DT2>);       // for python (returns string)
  expose((bcast_model_to_workers<DT1,NBM1>));
  expose((single_nbm_predict<DT1,S_LMAT1>));
  expose((parallel_nbm_predict<DT1,S_LMAT1>));
  expose((parallel_nbm_predict_with_broadcast<DT1,R_MAT1,R_LMAT1>));   // for python
  expose((parallel_nbm_predict_with_broadcast<DT2,R_MAT2,R_LMAT2>));   // for python
  expose((parallel_nbm_predict_with_broadcast<DT1,S_MAT14,S_LMAT14>)); // for python
  expose((parallel_nbm_predict_with_broadcast<DT1,S_MAT15,S_LMAT15>)); // for python
  expose((parallel_nbm_predict_with_broadcast<DT2,S_MAT24,S_LMAT24>)); // for python
  expose((parallel_nbm_predict_with_broadcast<DT2,S_MAT25,S_LMAT25>)); // for python
  // --- frovedis FMModel ---
  //expose(show_model<FMM1>);   // not supported
  expose(release_model<FMM1>);
  expose(save_fmm<DT1>);
  //expose(load_model<FMM1>);   // not supported
  //expose(show_model<FMM2>);   // for python (not supported)
  expose(release_model<FMM2>);  // for python
  expose(save_fmm<DT2>);        // for python
  //expose(load_model<FMM2>);   // for python (not supported)
  expose(bcast_fmm_to_workers<DT1>);
  expose((single_fmm_predict<DT1,S_LMAT1>));
  expose((parallel_fmm_predict<DT1,S_LMAT1>));
  expose((parallel_fmm_predict_with_broadcast<DT1,S_MAT1,S_LMAT1>)); // for python
  expose((parallel_fmm_predict_with_broadcast<DT2,S_MAT2,S_LMAT2>)); // for python
  expose((parallel_fmm_predict_with_broadcast<DT1,S_MAT14,S_LMAT14>)); // for python
  expose((parallel_fmm_predict_with_broadcast<DT1,S_MAT15,S_LMAT15>)); // for python
  expose((parallel_fmm_predict_with_broadcast<DT2,S_MAT24,S_LMAT24>)); // for python
  expose((parallel_fmm_predict_with_broadcast<DT2,S_MAT25,S_LMAT25>)); // for python
  // --- frovedis LogisticRegressionModel ---
  expose(show_model<LRM1>);
  expose(release_model<LRM1>);
  expose(save_model<LRM1>);
  expose(load_glm<LRM1>);
  expose(show_model<LRM2>);    // for python
  expose(release_model<LRM2>); // for python
  expose(save_model<LRM2>);    // for python
  expose(load_glm<LRM2>);      // for python
  expose((set_glm_threshold<DT1,LRM1>));
  expose((bcast_model_to_workers<DT1,LRM1>));
  expose((single_glm_predict<DT1,S_LMAT1,LRM1>));
  expose((parallel_glm_predict<DT1,S_LMAT1,LRM1>));
  expose((pgp2<DT1,R_MAT1,R_LMAT1,LRM1>));  // for python
  expose((pgp2<DT2,R_MAT2,R_LMAT2,LRM2>));  // for python
  expose((pgp2<DT1,S_MAT14,S_LMAT14,LRM1>));  // for python
  expose((pgp2<DT1,S_MAT15,S_LMAT15,LRM1>));  // for python
  expose((pgp2<DT2,S_MAT24,S_LMAT24,LRM2>));  // for python
  expose((pgp2<DT2,S_MAT25,S_LMAT25,LRM2>));  // for python
  // --- frovedis LinearRegressionModel ---
  expose(show_model<LNRM1>);
  expose(release_model<LNRM1>);
  expose(save_model<LNRM1>);
  expose(load_lnrm<DT1>);
  expose(show_model<LNRM2>);    // for python
  expose(release_model<LNRM2>); // for python
  expose(save_model<LNRM2>);    // for python
  expose(load_lnrm<DT2>);       // for python
  expose((bcast_model_to_workers<DT1,LNRM1>));
  expose((single_lnrm_predict<DT1,S_LMAT1>));
  expose((parallel_lnrm_predict<DT1,S_LMAT1>));
  expose((p_lnrm_p2<DT1,R_MAT1,R_LMAT1>));    // for python
  expose((p_lnrm_p2<DT2,R_MAT2,R_LMAT2>));    // for python
  expose((p_lnrm_p2<DT1,S_MAT14,S_LMAT14>));  // for python
  expose((p_lnrm_p2<DT1,S_MAT15,S_LMAT15>));  // for python
  expose((p_lnrm_p2<DT2,S_MAT24,S_LMAT24>));  // for python
  expose((p_lnrm_p2<DT2,S_MAT25,S_LMAT25>));  // for python
  // --- frovedis SVMModel ---
  expose(show_model<SVM1>);
  expose(release_model<SVM1>);
  expose(save_model<SVM1>);
  expose(load_glm<SVM1>);
  expose(show_model<SVM2>);    // for python
  expose(release_model<SVM2>); // for python
  expose(save_model<SVM2>);    // for python
  expose(load_glm<SVM2>);      // for python
  expose((set_glm_threshold<DT1,SVM1>));
  expose((bcast_model_to_workers<DT1,SVM1>));
  expose((single_glm_predict<DT1,S_LMAT1,SVM1>));
  expose((parallel_glm_predict<DT1,S_LMAT1,SVM1>));
  expose((pgp2<DT1,R_MAT1,R_LMAT1,SVM1>));   // for python
  expose((pgp2<DT2,R_MAT2,R_LMAT2,SVM2>));   // for python
  expose((pgp2<DT1,S_MAT14,S_LMAT14,SVM1>)); // for python
  expose((pgp2<DT1,S_MAT15,S_LMAT15,SVM1>)); // for python
  expose((pgp2<DT2,S_MAT24,S_LMAT24,SVM2>)); // for python
  expose((pgp2<DT2,S_MAT25,S_LMAT25,SVM2>)); // for python
  // --- frovedis MatrixFactorizationModel ---
  expose(show_model<MFM1>);
  expose(release_model<MFM1>);
  expose(save_model<MFM1>);
  expose(load_mfm<DT1>);
  expose(show_model<MFM2>);    // for python
  expose(release_model<MFM2>); // for python
  expose(save_model<MFM2>);    // for python
  expose(load_mfm<DT2>);       // for python
  expose((bcast_model_to_workers<DT1,MFM1>));
  expose(single_mfm_predict<DT1>);
  expose(parallel_mfm_predict<DT1>);
  expose(frovedis_als_predict<DT1>);
  expose(recommend_users<DT1>);
  expose(recommend_products<DT1>);
  expose(frovedis_als_predict<DT2>); // for python
  expose(recommend_users<DT2>);      // for python
  expose(recommend_products<DT2>);   // for python
  // --- frovedis kmeans model (rowmajor_matrix_local<T>) ---
  expose(show_model<KMM1>);
  expose(release_model<KMM1>);
  expose(save_model<KMM1>);
  expose(load_kmm<DT1>);
  expose(show_model<KMM2>);    // for python
  expose(release_model<KMM2>); // for python
  expose(save_model<KMM2>);    // for python
  expose(load_kmm<DT2>);       // for python
  expose((bcast_model_to_workers<DT1,KMM1>));
  expose((single_kmm_predict<S_LMAT1,KMM1>));
  expose((parallel_kmm_predict<S_LMAT1,KMM1>));
  expose((pkp2<R_MAT1,R_LMAT1,KMM1>));   // for python
  expose((pkp2<R_MAT2,R_LMAT2,KMM2>));   // for python
  expose((pkp2<S_MAT14,S_LMAT14,KMM1>)); // for python
  expose((pkp2<S_MAT15,S_LMAT15,KMM1>)); // for python
  expose((pkp2<S_MAT24,S_LMAT24,KMM2>)); // for python
  expose((pkp2<S_MAT25,S_LMAT25,KMM2>)); // for python
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
  // --- frovedis dataframe ---
  expose(create_dataframe);
  expose(release_data<dftable>);
  expose(release_data<grouped_dftable>);
  expose(show_dataframe);
  expose(get_dfoperator<int>);
  expose(get_dfoperator<long>);
  expose(get_dfoperator<float>);
  expose(get_dfoperator<double>);
  expose(get_str_dfoperator);
  expose(get_dfANDoperator);
  expose(get_dfORoperator);
  expose((release_data<std::shared_ptr<dfoperator>>));
  expose(filter_df);
  expose(select_df);
  expose(sort_df);
  expose(join_df);
  expose(group_by_df);
  expose(frovedis_df_size);
  expose(frovedis_df_cnt);
  expose(frovedis_df_sum);
  expose(frovedis_df_avg);
  expose(frovedis_df_min);
  expose(frovedis_df_max);
  expose(frovedis_df_std);
  expose(frovedis_df_rename);
  expose(get_df_int_col);
  expose(get_df_long_col);
  expose(get_df_float_col);
  expose(get_df_double_col);
  expose(get_df_string_col);

  frovedis::init_frovedis_server(argc, argv);
  return 0;
}
