#include "exrpc_graphx.hpp"
#include "short_hand_sparse_type.hpp"

using namespace frovedis;

void expose_frovedis_graph_functions(){
  expose(save_graph<graph<DT1>>);
  expose((load_graph<graph<DT1>,DT1>));
  expose(copy_graph<graph<DT1>>);
  expose(show_graph<graph<DT1>>);
  expose(release_graph<graph<DT1>>);
  expose((set_graph_data<S_MAT1,DT1>));
  expose((get_graph_edge_data<graph<DT1>,DT1>));
  expose(get_graph_vertex_data<graph<DT1>>);
  expose(get_graph_data_dummy_matrix<graph<DT1>>);
  expose(frovedis_pagerank<graph<DT1>>);
  expose(frovedis_normalized_pagerank<graph<DT1>>);
  expose((frovedis_sssp<graph<DT1>,DT1,DT5>));
  expose((frovedis_bfs<graph<DT1>,DT5>));
  expose((frovedis_cc<graph<DT1>,DT5>));
}
