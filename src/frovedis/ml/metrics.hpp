#ifndef _METRICS_
#define _METRICS_

#include "macro.hpp"

namespace frovedis {

template <class T>
float calc_squared_error_diff(const std::vector<T>& pred_label,
                              const std::vector<T>& true_label) {
  float sq_error = 0.0;
  auto size = pred_label.size();
  auto plblptr = pred_label.data();
  auto tlblptr = true_label.data();
  for(size_t i = 0; i < size; ++i) {
    float error = tlblptr[i] - plblptr[i];
    sq_error += (error * error);
  }
  return sq_error;
}

template <class T>
float calc_squared_mean_error_diff(const std::vector<T>& true_label) {
  T sum = 0;
  auto size = true_label.size();
  auto tlblptr = true_label.data();
  for(size_t i = 0; i < size; ++i) sum += tlblptr[i];
  float true_label_mean = static_cast<float>(sum) / size;
  float sq_error = 0.0;
  for(size_t i = 0; i < size; ++i) {
    float error = tlblptr[i] - true_label_mean;
    sq_error += (error * error);
  }
  return sq_error;
}

template <class T>
float r2_score(const std::vector<T>& pred_label,
               const std::vector<T>& true_label) {
  auto size = pred_label.size();
  require(size == true_label.size(),
  "r2_score: size differs in predicted labels and actual labels!\n");
  //debug_print_vector(pred_label, 10);
  //debug_print_vector(true_label, 10);
  auto u = calc_squared_error_diff(pred_label, true_label);
  auto v = calc_squared_mean_error_diff(true_label);
  return 1 - u / v; 
}

template <class T>
float accuracy_score(const std::vector<T>& pred_label,
                     const std::vector<T>& true_label) {
  auto size = pred_label.size();
  require(size == true_label.size(), 
  "accuracy_score: size differs in predicted labels and actual labels!\n");
  size_t count = 0;
  auto predp = pred_label.data();
  auto actualp = true_label.data();
  //debug_print_vector(pred_label, 10);
  //debug_print_vector(true_label, 10);
  for(size_t i = 0; i < size; ++i) count += (predp[i] == actualp[i]);
  RLOG(INFO) << "[accuracy_score] " << count << "/" << size 
             << " predicted results have been matched!\n";
  return static_cast<float>(count) / size;
}

}

#endif
