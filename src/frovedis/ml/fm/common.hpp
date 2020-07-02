#ifndef _FM_COMMON_HPP_
#define _FM_COMMON_HPP_

#include <assert.h>
#include <frovedis/core/exceptions.hpp>
#include <frovedis/ml/utility/matrix_conversion.hpp>

namespace frovedis {
namespace fm {


template <class T, class I, class O> 
void
divide_to_minibatch_with_size(crs_matrix_local<T,I,O>& data, std::vector<T>& label, 
                              std::vector<crs_matrix_local<T,I,O>>& data_batches,
                              std::vector<std::vector<T>>& label_batches,
                              size_t pre_batch_size) {
  assert(pre_batch_size > 0);
  
  size_t local_batch_count = ceil_div(data.local_num_row, pre_batch_size);
  // TODO: extend typed_allreduce() and make below concise.
  float local_batch_count_f = local_batch_count;
  float batch_count_f;
  typed_allreduce(&local_batch_count_f, &batch_count_f, 1, MPI_MAX, frovedis_comm_rpc);
  size_t batch_count = batch_count_f; 
  LOG(DEBUG) << "Number of minibatches: " << batch_count << std::endl;
  
  data_batches = std::vector<crs_matrix_local<T,I,O>>(batch_count);
  label_batches = std::vector<std::vector<T>>(batch_count);
  
  for (size_t ib = 0; ib < batch_count; ib++) {
    // data
    auto& batch = data_batches[ib];
    batch.local_num_col = data.local_num_col;

    auto start_row = std::min(ib * pre_batch_size, data.local_num_row);
    auto end_row =  std::min((ib+1) * pre_batch_size, data.local_num_row);
    batch.local_num_row = end_row - start_row;
    
    auto start_off = data.off[start_row];
    auto end_off = data.off[end_row];
    auto off_size = end_off - start_off;

    batch.val.resize(off_size);
    batch.idx.resize(off_size);
    batch.off.resize(end_row - start_row + 1); 
    
    auto* ptr_val = data.val.data();
    auto* ptr_idx = data.idx.data();
    auto* ptr_batch_val = batch.val.data();
    auto* ptr_batch_idx = batch.idx.data();
    for(size_t j = 0; j < off_size; j++) {
      ptr_batch_val[j] = ptr_val[j + start_off];
      ptr_batch_idx[j] = ptr_idx[j + start_off];
    }
    
    auto* ptr_off = data.off.data();
    auto* ptr_batch_off = batch.off.data();
    for(size_t j = 0; j < end_row - start_row + 1; j++) {
      ptr_batch_off[j] = ptr_off[start_row + j] - start_off;
    }   
    
    // label
    auto& batch_lb = label_batches[ib];
    batch_lb.resize(end_row - start_row);
    auto* ptr_lb = label.data();
    auto* ptr_batch_lb = batch_lb.data();
    for (size_t j = 0; j < end_row - start_row; j++) {
      ptr_batch_lb[j] = ptr_lb[start_row + j];
    }    
  }
  // return std::make_tuple(std::move(data_batches), std::move(label_batches));
}
  
  
}  // namespace fm
}  // namespace frovedis


#endif  // _FM_COMMON_HPP_

