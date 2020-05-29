#ifndef _KERNEL_SVM_WS_SELECTION_HPP_
#define _KERNEL_SVM_WS_SELECTION_HPP_

#include <vector>
#include <cstddef>

#include "../../core/radix_sort.hpp"
#include "kernel_svm_utility.hpp"


namespace frovedis {
namespace kernel {

struct working_set_selection {

public:
  working_set_selection(size_t data_size):
  ws_indicator(data_size), f_idx2sort(data_size), f_val2sort(data_size) {};
	
  void operator()(
    const std::vector<int>& y, const std::vector<double>& alpha, const std::vector<double>& f_val,
    double Cp, double Cn, size_t select_size, size_t select_offset, std::vector<int>& working_set
  ) {
    alg_original(y, alpha, f_val, Cp, Cn, select_size, select_offset, working_set);
  } 
		
private:
  void alg_original(
    const std::vector<int>& y, const std::vector<double>& alpha, const std::vector<double>& f_val,
    double Cp, double Cn, size_t select_size, size_t select_offset, std::vector<int>& working_set
  ) {
 
    size_t data_size = ws_indicator.size();
    int p_left = 0;
    int p_right = data_size - 1;
    int n_selected = 0;
    int *f_idx2sort_ptr = f_idx2sort.data();
    double* f_val2sort_ptr = f_val2sort.data();
    const double* f_val_ptr = f_val.data();

    // clear ws_indicator
    int *working_set_ptr = working_set.data();
    int *ws_indicator_ptr = ws_indicator.data();
    size_t ws_size = working_set.size();
    for (size_t i = 0; i < ws_size; i++) {
      if (i >= select_offset && i < select_offset + select_size) {
        int ws_id = working_set_ptr[i];
        ws_indicator_ptr[ws_id] = 0;
      }
    }
      
    // sort
    for (size_t i = 0; i < data_size; i++) {
      f_idx2sort_ptr[i] = i;
      f_val2sort_ptr[i] = f_val_ptr[i];
    }
    radix_sort<double, int>(f_val2sort, f_idx2sort);
    f_val2sort_ptr = f_val2sort.data();
    f_idx2sort_ptr = f_idx2sort.data();
    
    const int *y_ptr = y.data();
    const double *alpha_ptr = alpha.data();
    while (n_selected < select_size) {
      int i;
      if (p_left < data_size) {
        i = f_idx2sort_ptr[p_left];
        while (ws_indicator_ptr[i] == 1 || !is_I_up(alpha_ptr[i], y_ptr[i], Cp, Cn)) {
          //construct working set of I_up
          p_left++;
          if (p_left == data_size) break;
          i = f_idx2sort_ptr[p_left];
        }
        if (p_left < data_size) {
          working_set_ptr[select_offset + n_selected] = i;
          n_selected++;
          ws_indicator_ptr[i] = 1;
        }
      }
      if (p_right >= 0) {
        i = f_idx2sort_ptr[p_right];
        while (ws_indicator_ptr[i] == 1 || !is_I_low(alpha_ptr[i], y_ptr[i], Cp, Cn)) {
          //construct working set of I_low
          p_right--;
          if (p_right == -1) break;
          i = f_idx2sort_ptr[p_right];
        }
        if (p_right >= 0) {
          working_set_ptr[select_offset + n_selected] = i;
          n_selected++;
          ws_indicator_ptr[i] = 1;
        }
      }
    }
  } 

  
#if 0
  void alg_parallel_v1(
    const std::vector<int>& y, const std::vector<double>& alpha, const std::vector<double>& f_val,
    double Cp, double Cn, size_t select_size, size_t select_offset, std::vector<int>& working_set
  ) {
  
    size_t data_size = f_val.size();
    int* working_set_ptr = working_set.data();
    int* ws_indicator_ptr = ws_indicator.data();
    int* f_idx2sort_ptr = f_idx2sort.data();
    double* f_val2sort_ptr = f_val2sort.data();
    const int* y_ptr = y.data();
    const double* alpha_ptr = alpha.data();
    const double* f_val_ptr = f_val.data();  
    
    for (size_t i = 0; i < select_size; i++) {
      int ws_id = working_set_ptr[i + select_offset];
      ws_indicator_ptr[ws_id] = 0;
    }
    
    for (size_t i = 0; i < data_size; i++) {
      f_idx2sort_ptr[i] = i;
      f_val2sort_ptr[i] = f_val_ptr[i];
    }
    
    radix_sort<double, int>(f_val2sort, f_idx2sort); // ascending
    
    int p_min_up;
    int p_max_low;
    double min_up;
    double max_low;
    int select_min_up = 0;
    int select_max_low = 0;

    for (int p = 0; p < data_size; p++) {
      int p_left = p;
      int p_right = data_size - p - 1;

      if (!select_min_up) {
        int i_left = f_idx2sort_ptr[p_left];
        if (is_I_up(alpha_ptr[i_left], y_ptr[i_left], Cp, Cn)){
          p_min_up = p_left;
          min_up = f_val2sort_ptr[p_left];
          select_min_up = 1;
        }
      }
      if (!select_max_low){
        int i_right = f_idx2sort_ptr[p_right];
        if (is_I_low(alpha_ptr[i_right], y_ptr[i_right], Cp, Cn)){
          p_max_low = p_right;
          max_low = f_val2sort_ptr[p_right];
          select_max_low = 1;
        }
      }
      if (select_min_up && select_max_low) break;
    }

    double bias = 0;
    int n_interval = 0;
    double sum = 0;

    for (int p = p_min_up; p < p_max_low + 1; p++) {
      double f = f_val2sort_ptr[p];
      sum += f;
      n_interval++;
    }
    bias = sum / n_interval;
      
    for (int p = 0; p < data_size; p++) {
      int i = f_idx2sort_ptr[p];
      
      if (is_I_up_bound(alpha_ptr[i], y_ptr[i], Cp, Cn)){  // I_up\I_0
        // skip
      } else if (is_I_low_bound(alpha_ptr[i], y_ptr[i], Cp, Cn)){  // I_low\I_0
        f_val2sort_ptr[p] = 2 * bias - f_val2sort_ptr[p];
      } else {  // I_0
        if (f_val2sort_ptr[p] > bias) {
          f_val2sort_ptr[p] = 2 * bias - f_val2sort_ptr[p];
        }
      }
    }

    radix_sort(f_val2sort, f_idx2sort);
    
    int n_selected = 0;
    for (int p = 0; p < data_size; p++) {
      int i = f_idx2sort_ptr[p];
      if (ws_indicator_ptr[i] == 0){
        working_set_ptr[n_selected + select_offset] = i;
        n_selected++;
        ws_indicator_ptr[i] = 1;
      }
      if (n_selected >= select_size) break;
    }
  }
  #endif

  
  std::vector<int> ws_indicator;
  std::vector<int> f_idx2sort;
  std::vector<double> f_val2sort;
};
	
}  // namespace kernel
}  // namespace frovedis

#endif  // _KERNEL_SVM_WS_SELECTION_HPP_