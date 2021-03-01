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
                  std::vector<T>& cls_counts,
                  std::vector<T>& weight) {
  std::vector<size_t> u_ind, u_enc, u_cnt;
  uniq_labels = vector_unique(label.gather(), u_ind, u_enc, u_cnt,
                              weight, cls_counts);
  return make_dvector_scatter(u_enc);
}

template <class T, class LOC_MATRIX>
std::vector<T> get_freq(LOC_MATRIX& mat, 
                        std::vector<size_t>& encoded_label,
                        size_t nclasses,
                        std::vector<T>& weight) {
  auto nsamples   = encoded_label.size(); 
  rowmajor_matrix_local<T> cls_mat(nclasses, nsamples);
  auto clsmat_ptr = cls_mat.val.data();
  auto en_label_ptr = encoded_label.data();
  auto weightp = weight.data();
  for(auto i = 0; i < nsamples; i++) {
    size_t index = en_label_ptr[i];
    clsmat_ptr[index * nsamples + i] = weightp[i];
  }
  auto freq_table = cls_mat * mat;
  return freq_table.val;
}

template <class T>
rowmajor_matrix_local<T> get_theta (std::vector<T>& freq_table, // destroyed 
                                    std::vector<T>& cls_counts,
                                    double lambda, 
                                    const std::string& modelType,
                                    size_t nfeatures,
                                    size_t nclasses) {
  auto freqp = freq_table.data();
  auto cls_countsp = cls_counts.data();
  if (modelType == "bernoulli") {
    for(size_t i = 0; i < nclasses; ++i) {
      #pragma _NEC ivdep
      for(auto j = 0; j < nfeatures; ++j) {
        freqp[i*nfeatures+j] = log( freqp[i*nfeatures+j] + lambda) -
                              log(cls_countsp[i] + lambda*2); // 2: as for bernoulli 
      }
    }
  }
  else if (modelType == "multinomial"){
#pragma _NEC nointerchange
    for(size_t i = 0; i < nclasses; ++i) {
      T total = 0;
      for(size_t j = 0; j < nfeatures; ++j) total +=  freqp[i*nfeatures+j] + lambda; 
      #pragma _NEC ivdep
      for(size_t j = 0; j < nfeatures; ++j) {
        freqp[i*nfeatures+j] = log( freqp[i*nfeatures+j] + lambda) - log(total);
      }
    }
  }
  else REPORT_ERROR(USER_ERROR, "Unsupported model type is specified!\n");
  rowmajor_matrix_local<T> theta;
  theta.val.swap(freq_table); // smoothened freq_table
  theta.set_local_num(nclasses, nfeatures);
  return theta;
}

template <class T>
std::vector<T> get_pi (std::vector<T>& cls_counts, 
                       size_t nsamples, 
                       bool fit_prior, 
                       const std::vector<T> class_prior) {
  auto nclasses = cls_counts.size();
  std::vector<T> pi(nclasses); 
  auto pip = pi.data();
  if(!class_prior.empty()) {
    require(class_prior.size() == nclasses,
    "class_prior size does not match with number of classes in input data");
    auto cpp = class_prior.data();
    for(size_t i = 0; i < nclasses; i++) pip[i] = log(cpp[i]);
  }
  else if(fit_prior) {
    auto cls_countsp = cls_counts.data();
    T cc_sum = vector_sum(cls_counts);
    for(size_t i = 0; i < nclasses; i++) {
      pip[i] = log(cls_countsp[i]) - log(cc_sum);
    }
  }
  else {
    for(size_t i = 0; i < nclasses; i++) pip[i] = -log(nclasses);
  }
  return pi; 
}

template <class LOC_MATRIX>
size_t get_local_row(LOC_MATRIX& mat) { return mat.local_num_row; }

// this is the common naive bayes implementation
// which computes theta, pi and unique labels in-place
template <class T, class MATRIX>
void naive_bayes_common(MATRIX& mat, 
                        dvector<T>& label,
                        double lambda,
                        bool fit_prior, 
                        const std::vector<T>& class_prior,
                        const std::vector<T>& sample_weight,
                        rowmajor_matrix_local<T>& theta, 
                        std::vector<T>& pi, 
                        std::vector<T>& uniq_labels,
                        std::vector<T>& cls_counts,
                        std::vector<T>& feature_count,
                        const std::string& modelType) {
  auto nsamples  = mat.num_row;
  auto nfeatures = mat.num_col;
  if (mat.num_row != label.size())
    REPORT_ERROR(USER_ERROR,"number of samples is not same in data and label.\n");
  auto& weight = const_cast<std::vector<T>&>(sample_weight);
  if (weight.empty()) weight = vector_ones<T>(nsamples);

  time_spent tencode(DEBUG), tfreq(DEBUG), ttheta(DEBUG), tpi(DEBUG);
  tencode.lap_start();
  auto encoded_label = get_class_counts(label, uniq_labels, cls_counts, weight);
  tencode.lap_stop();
  tencode.show_lap("label encode time: ");
  auto nclasses  = uniq_labels.size();

  auto sizes = mat.get_local_num_rows();
  //re-aligning the label (needed for using dataframe functions for above class counts)
  if(encoded_label.sizes() != sizes) encoded_label.align_as(sizes);
 
  tfreq.lap_start();
  auto freq_table = mat.data.map(get_freq<T,typename MATRIX::local_mat_type>,
                                 encoded_label.viewas_node_local(),
                                 broadcast(nclasses),
                                 broadcast(weight)).vector_sum();
  feature_count = freq_table;
  tfreq.lap_stop();
  tfreq.show_lap("freq calc: ");

  ttheta.lap_start();
  // freq_table would be smoothened in-place and replaced with theta.val
  theta = get_theta(freq_table,cls_counts,lambda,modelType,nfeatures,nclasses);
  ttheta.lap_stop();
  ttheta.show_lap("theta calc: ");

  tpi.lap_start();
  pi = get_pi(cls_counts, nsamples, fit_prior, class_prior);
  tpi.lap_stop();
  tpi.show_lap("pi calc: ");
}

template <class T, class MATRIX>
naive_bayes_model<T>
multinomial_nb (MATRIX& mat,
                dvector<T>& label, 
                double lambda = 1.0,
                bool fit_prior = true,
                const std::vector<T>& class_prior = std::vector<T>(),
                const std::vector<T>& sample_weight = std::vector<T>()) {
  std::vector<T> pi, uniq_labels;
  std::vector<T> cls_counts;
  rowmajor_matrix_local<T> theta;
  std::vector<T> feature_count;
  naive_bayes_common(mat, label, lambda, fit_prior, class_prior,
                     sample_weight, 
                     theta, pi, uniq_labels, cls_counts,
                     feature_count, "multinomial");
  return naive_bayes_model<T>(std::move(theta),std::move(pi),
                              std::move(uniq_labels), std::move(cls_counts),
                              std::move(feature_count),"multinomial");
}

template <class T>
naive_bayes_model<T>
multinomial_nb (crs_matrix<T>& mat, 
                dvector<T>& label, 
                double lambda = 1.0,
                bool fit_prior = true,
                const std::vector<T>& class_prior = std::vector<T>(),
                const std::vector<T>& sample_weight = std::vector<T>()) {
  return multinomial_nb<T, crs_matrix<T>>(mat, label,
                        lambda, fit_prior, class_prior, sample_weight);
}

template <class T>
naive_bayes_model<T>
multinomial_nb (rowmajor_matrix<T>& mat, 
                dvector<T>& label, 
                double lambda = 1.0,
                bool fit_prior = true,
                const std::vector<T>& class_prior = std::vector<T>(),
                const std::vector<T>& sample_weight = std::vector<T>()) {
  return multinomial_nb<T, rowmajor_matrix<T>>(mat, label,
                        lambda, fit_prior, class_prior, sample_weight);
}

template <class T, class MATRIX>
naive_bayes_model<T>
bernoulli_nb (MATRIX& mat, 
              dvector<T>& label, 
              double lambda = 1.0,
              double binarize = 0.0,
              bool fit_prior = true, 
              const std::vector<T>& class_prior = std::vector<T>(),
              const std::vector<T>& sample_weight = std::vector<T>()) {
  std::vector<T> pi, uniq_labels;
  std::vector<T> cls_counts;
  rowmajor_matrix_local<T> theta;
  std::vector<T> feature_count;
  auto bin_mat = matrix_binarize(mat, static_cast<T>(binarize));
  naive_bayes_common(bin_mat, label, lambda,
                     fit_prior, class_prior, sample_weight, 
                     theta, pi, uniq_labels, cls_counts,
                     feature_count, "bernoulli");
  return naive_bayes_model<T>(std::move(theta),std::move(pi),
                              std::move(uniq_labels),
                              std::move(cls_counts),std::move(feature_count),
                              "bernoulli",binarize);
}

template <class T>
naive_bayes_model<T>
bernoulli_nb (crs_matrix<T>& mat, 
              dvector<T>& label, 
              double lambda = 1.0,
              double binarize = 0.0,
              bool fit_prior = true, 
              const std::vector<T>& class_prior = std::vector<T>(),
              const std::vector<T>& sample_weight = std::vector<T>()) {
  return bernoulli_nb<T, crs_matrix<T>>
    (mat, label, lambda, binarize, fit_prior, class_prior, sample_weight);
}

template <class T>
naive_bayes_model<T>
bernoulli_nb (rowmajor_matrix<T>& mat, 
              dvector<T>& label, 
              double lambda = 1.0,
              double binarize = 0.0,
              bool fit_prior = true, 
              const std::vector<T>& class_prior = std::vector<T>(),
              const std::vector<T>& sample_weight = std::vector<T>()) {
  return bernoulli_nb<T, rowmajor_matrix<T>>
    (mat, label, lambda, binarize, fit_prior, class_prior, sample_weight);
}

} // end of namespace

#endif
