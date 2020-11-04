#ifndef KMEANS_HPP
#define KMEANS_HPP

#include "kmeans_utils.hpp"
#include "../../core/radix_sort.hpp"
#include "../../core/set_operations.hpp"

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
rowmajor_matrix_local<T> kmeans_impl(crs_matrix<T,I,O>& samples, int k,
                                     int iter, double eps, long seed = 0) {
  time_spent t(TRACE);
  auto centroid = get_random_rows(samples, k, seed);
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
  for(int i = 0; i < iter; i++) {
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
    if(is_diff_centroid(next_centroid, centroid, eps))
      centroid = next_centroid;
    else {
      RLOG(DEBUG) << "converged. total num iteration: " << i << std::endl;
      return next_centroid;
    }
    t.show("diff centroid: ");
    t2.show("one iteration: ");
  }
  RLOG(DEBUG) << "total num iteration: " << iter << std::endl;
  bcasttime.show_lap("broadcast centroid time: ");
  normtime.show_lap("normalize centroid time: ");
  disttime.show_lap("distributed computation time: ");
  vecsumtime.show_lap("vector sum time: ");
  nextcentroidtime.show_lap("calculate next centroid time: ");
  return centroid;
}

// TODO: mostly the same as the sparse version; use same code
template <class T>
rowmajor_matrix_local<T> kmeans_impl(rowmajor_matrix<T>& mat, int k,
                                     int iter, double eps, long seed = 0) {
  time_spent t(TRACE);
  auto centroid = get_random_rows(mat, k, seed);
  size_t dim = mat.num_col;
  t.show("create initial centroid: ");
  time_spent t2(TRACE);
  time_spent bcasttime(DEBUG), normtime(DEBUG), disttime(DEBUG), vecsumtime(DEBUG),
    nextcentroidtime(DEBUG);
  auto dsum = make_node_local_allocate<std::vector<T>>();
  auto doccurrence = make_node_local_allocate<std::vector<size_t>>();
  auto norm_tmp = make_node_local_allocate<std::vector<T>>();
  for(int i = 0; i < iter; i++) {
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
    if(is_diff_centroid(next_centroid, centroid, eps))
      centroid = next_centroid;
    else {
      RLOG(DEBUG) << "converged. total num iteration: " << i << std::endl;
      return next_centroid;
    }
    t.show("diff centroid: ");
    t2.show("one iteration: ");
  }
  RLOG(DEBUG) << "total num iteration: " << iter << std::endl;
  bcasttime.show_lap("broadcast centroid time: ");
  normtime.show_lap("normalize centroid time: ");
  disttime.show_lap("distributed computation time: ");
  vecsumtime.show_lap("vector sum time: ");
  nextcentroidtime.show_lap("calculate next centroid time: ");
  return centroid;
}

template <class T, class I, class O>
rowmajor_matrix_local<T> kmeans(crs_matrix<T,I,O>& samples, int k, int iter,
                                double eps, long seed = 0) {
  return kmeans_impl(samples, k, iter, eps, seed);
}

template <class T, class I, class O>
rowmajor_matrix_local<T> kmeans(crs_matrix<T,I,O>&& samples, int k, int iter,
                                double eps, long seed = 0) {
  return kmeans_impl(samples, k, iter, eps, seed);
}

template <class T>
rowmajor_matrix_local<T> kmeans(rowmajor_matrix<T>& samples, int k, int iter,
                                double eps, long seed = 0) {
  return kmeans_impl(samples, k, iter, eps, seed);
}

// mostly same as calc_sum
// used for assigning data to cluster
template <class T, class I, class O>
std::vector<int> kmeans_assign_cluster(crs_matrix_local<T,I,O>& mat,
                                       rowmajor_matrix_local<T>& centroid) {
  time_spent t(DEBUG);
  auto prod = mat * centroid;
  t.show(" spmm: ");
  size_t num_centroids = prod.local_num_col;
  size_t num_samples = prod.local_num_row;
  std::vector<T> norm;
  calc_norm_local(centroid, norm);
  T* normp = &norm[0];
  std::vector<int> minloc(num_samples);
  T* prodvalp = &prod.val[0];
  int* minlocp = &minloc[0];
  for(size_t c = 0; c < num_centroids; c++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t r = 0; r < num_samples; r++) {
      prodvalp[num_centroids * r + c] =
        normp[c] - prodvalp[num_centroids * r + c];
    }
  }
  t.show(" norm minus product: ");
  auto Tmax = std::numeric_limits<T>::max();
  std::vector<T> minval(num_samples);
  auto minvalp = minval.data();
  for(size_t r = 0; r < num_samples; r++) {
    minvalp[r] = Tmax;
    minlocp[r] = INT_MAX;
  }
  for(size_t c = 0; c < num_centroids; c++) {
    for(size_t r = 0; r < num_samples; r++) {
      if(prodvalp[num_centroids * r + c] < minvalp[r]) {
        minvalp[r] = prodvalp[num_centroids * r + c];
        minlocp[r] = c;
      }
    }
  }
  for(size_t r = 0; r < num_samples; r++) {
    if (minlocp[r] == INT_MAX)
      REPORT_ERROR(USER_ERROR, "Please check input matrix: it seems to have invalid entry!\n");
  }
  t.show(" calc minloc: ");
  return minloc;
}

template <class T>
std::vector<int> kmeans_assign_cluster(rowmajor_matrix_local<T>& mat,
                                       rowmajor_matrix_local<T>& centroid) {
  time_spent t(DEBUG);
  auto prod = mat * centroid;
  t.show(" gemm: ");
  size_t num_centroids = prod.local_num_col;
  size_t num_samples = prod.local_num_row;
  std::vector<T> norm;
  calc_norm_local(centroid, norm);
  T* normp = &norm[0];
  std::vector<int> minloc(num_samples);
  T* prodvalp = &prod.val[0];
  int* minlocp = &minloc[0];
  for(size_t c = 0; c < num_centroids; c++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t r = 0; r < num_samples; r++) {
      prodvalp[num_centroids * r + c] =
        normp[c] - prodvalp[num_centroids * r + c];
    }
  }
  t.show(" norm minus product: ");
  auto Tmax = std::numeric_limits<T>::max();
  std::vector<T> minval(num_samples);
  auto minvalp = minval.data();
  for(size_t r = 0; r < num_samples; r++) {
    minvalp[r] = Tmax;
    minlocp[r] = INT_MAX;
  }
  for(size_t c = 0; c < num_centroids; c++) {
    for(size_t r = 0; r < num_samples; r++) {
      if(prodvalp[num_centroids * r + c] < minvalp[r]) {
        minvalp[r] = prodvalp[num_centroids * r + c];
        minlocp[r] = c;
      }
    }
  }
  for(size_t r = 0; r < num_samples; r++) {
    if (minlocp[r] == INT_MAX)
      REPORT_ERROR(USER_ERROR, "Please check input matrix: it seems to have invalid entry!\n");
  }
  t.show(" calc minloc: ");
  return minloc;
}

}
#endif
