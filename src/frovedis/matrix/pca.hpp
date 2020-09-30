#ifndef PCA_HPP
#define PCA_HPP

#include <limits>
#include <cmath>
#include "truncated_svd.hpp"
#include "blas_wrapper.hpp"

namespace frovedis {

// whiten: https://github.com/scikit-learn/scikit-learn/blob/7813f7efb/sklearn/decomposition/pca.py#L363
// used for fit_transform_pca only -> frovedis : use for modifying pca_scores
// see https://stats.stackexchange.com/questions/134282/relationship-between-svd-and-pca-how-to-use-svd-to-perform-pca

template <class T>
void scale_colmajor_matrix(colmajor_matrix_local<T>& mat, 
                           double n){
  auto vec = mat.val.data();
  auto sz = mat.val.size();
  for(size_t i = 0; i < sz; i++) vec[i] *= n;
}

template <class T>
void pca_helper(rowmajor_matrix<T>& mat_work, // destructed
                colmajor_matrix<T>& pca_directions, // components_ in sklearn
                colmajor_matrix<T>& pca_scores, // does not exist in sklearn
                std::vector<T>& eigen_values, // explained_variance_ in sklearn
                std::vector<T>& explained_variance_ratio, 
                std::vector<T>& singular_values,
                std::vector<T>& mean,
                T& noise_variance,
                int k,
                bool whiten,
                bool is_movable,
                bool to_standardize = false) {
  mean = compute_mean(mat_work, 0); // column-wise mean
  if(to_standardize) standardize(mat_work, mean);
  else centerize(mat_work, mean); 

  T var_sum = std::numeric_limits<T>::epsilon();
  auto total_var = variance(mat_work); // calculate before clear
  auto total_varp = total_var.data();
  for(size_t i = 0; i < total_var.size(); i++) var_sum += total_varp[i];
  size_t mat_num_row = mat_work.num_row;
  size_t mat_num_col = mat_work.num_col;

  colmajor_matrix<T> u;
  diag_matrix_local<T> s;

  if(is_movable) {
    truncated_svd<colmajor_matrix<T>, colmajor_matrix_local<T>>
      (std::move(mat_work), u, s, pca_directions, k, true);
  }
  else {
    truncated_svd<colmajor_matrix<T>, colmajor_matrix_local<T>>
      (mat_work, u, s, pca_directions, k, true);
  }

  // whiten parameter
  if(whiten){
    auto scal = broadcast(sqrt(mat_num_row - 1));
    u.data.mapv(scale_colmajor_matrix<T>, scal);
    pca_scores = std::move(u); // u is no longer needed
  }
  else pca_scores = u * s;

  eigen_values = s.val;
  T exp_var_sum = std::numeric_limits<T>::epsilon();
  T* valp = eigen_values.data();
  size_t size = eigen_values.size();
  T to_div = static_cast<T>(mat_num_row - 1);
  for(size_t i = 0; i < size; i++) {
    valp[i] = (valp[i] * valp[i]) / to_div;
    exp_var_sum += valp[i];
  }

  // based on scikit-learn implementation
  // see https://github.com/scikit-learn/scikit-learn/blob/master/sklearn/decomposition/pca.py (_fit_truncated)
  explained_variance_ratio = eigen_values;
  T* explained_variance_ratiop = explained_variance_ratio.data();
  T to_mul = static_cast<T>(1) / var_sum;
  for(size_t i = 0; i < explained_variance_ratio.size(); i++) {
    explained_variance_ratiop[i] *= to_mul;
  }

  noise_variance = (var_sum - exp_var_sum);
  noise_variance /= std::min(mat_num_row, mat_num_col) - k;

  singular_values = s.val;
}

template <class T>
void pca(rowmajor_matrix<T>& mat,
         colmajor_matrix<T>& pca_directions,
         colmajor_matrix<T>& pca_scores,
         std::vector<T>& eigen_values,
         std::vector<T>& explained_variance_ratio,
         std::vector<T>& singular_values,
         std::vector<T>& mean,
         T& noise_variance,
         int k,
         bool whiten = false,
         bool to_standardize = false,
         bool to_copy = true) {
  if(to_copy) {
    auto mat_work = mat;
    auto movable = true; // lvalue input is copied
    pca_helper(mat_work, pca_directions, pca_scores, eigen_values,
               explained_variance_ratio, singular_values, mean, 
               noise_variance, k, whiten, movable, to_standardize);
  }
  else {
    auto movable = false; // lvalue input would get modified in-place
    pca_helper(mat, pca_directions, pca_scores, eigen_values,
               explained_variance_ratio, singular_values, mean, 
               noise_variance, k, whiten, movable, to_standardize);
  }
}

template <class T>
void pca(rowmajor_matrix<T>&& mat,
         colmajor_matrix<T>& pca_directions,
         colmajor_matrix<T>& pca_scores,
         std::vector<T>& eigen_values,
         std::vector<T>& explained_variance_ratio,
         std::vector<T>& singular_values,
         std::vector<T>& mean,
         T& noise_variance,
         int k,
         bool whiten = false,
         bool to_standardize = false) {
  auto movable = true; // rvalue input
  pca_helper(mat, pca_directions, pca_scores, eigen_values,
             explained_variance_ratio, singular_values, mean, 
             noise_variance, k, whiten, movable, to_standardize);
}

template <class T>
void add_mat_vec(rowmajor_matrix_local<T>& mat, 
                 std::vector<T>& vec) {
  auto datap = mat.val.data();
  auto vecp = vec.data();
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  auto sz = vec.size();
  checkAssumption(sz == ncol);
  for(size_t j = 0; j < ncol; j++){
    for(size_t i = 0; i < nrow; i++){ // nrow >> ncol
      size_t ind = i*ncol + j;
      datap[ind] += vecp[j];
    }
  }
}

template <class T>
rowmajor_matrix<T> transform_pca(rowmajor_matrix<T>& x,
                                 colmajor_matrix<T>& pca_directions,
                                 std::vector<T>& explained_variance,
                                 bool whiten = false) {
  auto comp_rmjr = pca_directions.to_rowmajor(); // TODO: avoid to_rowmajor()...
  auto ret = x * comp_rmjr;
  if(whiten) {
    auto size = explained_variance.size();
    std::vector<T> one_by_expvar(size);
    auto onebyp = one_by_expvar.data();
    auto expvarp = explained_variance.data();
    for(size_t i = 0; i < size; ++i) {
      if(expvarp[i] != 0.0) onebyp[i] = 1.0 / sqrt(expvarp[i]);
      else                  onebyp[i] = 1.0;
    }
    scale_matrix<T>(ret, one_by_expvar);
  }
  return ret;
}

template <class T>
rowmajor_matrix<T> transform_pca(rowmajor_matrix<T>& x,
                                 std::vector<T>& mean,
                                 colmajor_matrix<T>& pca_directions,
                                 std::vector<T>& explained_variance,
                                 bool whiten = false){
  centerize(x, mean); // inplace centerize
  return transform_pca(x, pca_directions, explained_variance, whiten);
}

template <class T>
rowmajor_matrix<T> inverse_transform_pca(rowmajor_matrix<T>& x,
                                         std::vector<T>& mean,
                                         colmajor_matrix<T>& pca_directions,
                                         std::vector<T>& explained_variance,
                                         bool whiten = false){
  // distributed implementation of:
  // np.sqrt(self.explained_variance_[:, np.newaxis]) * self.components_)
  // Note: self.components_ = pca_directions.transpose()
  auto comp_rmjr = pca_directions.to_rowmajor(); // TODO: avoid to_rowmajor()...
  if(whiten){
    auto sz = explained_variance.size();
    std::vector<T> explained_variance_sqrt(sz);
    auto expvar_sqrtp = explained_variance_sqrt.data();
    auto expvarp = explained_variance.data();
    for(size_t i = 0; i < sz; i++) {
      if(expvarp[i] != 0.0) expvar_sqrtp[i] = sqrt(expvarp[i]);
      else                  expvar_sqrtp[i] = 1.0;
    }
    scale_matrix<T>(comp_rmjr, explained_variance_sqrt);
  }
  auto ret = x * comp_rmjr.transpose();
  ret.data.mapv(add_mat_vec<T>, broadcast(mean));
  return ret;
}

}
#endif
