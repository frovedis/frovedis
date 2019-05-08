/*
    Copyright (c) 2013, Taiga Nomi and the respective contributors
    All rights reserved.

    Use of this source code is governed by a BSD-style license that can be found
    in the LICENSE file.
*/
#pragma once

#include "tiny_dnn/core/params/fully_params.h"

extern "C" {
  void dgemv_(char *TRANS,
              int *M, int *N,
              double *ALPHA,
              double *A, int *LDA,
              double *X, int *INCX,
              double *BETA,
              double *Y, int *INCY);
  void sgemv_(char *TRANS,
              int *M, int *N,
              float *ALPHA,
              float *A, int *LDA,
              float *X, int *INCX,
              float *BETA,
              float *Y, int *INCY);

  void dgemm_(char *TRANSA, char *TRANSB,
              int *M, int *N, int *K,
              double *ALPHA,
              double *A, int *LDA,
              double *B, int *LDB,
              double *BETA,
              double *C, int *LDC);
  void sgemm_(char *TRANSA, char *TRANSB,
              int *M, int *N, int *K,
              float *ALPHA,
              float *A, int *LDA,
              float *B, int *LDB,
              float *BETA,
              float *C, int *LDC);
}


namespace tiny_dnn {
namespace kernels {

inline void fully_connected_op_ve(const tensor_t &in_data,
								  const vec_t &W,
								  const vec_t &bias,
								  tensor_t &out_data,
								  const core::fully_params &params,
								  const bool layer_parallelize) {
  float_t alpha = 1;
  float_t beta = 1;
  int out_size = params.out_size_;
  int in_size = params.in_size_;
  int size =in_data.size();
  float_t* weight = const_cast<float_t*>(W.data());

  std::vector<float_t> in_put(in_size * size, 0);
  std::vector<float_t> out_put(out_size * size, 0);

  if (bias.empty())
    memset(&out_put[0], 0, sizeof(float_t) * out_size * size);
  else{
    for (int i = 0; i < size; ++i)
      memcpy(&out_put[i * out_size], bias.data(), sizeof(float_t) * out_size);
  }
  for (int i = 0; i < size; ++i)
    memcpy(&in_put[i * in_size], in_data[i].data(), sizeof(float_t) * in_size);

#ifdef CNN_USE_DOUBLE
  dgemm_(const_cast<char*>("N"), const_cast<char*>("N"), &out_size, &size, &in_size, 
         &alpha, weight, &out_size, &in_put[0], &in_size, &beta, &out_put[0], &out_size);
#else
  sgemm_(const_cast<char*>("N"), const_cast<char*>("N"), &out_size, &size, &in_size,
         &alpha, weight, &out_size, &in_put[0], &in_size, &beta, &out_put[0], &out_size);
#endif

  for (int i = 0; i < size; ++i)
    memcpy(out_data[i].data(), &out_put[i * out_size], sizeof(float_t) * out_size);
}

inline void fully_connected_op_ve(const tensor_t &prev_out,
								  const vec_t &W,
								  tensor_t &dW,
								  tensor_t &db,
								  tensor_t &curr_delta,
								  tensor_t &prev_delta,
								  const core::fully_params &params,
								  const bool layer_parallelize) {
  float_t* weight = const_cast<float_t*>(W.data());
  int out_size = params.out_size_;
  int in_size = params.in_size_;
  float_t alpha = 1;
  float_t beta = 1;

  int size = prev_out.size();
  std::vector<float_t> curr(out_size * size);
  std::vector<float_t> prev(in_size * size);
  std::vector<float_t> prevout(in_size * size, 0);

  for (int sample = 0; sample < size; ++sample)
    memcpy(&curr[sample * out_size], curr_delta[sample].data(), sizeof(float_t) * out_size);
  for (int sample = 0; sample < size; ++sample)
    memcpy(&prev[sample * in_size], prev_delta[sample].data(), sizeof(float_t) * in_size);
  for (int sample = 0; sample < size; ++sample)
    memcpy(&prevout[sample * in_size], prev_out[sample].data(), sizeof(float_t) * in_size);
  if (params.has_bias_) {
    for (int sample = 0; sample < size; ++sample)
      for (size_t i = 0; i < out_size; ++i)
        db[sample][i] += curr_delta[sample][i];
  }

#ifdef CNN_USE_DOUBLE
  dgemm_(const_cast<char*>("T"), const_cast<char*>("N"), &in_size, &size, &out_size,
         &alpha, weight, &out_size, &curr[0], &out_size, &beta, &prev[0], &in_size);
#else
  sgemm_(const_cast<char*>("T"), const_cast<char*>("N"), &in_size, &size, &out_size,
         &alpha, weight, &out_size, &curr[0], &out_size, &beta, &prev[0], &in_size);
#endif

  for (int sample = 0; sample < size; ++sample) {
    memcpy(prev_delta[sample].data(), &prev[sample * in_size], sizeof(float_t) * in_size);
  }

  /*
	Originally, size of dW is size of mini-batch, and sum'ed after this function
	However, it is more efficient to call gemm and sum them here, so make the size 1
   */
  dW.resize(1);
#ifdef CNN_USE_DOUBLE
  dgemm_(const_cast<char*>("N"), const_cast<char*>("T"), &out_size, &in_size, &size,
		 &alpha, &curr[0], &out_size, &prevout[0], &in_size,
		 &beta, dW[0].data(), &out_size);
#else
  sgemm_(const_cast<char*>("N"), const_cast<char*>("T"), &out_size, &in_size, &size,
		 &alpha, &curr[0], &out_size, &prevout[0], &in_size,
		 &beta, dW[0].data(), &out_size);
#endif
}

}  // namespace kernels
}  // namespace tiny_dnn
