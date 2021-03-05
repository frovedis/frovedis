#include "../exrpc/exrpc_expose.hpp"

using namespace frovedis;

void expose_frovedis_dvector_functions();
void expose_frovedis_sparse_matrix_functions();
void expose_frovedis_dense_matrix_functions();
void expose_frovedis_ml_data_functions();
void expose_frovedis_dataframe_functions();
void expose_frovedis_wrapper_functions();
void expose_frovedis_glm_classification_functions();
void expose_frovedis_glm_regression_functions();
void expose_frovedis_tree_functions();
void expose_frovedis_other_classification_functions();
void expose_frovedis_clustering_functions();
void expose_frovedis_neighbor_functions();
void expose_frovedis_recommendation_functions();
void expose_frovedis_NL_functions();
void expose_frovedis_pattern_mining_functions();
void expose_frovedis_model_functions();
void expose_frovedis_graph_functions();

int main(int argc, char* argv[]) {
  frovedis::use_frovedis use(argc, argv);
  expose(get_nodesize);
  expose_frovedis_dvector_functions();
  expose_frovedis_sparse_matrix_functions();
  expose_frovedis_dense_matrix_functions();
  expose_frovedis_ml_data_functions();
  expose_frovedis_dataframe_functions();
  expose_frovedis_wrapper_functions();
  // --- frovedis ML trainers ---
  expose_frovedis_glm_classification_functions();
  expose_frovedis_glm_regression_functions();
  expose_frovedis_tree_functions();
  expose_frovedis_other_classification_functions();
  expose_frovedis_clustering_functions();
  expose_frovedis_neighbor_functions();
  expose_frovedis_recommendation_functions();
  expose_frovedis_NL_functions();
  expose_frovedis_pattern_mining_functions();
  expose_frovedis_model_functions();
  expose_frovedis_graph_functions();
  frovedis::init_frovedis_server(argc, argv);
  return 0;
}
