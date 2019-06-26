#include "exrpc_data_storage.hpp"
#include "short_hand_dense_type.hpp"

using namespace frovedis;

void expose_frovedis_dense_matrix_functions() {
  // --- frovedis dense data ---
  expose(load_local_data<R_LMAT1>);
  expose(load_local_data<R_LMAT2>);
  expose(load_local_data<R_LMAT3>);
  expose(load_local_data<R_LMAT4>);
  expose(create_rmm_data<DT1>);
  expose(create_rmm_data<DT2>);
  expose(create_rmm_data<DT3>);
  expose(create_rmm_data<DT4>);
  expose(create_cmm_data<DT1>);
  expose(create_cmm_data<DT2>);
  expose(create_cmm_data<DT3>);
  expose(create_cmm_data<DT4>);
  expose(create_bcm_data<DT1>);
  expose(create_bcm_data<DT2>);
  expose(load_rmm_matrix<DT1>);
  expose(load_rmm_matrix<DT2>);
  //expose(load_rmm_matrix<DT3>); // not supported
  expose(load_rmm_matrix<DT4>);
  expose(load_cmm_matrix<DT1>);
  expose(load_cmm_matrix<DT2>);
  //expose(load_cmm_matrix<DT3>); // not supported
  expose(load_cmm_matrix<DT4>);
  expose(load_bcm_matrix<DT1>);
  expose(load_bcm_matrix<DT2>);
  expose(release_data<R_MAT2>);
  expose(release_data<R_MAT3>);
  expose(release_data<R_MAT4>);
  expose(release_data<R_MAT1>);
  expose(release_data<C_MAT1>);
  expose(release_data<C_MAT2>);
  expose(release_data<C_MAT3>);
  expose(release_data<C_MAT4>);
  expose(release_data<B_MAT1>);
  expose(release_data<B_MAT2>);
  expose(show_data<R_MAT1>);
  expose(show_data<R_MAT2>);
  expose(show_data<R_MAT3>);
  expose(show_data<R_MAT4>);
  expose(show_data<C_MAT1>);
  expose(show_data<C_MAT2>);
  expose(show_data<C_MAT3>);
  expose(show_data<C_MAT4>);
  expose(show_data<B_MAT1>);
  expose(show_data<B_MAT2>);
  expose(save_matrix<R_MAT1>);
  expose(save_matrix<R_MAT2>);
  expose(save_matrix<R_MAT3>);
  expose(save_matrix<R_MAT4>);
  expose(save_matrix<C_MAT1>);
  expose(save_matrix<C_MAT2>);
  expose(save_matrix<C_MAT3>); 
  expose(save_matrix<C_MAT4>);
  expose(save_matrix<B_MAT1>);
  expose(save_matrix<B_MAT2>);
  expose(transpose_matrix<R_MAT1>);
  expose(transpose_matrix<R_MAT2>);
  expose(transpose_matrix<R_MAT3>);
  expose(transpose_matrix<R_MAT4>);
  //expose(transpose_matrix<C_MAT1>); // not yet supported
  //expose(transpose_matrix<C_MAT2>); // not yet supported
  //expose(transpose_matrix<C_MAT3>); // not yet supported
  //expose(transpose_matrix<C_MAT4>); // not yet supported
  expose(transpose_matrix<B_MAT1>);
  expose(transpose_matrix<B_MAT2>);
  //expose(copy_matrix<R_MAT1>); // not yet supported
  //expose(copy_matrix<C_MAT1>); // not yet supported
  expose(copy_matrix<B_MAT1>);
  expose(copy_matrix<B_MAT2>);
  expose((to_rowmajor_matrix<DT1,C_MAT1>));
  expose((to_rowmajor_matrix<DT2,C_MAT2>));
  expose((to_rowmajor_matrix<DT3,C_MAT3>));
  expose((to_rowmajor_matrix<DT4,C_MAT4>));
  expose((to_rowmajor_matrix<DT1,B_MAT1>));
  expose((to_rowmajor_matrix<DT2,B_MAT2>));
  expose((to_rowmajor_array<DT1,C_MAT1>));
  expose((to_rowmajor_array<DT1,B_MAT1>));
  expose(rowmajor_to_colmajor_array<DT1>);
  expose(colmajor_to_colmajor_array<DT1>);
  expose(blockcyclic_to_colmajor_array<DT1>);
  expose((matrix_to_array<DT1,R_MAT1,R_LMAT1>));
  expose((matrix_to_array<DT1,C_MAT1,C_LMAT1>));
  expose((matrix_to_array<DT1,B_MAT1,B_LMAT1>));
  expose((convert_and_get_all_rml_pointers<DT1,C_MAT1>));
  expose((convert_and_get_all_rml_pointers<DT2,C_MAT2>));
  expose((convert_and_get_all_rml_pointers<DT3,C_MAT3>));
  expose((convert_and_get_all_rml_pointers<DT4,C_MAT4>));
  expose((convert_and_get_all_rml_pointers<DT1,B_MAT1>));
  expose((convert_and_get_all_rml_pointers<DT2,B_MAT2>));
  expose((get_all_local_pointers<R_MAT1,R_LMAT1>));
  expose((get_all_local_pointers<R_MAT2,R_LMAT2>));
  expose((get_all_local_pointers<R_MAT3,R_LMAT3>));
  expose((get_all_local_pointers<R_MAT4,R_LMAT4>));
  expose((get_all_local_pointers<C_MAT1,C_LMAT1>));
  expose((get_all_local_pointers<B_MAT1,B_LMAT1>));
  expose((get_local_array<DT1,R_LMAT1>));
  expose((get_local_array<DT1,C_LMAT1>));
  expose((get_local_array<DT1,B_LMAT1>));
  expose((get_local_array<DT2,R_LMAT2>));
  expose((get_local_array<DT3,R_LMAT3>));
  expose((get_local_array<DT4,R_LMAT4>));
  expose(vec_to_array<DT1>);
  expose(vec_to_array<DT2>);
  expose(save_as_diag_matrix_local<DT1>);
  expose(save_as_diag_matrix_local<DT2>);
}
