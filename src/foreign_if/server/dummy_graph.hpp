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

struct sssp_result {
  sssp_result() {}
  sssp_result(std::vector<int>& dist, 
              std::vector<size_t>& pred) {
    checkAssumption(dist.size() == pred.size());
    nodes_dist.resize(dist.size());
    nodes_pred.resize(pred.size());
    auto distptr = dist.data();
    auto predptr = pred.data();
    auto nodes_distptr = nodes_dist.data();
    auto nodes_predptr = nodes_pred.data();
    for(size_t i = 0; i < pred.size(); ++i) {
      nodes_distptr[i] = distptr[i]; // no need for type casting
      nodes_predptr[i] = static_cast<long> (predptr[i]);
    }    
  }
  std::vector<int> nodes_dist;
  std::vector<long> nodes_pred;
  SERIALIZE(nodes_dist, nodes_pred)
};

struct bfs_result{
  bfs_result() {}
  bfs_result(std::vector<int>& dist,
             std::vector<size_t>& nid,
             size_t num_cc,
             std::vector<size_t>& nn) {
    checkAssumption(dist.size() == nid.size());
    auto num_nodes = dist.size();
    nodes_dist.resize(num_nodes);
    nodes_in_which_cc.resize(num_nodes); // nid
    num_nodes_in_each_cc.resize(num_cc); // nn
    auto distptr = dist.data();
    auto nidptr = nid.data();
    auto nnptr = nn.data();
    auto res_distptr = nodes_dist.data();
    auto res_nidptr = nodes_in_which_cc.data(); // nid
    auto res_nnptr = num_nodes_in_each_cc.data(); // nn
    for(size_t i = 0; i < num_nodes; ++i) {
      res_distptr[i] = distptr[i]; // no need for type casting
      res_nidptr[i] = static_cast<long> (nidptr[i]);
    }
    for(size_t i = 0; i < num_cc; ++i){
      // extracting first num_cc elements
      res_nnptr[i] = static_cast<long> (nnptr[i]); 
    }
  }  
  std::vector<int> nodes_dist;
  std::vector<long> num_nodes_in_each_cc;
  std::vector<long> nodes_in_which_cc;
  SERIALIZE(nodes_dist, num_nodes_in_each_cc, nodes_in_which_cc)
};

}
#endif
