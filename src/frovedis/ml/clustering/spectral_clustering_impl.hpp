#ifndef _SPECTRAL_CLUSTER_IMPL_
#define _SPECTRAL_CLUSTER_IMPL_

#include "kmeans.hpp"
#include "spectral_embedding.hpp"
#include "spectral_clustering_model.hpp"
#include <frovedis/core/vector_operations.hpp>
#include "../neighbors/knn.hpp"

namespace frovedis {

template <class T, class I, class O>
crs_matrix_local<T,I,O>    
compute_joint_probability_helper(const crs_matrix_local<T,I,O>& a, 
                                 const crs_matrix_local<T,I,O>& b) {  
  require(a.local_num_row == b.local_num_row &&
          a.local_num_col == b.local_num_col, 
  "compute_joint_probability: local matrix dimensions must be same!\n");
 
  auto a_nrow = a.local_num_row;
  auto a_sz = a.val.size();
  auto aidxp = a.idx.data();
  auto aoffp = a.off.data();
    
  auto b_nrow = b.local_num_row;
  auto b_sz = b.val.size();      
  auto bidxp = b.idx.data();    
  auto boffp = b.off.data();
    
  // find max of ncol
  auto maxA = vector_amax(a.idx);
  auto maxB = vector_amax(b.idx);
  auto max_ncol = std::max(maxA, maxB) + 1;
  
  // step1: create global_ids of size_t type (physical id can be large enough...)
  // from current row and idx
  std::vector<size_t> aidx_global(a_sz), bidx_global(b_sz); 
  auto aidx_gp = aidx_global.data();
  auto bidx_gp = bidx_global.data();
  for(size_t row = 0; row < a_nrow; ++row) {  
    for(O pos = aoffp[row]; pos < aoffp[row + 1]; pos++) {
      aidx_gp[pos] = row * max_ncol + aidxp[pos];
    }    
  }
  for(size_t row = 0; row < b_nrow; ++row) {
    for(O pos = boffp[row]; pos < boffp[row + 1]; pos++) {
      bidx_gp[pos] = row * max_ncol + bidxp[pos];
    }    
  }  

  // step2: concat
  auto concat_val = vector_concat(a.val, b.val);
  auto concat_id = vector_concat(aidx_global, bidx_global);
  vector_clear(aidx_global); // freeing memory after concat
  vector_clear(bidx_global); // freeing memory after concat
    
  // step3: find unique ids
  radix_sort(concat_id, concat_val);
  auto sep = set_separate(concat_id);
  auto sepsz = sep.size(); 
  auto unqsz = sepsz - 1;
  std::vector<size_t> unq_val(unqsz), unq_pos(unqsz), unq_cnt(unqsz);
  auto sepp = sep.data();  
  auto concat_idp = concat_id.data();
  auto unq_valp = unq_val.data();
  auto unq_posp = unq_pos.data();
  auto unq_cntp = unq_cnt.data();      
  for(size_t i = 0; i < unqsz; ++i) {
    unq_valp[i] = concat_idp[sepp[i]];
    unq_posp[i] = sepp[i];
    unq_cntp[i] = sepp[i + 1] - sepp[i];
  }
  
  // step4: sparse-vector addition of all the rows at a go...
  std::vector<T> val(unqsz);
  std::vector<I> cid(unqsz);
  std::vector<O> rid(unqsz);    
  auto valp = val.data();  
  auto ridp = rid.data();  
  auto cidp = cid.data();    
  auto concat_valp = concat_val.data();  
  for(size_t i = 0; i < unqsz; ++i) {
    auto pos = unq_posp[i];
    valp[i] = concat_valp[pos];
    cidp[i] = unq_valp[i] % max_ncol;
    ridp[i] = unq_valp[i] / max_ncol;
  }    

  size_t group = 2;
  auto grp2 = vector_find_eq(unq_cnt, group); // check all with count 2
  auto grp2p = grp2.data();  
  auto grp2sz = grp2.size();
  #pragma _NEC ivdep
  for(size_t i = 0; i < grp2sz; ++i) {
    auto tid = grp2p[i];
    auto id = unq_posp[tid];
    valp[tid] += concat_valp[id + 1];
  }
    
  for(size_t i = 0; i < unqsz; ++i) valp[i] *= 0.5; // joined probability calculation  

  // step5: resultant matrix construction
  crs_matrix_local<T,I,O> ret;
  ret.val.swap(val);
  ret.idx.swap(cid);
  ret.off = set_separate(rid);
  ret.local_num_row = a.local_num_row;
  ret.local_num_col = b.local_num_col;
  return ret;  
}    
           
template <class T, class I, class O>
crs_matrix<T,I,O>
compute_joint_probability(crs_matrix<T,I,O>& mat) {
  auto nrow = mat.num_row;
  auto ncol = mat.num_col;
  require(nrow == ncol, "compute_joint_probability: input is not a square matrix!\n");
  auto tmat = mat.transpose();
  //align local matrices with same distribution 
  auto sizes = mat.get_local_num_rows();
  tmat.align_as(sizes);
  crs_matrix<T,I,O> ret = mat.data.map(compute_joint_probability_helper<T,I,O>, 
                                       tmat.data);
  ret.num_row = nrow;
  ret.num_col = ncol;
  return ret;
}
    
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
  spectral_clustering_model<T> model;
 
  if(affinity == "precomputed") { // 'mat' is precomputed 'affinity'
    if(nrow != ncol)
      REPORT_ERROR(USER_ERROR, "Precomputed affinity matrix is not a square matrix\n");
    model.is_dense_affinity = true;   
    model.dense_affinity_matrix = input_movable ? std::move(mat) : mat;
  } else if (affinity == "nearest_neighbors") { 
    require(n_neighbors >= 1 && n_neighbors <= nrow,
    "no. of neighbors should be in between 1 to nsamples!\n");
    aff_t.lap_start();
    auto knn_model = knn<T>(mat, n_neighbors);
    auto connectivity = knn_model.create_graph("connectivity", nrow);
    model.is_dense_affinity = false;   
    model.sparse_affinity_matrix = \
      compute_joint_probability(connectivity).template change_datatype<T>();
    aff_t.lap_stop();
    aff_t.show_lap("affinity computation time: ");
    if(SAVE) model.dense_affinity_matrix.save("./dump/affinity");
  } else if (affinity == "rbf") { 
    auto gdata = mat.gather();
    if(input_movable) mat.clear();
    aff_t.lap_start();
    model.dense_affinity_matrix = construct_distance_matrix<T>(gdata, true); //locally created "gdata" is movable
    aff_t.lap_stop();
    if(SAVE) model.dense_affinity_matrix.save("./dump/distance");
    aff_t.lap_start();
    construct_affinity_matrix_inplace(model.dense_affinity_matrix, gamma);
    model.is_dense_affinity = true; 
    aff_t.lap_stop();
    aff_t.show_lap("affinity computation time: ");
    if(SAVE) model.dense_affinity_matrix.save("./dump/affinity");
  }
  else REPORT_ERROR(USER_ERROR, "Unsupported affinity value is provided!\n");

  // quick return cases...
  std::vector<int> labels;
  if (assign.k == 1) labels = vector_zeros<int>(nrow);
  else if (assign.k == nrow) labels = vector_arrange<int>(nrow);
  else {
    rowmajor_matrix<T> embed;
    if(model.is_dense_affinity) { 
      embed = compute_spectral_embedding<T>(model.dense_affinity_matrix, n_comp, 
                                            norm_laplacian, drop_first, mode);
    }
    else {
      embed = compute_spectral_embedding<T>(model.sparse_affinity_matrix, n_comp, 
                                            norm_laplacian, drop_first, mode);    
    }  
    time_spent kmeans_t(DEBUG);
    labels = assign.fit_predict(embed);
    kmeans_t.show("kmeans time: ");
    if(SAVE) assign.cluster_centers_().save("./dump/centroid");
  }
  model.labels.swap(labels);
  model.nclusters = assign.k;
  return model;
}
    
template <class T, class I, class O>
spectral_clustering_model<T>
spectral_clustering_impl(crs_matrix<T,I,O>& mat,
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
  spectral_clustering_model<T> model;
    
  if(affinity == "precomputed") { // 'mat' is precomputed 'affinity'
    if(nrow != ncol)
      REPORT_ERROR(USER_ERROR, "Precomputed affinity matrix is not a square matrix\n");
    model.is_dense_affinity = false;   
    model.sparse_affinity_matrix = mat.template change_datatype<T>();
  } else if (affinity == "nearest_neighbors") { 
    require(n_neighbors >= 1 && n_neighbors <= nrow,
    "no. of neighbors should be in between 1 to nsamples!\n");
    aff_t.lap_start();
    auto knn_model = knn<T>(mat, n_neighbors);
    auto connectivity = knn_model.create_graph("connectivity", nrow);
    model.is_dense_affinity = false;   
    model.sparse_affinity_matrix = \
      compute_joint_probability(connectivity).template change_datatype<T>();
    aff_t.lap_stop();
    aff_t.show_lap("affinity computation time: ");
    if(SAVE) model.dense_affinity_matrix.save("./dump/affinity");
  } else if (affinity == "rbf") { 
    auto gdata = mat.gather();
    if(input_movable) mat.clear();
    aff_t.lap_start();
    model.dense_affinity_matrix = construct_distance_matrix<T>(gdata, true); //locally created "gdata" is movable
    aff_t.lap_stop();
    if(SAVE) model.dense_affinity_matrix.save("./dump/distance");
    aff_t.lap_start();
    construct_affinity_matrix_inplace(model.dense_affinity_matrix, gamma);
    model.is_dense_affinity = true;  
    aff_t.lap_stop();
    aff_t.show_lap("affinity computation time: ");
    if(SAVE) model.dense_affinity_matrix.save("./dump/affinity");
  }
  else REPORT_ERROR(USER_ERROR, "Unsupported affinity value is provided!\n");

  // quick return cases...
  std::vector<int> labels;
  if (assign.k == 1) labels = vector_zeros<int>(nrow);
  else if (assign.k == nrow) labels = vector_arrange<int>(nrow);
  else {
    rowmajor_matrix<T> embed;
    if(model.is_dense_affinity) { 
      embed = compute_spectral_embedding<T>(model.dense_affinity_matrix, n_comp, 
                                            norm_laplacian, drop_first, mode);
    }
    else {
      embed = compute_spectral_embedding<T>(model.sparse_affinity_matrix, n_comp, 
                                            norm_laplacian, drop_first, mode);    
    }  
    time_spent kmeans_t(DEBUG);
    labels = assign.fit_predict(embed);
    kmeans_t.show("kmeans time: ");
    if(SAVE) assign.cluster_centers_().save("./dump/centroid");
  }
  model.labels.swap(labels);
  model.nclusters = assign.k;
  return model;
}    
    

template <class T, class MATRIX>
spectral_clustering_model<T>
spectral_clustering_train(const MATRIX& mat,
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

template <class T, class MATRIX>
spectral_clustering_model<T>
spectral_clustering_train(MATRIX&& mat,
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
