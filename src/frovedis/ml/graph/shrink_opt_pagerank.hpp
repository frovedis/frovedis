#ifndef _SHRINK_OPT_PAGERANK_HPP
#define _SHRINK_OPT_PAGERANK_HPP

#include <frovedis/core/utility.hpp>
#include <frovedis/core/set_operations.hpp>
#include <frovedis/matrix/jds_crs_hybrid.hpp>
#include "graph_common.hpp"

//#define PR_DEBUG

namespace frovedis {
namespace shrink {

template<class T, class I, class O>
crs_matrix_local<double,I,O> 
shrink_and_normalize(crs_matrix_local<T,I,O>& mat,
                     std::vector<double>& weight,
                     std::vector<I>& shrink_rows,
                     size_t& local_nrow,
                     size_t myst) {
  // step1: normalize edge values
  auto nz = mat.val.size();
  std::vector<double> rval(nz);
  std::vector<I> ridx(nz);
  auto rvalp = rval.data();
  auto ridxp = ridx.data();
  auto midxp = mat.idx.data();
  auto wptr = weight.data();
  for(size_t i = 0; i < nz; ++i) {
    ridxp[i] = midxp[i];
    rvalp[i] = wptr[midxp[i]]; // normalized here
  }

  // step2: find non-empty rows
  auto retoffidx = set_separate(mat.off);

  // step3: fill offset for shrinked rows
  local_nrow = retoffidx.size() - 2; // skipped: first-0 and last-size
  shrink_rows.resize(local_nrow); // to store ids for shrinked rows (output)
  std::vector<O> roff(local_nrow + 1);
  auto roffp = roff.data();
  auto moffp = mat.off.data();
  auto off_idxp = retoffidx.data();
  auto shr_rowsp = shrink_rows.data();
  roffp[0] = 0; 
  for(size_t i = 1; i < local_nrow + 1; ++i) {
    roffp[i] = moffp[off_idxp[i]];
    shr_rowsp[i - 1] = off_idxp[i] - 1 + myst;
  }
  
  // step4: construct resultant normalized-and-shrinked-matrix
  crs_matrix_local<double,I,O> ret;
  ret.local_num_row = local_nrow;
  ret.local_num_col = mat.local_num_col; // same as input matrix (shrinking only in rows);
  ret.val.swap(rval);
  ret.idx.swap(ridx);
  ret.off.swap(roff);
  return ret;
}

template<class T, class I, class O>
crs_matrix<double,I,O> 
prep_crs_pagerank(crs_matrix<T,I,O>& mat,
                  std::vector<size_t>& num_outgoing,
                  node_local<std::vector<I>>& shrink_rows_info) {
  auto nvert = num_outgoing.size();
  std::vector<double> weight(nvert);
  auto noutptr = num_outgoing.data();
  auto wptr = weight.data();
  for(size_t i = 0; i < nvert; ++i) {
    wptr[i] = noutptr[i] > 0 ? (1.0 / noutptr[i]) : 0.0;
  }
  auto nrows = mat.get_local_num_rows();
  std::vector<size_t> sidx(nrows.size()); sidx[0] = 0;
  for(size_t i = 1; i < nrows.size(); ++i) sidx[i] = sidx[i-1] + nrows[i-1];
  auto myst = make_node_local_scatter(sidx);
  auto local_nrow = make_node_local_allocate<size_t>();
  crs_matrix<double,I,O> ret (mat.data.map(shrink_and_normalize<T,I,O>, 
                                           broadcast(weight),
                                           shrink_rows_info,
                                           local_nrow, myst));
  ret.num_row = local_nrow.reduce(add<size_t>);
  ret.num_col = nvert;
  return ret;
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

template<class T, class I>
T cal_abs_diff_vec(std::vector<T>& v1, 
                   std::vector<T>& v2,
                   std::vector<I>& shrink_rows_info){
  T ret = 0;
  auto v1p = v1.data();
  auto v2p = v2.data();
  auto sptr = shrink_rows_info.data();
  size_t v_size = v1.size(); // shrink-size
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < v_size; ++i) {
    auto x = v1p[i];
    auto y = v2p[sptr[i]];
    ret += x > y ? (x - y) : (y - x);
    v2p[sptr[i]] = v1p[i]; // update as well
  }
  return ret;
}

template <class I, class MATRIX>
void pagerank_v1_helper(MATRIX& A,
                        std::vector<double>& prank,
                        std::vector<I>& shrink_rows_info,
                        double bias, double dfactor, 
                        double epsilon,
                        size_t max_iter) {
  auto nproc = get_nodesize();
  auto myrank = get_selfid();
  time_spent spmv_t(DEBUG), comm_t(DEBUG), other_t(DEBUG);

  for(size_t i = 1; i <= max_iter; ++i) {
    spmv_t.lap_start();
    // frovedis::PRmat and frovedis::shrink::PRmat are exactly same
    // thus invoking with shrink namespace just to avoid ambiguity during compilation
    auto prank_new = shrink::PRmat<MATRIX>(A, prank, bias, dfactor);
    spmv_t.lap_stop();

    comm_t.lap_start();
    if(nproc > 1) prank_new = do_allgather(prank_new);
    comm_t.lap_stop();
        
    other_t.lap_start();        
    //std::cout << "old rank: "; debug_print_vector(prank, 10);
    auto diff = cal_abs_diff_vec(prank_new, prank, shrink_rows_info); // update prank in-place
    //std::cout << "new rank: "; debug_print_vector(prank, 10);
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
  auto l_shrink_rows_info = make_node_local_allocate<std::vector<I>>();
  // TODO: perform normalize and shrink separately (spark expects normalized matrix)
  norm_mat = prep_crs_pagerank<T>(A, num_outgoing, l_shrink_rows_info);
  auto shrink_rows_info = l_shrink_rows_info.template moveto_dvector<size_t>()
                                            .gather();
  t.show("normalize and shrink input: ");

  auto num_nodes = A.num_row;
  auto inptr = num_incoming.data();
  auto outptr = num_outgoing.data();
  size_t num_active_nodes = 0;
  for(size_t i = 0; i < num_nodes; ++i) {
    num_active_nodes += (inptr[i] != 0 || outptr[i] != 0);
  }
  double bias = 1.0 / static_cast<double>(num_active_nodes);
  double dmax = std::numeric_limits<double>::max();
  std::vector<double> prank(num_nodes);
  auto rptr = prank.data();
  for(size_t i = 0; i < num_nodes; ++i) {
    if (inptr[i] == 0 && outptr[i] == 0) rptr[i] = dmax; // null vertex
    else if (inptr[i] == 0 && outptr[i] != 0) rptr[i] = dfactor * bias;
    else rptr[i] = bias;
  }
#ifdef PR_DEBUG
  std::cout << "incoming: "; debug_print_vector(num_incoming, 10);
  std::cout << "shrink_rows_info: "; debug_print_vector(shrink_rows_info, 10);
  std::cout << "initial rank: "; debug_print_vector(prank, 10);
  std::cout << "num_nodes: " << num_nodes << "\n";
  std::cout << "num_active_nodes: " << num_active_nodes << "\n";
#endif
  auto b_prank = broadcast(prank);
  auto b_shrink_rows_info = broadcast(shrink_rows_info);
  auto b_bias = broadcast(bias);
  auto b_df = broadcast(dfactor);
  auto b_eps = broadcast(epsilon);
  auto b_iter = broadcast(max_iter);
  t.show("initialization: ");
  
  if(matformat == HYBRID) {
    auto A_hyb = jds_crs_hybrid<double>(norm_mat);       
    t.show("hybrid conversion: ");
    A_hyb.data.mapv(pagerank_v1_helper<I, jds_crs_hybrid_local<double,I,O>>, 
                    b_prank, b_shrink_rows_info, b_bias, 
                    b_df, b_eps, b_iter);
  } 
  else {
    norm_mat.data.mapv(pagerank_v1_helper<I, crs_matrix_local<double,I,O>>, 
                       b_prank, b_shrink_rows_info, b_bias, 
                       b_df, b_eps, b_iter);
  }
  return b_prank.get(0);
}

}
}
#endif
