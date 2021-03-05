#include "exrpc_ml.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"

using namespace frovedis;

void expose_frovedis_tree_functions() {
  // (1) decision tree
  //expose((frovedis_dt<DT1,S_MAT1>)); // not supported currently
  expose((frovedis_dt<DT1,D_MAT1>));   // both spark and python case
  expose((frovedis_dt<DT2,D_MAT2>));   // python case
  // (2) Random forest
  expose((frovedis_rf<DT1,D_MAT1>));   // both spark and python case
  expose((frovedis_rf<DT2,D_MAT2>));   // python case
  // (3) GBT
  expose((frovedis_gbt<DT1,D_MAT1>));   
  expose((frovedis_gbt<DT2,D_MAT2>));
}

