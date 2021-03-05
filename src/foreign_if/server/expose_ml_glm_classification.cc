#include "exrpc_ml.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"

using namespace frovedis;

RegType get_regularizer(int regType) {
  RegType rtype = ZERO;
  if (regType == 0) rtype = ZERO;
  else if (regType == 1) rtype = L1;
  else if (regType == 2) rtype = L2;
  else REPORT_ERROR(USER_ERROR, 
       "Currently frovedis supports ZERO/L1/L2 regularizer!");
  return rtype;
}

void expose_frovedis_glm_classification_functions() {
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
  expose((frovedis_lr_shrink_sgd<DT1,S_MAT1>));    // spark case
  expose((frovedis_lr_shrink_sgd<DT1,S_MAT14>));   // python case
  expose((frovedis_lr_shrink_sgd<DT1,S_MAT15>));   // python case
  expose((frovedis_lr_shrink_sgd<DT2,S_MAT24>));   // python case
  expose((frovedis_lr_shrink_sgd<DT2,S_MAT25>));   // python case
  // (2) linear svm classification
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
}

