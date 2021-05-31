#include "exrpc_ml.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"

using namespace frovedis;

void expose_frovedis_neighbor_functions() {
  // (1) knn - Nearest Neighbors (NN)
  expose((frovedis_knn<DT1,R_MAT1>));
  expose((frovedis_knn<DT2,R_MAT2>)); 
  expose((frovedis_knn<DT1,S_MAT14>));
  expose((frovedis_knn<DT1,S_MAT25>));
  expose((frovedis_knn<DT2,S_MAT24>)); 
  expose((frovedis_knn<DT2,S_MAT25>)); 
  // (2) knc
  expose((frovedis_knc<DT1,R_MAT1>));
  expose((frovedis_knc<DT2,R_MAT2>)); 
  expose((frovedis_knc<DT1,S_MAT14>));
  expose((frovedis_knc<DT1,S_MAT15>));
  expose((frovedis_knc<DT2,S_MAT24>)); 
  expose((frovedis_knc<DT2,S_MAT25>)); 
  // (3) knr
  expose((frovedis_knr<DT1,R_MAT1>));
  expose((frovedis_knr<DT2,R_MAT2>)); 
  expose((frovedis_knr<DT1,S_MAT14>));
  expose((frovedis_knr<DT1,S_MAT15>));
  expose((frovedis_knr<DT2,S_MAT24>)); 
  expose((frovedis_knr<DT2,S_MAT25>)); 
}

