#ifndef _GRAPH_
#define _GRAPH_

#include <frovedis/ml/utility/mattype.hpp>
#include <frovedis/ml/graph/opt_sssp.hpp>
#include <frovedis/ml/graph/opt_pagerank.hpp>
#include <frovedis/ml/graph/opt_connected_components.hpp>

namespace frovedis {

template <class T, class I, class O>
std::vector<size_t> 
get_outgoing_edges(crs_matrix_local<T,I,O>& mat) {
  auto nrow = mat.local_num_row;
  std::vector<size_t> num_out(nrow);
  auto offptr = mat.off.data();
  auto retptr = num_out.data();
  for(size_t i = 0; i < nrow; ++i) {
    retptr[i] = offptr[i+1] - offptr[i];
  }
  return num_out;
}

template <class T, class I, class O>
size_t get_local_nnz(crs_matrix_local<T,I,O>& mat) {
  return mat.val.size();
}

template <class T>
T sum(T& a, T& b) { return a + b; }

template <class T, class I, class O>
void set_local_ncol(crs_matrix_local<T,I,O>& mat, size_t ncol) {
  mat.local_num_col = ncol;
}

template <class T, class I, class O>
crs_matrix<T,I,O>
check_input(crs_matrix<T,I,O>& mat) {
  auto nrow = mat.num_row;
  auto ncol = mat.num_col;
  if(nrow != ncol && ncol > nrow) 
    REPORT_ERROR(USER_ERROR, "input is not a square matrix!\n");
  if (nrow == ncol) return mat;
  else {
    auto ret = mat;
    ret.num_row = ret.num_col = nrow;
    ret.data.mapv(set_local_ncol<T,I,O>, broadcast(nrow));
    return ret;
  }
}

template <class T, class I, class O>
int check_equal_helper(crs_matrix_local<T,I,O>& amat,
                       crs_matrix_local<T,I,O>& bmat) {
  return (amat.local_num_row == bmat.local_num_row && 
          amat.local_num_col == bmat.local_num_col &&
          amat.val == bmat.val && amat.idx == bmat.idx && amat.off == bmat.off);
}

template <class T, class I, class O>
bool check_equal(crs_matrix<T,I,O>& amat,
                 crs_matrix<T,I,O>& bmat) {
  auto eq = amat.data.map(check_equal_helper<T,I,O>, bmat.data).gather();
  for(auto each: eq) if (!each) return false;
  return true;
}

template <class T, class I = size_t, class O = size_t>
struct graph {
  graph() {}
  graph(crs_matrix<T,I,O>& mat) {
    auto sqmat = check_input(mat);
    num_vertices = sqmat.num_col;
    num_edges = sqmat.data.map(get_local_nnz<T,I,O>)
                          .reduce(sum<size_t>);
    num_outgoing = sqmat.data.map(get_outgoing_edges<T,I,O>)
                             .template moveto_dvector<size_t>()
                             .gather();
    adj_mat = sqmat.transpose(); // frovedis expects src nodes to be in columns
    vertices.resize(num_vertices, 1.0); 
    if (check_equal(sqmat, adj_mat)) is_direct = false;
    else                             is_direct = true;
  }
  graph(const graph& gr) {
    num_edges = gr.num_edges;
    num_vertices = gr.num_vertices;
    num_outgoing = gr.num_outgoing;
    adj_mat = gr.adj_mat;
    is_direct = gr.is_direct;
    vertices = gr.vertices;
  }  
  sssp_result<T,I> single_source_shortest_path (size_t srcid) {
    return sssp_bf_impl(adj_mat, srcid);
  }
  cc_result<I> connected_components() {
    return cc(adj_mat, is_direct);
  }
  std::vector<double> 
  pagerank(double dfactor, 
           double epsilon, 
           size_t iter_max,
#if defined(_SX) || defined(__ve__)
           MatType mType = HYBRID) {
#else
           MatType mType = CRS) {
#endif
    crs_matrix<double,I,O> norm_mat;
    return pagerank_v1(adj_mat, dfactor, epsilon, iter_max, 
                       norm_mat, mType); 
  }
  graph<double,I,O>
  normalized_pagerank(double dfactor,
                      double epsilon,
                      size_t iter_max,
#if defined(_SX) || defined(__ve__)
                      MatType mType = HYBRID) {
#else
                      MatType mType = CRS) {
#endif
    crs_matrix<double,I,O> norm_mat;
    auto rank =  pagerank_v1(adj_mat, dfactor, epsilon, iter_max,
                             norm_mat, mType);
    graph<double,I,O> ret;
    ret.adj_mat = norm_mat;
    ret.num_edges = num_edges;
    ret.num_vertices = num_vertices;
    ret.num_outgoing = num_outgoing;
    ret.is_direct = is_direct;
    ret.vertices = rank;
    return ret;
  }

  crs_matrix<T,I,O> adj_mat;
  std::vector<size_t> num_outgoing;
  std::vector<double> vertices;
  size_t num_vertices, num_edges;
  bool is_direct;
  SERIALIZE(adj_mat, num_outgoing, vertices, num_vertices, num_edges, is_direct)
};

template <class T, class I = size_t, class O = size_t>
graph<T,I,O> read_edgelist(const std::string& filename) {
  auto mat = make_crs_matrix_loadcoo<T,I,O>(filename);
  return graph<T,I,O>(mat);
}

}
#endif
