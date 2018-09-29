#ifndef _SHORT_HAND_TYPE_
#define _SHORT_HAND_TYPE_ 

namespace frovedis {
  // --- BASIC Types ---
  typedef double DT1;
  typedef float  DT2;
  typedef long   DT3;
  typedef int    DT4;
  typedef size_t DT5;
  typedef unsigned long ulong;
  // --- Default SPARSE and DENSE Types ---
  typedef crs_matrix<DT1> S_MAT1;
  typedef crs_matrix<DT2> S_MAT2;
  typedef crs_matrix_local<DT1> S_LMAT1;
  typedef crs_matrix_local<DT2> S_LMAT2;
  typedef crs_matrix<DT1,DT4> S_MAT14;
  typedef crs_matrix<DT1,DT5> S_MAT15;
  typedef crs_matrix<DT2,DT4> S_MAT24;
  typedef crs_matrix<DT2,DT5> S_MAT25;
  typedef crs_matrix<DT3,DT4> S_MAT34;
  typedef crs_matrix<DT3,DT5> S_MAT35;
  typedef crs_matrix<DT4,DT4> S_MAT44;
  typedef crs_matrix<DT4,DT5> S_MAT45;
  typedef crs_matrix_local<DT1,DT4> S_LMAT14;
  typedef crs_matrix_local<DT1,DT5> S_LMAT15;
  typedef crs_matrix_local<DT2,DT4> S_LMAT24;
  typedef crs_matrix_local<DT2,DT5> S_LMAT25;
  typedef crs_matrix_local<DT3,DT4> S_LMAT34;
  typedef crs_matrix_local<DT3,DT5> S_LMAT35;
  typedef crs_matrix_local<DT4,DT4> S_LMAT44;
  typedef crs_matrix_local<DT4,DT5> S_LMAT45;
  // --- All DENSE Types ---
  typedef colmajor_matrix<DT1> D_MAT1;
  typedef colmajor_matrix<DT2> D_MAT2;
  typedef colmajor_matrix_local<DT1> D_LMAT1;
  typedef colmajor_matrix_local<DT2> D_LMAT2;
  typedef rowmajor_matrix<DT1> R_MAT1;
  typedef rowmajor_matrix<DT2> R_MAT2;
  typedef rowmajor_matrix<DT3> R_MAT3;
  typedef rowmajor_matrix<DT4> R_MAT4;
  typedef rowmajor_matrix_local<DT1> R_LMAT1;
  typedef rowmajor_matrix_local<DT2> R_LMAT2;
  typedef rowmajor_matrix_local<DT3> R_LMAT3;
  typedef rowmajor_matrix_local<DT4> R_LMAT4;
  typedef colmajor_matrix<DT1> C_MAT1;
  typedef colmajor_matrix<DT2> C_MAT2;
  typedef colmajor_matrix<DT3> C_MAT3;
  typedef colmajor_matrix<DT4> C_MAT4;
  typedef colmajor_matrix_local<DT1> C_LMAT1;
  typedef colmajor_matrix_local<DT2> C_LMAT2;
  typedef colmajor_matrix_local<DT3> C_LMAT3;
  typedef colmajor_matrix_local<DT4> C_LMAT4;
  typedef blockcyclic_matrix<DT1> B_MAT1;
  typedef blockcyclic_matrix<DT2> B_MAT2;
  typedef blockcyclic_matrix<DT3> B_MAT3;
  typedef blockcyclic_matrix<DT4> B_MAT4;
  typedef blockcyclic_matrix_local<DT1> B_LMAT1;
  typedef blockcyclic_matrix_local<DT2> B_LMAT2;
  typedef blockcyclic_matrix_local<DT3> B_LMAT3;
  typedef blockcyclic_matrix_local<DT4> B_LMAT4;
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
  typedef decision_tree_model<DT1> DTM1;
  typedef decision_tree_model<DT2> DTM2;
  typedef fm::fm_model<DT1> FMM1;
  typedef fm::fm_model<DT2> FMM2;
  typedef naive_bayes_model<DT1> NBM1;
  typedef naive_bayes_model<DT2> NBM2;
}
#endif
