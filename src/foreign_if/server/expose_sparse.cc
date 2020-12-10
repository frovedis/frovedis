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
  expose((get_crs_matrix_local<DT1,DT4,DT5>));
  expose((get_crs_matrix_local<DT1,DT5,DT5>));
  expose((get_crs_matrix_local<DT2,DT4,DT5>));
  expose((get_crs_matrix_local<DT2,DT5,DT5>));
  expose((get_crs_matrix_local<DT3,DT4,DT5>));
  expose((get_crs_matrix_local<DT3,DT5,DT5>));
  expose((get_crs_matrix_local<DT4,DT4,DT5>));
  expose((get_crs_matrix_local<DT4,DT5,DT5>));
  expose((get_all_crs_matrix_local_pointers<DT1,DT4,DT5>));
  expose((get_all_crs_matrix_local_pointers<DT1,DT5,DT5>));
  expose((get_all_crs_matrix_local_pointers<DT2,DT4,DT5>));
  expose((get_all_crs_matrix_local_pointers<DT2,DT5,DT5>));
  expose((get_all_crs_matrix_local_pointers<DT3,DT4,DT5>));
  expose((get_all_crs_matrix_local_pointers<DT3,DT5,DT5>));
  expose((get_all_crs_matrix_local_pointers<DT4,DT4,DT5>));
  expose((get_all_crs_matrix_local_pointers<DT4,DT5,DT5>));
  expose((get_computed_spmv<DT1,DT4,DT5>));
  expose((get_computed_spmv<DT1,DT5,DT5>));
  expose((get_computed_spmv<DT2,DT4,DT5>));
  expose((get_computed_spmv<DT2,DT5,DT5>));
  expose((get_computed_spmv<DT3,DT4,DT5>));
  expose((get_computed_spmv<DT3,DT5,DT5>));
  expose((get_computed_spmv<DT4,DT4,DT5>));
  expose((get_computed_spmv<DT4,DT5,DT5>));
  expose((load_crs_matrix<DT1,DT4,DT5>));
  expose((load_crs_matrix<DT1,DT5,DT5>));
  expose((load_crs_matrix<DT2,DT4,DT5>));
  expose((load_crs_matrix<DT2,DT5,DT5>));
  expose((transpose_matrix<S_MAT44,S_LMAT44>));
  expose((transpose_matrix<S_MAT45,S_LMAT45>));
  expose((transpose_matrix<S_MAT34,S_LMAT34>));
  expose((transpose_matrix<S_MAT35,S_LMAT35>));
  expose((transpose_matrix<S_MAT24,S_LMAT24>));
  expose((transpose_matrix<S_MAT25,S_LMAT25>));
  expose((transpose_matrix<S_MAT14,S_LMAT14>));
  expose((transpose_matrix<S_MAT15,S_LMAT15>));
  expose((to_rowmajor_matrix<DT4,S_MAT44>));
  expose((to_rowmajor_matrix<DT4,S_MAT45>));
  expose((to_rowmajor_matrix<DT3,S_MAT34>));
  expose((to_rowmajor_matrix<DT3,S_MAT35>));
  expose((to_rowmajor_matrix<DT2,S_MAT24>));
  expose((to_rowmajor_matrix<DT2,S_MAT25>));
  expose((to_rowmajor_matrix<DT1,S_MAT14>));
  expose((to_rowmajor_matrix<DT1,S_MAT15>));
  expose((to_colmajor_matrix<DT4,S_MAT44>));
  expose((to_colmajor_matrix<DT4,S_MAT45>));
  expose((to_colmajor_matrix<DT3,S_MAT34>));
  expose((to_colmajor_matrix<DT3,S_MAT35>));
  expose((to_colmajor_matrix<DT2,S_MAT24>));
  expose((to_colmajor_matrix<DT2,S_MAT25>));
  expose((to_colmajor_matrix<DT1,S_MAT14>));
  expose((to_colmajor_matrix<DT1,S_MAT15>));
  //expose((load_crs_matrix<DT3,DT4,DT5>)); // not supported
  //expose((load_crs_matrix<DT3,DT5,DT5>)); // not supported
  expose((load_crs_matrix<DT4,DT4,DT5>));
  expose((load_crs_matrix<DT4,DT5,DT5>));
  expose((to_dummy_matrix<S_MAT44,S_LMAT44>));
  expose((to_dummy_matrix<S_MAT45,S_LMAT45>));
  expose((to_dummy_matrix<S_MAT34,S_LMAT34>));
  expose((to_dummy_matrix<S_MAT35,S_LMAT35>));
  expose((to_dummy_matrix<S_MAT24,S_LMAT24>));
  expose((to_dummy_matrix<S_MAT25,S_LMAT25>));
  expose((to_dummy_matrix<S_MAT14,S_LMAT14>));
  expose((to_dummy_matrix<S_MAT15,S_LMAT15>));
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
  expose((get_global_data<S_MAT1,S_LMAT1>));
  expose((copy_matrix<S_MAT1,S_LMAT1>));
  expose((to_rowmajor_matrix<DT1,S_MAT1>));

  // --- frovedis to_spark API --- 
  expose((get_all_nnz<DT1,DT5,DT5>));
  expose((get_all_nrow<DT1,DT5,DT5>));
}
