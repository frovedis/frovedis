#ifndef _NAIVE_BAYES_HPP_
#define _NAIVE_BAYES_HPP_

#include <algorithm>
#include <frovedis/matrix/rowmajor_matrix.hpp>
#include <frovedis/matrix/blas_wrapper.hpp>
#include <frovedis/matrix/matrix_operations.hpp>
#include "nb_model.hpp"

namespace frovedis {

template <class T>
dvector<size_t> 
get_class_counts (dvector<T>& label, 
                  std::vector<T>& uniq_labels, 
                  std::vector<size_t>& cls_counts) {
  std::vector<size_t> u_ind, u_enc;
  uniq_labels = vector_unique(label.gather(), u_ind, u_enc, cls_counts);
  return make_dvector_scatter(u_enc);
}

template <class T, class LOC_MATRIX>
std::vector<T> get_freq(LOC_MATRIX& mat, 
                        std::vector<size_t>& encoded_label,
                        size_t nclasses) {
  auto nsamples   = encoded_label.size(); 
  rowmajor_matrix_local<T> cls_mat(nsamples, nclasses);
  auto clsmat_ptr = cls_mat.val.data();
  auto en_label_ptr = encoded_label.data();
  for(auto i = 0; i < nsamples; i++) {
    size_t index = en_label_ptr[i];
    clsmat_ptr[i * nclasses + index] = 1.0;
  }
  //cls_mat.debug_print();

  auto freq_table = mat.transpose() * cls_mat;
  // freq_table.debug_print();

  return freq_table.val;
}

template <class T>
rowmajor_matrix_local<T> get_theta (std::vector<T>& freq_table, // destroyed 
                                    std::vector<size_t>& cls_counts,
                                    double lambda, 
                                    const std::string& modelType,
                                    size_t nfeatures,
                                    size_t nclasses) {
  auto freqp = freq_table.data();
  auto cls_countsp = cls_counts.data();
  if (modelType == "bernoulli") {
    for(size_t i = 0; i < nclasses; ++i) {
#pragma _NEC ivdep
      for(auto j=0; j<nfeatures; ++j) {
        freqp[j*nclasses+i] = log(freqp[j*nclasses+i] + lambda) -
                              log(cls_countsp[i] + lambda*2); // 2: as for bernoulli 
      }
    }
  }
  else if (modelType == "multinomial"){
#pragma _NEC nointerchange
    for(size_t i = 0; i < nclasses; ++i) {
      T total = 0;
      for(size_t j = 0; j < nfeatures; ++j) total += freqp[j * nclasses + i] + lambda; 
#pragma _NEC ivdep
      for(size_t j = 0; j < nfeatures; ++j) {
        freqp[j * nclasses + i] = log(freqp[j*nclasses+i] + lambda) - log(total);
      }
    }
  }
  else REPORT_ERROR(USER_ERROR, "Unsupported model type is specified!\n");
  rowmajor_matrix_local<T> theta;
  theta.val.swap(freq_table); // smoothened freq_table
  theta.set_local_num(nfeatures, nclasses);
  //theta.debug_print();
  return theta;
}

template <class T>
std::vector<T> get_pi (std::vector<size_t>& cls_counts, 
                       double lambda, size_t nsamples) {
  auto nclasses = cls_counts.size();
  std::vector<T> pi(nclasses); 
  auto pip = pi.data();
  auto cls_countsp = cls_counts.data();
  auto subt = log(nsamples + nclasses * lambda);
  for(auto i = 0; i < nclasses; i++) {
    pip[i] = log(cls_countsp[i] + lambda) - subt;
  }
  return pi; // smoothened pi
}

template <class LOC_MATRIX>
size_t get_local_row(LOC_MATRIX& mat) { return mat.local_num_row; }

// this is the common naive bayes implementation
// which computes theta, pi and unique labels in-place
template <class T, class MATRIX, class LOC_MATRIX>
void naive_bayes_common(MATRIX& mat, 
                        dvector<T>& label,
                        double lambda,
                        rowmajor_matrix_local<T>& theta, 
                        std::vector<T>& pi, 
                        std::vector<T>& uniq_labels,
                        std::vector<size_t>& cls_counts,
                        const std::string& modelType) {

  auto nsamples  = mat.num_row;
  auto nfeatures = mat.num_col;
  if (mat.num_row != label.size())
    REPORT_ERROR(USER_ERROR,"number of samples is not same in data and label.\n");

  time_spent tencode(DEBUG), tfreq(DEBUG), ttheta(DEBUG), tpi(DEBUG);
  tencode.lap_start();
  auto encoded_label = get_class_counts(label, uniq_labels, cls_counts);
  tencode.lap_stop();
  tencode.show_lap("label encode time: ");
  auto nclasses  = uniq_labels.size();

  auto sizes = mat.data.map(get_local_row<LOC_MATRIX>).gather();
  //re-aligning the label (needed for using dataframe functions for above class counts)
  encoded_label.align_as(sizes);
 
  tfreq.lap_start();
  auto freq_table = mat.data.map(get_freq<T,LOC_MATRIX>,
                                 encoded_label.viewas_node_local(),
                                 broadcast(nclasses)).vector_sum();
  tfreq.lap_stop();
  tfreq.show_lap("freq calc: ");

  ttheta.lap_start();
  // freq_table would be smoothened in-place and replaced with theta.val
  theta = get_theta(freq_table,cls_counts,lambda,modelType,nfeatures,nclasses);
  ttheta.lap_stop();
  ttheta.show_lap("theta calc: ");

  tpi.lap_start();
  pi = get_pi<T>(cls_counts, lambda, nsamples);
  tpi.lap_stop();
  tpi.show_lap("pi calc: ");
}

template <class T, class MATRIX, class LOC_MATRIX>
naive_bayes_model<T>
multinomial_nb (MATRIX& mat, dvector<T>& label, double lambda = 1.0) {
  std::vector<T> pi, uniq_labels;
  std::vector<size_t> cls_counts;
  rowmajor_matrix_local<T> theta;
  naive_bayes_common<T,MATRIX,LOC_MATRIX>(mat,label,lambda,theta,pi,
                                          uniq_labels,cls_counts,"multinomial");
  return naive_bayes_model<T>(std::move(theta),std::move(pi),std::move(uniq_labels),
                              std::move(cls_counts),"multinomial");
}

template <class T>
naive_bayes_model<T>
multinomial_nb (crs_matrix<T>& mat, 
                dvector<T>& label, 
                double lambda = 1.0) {
  return multinomial_nb<T, crs_matrix<T>, crs_matrix_local<T>>(mat, label, lambda);
}

template <class T>
naive_bayes_model<T>
multinomial_nb (rowmajor_matrix<T>& mat, 
                dvector<T>& label, 
                double lambda = 1.0) {
  return multinomial_nb<T, rowmajor_matrix<T>, rowmajor_matrix_local<T>>(mat, label, lambda);
}

template <class T, class MATRIX, class LOC_MATRIX>
naive_bayes_model<T>
bernoulli_nb (MATRIX& mat, dvector<T>& label, 
              double lambda = 1.0, double binarize = 0.0) {
  std::vector<T> pi, uniq_labels;
  std::vector<size_t> cls_counts;
  rowmajor_matrix_local<T> theta;
  auto bin_mat = matrix_binarize(mat, static_cast<T>(binarize));
  naive_bayes_common<T,MATRIX,LOC_MATRIX>(bin_mat,label,lambda,theta,pi,
                                          uniq_labels,cls_counts,"bernoulli");
  return naive_bayes_model<T>(std::move(theta),std::move(pi),std::move(uniq_labels),
                              std::move(cls_counts),"bernoulli");
}

template <class T>
naive_bayes_model<T>
bernoulli_nb (crs_matrix<T>& mat, 
              dvector<T>& label, 
              double lambda = 1.0,
              double binarize = 0.0) {
  return bernoulli_nb<T, crs_matrix<T>, crs_matrix_local<T>>
    (mat, label, lambda, binarize);
}

template <class T>
naive_bayes_model<T>
bernoulli_nb (rowmajor_matrix<T>& mat, 
              dvector<T>& label, 
              double lambda = 1.0,
              double binarize = 0.0) {
  return bernoulli_nb<T, rowmajor_matrix<T>, rowmajor_matrix_local<T>>
    (mat, label, lambda, binarize);
}

} // end of namespace

#endif
