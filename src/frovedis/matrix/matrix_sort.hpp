#ifndef _MATRIX_SORT_HPP_
#define _MATRIX_SORT_HPP_

#include <frovedis/matrix/rowmajor_matrix.hpp>
#define MB 1024 * 1024 

namespace frovedis {

template <class T>
size_t get_nrows_per_chunk(size_t nrow, size_t ncol,
                           float chunk_size = 1.0) {
  require(chunk_size > 0, "chunk_size (in mb) should be a positive non-zero value!\n");
  size_t tot_elems_per_chunk = chunk_size / sizeof(T) * MB; 
  size_t rows_per_chunk = ceil_div(tot_elems_per_chunk, ncol);
  return 4; // std::min(nrow, rows_per_chunk);
}

template <class T>
void sort_row_segment_impl(T* valp, size_t nrow, size_t ncol,
                           bool positive_only = false) {
  auto valsz = nrow * ncol;
  if (!valsz) return;
  auto tmp = vector_arrange<size_t>(valsz); // 0, 1, 2, 3, 4, ..., N-1
  auto tmpp = tmp.data();

  // --- copy inputs ---
  std::vector<T> actual_val(valsz);
  auto c_valp = actual_val.data();
  for(size_t i = 0; i < valsz; ++i) c_valp[i] = valp[i];

  // first time sorting entire values
  radix_sort(valp, tmpp, valsz, positive_only);

  // constructing row-index and column-index based on 'position'
  std::vector<size_t> row_index(valsz);
  auto ridxp = row_index.data();
  for(size_t i = 0; i < valsz; ++i) {
    ridxp[i] = tmpp[i] / ncol; // row-index
    tmpp[i] = tmpp[i] % ncol;  // col-index (updated in-place to reduce memory)
  }

  // second time sorting to re-position sorted values row-wise
  radix_sort(ridxp, tmpp, valsz, true); // specified true, since indices are always positive

  // -------- copy-back --------
  for(size_t i = 0; i < nrow; ++i) {
    #pragma _NEC ivdep
    for(size_t j = 0; j < ncol; ++j) {
      auto sorted_col_idx = tmpp[i * ncol + j];
      valp[i * ncol + j] = c_valp[i * ncol + sorted_col_idx];
    }
  }
}

template <class T>
void sort_each_row(std::vector<T>& val,
                   size_t nrow, size_t ncol,
                   bool positive_only = false) {
  sort_row_segment_impl(val.data(), nrow, ncol, positive_only);
}

// -------- for key-value pair --------
template <class T, class I>
void sort_row_segment_impl(T* valp, I* posp,
                           size_t nrow, size_t ncol,
                           bool positive_only = false) {
  auto valsz = nrow * ncol;
  if (!valsz) return;
  auto tmp = vector_arrange<size_t>(valsz); // 0, 1, 2, 3, 4, ..., N-1
  auto tmpp = tmp.data();

  // --- copy inputs ---
  std::vector<T> actual_val(valsz);
  std::vector<I> actual_pos(valsz);
  auto c_valp = actual_val.data();
  auto c_posp = actual_pos.data();
  for(size_t i = 0; i < valsz; ++i) {
    c_valp[i] = valp[i];
    c_posp[i] = posp[i];
  }

  // first time sorting entire values
  radix_sort(valp, tmpp, valsz, positive_only);

  // constructing row-index and column-index based on 'position'
  std::vector<size_t> row_index(valsz);
  auto ridxp = row_index.data();
  for(size_t i = 0; i < valsz; ++i) {
    ridxp[i] = tmpp[i] / ncol; // row-index
    tmpp[i] = tmpp[i] % ncol;  // col-index (updated in-place to reduce memory)
  }

  // second time sorting to re-position sorted values row-wise
  radix_sort(ridxp, tmpp, valsz, true); // specified true, since indices are always positive

  // -------- copy-back --------
  for(size_t i = 0; i < nrow; ++i) {
    #pragma _NEC ivdep
    for(size_t j = 0; j < ncol; ++j) {
      auto sorted_col_idx = tmpp[i * ncol + j];
      valp[i * ncol + j] = c_valp[i * ncol + sorted_col_idx];
      posp[i * ncol + j] = c_posp[i * ncol + sorted_col_idx];
    }
  }
}

template <class T, class I>
void sort_each_row(std::vector<T>& val,
                   std::vector<I>& pos,
                   size_t nrow, size_t ncol,
                   bool positive_only = false) {
  checkAssumption(val.size() == pos.size());
  sort_row_segment_impl(val.data(), pos.data(), nrow, ncol, positive_only);
}

template <class T>
void matrix_sort_by_rows(rowmajor_matrix_local<T>& mat,
                         bool positive_only = false,
                         float chunk_size = 1.0) { // 1 MB default
  auto valp = mat.val.data();
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  auto rows_per_chunk = get_nrows_per_chunk<T>(nrow, ncol, chunk_size);
  size_t niter = nrow / rows_per_chunk;
  for(size_t i = 0; i < niter; ++i) {
    sort_row_segment_impl(valp + i * rows_per_chunk * ncol, rows_per_chunk, ncol, positive_only);
  }
  auto rem_nrow = nrow - (niter * rows_per_chunk);
  if (rem_nrow) {
    sort_row_segment_impl(valp + niter * rows_per_chunk * ncol, rem_nrow, ncol, positive_only);
  }
}

template <class T>
void matrix_sort_by_rows(rowmajor_matrix<T>& mat, 
                         bool positive_only = false,
                         float chunk_size = 1.0) { // 1 MB default
  mat.data.mapv(+[](rowmajor_matrix_local<T>& lmat, 
                    bool positive_only, float chunk_size) {
    matrix_sort_by_rows(lmat, positive_only, chunk_size);
  }, broadcast(positive_only), broadcast(chunk_size));
}

template <class T, class I>
void matrix_sort_by_rows(rowmajor_matrix_local<T>& val_mat,
                         rowmajor_matrix_local<I>& pos_mat,
                         bool positive_only = false,
                         float chunk_size = 1.0) { // 1 MB default
  auto valp = val_mat.val.data();
  auto posp = pos_mat.val.data();
  auto nrow = val_mat.local_num_row;
  auto ncol = val_mat.local_num_col;
  checkAssumption(val_mat.val.size() == pos_mat.val.size() &&
                  nrow == pos_mat.local_num_row &&
                  ncol == pos_mat.local_num_col);
  auto rows_per_chunk = get_nrows_per_chunk<T>(nrow, ncol, chunk_size);
  size_t niter = nrow / rows_per_chunk;
  for(size_t i = 0; i < niter; ++i) {
    sort_row_segment_impl(valp + i * rows_per_chunk * ncol, 
                          posp + i * rows_per_chunk * ncol,
                          rows_per_chunk, ncol, positive_only);
  }
  auto rem_nrow = nrow - (niter * rows_per_chunk);
  if (rem_nrow) {
    sort_row_segment_impl(valp + niter * rows_per_chunk * ncol, 
                          posp + niter * rows_per_chunk * ncol,
                          rem_nrow, ncol, positive_only);
  }
}

template <class T, class I>
void matrix_sort_by_rows(rowmajor_matrix<T>& val_mat,
                         rowmajor_matrix<I>& pos_mat,
                         bool positive_only = false,
                         float chunk_size = 1.0) { // 1 MB default
  val_mat.data.mapv(+[](rowmajor_matrix_local<T>& lval_mat, 
                        rowmajor_matrix_local<I>& lpos_mat,
                        bool positive_only, float chunk_size) {
     matrix_sort_by_rows(lval_mat, lpos_mat, positive_only, chunk_size);
  }, pos_mat.data, broadcast(positive_only), broadcast(chunk_size));
}

template <class T>
void matrix_sort_by_cols(rowmajor_matrix_local<T>& mat,
                         bool positive_only = false,
                         float chunk_size = 1.0) { // 1 MB default
  auto tmat = mat.transpose();  mat.clear(); 
  matrix_sort_by_rows(tmat, positive_only, chunk_size);
  mat = tmat.transpose();
}

template <class T>
void matrix_sort_by_cols(rowmajor_matrix<T>& mat, 
                         bool positive_only = false,
                         float chunk_size = 1.0) { // 1 MB default
  auto tmat = mat.transpose();  mat.clear();
  tmat.data.mapv(+[](rowmajor_matrix_local<T>& lmat, 
                     bool positive_only, float chunk_size) {
    matrix_sort_by_rows(lmat, positive_only, chunk_size);
  }, broadcast(positive_only), broadcast(chunk_size));
  mat = tmat.transpose();
}

template <class T, class I>
void matrix_sort_by_cols(rowmajor_matrix_local<T>& val_mat,
                         rowmajor_matrix_local<I>& pos_mat,
                         bool positive_only = false,
                         float chunk_size = 1.0) { // 1 MB default
  auto tval_mat = val_mat.transpose();  val_mat.clear(); 
  auto tpos_mat = pos_mat.transpose();  pos_mat.clear();
  matrix_sort_by_rows(tval_mat, tpos_mat, positive_only, chunk_size);
  val_mat = tval_mat.transpose();
  pos_mat = tpos_mat.transpose();
}

template <class T, class I>
void matrix_sort_by_cols(rowmajor_matrix<T>& val_mat,
                         rowmajor_matrix<I>& pos_mat,
                         bool positive_only = false,
                         float chunk_size = 1.0) { // 1 MB default
  auto tval_mat = val_mat.transpose();  val_mat.clear();
  auto tpos_mat = pos_mat.transpose();  pos_mat.clear();
  tval_mat.data.mapv(+[](rowmajor_matrix_local<T>& lval_mat, 
                         rowmajor_matrix_local<I>& lpos_mat,
                         bool positive_only, float chunk_size) {
     matrix_sort_by_rows(lval_mat, lpos_mat, positive_only, chunk_size);
  }, tpos_mat.data, broadcast(positive_only), broadcast(chunk_size));
  val_mat = tval_mat.transpose();
  pos_mat = tpos_mat.transpose();
}

template <class T>
std::vector<double> 
matrix_median_by_rows(rowmajor_matrix_local<T>& mat, 
                      bool sort_inplace = false) { // whether to sort matrix inplace
  T* matp = NULL;
  rowmajor_matrix_local<T> copy_mat;
  if (sort_inplace) {
    matrix_sort_by_rows(mat);
    matp = mat.val.data();
  } else {
    copy_mat = mat;
    matrix_sort_by_rows(copy_mat);
    matp = copy_mat.val.data();
  }
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  auto mid = ncol / 2;
  std::vector<double> ret(nrow);
  auto retp = ret.data();
  if (ncol % 2 == 0) {
    for(size_t i = 0; i < nrow; ++i) {
      auto x = matp[i * ncol + mid];
      auto y = matp[i * ncol + (mid - 1)];
      retp[i] = (x + y) * 0.5;
    }
  } else {
    for(size_t i = 0; i < nrow; ++i) {
      retp[i] = static_cast<double>(matp[i * ncol + mid]); 
    }
  }
  return ret;
}

template <class T>
std::vector<double> 
matrix_median_by_rows(rowmajor_matrix<T>& mat,
                      bool sort_inplace = false) { // whether to sort matrix inplace
  return mat.data.map(+[](rowmajor_matrix_local<T>& lmat,
                          bool sort_inplace) {
    return matrix_median_by_rows(lmat, sort_inplace);
  }, broadcast(sort_inplace)).template moveto_dvector<double>().gather();
}

template <class T>
std::vector<double>
matrix_median_by_cols(rowmajor_matrix_local<T>& mat,
                      bool sort_inplace = false) { // whether to sort matrix inplace
  auto tmat = mat.transpose();
  if (sort_inplace) mat.clear();
  auto ret = matrix_median_by_rows(tmat, sort_inplace);
  if (sort_inplace) mat = tmat.transpose();
  return ret;
}

template <class T>
std::vector<double>
matrix_median_by_cols(rowmajor_matrix<T>& mat,
                      bool sort_inplace = false) { // whether to sort matrix inplace
  auto tmat = mat.transpose();
  if (sort_inplace) mat.clear();
  auto ret = tmat.data.map(+[](rowmajor_matrix_local<T>& lmat,
                               bool sort_inplace) {
    return matrix_median_by_rows(lmat, sort_inplace);
  }, broadcast(sort_inplace)).template moveto_dvector<double>().gather();
  if (sort_inplace) mat = tmat.transpose();
  return ret;
}

}
#endif
