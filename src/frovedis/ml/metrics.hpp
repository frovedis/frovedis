#ifndef _METRICS_
#define _METRICS_

#include <frovedis/core/exceptions.hpp>
#include <frovedis/core/vector_operations.hpp>
#include <frovedis/matrix/rowmajor_matrix.hpp>
#include <frovedis/matrix/matrix_operations.hpp>

/* this header contains user apis for metrics evaluation:
 *     construct_contingency_matrix(): returns contingency_matrix based on true and predicted labels
 *     construct_confusion_matrix(): returns confusion_matrix based on true and predicted labels
 *     r2_score(): returns float r2 score based on true label and predicted label
 *     accuracy_score(): returns float accuracy score based on true label and predicted label
 *     entropy(): returns double entropy value for given labels
 *     homogeneity_score(): under development
 *     require(COND, MSG): macro used for exception check
 *     STR(n): returns string equivalent of n
 */

namespace frovedis {

template <class T, class I = size_t>
struct triplet {
  triplet() {}
  triplet(const std::vector<T>& v,
          const std::vector<I>& ri,
          const std::vector<I>& ci) {
    val = v;
    ridx = ri;
    cidx = ci;
  }
  triplet(std::vector<T>&& v,
          std::vector<I>&& ri,
          std::vector<I>&& ci) {
    val.swap(v);
    ridx.swap(ri);
    cidx.swap(ci);
  }
  void debug_print(const size_t limit = 0) const {
    std::cout << "ridx: "; debug_print_vector(ridx, limit);
    std::cout << "cidx: "; debug_print_vector(cidx, limit);
    std::cout << "val: ";  debug_print_vector(val, limit);
  }
  void save(const std::string& fname) const {
    make_dvector_scatter(val).saveline(fname + "_val");
    make_dvector_scatter(ridx).saveline(fname + "_ridx");
    make_dvector_scatter(cidx).saveline(fname + "_cidx");
  }
  std::vector<T> val;
  std::vector<I> ridx, cidx;
  SERIALIZE(val, ridx, cidx)
};

// group by pair of (ridx, cidx) and count each grouped pair
// ncol must be max(cidx) + 1 -> cidx[vector_argmax(cidx)] + 1
template <class T>
triplet<T,T> 
count_duplicates(const std::vector<T>& ridx,
                 const std::vector<T>& cidx, 
                 size_t ncol) {
  auto nz = ridx.size();
  checkAssumption(nz == cidx.size());
  auto ridxp = ridx.data();
  auto cidxp = cidx.data();
  std::vector<T> tmp(nz);
  auto tptr = tmp.data();
  // combining rowid and colid to create actual 1D index
  for(size_t i = 0; i < nz; ++i) tptr[i] = ridxp[i] * ncol + cidxp[i];
  radix_sort(tmp, true); // true: index is always positive
  auto sep = set_separate(tmp);
  auto sepsz = sep.size() - 1;
  std::vector<T> ret_ridx(sepsz), ret_cidx(sepsz), ret_cnt(sepsz);
  auto r_ridxp = ret_ridx.data();
  auto r_cidxp = ret_cidx.data();
  auto r_cntp = ret_cnt.data();
  auto sptr = sep.data();
  for(size_t i = 0; i < sepsz; ++i) {
    auto id = tptr[sptr[i]];
    auto r = id / ncol;
    auto c = id - (r * ncol);
    r_ridxp[i] = r;
    r_cidxp[i] = c;
    r_cntp[i] = sptr[i + 1] - sptr[i];
  }
  return triplet<T,T>(std::move(ret_cnt), std::move(ret_ridx), 
                      std::move(ret_cidx));
}

template <class T, class I = size_t>
triplet<T,I>
rowmajor_to_coo(const rowmajor_matrix_local<T>& mat) { // optimized to_crs()
  auto nz = vector_count_nonzero(mat.val);
  std::vector<T> val(nz); 
  std::vector<I> ridx(nz), cidx(nz);
  auto sz = mat.val.size();
  auto mptr = mat.val.data();
  auto ridxp = ridx.data();
  auto cidxp = cidx.data();
  auto valp  = val.data();
  auto ncol = mat.local_num_col;
  size_t k = 0;
  for(size_t i = 0; i < sz; ++i) {
    if (mptr[i] != 0) {
      auto r = i / ncol;
      auto c = i - (r * ncol);  // faster than (i % ncol)
      ridxp[k] = r;
      cidxp[k] = c;
      valp[k] = mptr[i];
      k++;
    }
  }
  return triplet<T,I>(std::move(val), std::move(ridx), std::move(cidx));
}

template <class T, class I>
rowmajor_matrix_local<T>
coo_to_rowmajor(const triplet<T,I>& t, 
                size_t nrow, size_t ncol) {
  auto sz = t.val.size();
  auto tvalp = t.val.data();
  auto ridxp = t.ridx.data();
  auto cidxp = t.cidx.data();
  rowmajor_matrix_local<T> ret(nrow, ncol);
  auto rvalp  = ret.val.data();
  for(size_t i = 0; i < sz; ++i) {
    auto r = ridxp[i];
    auto c = cidxp[i];
    rvalp[r * ncol + c] = tvalp[i];
  }
  return ret;
}

template <class T>
void print_eval_matrix(const rowmajor_matrix_local<T>& mat,
                       const std::vector<int>& row_index,
                       const std::vector<int>& col_index,
                       const size_t& limit = 0) {
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  // -------- print header --------
  std::cout << "     ";
  if (limit == 0 || ncol <= 2 * limit) {
    for(size_t i = 0; i < ncol; ++i) {
      std::cout << "l(" << col_index[i] << ") ";
    }
  }
  else {
    for(size_t i = 0; i < limit; ++i) {
      std::cout << "l(" << col_index[i] << ") ";
    }
    std::cout << " ... ";
    for(size_t i = ncol - limit; i < ncol; ++i) {
      std::cout << "l(" << col_index[i] << ") ";
    }
  }
  std::cout << std::endl;
  // -------- print matrix --------
  if (limit == 0 || nrow <= 2 * limit) {
    for(size_t i = 0; i < nrow; ++i) {
      std::cout << "l(" << row_index[i] << "): ";
      if (limit == 0 || ncol <= 2 * limit) {
        for(size_t j = 0; j < ncol; ++j) {
          std::cout << mat.val[i * ncol + j] << "   ";
        }
      }
      else {
        for(size_t j = 0; j < limit; ++j) {
          std::cout << mat.val[i * ncol + j] << "   ";
        }
        std::cout << " ...   ";
        for(size_t j = ncol - limit; j < ncol; ++j) {
          std::cout << mat.val[i * ncol + j] << "   ";
        }
      }
      std::cout << std::endl;
    }
  }
  else {
    for(size_t i = 0; i < limit; ++i) {
      std::cout << "l(" << row_index[i] << "): ";
      if (limit == 0 || ncol <= 2 * limit) {
        for(size_t j = 0; j < ncol; ++j) {
          std::cout << mat.val[i * ncol + j] << "   ";
        }
      }
      else {
        for(size_t j = 0; j < limit; ++j) {
          std::cout << mat.val[i * ncol + j] << "   ";
        }
        std::cout << " ...   ";
        for(size_t j = ncol - limit; j < ncol; ++j) {
          std::cout << mat.val[i * ncol + j] << "   ";
        }
      }
      std::cout << std::endl;
    }
    std::cout << " :\n :\n";
    for(size_t i = nrow - limit; i < nrow; ++i) {
      std::cout << "l(" << row_index[i] << "): ";
      if (limit == 0 || ncol <= 2 * limit) {
        for(size_t j = 0; j < ncol; ++j) {
          std::cout << mat.val[i * ncol + j] << "   ";
        }
      }
      else {
        for(size_t j = 0; j < limit; ++j) {
          std::cout << mat.val[i * ncol + j] << "   ";
        }
        std::cout << " ...   ";
        for(size_t j = ncol - limit; j < ncol; ++j) {
          std::cout << mat.val[i * ncol + j] << "   ";
        }
      }
      std::cout << std::endl;
    }
  }
}

template <class T>
struct confusion_matrix {
  confusion_matrix() {}
  confusion_matrix(const rowmajor_matrix_local<T>& m, 
                   const std::vector<int>& ridx,
                   const std::vector<int>& cidx) {
    checkAssumption(ridx.size() == cidx.size());
    row_index = ridx;
    col_index = cidx;
    data = m;
  }
  confusion_matrix(rowmajor_matrix_local<T>&& m,
                   std::vector<int>&& ridx,
                   std::vector<int>&& cidx) {
    checkAssumption(ridx.size() == cidx.size());
    row_index.swap(ridx);
    col_index.swap(cidx);
    data = std::move(m);
  }
  triplet<T,T> sparsify() const { 
    return rowmajor_to_coo(data);
  }
  void show(const size_t& limit = 0) const {
    print_eval_matrix(data, row_index, col_index, limit);
  }
  rowmajor_matrix_local<T> data; 
  std::vector<int> row_index, col_index;
  SERIALIZE(data, row_index, col_index)
};

template <class T>
struct contingency_matrix {
  contingency_matrix() {}
  contingency_matrix(const rowmajor_matrix_local<T>& m,
                     const std::vector<int>& ridx,
                     const std::vector<int>& cidx) {
    row_index = ridx;
    col_index = cidx;
    data = m;
  }
  contingency_matrix(rowmajor_matrix_local<T>&& m,
                     std::vector<int>&& ridx,
                     std::vector<int>&& cidx) {
    row_index.swap(ridx);
    col_index.swap(cidx);
    data = std::move(m);
  }
  triplet<T,T> sparsify() const {
    return rowmajor_to_coo(data);
  }
  void show(const size_t& limit = 0) const {
    print_eval_matrix(data, row_index, col_index, limit);
  }
  rowmajor_matrix_local<T> data;
  std::vector<int> row_index, col_index;
  SERIALIZE(data, row_index, col_index)
};

template <class I>
rowmajor_matrix_local<I>
construct_eval_matrix_helper(const std::vector<I>& t_enc,
                             const std::vector<I>& p_enc,
                             size_t ncls, size_t nclus) {
  auto t = count_duplicates(t_enc, p_enc, nclus);
  return coo_to_rowmajor(t, ncls, nclus);
}

template <class T>
rowmajor_matrix_local<size_t>
construct_eval_matrix(const std::vector<T>& tlbl,
                      const std::vector<T>& plbl,
                      std::vector<T>& t_unq,
                      std::vector<T>& p_unq) {
  checkAssumption(tlbl.size() == plbl.size());
  std::vector<size_t> t_ind, t_enc, t_cnt, p_ind, p_enc, p_cnt;
  t_unq = vector_unique(tlbl, t_ind, t_enc, t_cnt);
  p_unq = vector_unique(plbl, p_ind, p_enc, p_cnt);
  return construct_eval_matrix_helper(t_enc, p_enc, t_unq.size(), p_unq.size());
}

template <class T>
contingency_matrix<size_t>
construct_contingency_matrix(const std::vector<T>& tlbl,
                             const std::vector<T>& plbl) {
  std::vector<T> t_unq, p_unq;
  auto mat = construct_eval_matrix(tlbl, plbl, t_unq, p_unq);
  return contingency_matrix<size_t>(mat, 
                                    vector_astype<int>(t_unq), 
                                    vector_astype<int>(p_unq));
}

template <class T>
confusion_matrix<size_t>
construct_confusion_matrix(const std::vector<T>& tlbl,
                           const std::vector<T>& plbl) {
  std::vector<T> t_unq, p_unq;
  auto mat = construct_eval_matrix(tlbl, plbl, t_unq, p_unq);
  return confusion_matrix<size_t>(mat, 
                                  vector_astype<int>(t_unq), 
                                  vector_astype<int>(p_unq));
}

// ref: https://github.com/scikit-learn/scikit-learn/blob/0fb307bf3/sklearn/metrics/cluster/_supervised.py#L960
template <class T>
float entropy_helper(const std::vector<T>& lbl_cnt) {
  auto pi = vector_astype<double>(lbl_cnt);
  auto pi_sum = vector_sum(pi);
  return -1.0f * vector_sum((pi / pi_sum) * (vector_log(pi) - std::log(pi_sum)));
}

template <class T>
float entropy(const std::vector<T>& lbl) {
  if (lbl.size() == 0) return 1.0;
  std::vector<size_t> ind, cnt;
  auto unq = vector_unique(lbl, ind, cnt); 
  return entropy_helper(cnt); // needed only count of unique labels for entropy
}

// ref: https://github.com/scikit-learn/scikit-learn/blob/0fb307bf3/sklearn/metrics/cluster/_supervised.py#L571
template <class T>
float mutual_info_score(const contingency_matrix<T>& cmat) {
  auto t = cmat.sparsify();
  //t.debug_print(10); 
  auto nzx = t.ridx;
  auto nzy = t.cidx;
  auto nz_val = vector_astype<double>(t.val);
  auto mat = cmat.data; // densified local rowmajor matrix
  auto contingency_sum = vector_sum(nz_val) ; // avoiding zero addition in matrix_sum(mat)
  auto pi = matrix_sum(mat, 1);
  auto pj = matrix_sum(mat, 0);
  auto log_contingency = vector_log(nz_val);
  auto log_contingency_sum = std::log(contingency_sum);
  auto contingency_nm = nz_val / contingency_sum;
  auto outer = vector_take(pi, nzx) * vector_take(pj, nzy);
  auto log_outer = vector_negative_log(outer) + 
                   (std::log(vector_sum(pi)) + std::log(vector_sum(pj)));
  auto mi = contingency_nm * (log_contingency - log_contingency_sum) +
            (contingency_nm * log_outer);
  auto mi_sum = vector_sum(mi);
  return mi_sum < 0.0 ? 0.0 : mi_sum;
}

template <class T>
float r2_score(const std::vector<T>& true_label,
               const std::vector<T>& pred_label) {
  auto size = true_label.size();
  require(size == pred_label.size(),
  "r2_score: size differs in predicted labels and actual labels!\n");
  //debug_print_vector(true_label, 10);
  //debug_print_vector(pred_label, 10);
  /*
   *  input can be int-vectors.
   *  hence overflow might occur inside vector_ssd() or vector_ssmd().
   *  therefore, type casting to double-vector is required.
  */   
  auto tlbl = vector_astype<double>(true_label); // astype() is required, since input can be int
  auto plbl = vector_astype<double>(pred_label);
  auto u = vector_ssd(tlbl, plbl);
  auto v = vector_ssmd(tlbl);
  float u_by_v = v ? static_cast<float>(u) / static_cast<float>(v) : 1.0f;
  //std::cout << u << " " << v << " " << u_by_v << std::endl;
  return 1.0f - u_by_v;
}

template <class T>
float accuracy_score(const std::vector<T>& true_label,
                     const std::vector<T>& pred_label) {
  auto size = true_label.size();
  require(size == pred_label.size(), 
  "accuracy_score: size differs in predicted labels and actual labels!\n");
  size_t count = 0;
  auto predp = pred_label.data();
  auto actualp = true_label.data();
  //debug_print_vector(true_label, 10);
  //debug_print_vector(pred_label, 10);
  for(size_t i = 0; i < size; ++i) count += (predp[i] == actualp[i]);
  RLOG(INFO) << "[accuracy_score] " << count << "/" << size 
             << " predicted results have been matched!\n";
  return static_cast<float>(count) / size;
}

// ref: https://github.com/scikit-learn/scikit-learn/blob/0fb307bf3/sklearn/metrics/cluster/_supervised.py#L247
// T: must be integral type: int, long, size_t etc.
// floating point like 1.0, 2.0 is also OK, but 1.5, 2.2 etc. are nlot allowed!
template <class T>
std::vector<float> 
get_supervised_clustering_score(const std::vector<T>& true_label,
                                const std::vector<T>& pred_label,
                                const std::string& target,
                                const float& beta = 1.0) { // used for v_measure
  auto size = true_label.size();
  require(size == pred_label.size(),
  target + "_score: size differs in predicted labels and actual labels!\n");
  //debug_print_vector(true_label, 10);
  //debug_print_vector(pred_label, 10);
  
  if (size == 0) return std::vector<float>({1.0f, 1.0f, 1.0f}); // quick return

  /* manual in-line expansion of construct_contingency_matrix():
   * objective: to reduce no. of calls for vector_unique()
   *   reused result of vector_unique for true_label/pred_label 
   *   in calculations for entropy of respective labels and contingency matrix.
   *
   *   auto entropy_C = entropy(true_label); 
   *   auto entropy_K = entropy(pred_label);
   *   auto cont_mat = construct_contingency_matrix(true_label, pred_label);
   */
  std::vector<size_t> t_ind, t_enc, t_cnt, p_ind, p_enc, p_cnt;
  auto t_unq = vector_unique(true_label, t_ind, t_enc, t_cnt);
  auto p_unq = vector_unique(pred_label, p_ind, p_enc, p_cnt);
  auto mat = construct_eval_matrix_helper(t_enc, p_enc, 
                                          t_unq.size(), p_unq.size());
  auto cont_mat = contingency_matrix<size_t>(mat,
                                             vector_astype<int>(t_unq), 
                                             vector_astype<int>(p_unq));
  auto mi = mutual_info_score(cont_mat);

  std::vector<float> ret = {-1.0, -1.0, -1.0};
  // for v_measure: it computes all ret[0], ret[1] and ret[2]
  if (target == "homogeneity" || target == "v_measure") {
    auto entropy_C = entropy_helper(t_cnt); // re-used t_cnt here
    ret[0] = entropy_C ? (mi / entropy_C) : 1.0;
  }
  if (target == "completeness" || target == "v_measure") {
    auto entropy_K = entropy_helper(p_cnt); // re-used p_cnt here
    ret[1] = entropy_K ? (mi / entropy_K) : 1.0;
  }
  if (target == "v_measure") {
    auto homogeneity = ret[0];
    auto completeness = ret[1];
    if (homogeneity + completeness == 0.0) ret[2] = 0.0f;
    else  ret[2] = (1 + beta) * homogeneity * completeness / 
                   (beta * homogeneity + completeness);
  }
  return ret;
}

template <class T>
float homogeneity_score(const std::vector<T>& true_label,
                        const std::vector<T>& pred_label) {
  return get_supervised_clustering_score(true_label, 
         pred_label, "homogeneity")[0];
}

template <class T>
float completeness_score(const std::vector<T>& true_label,
                         const std::vector<T>& pred_label) {
  return get_supervised_clustering_score(true_label, 
         pred_label, "completeness")[1];
}

template <class T>
float v_measure_score(const std::vector<T>& true_label,
                      const std::vector<T>& pred_label,
                      const float& beta = 1.0) {
  return get_supervised_clustering_score(true_label, 
         pred_label, "v_measure", beta)[2];
}

}
#endif
