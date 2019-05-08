/*
    Copyright (c) 2013, Taiga Nomi and the respective contributors
    All rights reserved.

    Use of this source code is governed by a BSD-style license that can be found
    in the LICENSE file.
*/
#pragma once
#ifdef __ve__
#include "vednn/vednn.h"
#endif

namespace tiny_dnn {
namespace kernels {

inline void conv2d_op_ve(const tensor_t &in_data,
                               const vec_t &W,
                               const vec_t &bias,
                               tensor_t &out_data,
                               const core::conv_params &params,
                               const bool parallelize) {

#ifdef __ve__ // fail safe  
  size_t default_iw {params.in.width_}; 
  size_t default_ih {params.in.height_}; 
  size_t iw          = params.in_padded.width_;
  size_t ih          = params.in_padded.height_;
  size_t id          = params.in.depth_;
  size_t ow          = params.out.width_;
  size_t oh          = params.out.height_;
  size_t od          = params.out.depth_;
  size_t kw          = params.weight.width_;
  size_t kh          = params.weight.height_;
  size_t w_dilation  = params.w_dilation;
  size_t h_dilation  = params.h_dilation;
  size_t strideWidth = params.w_stride;
  size_t strideHeight = params.h_stride;

  dataType_t dtype {DTYPE_FLOAT};
  int batch_in {1};
  int batch_out {1};
  int padWidth {iw - default_iw};
  int padHeight {ih - default_ih};

  int group {1};

  vednnTensorParam_t ParamIn {dtype, batch_in, id, iw, ih};
  vednnFilterParam_t ParamKernel {dtype, iw, ow, kw, kh};
  vec_t DataKernel {W};
  vednnBiasParam_t ParamBias {dtype, id};
  vec_t DataBias {bias};
  vednnTensorParam_t ParamOut {dtype, batch_out, od, ow, oh};
  vednnConvolutionParam_t ParamConv {group, strideWidth, strideHeight, padWidth, padHeight, w_dilation, h_dilation};
  vednnConvolutionAlgorithm_t algo {VEDNN_CONV_ALGORITHM_DIRECT};

  auto *pParamIn {&ParamIn};
  auto *pParamKernel {&ParamKernel};
  auto *pDataKernel {DataKernel.data()};
  auto *pParamBias {&ParamBias};
  auto *pDataBias {DataBias.data()};
  auto *pParamOut {&ParamOut};
  auto *pParamConv {&ParamConv};
        
  int num_in_data {in_data.size()};

  for (size_t sample = 0u; sample < num_in_data; ++sample) {
      
    auto *pDataIn {in_data[sample].data()};
    auto *pDataOut {out_data[sample].data()};
    
    if(params.has_bias){
      vednnConvolutionForwardAddBias(pParamIn, pDataIn, pParamKernel, pDataKernel, pParamBias, pDataBias,
                                   pParamOut, pDataOut, pParamConv, algo);
    } else {
    vednnConvolutionForward(pParamIn, pDataIn, pParamKernel, pDataKernel,
                            pParamOut, pDataOut, pParamConv, algo);
    }    
  }
#else
  throw std::runtime_error("Calling Vector Engine code");
#endif
}


/******************************************************************/

template <typename tensor_t, typename vec_t>
void conv2d_op_ve(const tensor_t &prev_out,
                        const vec_t &W,
                        tensor_t &dW,
                        tensor_t &db,
                        tensor_t &curr_delta,
                        tensor_t &prev_delta,
                        const core::conv_params &params,
                        const bool parallelize) {
#ifdef __ve__ // fail safe
  typedef typename vec_t::value_type float_t;

  size_t default_iw {params.in.width_}; 
  size_t default_ih {params.in.height_}; 
  size_t iw          = params.in_padded.width_;
  size_t ih          = params.in_padded.height_;
  size_t id          = params.in.depth_;
  size_t ow          = params.out.width_;
  size_t oh          = params.out.height_;
  size_t od          = params.out.depth_;
  size_t kw          = params.weight.width_;
  size_t kh          = params.weight.height_;
  size_t kd          = params.weight.depth_;
  size_t w_dilation  = params.w_dilation;
  size_t h_dilation  = params.h_dilation;
  size_t strideWidth = params.w_stride;
  size_t strideHeight = params.h_stride;
  
  dataType_t dtype {DTYPE_FLOAT};
  int batch_in {1};
  int batch_out {1};
  int padWidth {iw - default_iw};
  int padHeight {ih - default_ih};
  int group {1};

  vednnTensorParam_t ParamIn {dtype, batch_in, id, iw, ih};
  vednnFilterParam_t ParamKernel {dtype, id, od, kw, kh};
  vec_t DataKernel {W};
  vednnTensorParam_t ParamOut {dtype, batch_out, od, ow, oh};
  vednnConvolutionParam_t ParamConv {group, strideWidth, strideHeight, padWidth, padHeight, w_dilation, h_dilation};
  vednnConvolutionAlgorithm_t algo {VEDNN_CONV_ALGORITHM_DIRECT};

  vednnTensorParam_t ParamGradIn {dtype, batch_in, id, iw, ih};
  vednnFilterParam_t ParamGradKernel {dtype, id, od, kw, kh};
  vednnTensorParam_t ParamGradOut {dtype, batch_out, od, ow, oh};
  
  auto *pParamIn {&ParamIn};
  auto *pParamKernel {&ParamKernel};
  auto *pDataKernel {DataKernel.data()};
  auto *pParamOut {&ParamOut};
  auto *pParamConv {&ParamConv};
        
  auto *pParamGradIn {&ParamGradIn};
  auto *pParamGradKernel {&ParamGradKernel};
  auto *pParamGradOut {&ParamGradOut};
  
  int num_prev_out {prev_out.size()};

  for(size_t sample; sample < num_prev_out; ++sample) {
  
    auto *pDataIn {prev_out[sample].data()};
    auto *pDataGradIn {prev_delta[sample].data()};
    auto *pDataGradKernel {dW[sample].data()};
    auto *pDataGradOut {curr_delta[sample].data()};
    
    vednnConvolutionBackwardData(pParamGradIn, pDataGradIn, pParamKernel, pDataKernel,
                                 pParamGradOut, pDataGradOut, pParamConv,	algo);
    vednnConvolutionBackwardFilter(pParamIn, pDataIn, pParamGradOut, pDataGradOut,
                                   pParamGradKernel, pDataGradKernel, pParamConv, algo);
  }
#else
  throw std::runtime_error("Calling Vector Engine code");
#endif
}

}  // namespace kernels
}  // namespace tiny_dnn
