#ifndef _DUMMY_MATRIX_HPP_
#define _DUMMY_MATRIX_HPP_

namespace frovedis {
struct dummy_matrix {
  dummy_matrix() {}
  dummy_matrix(exrpc_ptr_t ptr, size_t nr, size_t nc): 
    mptr(ptr), nrow(nr), ncol(nc) {}

  exrpc_ptr_t mptr;
  size_t nrow, ncol;
  SERIALIZE(mptr, nrow, ncol)
};
}
#endif
