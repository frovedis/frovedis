#ifndef DNN_HPP
#define DNN_HPP

#include "tiny_dnn/config.h"
#include "tiny_dnn/network.h"
#include "tiny_dnn/nodes.h"
#include "tiny_dnn/core/framework/device.h"
#include "tiny_dnn/core/framework/program_manager.h"
#include "tiny_dnn/lossfunctions/loss_function.h"
#include "tiny_dnn/optimizers/optimizer.h"
#include "tiny_dnn/util/util.h"
#include "tiny_dnn/util/product.h"
#include "tiny_dnn/util/deserialization_helper.h"
#include "tiny_dnn/util/serialization_helper.h"
#include "tiny_dnn/util/serialization_upcast.h"

#include "dist_optimizer.hpp"
#include "../../core/tuple_serializer.hpp"

namespace frovedis {

template <class Error,
          class Network,
          class Optimizer,
          class OnBatchEnumerate,
          class OnEpochEnumerate>
void dnn_train_local(std::vector<tiny_dnn::vec_t>& inputs,
                     std::vector<tiny_dnn::label_t>& class_labels,
                     std::vector<intptr_t> intptrs,
                     std::string& ser_nn, 
                     Optimizer& optimizer,
                     size_t batch_size,
                     int epoch,
                     OnBatchEnumerate on_batch_enumerate,
                     OnEpochEnumerate on_epoch_enumerate) {
  if(get_selfid() == 0) {
    Network& nn = *reinterpret_cast<Network*>(intptrs[0]);
    OnBatchEnumerate& on_batch_enumerate0 =
      *reinterpret_cast<OnBatchEnumerate*>(intptrs[1]);
    OnEpochEnumerate& on_epoch_enumerate0 =
      *reinterpret_cast<OnEpochEnumerate*>(intptrs[2]);
    nn.template train<Error>(optimizer, inputs, class_labels, batch_size,
                             epoch, on_batch_enumerate0, on_epoch_enumerate0);
  } else {
    Network nn;
    std::istringstream iss(ser_nn);
    cereal::BinaryInputArchive bi(iss);
    nn.from_archive(bi, tiny_dnn::content_type::weights_and_model);
    nn.template train<Error>(optimizer, inputs, class_labels, batch_size,
                             epoch, on_batch_enumerate, on_epoch_enumerate);
  }
}

template <class Error,
          class Network,
          class Optimizer,
          class OnBatchEnumerate,
          class OnEpochEnumerate>
void dnn_train(Network& nn, 
               Optimizer& optimizer,
               dvector<tiny_dnn::vec_t>& inputs,
               dvector<tiny_dnn::label_t>& class_labels,
               size_t local_batch_size, // note that this is local size!
               int epoch,
               OnBatchEnumerate& on_batch_enumerate,
               OnEpochEnumerate& on_epoch_enumerate) {
  class_labels.align_to(inputs);
  std::ostringstream oss;
  {
    cereal::BinaryOutputArchive bo(oss);
    nn.to_archive(bo, tiny_dnn::content_type::weights_and_model);
  }
  std::string ser_nn = oss.str();
  intptr_t nn_ptr = reinterpret_cast<intptr_t>(&nn);
  intptr_t on_batch_enumerate_ptr =
    reinterpret_cast<intptr_t>(&on_batch_enumerate);
  intptr_t on_epoch_enumerate_ptr =
    reinterpret_cast<intptr_t>(&on_epoch_enumerate);
  std::vector<intptr_t> intptrs = {nn_ptr,
                                   on_batch_enumerate_ptr,
                                   on_epoch_enumerate_ptr};
                                 
  inputs.viewas_node_local().
    mapv(dnn_train_local<Error,Network,Optimizer,OnBatchEnumerate,
         OnEpochEnumerate>,
         class_labels.viewas_node_local(),
         broadcast(intptrs),
         broadcast(ser_nn),
         broadcast(optimizer),
         broadcast(local_batch_size),
         broadcast(epoch),
         broadcast(on_batch_enumerate),
         broadcast(on_epoch_enumerate));
}


}
#endif
