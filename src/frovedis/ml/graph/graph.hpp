#ifndef _GRAPH_
#define _GRAPH_

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
  graph(crs_matrix<T,I,O>& mat) {
    auto sqmat = check_input(mat);
    num_outgoing = sqmat.data.map(count_edges<T,I,O>)
                             .template moveto_dvector<size_t>()
                             .gather();
    adj_mat = sqmat.transpose(); // frovedis expects src nodes to be in columns
    num_incoming = adj_mat.data.map(count_edges<T,I,O>)
                               .template moveto_dvector<size_t>()
                               .gather();
    num_edges = vec_sum(num_outgoing);
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
      ret.predecessors.swap(bfs_res.predecessors);
      ret.distances.resize(num_vertices); 
      auto rdptr = ret.distances.data();     // T-type result
      auto tptr = bfs_res.distances.data();  // I-type result
      auto tmax = std::numeric_limits<T>::max();
      auto imax = std::numeric_limits<I>::max();
      for(size_t i = 0; i < num_vertices; ++i) {
        if (tptr[i] == imax) rdptr[i] = tmax;
        else rdptr[i] = static_cast<T>(tptr[i]);
      }
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
      double threshold = 0.4) {
    return bfs_impl(adj_mat, num_incoming, num_outgoing, 
                    source_node, threshold, is_directed, opt_level);
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
    require(threshold >= 0.0 && threshold <= 1.0, 
    "pagerank: threshold value should be in between 0 to 1!\n");
    crs_matrix<double,I,O> norm_mat;
    std::vector<double> rank;
    auto nodes_with_incoming_edges = count_non_zero(num_incoming);
    if(nodes_with_incoming_edges <= num_vertices * threshold) {
      RLOG(DEBUG) << "pagerank: " << nodes_with_incoming_edges << "/" 
                  << num_vertices << " nodes with incoming edges are "
                  << "detected! invoking shrink version\n";
      rank = shrink::pagerank_v1(adj_mat, num_incoming, num_outgoing, 
                                 dfactor, epsilon, iter_max,
                                 norm_mat, mType); 
    }
    else {
      rank = pagerank_v1(adj_mat, num_incoming, num_outgoing, 
                         dfactor, epsilon, iter_max,
                         norm_mat, mType); 
    }
    return rank;
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
    auto nodes_with_incoming_edges = count_non_zero(num_incoming);
    if(nodes_with_incoming_edges <= num_vertices * threshold) {
      RLOG(DEBUG) << "pagerank: " << nodes_with_incoming_edges << "/" 
                  << num_vertices << " nodes with incoming edges are "
                  << "detected! invoking shrink version\n";
      rank = shrink::pagerank_v1(adj_mat, num_incoming, num_outgoing, 
                                 dfactor, epsilon, iter_max,
                                 norm_mat, mType); 
    }
    else {
      rank = pagerank_v1(adj_mat, num_incoming, num_outgoing, 
                         dfactor, epsilon, iter_max,
                         norm_mat, mType); 
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
graph<T,I,O> read_edgelist(const std::string& filename) {
  auto mat = make_crs_matrix_loadcoo<T,I,O>(filename);
  return graph<T,I,O>(mat);
}

}
#endif
