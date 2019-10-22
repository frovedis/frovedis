#ifndef _SOFTMAX_GRADIENT_DESCENT_HPP_
#define _SOFTMAX_GRADIENT_DESCENT_HPP_

#include <iostream>
#include <cmath>
#include "../../core/exceptions.hpp"
#include "../../matrix/blas_wrapper.hpp"

//#define DIRECT_EXP

namespace frovedis {

template <class T>
rowmajor_matrix_local<T>
trans_mm(colmajor_matrix_local<T>& m1,
         rowmajor_matrix_local<T>& m2);
  
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

// --- matrix  multiplication function added to support dense data ---
// function performs A.transpose * B (without physical transpose)
template <class T>
rowmajor_matrix_local<T>
trans_mm(colmajor_matrix_local<T>& m1,
         rowmajor_matrix_local<T>& m2 ) {

  size_t nrow1 = m1.local_num_col;
  size_t ncol1 = m1.local_num_row; // locally transpose nrow and ncol
  size_t nrow2 = m2.local_num_row;
  size_t ncol2 = m2.local_num_col;

  checkAssumption(ncol1 == nrow2);

  rowmajor_matrix_local<T> res(nrow1, ncol2);

  auto m1p = m1.val.data();
  auto m2p = m2.val.data();
  auto resp = res.val.data();

#pragma _NEC nointerchange
  for(size_t j = 0; j < ncol2; ++j) {
    for(size_t k = 0; k < ncol1; ++k) {
      for(size_t i = 0; i < nrow1; ++i) {
         auto ind3 = i * ncol2 + j;
         auto ind1 = i * ncol1 + k;
         auto ind2 = k * ncol2 + j;
         resp[ind3] += m1p[ind1] * m2p[ind2];
       }
    }
  }
  return res;
}

template <class T, class DATA_MATRIX> 
rowmajor_matrix_local<T>
softmax_gradient_descent::compute_gradient(
  DATA_MATRIX& data,
  std::vector<T>& target,
  rowmajor_matrix_local<T>& weight,
  std::vector<T>& icpt) {
  
  //std::cout<<"\n Comppute gradient for cmm \n\n";
  auto wtx = compute_wtx<T>(data,weight,icpt);
  //std::cout <<"compute_gradient|wtx: "; wtx.debug_print(); 
  auto softmax_mat = compute_softmax_probability<T>(wtx);
  //std::cout <<"compute_gradient|softmax_mat: "; softmax_mat.debug_print(); 
  compute_error_inplace<T>(target,softmax_mat);
  //std::cout <<"compute_gradient|label-softmax_mat: "; softmax_mat.debug_print(); 
  auto grad_mat = trans_mm(data, softmax_mat);   
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
  //std::cout <<"compute_gradient|wtx: "; wtx.debug_print(); 
  auto softmax_mat = compute_softmax_probability<T>(wtx);
  //std::cout <<"compute_gradient|softmax_mat: "; softmax_mat.debug_print(); 
  compute_error_inplace<T>(target,softmax_mat);
  //std::cout <<"compute_gradient|label-softmax_mat: "; softmax_mat.debug_print(); 
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
#pragma _NEC nointerchange
    for(size_t j = 0; j < nclasses; ++j) {
      for(size_t i = 0; i < nsamples; ++i) {
        wtxp[i*nclasses+j] += icpt[j];
      }
    }
  }
  return wtx;
}

template <class T>
void compute_exp_matrix(rowmajor_matrix_local<T>& wtx) { // update input in-place
#ifdef DIRECT_EXP
  // pros: best loop-length with very good memory access
  // cons: prone to overflow during exp() calculation
  auto wtxp = wtx.val.data();
  for(size_t i = 0; i < wtx.val.size(); ++i) wtxp[i] = exp(wtxp[i]);
#else
  // pros: overflow in exp() overation is taken care of 
  // cons: loop-length is smaller and stride in memory access is increased
  auto nrow = wtx.local_num_row;
  auto ncol = wtx.local_num_col; // ncol has to be >= 1
  std::vector<T> max_of_each_wtx(nrow);
  auto maxp = max_of_each_wtx.data();
  auto wtxp = wtx.val.data();
  // marked 0th column (j = 0) as max
  for(size_t i = 0; i < nrow; ++i) maxp[i] = wtxp[i*ncol+0]; 
#pragma _NEC nointerchange
  for(size_t j = 1; j < ncol; ++j) {
    for(size_t i = 0; i < nrow; ++i) { // nrow >> ncol (thus loop-interchange)
      if (wtxp[i*ncol+j] > maxp[i]) maxp[i] = wtxp[i*ncol+j];
    }
  }
  // updating wtx matrix as exp(wtx - max_of_each_wtx)
#pragma _NEC nointerchange
  for(size_t j = 0; j < ncol; ++j) {
    for(size_t i = 0; i < nrow; ++i) { // nrow >> ncol (thus loop-interchange)
      wtxp[i*ncol+j] = exp( wtxp[i*ncol+j] - maxp[i] );
    }
  }
#endif
}

template <class T>
rowmajor_matrix_local<T>
softmax_gradient_descent::compute_softmax_probability (
  rowmajor_matrix_local<T>& wtx) {
  //std::cout << "wtx: \n"; wtx.debug_print();
  compute_exp_matrix(wtx); // wtx would be updated with exp(wtx)
  //std::cout << "exp(wtx): \n"; wtx.debug_print();
  auto sum_vec = sum_of_cols(wtx);
  //std::cout << "sum_vec: "; for(auto i: sum_vec) std::cout << i << " "; std::cout << std::endl;
  diag_matrix_local<T> one_by_tot(sum_vec.size());
  auto diagp = one_by_tot.val.data();
  auto sum_vecp = sum_vec.data();
  for(size_t i = 0; i < sum_vec.size(); ++i) {
    //if(sum_vecp[i] == 0)   REPORT_ERROR(INTERNAL_ERROR, "sum_vec becomes zero: REPORT BUG!\n");
    //if(isnan(sum_vecp[i])) REPORT_ERROR(INTERNAL_ERROR, "sum_vec becomes nan:  REPORT BUG!\n");
    diagp[i] = 1 / sum_vecp[i];
  }
  //std::cout << "diag_mat: "; for(auto i: one_by_tot.val) std::cout << i << " "; std::cout << std::endl;
  auto ret = one_by_tot * wtx;
  //std::cout << "softmax mat: \n"; ret.debug_print();
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
#pragma _NEC nointerchange
  for(size_t j = 0; j < nclasses; ++j) {
    for(size_t i = 0; i < nsamples; ++i) {
      //smatp[i*nclasses+j] = labelp[i] - smatp[i*nclasses+j];
      smatp[i*nclasses+j] -= (labelp[i] == j); // error = proba - actual
    }
  }
}

}
#endif
