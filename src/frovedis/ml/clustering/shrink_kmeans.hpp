#ifndef SHRINK_KMEANS_HPP
#define SHRINK_KMEANS_HPP

#include "../../matrix/shrink_matrix.hpp"
#include "kmeans_utils.hpp"
#include "kmeans_impl.hpp" // resuses dense implementation

namespace frovedis {
namespace shrink {

template <class T, class I, class O>
rowmajor_matrix_local<T> get_random_rows(crs_matrix<T,I,O>& mat, int k,
                                         long seed, std::vector<T>& norm) {
  rowmajor_matrix_local<T> ret(mat.num_col, k);
  srand48(seed);
  for(int i = 0; i < k; i++) {
    int pos = static_cast<int>(drand48() * (mat.num_row - 1));
    auto sv = mat.get_row(pos);
    T each_norm = 0;
    auto retvalp = ret.val.data();
    auto svidxp = sv.idx.data();
    auto svvalp = sv.val.data();
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t j = 0; j < sv.val.size(); j++) {
      retvalp[k * svidxp[j] + i] = svvalp[j];
    }
    for(size_t j = 0; j < sv.val.size(); j++) {
      each_norm += svvalp[j] * svvalp[j] * 0.5;
    }
    norm[i] = each_norm;
  }
  return ret;
}

template <class T, class I, class O>
void kmeans_calc_sum_helper(rowmajor_matrix_local<T>& prod,
                            crs_matrix_local<T,I,O>& samples,
                            rowmajor_matrix_local<T>& centroid,
                            std::vector<T>& norm,
                            std::vector<size_t>& occurrence,
                            rowmajor_matrix_local<T>& sum) {
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

  sum.local_num_row = centroid.local_num_row; 
  sum.local_num_col = num_centroids;
  sum.val.clear();
  sum.val.resize(sum.local_num_row * sum.local_num_col);
  auto sump = sum.val.data();
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
                     rowmajor_matrix_local<T>& sum) {
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
                     rowmajor_matrix_local<T>& sum) {
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
                     rowmajor_matrix_local<T>& sum) {
  time_spent t(TRACE);
  auto prod = mat * centroid;
  t.show(" spmm (crs): ");
  kmeans_calc_sum_helper(prod, samples, centroid, norm, occurrence, sum);
}
#else
#error specify matrix format by defining JDS, CRS, or HYBRID
#endif

template <class T>
rowmajor_matrix_local<T>
calc_new_centroid_helper(rowmajor_matrix_local<T>& sum,
                         std::vector<size_t>& occurrence,
                         std::vector<T>& norm_tmp) {
  T* sump = &sum.val[0];
  size_t* occurrencep = &occurrence[0];
  size_t occurrencesize = occurrence.size();
  size_t non_zero = 0;
  for(size_t i = 0; i < occurrence.size(); i++) {
    if(occurrencep[i] > 0) non_zero++;
  }
  size_t local_dim = sum.local_num_row;
  rowmajor_matrix_local<T> ret(local_dim, non_zero);
  norm_tmp.clear();
  norm_tmp.resize(non_zero);
  T* retvalp = &ret.val[0];
#if defined(_SX) || defined(__ve__)
  size_t r = 0;
  for(size_t i = 0; i < occurrencesize; i++) {
    if(occurrencep[i] > 0) {
      for(size_t d = 0; d < local_dim; d++) {
        retvalp[non_zero * d + r] =
          sump[occurrencesize * d + i] / occurrencep[i];
      }
      r++;
    }
  }
#else
  for(size_t d = 0; d < local_dim; d++) {
    size_t r = 0;
    for(size_t i = 0; i < occurrencesize; i++) {
      if(occurrencep[i] > 0) {
        retvalp[non_zero * d + r] =
          sump[occurrencesize * d + i] / occurrencep[i];
        r++;
      }
    }
  }
#endif
  for(size_t i = 0; i < non_zero; i++) {
    T total = 0;
    for(size_t d = 0; d < local_dim; d++) {
      total += retvalp[non_zero * d + i] * retvalp[non_zero * d + i];
    }
    norm_tmp[i] = total * 0.5;
  }
  return ret;
}

template <class T>
rowmajor_matrix<T>
calc_new_centroid(rowmajor_matrix<T>& sum, std::vector<size_t>& occurrence,
                  node_local<std::vector<T>>& norm_tmp, size_t dim) {
  rowmajor_matrix<T> ret;
  ret.data = sum.data.map(calc_new_centroid_helper<T>, broadcast(occurrence),
                          norm_tmp);
  ret.num_row = dim;
  ret.num_col = ret.data.get_dvid().get_selfdata()->local_num_col;
  return ret;
}

template <class T, class I, class O>
rowmajor_matrix_local<T> kmeans_impl(crs_matrix<T,I,O>& samples, int k,
                                     int iter, double eps, long seed = 0,
                                     int& n_iter_ = 0) {
  time_spent t(TRACE);
  size_t dim = samples.num_col;
  std::vector<T> norm(k);
  auto lcentroid = get_random_rows(samples, k, seed, norm);
  auto tbl = shrink_column(samples);
  t.show("shrink_colum: ");
  auto info = prepare_shrink_comm(tbl, samples);
  t.show("prepare_shrink_comm: ");
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
  
  auto sizes = get_scatter_size(info);
  auto centroid = make_rowmajor_matrix_scatter(lcentroid, sizes);
  t.show("create initial centroid: ");
  auto bnorm = make_node_local_broadcast(norm);
  t.show("bcast norm: ");

  time_spent t2(TRACE);
  time_spent bcasttime(DEBUG), normtime(DEBUG), disttime(DEBUG),
    vecsumtime(DEBUG), nextcentroidtime(DEBUG);
  
  auto dsum = make_node_local_allocate<rowmajor_matrix_local<T>>();
  auto doccurrence = make_node_local_allocate<std::vector<size_t>>();
  int i;
  for(i = 0; i < iter; i++) {
    RLOG(TRACE) << "num centroids: " << centroid.num_col << std::endl;
    bcasttime.lap_start();
    auto bcentroid = shrink_rowmajor_matrix_bcast(centroid, info);
    bcasttime.lap_stop();
    t.show("bcast centroid: ");
    disttime.lap_start();
    mat.data.mapv(kmeans_calc_sum<T,I,O>, samples.data, bcentroid, bnorm, 
                  doccurrence, dsum);
    disttime.lap_stop();
    t.show("calc_sum map: ");
    vecsumtime.lap_start();
    auto sum = shrink_rowmajor_matrix_sum(dsum, info);
    auto occurrence = doccurrence.vector_sum();
    vecsumtime.lap_stop();
    t.show("calc_sum vector_sum: ");
    nextcentroidtime.lap_start();
    auto norm_tmp = make_node_local_allocate<std::vector<T>>();
    auto next_centroid = calc_new_centroid(sum, occurrence, norm_tmp, dim);
    nextcentroidtime.lap_stop();
    t.show("calc next centroid: ");
    normtime.lap_start();
    bnorm = broadcast(norm_tmp.vector_sum());
    normtime.lap_stop();
    t.show("bcast norm: ");
    if(is_diff_centroid(next_centroid, centroid, eps))
      centroid = next_centroid;
    else {
      RLOG(DEBUG) << "converged at iteration: " << i << std::endl;
      centroid = next_centroid;
      break;
    }
    t.show("diff centroid: ");
    t2.show("one iteration: ");
  }
  RLOG(DEBUG) << "total num iteration: " << i << std::endl;
  bcasttime.show_lap("broadcast centroid time: ");
  normtime.show_lap("normalize centroid time: ");
  disttime.show_lap("distributed computation time: ");
  vecsumtime.show_lap("vector sum time: ");
  nextcentroidtime.show_lap("calculate next centroid time: ");
  n_iter_ = i + 1;
  return centroid.gather();
}

template <class T, class I, class O>
rowmajor_matrix_local<T> kmeans(crs_matrix<T,I,O>& samples, int k, int iter,
                                double eps, long seed, int& n_iter_) {
  return frovedis::shrink::kmeans_impl(samples, k, iter, eps, seed, n_iter_);
}

template <class T, class I, class O>
rowmajor_matrix_local<T> kmeans(crs_matrix<T,I,O>&& samples, int k, int iter,
                                double eps, long seed, int& n_iter_) {
  return frovedis::shrink::kmeans_impl(samples, k, iter, eps, seed, n_iter_);
}

template <class T>
rowmajor_matrix_local<T> kmeans(rowmajor_matrix<T>& samples, int k, int iter,
                                double eps, long seed, int& n_iter_) {
  std::string msg = "shrink version is supported only for sparse data!\n";
  msg += "dense data is detected! by-passing the call to non-shrink version!.\n";
  REPORT_WARNING(WARNING_MESSAGE, msg);
  return frovedis::kmeans_impl(samples, k, iter, eps, seed, n_iter_);
}

// for backward compatibility with previously released shrink::kmeans APIs
template <class T, class I, class O>
rowmajor_matrix_local<T> kmeans(crs_matrix<T,I,O>& samples, int k, int iter,
                                double eps, long seed = 0) {
  int n_iter_ = 0;
  return frovedis::shrink::kmeans_impl(samples, k, iter, eps, seed, n_iter_);
}

template <class T, class I, class O>
rowmajor_matrix_local<T> kmeans(crs_matrix<T,I,O>&& samples, int k, int iter,
                                double eps, long seed = 0) {
  int n_iter_ = 0;
  return frovedis::shrink::kmeans_impl(samples, k, iter, eps, seed, n_iter_);
}

template <class T>
rowmajor_matrix_local<T> kmeans(rowmajor_matrix<T>& samples, int k, int iter,
                                double eps, long seed = 0) {
  int n_iter_ = 0;
  return frovedis::shrink::kmeans(samples, k, iter, eps, seed, n_iter_);
}

} // end of namespace shrink
} // end of namespace frovedis
#endif
