/*
    Copyright (c) 2013, Taiga Nomi and the respective contributors
    All rights reserved.

    Use of this source code is governed by a BSD-style license that can be found
    in the LICENSE file.
*/
#pragma once

#include "tiny_dnn/config.h"
#include "tiny_dnn/network.h"
#include "tiny_dnn/nodes.h"

// comment out to allow compilation with gcc 4.8.5
//#include "tiny_dnn/core/framework/tensor.h"

#include "tiny_dnn/core/framework/device.h"
#include "tiny_dnn/core/framework/program_manager.h"

#include "tiny_dnn/activations/asinh_layer.h"
#include "tiny_dnn/activations/elu_layer.h"
#include "tiny_dnn/activations/leaky_relu_layer.h"
#include "tiny_dnn/activations/relu_layer.h"
#include "tiny_dnn/activations/selu_layer.h"
#include "tiny_dnn/activations/sigmoid_layer.h"
#include "tiny_dnn/activations/softmax_layer.h"
#include "tiny_dnn/activations/softplus_layer.h"
#include "tiny_dnn/activations/softsign_layer.h"
#include "tiny_dnn/activations/tanh_layer.h"
#include "tiny_dnn/activations/tanh_p1m2_layer.h"
#include "tiny_dnn/layers/arithmetic_layer.h"
#include "tiny_dnn/layers/average_pooling_layer.h"
#include "tiny_dnn/layers/average_unpooling_layer.h"
#include "tiny_dnn/layers/batch_normalization_layer.h"
#include "tiny_dnn/layers/cell.h"
#include "tiny_dnn/layers/cells.h"
#include "tiny_dnn/layers/concat_layer.h"
#include "tiny_dnn/layers/convolutional_layer.h"
#include "tiny_dnn/layers/deconvolutional_layer.h"
#include "tiny_dnn/layers/dropout_layer.h"
#include "tiny_dnn/layers/fully_connected_layer.h"
#include "tiny_dnn/layers/global_average_pooling_layer.h"
#include "tiny_dnn/layers/input_layer.h"
#include "tiny_dnn/layers/linear_layer.h"
#include "tiny_dnn/layers/lrn_layer.h"
#include "tiny_dnn/layers/max_pooling_layer.h"
#include "tiny_dnn/layers/max_unpooling_layer.h"
#include "tiny_dnn/layers/power_layer.h"
#include "tiny_dnn/layers/quantized_convolutional_layer.h"
#include "tiny_dnn/layers/quantized_deconvolutional_layer.h"
#include "tiny_dnn/layers/recurrent_layer.h"
#include "tiny_dnn/layers/slice_layer.h"
#include "tiny_dnn/layers/zero_pad_layer.h"

#ifdef CNN_USE_GEMMLOWP
#include "tiny_dnn/layers/quantized_fully_connected_layer.h"
#endif  // CNN_USE_GEMMLOWP

#include "tiny_dnn/lossfunctions/loss_function.h"
#include "tiny_dnn/optimizers/optimizer.h"

#include "tiny_dnn/util/deform.h"
#include "tiny_dnn/util/graph_visualizer.h"
#include "tiny_dnn/util/product.h"
#include "tiny_dnn/util/weight_init.h"

#include "tiny_dnn/io/cifar10_parser.h"
#include "tiny_dnn/io/display.h"
#include "tiny_dnn/io/layer_factory.h"
#include "tiny_dnn/io/mnist_parser.h"

#ifdef DNN_USE_IMAGE_API
#include "tiny_dnn/util/image.h"
#endif  // DNN_USE_IMAGE_API

#ifndef CNN_NO_SERIALIZATION
#include "tiny_dnn/util/deserialization_helper.h"
#include "tiny_dnn/util/serialization_helper.h"
#include "tiny_dnn/util/serialization_upcast.h"
/* // separate to different file
// to allow upcasting
CEREAL_REGISTER_TYPE(tiny_dnn::elu_layer)
CEREAL_REGISTER_TYPE(tiny_dnn::leaky_relu_layer)
CEREAL_REGISTER_TYPE(tiny_dnn::relu_layer)
CEREAL_REGISTER_TYPE(tiny_dnn::sigmoid_layer)
CEREAL_REGISTER_TYPE(tiny_dnn::softmax_layer)
CEREAL_REGISTER_TYPE(tiny_dnn::softplus_layer)
CEREAL_REGISTER_TYPE(tiny_dnn::softsign_layer)
CEREAL_REGISTER_TYPE(tiny_dnn::tanh_layer)
CEREAL_REGISTER_TYPE(tiny_dnn::tanh_p1m2_layer)
*/
#endif  // CNN_NO_SERIALIZATION

// shortcut version of layer names
// are moved to each layer definition
namespace tiny_dnn {
#include "tiny_dnn/models/alexnet.h"
}  // namespace tiny_dnn

#ifdef CNN_USE_CAFFE_CONVERTER
// experimental / require google protobuf
#include "tiny_dnn/io/caffe/layer_factory.h"
#endif
