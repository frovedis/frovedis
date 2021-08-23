#include "exrpc_ml.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"

using namespace frovedis;

void expose_frovedis_other_classification_functions() {
  // (1) naive bayes
  expose((frovedis_nb<DT1,R_MAT1>));     // both spark and python case
  expose((frovedis_nb<DT2,R_MAT2>));     // python case
  expose((frovedis_nb<DT1,S_MAT1>));     // spark case
  expose((frovedis_nb<DT1,S_MAT14>));    // python case
  expose((frovedis_nb<DT1,S_MAT15>));    // python case
  expose((frovedis_nb<DT2,S_MAT24>));    // python case
  expose((frovedis_nb<DT2,S_MAT25>));    // python case
  // (2) svm kernel
  expose((frovedis_svc<DT1,R_MAT1>));    // both spark and python case
  expose((frovedis_svc<DT2,R_MAT2>));    // python case
  //expose((frovedis_svc<DT1,S_MAT1>));  // spark case
  //expose((frovedis_svc<DT1,S_MAT14>)); // python case
  //expose((frovedis_svc<DT1,S_MAT15>)); // python case
  //expose((frovedis_svc<DT2,S_MAT24>)); // python case
  //expose((frovedis_svc<DT2,S_MAT25>)); // python case
  // (3) Standard Scaler
  expose((frovedis_scaler_partial_fit<DT1,R_MAT1>));
  expose((frovedis_scaler_partial_fit<DT2,R_MAT2>));
  expose((frovedis_scaler_partial_fit<DT1,S_MAT14>));
  expose((frovedis_scaler_partial_fit<DT1,S_MAT15>));
  expose((frovedis_scaler_partial_fit<DT2,S_MAT24>));
  expose((frovedis_scaler_partial_fit<DT2,S_MAT25>));
  //transform
  expose((frovedis_scaler_transform<DT1,R_MAT1,R_MAT1,R_LMAT1>));
  expose((frovedis_scaler_transform<DT2,R_MAT2,R_MAT2,R_LMAT2>));   // for python
  expose((frovedis_scaler_transform<DT2,S_MAT24,S_MAT24,S_LMAT24>));
  expose((frovedis_scaler_transform<DT2,S_MAT25,S_MAT25,S_LMAT25>));
  expose((frovedis_scaler_transform<DT1,S_MAT14,S_MAT14,S_LMAT14>));
  expose((frovedis_scaler_transform<DT1,S_MAT15,S_MAT15,S_LMAT15>));

  expose((frovedis_scaler_inverse_transform<DT2,R_MAT2,R_MAT2,R_LMAT2>));
  expose((frovedis_scaler_inverse_transform<DT1,R_MAT1,R_MAT1,R_LMAT1>));
  expose((frovedis_scaler_inverse_transform<DT2,S_MAT24,S_MAT24,S_LMAT24>));
  expose((frovedis_scaler_inverse_transform<DT2,S_MAT25,S_MAT25,S_LMAT25>));
  expose((frovedis_scaler_inverse_transform<DT1,S_MAT14,S_MAT14,S_LMAT14>));
  expose((frovedis_scaler_inverse_transform<DT1,S_MAT15,S_MAT15,S_LMAT15>));    
}

