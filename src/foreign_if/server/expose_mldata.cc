#include "exrpc_data_storage.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"

using namespace frovedis;

void expose_frovedis_ml_data_functions() {
  // --- frovedis glm data ---
  expose((load_local_glm_data<DT1,S_LMAT1>));
  expose((create_and_set_glm_data<DT1,S_MAT1,S_LMAT1>));
  expose((release_glm_data<DT1,S_MAT1>));
  expose((release_glm_data<DT1,R_MAT1>));
  expose((release_glm_data<DT1,C_MAT1>));
  expose((show_glm_data<DT1,S_MAT1>));
  expose((show_glm_data<DT1,R_MAT1>));
  expose((show_glm_data<DT1,C_MAT1>));
  // --- frovedis generic data ---
  expose(load_local_data<std::vector<std::string>>);
  expose((release_data<lvec<int>>));           //ScaLAPACK(IPIV)
}
