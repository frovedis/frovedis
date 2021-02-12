#ifndef KMEANS_UTILS
#define KMEANS_UTILS

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

#define KMEANS_VLEN 4096

namespace frovedis {

template <class T, class I, class O>
rowmajor_matrix_local<T> 
get_random_rows(crs_matrix<T,I,O>& mat, int k) {
  rowmajor_matrix_local<T> ret(mat.num_col, k);
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

template <class T, class I, class O>
rowmajor_matrix_local<T> 
get_random_rows(crs_matrix<T,I,O>& mat, int k, long seed) {
  srand48(seed);
  return get_random_rows(mat, k);
}

template <class T>
rowmajor_matrix_local<T> 
get_random_rows(rowmajor_matrix<T>& mat, int k) {
  size_t num_col = mat.num_col;
  size_t num_row = mat.num_row;
  rowmajor_matrix_local<T> ret(num_col, k); // transposed
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
rowmajor_matrix_local<T> 
get_random_rows(rowmajor_matrix<T>& mat, int k, long seed) {
  srand48(seed);
  return get_random_rows(mat, k);
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
#pragma _NEC novector
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
void calc_norm_local(rowmajor_matrix_local<T>& centroid,
                     std::vector<T>& ret) {
  size_t num_centroids = centroid.local_num_col;
  size_t dim = centroid.local_num_row;
  ret.clear();
  ret.resize(num_centroids);
  T* retp = &ret[0];
  T* valp = &centroid.val[0];
#pragma cdir novector // should be small
#pragma _NEC novector
  for(size_t k = 0; k < num_centroids; k++) {
#pragma cdir vector
    for(size_t i = 0; i < dim; i++) {
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

template <class T>
T calc_centroid_diff(rowmajor_matrix_local<T>& a,
                     rowmajor_matrix_local<T>& b) {
  return vector_ssd(a.val, b.val);
}

template <class T>
bool is_diff_centroid(rowmajor_matrix_local<T>& a,
                      rowmajor_matrix_local<T>& b, 
                      double eps) {
  if(a.val.size() != b.val.size()) 
    return true;
  else 
    return calc_centroid_diff(a, b) > eps;
}

template <class T>
bool is_diff_centroid(rowmajor_matrix<T>& a,
                      rowmajor_matrix<T>& b, 
                      double eps) {
  if(a.num_col != b.num_col) return true;
  else {
    auto error = a.data.map(calc_centroid_diff<T>, b.data)
                       .reduce(add<T>);
    return error > eps;
  }
}

template <class T>
void norm_minus_product(std::vector<T>& norm, std::vector<T>& prodval,
                        size_t num_centroids, size_t num_samples) {
  auto normp = norm.data();
  auto prodvalp = prodval.data();
#ifdef __ve__
  size_t c = 0;
  for(; c+7 < num_centroids; c+=8) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t r = 0; r < num_samples; r++) {
      prodvalp[num_centroids * r + c] =
        normp[c] - prodvalp[num_centroids * r + c];
      prodvalp[num_centroids * r + c + 1] =
        normp[c+1] - prodvalp[num_centroids * r + c + 1];
      prodvalp[num_centroids * r + c + 2] =
        normp[c+2] - prodvalp[num_centroids * r + c + 2];
      prodvalp[num_centroids * r + c + 3] =
        normp[c+3] - prodvalp[num_centroids * r + c + 3];
      prodvalp[num_centroids * r + c + 4] =
        normp[c+4] - prodvalp[num_centroids * r + c + 4];
      prodvalp[num_centroids * r + c + 5] =
        normp[c+5] - prodvalp[num_centroids * r + c + 5];
      prodvalp[num_centroids * r + c + 6] =
        normp[c+6] - prodvalp[num_centroids * r + c + 6];
      prodvalp[num_centroids * r + c + 7] =
        normp[c+7] - prodvalp[num_centroids * r + c + 7];
    }
  }
  for(; c+3 < num_centroids; c+=4) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t r = 0; r < num_samples; r++) {
      prodvalp[num_centroids * r + c] =
        normp[c] - prodvalp[num_centroids * r + c];
      prodvalp[num_centroids * r + c + 1] =
        normp[c+1] - prodvalp[num_centroids * r + c + 1];
      prodvalp[num_centroids * r + c + 2] =
        normp[c+2] - prodvalp[num_centroids * r + c + 2];
      prodvalp[num_centroids * r + c + 3] =
        normp[c+3] - prodvalp[num_centroids * r + c + 3];
    }
  }
  for(; c < num_centroids; c++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t r = 0; r < num_samples; r++) {
      prodvalp[num_centroids * r + c] =
        normp[c] - prodvalp[num_centroids * r + c];
    }
  }
#else
  for(size_t r = 0; r < num_samples; r++) {
    for(size_t c = 0; c < num_centroids; c++) {
      prodvalp[num_centroids * r + c] =
        normp[c] - prodvalp[num_centroids * r + c];
    }
  }
#endif
}                        

template <class T>
void calc_sq_euclidean_distance(std::vector<T>& a2, 
                                std::vector<T>& norm, 
                                std::vector<T>& prodval,
                                size_t num_centroids, 
                                size_t num_samples) {
  auto a2p = a2.data();           // a2
  auto normp = norm.data();       // 0.5 * b2
  auto prodvalp = prodval.data(); // ab
  // computes: a2 + 2.0 * (0.5 * b2 - ab) -> a2 + b2 - 2ab
#ifdef __ve__
  size_t c = 0;
  for(; c+7 < num_centroids; c+=8) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t r = 0; r < num_samples; r++) {
      prodvalp[num_centroids * r + c] =
        a2p[r] + 2.0 * (normp[c] - prodvalp[num_centroids * r + c]);
      prodvalp[num_centroids * r + c + 1] =
        a2p[r] + 2.0 * (normp[c+1] - prodvalp[num_centroids * r + c + 1]);
      prodvalp[num_centroids * r + c + 2] =
        a2p[r] + 2.0 * (normp[c+2] - prodvalp[num_centroids * r + c + 2]);
      prodvalp[num_centroids * r + c + 3] =
        a2p[r] + 2.0 * (normp[c+3] - prodvalp[num_centroids * r + c + 3]);
      prodvalp[num_centroids * r + c + 4] =
        a2p[r] + 2.0 * (normp[c+4] - prodvalp[num_centroids * r + c + 4]);
      prodvalp[num_centroids * r + c + 5] =
        a2p[r] + 2.0 * (normp[c+5] - prodvalp[num_centroids * r + c + 5]);
      prodvalp[num_centroids * r + c + 6] =
        a2p[r] + 2.0 * (normp[c+6] - prodvalp[num_centroids * r + c + 6]);
      prodvalp[num_centroids * r + c + 7] =
        a2p[r] + 2.0 * (normp[c+7] - prodvalp[num_centroids * r + c + 7]);
    }
  }
  for(; c+3 < num_centroids; c+=4) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t r = 0; r < num_samples; r++) {
      prodvalp[num_centroids * r + c] =
        a2p[r] + 2.0 * (normp[c] - prodvalp[num_centroids * r + c]);
      prodvalp[num_centroids * r + c + 1] =
        a2p[r] + 2.0 * (normp[c+1] - prodvalp[num_centroids * r + c + 1]);
      prodvalp[num_centroids * r + c + 2] =
        a2p[r] + 2.0 * (normp[c+2] - prodvalp[num_centroids * r + c + 2]);
      prodvalp[num_centroids * r + c + 3] =
        a2p[r] + 2.0 * (normp[c+3] - prodvalp[num_centroids * r + c + 3]);
    }
  }
  for(; c < num_centroids; c++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t r = 0; r < num_samples; r++) {
      prodvalp[num_centroids * r + c] =
        a2p[r] + 2.0 * (normp[c] - prodvalp[num_centroids * r + c]);
    }
  }
#else
  for(size_t r = 0; r < num_samples; r++) {
    for(size_t c = 0; c < num_centroids; c++) {
      prodvalp[num_centroids * r + c] =
        a2p[r] + 2.0 * (normp[c] - prodvalp[num_centroids * r + c]);
    }
  }
#endif
}

template <class T>
void calc_minloc(std::vector<int>& minloc, std::vector<T>& minval,
                 std::vector<T>& prodval,
                 size_t num_centroids, size_t num_samples) {
  auto minlocp = minloc.data();
  auto minvalp = minval.data();
  auto prodvalp = prodval.data();
  auto Tmax = std::numeric_limits<T>::max();
  for(size_t r = 0; r < num_samples; r++) {
    minvalp[r] = Tmax;
    minlocp[r] = INT_MAX;
  }
#ifdef __ve__
  size_t c = 0;
  for(; c+7 < num_centroids; c+=8) {
    for(size_t r = 0; r < num_samples; r++) {
      auto minvalpr = minvalp[r];
      size_t minlocpr = minlocp[r];
      if(prodvalp[num_centroids * r + c] < minvalpr) {
        minvalpr = prodvalp[num_centroids * r + c];
        minlocpr = c;
      }
      if(prodvalp[num_centroids * r + c + 1] < minvalpr) {
        minvalpr = prodvalp[num_centroids * r + c + 1];
        minlocpr = c + 1;
      }
      if(prodvalp[num_centroids * r + c + 2] < minvalpr) {
        minvalpr = prodvalp[num_centroids * r + c + 2];
        minlocpr = c + 2;
      }
      if(prodvalp[num_centroids * r + c + 3] < minvalpr) {
        minvalpr = prodvalp[num_centroids * r + c + 3];
        minlocpr = c + 3;
      }
      if(prodvalp[num_centroids * r + c + 4] < minvalpr) {
        minvalpr = prodvalp[num_centroids * r + c + 4];
        minlocpr = c + 4;
      }
      if(prodvalp[num_centroids * r + c + 5] < minvalpr) {
        minvalpr = prodvalp[num_centroids * r + c + 5];
        minlocpr = c + 5;
      }
      if(prodvalp[num_centroids * r + c + 6] < minvalpr) {
        minvalpr = prodvalp[num_centroids * r + c + 6];
        minlocpr = c + 6;
      }
      if(prodvalp[num_centroids * r + c + 7] < minvalpr) {
        minvalpr = prodvalp[num_centroids * r + c + 7];
        minlocpr = c + 7;
      }
      minvalp[r] = minvalpr;
      minlocp[r] = minlocpr;
    }
  }
  for(; c+3 < num_centroids; c+=4) {
    for(size_t r = 0; r < num_samples; r++) {
      auto minvalpr = minvalp[r];
      size_t minlocpr = minlocp[r];
      if(prodvalp[num_centroids * r + c] < minvalpr) {
        minvalpr = prodvalp[num_centroids * r + c];
        minlocpr = c;
      }
      if(prodvalp[num_centroids * r + c + 1] < minvalpr) {
        minvalpr = prodvalp[num_centroids * r + c + 1];
        minlocpr = c + 1;
      }
      if(prodvalp[num_centroids * r + c + 2] < minvalpr) {
        minvalpr = prodvalp[num_centroids * r + c + 2];
        minlocpr = c + 2;
      }
      if(prodvalp[num_centroids * r + c + 3] < minvalpr) {
        minvalpr = prodvalp[num_centroids * r + c + 3];
        minlocpr = c + 3;
      }
      minvalp[r] = minvalpr;
      minlocp[r] = minlocpr;
    }
  }
  for(; c < num_centroids; c++) {
    for(size_t r = 0; r < num_samples; r++) {
      if(prodvalp[num_centroids * r + c] < minvalp[r]) {
        minvalp[r] = prodvalp[num_centroids * r + c];
        minlocp[r] = c;
      }
    }
  }
#else
  for(size_t r = 0; r < num_samples; r++) {
    for(size_t c = 0; c < num_centroids; c++) {
      if(prodvalp[num_centroids * r + c] < minvalp[r]) {
        minvalp[r] = prodvalp[num_centroids * r + c];
        minlocp[r] = c;
      }
    }
  }
#endif
  for(size_t r = 0; r < num_samples; r++) {
    if (minlocp[r] == INT_MAX)
      REPORT_ERROR(USER_ERROR, "Please check input matrix: it seems to have invalid entry!\n");
  }
  // segfault might occur if not checked above case
}

template <class T>
void calc_minloc(std::vector<int>& minloc, 
                 std::vector<T>& prodval,
                 size_t num_centroids, size_t num_samples) {
  std::vector<T> minval(num_samples);
  calc_minloc(minloc, minval, prodval, num_centroids, num_samples);
}

inline
void calc_occurrence(std::vector<size_t>& occurrence, int* minlocp,
                     size_t num_centroids, size_t num_samples) {
  occurrence.clear();
  occurrence.resize(num_centroids);
  size_t* occurrencep = occurrence.data();
#ifdef __ve__
  std::vector<size_t> hist(num_centroids*KMEANS_VLEN);
  auto histp = hist.data();
  size_t r = 0;
  for(; r+KMEANS_VLEN-1 < num_samples; r+=KMEANS_VLEN) {
#pragma _NEC ivdep
    for(size_t i = 0; i < KMEANS_VLEN; i++) {
      histp[i * num_centroids + minlocp[r+i]]++;
    }
  }
  for(; r < num_samples; r++) {
    occurrencep[minlocp[r]]++;
  }
  for(size_t i = 0; i < KMEANS_VLEN; i++) {
#pragma _NEC ivdep
    for(size_t j = 0; j < num_centroids; j++) {
      occurrencep[j] += histp[i * num_centroids + j];
    }
  }
#else
  for(size_t r = 0; r < num_samples; r++) occurrencep[minlocp[r]]++;
#endif
}

}
#endif
