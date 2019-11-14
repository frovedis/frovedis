#ifndef _KNN_RESULT_HPP_
#define _KNN_RESULT_HPP_

namespace frovedis {
struct knn_result {
  knn_result() {}
  knn_result(int k, 
             exrpc_ptr_t indices_ptr, size_t nrow_ind, size_t ncol_ind, 
             exrpc_ptr_t distances_ptr, size_t nrow_dist, size_t ncol_dist):
             k(k), nrow_ind(nrow_ind), ncol_ind(ncol_ind), 
             nrow_dist(nrow_dist), ncol_dist(ncol_dist), 
             indices_ptr(indices_ptr), distances_ptr(distances_ptr) {}

  int k;
  size_t nrow_ind, ncol_ind, nrow_dist, ncol_dist;
  exrpc_ptr_t indices_ptr, distances_ptr;
  SERIALIZE(k, nrow_ind, ncol_ind, nrow_dist, ncol_dist, indices_ptr,
           distances_ptr)
};
}

#endif
