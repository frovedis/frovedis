/*
    Copyright (c) 2013, Taiga Nomi and the respective contributors
    All rights reserved.

    Use of this source code is governed by a BSD-style license that can be found
    in the LICENSE file.
*/
#pragma once

#include <algorithm>
#include <string>
#include <utility>

#include "tiny_dnn/activations/activation_layer.h"
#include "tiny_dnn/layers/layer.h"
#ifdef __ve__
#include "vednn/vednn.h"
// #define USE_VEDNN_RELU
#endif

namespace tiny_dnn {

class relu_layer : public activation_layer {
 public:
  using activation_layer::activation_layer;

  std::string layer_type() const override { return "relu-activation"; }

  void forward_activation(const vec_t &x, vec_t &y) override {
#if defined(__ve__) && defined(USE_VEDNN_RELU)
    vednnActivationMode_t mode  = VEDNN_ACTIVATION_RELU;
    auto *pDataIn = x.data();
    auto *pDataOut = y.data();
    const unsigned long nElements {x.size()};
    vednnActivationForward(mode, pDataIn, pDataOut, nElements);
#else
    int num_x = x.size();
    for (size_t j = 0; j < num_x; ++j) {
      y[j] = (x[j] > float_t(0) ? x[j] : float_t(0));
    }
#endif
  }

  void backward_activation(const vec_t &x,
                           const vec_t &y,
                           vec_t &dx,
                           const vec_t &dy) override {
#if defined(__ve__) && defined(USE_VEDNN_RELU)
    vednnActivationMode_t mode = VEDNN_ACTIVATION_RELU;
    auto *pDataIn = x.data();
    auto *pDataGradIn = dx.data();
    auto *pDataGradOut = dy.data();
    const unsigned long nElements = x.size();
    vednnActivationBackward(mode, pDataGradOut, pDataIn, pDataGradIn, nElements);
#else
    int num_x = x.size();
    for (size_t j = 0; j < num_x; ++j) {
      dx[j] = dy[j] * (y[j] > float_t(0) ? float_t(1) : float_t(0));
    }
#endif
  }

  std::pair<float_t, float_t> scale() const override {
    return std::make_pair(float_t(0.1), float_t(0.9));
  }

  friend struct serialization_buddy;
};

}  // namespace tiny_dnn

namespace tiny_dnn {
namespace activation {
using relu = tiny_dnn::relu_layer;
using rectified_linear = tiny_dnn::relu_layer;
}
}
