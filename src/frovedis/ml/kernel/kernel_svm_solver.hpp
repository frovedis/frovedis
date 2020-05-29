#ifndef _KERNEL_SVM_SOLVER_HPP_
#define _KERNEL_SVM_SOLVER_HPP_

//#include <climits>
#include <limits>
#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>

#include "kernel_config.hpp"
#include "kernel_svm_ws_selection.hpp"
#include "kernel_matrix.hpp"
#include "kernel_svm_fvalue.hpp"
#include "kernel_svm_utility.hpp"


namespace frovedis {
namespace kernel {

template <typename K>
struct csmo_solver {
public:

  template <typename Matrix>
  void solve(
    Matrix& data, const std::vector<int> &y, std::vector<double> &alpha, double &rho,
    std::vector<double> &f_val, double eps, double Cp, double Cn, int ws_size, int max_iter,
    kernel_function_type kernel_ty, double gamma, double coef0, int degree
  ) const;

private:  
  double calculate_rho(
    const std::vector<double>& f_val, const std::vector<int>& y, const std::vector<double>& alpha,
    double Cp, double Cn
  ) const;
  
  void local_smo(
    const std::vector<int>& y, const std::vector<int>& working_set, double Cp, double Cn, const rowmajor_matrix_local<K>& kernel_rows,
    const kernel_matrix<K>& kmatrix, const std::vector<double>& f_val, double eps, size_t max_iter,
    std::vector<double>& alpha, std::vector<double>& alpha_diff, double& gap, size_t& progress 
  ) const;
};


template <typename K>
template <typename Matrix>
void csmo_solver<K>::solve(
  Matrix& data, const std::vector<int> &y, std::vector<double> &alpha, double &rho,
  std::vector<double> &f_val, double eps, double Cp, double Cn, int ws_size, int max_iter,
  kernel_function_type kernel_ty, double gamma, double coef0, int degree
) const {
  
  assert(data.local_num_row == y.size());  // NOTE: Is it correct ?
  assert(ws_size > 0 && ws_size % 2 == 0);

  size_t data_size = data.local_num_row;
  size_t ws_size_half = ws_size / 2;

  std::vector<int> working_set(ws_size);
  working_set_selection ws_select(data_size);
  bool is_first = true;
  
  rowmajor_matrix_local<K> kernel_rows(ws_size, data_size);

  LOG(DEBUG) << "declare kernel matrix" << std::endl;
  kernel_matrix<K> kmatrix(data, kernel_ty, gamma, coef0, degree);

  std::vector<double> alpha_diff(ws_size); 
  double gap = 0;
  const int int_max = std::numeric_limits<int>::max();
  size_t local_max_iter = std::max(100000, ws_size > int_max / 100 ? int_max : 100 * ws_size);
  size_t local_iter = 0;
  size_t progress = 0;

  size_t same_gap_cnt = 0;
  double previous_gap = std::numeric_limits<double>::infinity();

  // init_f
  LOG(DEBUG) << "initialize f-value" << std::endl;
  init_f(alpha, y, data, kernel_rows, kmatrix, f_val);

  LOG(DEBUG) << "begin solve iteration" << std::endl;  
  for (int iter = 0;; ++iter) {
    
    // select working set 
    size_t select_size, select_offset;
    if (iter == 0) {
      select_size = ws_size;
      select_offset = 0;
    } else if (is_first) {
      select_size = ws_size_half;
      select_offset = 0;
      is_first = false;
    } else {
      select_size = ws_size_half;
      select_offset = ws_size_half;
      is_first = true;
    }
    ws_select(y, alpha, f_val, Cp, Cn, select_size, select_offset, working_set);    

    // get rows
    std::vector<int> indices(select_size);
    for (size_t i = 0; i < select_size; i++)  indices[i] = working_set[i + select_offset]; 
    kmatrix.get_rows(indices, select_offset, kernel_rows);   

    // local_smo
    local_smo(y, working_set, Cp, Cn, kernel_rows, kmatrix, f_val, eps, local_max_iter, alpha, alpha_diff, gap, progress);
    
    double alpha_diff_sum = 0.;
    for (int i = 0; i < ws_size; i++) {
      alpha_diff_sum += alpha_diff[i];
    }
    
    // update_f
    update_f<K>(alpha_diff, kernel_rows, f_val);

    local_iter += progress;
    if (std::fabs(gap - previous_gap) < eps * 0.001) {
      same_gap_cnt++;
    } else {
      same_gap_cnt = 0;
      previous_gap = gap;
    }
    
    size_t print_iter = 1;
    if (iter % print_iter == 0) {
      LOG(DEBUG) << "global iter = " << iter << ", total local iter = " << local_iter << ", diff = " << gap << std::endl;
    }

    bool terminate = (
      (same_gap_cnt >= 10 && std::fabs(gap - 2.0) > eps) ||
      gap < eps ||
      (max_iter != -1 && iter == max_iter)
    );
    if (terminate) {
      rho = calculate_rho(f_val, y, alpha, Cp, Cn);
      LOG(DEBUG) << "global iter = " << iter << ", total local iter = " << local_iter << ", diff = " << gap << std::endl;
      break;
    }      
  }
}


template <typename K>
double csmo_solver<K>::calculate_rho(
  const std::vector<double>& f_val, const std::vector<int>& y, const std::vector<double>& alpha,
  double Cp, double Cn
) const {
  
  int n_free = 0;
  double sum_free = 0;
  double up_value = 100000;
  double low_value = - up_value;
  
  const double* f_val_ptr = f_val.data();
  const int* y_ptr = y.data();
  const double* alpha_ptr = alpha.data();
  
  for (size_t i = 0; i < alpha.size(); i++) {
    if (is_free(alpha_ptr[i], y_ptr[i], Cp, Cn)) {
      n_free++;
      sum_free += f_val_ptr[i];
    }
    if (is_I_up(alpha_ptr[i], y_ptr[i], Cp, Cn)) {
      up_value = std::min(up_value, f_val_ptr[i]);
    }
    if (is_I_low(alpha_ptr[i], y_ptr[i], Cp, Cn)) {
      low_value = std::max(low_value, f_val_ptr[i]);
    }
  }

  if (n_free != 0) {
    return sum_free / n_free;
  } else {
    return - (up_value + low_value) / 2;
  }  
}


template <typename K>
void csmo_solver<K>::local_smo(
  const std::vector<int>& y_val, const std::vector<int>& working_set, double Cp, double Cn, const rowmajor_matrix_local<K>& kernel_rows,
  const kernel_matrix<K>& kmatrix, const std::vector<double>& f_val, double eps, size_t max_iter,
  std::vector<double>& alpha, std::vector<double>& alpha_diff, double& gap, size_t& progress 
) const {

  size_t ws_size = working_set.size();
  size_t data_size = kernel_rows.local_num_col;

  auto diag = kmatrix.get_diag();
  
  const int* y_ptr = y_val.data();
  const int* working_set_ptr = working_set.data();
  const K* kernel_rows_ptr = kernel_rows.val.data(); 
  const double* f_val_ptr = f_val.data();
  const K* diag_ptr = diag.data();
  double* alpha_ptr = alpha.data();
  double* alpha_diff_ptr = alpha_diff.data();

  std::vector<double> ws_y(ws_size);
//  std::vector<int> ws_y(ws_size);
  std::vector<double> ws_f(ws_size);
  std::vector<double> ws_a(ws_size);
  std::vector<double> ws_a_old(ws_size);
  std::vector<K> ws_diag(ws_size);
  std::vector<double> ws_kernel(ws_size * ws_size);
      
  double* ws_y_ptr = ws_y.data();
//  int* ws_y_ptr = ws_y.data();
  double* ws_f_ptr = ws_f.data();
  double* ws_a_ptr = ws_a.data();
  double* ws_a_old_ptr = ws_a_old.data();
  K* ws_diag_ptr = ws_diag.data();
  double* ws_kernel_ptr = ws_kernel.data();

  for (size_t i = 0; i < ws_size; i++) {
    int ws_id = working_set_ptr[i];
    ws_y_ptr[i] = y_ptr[ws_id];
    ws_f_ptr[i] = f_val_ptr[ws_id];
    ws_a_ptr[i] = alpha_ptr[ws_id];
    ws_a_old_ptr[i] = alpha_ptr[ws_id];
    ws_diag_ptr[i] = diag_ptr[ws_id];
  }
  for (size_t i = 0; i < ws_size; i++) {
    for (size_t j = 0; j < ws_size; j++) { 
      ws_kernel_ptr[i * ws_size + j] = kernel_rows_ptr[i * data_size + working_set_ptr[j]];
    }
  }
  
  double local_eps;
  size_t iter = 0;
  const double large_number = 100000;
  
  while (1) {
    
    int i_min_up = 0;
    // int j1_max_low = 0;
    double min_up = large_number;
    double max_low = - large_number;
//    double min_up = std::numeric_limits<double>::infinity();
//    double max_low = - std::numeric_limits<double>::infinity();;
    for (size_t i = 0; i < ws_size; i++) { 
      double y = ws_y_ptr[i];
      double a = ws_a_ptr[i];
      double f = ws_f_ptr[i];
      
      if (is_I_up(a, y, Cp, Cn)) {
        if (f < min_up) {
          min_up = f;
          i_min_up = i;
        }
      }
      if (is_I_low(a, y, Cp, Cn)) {
        if (f > max_low) {
          max_low = f;
          // j1_max_low = i;
        }
      }      
    }
      
    double local_gap = max_low - min_up;
    if (iter == 0) {
      local_eps = std::max(eps, 0.1f * local_gap);
      gap = local_gap;
    }
    
    int terminate = (
      iter > max_iter ||
      local_gap < local_eps 
    );
    if (terminate) {
      for (size_t i = 0; i < ws_size; i++) {
        int ws_id = working_set_ptr[i];
        alpha_ptr[ws_id] = ws_a_ptr[i];
        alpha_diff_ptr[i] = - (ws_a_ptr[i] - ws_a_old_ptr[i]) * ws_y_ptr[i];
        progress = iter;
      }
      break;
    }
    
    // select with second order heuristics
    int j2_ex_low = 0;
    double ex_low = large_number;
    std::vector<double> grad_vec(ws_size);
    for (size_t i = 0; i < ws_size; i++) { 
      double y = ws_y_ptr[i];
      double a = ws_a_ptr[i];
      double f = ws_f_ptr[i];

      double sqrt_numer = - min_up + f;
      if (sqrt_numer > 0 && is_I_low(a, y, Cp, Cn)) {
        // gather
        double denom = ws_diag_ptr[i_min_up] + ws_diag_ptr[i] - 2 * ws_kernel_ptr[i_min_up * ws_size + i]; 
        double grad = - sqrt_numer * sqrt_numer / denom;
        grad_vec[i] = grad;
        if (grad < ex_low) {
          j2_ex_low = i;
          ex_low = grad;
        }
      }
    }
    
    // update alpha
    double y_i_min_up = ws_y_ptr[i_min_up];
    double y_j2_ex_low = ws_y_ptr[j2_ex_low];
    
    double alpha_i_diff = y_i_min_up > 0 ? Cp - ws_a_ptr[i_min_up] : ws_a_ptr[i_min_up];
    
    double tmp_a = y_j2_ex_low > 0 ? ws_a_ptr[j2_ex_low] : Cn - ws_a_ptr[j2_ex_low];
    double tmp_b = (- min_up + ws_f_ptr[j2_ex_low]) / (ws_diag_ptr[i_min_up] + ws_diag_ptr[j2_ex_low] - 2 * ws_kernel_ptr[i_min_up * ws_size + j2_ex_low]);
    double alpha_j_diff = std::min(tmp_a, tmp_b);
    
    double l = std::min(alpha_i_diff, alpha_j_diff);
    
    ws_a_ptr[i_min_up] += l * y_i_min_up;
    ws_a_ptr[j2_ex_low] -= l * y_j2_ex_low;
    
    // update f
    for (size_t i = 0; i < ws_size; i++) {  
      double ka = ws_kernel_ptr[j2_ex_low * ws_size + i];
      double kb = ws_kernel_ptr[i_min_up * ws_size + i];
      ws_f_ptr[i] -= l * (ka - kb);
    }
    
    iter++;
  } 
}

}  // namespace kernel
}  // namespace frovedis
 

#endif  // _KERNEL_SVM_SOLVER_HPP_

