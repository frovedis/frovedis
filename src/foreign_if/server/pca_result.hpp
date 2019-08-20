#ifndef _PCA_RESULT_HPP_
#define _PCA_RESULT_HPP_

namespace frovedis {
struct pca_result {
  pca_result() {}
  pca_result(int m, int n, int k, double noise,
             exrpc_ptr_t comp_ptr, exrpc_ptr_t score_ptr, 
             exrpc_ptr_t eig_ptr,  exrpc_ptr_t var_ratio_ptr,
             exrpc_ptr_t sval_ptr, exrpc_ptr_t mean_ptr):
    n_samples(m), n_features(n), n_components(k), noise(noise),
    comp_ptr(comp_ptr), score_ptr(score_ptr), eig_ptr(eig_ptr),
    var_ratio_ptr(var_ratio_ptr), sval_ptr(sval_ptr), mean_ptr(mean_ptr) {}

  int n_samples, n_features, n_components;
  double noise;
  exrpc_ptr_t comp_ptr, score_ptr, eig_ptr, var_ratio_ptr, sval_ptr, mean_ptr;
  SERIALIZE(n_samples, n_features, n_components, noise,
            comp_ptr, score_ptr, eig_ptr, 
            var_ratio_ptr, sval_ptr, mean_ptr)
};
}

#endif
