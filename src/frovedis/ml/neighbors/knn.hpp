#ifndef _KNN_HPP_
#define _KNN_HPP_

#define COMPUTE_ONLY_B2_FOR_PARTIAL_DISTANCE 
//#define DEBUG_SAVE
//#define NEED_TRANS_FOR_MULT

#include <frovedis/core/radix_sort.hpp>
#include <frovedis/matrix/blas_wrapper.hpp>

namespace frovedis {

template <class T, class I = size_t>
struct knn_model {
  knn_model() {}
  rowmajor_matrix<I> indices;
  rowmajor_matrix<T> distances;
  int k;
  SERIALIZE(indices, distances, k)
};

template <class T>
rowmajor_matrix_local<T>
mult_a_with_trans_b (rowmajor_matrix_local<T>& a,
                     rowmajor_matrix_local<T>& b) {
    auto a_nrow = a.local_num_row;
    auto b_nrow = b.local_num_row;
    auto a_ncol = a.local_num_col;
    auto b_ncol = a.local_num_col;

    sliced_colmajor_matrix_local<T> sm1;
    sm1.ldm = a.local_num_col;
    sm1.data = a.val.data();
    sm1.sliced_num_row = a_ncol;
    sm1.sliced_num_col = a_nrow;

    sliced_colmajor_matrix_local<T> sm2;
    sm2.ldm = b.local_num_col;
    sm2.data = b.val.data();
    sm2.sliced_num_row = b_ncol;
    sm2.sliced_num_col = b_nrow;

    rowmajor_matrix_local<T> ret(a_nrow, b_nrow); // ret = a * trans(b)
    sliced_colmajor_matrix_local<T> sm3;
    sm3.ldm = b_nrow;
    sm3.data = ret.val.data();
    sm3.sliced_num_row = b_nrow;
    sm3.sliced_num_col = a_nrow;
    gemm<T>(sm2, sm1, sm3, 'T', 'N');
    return ret;
}

template <class T>
struct compute_dist {
  compute_dist() {}
  compute_dist(std::string& metric, bool is_trans_b,
               bool need_distance) {
    this->metric = metric;
    this->is_trans_b = is_trans_b;
    this->need_distance = need_distance;
  }
  rowmajor_matrix_local<T> 
  operator()(rowmajor_matrix_local<T>& a,
             rowmajor_matrix_local<T>& b) {
    if (!a.local_num_row || !b.local_num_row) return std::vector<T>();
    std::vector<T> a2, b2;
    const T *a2ptr, *b2ptr;
    a2ptr = b2ptr = NULL;
#ifdef COMPUTE_ONLY_B2_FOR_PARTIAL_DISTANCE
    if (need_distance) {
      a2 = squared_sum_of_cols(a);
      a2ptr = a2.data();
    }
    if (is_trans_b) b2 = squared_sum_of_rows(b); // b is transposed
    else b2 = squared_sum_of_cols(b); // b is not transposed
    b2ptr = b2.data();
#else
    a2 = squared_sum_of_cols(a);
    a2ptr = a2.data();
    if (need_distance) {
      if (is_trans_b) b2 = squared_sum_of_rows(b); // b is transposed
      else b2 = squared_sum_of_cols(b); // b is not transposed
      b2ptr = b2.data();
    }
#endif
    rowmajor_matrix_local<T> ab;
    if (is_trans_b) ab = a * b;
    else ab = mult_a_with_trans_b(a, b); // without physical transpose
    auto abptr = ab.val.data();
    auto nrow = ab.local_num_row;
    auto ncol = ab.local_num_col;
    if (metric == "euclidean" && need_distance) {
      if (nrow > ncol) {
#pragma _NEC nointerchange
        for(size_t j = 0; j < ncol; ++j) {
          for(size_t i = 0; i < nrow; ++i) {
            abptr[i*ncol+j] = std::sqrt(a2ptr[i] + b2ptr[j] - 2 * abptr[i*ncol+j]);
          }
        }
      }
      else {
#pragma _NEC nointerchange
        for(size_t i = 0; i < nrow; ++i) {
          for(size_t j = 0; j < ncol; ++j) {
            abptr[i*ncol+j] = std::sqrt(a2ptr[i] + b2ptr[j] - 2 * abptr[i*ncol+j]);
          }
        }
      }
    }
    else if (metric == "euclidean" && !need_distance) {
      if (nrow > ncol) {
#pragma _NEC nointerchange
        for(size_t j = 0; j < ncol; ++j) {
          for(size_t i = 0; i < nrow; ++i) {
            abptr[i*ncol+j] = b2ptr[j] - 2 * abptr[i*ncol+j];
          }
        }
      }
      else {
#pragma _NEC nointerchange
        for(size_t i = 0; i < nrow; ++i) {
          for(size_t j = 0; j < ncol; ++j) {
            abptr[i*ncol+j] = b2ptr[j] - 2 * abptr[i*ncol+j];
          }
        }
      }
    }
    else if (metric == "seuclidean" and need_distance) {
      if (nrow > ncol) {
#pragma _NEC nointerchange
        for(size_t j = 0; j < ncol; ++j) {
          for(size_t i = 0; i < nrow; ++i) {
            abptr[i*ncol+j] = (a2ptr[i] + b2ptr[j] - 2 * abptr[i*ncol+j]);
          }
        }
      }
      else {
#pragma _NEC nointerchange
        for(size_t i = 0; i < nrow; ++i) {
          for(size_t j = 0; j < ncol; ++j) {
            abptr[i*ncol+j] = (a2ptr[i] + b2ptr[j] - 2 * abptr[i*ncol+j]);
          }
        }
      }
    }
    else if (metric == "seuclidean" and !need_distance) {
      if (nrow > ncol) {
#pragma _NEC nointerchange
        for(size_t j = 0; j < ncol; ++j) {
          for(size_t i = 0; i < nrow; ++i) {
            abptr[i*ncol+j] = b2ptr[j] - 2 * abptr[i*ncol+j];
          }
        }
      }
      else {
#pragma _NEC nointerchange
        for(size_t i = 0; i < nrow; ++i) {
          for(size_t j = 0; j < ncol; ++j) {
            abptr[i*ncol+j] = b2ptr[j] - 2 * abptr[i*ncol+j];
          }
        }
      }
    }

    else
      throw std::runtime_error("currently frovedis knn supports only euclidean/seuclidean distance!\n");
    return ab;
  }
  std::string metric;
  bool is_trans_b, need_distance;
  SERIALIZE(metric, is_trans_b, need_distance)
};

template <class T, class I>
rowmajor_matrix_local<I> 
sort_rows(rowmajor_matrix_local<T>& mat) {
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  std::vector<T> tmp(nrow*ncol); // to copy mat.val
  rowmajor_matrix_local<I> indices(nrow, ncol); // reused later for col-index
  std::vector<I> row_indx(nrow*ncol);
  auto mat_ptr = mat.val.data();
  auto tmp_ptr = tmp.data();
  auto indx_ptr = indices.val.data();
  auto rindx_ptr = row_indx.data();
  for(size_t i = 0; i < nrow *  ncol; ++i) {
    tmp_ptr[i] = mat_ptr[i]; // copy before sort
    indx_ptr[i] = i;
  }
  radix_sort(mat.val, indices.val, true); // sort entire matrix data in one go
  for(size_t i = 0; i < nrow *  ncol; ++i) {
    rindx_ptr[i] = indx_ptr[i] / ncol;
    indx_ptr[i] = indx_ptr[i] % ncol; // physical index to col index conversion: reuse of memory 
  }
  radix_sort(row_indx, indices.val, true); // sort to know positional changes in each row
  for(size_t k = 0; k < nrow *  ncol; ++k) {
    auto i = rindx_ptr[k];
    auto j = indx_ptr[k]; //actually col-index
    mat_ptr[k] = tmp_ptr[i * ncol + j]; // update mat in-place using its copy (before sort)
  }
  return indices;
}

template <class T, class I>
rowmajor_matrix_local<I>
sort_row_indices(rowmajor_matrix_local<T>& mat) {
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  rowmajor_matrix_local<I> indices(nrow, ncol); // reused later as col-index
  std::vector<I> row_indx(nrow*ncol);
  auto indx_ptr = indices.val.data();
  auto rindx_ptr = row_indx.data();
  for(size_t i = 0; i < nrow *  ncol; ++i) indx_ptr[i] = i;
  radix_sort(mat.val, indices.val, true); // sort entire matrix data in one go
  for(size_t i = 0; i < nrow *  ncol; ++i) {
    rindx_ptr[i] = indx_ptr[i] / ncol;
    indx_ptr[i] = indx_ptr[i] % ncol; // physical index to col index conversion: reuse of memory
  }
  radix_sort(row_indx, indices.val, true); // sort to know positional changes in each row
  return indices;
}

template <class T>
struct extract_k_cols {
  extract_k_cols() {}
  extract_k_cols(int k_): k(k_) {}
  rowmajor_matrix_local<T> operator()(rowmajor_matrix_local<T>& mat) {
    auto nrow = mat.local_num_row;
    auto ncol = mat.local_num_col;
    rowmajor_matrix_local<T> ret(nrow, k);
    auto rptr = ret.val.data();
    auto mptr = mat.val.data();
#pragma _NEC nointerchange
    for(size_t i = 0; i < k; ++i) {
      for(size_t j = 0; j < nrow; ++j) { // nrow >> k
        rptr[j * k + i] = mptr[j * ncol + i];
      }
    }
    return ret;
  }
  int k;
  SERIALIZE(k)
};
 
template <class T, class I = size_t>
knn_model<T, I> knn(rowmajor_matrix<T>& x_mat,
                    rowmajor_matrix<T>& y_mat,
                    int k,
                    const std::string& metric = "euclidean",
                    bool need_distance = false) {
  std::string dist_metric = metric; // for const
  auto nsamples = x_mat.num_row;
  auto nquery   = y_mat.num_row;
  if (k < 0 || k > nsamples) 
    throw std::runtime_error("invalid value for k: " + std::to_string(k) + "\n");

  rowmajor_matrix<T> dist_mat;
  rowmajor_matrix<I> indx_mat;
  time_spent calc_dist(INFO), sort_dist(INFO), extract_dist(INFO);

  // a simple heuristic to decide which matrix to broadcast
  if (nsamples/100 <= nquery) {
    auto g_x_mat = x_mat.gather();
    calc_dist.lap_start();
#ifdef NEED_TRANS_FOR_MULT
    auto b_mat = broadcast(g_x_mat.transpose());
    auto trans_b = true;
#else
    auto b_mat = broadcast(g_x_mat);
    auto trans_b = false;
#endif
    auto loc_dist_mat = y_mat.data.map(compute_dist<T>(dist_metric,
                                       trans_b,need_distance), b_mat);
    dist_mat.data = std::move(loc_dist_mat);
    dist_mat.num_row = nquery;
    dist_mat.num_col = nsamples; // no tranpose required, since Y * Xt is performed
    calc_dist.lap_stop();
    calc_dist.show_lap("dist calculation: ");
  }

  else { // nsamples >> nquery (more than 100 times)
    auto g_y_mat = y_mat.gather();
    calc_dist.lap_start();
#ifdef NEED_TRANS_FOR_MULT
    auto b_mat = broadcast(g_y_mat.transpose());
    auto trans_b = true;
#else
    auto b_mat = broadcast(g_y_mat);
    auto trans_b = false;
#endif
    auto loc_dist_mat = x_mat.data.map(compute_dist<T>(dist_metric,
                                       trans_b,need_distance), b_mat);
    rowmajor_matrix<T> tmp_dist_mat(std::move(loc_dist_mat));
    tmp_dist_mat.num_row = nsamples;
    tmp_dist_mat.num_col = nquery;
    dist_mat = tmp_dist_mat.transpose(); // transpose is needed, since X * Yt is performed
    calc_dist.lap_stop();
    calc_dist.show_lap("dist calculation: ");
  }

#ifdef DEBUG_SAVE
  dist_mat.save("unsorted_distance_matrix");
#endif

  sort_dist.lap_start();
  if (need_distance)
    indx_mat = dist_mat.data.map(sort_rows<T,I>);
  else
    indx_mat = dist_mat.data.map(sort_row_indices<T,I>); // dist_mat would be destroyed
  sort_dist.lap_stop();
  sort_dist.show_lap("sorting distance: ");

  knn_model<T, I> ret; 
  extract_dist.lap_start();
  if (k != nsamples) { // slicing is required
    ret.k = k;
    if (need_distance) {
      ret.distances.data = dist_mat.data.map(extract_k_cols<T>(k));
      ret.distances.num_row = dist_mat.num_row; // indx_mat.num_row;
      ret.distances.num_col = k;
    }
    ret.indices.data = indx_mat.data.map(extract_k_cols<I>(k));
    ret.indices.num_row = dist_mat.num_row; // indx_mat.num_row;
    ret.indices.num_col = k;
  }
  else { // no slicing is required
    ret.k = k;
    if (need_distance) ret.distances = std::move(dist_mat);
    ret.indices = std::move(indx_mat);
  }
  extract_dist.lap_stop();
  extract_dist.show_lap("k-nearest neighbour extraction: ");

  return ret;
}

template <class T, class I = size_t>
knn_model<T, I> knn(rowmajor_matrix<T>& mat,
                    int k,
                    const std::string& metric = "euclidean",
                    bool need_distance = false) {
  return knn(mat, mat, k, metric, need_distance);
}

}

#endif
