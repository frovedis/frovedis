#include "exrpc_ml.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"

using namespace frovedis;

void expose_frovedis_glm_regression_functions() {
  // (1) linear svm regression
  expose((frovedis_svm_regressor_sgd<DT1,D_MAT1>));    // both spark and python case
  expose((frovedis_svm_regressor_sgd<DT2,D_MAT2>));    // python case
  expose((frovedis_svm_regressor_sgd<DT1,S_MAT14>));   // python case
  expose((frovedis_svm_regressor_sgd<DT1,S_MAT15>));   // python case
  expose((frovedis_svm_regressor_sgd<DT2,S_MAT24>));   // python case
  expose((frovedis_svm_regressor_sgd<DT2,S_MAT25>));   // python case
  // (2) linear regression
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
  expose((frovedis_lnr_lapack<DT1,D_MAT1>));   // both spark and python case
  expose((frovedis_lnr_lapack<DT2,D_MAT2>));   // python case
  expose((frovedis_lnr_scalapack<DT1,D_MAT1>));   // both spark and python case
  expose((frovedis_lnr_scalapack<DT2,D_MAT2>));   // python case
  // (3) lasso regression
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
  // (4) ridge regression
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
}

