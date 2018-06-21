#ifndef _SHORT_HAND_TYPE_
#define _SHORT_HAND_TYPE_ 
namespace frovedis {
  // --- BASIC Types ---
  typedef double DT1;
  typedef float DT2;
  // --- Default SPARSE and DENSE Types ---
  typedef crs_matrix<DT1> S_MAT1;
  typedef crs_matrix<DT2> S_MAT2;
  typedef crs_matrix_local<DT1> S_LMAT1;
  typedef crs_matrix_local<DT2> S_LMAT2;
  typedef colmajor_matrix<DT1> D_MAT1;
  typedef colmajor_matrix<DT2> D_MAT2;
  typedef colmajor_matrix_local<DT1> D_LMAT1;
  typedef colmajor_matrix_local<DT2> D_LMAT2;
  // --- All DENSE Types ---
  typedef rowmajor_matrix<DT1> R_MAT1;
  typedef rowmajor_matrix<DT2> R_MAT2;
  typedef rowmajor_matrix_local<DT1> R_LMAT1;
  typedef rowmajor_matrix_local<DT2> R_LMAT2;
  typedef colmajor_matrix<DT1> C_MAT1;
  typedef colmajor_matrix<DT2> C_MAT2;
  typedef colmajor_matrix_local<DT1> C_LMAT1;
  typedef colmajor_matrix_local<DT2> C_LMAT2;
  typedef blockcyclic_matrix<DT1> B_MAT1;
  typedef blockcyclic_matrix<DT2> B_MAT2;
  typedef blockcyclic_matrix_local<DT1> B_LMAT1;
  typedef blockcyclic_matrix_local<DT2> B_LMAT2;
  // --- All Frovedis ML MODEL Types ---
  typedef logistic_regression_model<DT1> LRM1;
  typedef logistic_regression_model<DT2> LRM2;
  typedef linear_regression_model<DT1> LNRM1;
  typedef linear_regression_model<DT2> LNRM2;
  typedef svm_model<DT1> SVM1;
  typedef svm_model<DT2> SVM2; 
  typedef matrix_factorization_model<DT1> MFM1;
  typedef matrix_factorization_model<DT2> MFM2;
  typedef rowmajor_matrix_local<DT1> KMM1;
  typedef rowmajor_matrix_local<DT2> KMM2;
}
#endif
