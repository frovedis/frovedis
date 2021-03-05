#include "exrpc_ml.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"

using namespace frovedis;

void expose_frovedis_recommendation_functions() {
  // (1) matrix factorization using ALS
  expose((frovedis_mf_als<DT1,S_MAT1>));  // spark case
  expose((frovedis_mf_als<DT1,S_MAT14>)); // python case
  expose((frovedis_mf_als<DT1,S_MAT15>)); // python case
  expose((frovedis_mf_als<DT2,S_MAT24>)); // python case
  expose((frovedis_mf_als<DT2,S_MAT25>)); // python case
  // (2) factorization machine
  expose((frovedis_fm<DT1,S_MAT1>));      // spark case
  expose((frovedis_fm<DT1,S_MAT14>));     // python case
  expose((frovedis_fm<DT1,S_MAT15>));     // python case
  expose((frovedis_fm<DT2,S_MAT24>));     // python case
  expose((frovedis_fm<DT2,S_MAT25>));     // python case
}
