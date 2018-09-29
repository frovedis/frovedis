#include "../exrpc/exrpc_expose.hpp"

using namespace frovedis;

void expose_frovedis_dvector_functions();
void expose_frovedis_sparse_matrix_functions();
void expose_frovedis_dense_matrix_functions();
void expose_frovedis_ml_data_functions();
void expose_frovedis_dataframe_functions();
void expose_frovedis_wrapper_functions();
void expose_frovedis_ml_functions();
void expose_frovedis_model_functions();

int main(int argc, char* argv[]) {
  frovedis::use_frovedis use(argc, argv);
  expose(get_nodesize);
  expose_frovedis_dvector_functions();
  expose_frovedis_sparse_matrix_functions();
  expose_frovedis_dense_matrix_functions();
  expose_frovedis_ml_data_functions();
  expose_frovedis_dataframe_functions();
  expose_frovedis_wrapper_functions();
  expose_frovedis_ml_functions();
  expose_frovedis_model_functions();
  frovedis::init_frovedis_server(argc, argv);
  return 0;
}
