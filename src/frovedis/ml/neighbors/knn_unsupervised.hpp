#ifndef _KNN_UNSUPERVISED_HPP_
#define _KNN_UNSUPERVISED_HPP_

#include <frovedis/ml/neighbors/knn.hpp>

namespace frovedis {

// sklearn-like interface
template <class T>
struct nearest_neighbors {
  nearest_neighbors(int k, float rad, const std::string& algo,
                    const std::string& met, float c_sz):
    n_neighbors(k), radius(rad), algorithm(algo), metric(met), 
    chunk_size(c_sz) {}
  
  void fit(rowmajor_matrix<T>& mat) {
    observation_data = mat;
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

  template <class I = size_t, class O = size_t>
  crs_matrix<T, I, O>
  radius_neighbors(rowmajor_matrix<T>& enquiry_data,
                   float rad = 0,
                   bool need_distance = true) {
    if (rad == 0) rad = radius;
    return knn_radius<T,I>(observation_data, enquiry_data,
                           rad, algorithm, metric, need_distance, chunk_size);
  }

  template <class I = size_t, class O = size_t>
  crs_matrix<T, I, O>
  radius_neighbors_graph(rowmajor_matrix<T>& enquiry_data,
                         float rad = 0,
                         const std::string& mode = "connectivity") {
    auto need_distance = (mode == std::string("distance"));
    return radius_neighbors<I>(enquiry_data, rad, need_distance);
  }

  int n_neighbors;
  float radius;
  std::string algorithm;
  std::string metric;
  float chunk_size;
  rowmajor_matrix<T> observation_data;
  SERIALIZE(n_neighbors, radius, algorithm, metric, chunk_size, observation_data)
};

}

#endif
