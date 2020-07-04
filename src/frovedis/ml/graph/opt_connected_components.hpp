#ifndef _OPT_CC_
#define _OPT_CC_

#include <frovedis/core/radix_sort.hpp>
#include <frovedis/core/set_operations.hpp>
#include <frovedis/matrix/crs_matrix.hpp>
#include <frovedis/matrix/ccs_matrix.hpp>
#include "graph_common.hpp"

#define HYB_THRESHOLD 1e8 

namespace frovedis {

template <class I>
struct cc_result {
  cc_result() {}
  cc_result(size_t ncc,
            std::vector<I>& nn_each_cc,
            std::vector<I>& which_cc,
            std::vector<I>& dist) {
    num_cc = ncc;
    num_nodes_in_each_cc.resize(num_cc); 
    auto nn_eachp =  num_nodes_in_each_cc.data();
    auto nn_each_extrap = nn_each_cc.data();
    for(size_t i = 0; i < num_cc; ++i) nn_eachp[i] = nn_each_extrap[i];
    num_nodes_in_which_cc = which_cc;
    dist_nodes = dist;
  }
  cc_result(size_t ncc,
            std::vector<I>&& nn_each_cc,
            std::vector<I>&& which_cc,
            std::vector<I>&& dist) {
    num_cc = ncc;
    num_nodes_in_each_cc.resize(num_cc); 
    auto nn_eachp =  num_nodes_in_each_cc.data();
    auto nn_each_extrap = nn_each_cc.data();
    for(size_t i = 0; i < num_cc; ++i) nn_eachp[i] = nn_each_extrap[i];
    num_nodes_in_which_cc.swap(which_cc);
    dist_nodes.swap(dist);
  }
  void debug_print(size_t n = 0) {
    std::cout << "num_cc: " << num_cc << std::endl;
    std::cout << "num_nodes_in_each_cc: "; debug_print_vector(num_nodes_in_each_cc, n);
    std::cout << "num_nodes_in_which_cc: "; debug_print_vector(num_nodes_in_which_cc, n);
    std::cout << "dist_nodes: "; debug_print_vector(dist_nodes, n);
  }
  void save(const std::string& path) {
    make_dvector_scatter(num_nodes_in_each_cc).saveline(path + "_num_nodes_in_each_cc");
    make_dvector_scatter(num_nodes_in_which_cc).saveline(path + "_num_nodes_in_which_cc");
    make_dvector_scatter(dist_nodes).saveline(path + "_dist_nodes");
  }
  size_t num_cc;
  std::vector<I> num_nodes_in_each_cc, num_nodes_in_which_cc, dist_nodes;
  SERIALIZE(num_cc, num_nodes_in_each_cc, num_nodes_in_which_cc, dist_nodes)
};

template <class I>
void update_record(std::vector<int>& visited, 
                   std::vector<I>& nodes_cur,
                   std::vector<I>& nodes_in_which_cc, I rootid,
                   std::vector<I>& nodes_dist, I curlevel) {
  auto curp = nodes_cur.data();
  auto visitedp = visited.data();
  auto which_ccp = nodes_in_which_cc.data();
  auto distp = nodes_dist.data();
  for(size_t i = 0; i < nodes_cur.size(); ++i) {
    auto curidx = curp[i];
    visitedp[curidx] = 1;
    which_ccp[curidx] = rootid;
    distp[curidx] = curlevel;
  }
}

// TODO: Add version for crs_matrix_local<T,I,O>
template <class T, class I, class O>
std::vector<I>
cc_scatter(ccs_matrix_local<T,I,O>& mat, 
           std::vector<I>& cur_frontier, 
           std::vector<int>& visited, 
           size_t myst)  {
  auto myrank = get_selfid();
  auto nrow = mat.local_num_row;
  auto midxp = mat.idx.data();
  auto moffp = mat.off.data();
  auto valp = cur_frontier.data();
  auto visitedp = visited.data();
  std::vector<int> tmp(nrow, 0); auto tptr = tmp.data();

  time_spent spmv_t(TRACE), extract_t(TRACE);
  spmv_t.lap_start();
  for(size_t i = 0; i < cur_frontier.size(); ++i) {
#pragma _NEC ivdep
    for(size_t j = moffp[valp[i]]; j < moffp[valp[i] + 1]; ++j) {
      tptr[midxp[j]] = 1 - visitedp[midxp[j] + myst];
    }
  }
  spmv_t.lap_stop();

  extract_t.lap_start();
  I count = 0;
  auto tptr2 = new I[nrow]; // new'ing to avoid filled with zeros in constructor of std::vector
  for(size_t i = 0; i < nrow; ++i) {
    if(tptr[i] != 0) tptr2[count++] = i + myst;
  }
  std::vector<I> ret(count); auto retp = ret.data();
  for(size_t i = 0; i < count; ++i) retp[i] = tptr2[i];
  delete tptr2;
  extract_t.lap_stop();
  if(myrank == 0) {
    spmv_t.show_lap("spmv time: ");
    extract_t.show_lap("frontier extract time: ");
  }
  return ret;
}

template <class T, class I, class O>
std::vector<I>
store_neighbor_indices(ccs_matrix_local<T,I,O>& mat, 
                       std::vector<I>& cur_frontier, 
                       std::vector<int>& visited, 
                       size_t myst) {
  auto midxp = mat.idx.data();
  auto moffp = mat.off.data();
  auto valp = cur_frontier.data();
  auto visitedp = visited.data();

  I tot_targets = 0;
  for(size_t i = 0; i < cur_frontier.size(); ++i) {
    tot_targets += moffp[valp[i] + 1] -  moffp[valp[i]];
  }
  std::vector<I> tmp(tot_targets); auto tptr = tmp.data();

  size_t k = 0;
  for(size_t i = 0; i < cur_frontier.size(); ++i) {
    for(size_t j = moffp[valp[i]]; j < moffp[valp[i] + 1]; ++j) {
      auto idx = midxp[j] + myst; // global rowid
      tptr[k++] = !visitedp[idx] * idx; // collecting indices (ignore 0th)
    }
  }
  return tmp;
}

template <class T, class I, class O>
std::vector<I>
store_neighbor_indices(crs_matrix_local<T,I,O>& mat,
                       std::vector<I>& cur_frontier,
                       std::vector<int>& visited,
                       size_t myst) {
  auto nrow = mat.local_num_row;
  auto myend = myst + nrow - 1;
  auto midxp = mat.idx.data();
  auto moffp = mat.off.data();
  auto valp = cur_frontier.data();
  auto visitedp = visited.data();

  I tot_targets = 0;
  for(size_t i = 0; i < cur_frontier.size(); ++i) {
    auto nodeid = valp[i];
    // for CRS: all frontier nodes are not available at all process
    if (nodeid >= myst && nodeid <= myend) { 
      tot_targets += moffp[nodeid - myst + 1] -  moffp[nodeid - myst];
    }
  }
  std::vector<I> tmp(tot_targets); auto tptr = tmp.data();
  // this if check will improve performance, 
  // by avoiding unnecessary parsing of cur_frontier vector
  if (tot_targets > 0) { 
    size_t k = 0;
    for(size_t i = 0; i < cur_frontier.size(); ++i) {
      auto nodeid = valp[i];
      // for CRS: all frontier nodes are not available at all process
      if (nodeid >= myst && nodeid <= myend) { 
        for(size_t j = moffp[nodeid - myst]; j < moffp[nodeid - myst + 1]; ++j) {
          auto idx = midxp[j];
          tptr[k++] = !visitedp[idx] * idx; // collecting indices (ignore 0th)
        }
      }
    }
  }
  return tmp;
}

template <class I>
std::vector<I> 
get_unique_indices(std::vector<I>& vec) {
  if (vec.size() == 0) return vec; // quick return
  std::vector<I> unq_idx_neat;
  radix_sort(vec, true);
  auto unq_idx = set_separate(vec); 
  auto unq_idxp = unq_idx.data();
  auto vptr = vec.data();
  auto unqCount = unq_idx.size();
  if (vptr[unq_idxp[0]] == 0) {
    unq_idx_neat.resize(unqCount - 2);
    auto unq_idx_neatp = unq_idx_neat.data();
    for(size_t i = 0; i < unqCount - 2; i++) {
      unq_idx_neatp[i] = vptr[unq_idxp[i + 1]]; // ignoring 0th
    }
  }
  else {
    unq_idx_neat.resize(unqCount - 1);
    auto unq_idx_neatp = unq_idx_neat.data();
    for(size_t i = 0; i < unqCount - 1; i++) {
      unq_idx_neatp[i] = vptr[unq_idxp[i]];
    }
  }
  return unq_idx_neat;
}

template <class I, class MATRIX>
std::vector<I>
cc_idxsort(MATRIX& mat, 
           std::vector<I>& cur_frontier, 
           std::vector<int>& visited, 
           size_t myst)  {
  auto myrank = get_selfid();

  time_spent spmv_t(TRACE), extract_t(TRACE);
  spmv_t.lap_start();
  auto tmp = store_neighbor_indices(mat, cur_frontier, visited, myst);
  spmv_t.lap_stop();
  //std::cout << "[rank " << myrank << "] tmp: "; debug_print_vector(tmp, 10);

  extract_t.lap_start();
  auto unq_idx_neat = get_unique_indices(tmp);
  extract_t.lap_stop();
  //std::cout << "[rank " << myrank << "] unq_idx_neat: "; debug_print_vector(unq_idx_neat, 10);
  if(myrank == 0) {
    spmv_t.show_lap("spmv time: ");
    extract_t.show_lap("frontier extract time: ");
  }
  return unq_idx_neat;
}

template <class I, class MATRIX>
std::vector<I> 
get_next_frontier(MATRIX& mat, 
                  std::vector<I>& cur_frontier,
                  std::vector<int>& visited,
                  size_t myst) {
  /*
   * TODO: add cc_scatter for MATRIX type: crs_matrix_local<T,I,O>   
  if(cur_frontier.size() >= HYB_THRESHOLD) 
    return cc_scatter(mat, cur_frontier, visited, myst);
  else
  */
    return cc_idxsort<I, MATRIX>(mat, cur_frontier, visited, myst);
}

template <class I, class MATRIX>
cc_result<I> calc_cc(MATRIX& mat, 
                     bool is_direct, 
                     size_t myst) {
  auto myrank = get_selfid();
  auto nvert = mat.local_num_col;
  std::vector<int> visited(nvert,0); auto visitedp = visited.data();
  // --- resultant values ---
  auto num_cc = 0;
  std::vector<I> nodes_in_which_cc(nvert), nodes_dist(nvert); 
  std::vector<I> num_nodes_in_each_cc(nvert,0); 
  auto nn_eachp = num_nodes_in_each_cc.data();
  // --- main loop ---
  time_spent comp_t(DEBUG), comm_t(DEBUG), update_t(DEBUG);
  time_spent trace_iter(TRACE);
  for(size_t i = 0; i < nvert; ++i) {
    if(!visitedp[i]) {
      auto srcid = i;
      I curlevel = 0;
      std::vector<I> nodes_next(1, srcid);
      std::vector<I> nodes_cur;
      while (nodes_next.size() > 0) {
        // updated records 
        update_t.lap_start();
        nodes_cur.swap(nodes_next); // avoids copying
        update_record(visited, nodes_cur, nodes_in_which_cc, srcid, 
                      nodes_dist, curlevel);
        nn_eachp[num_cc] += nodes_cur.size();
        curlevel++;
        update_t.lap_stop();

        // computes frontier list level-wise (bfs)
        comp_t.lap_start();
        auto nodes_next_loc = get_next_frontier<I, MATRIX>(mat, nodes_cur, 
                                                           visited, myst);
        comp_t.lap_stop();

        // communication to gather frontiers from all processes
        comm_t.lap_start();
        nodes_next = do_allgather(nodes_next_loc); //takes less time than gather() + bcast() at master
        comm_t.lap_stop();

        if (!is_direct) { // when MATRIX: crs_matrix_local<T,I,O>
          // gathered data might have duplicate indices
          comp_t.lap_start();
          nodes_next = get_unique_indices(nodes_next);
          comp_t.lap_stop();
        }

        trace_iter.show("one iter: ");
      }
      num_cc++; // one connected component is completely parsed at this level
    } 
  }
  if(myrank == 0) {
    comp_t.show_lap("calculation time: ");
    update_t.show_lap("record update time: ");
    comm_t.show_lap("communication time: ");
  }
  return cc_result<I>(num_cc, 
                      std::move(num_nodes_in_each_cc), 
                      std::move(nodes_in_which_cc), 
                      std::move(nodes_dist));
}

// if input graph is an undirected graph, 
// please specify is_direct = false for performance benefit
template <class T, class I, class O>
cc_result<I> cc(crs_matrix<T, I, O>& gr, 
                bool is_direct = true) {
  //std::cout << "input adjacency matrix (src nodes in column): \n";
  //gr.debug_print(10);
  auto nrows = gr.get_local_num_rows();
  std::vector<int> sidx(nrows.size()); sidx[0] = 0;
  for(size_t i = 1; i < nrows.size(); ++i) sidx[i] = sidx[i-1] + nrows[i-1];
  cc_result<I> res;
  if (is_direct) {
    time_spent ccs_t(DEBUG);
    ccs_t.lap_start();
    auto ccsmat = ccs_matrix<T,I,O>(gr);
    ccs_t.lap_stop();
    ccs_t.show_lap("ccs conv time: ");
    res = ccsmat.data.map(calc_cc<I, ccs_matrix_local<T,I,O>>, 
                          broadcast(is_direct),
                          make_node_local_scatter(sidx)) 
                     .get(0); // all process contains same result
  }
  else {
    res = gr.data.map(calc_cc<I, crs_matrix_local<T,I,O>>,
                      broadcast(is_direct),
                      make_node_local_scatter(sidx))
                 .get(0); // all process contains same result
  }
  return res;
}

}

#endif
