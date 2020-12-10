#ifndef _TSNE_RESULT_HPP_
#define _TSNE_RESULT_HPP_

namespace frovedis {
struct tsne_result {
  tsne_result() {}
  tsne_result(size_t m, size_t k, size_t n_iter_, double kl_divergence_,
              exrpc_ptr_t embedding_ptr):
    n_samples(m), n_comps(k),
    n_iter_(n_iter_), kl_divergence_(kl_divergence_), 
    embedding_ptr(embedding_ptr) {}

  size_t n_samples, n_comps, n_iter_;
  double kl_divergence_;
  exrpc_ptr_t embedding_ptr;
  SERIALIZE(n_samples, n_comps, n_iter_, kl_divergence_, 
            embedding_ptr)
};
}
#endif
