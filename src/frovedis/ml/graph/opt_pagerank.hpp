#ifndef _OPT_PAGERANK_HPP
#define _OPT_PAGERANK_HPP

#define TIME_RECORD_LOG_LEVEL DEBUG

#include <frovedis/core/utility.hpp>
#include <frovedis/matrix/jds_crs_hybrid.hpp>
#include <frovedis/matrix/jds_matrix.hpp>
#include <frovedis/matrix/crs_matrix.hpp>
#include "graph_common.hpp"

namespace frovedis {

template<class T, class I, class O>
std::vector<double> 
calc_norm_wt(crs_matrix_local<T,I,O>& mat){
  std::vector<double> normalized_weight(mat.off.size() - 1, 0);
  auto moffp = mat.off.data();
  auto nwp = normalized_weight.data();
  auto mat_off_size = mat.off.size();
  for (size_t i = 0; i < mat_off_size - 1; ++i){
    if (moffp[i+1] != moffp[i]) {
      nwp[i] = static_cast<double>(1.0) / (moffp[i+1] - moffp[i]);
    } else {
      nwp[i] = 0;
    }
  }
  return normalized_weight;
}

template<class T, class I, class O>
void update_crs_mat(crs_matrix_local<T,I,O>& mat, 
                    std::vector<double>& normalized_weight){
  auto midxp = mat.idx.data();
  auto mvalp = mat.val.data();
  auto nwp = normalized_weight.data();
  auto mat_val_size = mat.val.size();
  for (size_t i = 0; i < mat_val_size; ++i){
    mvalp[i] = nwp[midxp[i]];
  }
}

template<class T, class I, class O>
crs_matrix<double,I,O> 
prep_crs_pagerank(crs_matrix<T,I,O>& mat){
  auto dmat = mat.template change_datatype<double,I,O>();
  auto ret = dmat.data.map(calc_norm_wt<double,I,O>)
                 .template moveto_dvector<double>().gather();
  dmat.data.mapv(update_crs_mat<double,I,O>, broadcast(ret));
  return dmat;
}

template<class T, class MATRIX>
std::vector<T>
PRmat(MATRIX& m, std::vector<T>& prank, 
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

template <class T, class MATRIX>
void pagerank_v1_helper(MATRIX& A,
                        std::vector<T>& prank,
                        double bias, double dfactor, double epsilon,
                        size_t iter_max) {
  double diff = epsilon + 1;
  size_t iter_index = 0;

  time_spent spmv_t(TIME_RECORD_LOG_LEVEL);
  time_spent comm(TIME_RECORD_LOG_LEVEL);
  time_spent other(TIME_RECORD_LOG_LEVEL);

  RLOG(TIME_RECORD_LOG_LEVEL) << "iter_idx\t" << "diff(abs)" << std::endl;
  while(diff > epsilon && iter_index < iter_max){
    iter_index ++;

    spmv_t.lap_start();
    auto res = PRmat<T, MATRIX>(A, prank, bias, dfactor);
    spmv_t.lap_stop();

    comm.lap_start();
    auto prank_new = do_allgather(res);
    comm.lap_stop();
        
    other.lap_start();        
    diff = cal_abs_diff_vec<T>(prank_new, prank);
    prank.swap(prank_new);
    other.lap_stop();  
  }
  spmv_t.show_lap("spmv_t: ");
  comm.show_lap("comm: ");
  other.show_lap("other: ");
}

template<class T, class I, class O>
std::vector<double>
pagerank_v1(crs_matrix<T,I,O>& A, 
            double dfactor, 
            double epsilon, 
            size_t iter_max,
            crs_matrix<double,I,O>& norm_mat, 
            MatType matformat) { 
  time_spent t_all(TIME_RECORD_LOG_LEVEL);
  t_all.lap_start();
  norm_mat = prep_crs_pagerank<T>(A);
  size_t num_nodes = norm_mat.num_row;
  double bias = 1.0 / static_cast<double>(num_nodes);
  double prank_init = 1.0 / static_cast<double>(num_nodes);
  std::vector<double> prank(num_nodes, prank_init);

  auto lb = frovedis::make_node_local_broadcast(bias);
  auto ldf = frovedis::make_node_local_broadcast(dfactor);
  auto leps = frovedis::make_node_local_broadcast(epsilon);
  auto liter_max = frovedis::make_node_local_broadcast(iter_max);
  auto lprank = frovedis::make_node_local_broadcast(prank);
  
  if(matformat == HYBRID) {
    auto A_hyb = jds_crs_hybrid<double>(norm_mat);       
    A_hyb.data.mapv(pagerank_v1_helper<double, jds_crs_hybrid_local<double,I,O>>, 
                    lprank, lb, 
                    ldf, leps, liter_max);
  } 
  else {
    norm_mat.data.mapv(pagerank_v1_helper<double, crs_matrix_local<double,I,O>>, 
                       lprank, lb, 
                       ldf, leps, liter_max);
  }
  t_all.lap_stop();
  t_all.show_lap("all: ");
  return lprank.get(0);
}

}
#endif
