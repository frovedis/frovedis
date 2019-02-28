#ifndef KMEANS_HPP
#define KMEANS_HPP

#include <cfloat>
#include <climits>

#include "../../matrix/jds_crs_hybrid.hpp"
#include "../../matrix/rowmajor_matrix.hpp"
#include "../../matrix/blas_wrapper.hpp" // for operator*
#include "../../core/utility.hpp"

#if !defined(KMEANS_JDS) && !defined(KMEANS_CRS) && !defined(KMEANS_HYBRID)
#if defined(_SX) || defined(__ve__)
#define KMEANS_HYBRID
#else
#define KMEANS_CRS
#endif
#endif

namespace frovedis {

template <class T, class I, class O>
rowmajor_matrix_local<T> get_random_rows(crs_matrix<T,I,O>& mat, int k,
                                         long seed) {
  rowmajor_matrix_local<T> ret(mat.num_col, k);
  srand48(seed);
  for(int i = 0; i < k; i++) {
    int pos = static_cast<int>(drand48() * (mat.num_row - 1));
    auto sv = mat.get_row(pos);
    auto retvalp = ret.val.data();
    auto svidxp = sv.idx.data();
    auto svvalp = sv.val.data();
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t j = 0; j < sv.val.size(); j++) {
      retvalp[k * svidxp[j] + i] = svvalp[j];
    }
  }
  return ret;
}

template <class T>
rowmajor_matrix_local<T> get_random_rows(rowmajor_matrix<T>& mat, int k,
                                         long seed) {
  size_t num_col = mat.num_col;
  size_t num_row = mat.num_row;
  rowmajor_matrix_local<T> ret(num_col, k); // transposed
  srand48(seed);
  for(int i = 0; i < k; i++) {
    int pos = static_cast<int>(drand48() * (num_row - 1));
    auto v = mat.get_row(pos);
    auto retvalp = ret.val.data();
    auto vp = v.data();
    for(size_t j = 0; j < num_col; j++) {
      retvalp[k * j + i] = vp[j];
    }
  }
  return ret;
}

template <class T>
void calc_norm_helper(rowmajor_matrix_local<T>& centroid,
                      std::vector<T>& ret) {
  size_t num_centroids = centroid.local_num_col;
  size_t dim = centroid.local_num_row;
  ret.clear();
  ret.resize(num_centroids);
  T* retp = &ret[0];
  T* valp = &centroid.val[0];
  auto each = ceil_div(dim, static_cast<size_t>(get_nodesize()));
  auto mystart = each * get_selfid();
  auto myend = std::min(dim, each * (get_selfid() + 1));
#pragma cdir novector // should be small
  for(size_t k = 0; k < num_centroids; k++) {
#pragma cdir vector
    for(size_t i = mystart; i < myend; i++) {
      // x^2 + y^2 - 2xy; later, xy is used, so y^2 * 0.5 is used as norm
      retp[k] +=
        valp[num_centroids * i + k] * valp[num_centroids * i + k] * 0.5;
    }
  }
}

template <class T>
std::vector<T> calc_norm(node_local<rowmajor_matrix_local<T>>& bcentroid,
                         node_local<std::vector<T>>& norm_tmp) {
  bcentroid.mapv(calc_norm_helper<T>, norm_tmp);
  return norm_tmp.vector_sum();
}

#ifdef KMEANS_HYBRID
template <class T, class I, class O>
void kmeans_calc_sum(jds_crs_hybrid_local<T,I,O>& mat,
                     rowmajor_matrix_local<T>& centroid,
                     std::vector<T>& norm,
                     std::vector<size_t>& occurrence,
                     std::vector<T>& sum) {
  time_spent t(TRACE);
  auto prod = mat * centroid;
  t.show(" spmm (hybrid): ");
  size_t num_centroids = prod.local_num_col;
  size_t num_samples = prod.local_num_row;
  std::vector<int> minloc(num_samples);
  T* prodvalp = &prod.val[0];
  T* normp = &norm[0];
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
  occurrence.clear();
  occurrence.resize(num_centroids);
  size_t* occurrencep = &occurrence[0];
  for(size_t r = 0; r < num_samples; r++) {
    T min = DBL_MAX;
    int pos = INT_MAX;
    for(size_t c = 0; c < num_centroids; c++) {
      if(prodvalp[num_centroids * r + c] < min) {
        min = prodvalp[num_centroids * r + c];
        pos = c;
      }
    }
    minlocp[r] = pos;
    occurrencep[pos]++;
  }
  t.show(" calc minloc: ");
  size_t dim = mat.local_num_col;
  sum.clear();
  sum.resize(dim * num_centroids); // rowmajor dim x k
  T* sump = &sum[0];
  T* jdsvalp = &mat.jds.val[0];
  I* jdsidxp = &mat.jds.idx[0];
  O* jdsoffp = &mat.jds.off[0];
  O* jdspermp = &mat.jds.perm[0];
  std::vector<int> permminloc(minloc.size());
  int* permminlocp = &permminloc[0];
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < permminloc.size(); i++) {
    permminlocp[i] = minlocp[jdspermp[i]];
  }
  for(size_t pc = 0; pc < mat.jds.off.size() - 1; pc++) {
    size_t r = 0;
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t o = jdsoffp[pc]; o < jdsoffp[pc+1]; o++, r++) {
      sump[jdsidxp[o] * num_centroids + permminlocp[r]] += jdsvalp[o];
    }
  }
  T* crsvalp = &mat.crs.val[0];
  I* crsidxp = &mat.crs.idx[0];
  O* crsoffp = &mat.crs.off[0];
  for(size_t r = 0; r < mat.crs.local_num_row; r++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t c = crsoffp[r]; c < crsoffp[r+1]; c++) {
      sump[crsidxp[c] * num_centroids + permminlocp[r]] += crsvalp[c];
    }
  }
  t.show(" calc sum: ");
}
#elif defined KMEANS_JDS
template <class T, class I, class O>
void kmeans_calc_sum(jds_matrix_local<T,I,O>& mat,
                     rowmajor_matrix_local<T>& centroid,
                     std::vector<T>& norm,
                     std::vector<size_t>& occurrence,
                     std::vector<T>& sum) {
  time_spent t(TRACE);
  auto prod = mat * centroid;
  t.show(" spmm (jds): ");
  size_t num_centroids = prod.local_num_col;
  size_t num_samples = prod.local_num_row;
  std::vector<int> minloc(num_samples);
  T* prodvalp = &prod.val[0];
  T* normp = &norm[0];
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
  occurrence.clear();
  occurrence.resize(num_centroids);
  size_t* occurrencep = &occurrence[0];
  for(size_t r = 0; r < num_samples; r++) {
    T min = DBL_MAX;
    int pos = INT_MAX;
    for(size_t c = 0; c < num_centroids; c++) {
      if(prodvalp[num_centroids * r + c] < min) {
        min = prodvalp[num_centroids * r + c];
        pos = c;
      }
    }
    minlocp[r] = pos;
    occurrencep[pos]++;
  }
  t.show(" calc minloc: ");
  size_t dim = mat.local_num_col;
  sum.clear();
  sum.resize(dim * num_centroids); // rowmajor dim x k
  T* sump = &sum[0];
  T* valp = &mat.val[0];
  I* idxp = &mat.idx[0];
  O* offp = &mat.off[0];
  O* permp = &mat.perm[0];
  std::vector<int> permminloc(minloc.size());
  int* permminlocp = &permminloc[0];
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < permminloc.size(); i++) {
    permminlocp[i] = minlocp[permp[i]];
  }
  for(size_t pc = 0; pc < mat.off.size() - 1; pc++) {
    size_t r = 0;
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t o = offp[pc]; o < offp[pc+1]; o++, r++) {
      sump[idxp[o] * num_centroids + permminlocp[r]] += valp[o];
    }
  }
  t.show(" calc sum: ");
}
#elif defined KMEANS_CRS
template <class T, class I, class O>
void kmeans_calc_sum(crs_matrix_local<T,I,O>& mat,
                     rowmajor_matrix_local<T>& centroid,
                     std::vector<T>& norm,
                     std::vector<size_t>& occurrence,
                     std::vector<T>& sum) {
  time_spent t(TRACE);
  auto prod = mat * centroid;
  t.show(" spmm (crs): ");
  size_t num_centroids = prod.local_num_col;
  size_t num_samples = prod.local_num_row;
  std::vector<int> minloc(num_samples);
  T* prodvalp = &prod.val[0];
  T* normp = &norm[0];
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
  occurrence.clear();
  occurrence.resize(num_centroids);
  size_t* occurrencep = &occurrence[0];
  for(size_t r = 0; r < num_samples; r++) {
    T min = DBL_MAX;
    int pos = INT_MAX;
    for(size_t c = 0; c < num_centroids; c++) {
      if(prodvalp[num_centroids * r + c] < min) {
        min = prodvalp[num_centroids * r + c];
        pos = c;
      }
    }
    minlocp[r] = pos;
    occurrencep[pos]++;
  }
  t.show(" calc minloc: ");
  size_t dim = mat.local_num_col;
  sum.clear();
  sum.resize(dim * num_centroids); // rowmajor dim x k
  T* sump = &sum[0];
  T* valp = &mat.val[0];
  I* idxp = &mat.idx[0];
  O* offp = &mat.off[0];
  for(size_t r = 0; r < num_samples; r++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t c = offp[r]; c < offp[r+1]; c++) {
      sump[idxp[c] * num_centroids + minlocp[r]] += valp[c];
    }
  }
  t.show(" calc sum: ");
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
  T* prodvalp = &prod.val[0];
  T* normp = &norm[0];
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
  occurrence.clear();
  occurrence.resize(num_centroids);
  size_t* occurrencep = &occurrence[0];
  for(size_t r = 0; r < num_samples; r++) {
    T min = DBL_MAX;
    int pos = INT_MAX;
    for(size_t c = 0; c < num_centroids; c++) {
      if(prodvalp[num_centroids * r + c] < min) {
        min = prodvalp[num_centroids * r + c];
        pos = c;
      }
    }
    minlocp[r] = pos;
    occurrencep[pos]++;
  }
  t.show(" calc minloc: ");
  size_t dim = mat.local_num_col;
  sum.clear();
  sum.resize(dim * num_centroids); // rowmajor dim x k
  T* sump = &sum[0];
  T* valp = &mat.val[0];
  for(size_t r = 0; r < num_samples; r++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t c = 0; c < dim; c++) {
      sump[num_centroids * c + minlocp[r]] += valp[dim * r + c];
    }
  }
  t.show(" calc sum: ");
}

template <class T>
rowmajor_matrix_local<T>
calc_new_centroid(std::vector<T>& sum, std::vector<size_t>& occurrence,
                  size_t dim) {
  T* sump = &sum[0];
  size_t* occurrencep = &occurrence[0];
  size_t occurrencesize = occurrence.size();
  size_t non_zero = 0;
  for(size_t i = 0; i < occurrence.size(); i++) {
    if(occurrencep[i] > 0) non_zero++;
  }
  rowmajor_matrix_local<T> ret(dim, non_zero);
  T* retvalp = &ret.val[0];
#if defined(_SX) || defined(__ve__)
  size_t r = 0;
  for(size_t i = 0; i < occurrencesize; i++) {
    if(occurrencep[i] > 0) {
      for(size_t d = 0; d < dim; d++) {
        retvalp[non_zero * d + r] =
          sump[occurrencesize * d + i] / occurrencep[i];
      }
      r++;
    }
  }
#else
  for(size_t d = 0; d < dim; d++) {
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
  return ret;
}

template <class T>
bool is_diff_centroid(rowmajor_matrix_local<T>& a,
                      rowmajor_matrix_local<T>& b, double eps) {
  if(a.val.size() != b.val.size()) return true;
  else {
    double error = 0;
    auto avalp = a.val.data();
    auto bvalp = b.val.data();
    for(size_t i = 0; i < a.val.size(); i++) {
      error += (avalp[i] - bvalp[i]) * (avalp[i] - bvalp[i]);
    }
    if(error > eps) return true;
    else return false;
  }
}

template <class T, class I, class O>
rowmajor_matrix_local<T> kmeans_impl(crs_matrix<T,I,O>& samples, int k,
                                     int iter, double eps, long seed = 0,
                                     bool input_removable = false) {
  time_spent t(TRACE);
  auto centroid = get_random_rows(samples, k, seed);
  size_t dim = samples.num_col;
  t.show("create initial centroid: ");
#ifdef KMEANS_HYBRID
  jds_crs_hybrid<T,I,O,O> mat(samples);
  if(input_removable) samples.clear();
  t.show("convert matrix (hybrid): ");
#elif defined KMEANS_JDS
  jds_matrix<T,I,O,O> mat(samples);
  if(input_removable) samples.clear();
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
    mat.data.mapv(kmeans_calc_sum<T,I,O>, bcentroid, bnorm, doccurrence, dsum);
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
  return kmeans_impl(samples, k, iter, eps, seed, false);
}

template <class T, class I, class O>
rowmajor_matrix_local<T> kmeans(crs_matrix<T,I,O>&& samples, int k, int iter,
                                double eps, long seed = 0) {
  return kmeans_impl(samples, k, iter, eps, seed, true);
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
  calc_norm_helper(centroid, norm);
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
  for(size_t r = 0; r < num_samples; r++) {
    T min = DBL_MAX;
    int pos = INT_MAX;
    for(size_t c = 0; c < num_centroids; c++) {
      if(prodvalp[num_centroids * r + c] < min) {
        min = prodvalp[num_centroids * r + c];
        pos = c;
      }
    }
    minlocp[r] = pos;
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
  calc_norm_helper(centroid, norm);
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
  for(size_t r = 0; r < num_samples; r++) {
    T min = DBL_MAX;
    int pos = INT_MAX;
    for(size_t c = 0; c < num_centroids; c++) {
      if(prodvalp[num_centroids * r + c] < min) {
        min = prodvalp[num_centroids * r + c];
        pos = c;
      }
    }
    minlocp[r] = pos;
  }
  t.show(" calc minloc: ");
  return minloc;
}

}
#endif
