#ifndef _MAT_OPERATIONS_
#define _MAT_OPERATIONS_

/*
 *  This header contains frequently used matrix operations in ML algorithms
 *  similar to following numpy operations on 2D array
 *    numpy.sum(x) -> matrix_sum(x) [supports rowmajor, colmajor and crs (both local and distributed)]
 *    numpy.mean(x) -> matrix_mean(x) [supports rowmajor, colmajor and crs (both local and distributed)]
 *    numpy.sum(numpy_square(x)) -> matrix_squared_sum(x) [supports rowmajor, colmajor and crs (both local and distributed)]
 *    sklearn.preprocessing.binarize(x, thr) -> matrix_binarize(x, thr) [supports rowmajor, colmajor and crs (both local and distributed)]
 *
 *  Additionally supports:
 *    get_start_indices(m): return starting global indices for each local matrices [supports rowmajor, colmajor and crs (only distributed version)]
 */

namespace frovedis {

// must be invoked with distributed MATRIX (rowmajor, colmajor or crs)
template <class MATRIX>
node_local<size_t> 
get_start_indices(MATRIX& mat) {
  auto nrows = mat.get_local_num_rows();
  std::vector<size_t> sidx(nrows.size()); sidx[0] = 0;
  for(size_t i = 1; i < nrows.size(); ++i) 
    sidx[i] = sidx[i - 1] + nrows[i - 1];
  return make_node_local_scatter(sidx);
}

// similar to numpy.sum(m)
template <class MATRIX>
std::vector<typename MATRIX::value_type>
matrix_sum(MATRIX& mat, int axis = -1) {
  if (axis == 0) return sum_of_rows(mat);
  else if (axis == 1) return sum_of_cols(mat);
  else return std::vector<typename MATRIX::value_type>(1, sum_of_elements(mat));
}

// similar to numpy.sum(numpy_square(m))
template <class MATRIX>
std::vector<typename MATRIX::value_type>
matrix_squared_sum(MATRIX& mat, int axis = -1) {
  if (axis == 0) return squared_sum_of_rows(mat);
  else if (axis == 1) return squared_sum_of_cols(mat);
  else return std::vector<typename MATRIX::value_type>(1, squared_sum_of_elements(mat));
}

// similar to numpy.mean(m)
template <class MATRIX>
std::vector<typename MATRIX::value_type>
matrix_mean(MATRIX& mat, int axis = -1) {
  return compute_mean(mat, axis);
}

// similar to sklearn.preprocessing.binarize()
template <class MATRIX>
MATRIX matrix_binarize(MATRIX& mat, 
                       typename MATRIX::value_type threshold = 0) {
  return binarize(mat, threshold);
}

}
#endif
