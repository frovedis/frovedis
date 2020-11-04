#ifndef _KNN_SUPERVISED_HPP_
#define _KNN_SUPERVISED_HPP_

#include <frovedis/dataframe/dftable.hpp>
#include <frovedis/dataframe/dfoperator.hpp>
#include <frovedis/ml/neighbors/knn.hpp>
#include <frovedis/ml/metrics.hpp>

namespace frovedis {

template <class T>
dvector<size_t> 
encode_label(dvector<T>& label) {
  dftable left;
  left.append_column("labels", label);
  std::vector<std::string> target = {std::string("labels")};
  auto right = left.group_by(target).select(target)
                   .rename("labels", "src")
                   .sort("src")
                   .append_rowid("target"); // zero based ids (col type: size_t)
  auto joined = left.bcast_join(right, eq("labels", "src")); // encoding by joining
  return joined.as_dvector<size_t>("target");
}

template <class T>
void do_sort(std::vector<T>& vec) {
  radix_sort(vec, false);
}

template <class T>
std::vector<T> 
get_uniq_labels(dvector<T>& label) {
  return label.as_node_local()
              .mapv(do_sort<T>)
              .map(set_unique<T>)
              .reduce(set_union<T>);
}

// --- Classifier ---

template <class T>
struct kneighbors_classifier {
  kneighbors_classifier(int k, const std::string& algo,
                        const std::string& met, float c_sz):
    n_neighbors(k), algorithm(algo), metric(met), chunk_size(c_sz) {}
  
  void fit(rowmajor_matrix<T>& mat,
           dvector<T>& label) { // can support any values for input label
    if (mat.num_row != label.size()) 
      REPORT_ERROR(USER_ERROR, "number of entries differ in input matrix and label!\n");
    observation_data = mat;
    encoded_label = encode_label(label).gather();
    uniq_labels = get_uniq_labels(label);
    nclasses = uniq_labels.size();
  }

  template <class I = size_t>
  knn_model<T,I>
  kneighbors(rowmajor_matrix<T>& enquiry_data, 
             int k = 0, 
             bool need_distance = true) {
    if (k == 0) k = n_neighbors;
    return knn<T,I>(observation_data, enquiry_data, 
                    k, algorithm, metric, need_distance, chunk_size);
  }

  template <class I = size_t, class O = size_t>
  crs_matrix<T, I, O>
  kneighbors_graph(rowmajor_matrix<T>& enquiry_data,
                   int k = 0,
                   const std::string& mode = "connectivity") {
    auto need_distance = (mode == std::string("distance"));
    auto model = kneighbors<I>(enquiry_data, k, need_distance);
    auto nsamples = observation_data.num_row;
    return model.create_graph(mode, nsamples);
  }

  template <class I = size_t>
  dvector<T> predict(rowmajor_matrix<T>& mat, 
                     bool save_proba = false);

  template <class I = size_t>
  rowmajor_matrix<T> predict_probability(rowmajor_matrix<T>& mat);

  template <class I = size_t>
  float score(rowmajor_matrix<T>& mat, dvector<T>& true_label);
 
  int n_neighbors;
  std::string algorithm;
  std::string metric;
  float chunk_size;
  rowmajor_matrix<T> observation_data;
  std::vector<size_t> encoded_label;
  std::vector<T> uniq_labels;
  int nclasses;
  SERIALIZE(n_neighbors, algorithm, metric, chunk_size,
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

template <class T>
template <class I>
dvector<T> 
kneighbors_classifier<T>::predict(rowmajor_matrix<T>& mat,
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

template <class T>
template <class I>
rowmajor_matrix<T> 
kneighbors_classifier<T>::predict_probability(rowmajor_matrix<T>& mat) {
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

template <class T>
template <class I>
float kneighbors_classifier<T>::score(rowmajor_matrix<T>& mat,
                                      dvector<T>& true_label) {
  auto pred_label = predict<I>(mat);
  return accuracy_score(pred_label.gather(), true_label.gather());
}

// --- Regressor ---

template <class T>
struct kneighbors_regressor {
  kneighbors_regressor(int k, const std::string& algo,
                       const std::string& met, float c_sz):
    n_neighbors(k), algorithm(algo), metric(met), chunk_size(c_sz) {}

  void fit(rowmajor_matrix<T>& mat, dvector<T>& label) { 
    if (mat.num_row != label.size())
      REPORT_ERROR(USER_ERROR, "number of entries differ in input matrix and label!\n");
    observation_data = mat;
    observation_labels = label.gather();
  }

  template <class I = size_t>
  knn_model<T,I>
  kneighbors(rowmajor_matrix<T>& enquiry_data,
             int k = 0,
             bool need_distance = true) {
    if (k == 0) k = n_neighbors;
    return knn<T,I>(observation_data, enquiry_data,
                    k, algorithm, metric, need_distance, chunk_size);
  }

  template <class I = size_t, class O = size_t>
  crs_matrix<T, I, O>
  kneighbors_graph(rowmajor_matrix<T>& enquiry_data,
                   int k = 0,
                   const std::string& mode = "connectivity") {
    auto need_distance = (mode == std::string("distance"));
    auto model = kneighbors<I>(enquiry_data, k, need_distance);
    auto nsamples = observation_data.num_row;
    return model.create_graph(mode, nsamples);
  }

  template <class I = size_t>
  dvector<T> predict(rowmajor_matrix<T>& mat);

  template <class I = size_t>
  float score(rowmajor_matrix<T>& mat, dvector<T>& true_label);

  int n_neighbors;
  std::string algorithm;
  std::string metric;
  float chunk_size;
  rowmajor_matrix<T> observation_data;
  std::vector<T> observation_labels;
  SERIALIZE(n_neighbors, algorithm, metric, chunk_size,
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

template <class T>
template <class I>
dvector<T> 
kneighbors_regressor<T>::predict(rowmajor_matrix<T>& mat) {
  auto k = n_neighbors;
  auto need_distance = false;
  auto model = kneighbors<I>(mat, k, need_distance);
  return model.indices.data.map(predict_from_mean<T,I>,
                                broadcast(observation_labels))
                           .template moveto_dvector<T>();
}

template <class T>
template <class I>
float kneighbors_regressor<T>::score(rowmajor_matrix<T>& mat,
                                     dvector<T>& true_label) {
  auto pred_label = predict<I>(mat);
  return r2_score(pred_label.gather(), true_label.gather());
}

}
#endif
