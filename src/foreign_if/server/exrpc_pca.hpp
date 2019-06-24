#ifndef _EXRPC_PCA_HPP_
#define _EXRPC_PCA_HPP_

#include "frovedis.hpp"
#include "frovedis/matrix/pca.hpp"
#include "../exrpc/exrpc_expose.hpp"
#include "pca_result.hpp"

using namespace frovedis;

template <class T>
void rearrange_pca_output(colmajor_matrix<T>& components,
                          std::vector<T>& variance) {
  auto ncomp = variance.size();
  auto sptr = variance.data();
  for(size_t i = 0; i < ncomp/2; ++i) {
    auto temp = sptr[i];
    sptr[i] = sptr[ncomp - i - 1];
    sptr[ncomp - i - 1] = temp;
  }
  // rearrange_colmajor_data_inplace: defined in exrpc_svd.hpp
  components.data.mapv(rearrange_colmajor_data_inplace<T>);
}


template <class MATRIX, class T>
pca_result frovedis_pca(exrpc_ptr_t& data_ptr, int& k, 
                        bool& isMovableInput = false,
                        bool& rearrange_out = true) {
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);      
  auto pca_directions = new colmajor_matrix<T>(); 
  auto explained_variance_ratio = new std::vector<T>();  
  colmajor_matrix<T> pca_scores;
  std::vector<T> eigen_values, singular_values;
  pca(mat,*pca_directions,pca_scores,
      eigen_values,*explained_variance_ratio,
      singular_values,k);
#ifdef _EXRPC_DEBUG_
  std::cout << "components: \n"; pca_directions->debug_print();
  std::cout << "ratio: \n"; pca_scores.debug_print();
  std::cout << "eigen values: \n"; 
  for(auto e: eigen_values) std::cout << e << " "; std::cout << std::endl;
  std::cout << "variance ratio: \n";
  for(auto e: *explained_variance_ratio) std::cout << e << " "; std::cout << std::endl;
  std::cout << "singular values: \n";
  for(auto e: singular_values) std::cout << e << " "; std::cout << std::endl;
#endif
  // if input is movable, destroying Frovedis side data after computation is done.
  if (isMovableInput)  mat.clear();
  if (rearrange_out) rearrange_pca_output(*pca_directions,*explained_variance_ratio); 
  auto mptr = reinterpret_cast<exrpc_ptr_t>(pca_directions);
  auto vptr = reinterpret_cast<exrpc_ptr_t>(explained_variance_ratio);
  auto nrows = pca_directions->num_row;
  auto ncols = pca_directions->num_col;
  return pca_result(mptr,nrows,ncols,vptr,k);
}

#endif
