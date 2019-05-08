/*
    Copyright (c) 2013, Taiga Nomi and the respective contributors
    All rights reserved.

    Use of this source code is governed by a BSD-style license that can be found
    in the LICENSE file.
*/
#pragma once

#include <string>
#include <utility>

#include "tiny_dnn/activations/activation_layer.h"
#include "tiny_dnn/layers/layer.h"
#ifdef __ve__
#include "vednn/vednn.h"
// #define USE_VEDNN_SOFTMAX
#endif

namespace tiny_dnn {

class softmax_layer : public activation_layer {
 public:
  using activation_layer::activation_layer;

  std::string layer_type() const override { return "softmax-activation"; }

  void forward_activation(const vec_t &x, vec_t &y) override {
#if defined(__ve__) && defined(USE_VEDNN_SOFTMAX)
    const vednnSoftmaxMode_t mode = VEDNN_SOFTMAX_FAST;
    //const vednnSoftmaxMode_t mode = VEDNN_SOFTMAX_ACCURATE;
    //const vednnSoftmaxMode_t mode = VEDNN_SOFTMAX_LOG;

    const int nBatch = 1;
    int nClass = y.size();
    auto *pDataIn = x.data();
    auto *pDataOut = y.data();

    vednnSoftmaxForward(mode, pDataIn, pDataOut, nBatch, nClass);
#else
    const float_t alpha = *std::max_element(x.begin(), x.end());
    float_t denominator = 0;
    auto num_x = x.size();
    
    for (size_t j = 0; j < num_x; j++) {
      y[j] = std::exp(x[j] - alpha);
      denominator += y[j];
    }

    auto inv_denominator = 1.0 / denominator;
    for (size_t j = 0; j < num_x; j++) {
      y[j] *= inv_denominator;
    }
#endif
  }

  void backward_activation(const vec_t &x,
                           const vec_t &y,
                           vec_t &dx,
                           const vec_t &dy) override {
    const size_t len = dy.size();

// auxilliary vector to store element wise softmax gradients of all elements

#if HAS_CXX11_THREAD_LOCAL
    thread_local
#endif
    vec_t df(len, 0);
    int num_x = x.size();

    for (size_t j = 0; j < num_x; ++j) {
      for (size_t k = 0; k < num_x; ++k) {
        auto temp = (k == j) ? y[j] * (float_t(1) - y[j]) : - y[k] * y[j];
        dx[j] += dy[k] * temp;
      }
    }
  }

  std::pair<float_t, float_t> scale() const override {
    return std::make_pair(float_t(0), float_t(1));
  }

  friend struct serialization_buddy;
};

}  // namespace tiny_dnn

namespace tiny_dnn {
namespace activation {
using softmax = tiny_dnn::softmax_layer;
}
}
