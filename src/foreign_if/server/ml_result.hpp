#ifndef _ML_RESULT_HPP_
#define _ML_RESULT_HPP_

using namespace frovedis;

struct lu_fact_result {
  lu_fact_result() {}
  lu_fact_result(exrpc_ptr_t ptr, int stat): 
    ipiv_ptr(ptr), info(stat) {}
  
  exrpc_ptr_t ipiv_ptr;
  int info;
  SERIALIZE(ipiv_ptr, info)
};

struct svd_result {
  svd_result() {}
  svd_result(exrpc_ptr_t sptr, exrpc_ptr_t uptr, exrpc_ptr_t vptr,
             int mm, int nn, int kk, int stat) :
    svec_ptr(sptr), umat_ptr(uptr), vmat_ptr(vptr),
    m(mm), n(nn), k(kk), info(stat) {}

  exrpc_ptr_t svec_ptr, umat_ptr, vmat_ptr;
  int m, n, k, info;
  SERIALIZE(svec_ptr,umat_ptr,vmat_ptr,m,n,k,info)
};

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

struct kmeans_result {
  kmeans_result() { n_iter_ = 0; } // to suppress compiler warning in spark wrapper
  kmeans_result(const std::vector<int>& label, 
                const int n_iter, 
                const float inertia,
                const int n_clusters,
                exrpc_ptr_t trans_mat_ptr,
                size_t trans_mat_nsamples):
    label_(label), n_iter_(n_iter), inertia_(inertia),
    n_clusters_(n_clusters), 
    trans_mat_ptr(trans_mat_ptr),
    trans_mat_nsamples(trans_mat_nsamples) {}

  std::vector<int> label_;
  int n_iter_;
  float inertia_;
  int n_clusters_;
  exrpc_ptr_t trans_mat_ptr;
  size_t trans_mat_nsamples;
  SERIALIZE(label_, n_iter_, inertia_, n_clusters_, 
            trans_mat_ptr, trans_mat_nsamples)
};
    
struct gmm_result {
  gmm_result() { n_iter_ = 0; } // to suppress compiler warning in spark wrapper
  gmm_result(const int n_iter, 
             const double likelihood): 
             n_iter_(n_iter), 
             likelihood_(likelihood){}
  int n_iter_;
  double likelihood_;
  SERIALIZE(n_iter_, likelihood_);
};

template <class T>
struct lnr_result {
  lnr_result() { n_iter = 0; } // to suppress compiler warning in spark wrapper
  lnr_result(int n_iter,
             int rank, std::vector<T>& sval):
             n_iter(n_iter), rank(rank) {
    singular.swap(sval); // assumes sval is no longer required at caller side
  }
  int n_iter, rank;
  std::vector<T> singular;
  SERIALIZE(n_iter, rank, singular);
};


#endif
