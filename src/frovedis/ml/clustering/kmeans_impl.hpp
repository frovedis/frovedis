#ifndef KMEANS_IMPL_HPP
#define KMEANS_IMPL_HPP

#include "kmeans_utils.hpp"
#include "../../core/radix_sort.hpp"
#include "../../core/set_operations.hpp"
#include "../../matrix/matrix_operations.hpp"

namespace frovedis {

template <class T, class I, class O>
void kmeans_calc_sum_helper(rowmajor_matrix_local<T>& prod,
                            crs_matrix_local<T,I,O>& samples,
                            rowmajor_matrix_local<T>& centroid,
                            std::vector<T>& norm,
                            std::vector<size_t>& occurrence,
                            std::vector<T>& sum) {
  time_spent t(TRACE);
  size_t num_centroids = prod.local_num_col;
  size_t num_samples = prod.local_num_row;
  std::vector<int> minloc(num_samples);
  auto minlocp = minloc.data();
  norm_minus_product(norm, prod.val, num_centroids, num_samples);
  t.show(" norm minus product: ");
  calc_minloc(minloc, prod.val, num_centroids, num_samples);
  t.show(" calc minloc: ");
  calc_occurrence(occurrence, minlocp, num_centroids, num_samples);
  t.show(" calc occurrence: ");

  size_t dim = samples.local_num_col;
  sum.clear();
  sum.resize(dim * num_centroids); // rowmajor dim x k
  auto sump = sum.data();
  auto valp = samples.val.data();
  auto idxp = samples.idx.data();
  auto offp = samples.off.data();
  for(size_t r = 0; r < num_samples; r++) {
#pragma _NEC vovertake
#pragma _NEC vob
#pragma _NEC ivdep
    for(size_t c = offp[r]; c < offp[r+1]; c++) {
      sump[idxp[c] * num_centroids + minlocp[r]] += valp[c];
    }
  }
  t.show(" calc sum: ");
}

#ifdef KMEANS_HYBRID
template <class T, class I, class O>
void kmeans_calc_sum(jds_crs_hybrid_local<T,I,O>& mat,
                     crs_matrix_local<T,I,O>& samples,
                     rowmajor_matrix_local<T>& centroid,
                     std::vector<T>& norm,
                     std::vector<size_t>& occurrence,
                     std::vector<T>& sum) {
  time_spent t(TRACE);
  auto prod = mat * centroid;
  t.show(" spmm (hybrid): ");
  kmeans_calc_sum_helper(prod, samples, centroid, norm, occurrence, sum);
}
#elif defined KMEANS_JDS
template <class T, class I, class O>
void kmeans_calc_sum(jds_matrix_local<T,I,O>& mat,
                     crs_matrix_local<T,I,O>& samples,
                     rowmajor_matrix_local<T>& centroid,
                     std::vector<T>& norm,
                     std::vector<size_t>& occurrence,
                     std::vector<T>& sum) {
  time_spent t(TRACE);
  auto prod = mat * centroid;
  t.show(" spmm (jds): ");
  kmeans_calc_sum_helper(prod, samples, centroid, norm, occurrence, sum);
}
#elif defined KMEANS_CRS
template <class T, class I, class O>
void kmeans_calc_sum(crs_matrix_local<T,I,O>& mat,
                     crs_matrix_local<T,I,O>& samples, // same as mat
                     rowmajor_matrix_local<T>& centroid,
                     std::vector<T>& norm,
                     std::vector<size_t>& occurrence,
                     std::vector<T>& sum) {
  time_spent t(TRACE);
  auto prod = mat * centroid;
  t.show(" spmm (crs): ");
  kmeans_calc_sum_helper(prod, samples, centroid, norm, occurrence, sum);
}
#else
#error specify matrix format by defining JDS, CRS, or HYBRID
#endif

template <class T>
void kmeans_calc_sum_rowmajor(rowmajor_matrix_local<T>& mat,
                              rowmajor_matrix_local<T>& centroid,
                              std::vector<T>& norm,
                              std::vector<size_t>& occurrence,
                              std::vector<T>& sum) {
  time_spent t(TRACE);
  auto prod = mat * centroid;
  t.show(" gemm (rowmajor): ");
  size_t num_centroids = prod.local_num_col;
  size_t num_samples = prod.local_num_row;
  std::vector<int> minloc(num_samples);
  auto minlocp = minloc.data();
  norm_minus_product(norm, prod.val, num_centroids, num_samples);
  t.show(" norm minus product: ");
  calc_minloc(minloc, prod.val, num_centroids, num_samples);
  t.show(" calc minloc: ");
  calc_occurrence(occurrence, minlocp, num_centroids, num_samples);
  t.show(" calc occurrence: ");
  size_t dim = mat.local_num_col;
  sum.clear();
  sum.resize(dim * num_centroids); // rowmajor dim x k
  auto sump = sum.data();
  auto valp = mat.val.data();
#ifdef __ve__
  std::vector<int> idx(num_samples);
  auto idxp = idx.data();
  for(size_t i = 0; i < num_samples; i++) idxp[i] = i;
  radix_sort(minlocp, idxp, num_samples, true);
  auto sep = set_separate(minloc);
  auto sep_size = sep.size();
  auto sepp = sep.data();
  for(size_t i = 0; i < sep_size-1; i++) {
    auto begin = sepp[i];
    auto end = sepp[i+1];
    auto pos = minlocp[begin];
    for(size_t c = 0; c < dim; c++) {
      T crnt_sum = 0;
      for(size_t j = begin; j < end; j++) {
        crnt_sum += valp[dim * idxp[j] + c];
      }
      sump[num_centroids * c + pos] = crnt_sum;
    }
  }
#else
  for(size_t r = 0; r < num_samples; r++) {
    for(size_t c = 0; c < dim; c++) {
      sump[num_centroids * c + minlocp[r]] += valp[dim * r + c];
    }
  }
#endif
  t.show(" calc sum: ");
}

template <class T, class I, class O>
rowmajor_matrix_local<T> 
kmeans_impl(crs_matrix<T,I,O>& samples, int k,
            int iter, double eps, 
            int& n_iter_) {
  time_spent t(TRACE);
  auto centroid = get_random_rows(samples, k);
  size_t dim = samples.num_col;
  t.show("create initial centroid: ");
#ifdef KMEANS_HYBRID
  jds_crs_hybrid<T,I,O,O> mat(samples);
  t.show("convert matrix (hybrid): ");
#elif defined KMEANS_JDS
  jds_matrix<T,I,O,O> mat(samples);
  t.show("convert matrix (jds): ");
#elif defined KMEANS_CRS
  crs_matrix<T,I,O>& mat = samples;
#else
#error specify matrix format by defining JDS, CRS, or HYBRID
#endif
  time_spent t2(TRACE);
  time_spent bcasttime(DEBUG), normtime(DEBUG), disttime(DEBUG), vecsumtime(DEBUG),
    nextcentroidtime(DEBUG);
  auto dsum = make_node_local_allocate<std::vector<T>>();
  auto doccurrence = make_node_local_allocate<std::vector<size_t>>();
  auto norm_tmp = make_node_local_allocate<std::vector<T>>();
  int i = 0;
  for(i = 0; i < iter; i++) {
    RLOG(TRACE) << "num centroids: " << centroid.local_num_col << std::endl;
    bcasttime.lap_start();
    auto bcentroid = centroid.broadcast();
    bcasttime.lap_stop();
    t.show("broadcast: ");
    normtime.lap_start();
    auto bnorm = broadcast(calc_norm<T>(bcentroid, norm_tmp));
    normtime.lap_stop();
    t.show("calc norm: ");
    disttime.lap_start();
    mat.data.mapv(kmeans_calc_sum<T,I,O>, samples.data, bcentroid, bnorm,
                  doccurrence, dsum);
    disttime.lap_stop();
    t.show("calc_sum map: ");
    vecsumtime.lap_start();
    auto sum = dsum.vector_sum();
    auto occurrence = doccurrence.vector_sum();
    vecsumtime.lap_stop();
    t.show("calc_sum vector_sum: ");
    nextcentroidtime.lap_start();
    auto next_centroid = calc_new_centroid(sum, occurrence, dim);
    nextcentroidtime.lap_stop();
    t.show("calc next centroid: ");
    auto diff = is_diff_centroid(next_centroid, centroid, eps);
    centroid = std::move(next_centroid);
    t.show("diff centroid: ");
    t2.show("one iteration: ");
    if (!diff) break;
  }
  n_iter_ = i + 1;
  RLOG(DEBUG) << "converged in n_iter_: " << n_iter_ << std::endl;
  bcasttime.show_lap("broadcast centroid time: ");
  normtime.show_lap("normalize centroid time: ");
  disttime.show_lap("distributed computation time: ");
  vecsumtime.show_lap("vector sum time: ");
  nextcentroidtime.show_lap("calculate next centroid time: ");
  return centroid;
}

// TODO: mostly the same as the sparse version; use same code
template <class T>
rowmajor_matrix_local<T> 
kmeans_impl(rowmajor_matrix<T>& mat, int k,
            int iter, double eps, 
            int& n_iter_) {
  time_spent t(TRACE);
  auto centroid = get_random_rows(mat, k);
  size_t dim = mat.num_col;
  t.show("create initial centroid: ");
  time_spent t2(TRACE);
  time_spent bcasttime(DEBUG), normtime(DEBUG), disttime(DEBUG), vecsumtime(DEBUG),
    nextcentroidtime(DEBUG);
  auto dsum = make_node_local_allocate<std::vector<T>>();
  auto doccurrence = make_node_local_allocate<std::vector<size_t>>();
  auto norm_tmp = make_node_local_allocate<std::vector<T>>();
  int i = 0;
  for(i = 0; i < iter; i++) {
    RLOG(TRACE) << "num centroids: " << centroid.local_num_col << std::endl;
    bcasttime.lap_start();
    auto bcentroid = centroid.broadcast();
    bcasttime.lap_stop();
    t.show("broadcast: ");
    normtime.lap_start();
    auto bnorm = broadcast(calc_norm<T>(bcentroid, norm_tmp));
    normtime.lap_stop();
    t.show("calc norm: ");
    disttime.lap_start();
    mat.data.mapv(kmeans_calc_sum_rowmajor<T>, bcentroid, bnorm, doccurrence,
                  dsum);
    disttime.lap_stop();
    t.show("calc_sum map: ");
    vecsumtime.lap_start();
    auto sum = dsum.vector_sum();
    auto occurrence = doccurrence.vector_sum();
    vecsumtime.lap_stop();
    t.show("calc_sum vector_sum: ");
    nextcentroidtime.lap_start();
    auto next_centroid = calc_new_centroid(sum, occurrence, dim);
    nextcentroidtime.lap_stop();
    t.show("calc next centroid: ");
    auto diff = is_diff_centroid(next_centroid, centroid, eps);
    centroid = std::move(next_centroid);
    t.show("diff centroid: ");
    t2.show("one iteration: ");
    if (!diff) break;
  }
  n_iter_ = i + 1;
  RLOG(DEBUG) << "converged in n_iter_: " << n_iter_ << std::endl;
  bcasttime.show_lap("broadcast centroid time: ");
  normtime.show_lap("normalize centroid time: ");
  disttime.show_lap("distributed computation time: ");
  vecsumtime.show_lap("vector sum time: ");
  nextcentroidtime.show_lap("calculate next centroid time: ");
  return centroid;
}

// MATRIX: any LOCAL matrix which supports operator* with rowmajor_matrix_local<T>
template <class MATRIX, class T>
rowmajor_matrix_local<T>
kmeans_partial_transform(MATRIX& mat,
                         rowmajor_matrix_local<T>& centroid) {
  time_spent t(DEBUG);
  auto prod = mat * centroid;
  t.show(" calc gemm/spmm: ");
  std::vector<T> norm;
  calc_norm_local(centroid, norm);
  t.show(" calc norm: ");
  norm_minus_product(norm, prod.val, prod.local_num_col, prod.local_num_row);
  t.show(" norm minus product: ");
  return prod; // 0.5 * b2 - ab
}

// MATRIX: any LOCAL matrix which supports operator* with rowmajor_matrix_local<T>
template <class MATRIX, class T>
rowmajor_matrix_local<T>
kmeans_transform(MATRIX& mat,
                 rowmajor_matrix_local<T>& centroid) {
  time_spent t(DEBUG);
  auto prod = mat * centroid;
  t.show(" calc gemm/spmm: ");
  std::vector<T> norm;
  calc_norm_local(centroid, norm);
  t.show(" calc norm: ");
  auto a2 = matrix_squared_sum(mat, 1); 
  t.show(" calc a2: ");
  calc_sq_euclidean_distance(a2, norm, prod.val, 
                             prod.local_num_col, prod.local_num_row);
  vector_sqrt_inplace(prod.val); // for eqclidean distance
  t.show(" calc euclidean distance: ");
  return prod; 
}

// MATRIX: any DISTRIBUTED matrix whose local version supports 
// operator* with rowmajor_matrix_local<T>
template <class MATRIX, class T>
rowmajor_matrix<T>
parallel_kmeans_transform(
  MATRIX& mat,
  rowmajor_matrix_local<T>& centroid) {
  rowmajor_matrix<T> ret(mat.data.map(kmeans_transform
                                      <typename MATRIX::local_mat_type, T>,
                                      frovedis::broadcast(centroid)));
  ret.num_row = mat.num_row;
  ret.num_col = centroid.local_num_col;
  return ret;
}

// MATRIX: any LOCAL matrix which supports operator* with rowmajor_matrix_local<T>
template <class MATRIX, class T>
std::vector<int> 
kmeans_assign_cluster(MATRIX& mat,
                      rowmajor_matrix_local<T>& centroid,
                      float& inertia,
                      bool need_inertia = true) {
  auto prod = kmeans_partial_transform(mat, centroid);
  time_spent t(DEBUG);
  auto nsamples = prod.local_num_row;
  std::vector<int> minloc(nsamples);
  std::vector<T> minval(nsamples);
  calc_minloc(minloc, minval, prod.val, prod.local_num_col, nsamples);
  t.show(" calc minloc: ");
  if (need_inertia) {
    inertia = 0.0;
    auto a2 = matrix_squared_sum(mat, 1);
    auto a2p = a2.data();
    auto minvalp = minval.data();
    // minvalp[r] = 0.5 * b2 - ab
    // 2 * minvalp[r] = b2 - 2ab
    // a2 + b2 - 2ab = a2 + 2 * minvalp[r]
    for(size_t r = 0; r < nsamples; r++) inertia += a2p[r] + 2 * minvalp[r];
    t.show(" calc inertia: ");
  }
  return minloc;
}

// for backward compatibility with previously released kmeans APIs
template <class MATRIX, class T>
std::vector<int> 
kmeans_assign_cluster(MATRIX& mat,
                      rowmajor_matrix_local<T>& centroid) {
  float inertia = 0.0f;
  bool need_inertia = false;
  return kmeans_assign_cluster(mat, centroid, inertia, need_inertia);
}

// for calling from inside map - used in parallel_kmeans_assign_cluster
template <class MATRIX, class T>
std::vector<int> 
invoke_kmeans_assign_cluster(MATRIX& mat,
                             rowmajor_matrix_local<T>& centroid,
                             float& inertia,
                             bool need_inertia = true) {
  return kmeans_assign_cluster(mat, centroid, inertia, need_inertia);
}

// MATRIX: any DISTRIBUTED matrix whose local version supports 
// operator* with rowmajor_matrix_local<T>
template <class MATRIX, class T>
std::vector<int>
parallel_kmeans_assign_cluster(
  MATRIX& mat,
  rowmajor_matrix_local<T>& centroid,
  float& inertia,
  bool need_inertia = true) {
  auto l_inertia = make_node_local_allocate<float>();
  auto lbl =  mat.data.map(invoke_kmeans_assign_cluster
                           <typename MATRIX::local_mat_type, T>,
                           broadcast(centroid),
                           l_inertia, broadcast(need_inertia))
                      .template moveto_dvector<int>()
                      .gather();
  if (need_inertia) inertia = l_inertia.reduce(add<float>);
  return lbl;
}

// for backward compatibility with previously released kmeans APIs
template <class MATRIX, class T>
std::vector<int>
parallel_kmeans_assign_cluster(
  MATRIX& mat,
  rowmajor_matrix_local<T>& centroid) {
  float inertia = 0.0f;
  bool need_inertia = false;
  return parallel_kmeans_assign_cluster(mat, centroid, inertia, need_inertia);
}

// MATRIX: any matrix which supports operator* with rowmajor_matrix_local<T>
template <class MATRIX>
rowmajor_matrix_local<typename MATRIX::value_type> 
kmeans(MATRIX& samples, int k, int iter,
       double eps, long seed, int n_init, 
       int& n_iter_, float& inertia,
       std::vector<int>& labels) {
  auto nsamples = samples.num_row;
  std::string msg = "kmeans: n_samples=" + STR(nsamples) +
                    " must be >= n_clusters=" + STR(k);
  require(nsamples >= k, msg);
  n_iter_ = 0;
  inertia = 0.0f;
  srand48(seed);
  auto centroid = kmeans_impl(samples, k, iter, eps, n_iter_);
  labels = parallel_kmeans_assign_cluster(samples, centroid, inertia);
  for (size_t i = 1; i < n_init; ++i) {
    int t_n_iter = 0;
    float t_inertia = 0.0f;
    auto t_centroid = kmeans_impl(samples, k, iter, eps, t_n_iter);
    auto t_labels = parallel_kmeans_assign_cluster(samples, t_centroid, 
                                                   t_inertia);
    if (t_inertia < inertia) {
      inertia = t_inertia;
      n_iter_ = t_n_iter;
      labels.swap(t_labels);
      centroid = std::move(t_centroid);
    }
  }
  return centroid;
}

// for backward compatibility with previously released kmeans APIs
template <class MATRIX>
rowmajor_matrix_local<typename MATRIX::value_type> 
kmeans(MATRIX& samples, int k, int iter,
       double eps, long seed = 0, int n_init = 1) {
  int n_iter_ = 0;
  float inertia = 0.0f;
  std::vector<int> labels;
  return kmeans(samples, k, iter, eps, seed, n_init, 
                n_iter_, inertia, labels);
}

}
#endif
