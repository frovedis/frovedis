#ifndef _GRAPH_
#define _GRAPH_

#include <frovedis/dataframe.hpp>
#include <frovedis/ml/utility/mattype.hpp>
#include <frovedis/ml/graph/graph_common.hpp>
#include <frovedis/ml/graph/opt_sssp.hpp>
#include <frovedis/ml/graph/opt_pagerank.hpp>
#include <frovedis/ml/graph/shrink_opt_pagerank.hpp>
#include <frovedis/ml/graph/opt_connected_components.hpp>

namespace frovedis {

//TODO: shrink vertices while constructing graph
template <class T, class I = size_t, class O = size_t>
struct graph {
  graph() {}
  graph(const crs_matrix<T,I,O>& mat) {
    auto sqmat = check_input(mat);
    num_outgoing = sqmat.data.map(count_edges<T,I,O>)
                             .template moveto_dvector<size_t>()
                             .gather();
    adj_mat = sqmat.transpose(); // frovedis expects src nodes to be in columns
    num_incoming = adj_mat.data.map(count_edges<T,I,O>)
                               .template moveto_dvector<size_t>()
                               .gather();
    num_edges = vector_sum(num_outgoing);
    num_vertices = adj_mat.num_col;
    is_directed = has_direction(sqmat, adj_mat);
    is_weighted = has_weight(adj_mat);
    vertices.resize(num_vertices, 1.0); // used for pagerank 
  }
  graph(const graph& gr) {
    num_edges = gr.num_edges;
    num_vertices = gr.num_vertices;
    num_incoming = gr.num_incoming;
    num_outgoing = gr.num_outgoing;
    adj_mat = gr.adj_mat;
    vertices = gr.vertices;
    is_directed = gr.is_directed;
    is_weighted = gr.is_weighted;
  } 
  void debug_print(size_t n = 0) {
    std::cout << "is directed: " << is_directed << "\n";
    std::cout << "is weighted: " << is_weighted << "\n";
    std::cout << "in-degree: \n"; debug_print_vector(num_incoming, n);
    std::cout << "out-degree: \n"; debug_print_vector(num_outgoing, n);
    std::cout << "adjacency matrix: \n"; 
    adj_mat.transpose().gather().debug_pretty_print();
  } 
  sssp_result<T,I> 
  single_source_shortest_path (size_t source_node) {
    sssp_result<T,I> ret;
    if(!is_directed && !is_weighted) {
      RLOG(DEBUG) << "sssp: undirected and unweighted graph is detected... calling bfs!\n";
      auto bfs_res = bfs(source_node); // calling bfs for faster convergence
      ret.destids.swap(bfs_res.destids);
      ret.predecessors.swap(bfs_res.predecessors);
      auto nnode_visited = bfs_res.distances.size();
      ret.distances.resize(nnode_visited); 
      auto rdptr = ret.distances.data();     // T-type result
      auto tptr = bfs_res.distances.data();  // I-type result
      for(size_t i = 0; i < nnode_visited; ++i) rdptr[i] = static_cast<T>(tptr[i]);
    }
    else { // invokes bellman-ford implementation
      ret = sssp_bf_impl(adj_mat, num_incoming, num_outgoing, source_node);
    }
    return ret;
  }
  cc_result<I> 
  connected_components(int opt_level = 2, 
                       double threshold = 0.4) {
    require(!is_directed, 
    "connected_components: currently supported for undirected graph only!\n"); 
    return cc_impl(adj_mat, num_incoming, num_outgoing, 
                   threshold, is_directed, opt_level);
  }
  bfs_result<I> 
  bfs(size_t source_node, 
      int opt_level = 1, 
      double threshold = 0.4,
      size_t depth_limit = std::numeric_limits<size_t>::max()) {
    return bfs_impl(adj_mat, num_incoming, num_outgoing, 
                    source_node, threshold, is_directed, opt_level, 
                    depth_limit);
  }
  std::vector<double> 
  pagerank(double dfactor, 
           double epsilon, 
           size_t iter_max,
           size_t& num_active_nodes,
           double threshold = 0.4,
#if defined(_SX) || defined(__ve__)
           MatType mType = HYBRID) {
#else
           MatType mType = CRS) {
#endif
    require(threshold >= 0.0 && threshold <= 1.0, 
    "pagerank: threshold value should be in between 0 to 1!\n");
    crs_matrix<double,I,O> norm_mat;
    std::vector<double> rank;
    auto nodes_with_incoming_edges = vector_count_nonzero(num_incoming);
    if(nodes_with_incoming_edges <= num_vertices * threshold) {
      RLOG(DEBUG) << "pagerank: " << nodes_with_incoming_edges << "/" 
                  << num_vertices << " nodes with incoming edges are "
                  << "detected! invoking shrink version\n";
      rank = shrink::pagerank_v1(adj_mat, num_incoming, num_outgoing, 
                                 dfactor, epsilon, iter_max,
                                 norm_mat, num_active_nodes, mType); 
    }
    else {
      rank = pagerank_v1(adj_mat, num_incoming, num_outgoing, 
                         dfactor, epsilon, iter_max,
                         norm_mat, num_active_nodes, mType); 
    }
    return rank;
  }
  std::vector<double>
  pagerank(double dfactor,
           double epsilon,
           size_t iter_max,
           double threshold = 0.4,
#if defined(_SX) || defined(__ve__)
           MatType mType = HYBRID) {
#else
           MatType mType = CRS) {
#endif
    size_t num_active_nodes;
    return pagerank(dfactor, epsilon, iter_max, 
                    num_active_nodes, threshold, mType);
  }
  graph<double,I,O>
  normalized_pagerank(double dfactor,
                      double epsilon,
                      size_t iter_max,
                      double threshold = 0.4,
#if defined(_SX) || defined(__ve__)
                      MatType mType = HYBRID) {
#else
                      MatType mType = CRS) {
#endif
    require(threshold >= 0.0 && threshold <= 1.0, 
    "pagerank: threshold value should be in between 0 to 1!\n");
    crs_matrix<double,I,O> norm_mat;
    std::vector<double> rank;
    size_t num_active_nodes;
    auto nodes_with_incoming_edges = vector_count_nonzero(num_incoming);
    if(nodes_with_incoming_edges <= num_vertices * threshold) {
      RLOG(DEBUG) << "pagerank: " << nodes_with_incoming_edges << "/" 
                  << num_vertices << " nodes with incoming edges are "
                  << "detected! invoking shrink version\n";
      rank = shrink::pagerank_v1(adj_mat, num_incoming, num_outgoing, 
                                 dfactor, epsilon, iter_max,
                                 norm_mat, num_active_nodes, mType); 
    }
    else {
      rank = pagerank_v1(adj_mat, num_incoming, num_outgoing, 
                         dfactor, epsilon, iter_max,
                         norm_mat, num_active_nodes, mType); 
    }
    graph<double,I,O> ret;
    ret.num_edges = num_edges;
    ret.num_vertices = num_vertices;
    ret.num_incoming = num_incoming;
    ret.num_outgoing = num_outgoing;
    ret.is_directed = is_directed;
    ret.is_weighted = is_weighted;
    // pagerank results
    ret.adj_mat = std::move(norm_mat);
    ret.vertices.swap(rank);
    return ret;
  }

  crs_matrix<T,I,O> adj_mat;
  std::vector<size_t> num_outgoing, num_incoming;
  std::vector<double> vertices;
  size_t num_vertices, num_edges;
  bool is_directed, is_weighted;
  SERIALIZE(adj_mat, num_outgoing, num_incoming, vertices, 
            num_vertices, num_edges, 
            is_directed, is_weighted)
};

template <class T, class I = size_t, class O = size_t>
graph<T,I,O> 
read_edgelist(const std::string& filename,
              char delim = ' ',
              bool withWeight = false) {
  //auto mat = make_crs_matrix_loadcoo<T,I,O>(filename);
  //return graph<T,I,O>(mat);
  dftable df;
  if(withWeight) {
    df = make_dftable_loadtext(filename,
                               {"unsigned long", "unsigned long", "double"},
                               {"src", "dst", "wgt"}, delim);
  }
  else {
    df = make_dftable_loadtext(filename,
                               {"unsigned long", "unsigned long"},
                               {"src", "dst"}, delim);
  }
  auto sorted_df = df.sort("dst").sort("src");
  auto rowid = vector_astype<I>(sorted_df.as_dvector<unsigned long>("src").gather());
  auto colid = vector_astype<I>(sorted_df.as_dvector<unsigned long>("dst").gather());
  // to zero-base (if not already)
  if(vector_amin(rowid) != 0) rowid = rowid - static_cast<I>(1);
  if(vector_amin(colid) != 0) colid = colid - static_cast<I>(1);
  std::vector<T> weight;
  if(withWeight) weight = vector_astype<T>(sorted_df.as_dvector<double>("wgt")
                                                    .gather());
  else weight = vector_ones<T>(rowid.size());
  auto off = prefix_sum(vector_bincount<O>(rowid));
  auto nrow = off.size();
  crs_matrix_local<T,I,O> adj_mat;
  adj_mat.local_num_row = nrow;
  adj_mat.local_num_col = std::max(vector_amax(rowid), vector_amax(colid)) + 1;
  adj_mat.val.swap(weight);
  adj_mat.idx.swap(colid);
  adj_mat.off.resize(nrow + 1);
  auto matoffp = adj_mat.off.data();
  auto offp = off.data();
  for(size_t i = 0; i < nrow; ++i) matoffp[i+1] = offp[i];
  return graph<T,I,O>(make_crs_matrix_scatter(adj_mat));
}

}
#endif
