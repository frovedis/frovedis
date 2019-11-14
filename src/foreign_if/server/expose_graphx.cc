#include "exrpc_graphx.hpp"
#include "short_hand_sparse_type.hpp"

using namespace frovedis;

void expose_frovedis_graph_functions(){
  expose(save_graph<graph>);
  expose(load_graph<graph>);
  expose(copy_graph<graph>);
  expose(show_graph<graph>);
  expose(release_graph<graph>);
  expose(set_graph_data<S_MAT1>);
  expose(get_graph_data<graph>);
  expose(frovedis_pagerank<graph>);
}
