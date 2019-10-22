#ifndef _DUMMY_MATRIX_HPP_
#define _DUMMY_MATRIX_HPP_

namespace frovedis {
struct dummy_matrix {
  dummy_matrix() {}

  dummy_matrix(exrpc_ptr_t ptr, size_t nr, size_t nc): 
    mptr(ptr), nrow(nr), ncol(nc), active_elems(nr * nc) {}

  dummy_matrix(exrpc_ptr_t ptr, size_t nr, size_t nc, size_t n_nz): 
    mptr(ptr), nrow(nr), ncol(nc), active_elems(n_nz) {}

  exrpc_ptr_t mptr;
  size_t nrow, ncol, active_elems;
  SERIALIZE(mptr, nrow, ncol, active_elems)
};

template <class L_MATRIX>
size_t get_local_val_size(L_MATRIX& lmat) { return lmat.val.size(); }

template <class T>
T sum_size(T& x, T& y) { return x + y; }

// converts exrpc::matrix to exrpc::dummy_matrix
template <class MATRIX, class L_MATRIX>
dummy_matrix to_dummy_matrix(MATRIX* mptr) {
  size_t nr = mptr->num_row;
  size_t nc = mptr->num_col;
  size_t n_nz = mptr->data.map(get_local_val_size<L_MATRIX>)
                          .reduce(sum_size<size_t>);
  auto mptr_ = reinterpret_cast<exrpc_ptr_t>(mptr);
  return dummy_matrix(mptr_,nr,nc,n_nz);
}

// converts exrpc_ptr_t of a frovedis matrix to exrpc::dummy_matrix
template <class MATRIX, class L_MATRIX>
dummy_matrix to_dummy_matrix(exrpc_ptr_t& mptr) {
  auto& mat = *reinterpret_cast<MATRIX *>(mptr);
  size_t nr = mat.num_row;
  size_t nc = mat.num_col;
  size_t n_nz = mat.data.map(get_local_val_size<L_MATRIX>)
                        .reduce(sum_size<size_t>);
  return dummy_matrix(mptr,nr,nc,n_nz);
}

struct dummy_vector {
  dummy_vector() {}
  dummy_vector(exrpc_ptr_t ptr, size_t sz, short dt):
    vptr(ptr), size(sz), dtype(dt) {}
  exrpc_ptr_t vptr;
  size_t size;
  short dtype;
  SERIALIZE(vptr, size, dtype)
};

}
#endif
