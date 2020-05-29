#ifndef _KERNEL_SVM_FVALUE_HPP_
#define _KERNEL_SVM_FVALUE_HPP_

#include <vector>

#include "../../matrix/rowmajor_matrix.hpp"
#include "kernel_config.hpp"
#include "kernel_matrix.hpp"
#include "kernel_svm_utility.hpp"


namespace frovedis {
namespace kernel {

template <typename K>
void update_f(
  const std::vector<double>& alpha_diff, const rowmajor_matrix_local<K>& kernel_rows,
  std::vector<double>& f_val, size_t row_size = 0
){
  assert(alpha_diff.size() == kernel_rows.local_num_row);
  assert(f_val.size() == kernel_rows.local_num_col);
  assert(row_size <= kernel_rows.local_num_row);
	
  size_t data_size = kernel_rows.local_num_col;
  if (row_size == 0) {
    row_size = kernel_rows.local_num_row;
  }
  const double* alpha_diff_ptr = alpha_diff.data();
  const K* kernel_rows_ptr = kernel_rows.val.data();
  double* f_val_ptr = f_val.data();
	
#pragma omp parallel for 
  for (int i = 0; i < data_size; i++) {
	double sum_diff = 0;
  	for (int j = 0; j < row_size; j++) { 
      sum_diff += alpha_diff_ptr[j] * kernel_rows_ptr[data_size * j + i];
    }
  	f_val_ptr[i] -= sum_diff;
  }
}


template <typename K>
void init_f(
  const std::vector<double>& alpha, const std::vector<int>& y, 
  const rowmajor_matrix_local<K>& data,
  rowmajor_matrix_local<K>& kernel_rows, kernel_matrix<K>& kmatrix,
  std::vector<double>& f_val
){
  size_t ws_size = kernel_rows.local_num_row;
  size_t data_size = kernel_rows.local_num_col;
	
  const double* alpha_ptr = alpha.data();
  const int* y_ptr = y.data();
  std::vector<double> alpha_diff(ws_size);
	
  size_t loop = ceil_div(data_size, ws_size);
  size_t count = 0;
  for (size_t n = 0; n < loop; n++) {
    int step = std::min(ws_size, data_size - n * ws_size);
    if (step <= 0) continue;
		
    std::vector<int> indices(step);
    for (size_t i = 0; i < step; i++) {
      indices[i] = count + i;
      alpha_diff[i] = - alpha_ptr[count + i] * y_ptr[count + i];
    }		

    kmatrix.get_rows(indices, 0, kernel_rows);

    update_f(alpha_diff, kernel_rows, f_val, step);
    count += step;
	}
}

}  // namespace kernel
}  // namespace frovedis

#endif  // _KERNEL_SVM_FVALUE_HPP_
