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
struct bfs_result {
  bfs_result() {}
  bfs_result(std::vector<I>& pred_list,
             std::vector<I>& dist) {
    predecessors = pred_list; 
    distances = dist;
  }
  bfs_result(std::vector<I>&& pred_list,
             std::vector<I>&& dist) {
    predecessors.swap(pred_list);
    distances.swap(dist);
  }
  void debug_print(size_t n = 0) {
    std::cout << "predecessors: "; debug_print_vector(predecessors, n);
    std::cout << "distances: "; debug_print_vector(distances, n);
  }
  void save(const std::string& path) {
    make_dvector_scatter(predecessors).saveline(path + "_predecessors");
    make_dvector_scatter(distances).saveline(path + "_distances");
  }
  std::vector<I> predecessors, distances;
  SERIALIZE(predecessors, distances)
};

template <class I>
struct cc_result {
  cc_result() {}
  cc_result(size_t ncc,
            std::vector<I>& root_each_cc,
            std::vector<I>& nn_each_cc,
            std::vector<I>& which_cc,
            std::vector<I>& pred_list,
            std::vector<I>& dist) {
    num_cc = ncc;
    num_nodes_in_each_cc.resize(num_cc); 
    auto nn_eachp =  num_nodes_in_each_cc.data();
    auto nn_each_extrap = nn_each_cc.data();
    root_in_each_cc.resize(num_cc); 
    auto root_eachp =  root_in_each_cc.data();
    auto root_each_extrap = root_each_cc.data();
    for(size_t i = 0; i < num_cc; ++i) {
      nn_eachp[i] = nn_each_extrap[i];
      root_eachp[i] = root_each_extrap[i];
    }
    num_nodes_in_which_cc = which_cc;
    distances = dist;
    predecessors = pred_list;
  }
  cc_result(size_t ncc,
            std::vector<I>&& root_each_cc,
            std::vector<I>&& nn_each_cc,
            std::vector<I>&& which_cc,
            std::vector<I>&& pred_list,
            std::vector<I>&& dist) {
    num_cc = ncc;
    num_nodes_in_each_cc.resize(num_cc); 
    auto nn_eachp =  num_nodes_in_each_cc.data();
    auto nn_each_extrap = nn_each_cc.data();
    root_in_each_cc.resize(num_cc); 
    auto root_eachp =  root_in_each_cc.data();
    auto root_each_extrap = root_each_cc.data();
    for(size_t i = 0; i < num_cc; ++i) {
      nn_eachp[i] = nn_each_extrap[i];
      root_eachp[i] = root_each_extrap[i];
    }
    num_nodes_in_which_cc.swap(which_cc);
    distances.swap(dist);
    predecessors.swap(pred_list);
  }
  void debug_print(size_t n = 0) {
    std::cout << "num_cc: " << num_cc << std::endl;
    std::cout << "root_in_each_cc: ";
    debug_print_vector(root_in_each_cc, n);
    std::cout << "num_nodes_in_each_cc: ";
    debug_print_vector(num_nodes_in_each_cc, n);
    std::cout << "num_nodes_in_which_cc: ";
    debug_print_vector(num_nodes_in_which_cc, n);
    std::cout << "predecessors: "; debug_print_vector(predecessors, n);
    std::cout << "distances: "; debug_print_vector(distances, n);
  }
  void save(const std::string& path) {
    make_dvector_scatter(root_in_each_cc) \
      .saveline(path + "_root_in_each_cc");
    make_dvector_scatter(num_nodes_in_each_cc) \
      .saveline(path + "_num_nodes_in_each_cc");
    make_dvector_scatter(num_nodes_in_which_cc) \
      .saveline(path + "_num_nodes_in_which_cc");
    make_dvector_scatter(predecessors).saveline(path + "_predecessors");
    make_dvector_scatter(distances).saveline(path + "_distances");
  }
  size_t num_cc;
  std::vector<I> root_in_each_cc, num_nodes_in_each_cc, num_nodes_in_which_cc,
                 predecessors, distances;
  SERIALIZE(num_cc, root_in_each_cc, num_nodes_in_each_cc, 
            num_nodes_in_which_cc,
            predecessors, distances)
};

template <class I>
void update_record_bfs(std::vector<int>& visited, 
                       std::vector<I>& nodes_dist, I curlevel, 
                       std::vector<I>& pred_list,
                       std::vector<I>& nodes_cur, // 1-based
                       std::vector<I>& pred_cur) { //1-based
  auto visitedp = visited.data();
  auto distp = nodes_dist.data();
  auto predp = pred_list.data();
  auto curp = nodes_cur.data();
  auto pred_curp = pred_cur.data();

  for(size_t i = 0; i < nodes_cur.size(); ++i) {
    auto curidx = curp[i] - 1; //node_cur is 1-based
    visitedp[curidx] = 1;
    distp[curidx] = curlevel;
    predp[curidx] = pred_curp[i];
  }
}

template <class I>
void update_record_cc(std::vector<int>& visited, 
                      std::vector<I>& nodes_in_which_cc, I rootid,
                      std::vector<I>& nodes_dist, 
                      std::vector<I>& pred_list, 
                      I curlevel,
                      std::vector<I>& nodes_cur, //1-based
                      std::vector<I>& pred_cur) {//1-based
  auto visitedp = visited.data();
  auto which_ccp = nodes_in_which_cc.data();
  auto distp = nodes_dist.data();
  auto predp = pred_list.data();
  auto curp = nodes_cur.data();
  auto pred_curp = pred_cur.data();

  for(size_t i = 0; i < nodes_cur.size(); ++i) {
    auto curidx = curp[i] - 1; //node_cur is 1-based
    visitedp[curidx] = 1;
    which_ccp[curidx] = rootid;
    distp[curidx] = curlevel;
    predp[curidx] = pred_curp[i];
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
  auto tptr2 = new I[nrow];//avoiding constructor with zeros of std::vector
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
store_neighbor_indices_0(ccs_matrix_local<T,I,O>& mat, 
                         std::vector<I>& cur_frontier, //assuming it to be 1-based
                         std::vector<int>& visited, 
                         size_t myst,
                         std::vector<I>& pred_tmp) {
  auto midxp = mat.idx.data();
  auto moffp = mat.off.data();
  auto valp = cur_frontier.data();
  auto visitedp = visited.data();

  I tot_targets = 0;
  for(size_t i = 0; i < cur_frontier.size(); ++i) {
    auto nodeid = valp[i] - 1; //cur_frontier is 1-based
    tot_targets += moffp[nodeid + 1] -  moffp[nodeid];
  }

  std::vector<I> ret;
  if (tot_targets == 0)  return ret;

  size_t k = 0;
  std::vector<I> tmp_pred(tot_targets); auto tpptr = tmp_pred.data();
  std::vector<I> tmp(tot_targets); auto tptr = tmp.data();
  for (size_t i = 0; i < cur_frontier.size(); ++i) {
  auto nodeid = valp[i] - 1; //cur_frontier is 1-based
    for(size_t j = moffp[nodeid]; j < moffp[nodeid + 1]; ++j) {
      auto idx = midxp[j] + myst; // global rowid
      if (!visitedp[idx]) {
        tptr[k] = idx + 1;
        tpptr[k++] = nodeid + 1;
      }
    }
  }
  ret.resize(k);      auto rptr = ret.data();
  pred_tmp.resize(k); auto predp = pred_tmp.data();
  for (size_t i = 0; i < k; ++i) {
    rptr[i] = tptr[i];
    predp[i] = tpptr[i];
  }
  return ret;
}

template <class T, class I, class O>
std::vector<I>
store_neighbor_indices_1(ccs_matrix_local<T,I,O>& mat, 
                         std::vector<I>& cur_frontier, //assuming it to be 1-based
                         std::vector<int>& visited, 
                         size_t myst,
                         std::vector<I>& pred_tmp) {
  auto midxp = mat.idx.data();
  auto moffp = mat.off.data();
  auto valp = cur_frontier.data();
  auto visitedp = visited.data();

  I tot_targets = 0;
  for(size_t i = 0; i < cur_frontier.size(); ++i) {
    auto nodeid = valp[i] - 1; //cur_frontier is 1-based
    tot_targets += moffp[nodeid + 1] -  moffp[nodeid];
  }

  std::vector<I> ret;
  if (tot_targets == 0)  return ret;

  size_t k = 0;
  ret.resize(tot_targets);      auto tptr = ret.data();
  pred_tmp.resize(tot_targets); auto tpptr = pred_tmp.data();
  for(size_t i = 0; i < cur_frontier.size(); ++i) {
    auto nodeid = valp[i] - 1; //cur_frontier is 1-based
    for(size_t j = moffp[nodeid]; j < moffp[nodeid + 1]; ++j) {
      auto idx = midxp[j] + myst; // global rowid
      tptr[k] = !visitedp[idx] * (idx + 1);
      tpptr[k++] = !visitedp[idx] * (nodeid + 1);
    }
  }
  return ret;
}

template <class T, class I, class O>
std::vector<I>
store_neighbor_indices_2(ccs_matrix_local<T,I,O>& mat, 
                         std::vector<I>& cur_frontier, //assuming it to be 1-based
                         std::vector<int>& visited, 
                         size_t myst,
                         std::vector<I>& pred_tmp,
                         double threshold) {
  auto midxp = mat.idx.data();
  auto moffp = mat.off.data();
  auto valp = cur_frontier.data();
  auto visitedp = visited.data();

  I tot_targets = 0;
  for(size_t i = 0; i < cur_frontier.size(); ++i) {
    auto nodeid = valp[i] - 1; //cur_frontier is 1-based
    tot_targets += moffp[nodeid + 1] -  moffp[nodeid];
  }

  std::vector<I> ret;
  if (tot_targets == 0)  return ret;

  size_t k = 0;
  size_t visited_count = 0;
  ret.resize(tot_targets);      auto tptr = ret.data();
  pred_tmp.resize(tot_targets); auto tpptr = pred_tmp.data();
  for(size_t i = 0; i < cur_frontier.size(); ++i) {
    auto nodeid = valp[i] - 1; //cur_frontier is 1-based
    for(size_t j = moffp[nodeid]; j < moffp[nodeid + 1]; ++j) {
      auto idx = midxp[j] + myst; // global rowid
      visited_count += visitedp[idx];
      tptr[k] = !visitedp[idx] * (idx + 1);
      tpptr[k++] = !visitedp[idx] * (nodeid + 1);
    }
  }

  auto ntargets = tot_targets - visited_count;
  // simple heuristic to control sorting overhead in later steps, 
  // based on given threshold
  if (ntargets < tot_targets * threshold) {
    k = 0;
    std::vector<I> nzero_ret(ntargets); auto tptr2 = nzero_ret.data();
    std::vector<I> tmp_pred(ntargets); auto tpptr2 = tmp_pred.data();
    for (size_t i = 0; i < tot_targets; ++i) {
      if (tptr[i]) {
         tptr2[k] = tptr[i];
         tpptr2[k++] = tpptr[i];
      }
    }
    ret.swap(nzero_ret);
    pred_tmp.swap(tmp_pred);
  }
  return ret;
}

template <class T, class I, class O>
std::vector<I>
store_neighbor_indices_0(crs_matrix_local<T,I,O>& mat,
                         std::vector<I>& cur_frontier, //assuming it to be 1-based
                         std::vector<int>& visited,
                         size_t myst,
                         std::vector<I>& pred_tmp) {
  auto nrow = mat.local_num_row;
  auto myend = myst + nrow - 1;
  auto midxp = mat.idx.data();
  auto moffp = mat.off.data();
  auto valp = cur_frontier.data();
  auto visitedp = visited.data();

  I tot_targets = 0;
  for(size_t i = 0; i < cur_frontier.size(); ++i) {
    auto nodeid = valp[i] - 1; //cur_frontier is 1-based
    // for CRS: all frontier nodes are not available at all process
    if (nodeid >= myst && nodeid <= myend) { 
      tot_targets += moffp[nodeid - myst + 1] -  moffp[nodeid - myst];
    }
  }

  std::vector<I> ret;
  if (tot_targets == 0)  return ret;

  size_t k = 0;
  std::vector<I> tmp(tot_targets); auto tptr = tmp.data();
  std::vector<I> tmp_pred(tot_targets); auto tpptr = tmp_pred.data();
  for(size_t i = 0; i < cur_frontier.size(); ++i) {
    auto nodeid = valp[i] - 1; //cur_frontier is 1-based
    // for CRS: all frontier nodes are not available at all process
    if (nodeid >= myst && nodeid <= myend) { 
      for(size_t j = moffp[nodeid - myst]; j < moffp[nodeid - myst + 1]; ++j) {
        auto idx = midxp[j];
        if (!visitedp[idx]){
          tptr[k] = idx + 1;    
          tpptr[k++] = nodeid + 1;
        }
      }
    }
  }
  ret.resize(k); auto rptr = ret.data();
  pred_tmp.resize(k); auto predp = pred_tmp.data();
  for (size_t i = 0; i < k; ++i){
    rptr[i] = tptr[i];
    predp[i] = tpptr[i];
  }
  return ret;
}

template <class T, class I, class O>
std::vector<I>
store_neighbor_indices_1(crs_matrix_local<T,I,O>& mat,
                         std::vector<I>& cur_frontier, //assuming it to be 1-based
                         std::vector<int>& visited,
                         size_t myst,
                         std::vector<I>& pred_tmp) {
  auto nrow = mat.local_num_row;
  auto myend = myst + nrow - 1;
  auto midxp = mat.idx.data();
  auto moffp = mat.off.data();
  auto valp = cur_frontier.data();
  auto visitedp = visited.data();

  I tot_targets = 0;
  for(size_t i = 0; i < cur_frontier.size(); ++i) {
    auto nodeid = valp[i] - 1; //cur_frontier is 1-based
    // for CRS: all frontier nodes are not available at all process
    if (nodeid >= myst && nodeid <= myend) { 
      tot_targets += moffp[nodeid - myst + 1] -  moffp[nodeid - myst];
    }
  }

  std::vector<I> ret;
  if (tot_targets == 0)  return ret;

  size_t k = 0;
  ret.resize(tot_targets); auto tptr = ret.data();
  pred_tmp.resize(tot_targets); auto tpptr = pred_tmp.data();
  for(size_t i = 0; i < cur_frontier.size(); ++i) {
    auto nodeid = valp[i] - 1; //cur_frontier is 1-based
    // for CRS: all frontier nodes are not available at all process
    if (nodeid >= myst && nodeid <= myend) { 
      for(size_t j = moffp[nodeid - myst]; j < moffp[nodeid - myst + 1]; ++j) {
        auto idx = midxp[j];
        tptr[k] = !visitedp[idx] * (idx + 1);
        tpptr[k++] = !visitedp[idx] * (nodeid + 1);
      }
    }
  }
  return ret;
}


template <class T, class I, class O>
std::vector<I>
store_neighbor_indices_2(crs_matrix_local<T,I,O>& mat,
                         std::vector<I>& cur_frontier, //assuming it to be 1-based
                         std::vector<int>& visited,
                         size_t myst,
                         std::vector<I>& pred_tmp,
                         double threshold) {
  auto nrow = mat.local_num_row;
  auto myend = myst + nrow - 1;
  auto midxp = mat.idx.data();
  auto moffp = mat.off.data();
  auto valp = cur_frontier.data();
  auto visitedp = visited.data();

  I tot_targets = 0;
  for(size_t i = 0; i < cur_frontier.size(); ++i) {
    auto nodeid = valp[i] - 1; //cur_frontier is 1-based
    // for CRS: all frontier nodes are not available at all process
    if (nodeid >= myst && nodeid <= myend) { 
      tot_targets += moffp[nodeid - myst + 1] -  moffp[nodeid - myst];
    }
  }

  std::vector<I> ret;
  if (tot_targets == 0)  return ret;

  size_t k = 0;
  size_t visited_count = 0;
  ret.resize(tot_targets); auto tptr = ret.data();
  pred_tmp.resize(tot_targets); auto tpptr = pred_tmp.data();
  for(size_t i = 0; i < cur_frontier.size(); ++i) {
    auto nodeid = valp[i] - 1; //cur_frontier is 1-based
    // for CRS: all frontier nodes are not available at all process
    if (nodeid >= myst && nodeid <= myend) { 
      for(size_t j = moffp[nodeid - myst]; j < moffp[nodeid - myst + 1]; ++j) {
        auto idx = midxp[j];
        visited_count += visitedp[idx];
        tptr[k] = !visitedp[idx] * (idx + 1);
        tpptr[k++] = !visitedp[idx] * (nodeid + 1);
      }
    }
  }
  auto ntargets = tot_targets - visited_count;
  // simple heuristic to control sorting overhead in later steps, 
  // based on given threshold
  if (ntargets < tot_targets * threshold) {
    k = 0;
    std::vector<I> nzero_ret(ntargets); auto tptr2 = nzero_ret.data();
    std::vector<I> tmp_pred(ntargets); auto tpptr2 = tmp_pred.data();
    for (size_t i = 0; i < tot_targets; ++i) {
      if (tptr[i]) {
         tptr2[k] = tptr[i];
         tpptr2[k++] = tpptr[i];
      }
    }
    ret.swap(nzero_ret);
    pred_tmp.swap(tmp_pred);
  }
  return ret;
}

template <class I>
void get_unique_indices(std::vector<I>& front_tmp, 
                        std::vector<I>& pred_tmp, 
                        std::vector<I>& front_next,
                        std::vector<I>& pred_next) {
  if (front_tmp.size() == 0) return; // quick return
  radix_sort(front_tmp, pred_tmp, true);
  auto unq_idx = set_separate(front_tmp); 
  auto pred_tmpp = pred_tmp.data();
  auto front_tmpp = front_tmp.data();
  auto unqCount = unq_idx.size() - 1;
  auto unq_idxp = unq_idx.data();
  if (front_tmpp[unq_idxp[0]] == 0 ) {
    // ignoring zeros as they are already visited 
    // (applicable only for opt_level: 1)
    unqCount--; unq_idxp++;
  }
  front_next.resize(unqCount);
  pred_next.resize(unqCount);
  auto pred_nextp = pred_next.data();
  auto front_nextp = front_next.data();
  for(size_t i = 0; i < unqCount; i++) {
    front_nextp[i] = front_tmpp[unq_idxp[i]];
    pred_nextp[i] = pred_tmpp[unq_idxp[i]];
  }
}


template <class I, class MATRIX>
std::vector<I>
cc_idxsort(MATRIX& mat, 
           std::vector<I>& cur_frontier, 
           std::vector<int>& visited, 
           size_t myst,
           std::vector<I>& pred_next,
           int opt_level = 1,
           double threshold = 0.4) {
  auto myrank = get_selfid();
  std::vector<I> front_tmp, pred_tmp;

  time_spent spmv_t(TRACE), extract_t(TRACE);
  spmv_t.lap_start();
  switch(opt_level) {
    case 0: front_tmp = store_neighbor_indices_0(mat, cur_frontier, visited, 
                                                 myst, pred_tmp);
            break;
    case 1: front_tmp = store_neighbor_indices_1(mat, cur_frontier, visited, 
                                                 myst, pred_tmp);
            break;
    case 2: front_tmp = store_neighbor_indices_2(mat, cur_frontier, visited, 
                                                 myst, pred_tmp, threshold);
            break;
    default: REPORT_ERROR(USER_ERROR, 
             "Supported opt-level is either 0, 1 or 2!\n");
  }
  spmv_t.lap_stop();

  extract_t.lap_start();
  std::vector<I> front_next;
  get_unique_indices(front_tmp, pred_tmp, front_next, pred_next);
  extract_t.lap_stop();

  if(myrank == 0) {
    spmv_t.show_lap("neighbor store time: ");
    extract_t.show_lap("frontier extract time: ");
  }
  return front_next;
}

//Returns 1-based next frontiers(return value) and  
//1-based predicator(pred_next updated by reference)
template <class I, class MATRIX>
std::vector<I> 
get_next_frontier(MATRIX& mat, 
                  std::vector<I>& cur_frontier, //assumed to be 1-based
                  std::vector<int>& visited, 
                  size_t myst,
                  std::vector<I>& pred_next,
                  int opt_level = 1,
                  double threshold = 0.4) { // 40% seems to be a suitable candidate from experiment
  /*
   * TODO: add cc_scatter for MATRIX type: crs_matrix_local<T,I,O>   
  if(cur_frontier.size() >= HYB_THRESHOLD) 
    return cc_scatter(mat, cur_frontier, visited, myst);
  else
  */
    return cc_idxsort<I, MATRIX>(mat, cur_frontier, visited, 
                                 myst, pred_next, opt_level, threshold);
}

template <class I, class MATRIX>
bfs_result<I> calc_bfs(MATRIX& mat,
                       size_t srcid,
                       bool is_direct, 
                       size_t myst,
                       double threshold,
                       int opt_level = 1) {
  auto myrank = get_selfid();
  auto nvert = mat.local_num_col;
  std::vector<int> visited(nvert, 0);
  // --- resultant values ---
  std::vector<I> nodes_dist(nvert, std::numeric_limits<I>::max());
  std::vector<I> pred_list(nvert);
  auto pred_listp = pred_list.data();
  for (size_t i = 0; i < nvert; ++i) pred_listp[i] = i + 1;
  // --- main loop ---
  time_spent comp_t(DEBUG), comm_t(DEBUG), update_t(DEBUG);
  time_spent trace_iter(TRACE);
  I curlevel = 0;
  std::vector<I> nodes_next(1, srcid + 1);
  std::vector<I> nodes_cur;
  std::vector<I> pred_next(1, srcid + 1);
  std::vector<I> pred_cur;
  while (nodes_next.size() > 0) {
    // updated records 
    update_t.lap_start();
    nodes_cur.swap(nodes_next); // avoids copying
    pred_cur.swap(pred_next); // avoids copying
    //std::cout << "Curlevel = " << curlevel << std::endl;
    //std::cout << "Node Cur = \n"; debug_print_vector(nodes_cur);
    //std::cout << "Pred Cur = \n"; debug_print_vector(pred_cur);

    update_record_bfs(visited, nodes_dist, curlevel, pred_list, 
                      nodes_cur, pred_cur); 
    curlevel++;
    update_t.lap_stop();
    // computes frontier list level-wise (bfs)
    comp_t.lap_start();
    std::vector<I> pred_next_loc;
    auto nodes_next_loc = get_next_frontier<I, MATRIX>(mat, nodes_cur, 
                                                       visited, myst, 
                                                       pred_next_loc, 
                                                       opt_level,
                                                       threshold);
    comp_t.lap_stop();
    // communication to gather frontiers from all processes
    comm_t.lap_start();
    //--
    //takes less time than gather() + bcast() at master
    nodes_next = do_allgather(nodes_next_loc); 
    pred_next = do_allgather(pred_next_loc);
    //--
    comm_t.lap_stop();

    if (!is_direct) { // when MATRIX: crs_matrix_local<T,I,O>
      // gathered data might have duplicate indices
      comp_t.lap_start();
      std::vector<I> unq_nodes_next, unq_pred_next;
      get_unique_indices(nodes_next, pred_next, unq_nodes_next, unq_pred_next);
      nodes_next.swap(unq_nodes_next);
      pred_next.swap(unq_pred_next);
      comp_t.lap_stop();
    }

    trace_iter.show("one iter: ");
    //debug_print_vector(nodes_next);
  }
  //debug_print_vector(pred_list);
  //debug_print_vector(nodes_dist);
  if(myrank == 0) {
    comp_t.show_lap("calculation time: ");
    update_t.show_lap("record update time: ");
    comm_t.show_lap("communication time: ");
  }
  return bfs_result<I>(std::move(pred_list), 
                      std::move(nodes_dist));
}


template <class I, class MATRIX>
cc_result<I> 
calc_cc(MATRIX& mat, 
        std::vector<size_t>& num_incoming,
        std::vector<size_t>& num_outgoing,
        bool is_direct, 
        size_t myst,
        double threshold = 0.4,
        int opt_level = 1) {
  auto myrank = get_selfid();
  auto nvert = mat.local_num_col;
  std::vector<int> visited(nvert, 0); 
  auto visitedp = visited.data();
  // --- resultant values ---
  auto num_cc = 0;
  auto imax = std::numeric_limits<I>::max();
  std::vector<I> pred_list(nvert);
  std::vector<I> nodes_dist(nvert);
  std::vector<I> nodes_in_which_cc(nvert); 
  auto pred_listp = pred_list.data();
  auto distp = nodes_dist.data();
  auto whichp = nodes_in_which_cc.data();
  for (size_t i = 0; i < nvert; ++i) {
    pred_listp[i] = i + 1;
    distp[i] = imax;
    whichp[i] = imax; // non-existing node 
  }
  // root_in_each_cc, num_nodes_in_each_cc would be 
  // resized based on calculated num_cc 
  std::vector<I> num_nodes_in_each_cc(nvert); 
  std::vector<I> root_in_each_cc(nvert); 
  auto root_eachp = root_in_each_cc.data();
  auto nn_eachp = num_nodes_in_each_cc.data();
  // --- main loop ---
  time_spent comp_t(DEBUG), comm_t(DEBUG), update_t(DEBUG);
  time_spent trace_iter(TRACE);
  for(size_t i = 0; i < nvert; ++i) {
    if(check_if_exist(i, num_outgoing, num_incoming) && !visitedp[i]) {
      if (num_outgoing[i] == 0) continue;
      auto srcid = i;
      I curlevel = 0;
      std::vector<I> nodes_next(1, srcid + 1);
      std::vector<I> nodes_cur;
      std::vector<I> pred_next(1, srcid + 1);
      std::vector<I> pred_cur;
      while (nodes_next.size() > 0) {
        // updated records 
        update_t.lap_start();
        nodes_cur.swap(nodes_next); // avoids copying
        pred_cur.swap(pred_next); // avoids copying
        //std::cout << "Curlevel = " << curlevel << std::endl;
        //std::cout << "Node Cur = \n"; debug_print_vector(nodes_cur);
        //std::cout << "Pred Cur = \n"; debug_print_vector(pred_cur);
          
        update_record_cc(visited, nodes_in_which_cc, srcid + 1, 
                         nodes_dist, pred_list, curlevel, nodes_cur, pred_cur);
        nn_eachp[num_cc] += nodes_cur.size();
        curlevel++;
        update_t.lap_stop();

        // computes frontier list level-wise (bfs)
        comp_t.lap_start();
        std::vector<I> pred_next_loc;
        auto nodes_next_loc = \
          get_next_frontier<I, MATRIX>(mat, nodes_cur, 
                                       visited, myst, pred_next_loc, 
                                       opt_level, threshold);
        comp_t.lap_stop();
        // communication to gather frontiers from all processes
        comm_t.lap_start();
        //--
        //takes less time than gather() + bcast() at master
        nodes_next = do_allgather(nodes_next_loc); 
        pred_next = do_allgather(pred_next_loc);
        //--
        comm_t.lap_stop();

        if (!is_direct) { // when MATRIX: crs_matrix_local<T,I,O>
          // gathered data might have duplicate indices
          comp_t.lap_start();
          std::vector<I> unq_nodes_next, unq_pred_next;
          get_unique_indices(nodes_next, pred_next, unq_nodes_next, \
                             unq_pred_next);
          nodes_next.swap(unq_nodes_next);
          pred_next.swap(unq_pred_next);
          comp_t.lap_stop();
        }

        trace_iter.show("one iter: ");
      }
      root_eachp[num_cc] = srcid + 1;
      num_cc++; // one connected component is completely parsed at this level
    } 
  }
  if(myrank == 0) {
    comp_t.show_lap("calculation time: ");
    update_t.show_lap("record update time: ");
    comm_t.show_lap("communication time: ");
  }
  return cc_result<I>(num_cc, 
                      std::move(root_in_each_cc), 
                      std::move(num_nodes_in_each_cc), 
                      std::move(nodes_in_which_cc), 
                      std::move(pred_list), 
                      std::move(nodes_dist));
}

// if input graph is an undirected graph, 
// please specify is_direct = false for performance benefit
template <class T, class I, class O>
bfs_result<I> 
bfs_impl(crs_matrix<T, I, O>& gr,
         std::vector<size_t>& num_incoming,
         std::vector<size_t>& num_outgoing,
         size_t source_node,
         double threshold = 0.4,
         bool is_direct = true,
         int opt_level = 1) {
  require(threshold >= 0.0 and threshold <= 1.0, 
  "bfs: threshold should be within the range 0 to 1!\n");
  auto nvert = gr.num_col;
  require(source_node >= 1 && source_node <= nvert, 
    std::string("bfs: target source node should be within the range 1 to ") 
                + STR(nvert) +"!\n");
  auto srcid = source_node - 1;
  bfs_result<I> res;
  require(check_if_exist(srcid, num_outgoing, num_incoming),  
    std::string("bfs: source ") + STR(source_node) + 
    std::string(" not found in input graph!\n"));
  if(num_outgoing[srcid] == 0) {
    // no outgoing edge: thus no destination is reachable from given source
    std::vector<I> dist(nvert), pred(nvert);
    auto distp = dist.data();
    auto predp = pred.data();
    auto imax = std::numeric_limits<I>::max();
    for(size_t i = 0; i < nvert; ++i) {
      predp[i] = i + 1;
      distp[i] = imax;
    }
    distp[srcid] = 0;
    res = bfs_result<I>(std::move(pred), std::move(dist));
  }
  else {
    auto nrows = gr.get_local_num_rows();
    std::vector<int> sidx(nrows.size()); sidx[0] = 0;
    for(size_t i = 1; i < nrows.size(); ++i) sidx[i] = sidx[i-1] + nrows[i-1];
    if (is_direct) {
      time_spent ccs_t(DEBUG);
      ccs_t.lap_start();
      auto ccsmat = ccs_matrix<T,I,O>(gr);
      ccs_t.lap_stop();
      ccs_t.show_lap("ccs conv time: ");
      res = ccsmat.data.map(calc_bfs<I, ccs_matrix_local<T,I,O>>, 
                            broadcast(srcid),
                            broadcast(is_direct),
                            make_node_local_scatter(sidx),
                            broadcast(threshold),
                            broadcast(opt_level))
                       .get(0); // all process contains same result
    }
    else {
      res = gr.data.map(calc_bfs<I, crs_matrix_local<T,I,O>>,
                        broadcast(srcid),
                        broadcast(is_direct),
                        make_node_local_scatter(sidx),
                        broadcast(threshold),
                        broadcast(opt_level))
                   .get(0); // all process contains same result
    }
  }
  return res;
}

// if input graph is an undirected graph, 
// please specify is_direct = false for performance benefit
template <class T, class I, class O>
cc_result<I> 
cc_impl(crs_matrix<T, I, O>& gr, 
        std::vector<size_t>& num_incoming,
        std::vector<size_t>& num_outgoing,
        double threshold,
        bool is_direct = true,
        int opt_level = 2) {
  require(threshold >= 0.0 and threshold <= 1.0, 
  "connected_components: threshold should be within the range 0 to 1!\n");
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
                          broadcast(num_incoming),
                          broadcast(num_outgoing),
                          broadcast(is_direct),
                          make_node_local_scatter(sidx),
                          broadcast(threshold),
                          broadcast(opt_level))
                     .get(0); // all process contains same result
  }
  else {
    res = gr.data.map(calc_cc<I, crs_matrix_local<T,I,O>>,
                      broadcast(num_incoming),
                      broadcast(num_outgoing),
                      broadcast(is_direct),
                      make_node_local_scatter(sidx),
                      broadcast(threshold),
                      broadcast(opt_level))
                 .get(0); // all process contains same result
  }
  return res;
}

}

#endif
