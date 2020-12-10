#ifndef OPT_SSSP_HPP
#define	OPT_SSSP_HPP

#include <frovedis/matrix/crs_matrix.hpp>
#include "graph_common.hpp"

namespace frovedis {

template <class T, class I>
struct sssp_result {
  sssp_result() {}
  sssp_result(std::vector<T>& dist, std::vector<I>& pred) {
    distances = dist;
    predecessors = pred;
  }
  sssp_result(std::vector<T>&& dist, std::vector<I>&& pred) {
    distances.swap(dist);
    predecessors.swap(pred);
  }
  void debug_print(size_t n = 0) {
    std::cout << "distancess: "; debug_print_vector(distances, n);
    std::cout << "predecessorss: "; debug_print_vector(predecessors, n);
  }
  void save(const std::string& path) {
    make_dvector_scatter(distances).saveline(path + "_distances");
    make_dvector_scatter(predecessors).saveline(path + "_predecessors");
  }
  // TODO: add query()
  std::vector<T> distances;
  std::vector<I> predecessors;
  SERIALIZE(distances, predecessors)
};

template <class T, class I, class O>
crs_matrix_local<T, I, O>
construct_min_crs_matrix(crs_matrix_local<T, I, O>& mat,
                         std::vector<I>& rowid,
                         std::vector<T>& dist,
                         std::vector<I>& target_dest_ids,
                         size_t myst) {
  auto nrow = mat.local_num_row;
  auto nedges = mat.val.size();
  auto mvalp = mat.val.data();
  auto sidp = mat.idx.data();
  auto didp = rowid.data();
  auto distp = dist.data();
  auto Tmax = std::numeric_limits<T>::max();
  std::vector<T> minval(nedges); auto minvalp = minval.data();
  std::vector<I> minidx(nedges); auto minidxp = minidx.data();
  std::vector<O> tmpvec(nrow);   auto tmpvecp = tmpvec.data();
  size_t count = 0;
  for(size_t i = 0; i < nedges; ++i) {
    auto src = sidp[i];
    auto dst = didp[i];
    if(distp[src] != Tmax && ((distp[src] + mvalp[i]) < distp[dst + myst])) {
      minvalp[count] = distp[src] + mvalp[i];
      minidxp[count] = src;
      count++;
      tmpvecp[dst] = count;
    }
  }

  size_t k = 0;
  auto targetp = target_dest_ids.data();
  for(size_t dst = 0; dst < nrow; ++dst) if (tmpvecp[dst] != 0) targetp[k++] = dst;

  std::vector<O> countvec(k + 1); auto cntvecp = countvec.data();
  for(size_t i = 0; i < k; ++i) cntvecp[i+1] = tmpvecp[targetp[i]];
  O max_freq = 0;
  for(size_t i = 0; i < k; ++i) {
    auto freq = cntvecp[i+1] - cntvecp[i];
    if (freq > max_freq) max_freq = freq;
  }

  crs_matrix_local<T,I,O> ret;
  ret.val.swap(minval); // extra-sized (skipping extraction step to avoid copy of short loop-length)
  ret.idx.swap(minidx); // extra-sized (skipping here as well)
  ret.off.swap(countvec);
  ret.local_num_row = k;
  ret.local_num_col = max_freq;
  return ret;
}

template <class T, class I, class O>
std::vector<T>
compute_nextdist(crs_matrix_local<T, I, O>& mat,
                 std::vector<T>& dist,
                 std::vector<I>& pred,
                 std::vector<I>& target_dest_ids,
                 size_t myst) {
  auto nrow = pred.size();
  auto max_freq = mat.local_num_col;
  auto target_dest_cnt = mat.local_num_row;
  auto tmpvalp = mat.val.data();
  auto tmpidxp = mat.idx.data();
  auto tmpoffp = mat.off.data();
  std::vector<T> nextdist(nrow);
  auto nextdistp = nextdist.data();
  auto distp = dist.data();
  auto predp = pred.data();
  auto targetp = target_dest_ids.data();

  for(size_t i = 0; i < nrow; ++i) nextdistp[i] = distp[i + myst]; // initializing nextdist
#pragma _NEC nointerchange
  for(size_t j = 0; j < max_freq; ++j) {
#pragma _NEC ivdep
    for(size_t i = 0; i < target_dest_cnt; ++i) {
      int freq = tmpoffp[i+1] - tmpoffp[i] - j;
      if (freq > 0) { // guarding to check if jth column exist for ith row
        auto target_dest = targetp[i]; // 'target_dest' would be different across i-loop, hence 'ivdep' is applied
        auto src = tmpidxp[tmpoffp[i] + j];
        auto t_dist = tmpvalp[tmpoffp[i] + j];
        if(t_dist < nextdistp[target_dest]) {
          nextdistp[target_dest] = t_dist;
          predp[target_dest] = src + 1; // 1-based predecessors id
        }
      }
    }
  }
  return nextdist;
}

template <class T, class I, class O>
std::vector<T>
calc_sssp_bf_impl(crs_matrix_local<T, I, O>& mat,
                  std::vector<I>& rowid,
                  std::vector<T>& dist,
                  std::vector<I>& pred,
                  size_t myst) {
  auto myrank = get_selfid();
  auto nrow = mat.local_num_row;
  std::vector<I> target_dest_ids(nrow);
  time_spent t_compute(TRACE), t_update(TRACE);

  t_compute.lap_start();
  auto tmp_crs_mat = construct_min_crs_matrix(mat, rowid, dist,
                     target_dest_ids, myst);
  t_compute.lap_stop();
  //std::cout << "[" << myrank << "] tmp_crs_mat: "; tmp_crs_mat.debug_print(10);

  t_update.lap_start();
  auto nextdist = compute_nextdist(tmp_crs_mat, dist, pred,
                  target_dest_ids, myst);
  t_update.lap_stop();
  //std::cout << "[" << myrank << "] nextdist: "; debug_print_vector(nextdist, 10);

  if(myrank == 0) {
    RLOG(TRACE) << "target_dest_cnt: " << tmp_crs_mat.local_num_row
                << "; max_freq: " << tmp_crs_mat.local_num_col << std::endl;
    t_compute.show_lap("  - bf compute time: ");
    t_update.show_lap("  - distance update time: ");
  }
  return nextdist;
}

template <class T, class I, class O>
std::vector<I> compute_rowid(crs_matrix_local<T,I,O>& mat) {
  auto nrow = mat.local_num_row;
  auto nelem = mat.val.size();
  std::vector<I> ret(nelem);
  auto offp = mat.off.data();
  auto retp = ret.data();
  for(size_t i = 0; i < nrow; ++i) {
    for(size_t j = offp[i]; j < offp[i+1]; ++j) retp[j] = i;
  }
  return ret;
}

template <class T, class I, class O>
int get_local_nrows_int(crs_matrix_local<T,I,O>& mat) {
  return static_cast<int>(mat.local_num_row);
}

template <class T, class I, class O>
int check_negative_cycle(crs_matrix_local<T, I, O>& mat,
                         std::vector<I>& rowid,
                         std::vector<T>& dist,
                         size_t myst) {
  auto nedges = mat.val.size();
  auto mvalp = mat.val.data();
  auto sidp = mat.idx.data();
  auto didp = rowid.data();
  auto distp = dist.data();
  auto Tmax = std::numeric_limits<T>::max();
  size_t i = 0;
  for(i = 0; i < nedges; ++i) {
    auto src = sidp[i];
    auto dst = didp[i];
    if(distp[src] != Tmax && ((distp[src] + mvalp[i]) < distp[dst + myst])) break;
  }
  return (i == nedges) ? 0 : 1;
}

template <class T, class I, class O>
std::vector<T>
calc_sssp_bf(crs_matrix_local<T, I, O>& mat,
             std::vector<I>& rowid,
             std::vector<T>& dist,
             std::vector<I>& pred,
             std::vector<int>& nrows_per_process,
             std::vector<int>& stidx_per_process) {
  auto nrow = mat.local_num_row;
  auto nvert = mat.local_num_col;
  auto myrank = get_selfid();
  auto myst = stidx_per_process[myrank];
  auto step = 1;
  std::vector<T> all_nextdist(nvert);
  auto countp = nrows_per_process.data();
  auto displp = stidx_per_process.data();
  pred.resize(nrow); auto predp = pred.data();
  for(size_t i = 0; i < nrow; ++i) predp[i] = i + myst + 1; // initializing predecessor (1-based)
  while (step <= nvert) {
    time_spent t_iter(DEBUG), t_com(DEBUG);
    t_iter.lap_start();
    auto mynextdist = calc_sssp_bf_impl(mat, rowid, dist, pred, myst);
    t_com.lap_start();
    typed_allgatherv<T>(mynextdist.data(), nrow,
                        all_nextdist.data(), countp, displp,
                        frovedis_comm_rpc); // all_nextdist = gather all mynextdist process-by-process
    t_com.lap_stop();
    if(myrank == 0) t_com.show_lap("Communication time: ");

    if (all_nextdist == dist) {
      if(myrank == 0) {
        RLOG(INFO) << "Converged in " + std::to_string(step) + " steps!\n";
        t_iter.lap_stop();
        t_iter.show_lap("One iteration: ");
      }
      break;
    }
    dist.swap(all_nextdist); // exchange all_nextdist with dist
    step++;
    t_iter.lap_stop();
    if(myrank == 0) t_iter.show_lap("One iteration: ");
  }
  int sum_neg = 0;
  int is_neg = check_negative_cycle(mat, rowid, dist, myst);
  typed_allreduce(&is_neg, &sum_neg, 1, MPI_SUM, frovedis_comm_rpc);
  require(sum_neg == 0, "sssp: negative cost cycle is detected!");
  return dist;
}

template <class T, class I, class O>
sssp_result<T,I> 
sssp_bf_impl(crs_matrix<T, I, O>& gr, 
             std::vector<size_t>& num_incoming,
             std::vector<size_t>& num_outgoing,
             size_t source_node) {
  //std::cout << "input adjacency matrix (src nodes in column): \n";
  //gr.debug_print(10);
  auto nvert = gr.num_col;
  require(source_node >= 1 && source_node <= nvert, 
    std::string("sssp: target source node should be within the range 1 to ") 
                + STR(nvert) +"!\n");
  auto srcid = source_node - 1;

  // --- initialization step ---
  std::vector<I> pred;
  std::vector<T> dist(nvert); auto distp = dist.data(); // output vector
  auto Tmax = std::numeric_limits<T>::max();
  for(size_t i = 0; i < nvert; ++i) distp[i] = Tmax;
  dist[srcid] = 0;
  require(check_if_exist(srcid, num_outgoing, num_incoming),  
    std::string("sssp: source ") + STR(source_node) + 
    std::string(" not found in input graph!\n"));
  if(num_outgoing[srcid] == 0) {
    // no outgoing edge: thus no destination is reachable from given source
    pred.resize(nvert);
    auto predp = pred.data();
    for(size_t i = 0; i < nvert; ++i) predp[i] = i + 1;
  }
  else {
    // --- initialization related to distributed design ---
    auto rowid = gr.data.map(compute_rowid<T,I,O>);
    auto nrows = gr.data.map(get_local_nrows_int<T,I,O>).gather();
    std::vector<int> sidx(nrows.size()); sidx[0] = 0;
    for(size_t i = 1; i < nrows.size(); ++i) sidx[i] = sidx[i-1] + nrows[i-1];
    auto pred_loc = make_node_local_allocate<std::vector<I>>();
    // --- distributed computation step ---
    dist = gr.data.map(calc_sssp_bf<T,I,O>,
                       rowid, broadcast(dist), pred_loc,
                       broadcast(nrows), broadcast(sidx)).get(0); // all processes have updated distance
    pred = pred_loc.template moveto_dvector<I>().gather();
  }
  return sssp_result<T,I>(dist, pred);
}

}
#endif
