#ifndef _KNN_SUPERVISED_HPP_
#define _KNN_SUPERVISED_HPP_

#include <frovedis/ml/neighbors/knn.hpp>
#include <frovedis/ml/metrics.hpp>

namespace frovedis {

// --- Classifier ---
template <class T, class MATRIX1>
struct kneighbors_classifier {
  kneighbors_classifier(int k, const std::string& algo,
                        const std::string& met, float c_sz, 
                        double batch_f = std::numeric_limits<double>::max()):
    n_neighbors(k), algorithm(algo), metric(met), 
    chunk_size(c_sz), batch_fraction(batch_f) {}
  
  void fit(MATRIX1& mat,
           dvector<T>& label) { // can support any values for input label
    if (mat.num_row != label.size()) 
      REPORT_ERROR(USER_ERROR, "number of entries differ in input matrix and label!\n");
    observation_data = mat;
    std::vector<size_t> u_ind, u_cnt;
    uniq_labels = vector_unique(label.gather(), u_ind, encoded_label, u_cnt);
    nclasses = uniq_labels.size();
  }

  template <class I = size_t, class MATRIX2 = rowmajor_matrix<T>>
  knn_model<T,I>
  kneighbors(MATRIX2& enquiry_data, 
             int k = 0, 
             bool need_distance = true) {
    if (k == 0) k = n_neighbors;
    return knn<T,I>(observation_data, enquiry_data, 
                    k, algorithm, metric, need_distance, chunk_size, batch_fraction);
  }

  template <class I = size_t, class O = size_t, class MATRIX2 = rowmajor_matrix<T>>
  crs_matrix<T, I, O>
  kneighbors_graph(MATRIX2& enquiry_data,
                   int k = 0,
                   const std::string& mode = "connectivity") {
    auto need_distance = (mode == std::string("distance"));
    auto model = kneighbors<I>(enquiry_data, k, need_distance);
    auto nsamples = observation_data.num_row;
    return model.template create_graph<O>(mode, nsamples);
  }

  template <class I = size_t, class MATRIX2 = rowmajor_matrix<T>>
  dvector<T> predict(MATRIX2& mat, 
                     bool save_proba = false);

  template <class I = size_t, class MATRIX2 = rowmajor_matrix<T>>
  rowmajor_matrix<T> predict_probability(MATRIX2& mat);

  template <class I = size_t, class MATRIX2 = rowmajor_matrix<T>>
  float score(MATRIX2& mat, dvector<T>& true_label);
 
  int n_neighbors;
  std::string algorithm;
  std::string metric;
  float chunk_size;
  double batch_fraction;
  MATRIX1 observation_data;
  std::vector<size_t> encoded_label;
  std::vector<T> uniq_labels;
  int nclasses;
  SERIALIZE(n_neighbors, algorithm, metric, chunk_size, batch_fraction,
            observation_data, encoded_label, uniq_labels, nclasses);
};

template <class I>
rowmajor_matrix_local<I>
get_count_matrix_helper(rowmajor_matrix_local<I>& model_indx,
                        std::vector<size_t>& encoded_label,
                        int nclasses) {
  auto nrow = model_indx.local_num_row;
  auto ncol = model_indx.local_num_col; // actually k
  rowmajor_matrix_local<I> count(nrow, nclasses);
  auto indxptr = model_indx.val.data();
  auto countptr = count.val.data();
  // FIXME: label might be same, ivdep might be destructive
  for(size_t j = 0; j < ncol; ++j) {
#pragma _NEC ivdep
    for(size_t i = 0; i < nrow; ++i) {  // nodep since vectorization on i-direction
      auto indx = indxptr[i * ncol + j];
      auto label = encoded_label[indx]; // always zero based
      countptr[i * nclasses + label]++;
    }
  }
  return count;
}

template <class I>
rowmajor_matrix<I>
get_count_matrix(rowmajor_matrix<I>& indices,
                 std::vector<size_t>& encoded_label,
                 int nclasses) {
  rowmajor_matrix<I> count(indices.data.map(get_count_matrix_helper<I>,
                           broadcast(encoded_label), broadcast(nclasses)));
  count.num_row = indices.num_row;
  count.num_col = nclasses;
  return count;
}

template <class T, class I>
std::vector<T> 
predict_local(rowmajor_matrix_local<I>& count,
              std::vector<T>& uniq_labels) {
  auto nrow = count.local_num_row;
  auto nclasses = count.local_num_col;
  std::vector<T> pred(nrow);
  auto predptr = pred.data();
  auto ulabelptr = uniq_labels.data();
  auto countptr = count.val.data();
  for(size_t i = 0; i < nrow; ++i) {
    auto max_voted_count = countptr[i * nclasses + 0];
    auto max_voted_label_indx = 0;
    for(size_t j = 1; j < nclasses; ++j) {
      auto cur_count = countptr[i * nclasses + j];
      if (cur_count > max_voted_count) { 
        max_voted_count = cur_count;
        max_voted_label_indx = j;
      }
    }
    predptr[i] = ulabelptr[max_voted_label_indx];
  }
  return pred;
}

template <class T>
std::vector<T>
predict_from_proba(rowmajor_matrix_local<T>& proba_mat,
                   std::vector<T>& uniq_labels) {
  auto nrow = proba_mat.local_num_row; // nsamples_local
  auto ncol = proba_mat.local_num_col; // nclasses
  std::vector<T> tmp(nrow), pred(nrow);
  auto tmpptr = tmp.data();
  auto predptr = pred.data();
  auto ulblptr = uniq_labels.data();
  auto probaptr = proba_mat.val.data();
  
  for(size_t i = 0; i < nrow; ++i) {
    tmpptr[i] = probaptr[i * ncol + 0]; // mark proba of first class as max
    predptr[i] = ulblptr[0]; // mark first class as perdicted class 
  }
  if (ncol > nrow) {
#pragma _NEC nointerchange
    for(size_t i = 0; i < nrow; ++i) {
      for(size_t j = 1; j < ncol; ++j) {
        auto cur_proba = probaptr[i * ncol + j];
        if (cur_proba > tmpptr[i]) {
          tmpptr[i] = cur_proba;
          predptr[i] = ulblptr[j];
        }
      }
    }
  }
  else {
#pragma _NEC nointerchange
    for(size_t j = 1; j < ncol; ++j) {
      for(size_t i = 0; i < nrow; ++i) {
        auto cur_proba = probaptr[i * ncol + j];
        if (cur_proba > tmpptr[i]) {
          tmpptr[i] = cur_proba;
          predptr[i] = ulblptr[j];
        }
      }
    }
  } // end of else
  return pred;
}

template <class T, class MATRIX1>
template <class I, class MATRIX2>
dvector<T> 
kneighbors_classifier<T, MATRIX1>::predict(MATRIX2& mat,
                                  bool save_proba) {
#ifdef MAJORITY_VOTE
  auto k = n_neighbors;
  auto need_distance = false;
  auto model = kneighbors<I>(mat, k, need_distance);
  auto count = get_count_matrix(model.indices, encoded_label, nclasses);
  return count.data.map(predict_local<T,I>, broadcast(uniq_labels))
                   .template moveto_dvector<T>();
#else // based on probability
  auto proba = predict_probability<I>(mat);
  if(save_proba) proba.save("probability_matrix");
  return proba.data.map(predict_from_proba<T>, broadcast(uniq_labels))
                   .template moveto_dvector<T>();
#endif
}

template <class T, class I>
rowmajor_matrix_local<T> 
predict_probability_local(rowmajor_matrix_local<I>& count,
                    int k) {
  auto nrow = count.local_num_row;
  auto nclasses = count.local_num_col;
  rowmajor_matrix_local<T> pred(nrow, nclasses);
  auto predptr = pred.val.data();
  auto countptr = count.val.data();
  auto one_by_k = 1.0 / k;
  for(size_t i = 0; i < nrow * nclasses; ++i) {
    predptr[i] = countptr[i] * one_by_k;
  }
  return pred;
}

template <class T, class MATRIX1>
template <class I, class MATRIX2>
rowmajor_matrix<T> 
kneighbors_classifier<T, MATRIX1>::predict_probability(MATRIX2& mat) {
  auto k = n_neighbors;
  auto need_distance = false;
  auto model = kneighbors<I>(mat, k, need_distance);
  auto count = get_count_matrix(model.indices, encoded_label, nclasses);
  rowmajor_matrix<T> pred(count.data.map(predict_probability_local<T,I>, 
                                         broadcast(k))); 
  pred.num_row = mat.num_row;
  pred.num_col = nclasses;
  return pred;
}

template <class T, class MATRIX1>
template <class I, class MATRIX2>
float kneighbors_classifier<T, MATRIX1>::score(MATRIX2& mat,
                                      dvector<T>& true_label) {
  auto pred_label = predict<I>(mat);
  return accuracy_score(true_label.gather(), pred_label.gather());
}

// --- Regressor ---
template <class T, class MATRIX1>
struct kneighbors_regressor {
  kneighbors_regressor(int k, const std::string& algo,
                       const std::string& met, float c_sz, 
                       double batch_f = std::numeric_limits<double>::max()):
    n_neighbors(k), algorithm(algo), metric(met), chunk_size(c_sz), 
    batch_fraction(batch_f) {}

  void fit(MATRIX1& mat, dvector<T>& label) { 
    if (mat.num_row != label.size())
      REPORT_ERROR(USER_ERROR, "number of entries differ in input matrix and label!\n");
    observation_data = mat;
    observation_labels = label.gather();
  }

  template <class I = size_t, class MATRIX2 = rowmajor_matrix<T>>
  knn_model<T,I>
  kneighbors(MATRIX2& enquiry_data,
             int k = 0,
             bool need_distance = true) {
    if (k == 0) k = n_neighbors;
    return knn<T,I>(observation_data, enquiry_data,
                    k, algorithm, metric, need_distance, chunk_size, batch_fraction);
  }

  template <class I = size_t, class O = size_t, class MATRIX2 = rowmajor_matrix<T>>
  crs_matrix<T, I, O>
  kneighbors_graph(MATRIX2& enquiry_data,
                   int k = 0,
                   const std::string& mode = "connectivity") {
    auto need_distance = (mode == std::string("distance"));
    auto model = kneighbors<I>(enquiry_data, k, need_distance);
    auto nsamples = observation_data.num_row;
    return model.template create_graph<O>(mode, nsamples);
  }

  template <class I = size_t, class MATRIX2 = rowmajor_matrix<T>>
  dvector<T> predict(MATRIX2& mat);

  template <class I = size_t, class MATRIX2 = rowmajor_matrix<T>>
  float score(MATRIX2& mat, dvector<T>& true_label);

  int n_neighbors;
  std::string algorithm;
  std::string metric;
  float chunk_size;
  double batch_fraction;
  MATRIX1 observation_data;
  std::vector<T> observation_labels;
  SERIALIZE(n_neighbors, algorithm, metric, chunk_size, batch_fraction,
            observation_data, observation_labels);
};

template <class T, class I>
std::vector<T> predict_from_mean(rowmajor_matrix_local<I>& model_indx,
                                 std::vector<T>& observation_labels) {
  auto nrow = model_indx.local_num_row; // nquery_local
  auto ncol = model_indx.local_num_col; // n_neighbors
  std::vector<T> pred(nrow, 0);
  auto indxptr = model_indx.val.data();
  auto lblptr = observation_labels.data();
  auto predptr = pred.data();
  T one_by_ncol = 1.0 / ncol;
#pragma _NEC nointerchange
  for(size_t j = 0; j < ncol; ++j) {
#pragma _NEC ivdep
    for(size_t i = 0; i < nrow; ++i) { 
      predptr[i] += lblptr[indxptr[i * ncol + j]];
    }
  }
  // simply the mean of labels of k-neighbors
  for(size_t i = 0; i < nrow; ++i) predptr[i] *= one_by_ncol;
  return pred;
}

template <class T, class MATRIX1>
template <class I, class MATRIX2>
dvector<T> 
kneighbors_regressor<T, MATRIX1>::predict(MATRIX2& mat) {
  auto k = n_neighbors;
  auto need_distance = false;
  auto model = kneighbors<I>(mat, k, need_distance);
  return model.indices.data.map(predict_from_mean<T,I>,
                                broadcast(observation_labels))
                           .template moveto_dvector<T>();
}

template <class T, class MATRIX1>
template <class I, class MATRIX2>
float kneighbors_regressor<T, MATRIX1>::score(MATRIX2& mat,
                                     dvector<T>& true_label) {
  auto pred_label = predict<I>(mat);
  return r2_score(true_label.gather(), pred_label.gather());
}

}
#endif
