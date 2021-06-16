#ifndef _SHORT_HAND_DENSE_TYPE_
#define _SHORT_HAND_DENSE_TYPE_ 

#include "short_hand_dtype.hpp"

using namespace frovedis;

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

#endif
