#ifndef _DUMMY_GRAPH_HPP_
#define _DUMMY_GRAPH_HPP_

#include <frovedis/ml/graph/graph.hpp>
#include "dummy_matrix.hpp"

namespace frovedis {

struct dummy_graph {
  dummy_graph() {}
  dummy_graph(exrpc_ptr_t ptr, size_t nEdges, size_t nNodes): 
    dptr(ptr), num_edges(nEdges), num_nodes(nNodes) {
    vertices.resize(num_nodes, 1.0);
  }
  dummy_graph(exrpc_ptr_t ptr, size_t nEdges, size_t nNodes,
              std::vector<double>& vert): 
    dptr(ptr), num_edges(nEdges), num_nodes(nNodes) {
    vertices = vert;
  }
  dummy_graph(exrpc_ptr_t ptr, size_t nEdges, size_t nNodes,
              std::vector<double>&& vert): 
    dptr(ptr), num_edges(nEdges), num_nodes(nNodes) {
    vertices.swap(vert);
  }
  exrpc_ptr_t dptr;
  size_t num_edges, num_nodes;
  std::vector<double> vertices;
  SERIALIZE(dptr, num_edges, num_nodes, vertices)
};

template <class T>
dummy_graph to_dummy_graph(graph<T>* gptr) {
  size_t nNodes = gptr->num_vertices;
  size_t nEdges = gptr->num_edges;
  auto gptr_ = reinterpret_cast<exrpc_ptr_t>(gptr);
  return dummy_graph(gptr_, nEdges, nNodes, gptr->vertices);
}

}
#endif
