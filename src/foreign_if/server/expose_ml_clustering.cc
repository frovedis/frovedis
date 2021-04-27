#include "exrpc_ml.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"

using namespace frovedis;

void expose_frovedis_clustering_functions() {
  // (1) kmeans clustering
  // as for dense input, kmeans supports only rowmajor_matrix<T> for dense data
  expose((frovedis_kmeans_fit<DT1,R_MAT1>));  // both spark and python case
  expose((frovedis_kmeans_fit<DT2,R_MAT2>));  // python case
  expose((frovedis_kmeans_fit<DT1,S_MAT1>));  // spark case
  expose((frovedis_kmeans_fit<DT1,S_MAT14>)); // python case
  expose((frovedis_kmeans_fit<DT1,S_MAT15>)); // python case
  expose((frovedis_kmeans_fit<DT2,S_MAT24>)); // python case
  expose((frovedis_kmeans_fit<DT2,S_MAT25>)); // python case
  expose((frovedis_kmeans_fit_transform<DT1,R_MAT1>));  // both spark and python case
  expose((frovedis_kmeans_fit_transform<DT2,R_MAT2>));  // python case
  expose((frovedis_kmeans_fit_transform<DT1,S_MAT1>));  // spark case
  expose((frovedis_kmeans_fit_transform<DT1,S_MAT14>)); // python case
  expose((frovedis_kmeans_fit_transform<DT1,S_MAT15>)); // python case
  expose((frovedis_kmeans_fit_transform<DT2,S_MAT24>)); // python case
  expose((frovedis_kmeans_fit_transform<DT2,S_MAT25>)); // python case
  // (2) hierarchical clustering
  expose((frovedis_aca<DT1,R_MAT1>));  // python+spark case (fit-predict)
  expose((frovedis_aca<DT2,R_MAT2>));  // python case (fit-predict)
  // (3) spectral clustering
  expose((frovedis_sca<DT1,R_MAT1>));
  expose((frovedis_sca<DT2,R_MAT2>)); // python case
  // (4) spectral embedding
  expose((frovedis_sea<DT1,R_MAT1>));
  expose((frovedis_sea<DT2,R_MAT2>)); // python case
  // (5) dbscan
  expose((frovedis_dbscan<DT1,R_MAT1>)); // python case (fit-predict)
  expose((frovedis_dbscan<DT2,R_MAT2>)); // python case (fit-predict)
  // (6) gaussian mixture
  expose((frovedis_gmm<DT1,R_MAT1>)); // python case
  expose((frovedis_gmm<DT2,R_MAT2>)); // python case   
}

