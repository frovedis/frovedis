#ifndef PCA_HPP
#define PCA_HPP

#include "truncated_svd.hpp"
#include <limits>

namespace frovedis {

// see https://stats.stackexchange.com/questions/134282/relationship-between-svd-and-pca-how-to-use-svd-to-perform-pca
template <class T>
void pca_helper(rowmajor_matrix<T>& mat_work, // destructed
                colmajor_matrix<T>& pca_directions, // components_ in sklearn
                colmajor_matrix<T>& pca_scores, // does not exist in sklearn
                std::vector<T>& eigen_values, // explained_variance_ in sklearn
                std::vector<T>& explained_variance_ratio, 
                std::vector<T>& singular_values,
                int k,
                bool to_standardize = false) {
  if(to_standardize) standardize(mat_work);
  else centerize(mat_work); 

  T var_sum = std::numeric_limits<T>::epsilon();
  auto total_var = variance(mat_work); // calculate before clear
  auto total_varp = total_var.data();
  for(size_t i = 0; i < total_var.size(); i++) var_sum += total_varp[i];
  size_t mat_num_row = mat_work.num_row;

  colmajor_matrix<T> u;
  diag_matrix_local<T> s;

  truncated_svd<colmajor_matrix<T>, colmajor_matrix_local<T>>
    (std::move(mat_work), u, s, pca_directions, k, true);

  pca_scores = u * s;

  eigen_values = s.val;
  T* valp = eigen_values.data();
  size_t size = eigen_values.size();
  T to_div = static_cast<T>(mat_num_row - 1);
  for(size_t i = 0; i < size; i++) {
    valp[i] = (valp[i] * valp[i]) / to_div;
  }

  // based on scikit-learn implementation
  // see https://github.com/scikit-learn/scikit-learn/blob/master/sklearn/decomposition/pca.py (_fit_truncated)
  explained_variance_ratio = eigen_values;
  T* explained_variance_ratiop = explained_variance_ratio.data();
  T to_mul = static_cast<T>(1) / var_sum;
  for(size_t i = 0; i < explained_variance_ratio.size(); i++) {
    explained_variance_ratiop[i] *= to_mul;
  }

  singular_values = s.val;
}

template <class T>
void pca(rowmajor_matrix<T>& mat,
         colmajor_matrix<T>& pca_directions,
         colmajor_matrix<T>& pca_scores,
         std::vector<T>& eigen_values,
         std::vector<T>& explained_variance_ratio,
         std::vector<T>& singular_values,
         int k,
         bool to_standardize = false) {
  auto mat_work = mat;
  pca_helper(mat_work, pca_directions, pca_scores, eigen_values,
             explained_variance_ratio, singular_values, k, to_standardize);
}

template <class T>
void pca(rowmajor_matrix<T>&& mat,
         colmajor_matrix<T>& pca_directions,
         colmajor_matrix<T>& pca_scores,
         std::vector<T>& eigen_values,
         std::vector<T>& explained_variance_ratio,
         std::vector<T>& singular_values,
         int k,
         bool to_standardize = false) {
  pca_helper(mat, pca_directions, pca_scores, eigen_values,
             explained_variance_ratio, singular_values, k, to_standardize);
}

}
#endif
