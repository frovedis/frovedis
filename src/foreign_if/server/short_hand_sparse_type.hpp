#ifndef _SHORT_HAND_SPARSE_TYPE_
#define _SHORT_HAND_SPARSE_TYPE_ 

#include "short_hand_dtype.hpp"

namespace frovedis {
  // --- ALL SPARSE Types ---
  typedef crs_matrix<DT1> S_MAT1;
  typedef crs_matrix<DT2> S_MAT2;
  typedef crs_matrix_local<DT1> S_LMAT1;
  typedef crs_matrix_local<DT2> S_LMAT2;
  typedef crs_matrix<DT1,DT4> S_MAT14;
  typedef crs_matrix<DT1,DT5> S_MAT15;
  typedef crs_matrix<DT2,DT4> S_MAT24;
  typedef crs_matrix<DT2,DT5> S_MAT25;
  typedef crs_matrix<DT3,DT4> S_MAT34;
  typedef crs_matrix<DT3,DT5> S_MAT35;
  typedef crs_matrix<DT4,DT4> S_MAT44;
  typedef crs_matrix<DT4,DT5> S_MAT45;
  typedef crs_matrix_local<DT1,DT4> S_LMAT14;
  typedef crs_matrix_local<DT1,DT5> S_LMAT15;
  typedef crs_matrix_local<DT2,DT4> S_LMAT24;
  typedef crs_matrix_local<DT2,DT5> S_LMAT25;
  typedef crs_matrix_local<DT3,DT4> S_LMAT34;
  typedef crs_matrix_local<DT3,DT5> S_LMAT35;
  typedef crs_matrix_local<DT4,DT4> S_LMAT44;
  typedef crs_matrix_local<DT4,DT5> S_LMAT45;
}
#endif
