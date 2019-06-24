#ifndef _SPECTRAL_EMBED_
#define _SPECTRAL_EMBED_

#define SAVE false 
#define SPARSE false

#ifdef SPARSE
#include <frovedis/matrix/shrink_sparse_eigen.hpp>
#else
#include <frovedis/matrix/dense_eigen.hpp>
#endif

#include "common.hpp"
#include "spectral_embedding_model.hpp"
#include <typeinfo>

namespace frovedis {

template <class T>
rowmajor_matrix<T>
get_target_eigen_vector(rowmajor_matrix<T>& mat,
                        int num,
                        int mode) {
  diag_matrix_local<T> eig_val;
  colmajor_matrix<T> eig_vec;
#ifdef SPARSE
  auto smat = mat.to_crs();
  sparse_eigen_sym(smat,eig_val,eig_vec,std::string("LA"),num);
#else
  if (mode == 1) dense_eigen_sym(mat,eig_val,eig_vec,"LA",num); // standard mode
  else if (mode == 3) dense_eigen_sym(mat,eig_val,eig_vec,"LM",num,1.0); // shift-invert mode
  else REPORT_ERROR(USER_ERROR, "Unsupported arpack mode for eigen computation is encountered!\n");
#endif
  //std::cout << "eigen values: \n";  eig_val.debug_print();
  return eig_vec.to_rowmajor();
}

template <class T>
rowmajor_matrix_local<T>
drop_first_column_helper(rowmajor_matrix_local<T>& mat) {
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  rowmajor_matrix_local<T> ret(nrow, ncol-1);
  auto rptr = ret.val.data();
  auto mptr = mat.val.data();
  for(size_t i = 0; i < nrow; ++i) {
    for(size_t j = 1; j < ncol; ++j) {
      auto r_indx = i * (ncol - 1) + (j - 1);
      auto m_indx = i * ncol + j;
      rptr[r_indx] = mptr[m_indx];
    }
  }
  return ret;
}

template <class T>
rowmajor_matrix<T>
drop_first_column(rowmajor_matrix<T>& mat) {
  rowmajor_matrix<T> ret(mat.data.map(drop_first_column_helper<T>));
  ret.num_row = mat.num_row;
  ret.num_col = mat.num_col - 1;
  return ret;
}

template <class T>
rowmajor_matrix<T>
compute_spectral_embedding(rowmajor_matrix<T>& aff, //affinity matrix
                           int n_comp = 2,
                           bool norm_laplacian = true,
                           bool drop_first = true,
                           int mode = 1) {
  time_spent diag_t(INFO), laplace_t(INFO);
  time_spent embed_t(INFO), embed_norm_t(INFO);

  auto myst = get_start_indices(aff);
  if(drop_first) n_comp++;

  diag_t.lap_start();
  auto con_diag_loc = construct_connectivity_diagonals<T>(aff, myst);
  if (norm_laplacian) con_diag_loc.mapv(one_by_sqrt_inplace<T>);
  auto con_diag_g = con_diag_loc.template moveto_dvector<T>().gather();
  //std::cout << "connectivity diagonal: \n"; 
  //for(auto& t: con_diag_g) std::cout << t << " "; std::cout << std::endl;
  auto con_diag = broadcast(con_diag_g);
  diag_t.lap_stop();
  diag_t.show_lap("diagonal computation time: ");

  laplace_t.lap_start();
  auto laplace = construct_laplace_matrix(aff,con_diag,myst,norm_laplacian); // L= D - A
  if(SAVE) laplace.save("./dump/laplace");
  laplace.data.mapv(negate_inplace<T>); // negating as done in sklearn
  laplace_t.lap_stop();
  laplace_t.show_lap("laplace computation time: ");

  embed_t.lap_start();
  auto eig_vec = get_target_eigen_vector<T>(laplace,n_comp,mode);
  if(SAVE) eig_vec.save("./dump/eig_vec");
  embed_t.lap_stop();
  embed_t.show_lap("eigen computation time: ");

  embed_norm_t.lap_start();
  auto embed = construct_embed_matrix(eig_vec,con_diag,myst,norm_laplacian);
  embed_norm_t.lap_stop();
  embed_norm_t.show_lap("embed computation time: ");

  if(drop_first) {
    if(SAVE) embed.save("./dump/embed_compl");
    return drop_first_column(embed);
  }
  else return embed;
}

template <class T>
spectral_embedding_model<T>
spectral_embedding_impl(rowmajor_matrix<T>& mat, 
                        int n_comp = 2,
                        bool norm_laplacian = true,
                        bool precomputed = false,
                        bool drop_first = true,
                        double gamma = 1.0,
                        int mode = 1,
                        bool input_movable = false) {
  auto nrow = mat.num_row;
  auto ncol = mat.num_col;
  rowmajor_matrix<T> embed, affinity;

  if(precomputed) { // 'mat' is precomputed 'affinity'
    if(nrow != ncol) 
      REPORT_ERROR(USER_ERROR, "Precomputed affinity matrix is not a square matrix\n");
    if(input_movable) affinity = std::move(mat);
    else affinity = mat;
    embed = compute_spectral_embedding(mat,n_comp,norm_laplacian,drop_first,mode);
  }
  else { // 'mat' is input data
    time_spent aff_t(INFO);
    aff_t.lap_start();
    auto gdata = get_global_data(mat);
    if(input_movable) mat.clear();
    affinity = construct_distance_matrix(gdata,true); //locally created "gdata" is movable
    construct_affinity_matrix_inplace(affinity,gamma);
    if(SAVE) affinity.save("./dump/dmat");
    aff_t.lap_stop();
    aff_t.show_lap("affinity computation time: ");
    embed = compute_spectral_embedding(affinity,n_comp,norm_laplacian,drop_first,mode);
  }
  if(SAVE) embed.save("./dump/embed");
  spectral_embedding_model<T> model;
  model.embed_matrix = std::move(embed);
  model.affinity_matrix = std::move(affinity);
  return model;
}

template <class T>
spectral_embedding_model<T>
spectral_embedding(rowmajor_matrix<T>& mat, 
                   int n_comp = 2,
                   bool norm_laplacian = true,
                   bool precomputed = false,
                   bool drop_first = true,
                   double gamma = 1.0,
                   int mode = 1) {
  return spectral_embedding_impl(mat,n_comp,norm_laplacian,
                                 precomputed,drop_first,gamma,mode,false);
}
                   
template <class T>
spectral_embedding_model<T>
spectral_embedding(rowmajor_matrix<T>&& mat, 
                   int n_comp = 2,
                   bool norm_laplacian = true,
                   bool precomputed = false,
                   bool drop_first = true,
                   double gamma = 1.0,
                   int mode = 1) {
  return spectral_embedding_impl(mat,n_comp,norm_laplacian,
                                 precomputed,drop_first,gamma,mode,true);
}

}
#endif
