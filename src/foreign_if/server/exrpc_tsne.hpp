#ifndef _EXRPC_TSNE_HPP_
#define _EXRPC_TSNE_HPP_

#include "frovedis.hpp"
#include "frovedis/matrix/tsne.hpp"
#include "../exrpc/exrpc_expose.hpp"
#include "tsne_result.hpp"

using namespace frovedis;

template <class MATRIX, class T>
//template <class T>
tsne_result frovedis_tsne(exrpc_ptr_t& data_ptr,
                          double& perplexity = 30.0, 
                          double& early_exaggeration = 12.0,
                          double& min_grad_norm = 1e-7,
                          double& learning_rate = 200.0,
                          size_t& n_components = 2,
                          size_t& n_iter = 1000,
                          size_t& n_iter_without_progress = 300,
                          std::string& metric = "euclidean",
                          bool& verbose = false) {
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);
  // output attributes
  auto nrows = mat.num_row;
  size_t iter_cnt = 0;
  double kl_div = 0.0;

  // calling frovedis::tsne(...)
  auto res = tsne(mat, perplexity, early_exaggeration,
                  min_grad_norm, learning_rate, n_components,
                  n_iter, n_iter_without_progress, metric,
                  verbose, iter_cnt, kl_div);
  auto embeddings = new rowmajor_matrix<T>(std::move(res));

#ifdef _EXRPC_DEBUG_
  std::cout << " embeddings_ \n"; embeddings.debug_print();
  std::cout << " n_iter_ : "<<iter_cnt<<std::endl;
  std::cout << " kl_divergence_ : "<<kl_div<<std::endl;
#endif
  // handling output attributes
  auto emb_ptr = reinterpret_cast<exrpc_ptr_t>(embeddings);

  return tsne_result(nrows, n_components, iter_cnt, kl_div, emb_ptr);
}
#endif
