#ifndef _DUMMY_MATRIX_HPP_
#define _DUMMY_MATRIX_HPP_

#include "model_tracker.hpp"

using namespace frovedis;

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

struct dummy_dftable {
  dummy_dftable() {}
  dummy_dftable(exrpc_ptr_t dfptr_,
                size_t nrow_,
                std::vector<std::string>& names_,
                std::vector<int>& types_): 
                dfptr(dfptr_), nrow(nrow_), names(names_), types(types_) {}
  exrpc_ptr_t dfptr;
  size_t nrow;
  std::vector<std::string> names;
  std::vector<int> types;
  SERIALIZE(dfptr, nrow, names, types)
};

template <class DF>
dummy_dftable to_dummy_dftable(DF* df) {
  auto data_types = df->dtypes();
  auto nrow = df->num_row();
  auto sz = data_types.size();
  std::vector<std::string> df_names(sz); 
  std::vector<int> df_types(sz);
  for(size_t i = 0; i < sz; i++) {
    df_names[i] = data_types[i].first;
    df_types[i] = get_numeric_dtype(data_types[i].second);
  }
  return dummy_dftable(reinterpret_cast<exrpc_ptr_t>(df),
                       nrow, df_names, df_types);
}

struct csv_config {
  csv_config() {}
  csv_config(int sep, 
             const std::vector<std::string>& nullstr,
             const std::string& comment,
             size_t rows_to_see, 
             double separate_mb,
             bool to_separate,
             bool add_index, 
             bool verbose,
             bool mangle_dupe_cols,
             int index_col) {
    this->separator = sep;
    this->nullstr = nullstr;
    this->comment = comment;
    this->rows_to_see = rows_to_see;
    this->separate_mb = separate_mb;
    this->to_separate = to_separate;
    this->add_index = add_index;
    this->verbose_level = verbose ? 1 : 0;
    this->mangle_dupe_cols = mangle_dupe_cols;
    this->index_col = index_col;
  }
  void debug_print() const {
    std::cout << "sep: " << separator << "; "
              << "nullstr: ";
    for(auto& i: nullstr) std::cout << i << " ";
    std::cout << "; "
              << "comment: " << comment << "; "
              << "rows_to_see: " << rows_to_see << "; "
              << "separate_mb: " << separate_mb << "; "
              << "to_separate: " << to_separate << "; "
              << "add_index: " << add_index << "; "
              << "verbose_level: " << verbose_level << "; "
              << "mangle_dupe_cols: " << mangle_dupe_cols << "; "
              << "index_col: " << index_col << std::endl;
  } 
  int separator, index_col;
  std::vector<std::string> nullstr;
  std::string comment;
  size_t rows_to_see;
  double separate_mb;
  bool to_separate, add_index;
  int verbose_level;
  bool mangle_dupe_cols;
  SERIALIZE(separator, nullstr, comment, rows_to_see, separate_mb, 
            to_separate, add_index, verbose_level, mangle_dupe_cols,
            index_col)
};

#endif
