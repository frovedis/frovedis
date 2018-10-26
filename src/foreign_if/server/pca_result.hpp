#ifndef _PCA_RESULT_HPP_
#define _PCA_RESULT_HPP_

namespace frovedis {
struct pca_result {
  pca_result() {}
  pca_result(exrpc_ptr_t mptr, int nr, int nc,
             exrpc_ptr_t vptr, int k_) :
    pc_ptr(mptr), nrows(nr), ncols(nc), var_ptr(vptr), k(k_) {}

  exrpc_ptr_t pc_ptr;
  int nrows, ncols;
  exrpc_ptr_t var_ptr;
  int k;
  SERIALIZE(pc_ptr,nrows,ncols,var_ptr,k)
};
}

#endif
