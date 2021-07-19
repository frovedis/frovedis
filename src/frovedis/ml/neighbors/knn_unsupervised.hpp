#ifndef _KNN_UNSUPERVISED_HPP_
#define _KNN_UNSUPERVISED_HPP_

#include <frovedis/ml/neighbors/knn.hpp>

namespace frovedis {

// sklearn-like interface
template <class T, class MATRIX1>
struct nearest_neighbors {
  nearest_neighbors(int k, float rad, const std::string& algo,
                    const std::string& met, float c_sz, 
                    double batch_fraction = std::numeric_limits<double>::max()):
    n_neighbors(k), radius(rad), algorithm(algo), metric(met), 
    chunk_size(c_sz), batch_fraction(batch_fraction) {}
  
  void fit(MATRIX1& mat) {
    observation_data = mat;
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
    bool need_distance = true; // need correct distance for graph creation
    auto model = kneighbors<I>(enquiry_data, k, need_distance);
    auto nsamples = observation_data.num_row;
    return model.template create_graph<O>(mode, nsamples); 
  }

  template <class I = size_t, class O = size_t, class MATRIX2 = rowmajor_matrix<T>>
  crs_matrix<T, I, O>
  radius_neighbors(MATRIX2& enquiry_data,
                   float rad = 0) {
    if (rad == 0) rad = radius;
    std::string mode = "distance"; // mode is always distance for radius_neighbors
    return knn_radius<T,T,I,O>(observation_data, enquiry_data,
                               rad, batch_fraction, algorithm, metric, mode); 
  }

  template <class I = size_t, class O = size_t, class MATRIX2 = rowmajor_matrix<T>>
  crs_matrix<T, I, O>
  radius_neighbors_graph(MATRIX2& enquiry_data,
                         float rad = 0,
                         const std::string& mode = "connectivity") {
    if (rad == 0) rad = radius;
    return knn_radius<T,T,I,O>(observation_data, enquiry_data,
                               rad, batch_fraction, algorithm, metric, mode); 
  }

  int n_neighbors;
  float radius;
  std::string algorithm;
  std::string metric;
  float chunk_size;
  double batch_fraction;
  MATRIX1 observation_data;
  SERIALIZE(n_neighbors, radius, algorithm, metric, chunk_size, batch_fraction, observation_data)
};

}
#endif
