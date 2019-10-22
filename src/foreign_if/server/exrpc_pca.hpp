#ifndef _EXRPC_PCA_HPP_
#define _EXRPC_PCA_HPP_

#include "frovedis.hpp"
#include "frovedis/matrix/pca.hpp"
#include "../exrpc/exrpc_expose.hpp"
#include "pca_result.hpp"

using namespace frovedis;

template <class MATRIX, class T>
pca_result frovedis_pca(exrpc_ptr_t& data_ptr, int& k, 
                        bool& whiten,
                        bool& toCopy = true,
                        bool& rearrange_out = true,
                        bool& need_pca_scores = false,
                        bool& need_eigen_values = false,
                        bool& need_variance_ratio = false,
                        bool& need_singular_values = false,
                        bool& need_mean = false,
                        bool& isMovableInput = false ) {
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);  // rowmajor_matrix<T> only at this moment    
  auto nrows = mat.num_row;
  auto ncols = mat.num_col;
  // output attributes
  auto pca_directions = new colmajor_matrix<T>(); // components_
  auto pca_scores = new colmajor_matrix<T>();
  auto eigen_values = new std::vector<T>(); // explained_varaiance_
  auto explained_variance_ratio = new std::vector<T>();  
  auto singular_values = new std::vector<T>();
  auto mean = new std::vector<T>();
  T noise_variance = 0;
  // calling frovedis::pca(...)
  pca(mat,*pca_directions,*pca_scores,
      *eigen_values,*explained_variance_ratio,
      *singular_values,*mean,noise_variance,k,whiten,false,toCopy);
  // if input is movable, destroying Frovedis side data after computation is done.
  if (isMovableInput)  mat.clear();
#ifdef _EXRPC_DEBUG_
  std::cout << "components: \n"; pca_directions->debug_print();
  std::cout << "ratio: \n"; pca_scores->debug_print();
  std::cout << "eigen values: \n"; 
  for(auto e: *eigen_values) std::cout << e << " "; std::cout << std::endl;
  std::cout << "variance ratio: \n";
  for(auto e: *explained_variance_ratio) std::cout << e << " "; std::cout << std::endl;
  std::cout << "singular values: \n";
  for(auto e: *singular_values) std::cout << e << " "; std::cout << std::endl;
  std::cout << "column wise mean: \n";
  for(auto e: *mean) std::cout << e << " "; std::cout << std::endl;
  std::cout << "noise_variance: " << noise_variance << std::endl;
#endif
  if (rearrange_out) { // true for python/spark cases
    // rearrange functionalities: defined in exrpc_svd.hpp
    rearrange_colmajor_data_inplace(*pca_directions); // mandatory output
    if(need_pca_scores) rearrange_colmajor_data_inplace(*pca_scores);
    if(need_eigen_values) rearrange_vector_data_inplace(*eigen_values);
    if(need_variance_ratio) rearrange_vector_data_inplace(*explained_variance_ratio);
    if(need_singular_values) rearrange_vector_data_inplace(*singular_values);
  }
  // handling output attributes
  auto comp_ptr = reinterpret_cast<exrpc_ptr_t>(pca_directions);

  exrpc_ptr_t score_ptr = -1;
  if(need_pca_scores) 
    score_ptr = reinterpret_cast<exrpc_ptr_t>(pca_scores); 
  else delete pca_scores;

  exrpc_ptr_t eig_ptr = -1;
  if(need_eigen_values) 
    eig_ptr = reinterpret_cast<exrpc_ptr_t>(eigen_values);
  else delete eigen_values;

  exrpc_ptr_t var_ratio_ptr = -1;
  if(need_variance_ratio)
    var_ratio_ptr = reinterpret_cast<exrpc_ptr_t>(explained_variance_ratio);
  else delete explained_variance_ratio;

  exrpc_ptr_t sval_ptr = -1;
  if(need_singular_values) 
    sval_ptr = reinterpret_cast<exrpc_ptr_t>(singular_values);
  else delete singular_values;

  exrpc_ptr_t mean_ptr = -1;
  if(need_mean) mean_ptr = reinterpret_cast<exrpc_ptr_t>(mean);
  else delete mean;

  return pca_result(nrows, ncols, k, noise_variance,
                    comp_ptr, score_ptr, eig_ptr,
                    var_ratio_ptr, sval_ptr, mean_ptr);
}

template <class MATRIX, class L_MATRIX, class T>
dummy_matrix
frovedis_pca_transform(exrpc_ptr_t& data_ptr,
             exrpc_ptr_t& pc_ptr,
             exrpc_ptr_t& var_ptr,
             exrpc_ptr_t& mean_ptr,
             bool& whiten){
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);
  colmajor_matrix<T>& pca_directions = *reinterpret_cast<colmajor_matrix<T>*>(pc_ptr);
  std::vector<T>& explained_variance = *reinterpret_cast<std::vector<T>*>(var_ptr);
  std::vector<T>& mean = *reinterpret_cast<std::vector<T>*>(mean_ptr);
  auto res = transform_pca(mat, mean, pca_directions, explained_variance, whiten);
  auto x_transformed = new MATRIX(std::move(res)); // move: stack to heap
  return to_dummy_matrix<MATRIX, L_MATRIX>(x_transformed);
}

template <class MATRIX, class L_MATRIX, class T>
dummy_matrix
frovedis_pca_inverse_transform(exrpc_ptr_t& data_ptr,
             exrpc_ptr_t& pc_ptr,
             exrpc_ptr_t& var_ptr,
             exrpc_ptr_t& mean_ptr,
             bool& whiten){
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);
  colmajor_matrix<T>& pca_directions = *reinterpret_cast<colmajor_matrix<T>*>(pc_ptr);
  std::vector<T>& explained_variance = *reinterpret_cast<std::vector<T>*>(var_ptr);
  std::vector<T>& mean = *reinterpret_cast<std::vector<T>*>(mean_ptr);
  auto res =  inverse_transform_pca(mat, mean, pca_directions, explained_variance, whiten);
  auto x_inverse_transformed = new MATRIX(std::move(res));
  return to_dummy_matrix<MATRIX, L_MATRIX>(x_inverse_transformed);
}

#endif
