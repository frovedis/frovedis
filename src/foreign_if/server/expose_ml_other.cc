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
}

