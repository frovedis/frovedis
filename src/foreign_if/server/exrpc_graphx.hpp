#ifndef _EXRPC_GRAPHX_HPP_
#define _EXRPC_GRAPHX_HPP_

#include "../exrpc/exrpc_expose.hpp"
#include "dummy_graph.hpp"

using namespace frovedis;

template <class T>
std::vector<size_t> get_outgoing_edges(crs_matrix_local<T>& mat) {
  auto nrow = mat.local_num_row;
  std::vector<size_t> num_out(nrow);
  auto offptr = mat.off.data();
  auto retptr = num_out.data();
  for(size_t i = 0; i < nrow; ++i) {
    retptr[i] = offptr[i+1] - offptr[i];
  }
  return num_out;
}

template <class DATA>
exrpc_ptr_t set_graph_data(exrpc_ptr_t& dptr) {
  auto& mat = *reinterpret_cast<DATA*>(dptr); // adjacency matrix
  if(mat.num_row != mat.num_col)
    REPORT_ERROR(USER_ERROR, "Input matrix needs to be a sqaure matrix!\n");
  auto num_outgoing = mat.data
                         .map(get_outgoing_edges<TYPE_MATRIX_PAGERANK>)
                         .template as_dvector<size_t>().gather();
  checkAssumption(num_outgoing.size() == mat.num_row);
  auto g = new graph();
  g->A_pg = mat.transpose(); //transpose:since frovedis assumes nodes in column
  g->num_nodes = mat.num_row;
  g->num_outgoing = num_outgoing;
  return reinterpret_cast<exrpc_ptr_t>(g);
}

template <class GRAPH>
void save_graph(exrpc_ptr_t& dptr,
                std::string& fname) {
  auto& graph = *reinterpret_cast<GRAPH*>(dptr);
  graph.A_pg.save(fname);
}

template <class GRAPH>
dummy_graph load_graph(std::string& fname) {
  auto mat = make_crs_matrix_load<TYPE_MATRIX_PAGERANK>(fname);
  if(mat.num_row != mat.num_col)
    REPORT_ERROR(USER_ERROR, "Loaded matrix needs to be a sqaure matrix!\n");
  //transpose needed, sinced saved graph considers nodes in column
  auto num_outgoing = mat.transpose() 
                         .data
                         .map(get_outgoing_edges<TYPE_MATRIX_PAGERANK>)
                         .template as_dvector<size_t>().gather();
  checkAssumption(num_outgoing.size() == mat.num_row);
  auto g = new GRAPH();
  g->A_pg = mat;
  g->num_nodes = mat.num_row;
  g->num_outgoing = num_outgoing;
  return to_dummy_graph<double>(g); // double: for pagerank
}

template <class GRAPH>
crs_matrix_local<TYPE_MATRIX_PAGERANK>
get_graph_data(exrpc_ptr_t& dptr) {
  auto& graph = *reinterpret_cast<GRAPH*>(dptr);
  return graph.A_pg.gather();
}

template <class T>
void normalize_local_graph(crs_matrix_local<T>& mat,
                           std::vector<T>& nodeWeight) {
  auto nzero = mat.val.size(); 
  auto valptr = mat.val.data();
  auto idxptr = mat.idx.data();
  auto wgtptr = nodeWeight.data();
  for(size_t i = 0; i < nzero; ++i) { valptr[i] = wgtptr[idxptr[i]]; }
}

template <class GRAPH>
void normalize_graph(exrpc_ptr_t& dptr) {
  auto& graph = *reinterpret_cast<GRAPH*>(dptr);
  std::vector<TYPE_MATRIX_PAGERANK> nodeWeight(graph.num_nodes);
  auto countptr = graph.num_outgoing.data();
  auto wgtptr = nodeWeight.data();
  for(size_t i = 0; i < graph.num_nodes; ++i) {
    if (countptr[i] != 0) 
      wgtptr[i] = 1 / (double) countptr[i];
    else
      wgtptr[i] = 0;
  }
  graph.A_pg.data.mapv(normalize_local_graph<TYPE_MATRIX_PAGERANK>, 
                       broadcast(nodeWeight));
}

template <class GRAPH>
exrpc_ptr_t copy_graph(exrpc_ptr_t& dptr) {
  auto& input = *reinterpret_cast<GRAPH*>(dptr);
  auto g = new graph();
  g->A_pg = input.A_pg;
  g->num_nodes = input.num_nodes;
  g->num_outgoing = input.num_outgoing;
  return reinterpret_cast<exrpc_ptr_t>(g);
}

template <class GRAPH>
void show_graph(exrpc_ptr_t& dptr) {
  auto& graph = *reinterpret_cast<GRAPH*>(dptr);
  graph.A_pg.debug_print();
}

template <class GRAPH>
void release_graph(exrpc_ptr_t& dptr) {
  auto graph = reinterpret_cast<GRAPH*>(dptr);
  delete graph;
}

template <class GRAPH>
std::vector<double> 
frovedis_pagerank(exrpc_ptr_t& dptr, 
                  double& epsilon, 
                  double& dfactor,
                  int& max_iter, 
                  int& verbose) { 
  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);
  normalize_graph<GRAPH>(dptr); // normalize edge weights before page rank
  auto& graph = *reinterpret_cast<GRAPH*>(dptr);
#if !defined(_SX) && !defined(__ve__)
  graph.pagerank_v1(std::string("CRS"), dfactor, epsilon, max_iter);
#else
  graph.pagerank_v1(std::string("HYB"), dfactor, epsilon, max_iter);
#endif
  frovedis::set_loglevel(old_level);
  return graph.get_prank();
}

template <class GRAPH>
sssp_result frovedis_sssp(exrpc_ptr_t& dptr, 
                          long& source_vertex,
                          int& verbose) { 
  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);
  auto& graph = *reinterpret_cast<GRAPH*>(dptr);
  if ((graph.A_pg.num_row != graph.A.num_row) || 
      (graph.A_pg.num_col != graph.A.num_col)) 
    graph.A = graph.A_pg.template change_datatype<int>();
  graph.sssp_bf_spmv(false, source_vertex);
  frovedis::set_loglevel(old_level);
  return sssp_result(graph.get_nodes_dist(), graph.get_nodes_pred());
}

template <class GRAPH>
bfs_result frovedis_bfs(exrpc_ptr_t& dptr, 
                        int& verbose) { 
  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);
  auto& graph = *reinterpret_cast<GRAPH*>(dptr);
  if ((graph.A_pg.num_row != graph.A.num_row) || 
      (graph.A_pg.num_col != graph.A.num_col)) 
    graph.A = graph.A_pg.template change_datatype<int>();
  graph.cc_bfs_scatter();
  frovedis::set_loglevel(old_level);
  return bfs_result(graph.get_nodes_dist(), 
                    graph.get_nodes_in_which_cc(),
                    graph.get_num_cc(),
                    graph.get_num_nodes_in_each_cc());
}
#endif
