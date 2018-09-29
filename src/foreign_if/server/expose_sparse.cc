#include "exrpc_data_storage.hpp"
#include "short_hand_sparse_type.hpp"

using namespace frovedis;

void expose_frovedis_sparse_matrix_functions() {
  // --- frovedis crs data ---
  expose(load_local_data<S_LMAT1>); //spark
  expose((create_and_set_data<S_MAT1,S_LMAT1>)); //spark
  expose(create_crs_from_local_coo_string_vectors<DT1>); //spark
  expose(create_crs_data<DT1>); //spark
  expose(load_crs_matrix<DT1>); //spark
  expose(release_data<S_MAT1>); //spark
  expose(show_data<S_MAT1>);    //spark
  expose(save_matrix<S_MAT1>);  //spark
  expose(load_local_data<S_LMAT14>); 
  expose(load_local_data<S_LMAT15>); 
  expose(load_local_data<S_LMAT24>); 
  expose(load_local_data<S_LMAT25>); 
  expose(load_local_data<S_LMAT34>); 
  expose(load_local_data<S_LMAT35>); 
  expose(load_local_data<S_LMAT44>); 
  expose(load_local_data<S_LMAT45>); 
  expose((create_crs_data<DT1,DT4,DT5>));
  expose((create_crs_data<DT1,DT5,DT5>));
  expose((create_crs_data<DT2,DT4,DT5>));
  expose((create_crs_data<DT2,DT5,DT5>));
  expose((create_crs_data<DT3,DT4,DT5>));
  expose((create_crs_data<DT3,DT5,DT5>));
  expose((create_crs_data<DT4,DT4,DT5>));
  expose((create_crs_data<DT4,DT5,DT5>));
  expose((load_crs_matrix<DT1,DT4,DT5>));
  expose((load_crs_matrix<DT1,DT5,DT5>));
  expose((load_crs_matrix<DT2,DT4,DT5>));
  expose((load_crs_matrix<DT2,DT5,DT5>));
  //expose((load_crs_matrix<DT3,DT4,DT5>)); // not supported
  //expose((load_crs_matrix<DT3,DT5,DT5>)); // not supported
  expose((load_crs_matrix<DT4,DT4,DT5>));
  expose((load_crs_matrix<DT4,DT5,DT5>));
  expose(release_data<S_MAT14>);
  expose(release_data<S_MAT15>);
  expose(release_data<S_MAT24>);
  expose(release_data<S_MAT25>);
  expose(release_data<S_MAT34>);
  expose(release_data<S_MAT35>);
  expose(release_data<S_MAT44>);
  expose(release_data<S_MAT45>);
  expose(show_data<S_MAT14>);
  expose(show_data<S_MAT15>);
  expose(show_data<S_MAT24>);
  expose(show_data<S_MAT25>);
  expose(show_data<S_MAT34>);
  expose(show_data<S_MAT35>);
  expose(show_data<S_MAT44>);
  expose(show_data<S_MAT45>);
  expose(save_matrix<S_MAT14>);
  expose(save_matrix<S_MAT15>);
  expose(save_matrix<S_MAT24>);
  expose(save_matrix<S_MAT25>);
  expose(save_matrix<S_MAT34>);
  expose(save_matrix<S_MAT35>);
  expose(save_matrix<S_MAT44>);
  expose(save_matrix<S_MAT45>);
}
