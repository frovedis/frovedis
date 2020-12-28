#ifndef _MAT_OPERATIONS_
#define _MAT_OPERATIONS_

/*
 *  This header contains frequently used matrix operations in ML algorithms
 *  similar to following numpy operations on 2D array
 *    numpy.sum(x, axis) -> matrix_sum(x, axis) [supports rowmajor, colmajor and crs (both local and distributed)]
 *    numpy.mean(x, axis) -> matrix_mean(x, axis) [supports rowmajor, colmajor and crs (both local and distributed)]
 *    numpy.sum(numpy_square(x, axis)) -> matrix_squared_sum(x, axis) [supports rowmajor, colmajor and crs (both local and distributed)]
 *    numpy.argmin(x, axis) -> matrix_argmin(x, axis) [supports rowmajor, colmajor (both local and distributed)]
 *    numpy.argmax(x, axis) -> matrix_argmax(x, axis) [supports rowmajor, colmajor (both local and distributed)]
 *    numpy.amin(x, axis) -> matrix_amin(x, axis) [supports rowmajor, colmajor (both local and distributed)]
 *    numpy.amax(x, axis) -> matrix_amax(x, axis) [supports rowmajor, colmajor (both local and distributed)]
 *    sklearn.preprocessing.binarize(x, thr) -> matrix_binarize(x, thr) [supports rowmajor, colmajor and crs (both local and distributed)]
 *
 */

namespace frovedis {

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

// similar to numpy.argmin(arr, axis)
template <class MATRIX>
std::vector<size_t>
matrix_argmin(MATRIX& mat, int axis = -1) {
  return get_values(argmin_pair(mat, axis));
}

// similar to numpy.amin(arr, axis)
template <class MATRIX>
std::vector<typename MATRIX::value_type>
matrix_amin(MATRIX& mat, int axis = -1) {
  return get_keys(argmin_pair(mat, axis));
}

// similar to numpy.argmax(arr, axis)
template <class MATRIX>
std::vector<size_t>
matrix_argmax(MATRIX& mat, int axis = -1) {
  return get_values(argmax_pair(mat, axis));
}

// similar to numpy.amax(arr, axis)
template <class MATRIX>
std::vector<typename MATRIX::value_type>
matrix_amax(MATRIX& mat, int axis = -1) {
  return get_keys(argmax_pair(mat, axis));
}

}
#endif
