#ifndef _EXRPC_GRAPHX_HPP_
#define _EXRPC_GRAPHX_HPP_

#include "../exrpc/exrpc_expose.hpp"
#include "dummy_graph.hpp"

using namespace frovedis;

template <class DATA, class T>
exrpc_ptr_t set_graph_data(exrpc_ptr_t& dptr) {
  auto& mat = *reinterpret_cast<DATA*>(dptr); // adjacency matrix
  auto g = new graph<T>(mat);
  return reinterpret_cast<exrpc_ptr_t>(g);
}

template <class GRAPH>
void save_graph(exrpc_ptr_t& dptr,
                std::string& fname) {
  auto& graph = *reinterpret_cast<GRAPH*>(dptr);
  graph.adj_mat.transpose().save(fname);
}

template <class GRAPH, class T>
dummy_graph load_graph(std::string& fname) {
  auto mat = make_crs_matrix_load<T>(fname);
  auto g = new GRAPH(mat);
  return to_dummy_graph<double>(g); 
}

template <class GRAPH, class T>
crs_matrix_local<T>
get_graph_edge_data(exrpc_ptr_t& dptr) {
  auto& graph = *reinterpret_cast<GRAPH*>(dptr);
  return graph.adj_mat.gather();
}

template <class GRAPH>
std::vector<double>
get_graph_vertex_data(exrpc_ptr_t& dptr) {
  auto& graph = *reinterpret_cast<GRAPH*>(dptr);
  return graph.vertices;
}

template <class GRAPH>
dummy_matrix
get_graph_data_dummy_matrix(exrpc_ptr_t& dptr) {
  auto& graph = *reinterpret_cast<GRAPH*>(dptr);
  auto cmat = new crs_matrix<double>(graph.adj_mat);
  return to_dummy_matrix<crs_matrix<double>, crs_matrix_local<double>>(cmat);
}

template <class GRAPH>
exrpc_ptr_t copy_graph(exrpc_ptr_t& dptr) {
  auto& input = *reinterpret_cast<GRAPH*>(dptr);
  auto g = new GRAPH(input);
  return reinterpret_cast<exrpc_ptr_t>(g);
}

template <class GRAPH>
void show_graph(exrpc_ptr_t& dptr) {
  auto& graph = *reinterpret_cast<GRAPH*>(dptr);
  graph.debug_print();
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
  auto& graph = *reinterpret_cast<GRAPH*>(dptr);
  auto rank = graph.pagerank(dfactor, epsilon, max_iter);
  frovedis::set_loglevel(old_level);
  return rank;
}

template <class GRAPH> 
dummy_graph
frovedis_normalized_pagerank(exrpc_ptr_t& dptr,
                             double& epsilon,
                             double& dfactor,
                             int& max_iter,
                             int& verbose) {
  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);
  auto& graph = *reinterpret_cast<GRAPH*>(dptr);
  auto ret = graph.normalized_pagerank(dfactor, epsilon, max_iter);
  frovedis::set_loglevel(old_level);
  return to_dummy_graph(new GRAPH(std::move(ret)));
}

template <class GRAPH, class T, class I>
sssp_result<T,I> 
frovedis_sssp(exrpc_ptr_t& dptr, 
              long& source_vertex,
              int& verbose) { 
  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);
  auto& graph = *reinterpret_cast<GRAPH*>(dptr);
  auto ret = graph.single_source_shortest_path(source_vertex);
  frovedis::set_loglevel(old_level);
  return ret;
}

template <class GRAPH, class I>
bfs_result<I>
frovedis_bfs(exrpc_ptr_t& dptr,
             long& source_vertex,
             int& opt_level,
             double& hyb_threshold,
             int& verbose) {
  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);
  auto& graph = *reinterpret_cast<GRAPH*>(dptr);
  auto ret = graph.bfs(source_vertex, opt_level, hyb_threshold);
  frovedis::set_loglevel(old_level);
  return ret;
}

template <class GRAPH, class I>
cc_result<I> 
frovedis_cc(exrpc_ptr_t& dptr,
            int& opt_level,
            double& hyb_threshold, 
            int& verbose) { 
  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);
  auto& graph = *reinterpret_cast<GRAPH*>(dptr);
  auto ret = graph.connected_components(opt_level, hyb_threshold);
  frovedis::set_loglevel(old_level);
  return ret;
}

#endif
