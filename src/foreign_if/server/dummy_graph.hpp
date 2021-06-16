#ifndef _DUMMY_GRAPH_HPP_
#define _DUMMY_GRAPH_HPP_

#include <frovedis/ml/graph/graph.hpp>
#include "dummy_matrix.hpp"

using namespace frovedis;

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

template <class T>
struct py_pagerank_result {
  py_pagerank_result() {}
  py_pagerank_result(std::vector<T>& result, // might be moved
                     size_t num_active_node) {
    nodeid.resize(num_active_node); auto nptr = nodeid.data();
    if (num_active_node != result.size()) {
      auto index = vector_find_not_tmax(result);
      //std::cout << num_active_node << " "  << result.size() 
      //          << " " << index.size() << std::endl;
      auto iptr = index.data();
      auto resptr = result.data();
      rank.resize(num_active_node); auto rptr = rank.data();
      for(size_t i = 0; i < num_active_node; ++i) {
        auto id = iptr[i]; 
        nptr[i] = static_cast<long>(id + 1); // 1-based
        rptr[i] = resptr[id];
      }
    }
    else {
      rank.swap(result);
      for(size_t i = 0; i < num_active_node; ++i) {
        nptr[i] = static_cast<long>(i + 1); // 1-based
      }
    }
  }
  std::vector<long> nodeid;
  std::vector<T> rank; 
  SERIALIZE(nodeid, rank)
};

#endif
