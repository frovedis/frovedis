/*
    Copyright (c) 2013, Taiga Nomi and the respective contributors
    All rights reserved.

    Use of this source code is governed by a BSD-style license that can be found
    in the LICENSE file.
*/
#pragma once

#include <limits>
#include <vector>
#ifdef __ve__
#include "vednn/vednn.h"
// #define USE_VEDNN_MAXPOOL_BACKWARD
#endif

namespace tiny_dnn {
namespace kernels {

inline void maxpool_op_ve(const tensor_t &in_data,
                                tensor_t &out_data,
                                std::vector<std::vector<size_t>> &max_idx,
                                const std::vector<std::vector<size_t>> &out2in,
                                const bool layer_parallelize,
                                const core::maxpool_params &params) {
#ifdef __ve__ // fail safe  
  dataType_t dtype {DTYPE_FLOAT};
  int batch_in {1};
  int batch_out {1};
  int padWidth {0};
  int padHeight {0};

  size_t iw {params.in.width_};
  size_t ih {params.in.height_};
  size_t id {params.in.depth_};
  size_t ow {params.out.width_};
  size_t oh {params.out.height_};
  size_t od {params.out.depth_};
  size_t windowWidth {params.pool_size_x};
  size_t windowHeight {params.pool_size_y};
  size_t strideWidth {params.stride_x};
  size_t strideHeight {params.stride_y};
  
  vednnTensorParam_t ParamIn {dtype, batch_in, id, iw, ih};
  vednnTensorParam_t ParamOut {dtype, batch_out, od, ow, oh};
  vednnPoolingParam_t ParamPool {windowWidth, windowHeight, strideWidth, strideHeight,
                                 padWidth, padHeight};

  auto *pParamIn {&ParamIn};
  auto *pParamOut {&ParamOut};
  auto *pParamPool {&ParamPool};

  int num_in_data {in_data.size()};

  for(size_t sample = 0; sample < num_in_data; sample++) {

    auto *pDataIn = in_data[sample].data();
    auto *pDataOut = out_data[sample].data();
    
    vednnMaxPoolingForward(pParamIn, pDataIn, pParamOut, pDataOut, pParamPool);
  }
#else
  throw std::runtime_error("Calling Vector Engine code");
#endif
}

inline void maxpool_grad_op_ve(const tensor_t &prev_data,
                                     tensor_t &prev_delta,
                                     tensor_t &curr_data,
                                     const tensor_t &curr_delta,
                                     std::vector<std::vector<size_t>> &max_idx,
                                     const std::vector<size_t> &in2out,
                                     const bool layer_parallelize,
                                     const core::maxpool_params &params) {
#if defined(__ve__) && defined(USE_VEDNN_MAXPOOL_BACKWARD)
  dataType_t dtype {DTYPE_FLOAT};
  int batch_in {1};
  int batch_out {1};
  int padWidth {0};
  int padHeight {0};

  size_t iw {params.in.width_};
  size_t ih {params.in.height_};
  size_t id {params.in.depth_};
  size_t ow {params.out.width_};
  size_t oh {params.out.height_};
  size_t od {params.out.depth_};
  size_t windowWidth {params.pool_size_x};
  size_t windowHeight {params.pool_size_y};
  size_t strideWidth {params.stride_x};
  size_t strideHeight {params.stride_y};
  
  vednnTensorParam_t ParamIn {dtype, batch_in, id, iw, ih};
  vednnTensorParam_t ParamOut {dtype, batch_out, od, ow, oh};
  vednnPoolingParam_t ParamPool {windowWidth, windowHeight, strideWidth, strideHeight,
                                 padWidth, padHeight};
  vednnTensorParam_t ParamGradIn {dtype, batch_in, id, iw, ih};
  vednnTensorParam_t ParamGradOut {dtype, batch_out, od, ow, oh};
  
  auto *pParamIn {&ParamIn};
  auto *pParamOut {&ParamOut};
  auto *pParamPool {&ParamPool};
  auto *pParamGradout {&ParamGradOut};
  auto *pParamGradIn {&ParamGradIn};
  
  int num_prev_delta {prev_delta.size()};
  
  for(size_t sample = 0; sample < num_prev_delta; sample++) {
    auto *pDataIn {prev_data[sample].data()};
    auto *pDataOut {curr_data[sample].data()};
    auto *pDataGradIn {prev_delta[sample].data()};
    auto *pDataGradOut {curr_delta[sample].data()};
    
    vednnMaxPoolingBackward(pParamGradout, pDataGradOut, pParamOut, pDataOut, pParamIn,
                            pDataIn, pParamGradIn, pDataGradIn, pParamPool);
  }
#else
  int num_prev_delta = prev_delta.size();
  int num_in2out = in2out.size();

  for(size_t sample = 0; sample < num_prev_delta; sample++) {
    vec_t &prev                    = prev_delta[sample];
    const vec_t &curr              = curr_delta[sample];
    const std::vector<size_t> &max = max_idx[sample];

    for (size_t i = 0; i < num_in2out; i++) {
      size_t outi = in2out[i];
      prev[i]     = (max[outi] == i) ? curr[outi] : float_t{0};
    }
  }
#endif
}

}  // namespace kernels
}  // namespace tiny_dnn
