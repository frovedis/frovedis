#ifndef _VESVM_KERNELMATRIX_HPP_
#define _VESVM_KERNELMATRIX_HPP_

#include <vector>
#include <cassert>

#include <omp.h>

#include "../../matrix/rowmajor_matrix.hpp"
#include "../../matrix/blas_wrapper.hpp"
#include "kernel_config.hpp"


namespace frovedis {
namespace kernel {

namespace {

template <typename T>
T typed_exp(T);
template <>
inline float typed_exp(float x) { return expf(x); }
template <>
inline double typed_exp(double x) { return exp(x); }

template <typename T>
T typed_tanh(T);
template <>
inline float typed_tanh(float x) { return tanhf(x); }
template <>
inline double typed_tanh(double x) { return tanh(x); }

template <typename T>
T typed_pow(T, T);
template <>
inline float typed_pow(float x, float y) { return powf(x, y); }
template <>
inline double typed_pow(double x, double y) { return pow(x, y); }

template <typename T>
void typed_gemm(char* TRANSA, char* TRANSB,
                int* M, int* N, int* K,
                T* ALPHA, T* A, int* LDA, T* B, int* LDB,
                T* BETA, T* C, int* LDC);
template <>
void typed_gemm(char* TRANSA, char* TRANSB,
                int* M, int* N, int* K,
                float* ALPHA, float* A, int* LDA, float* B, int* LDB,
                float* BETA, float* C, int* LDC)
{
  sgemm_(TRANSA, TRANSB, M, N, K, ALPHA, A, LDA, B, LDB, BETA, C, LDC);
}
template <>
void typed_gemm(char* TRANSA, char* TRANSB,
                int* M, int* N, int* K,
                double* ALPHA, double* A, int* LDA, double* B, int* LDB,
                double* BETA, double* C, int* LDC)
{
  dgemm_(TRANSA, TRANSB, M, N, K, ALPHA, A, LDA, B, LDB, BETA, C, LDC);
}

template <typename T>
void omp_parallel_gemm_T_N(int m, int n, int k, T* A, T* B, T* C) 
{
  // Partition B for thread
#pragma omp parallel
  {
    size_t th_num = omp_get_num_threads();
    size_t th_id = omp_get_thread_num();
    
    int len = n / th_num;
    int off = len * th_id;
    int rem = n % th_num;
    if (th_id < rem) {
      len += 1;
      off += th_id;
    } else {
      off += rem;
    }

    char* TRANSA = "T";
    char* TRANSB = "N";
    T alpha = 1;
    int lda = k;
    int ldb = k;
    T beta = 0;
    int ldc = m;

    T* XB = &B[off * k];
    T* XC = &C[off * m];

    if (len > 0) {
      typed_gemm<T>(TRANSA, TRANSB, &m, &len, &k, 
                    &alpha, A, &lda, XB, &ldb, &beta, XC, &ldc);
    }
  }  // omp parallel
}

}  // namespace


template <typename K>
struct kernel_matrix {
public:
  kernel_matrix(
    const rowmajor_matrix_local<K>& data,
    kernel_function_type kernel_ty, double gamma, double coef0, int degree
  ):
    data(data), kernel_ty(kernel_ty), gamma(gamma), coef0(coef0), degree(degree)
    {
      sample_size = data.local_num_row;
      feature_size = data.local_num_col;
      buffer = rowmajor_matrix_local<K>(1, feature_size);

      make_self_dot();
      make_diag();      
    }
  void get_rows(
    const std::vector<int>& indices, size_t row_offset, rowmajor_matrix_local<K>& kernel_rows
  );   
  void get_rows(
    const rowmajor_matrix_local<K>& target, size_t target_offset, size_t target_size,
    size_t row_offset, rowmajor_matrix_local<K>& kernel_rows
  );
  void make_self_dot();
  void make_diag();
  const std::vector<K>& get_diag() const { return diag;}
  void check_shape() {
    bool shape_is_changed = (sample_size != data.local_num_row) || (feature_size != data.local_num_col);
    if (shape_is_changed) {
      throw std::runtime_error("Shape of data matrix is changed from initial");
    } 
  }
  
private:
  const rowmajor_matrix_local<K>& data;
  kernel_function_type kernel_ty;
  double gamma;
  double coef0;
  int degree;
  size_t sample_size;
  size_t feature_size;

  rowmajor_matrix_local<K> buffer; 
  std::vector<K> self_dot;
  std::vector<K> diag;

  void rbf_kernel(
    const std::vector<int>& indices, size_t row_offset,
    rowmajor_matrix_local<K>& kernel_rows
  );
  void rbf_kernel(
    const rowmajor_matrix_local<K>& target, size_t target_offset, size_t target_size,
    size_t row_offset, rowmajor_matrix_local<K>& kernel_rows
  );
  void poly_kernel(
    const std::vector<int>& indices, size_t row_offset,
    rowmajor_matrix_local<K>& kernel_rows
  );
  void poly_kernel(
    const rowmajor_matrix_local<K>& target, size_t target_offset, size_t target_size,
    size_t row_offset, rowmajor_matrix_local<K>& kernel_rows
  );
  void sigmoid_kernel(
    const std::vector<int>& indices, size_t row_offset,
    rowmajor_matrix_local<K>& kernel_rows
  );
  void sigmoid_kernel(
    const rowmajor_matrix_local<K>& target, size_t target_offset, size_t target_size,
    size_t row_offset, rowmajor_matrix_local<K>& kernel_rows
  );
};


template <typename K>
void kernel_matrix<K>::get_rows(
  const std::vector<int>& indices, size_t row_offset, rowmajor_matrix_local<K>& kernel_rows
) {

  assert(row_offset + indices.size() <= kernel_rows.local_num_row);
  assert(sample_size == kernel_rows.local_num_col);
  check_shape();

  if (buffer.local_num_row < indices.size()) {
    buffer.val.resize(indices.size() * feature_size);
    buffer.local_num_row = indices.size();
    buffer.local_num_col = feature_size;
  }
  // fill buffer
  for (size_t i = 0; i < indices.size(); i++) {
    int t_i = indices[i];
    for (size_t f_i = 0; f_i < feature_size; f_i++) {   
      buffer.val[f_i + i * feature_size] = data.val[f_i + t_i * feature_size]; 
    }
  }

  int m = sample_size;
  int n = indices.size();
  int k = feature_size;
  K* A = const_cast<K*>(data.val.data());
  K* B = buffer.val.data();
  K* C = &kernel_rows.val[row_offset * sample_size];

  omp_parallel_gemm_T_N<K>(m, n, k, A, B, C);

  // apply kernel function
  switch (kernel_ty) {
    case kernel_function_type::RBF:
      rbf_kernel(indices, row_offset, kernel_rows);
      break;
      
    case kernel_function_type::POLY:
      poly_kernel(indices, row_offset, kernel_rows);
      break;
      
    case kernel_function_type::SIGMOID:
      sigmoid_kernel(indices, row_offset, kernel_rows);
      break;
      
    default:
      throw std::logic_error("The operation using kernel function is not implemented");
      break;
  }
}


template <typename K>
void kernel_matrix<K>::get_rows(
    const rowmajor_matrix_local<K>& target, size_t target_offset, size_t target_size,
    size_t row_offset, rowmajor_matrix_local<K>& kernel_rows
) {
  assert(target.local_num_col == feature_size);
  assert(target_offset + target_size <= target.local_num_row);
  assert(row_offset + target_size <= kernel_rows.local_num_row);
  check_shape();

  int m = sample_size;
  int n = target_size;
  int k = feature_size;
  K* A = const_cast<K*>(data.val.data());
  K* B = const_cast<K*>(&target.val[target_offset * feature_size]);
  K* C = &kernel_rows.val[row_offset * sample_size];

  omp_parallel_gemm_T_N<K>(m, n, k, A, B, C);

  // apply kernel function
  switch (kernel_ty) {
    case kernel_function_type::RBF:
      rbf_kernel(target, target_offset, target_size, row_offset, kernel_rows);
      break;
      
    case kernel_function_type::POLY:
      poly_kernel(target, target_offset, target_size, row_offset, kernel_rows);
      break;
      
    case kernel_function_type::SIGMOID:
      sigmoid_kernel(target, target_offset, target_size, row_offset, kernel_rows);
      break;
      
    default:
      throw std::logic_error("The operation using kernel function is not implemented");
      break;
  }
}


template <typename K>
void kernel_matrix<K>::make_self_dot() {
  const K* data_ptr = data.val.data();
  self_dot.resize(sample_size);
  K* self_dot_ptr = self_dot.data();
  
  for (size_t i = 0; i < sample_size; i++) {
    K sum = 0;
    for (size_t j = 0; j < feature_size; j++) {   
      sum += data_ptr[i * feature_size + j] * data_ptr[i * feature_size + j];
    }
    self_dot_ptr[i] = sum;
  }
}


template <typename K>
void kernel_matrix<K>::make_diag() {
  diag.resize(sample_size);
  K* diag_ptr = diag.data();
  
  switch (kernel_ty) {
    case kernel_function_type::RBF:
      for (size_t i = 0; i < sample_size; i++) diag_ptr[i] = 1;
      break;
    
    case kernel_function_type::POLY:
      for (size_t i = 0; i < sample_size; i++) {
        diag_ptr[i] = typed_pow<K>(gamma * self_dot[i] + coef0, degree);
      }
      break;

    case kernel_function_type::SIGMOID:
      for (size_t i = 0; i < sample_size; i++) {
        diag_ptr[i] = typed_tanh<K>(gamma * self_dot[i] + coef0);
      }
      break;

    default :
      throw std::logic_error("The operation using the kernel function is not implemented");
      break;
  }
}  


template <typename K>
void kernel_matrix<K>::rbf_kernel(
  const std::vector<int>& indices, size_t row_offset,
  rowmajor_matrix_local<K>& kernel_rows
) {
  assert(row_offset + indices.size() <= kernel_rows.local_num_row);
  check_shape();

#pragma omp parallel for schedule(static)
  for (size_t i = 0; i < indices.size(); i++) {
    size_t t_i = indices[i];
    K* row_ptr = &kernel_rows.val[(i + row_offset) * sample_size];
    for (size_t s_i = 0; s_i < sample_size; s_i++) {   
      row_ptr[s_i] = typed_exp<K>(
        - (self_dot[s_i] + self_dot[t_i] - 2 * row_ptr[s_i]) * gamma
      );  
    }
  }
}

template <typename K>
void kernel_matrix<K>::rbf_kernel(
  const rowmajor_matrix_local<K>& target, size_t target_offset, size_t target_size,
  size_t row_offset, rowmajor_matrix_local<K>& kernel_rows
) {
  assert(target_offset + target_size <= target.local_num_row);
  assert(row_offset + target_size <= kernel_rows.local_num_row);
  check_shape();

  std::vector<K> self_dot_target(target_size);
  for (size_t i = 0; i < target_size; i++) {
    K sum = 0;
    for (size_t j = 0; j < feature_size; j++) {  
      sum += target.val[j + (i + target_offset) * feature_size];
    }
    self_dot_target[i] = sum;
  }

#pragma omp parallel for schedule(static)
  for (size_t i = 0; i < target_size; i++) {
    K* row_ptr = &kernel_rows.val[(i + row_offset) * sample_size];
    for (size_t s_i = 0; s_i < sample_size; s_i++) {  
      row_ptr[s_i] = typed_exp<K>(
        - (self_dot[s_i] + self_dot_target[i] - 2 * row_ptr[s_i]) * gamma
      );
    }
  }
}

template <typename K>
void kernel_matrix<K>::poly_kernel(
  const std::vector<int>& indices, size_t row_offset,
  rowmajor_matrix_local<K>& kernel_rows
) {
  assert(row_offset + indices.size() <= kernel_rows.local_num_row);
  check_shape();

#pragma omp parallel for schedule(static)
  for (size_t i = 0; i < indices.size(); i++) {
    K* row_ptr = &kernel_rows.val[(i + row_offset) * sample_size];
    for (size_t s_i = 0; s_i < sample_size; s_i++) {   
      row_ptr[s_i] = typed_pow<K>(gamma * row_ptr[s_i] + coef0, degree);
    }
  }
}

template <typename K>
void kernel_matrix<K>::poly_kernel(
  const rowmajor_matrix_local<K>& target, size_t target_offset, size_t target_size,
  size_t row_offset, rowmajor_matrix_local<K>& kernel_rows
) {
  assert(target_offset + target_size <= target.local_num_row);
  assert(row_offset + target_size <= kernel_rows.local_num_row);
  check_shape();

#pragma omp parallel for schedule(static)
  for (size_t i = 0; i < target_size; i++) {
    K* row_ptr = &kernel_rows.val[(i + row_offset) * sample_size];
    for (size_t s_i = 0; s_i < sample_size; s_i++) {
      row_ptr[s_i] = typed_pow<K>(gamma * row_ptr[s_i] + coef0, degree);
    }
  }
}

template <typename K>
void kernel_matrix<K>::sigmoid_kernel(
  const std::vector<int>& indices, size_t row_offset,
  rowmajor_matrix_local<K>& kernel_rows
) {
  assert(row_offset + indices.size() <= kernel_rows.local_num_row);
  check_shape();

#pragma omp parallel for schedule(static)
  for (size_t i = 0; i < indices.size(); i++) {
    K* row_ptr = &kernel_rows.val[(i + row_offset) * sample_size];
    for (size_t s_i = 0; s_i < sample_size; s_i++) {  
      row_ptr[s_i] = typed_tanh<K>(gamma * row_ptr[s_i] + coef0);
    }
  }
}

template <typename K>
void kernel_matrix<K>::sigmoid_kernel(
  const rowmajor_matrix_local<K>& target, size_t target_offset, size_t target_size,
  size_t row_offset, rowmajor_matrix_local<K>& kernel_rows
) {
  assert(target_offset + target_size <= target.local_num_row);
  assert(row_offset + target_size <= kernel_rows.local_num_row);
  check_shape();

#pragma omp parallel for schedule(static)
  for (size_t i = 0; i < target_size; i++) {
    K* row_ptr = &kernel_rows.val[(i + row_offset) * sample_size];
    for (size_t s_i = 0; s_i < sample_size; s_i++) {
      row_ptr[s_i] = typed_tanh<K>(gamma * row_ptr[s_i] + coef0);
    }
  }
}


}  // namespace kernel
}  // namespace frovedis

#endif  // _VESVM_KERNELMATRIX_HPP_


