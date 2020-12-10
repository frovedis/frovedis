#ifndef _GRID_SEARCH_CV_
#define _GRID_SEARCH_CV_

#include <frovedis/core/split_context.hpp>
#include <frovedis/matrix/rowmajor_matrix.hpp>
#include <frovedis/ml/model_selection/k_fold.hpp>
#include <frovedis/ml/model_selection/param.hpp>

#define PARALLEL_FIT

namespace frovedis {

template <class T>
int get_ngroups(T ncomb, T nproc, T min_proc_per_group) {
  if (ncomb < 1) return 1;
  else return std::min(ncomb, ceil_div(nproc, min_proc_per_group));
}

template <class T>
size_t get_max_index(const std::vector<T>& vec) {
  auto vecp = vec.data();
  auto size = vec.size();
  size_t max = 0;
  for(size_t i = 1; i < size; ++i) {
    if(vecp[i] > vecp[max]) max = i;
  }
  return max;
}

template <class ESTIMATOR, class MATRIX, class T>
void local_fit(ESTIMATOR& estimator,
               k_fold<MATRIX, T>& kf,
               paraml& param_list,
               rowmajor_matrix_local<float>& tr_score,
               rowmajor_matrix_local<float>& ts_score) {
  auto ncomb = param_list.size();
  auto nsplits = kf.get_nsplits();
  RLOG(DEBUG) << "ncomb: " << ncomb << "; nsplits: " << nsplits << "\n";
  // memory allocation for score matrices
  tr_score.val.resize(ncomb * nsplits);
  ts_score.val.resize(ncomb * nsplits);
  tr_score.set_local_num(ncomb, nsplits);
  ts_score.set_local_num(ncomb, nsplits);
  auto train_scorep = tr_score.val.data();
  auto test_scorep  = ts_score.val.data();
  for (size_t i = 0; i < nsplits; ++i){
    auto tmp = kf[i]; // train/test data gets scattered internally
    auto train_data = tmp.first;
    auto test_data = tmp.second;
    try {
      for (size_t j = 0; j < ncomb; ++j) {
        auto each = param_list[j];
        estimator.set_params(each);
        estimator.fit(train_data.point, train_data.label);
        train_scorep[j * nsplits + i] =
          estimator.score(train_data.point, train_data.label);
        test_scorep[j * nsplits + i]  =
          estimator.score(test_data.point, test_data.label);
      }
    }
    catch(std::exception& e) {
      std::cerr << "[split_" << std::to_string(i) 
                << "] exception caught: " << e.what() << "\n"; 
    }
  }
}

template <class ESTIMATOR, class MATRIX, class T>
void fit_in_groups(node_local<ESTIMATOR>& nl_est,
                   node_local<k_fold<MATRIX, T>>& nl_kf,
                   node_local<paraml>& nl_param,
                   node_local<rowmajor_matrix_local<float>>& nl_tr_score,
                   node_local<rowmajor_matrix_local<float>>& nl_ts_score) {
  auto est = nl_est.get(0);
  auto kf = nl_kf.get(0);
  auto param = nl_param.get(0);
  auto rank = get_split_rank_stack()[0];
  RLOG(DEBUG) << "parallel execution at group root: " << rank << "\n";
  rowmajor_matrix_local<float> tr_score, ts_score;
  local_fit(est, kf, param, tr_score, ts_score);
  nl_tr_score.put(0, tr_score);
  nl_ts_score.put(0, ts_score);
}

template <class ESTIMATOR>
struct grid_search_cv {
  grid_search_cv(ESTIMATOR& est,
                 std::map<std::string, std::vector<param_t>>& param_grid,
                 size_t nsplits = 5,
                 size_t min_proc_per_group = 2,
                 bool refit = true) {
    this->estimator = est;
    this->nsplits = nsplits;
    this->refit = refit;
    this->min_proc_per_group = min_proc_per_group;
    this->best_score = -1;
    this->param_list = evaluate_grid(param_grid);
  }

  template <class MATRIX, class T>
  grid_search_cv& 
  fit(MATRIX& mat, dvector<T>& label) {
    auto ncomb = param_list.size();
    size_t nproc = get_nodesize();
    // --- get splitted data ---
    k_fold<MATRIX, T> kf(nsplits);
    kf.fit(mat, label);
#ifdef PARALLEL_FIT
    // --- apply multy-group fit here (task level + data level parallelization) ---
    auto ngroups = get_ngroups(ncomb, nproc, min_proc_per_group);
    RLOG(INFO) << "search will be performed by " << ngroups << " parallel groups!\n";
    auto color = make_color(ngroups);
    auto nl_est = split_context_broadcast(color, estimator);
    auto nl_kf  = split_context_broadcast(color, kf);
    auto scattered_params = get_scattered_vectors(param_list, 
                                                  ncomb, 1, ngroups);
    auto nl_params = split_context_scatter(color, scattered_params);
    auto nl_tr_score = make_node_local_allocate<rowmajor_matrix_local<float>>();
    auto nl_ts_score = make_node_local_allocate<rowmajor_matrix_local<float>>();
    split_context_execution(color, fit_in_groups, nl_est, nl_kf, nl_params,
                            nl_tr_score, nl_ts_score); 
    auto tr_score = merge(split_context_gather(color, nl_tr_score));
    auto ts_score = merge(split_context_gather(color, nl_ts_score));
#else
    // --- apply single group fit here (only data level parallelization)---
    rowmajor_matrix_local<float> tr_score, ts_score;
    local_fit(estimator, kf, param_list , tr_score, ts_score);
#endif
    // --- result initializations ---
    initialize_result(mat, label, tr_score, ts_score);
    return *this;
  }

  template <class MATRIX>
  std::vector<typename MATRIX::value_type> 
  predict(MATRIX& mat) {
    std::vector<typename MATRIX::value_type> ret;
    if(refit) ret = best_estimator.predict(mat);
    else REPORT_ERROR(USER_ERROR, "predict() is available only when refit = true.\n");
    return ret;
  }

  void debug_print() {
    std::cout << "\ntrain score: \n"; train_score.debug_pretty_print(5);
    std::cout << "\ntest score: \n";  test_score.debug_pretty_print(5);
    if(refit) std::cout << "\nbest_score: " << best_score << "\n";
  }

  private:
  paraml evaluate_grid(std::map<std::string, 
                       std::vector<param_t>>& param_grid) {
    size_t ncombinations = 1;
    for(auto& e: param_grid) ncombinations *= e.second.size();
    paraml vec(ncombinations);
    size_t clusters = 1, clusterSize = ncombinations;
    for(auto& parm: param_grid) {
      size_t start = 0;
      size_t curr_vec_sz = parm.second.size();
      size_t curr_parm_cluster_cnt = clusterSize / curr_vec_sz;
      for(auto& elem: parm.second) {
        for(size_t i = 0; i < clusters; ++i) {
          for (size_t j = 0; j < curr_parm_cluster_cnt; ++j ) {
            vec[start+j+i*clusterSize].push_back(make_pair(parm.first, elem)); 
          }
        }
        start += curr_parm_cluster_cnt;
      }
      clusters *= curr_vec_sz;
      clusterSize = ncombinations / clusters;
    }
    return vec;
  }

  template <class MATRIX, class T>
  void initialize_result(MATRIX& mat, dvector<T>& label,
                         rowmajor_matrix_local<float>& tr_score,
                         rowmajor_matrix_local<float>& ts_score) {
    auto score = sum_of_cols(ts_score);
    auto scorep = score.data();
    auto one_by_nsplit = 1.0 / nsplits;
    auto ncomb = score.size();
    for(size_t i = 0; i < ncomb; ++i) scorep[i] *= one_by_nsplit;
    // returns index of best params (for which mean test_split_score is maximum)
    auto index = get_max_index(score);
    best_params = param_list[index];
    if(refit) {
      best_estimator = estimator.set_params(best_params)
                                .fit(mat, label);
      best_score = best_estimator.score(mat, label);
    }
    this->train_score = std::move(tr_score);
    this->test_score  = std::move(ts_score);
  }

  ESTIMATOR estimator;
  size_t nsplits;
  bool refit;
  paraml param_list;
  size_t min_proc_per_group;
  std::vector<std::pair<std::string, param_t>> best_params;
  rowmajor_matrix_local<float> train_score, test_score;
  // only when refit = true, else -1
  float best_score; 
  ESTIMATOR best_estimator;
  SERIALIZE(estimator, nsplits, refit, param_list, best_params,
            train_score, test_score, 
            best_score, best_estimator,
            min_proc_per_group)
};

// simplified global function 
template <class ESTIMATOR>
grid_search_cv<ESTIMATOR>
make_grid_search_cv(ESTIMATOR& est,
                    std::map<std::string, std::vector<param_t>>& param_grid,
                    size_t nsplits = 5,
                    size_t min_proc_per_group = 2,
                    bool refit = true) { 
  return grid_search_cv<ESTIMATOR>(est, param_grid, nsplits, min_proc_per_group, refit);
}

}
#endif
