#ifndef _SPECTRAL_CLUSTER_IMPL_
#define _SPECTRAL_CLUSTER_IMPL_

#include "kmeans.hpp"
#include "spectral_embedding.hpp"
#include "spectral_clustering_model.hpp"
#include "../neighbors/knn.hpp"

namespace frovedis {

// TODO: implement for crs graph
template <class T>
rowmajor_matrix<T>
compute_joint_probability(rowmajor_matrix<T>& mat) {
  auto nrow = mat.num_row;
  auto ncol = mat.num_col;
  require(nrow == ncol, "compute_joint_probability: input is not a square matrix!\n");
  auto tmat = mat.transpose();
  mat.data.mapv(+[](rowmajor_matrix_local<T>& m1,
                    rowmajor_matrix_local<T>& m2) {
    require(m1.val.size() == m2.val.size(), "matrix size differs!\n");
    auto sz = m1.val.size();
    auto m1p = m1.val.data();
    auto m2p = m2.val.data();
    for(size_t i = 0; i < sz; ++i) m2p[i] = 0.5 * (m1p[i] + m2p[i]);
  }, tmat.data); // tmat would get inplace updated
  return tmat;
}

template <class T>
spectral_clustering_model<T>
spectral_clustering_impl(rowmajor_matrix<T>& mat,
                         KMeans<T>& assign,
                         int n_comp = 2,
                         double gamma = 1.0,
                         const std::string& affinity = "rbf",
                         int n_neighbors = 10,
                         bool norm_laplacian = true,
                         bool drop_first = true,
                         int mode = 3,
                         bool input_movable = false) {
  // debug for the hyper-parameters
  RLOG(INFO)  << "n_cluster: "      << assign.k << "; "
              << "n_comp: "         << n_comp << "; " 
              << "max_iter: "       << assign.max_iter << "; " 
              << "n_init: "         << assign.n_init << "; " 
              << "seed: "           << assign.seed << "; " 
              << "eps: "            << assign.eps << "; " 
              << "gamma: "          << gamma << "; " 
              << "affinity: "       << affinity << "; " 
              << "n_neighbors: "    << n_neighbors << "; " 
              << "norm_laplacian: " << norm_laplacian << "; " 
              << "drop_first: "     << drop_first << "; " 
              << "mode: "           << mode << "; " 
              << "input_movable: "  << input_movable << std::endl; 

  auto nrow = mat.num_row;   
  auto ncol = mat.num_col;   
  require(assign.k >= 1 && assign.k <= nrow, 
  "no. of clusters should be in between 1 to nsamples!\n");
  require(n_comp >= 1 && n_comp <= ncol,
  "no. of components should be in between 1 to nfeatures!\n");
 
  time_spent aff_t(DEBUG);
  rowmajor_matrix<T> affinity_mat;
 
  if(affinity == "precomputed") { // 'mat' is precomputed 'affinity'
    if(nrow != ncol)
      REPORT_ERROR(USER_ERROR, "Precomputed affinity matrix is not a square matrix\n");
    affinity_mat = input_movable ? std::move(mat) : mat;
  } else if (affinity == "nearest_neighbors") { 
    require(n_neighbors >= 1 && n_neighbors <= nrow,
    "no. of neighbors should be in between 1 to nsamples!\n");
    aff_t.lap_start();
    auto knn_model = knn<T>(mat, n_neighbors);
    auto connectivity = knn_model.create_graph("connectivity", nrow).to_rowmajor();
    affinity_mat = compute_joint_probability(connectivity);
    aff_t.lap_stop();
    aff_t.show_lap("affinity computation time: ");
    if(SAVE) affinity_mat.save("./dump/affinity");
  } else if (affinity == "rbf") { 
    auto gdata = mat.gather();
    if(input_movable) mat.clear();
    aff_t.lap_start();
    affinity_mat = construct_distance_matrix<T>(gdata, true); //locally created "gdata" is movable
    aff_t.lap_stop();
    if(SAVE) affinity_mat.save("./dump/distance");
    aff_t.lap_start();
    construct_affinity_matrix_inplace(affinity_mat, gamma);
    aff_t.lap_stop();
    aff_t.show_lap("affinity computation time: ");
    if(SAVE) affinity_mat.save("./dump/affinity");
  }
  else REPORT_ERROR(USER_ERROR, "Unsupported affinity value is provided!\n");

  // quick return cases...
  std::vector<int> labels;
  if (assign.k == 1) labels = vector_zeros<int>(nrow);
  else if (assign.k == nrow) labels = vector_arrange<int>(nrow);
  else {
    auto embed = compute_spectral_embedding(affinity_mat, n_comp, 
                                            norm_laplacian, drop_first, mode);
    time_spent kmeans_t(DEBUG);
    labels = assign.fit_predict(embed);
    kmeans_t.show("kmeans time: ");
    if(SAVE) assign.cluster_centers_().save("./dump/centroid");
  }
  spectral_clustering_model<T> model;
  model.affinity_matrix = std::move(affinity_mat);
  model.labels.swap(labels);
  model.nclusters = assign.k;
  return model;
}

template <class T>
spectral_clustering_model<T>
spectral_clustering_train(rowmajor_matrix<T>& mat,
                          int ncluster = 2,
                          int n_comp = 2,
                          int niter = 300,
                          int n_init = 1,
                          double eps = 0.0001,
                          int seed = 0,
                          double gamma = 1.0,
                          const std::string& affinity = "rbf",
                          int n_neighbors = 10,
                          bool norm_laplacian = true,
                          bool drop_first = true,
                          int mode = 3) {
  auto assign = KMeans<T>().set_k(ncluster)
                           .set_max_iter(niter)
                           .set_n_init(n_init)
                           .set_seed(seed)
                           .set_eps(eps);
  return spectral_clustering_impl(mat, assign, n_comp, gamma, affinity, n_neighbors,
                                  norm_laplacian, drop_first, mode, false);
}

template <class T>
spectral_clustering_model<T>
spectral_clustering_train(rowmajor_matrix<T>&& mat,
                          int ncluster = 2,
                          int n_comp = 2,
                          int niter = 300,
                          int n_init = 1,
                          double eps = 0.0001,
                          int seed = 0,
                          double gamma = 1.0,
                          const std::string& affinity = "rbf",
                          int n_neighbors = 10,
                          bool norm_laplacian = true,
                          bool drop_first = true,
                          int mode = 3) {
  auto assign = KMeans<T>().set_k(ncluster)
                           .set_max_iter(niter)
                           .set_n_init(n_init)
                           .set_seed(seed)
                           .set_eps(eps);
  return spectral_clustering_impl(mat, assign, n_comp, gamma, affinity, n_neighbors,
                                  norm_laplacian, drop_first, mode, true);
}

}
#endif
