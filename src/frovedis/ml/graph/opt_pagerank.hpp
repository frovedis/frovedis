#ifndef _OPT_PAGERANK_HPP
#define _OPT_PAGERANK_HPP

#include <frovedis/core/utility.hpp>
#include <frovedis/matrix/jds_crs_hybrid.hpp>
#include "graph_common.hpp"

//#define PR_DEBUG

namespace frovedis {

template<class T, class I, class O>
void update_crs_mat(crs_matrix_local<T,I,O>& mat, 
                    std::vector<double>& normalized_weight){
  auto midxp = mat.idx.data();
  auto mvalp = mat.val.data();
  auto nwp = normalized_weight.data();
  auto mat_val_size = mat.val.size();
  for (size_t i = 0; i < mat_val_size; ++i) mvalp[i] = nwp[midxp[i]];
}

template<class T, class I, class O>
crs_matrix<double,I,O> 
prep_crs_pagerank(crs_matrix<T,I,O>& mat,
                  std::vector<size_t>& num_outgoing) {
  auto nvert = num_outgoing.size();
  std::vector<double> weight(nvert);
  auto noutptr = num_outgoing.data();
  auto wptr = weight.data();
  for(size_t i = 0; i < nvert; ++i) {
    wptr[i] = noutptr[i] > 0 ? (1.0 / noutptr[i]) : 0.0;
  }
  auto dmat = mat.template change_datatype<double,I,O>();
  dmat.data.mapv(update_crs_mat<double,I,O>, broadcast(weight));
  return dmat;
}

template<class MATRIX>
std::vector<double>
PRmat(MATRIX& m, std::vector<double>& prank, 
      double b, double dfactor) {
  auto temp_prank = m * prank;
  size_t prank_size = temp_prank.size();
  auto temp_prankp = temp_prank.data(); 
  for(size_t i = 0; i < prank_size; ++i) {
    temp_prankp[i] = (1 - dfactor) * temp_prankp[i] + dfactor * b;
  }
  return temp_prank;
}

template<class T>
T cal_abs_diff_vec(std::vector<T>& v1, 
                   std::vector<T>& v2){
  T ret = 0;
  size_t v_size = v1.size();
  auto v1p = v1.data();
  auto v2p = v2.data();
  for(size_t i = 0; i < v_size; ++i) {
    auto toadd = v1p[i] > v2p[i] ? v1p[i] - v2p[i] : v2p[i] - v1p[i];
    ret += toadd;
  }
  return ret;
}

template <class MATRIX>
void pagerank_v1_helper(MATRIX& A,
                        std::vector<double>& prank,
                        double bias, double dfactor, double epsilon,
                        size_t max_iter) {
  auto nproc = get_nodesize();
  auto myrank = get_selfid();
  time_spent spmv_t(DEBUG), comm_t(DEBUG), other_t(DEBUG);

  for(size_t i = 1; i <= max_iter; ++i) {
    spmv_t.lap_start();
    auto prank_new = PRmat<MATRIX>(A, prank, bias, dfactor);
    spmv_t.lap_stop();

    comm_t.lap_start();
    if(nproc > 1) prank_new = do_allgather(prank_new);
    comm_t.lap_stop();
        
    other_t.lap_start();        
    auto diff = cal_abs_diff_vec(prank_new, prank);
    prank.swap(prank_new);
    other_t.lap_stop();  
    if(myrank == 0) {
      RLOG(DEBUG) << "[iteration: " << i << "] diff: " << diff << "\n";
    }
    if (diff < epsilon) {
      if(myrank == 0) {
        RLOG(INFO) << "pagerank: converged in "
                   << i << " iterations!\n";
      }
      break;
    }
  }
  if (myrank == 0) {
    spmv_t.show_lap("spmv_t: ");
    comm_t.show_lap("allgather_t: ");
    other_t.show_lap("diff_check_t: ");
  }
}

template<class T, class I, class O>
std::vector<double>
pagerank_v1(crs_matrix<T,I,O>& A,
            std::vector<size_t>& num_incoming, 
            std::vector<size_t>& num_outgoing, 
            double dfactor, 
            double epsilon, 
            size_t max_iter,
            crs_matrix<double,I,O>& norm_mat, 
            MatType matformat) {
  require(epsilon > 0,
  "pagerank: expected a positive epsilon!\n");
  require(dfactor >= 0 && dfactor <= 1,
  "pagerank: damping factor should be in between 0 to 1!\n");
  require(max_iter > 0,
  "pagerank: expected a positive maximum iteration count!\n");

  time_spent t(DEBUG);
  norm_mat = prep_crs_pagerank<T>(A, num_outgoing);
  t.show("normalize input: ");

  size_t num_nodes = A.num_row;
  auto inptr = num_incoming.data();
  auto outptr = num_outgoing.data();
  size_t num_active_nodes = 0;
  for(size_t i = 0; i < num_nodes; ++i) {
    num_active_nodes += (inptr[i] != 0 || outptr[i] != 0);
  }
  double bias = 1.0 / static_cast<double>(num_active_nodes);
  double dmax = std::numeric_limits<double>::max();
  std::vector<double> prank(num_nodes, bias);
#ifdef PR_DEBUG
  std::cout << "incoming: "; debug_print_vector(num_incoming, 10);
  std::cout << "initial rank: "; debug_print_vector(prank, 10);
  std::cout << "num_nodes: " << num_nodes << "\n";
  std::cout << "num_active_nodes: " << num_active_nodes << "\n";
#endif
  auto b_prank = broadcast(prank);
  auto b_bias = broadcast(bias);
  auto b_df = broadcast(dfactor);
  auto b_eps = broadcast(epsilon);
  auto b_iter = broadcast(max_iter);
  t.show("initialization: ");

  if(matformat == HYBRID) {
    auto A_hyb = jds_crs_hybrid<double>(norm_mat);
    t.show("hybrid conversion: ");
    A_hyb.data.mapv(pagerank_v1_helper<jds_crs_hybrid_local<double,I,O>>,
                    b_prank, b_bias,
                    b_df, b_eps, b_iter);
  } 
  else {
    norm_mat.data.mapv(pagerank_v1_helper<crs_matrix_local<double,I,O>>,
                       b_prank, b_bias,
                       b_df, b_eps, b_iter);
  }
  prank = b_prank.get(0);
  auto rptr = prank.data();
  // null vertices should be ranked as INF
  for(size_t i = 0; i < num_nodes; ++i) {
    if (inptr[i] == 0 && outptr[i] == 0) rptr[i] = dmax;
  }
  return prank;
}

}
#endif
