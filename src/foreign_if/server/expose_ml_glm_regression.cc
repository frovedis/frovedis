#include "exrpc_ml.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"

using namespace frovedis;

void expose_frovedis_glm_regression_functions() {
  // (1) linear svm regression
  expose((frovedis_svm_regressor<DT1,D_MAT1>));    // both spark and python case
  expose((frovedis_svm_regressor<DT2,D_MAT2>));    // python case
  expose((frovedis_svm_regressor<DT1,S_MAT14>));   // python case
  expose((frovedis_svm_regressor<DT1,S_MAT15>));   // python case
  expose((frovedis_svm_regressor<DT2,S_MAT24>));   // python case
  expose((frovedis_svm_regressor<DT2,S_MAT25>));   // python case
  // (2) linear regression
  expose((frovedis_lnr<DT1,D_MAT1>));      // both spark and python case
  expose((frovedis_lnr<DT2,D_MAT2>));      // python case
  expose((frovedis_lnr<DT1,S_MAT1>));      // spark case
  expose((frovedis_lnr<DT1,S_MAT14>));     // python case
  expose((frovedis_lnr<DT1,S_MAT15>));     // python case
  expose((frovedis_lnr<DT2,S_MAT24>));     // python case
  expose((frovedis_lnr<DT2,S_MAT25>));     // python case
  // (3) lasso regression
  expose((frovedis_lasso<DT1,D_MAT1>));    // both spark and python case
  expose((frovedis_lasso<DT2,D_MAT2>));    // python case
  expose((frovedis_lasso<DT1,S_MAT1>));    // spark case
  expose((frovedis_lasso<DT1,S_MAT14>));   // python case
  expose((frovedis_lasso<DT1,S_MAT15>));   // python case
  expose((frovedis_lasso<DT2,S_MAT24>));   // python case
  expose((frovedis_lasso<DT2,S_MAT25>));   // python case
  // (4) ridge regression
  expose((frovedis_ridge<DT1,D_MAT1>));    // both spark and python case
  expose((frovedis_ridge<DT2,D_MAT2>));    // python case
  expose((frovedis_ridge<DT1,S_MAT1>));    // spark case
  expose((frovedis_ridge<DT1,S_MAT14>));   // python case
  expose((frovedis_ridge<DT1,S_MAT15>));   // python case
  expose((frovedis_ridge<DT2,S_MAT24>));   // python case
  expose((frovedis_ridge<DT2,S_MAT25>));   // python case
}

