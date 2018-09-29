#ifndef _SHORT_HAND_MODEL_TYPE_
#define _SHORT_HAND_MODEL_TYPE_ 

#include "short_hand_dtype.hpp"

namespace frovedis {
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
