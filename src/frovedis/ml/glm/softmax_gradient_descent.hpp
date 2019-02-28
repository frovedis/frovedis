
#ifndef _SOFTMAX_GRADIENT_DESCENT_HPP_
#define _SOFTMAX_GRADIENT_DESCENT_HPP_

#include <iostream>
#include <cmath>
#include "../../core/exceptions.hpp"
#include "../../matrix/blas_wrapper.hpp"


namespace frovedis {

struct softmax_gradient_descent {
  softmax_gradient_descent (bool intercept) {
    isIntercept = intercept;
  }

  void debug_print() {
    std::cout << "isIntercept: " << isIntercept << "\n";
  } 

  template <class T, class DATA_MATRIX>
  rowmajor_matrix_local<T>
  compute_gradient(
    DATA_MATRIX& data,
    std::vector<T>& target,
    rowmajor_matrix_local<T>& weight,
    std::vector<T>& icpt);

  template <class T, class DATA_MATRIX, class TRANS_MATRIX> 
  rowmajor_matrix_local<T>
  compute_gradient(
    DATA_MATRIX& data,
    TRANS_MATRIX& trans,
    std::vector<T>& target,
    rowmajor_matrix_local<T>& weight,
    std::vector<T>& icpt);

  template <class T, class DATA_MATRIX>
  rowmajor_matrix_local<T>
  compute_wtx (DATA_MATRIX& data, 
               rowmajor_matrix_local<T>& weight,
               std::vector<T>& icpt);

  template <class T>
  rowmajor_matrix_local<T>
  compute_softmax_probability (rowmajor_matrix_local<T>& wtx); 
  
  template <class T>
  void compute_error_inplace(std::vector<T>& label, 
                             rowmajor_matrix_local<T>& softmax_mat);

  bool isIntercept;
  SERIALIZE(isIntercept)
};

template <class T, class DATA_MATRIX> 
rowmajor_matrix_local<T>
softmax_gradient_descent::compute_gradient(
  DATA_MATRIX& data,
  std::vector<T>& target,
  rowmajor_matrix_local<T>& weight,
  std::vector<T>& icpt) {
  auto wtx = compute_wtx<T>(data,weight,icpt);
  auto softmax_mat = compute_softmax_probability<T>(wtx);
  compute_error_inplace<T>(target,softmax_mat);
  auto grad_mat = trans_mv(data,softmax_mat);
  return grad_mat;
}

template <class T, class DATA_MATRIX, class TRANS_MATRIX> 
rowmajor_matrix_local<T>
softmax_gradient_descent::compute_gradient(
  DATA_MATRIX& data,
  TRANS_MATRIX& trans,
  std::vector<T>& target,
  rowmajor_matrix_local<T>& weight,
  std::vector<T>& icpt) {
  auto wtx = compute_wtx<T>(data,weight,icpt);
  auto softmax_mat = compute_softmax_probability<T>(wtx);
  compute_error_inplace<T>(target,softmax_mat);
  auto grad_mat = trans * softmax_mat;
  return grad_mat;
}

template <class T, class DATA_MATRIX>
rowmajor_matrix_local<T>
softmax_gradient_descent::compute_wtx (
  DATA_MATRIX& data,
  rowmajor_matrix_local<T>& weight,
  std::vector<T>& icpt) {
  auto wtx = data * weight;
  auto nsamples = wtx.local_num_row;
  auto nclasses = wtx.local_num_col;
  T* wtxp = &wtx.val[0];
  if(isIntercept) {
    for(size_t j = 0; j < nclasses; ++j) {
      for(size_t i = 0; i < nsamples; ++i) {
        wtxp[i*nclasses+j] += icpt[j];
      }
    }
  }
  return wtx;
}

template <class T>
rowmajor_matrix_local<T>
softmax_gradient_descent::compute_softmax_probability (
  rowmajor_matrix_local<T>& wtx) {
  T* wtxp = &wtx.val[0];
  for(size_t i=0; i<wtx.val.size(); ++i) wtxp[i] = exp(wtxp[i]);
  auto sum_vec = sum_of_cols(wtx);
  //for(auto i: sum_vec) std::cout << i << " "; std::cout << std::endl;
  T* vecp = &sum_vec[0];
  for(size_t i=0; i<sum_vec.size(); ++i) vecp[i] = 1/vecp[i];
  //for(auto i: sum_vec) std::cout << i << " "; std::cout << std::endl;
  diag_matrix_local<T> one_by_tot(sum_vec.size());
  one_by_tot.val.swap(sum_vec);
  auto ret = one_by_tot * wtx;
  //auto tmp = sum_of_cols(ret);
  //for(auto i: tmp) std::cout << i << " "; std::cout << std::endl; // should be all ones
  return ret;
}

template <class T>
void softmax_gradient_descent::compute_error_inplace (
  std::vector<T>& label, 
  rowmajor_matrix_local<T>& softmax_mat) {
  auto nsamples = softmax_mat.local_num_row;
  auto nclasses = softmax_mat.local_num_col;
  T *smatp = &softmax_mat.val[0];
  auto labelp = label.data();
  for(size_t j = 0; j < nclasses; ++j) {
    for(size_t i = 0; i < nsamples; ++i) {
      smatp[i*nclasses+j] = labelp[i] - smatp[i*nclasses+j];
    }
  }
}

}
#endif
