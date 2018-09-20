#ifndef _NAIVE_BAYES_HPP_
#define _NAIVE_BAYES_HPP_

#include <algorithm>
#include "../../matrix/rowmajor_matrix.hpp"
#include "../../matrix/blas_wrapper.hpp"
#ifdef USE_DF
#include "../../dataframe.hpp"
#endif
#include "nb_model.hpp"

namespace frovedis {

#ifdef USE_DF
template <class T>
struct fill_data {
  fill_data () {}
  fill_data (T& val): value(val) {}
  void operator() (std::vector<T>& vec, 
                   const std::vector<size_t>& sizes) {
    vec.resize(sizes[frovedis::get_selfid()],value);
  }
  T value;
  SERIALIZE(value)
};

// to create dvector with given value and size-vector
template <class T>
dvector<T> make_dvector(const std::vector<size_t>& sizes, 
                        T value) {
  auto local_vec = make_node_local_allocate<std::vector<T>>();
  local_vec.mapv(fill_data<T>(value),broadcast(sizes));
  return local_vec.template moveto_dvector<T>();
}
#else 
template <class T>
std::pair<T,T> get_pair(const T& data) {
  return std::make_pair(data,1);
}

template <class T>  T sum (const T& x, const T& y) { return x + y; }
#endif

template <class T>
size_t get_index(const std::vector<T>& src, const T& item) {
  // src needs to be sorted...
  auto id = std::lower_bound(src.begin(), src.end(), item);
  if(id == src.end() || *id != item) 
    REPORT_ERROR(INTERNAL_ERROR,"item not found\n"); // should not occur
  return id - src.begin();
}

template <class T>
void get_class_counts (dvector<T>& label, 
                       std::vector<T>& uniq_labels, 
                       std::vector<T>& cls_counts) {
#ifdef USE_DF
  // assigning ones to each label just to get uniq label counts
  auto mark = make_dvector<T>(label.sizes(),1);

  dftable tmp;
  tmp.append_column("label",label);
  tmp.append_column("mark",mark);
  //tmp.show();

  std::vector<std::string> target = {std::string("label")};
  auto grouped = tmp.group_by(target);
  auto agg = sum_as("mark","count");
  std::vector<std::shared_ptr<frovedis::dfaggregator>> aggv = {agg};
  auto with_count = grouped.select(target,aggv)
                           .sort("label"); // sort() needed for lower_bound search
  //with_count.show();

  uniq_labels = with_count.as_dvector<T>("label").gather(); 
  cls_counts  = with_count.as_dvector<T>("count").gather(); 
#else
  auto dv3 = label.map(get_pair<T>)
                  .reduce_by_key<T,T>(sum<T>)
                  .as_dvector().sort().gather(); // sort() needed for lower bound search
  auto nclasses = dv3.size();
  uniq_labels.resize(nclasses);
  cls_counts.resize(nclasses);
  for(int i=0; i<nclasses; i++) {
    uniq_labels[i] = dv3[i].first;
    cls_counts[i]  = dv3[i].second;
  }
#endif
  //display<T>(uniq_labels);
  //display<T>(cls_counts);
}

template <class T, class LOC_MATRIX>
std::vector<T> get_freq(LOC_MATRIX& mat, 
                        std::vector<T>& label,
                        std::vector<T>& uniq_labels,
                        std::vector<T>& cls_counts,
                        double lambda) {
  auto nfeatures  = mat.local_num_col;
  auto nsamples   = label.size(); 
  auto nclasses   = uniq_labels.size();

  // TODO: Handle exception when worker doesnt contain data
  // for workers which doesn't contain any values
  if (nsamples == 0) { 
     //std::cout << "[" << get_selfid() << "]: " << mat.local_num_row << std::endl;
     return std::vector<T>(nfeatures*nclasses,0);
  }

  // creation of class label matrix
  std::vector<T> temp(nsamples*nclasses,0);
  for(auto i=0; i<nsamples; i++) {
    auto index = get_index(uniq_labels,label[i]);
    temp[i*nclasses+index] = 1;
  }
  rowmajor_matrix_local<T> cls_mat;
  cls_mat.val.swap(temp);
  cls_mat.set_local_num(nsamples,nclasses);
  //cls_mat.debug_print();

  // calculation of frequency table 
  auto freq_table = mat.transpose() * cls_mat;
  // freq_table.debug_print();

  return freq_table.val;
}

template <class T>
rowmajor_matrix_local<T> get_theta (std::vector<T>& freq_table, 
                                    std::vector<T>& cls_counts,
                                    double lambda, 
                                    const std::string& modelType,
                                    size_t nfeatures,
                                    size_t nclasses) {
  T* freqp = &freq_table[0];
  T* cls_countsp = &cls_counts[0];
  if (modelType == "bernoulli") {
    for(auto i=0; i<nclasses; ++i) {
      for(auto j=0; j<nfeatures; ++j) {
        freqp[j*nclasses+i] = log(freqp[j*nclasses+i]+lambda) -
                              log(cls_countsp[i]+lambda*2); // 2: as for bernoulli 
      }
    }
  }
  else if (modelType == "multinomial"){
    for(auto i=0; i<nclasses; ++i) {
      T total = 0;
      for(auto j=0; j<nfeatures; ++j) {
        total += freqp[j*nclasses+i] + lambda; 
      }
      for(auto j=0; j<nfeatures; ++j) {
        freqp[j*nclasses+i] = log(freqp[j*nclasses+i]+lambda)-log(total);
      }
    }
  }
  else REPORT_ERROR(USER_ERROR, "Unsupported model type is specified!\n");
  rowmajor_matrix_local<T> theta;
  theta.val.swap(freq_table); // smoothened freq_table
  theta.set_local_num(nfeatures,nclasses);
  //theta.debug_print();
  return theta;
}

template <class T>
std::vector<T> get_pi (std::vector<T>& cls_counts, 
                       double lambda, size_t nsamples) {
  auto nclasses = cls_counts.size();
  std::vector<T> pi(nclasses); 
  T* pip = &pi[0];
  T* cls_countsp = &cls_counts[0];
  for(auto i=0; i<nclasses; i++) {
    pip[i] = log(cls_countsp[i]+lambda) - log(nsamples+nclasses*lambda);
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
                        const std::string& modelType) {

  if (mat.num_row != label.size())
    REPORT_ERROR(USER_ERROR,"number of samples is not same in data and label.\n");

  std::vector<T> cls_counts;
  get_class_counts(label,uniq_labels,cls_counts);
  auto sizes = mat.data.map(get_local_row<LOC_MATRIX>).gather();
  //re-aligning the label (needed for using dataframe functions for above class counts)
  label.align_as(sizes); 
  auto nsamples  = mat.num_row;
  auto nfeatures = mat.num_col;
  auto nclasses  = uniq_labels.size();
  auto freq_table = mat.data.map(get_freq<T,LOC_MATRIX>,
                                 label.viewas_node_local(),
                                 broadcast(uniq_labels),
                                 broadcast(cls_counts),
                                 broadcast(lambda)).vector_sum();
  // freq_table would be smoothened in-place and replaced with theta.val
  theta = get_theta(freq_table,cls_counts,lambda,modelType,nfeatures,nclasses);
  pi = get_pi(cls_counts,lambda,nsamples);
}

template <class T, class MATRIX, class LOC_MATRIX>
naive_bayes_model<T>
multinomial_nb (MATRIX& mat, dvector<T>& label, double lambda = 1.0) {
  std::vector<T> pi, uniq_labels;
  rowmajor_matrix_local<T> theta;
  naive_bayes_common<T,MATRIX,LOC_MATRIX>(mat,label,lambda,
                                          theta,pi,uniq_labels,"multinomial");
  return naive_bayes_model<T>(std::move(theta),std::move(pi),
                              std::move(uniq_labels),"multinomial");
}

template <class T, class MATRIX, class LOC_MATRIX>
naive_bayes_model<T>
bernoulli_nb (MATRIX& mat, dvector<T>& label, double lambda = 1.0) {
  std::vector<T> pi, uniq_labels;
  rowmajor_matrix_local<T> theta;
  naive_bayes_common<T,MATRIX,LOC_MATRIX>(mat,label,lambda,
                                          theta,pi,uniq_labels,"bernoulli");
  return naive_bayes_model<T>(std::move(theta),std::move(pi),
                              std::move(uniq_labels),"bernoulli");
}

} // end of namespace

#endif
