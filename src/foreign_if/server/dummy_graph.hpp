#ifndef _DUMMY_GRAPH_HPP_
#define _DUMMY_GRAPH_HPP_

#include <frovedis/ml/graph/graph.hpp>
#include "dummy_matrix.hpp"

namespace frovedis {

struct dummy_graph {
  dummy_graph() {}
  dummy_graph(exrpc_ptr_t ptr, size_t nEdges, size_t nNodes): 
    dptr(ptr), num_edges(nEdges), num_nodes(nNodes) { }

  exrpc_ptr_t dptr;
  size_t num_edges, num_nodes;
  SERIALIZE(dptr, num_edges, num_nodes)
};

template <class T>
dummy_graph to_dummy_graph(graph* gptr) {
  size_t nNodes = gptr->num_nodes;
  size_t nEdges = gptr->A_pg.data.map(get_local_val_size<crs_matrix_local<T>>)
                            .reduce(sum_size<size_t>);
  auto gptr_ = reinterpret_cast<exrpc_ptr_t>(gptr);
  return dummy_graph(gptr_, nEdges, nNodes);
}

}
#endif
