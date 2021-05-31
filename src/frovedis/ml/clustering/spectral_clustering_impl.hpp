#ifndef _SPECTRAL_CLUSTER_IMPL_
#define _SPECTRAL_CLUSTER_IMPL_

#include "spectral_embedding.hpp"
#include "kmeans.hpp"
#include "spectral_clustering_model.hpp"

namespace frovedis {

template <class T>
std::vector<int>
compute_spectral_cluster(rowmajor_matrix<T>& mat, // affinity matrix
                         int ncluster = 2,
                         int n_comp = 2,
                         int niter = 100,
                         double eps = 0.01,
                         bool norm_laplacian = true,
                         int mode = 1,
                         bool drop_first = false) {
  auto embed = compute_spectral_embedding(mat,n_comp,norm_laplacian,drop_first,mode);
  time_spent kmeans_t(DEBUG);
  auto clsf = KMeans<T>(ncluster).set_max_iter(niter).set_eps(eps);
  auto labels = clsf.fit_predict(embed);
  kmeans_t.show("kmeans time: ");
  if(SAVE) clsf.cluster_centers_().save("./dump/centroid");
  return labels;
}

template <class T>
spectral_clustering_model<T>
spectral_clustering_impl(rowmajor_matrix<T>& mat,
                         int ncluster = 2,
                         int n_comp = 2,
                         int niter = 100,
                         double eps = 0.01,
                         bool norm_laplacian = true,
                         bool precomputed = false,
                         double gamma = 1.0,
                         int mode = 1,
                         bool drop_first = false,
                         bool input_movable = false) {
  // debug for the hyper-parameters
  std::cout << "ncluster: "       << ncluster << "; "
            << "n_comp: "         << n_comp << "; " 
            << "niter: "          << niter << "; " 
            << "eps: "            << eps << "; " 
            << "norm_laplacian: " << norm_laplacian << "; " 
            << "precomputed: "    << precomputed << "; " 
            << "gamma: "          << gamma << "; " 
            << "mode: "           << mode << "; " 
            << "drop_first: "     << drop_first << "; " 
            << "input_movable: "  << input_movable << std::endl; 

  auto nrow = mat.num_row;   
  auto ncol = mat.num_col;   
  checkAssumption (ncluster >= 1 && ncluster <= nrow);
  checkAssumption (n_comp >= 1 && n_comp <= ncol);
 
  rowmajor_matrix<T> affinity;
 
  if(precomputed) { // 'mat' is precomputed 'affinity'
    if(nrow != ncol)
      REPORT_ERROR(USER_ERROR, "Precomputed affinity matrix is not a square matrix\n");
    if(input_movable) affinity = std::move(mat);
    else affinity = mat;
  }
  else { // 'mat' is input data
    time_spent aff_t(DEBUG);
    auto gdata = get_global_data(mat);
    if(input_movable) mat.clear();
    aff_t.lap_start();
    affinity = construct_distance_matrix<T>(gdata,true); //locally created "gdata" is movable
    aff_t.lap_stop();
    if(SAVE) affinity.save("./dump/distance");
    aff_t.lap_start();
    construct_affinity_matrix_inplace(affinity,gamma);
    aff_t.lap_stop();
    aff_t.show_lap("affinity computation time: ");
    //std::cout << "affinity: \n"; affinity.debug_print(10);
    if(SAVE) affinity.save("./dump/affinity");
  }

  // quick return cases...
  std::vector<int> labels;
  if (ncluster == 1) labels = alloc_vector<int>(nrow,false);
  else if (ncluster == nrow) labels = alloc_vector<int>(nrow,true);
  else labels = compute_spectral_cluster(affinity,ncluster,n_comp,
                                         niter,eps,
                                         norm_laplacian,mode,drop_first);
  spectral_clustering_model<T> model;
  model.affinity_matrix = std::move(affinity);
  model.labels.swap(labels);
  model.nclusters = ncluster;
  return model;
}

template <class T>
spectral_clustering_model<T>
spectral_clustering_train(rowmajor_matrix<T>& mat,
                          int ncluster = 2,
                          int n_comp = 2,
                          int niter = 100,
                          double eps = 0.01,
                          bool norm_laplacian = true,
                          bool precomputed = false,
                          bool drop_first = false,
                          double gamma = 1.0,
                          int mode = 1) {
  return spectral_clustering_impl(mat,ncluster,n_comp,niter,eps,
                                  norm_laplacian,precomputed,gamma,mode,drop_first,false);
}

template <class T>
spectral_clustering_model<T>
spectral_clustering_train(rowmajor_matrix<T>&& mat,
                          int ncluster = 2,
                          int n_comp = 2,
                          int niter = 100,
                          double eps = 0.01,
                          bool norm_laplacian = true,
                          bool precomputed = false,
                          bool drop_first = false,
                          double gamma = 1.0,
                          int mode = 1) {
  return spectral_clustering_impl(mat,ncluster,n_comp,niter,eps,
                                  norm_laplacian,precomputed,gamma,mode,drop_first,true);
}

}
#endif
