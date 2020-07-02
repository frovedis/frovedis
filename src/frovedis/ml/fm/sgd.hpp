#ifndef _FM_SGD_HPP_
#define _FM_SGD_HPP_

#include <frovedis/ml/fm/common.hpp>
#include <frovedis/ml/fm/model.hpp>
#include <frovedis/ml/utility/batch_chain_communicator.hpp>

namespace frovedis {
namespace fm {

template <class T, class I>
void aggregate_fm_parameter_to_managing_node(
    chain_schedule<I>& schedule, size_t batch_id, fm_parameter<T>& local_grad, 
    fm_parameter<T>& grad_accumulator, fm_config<T>& config) {

  typed_allreduce(&local_grad.w0, &grad_accumulator.w0, 1, MPI_SUM, frovedis_comm_rpc);

  aggregate_to_managing_node(schedule, batch_id, local_grad.w, grad_accumulator.w, 1);

  aggregate_to_managing_node(schedule, batch_id, local_grad.v.val, 
                             grad_accumulator.v.val, config.factor_size);
}

template <class T, class I>
void update_managed_fm_parameter(chain_schedule<I>& schedule, size_t batch_id,
                                 fm_parameter<T>& managed_parameter, fm_parameter<T>& grad_accumulator,
                                 fm_config<T>& config) {
  // No L1 term in FM    
  T scaled_learn_rate = config.learn_rate / config.batch_size_pernode / get_nodesize();

  T ag_w0 = grad_accumulator.w0;
  T w0 = managed_parameter.w0;   
  managed_parameter.w0 -= scaled_learn_rate * (ag_w0 + config.regular_w0 * w0);

  update_managed_parameter<T,I>(schedule, batch_id, managed_parameter.w, grad_accumulator.w, 
                                scaled_learn_rate, config.regular_w, 0, 1);

  update_managed_parameter<T,I>(schedule, batch_id, managed_parameter.v.val, grad_accumulator.v.val, 
				scaled_learn_rate, config.regular_v, 0, config.factor_size);              
                       
}

template <class T, class I>
fm_parameter<T> broadcast_fm_parameter_from_managing_node(
    chain_schedule<I>& schedule, size_t batch_id,
    fm_parameter<T>& managed_parameter, fm_config<T>& config) {

  auto w0 = managed_parameter.w0;

  auto w = broadcast_from_managing_node(schedule, batch_id, managed_parameter.w, 1);
  
  auto v_val = broadcast_from_managing_node(schedule, batch_id, managed_parameter.v.val, config.factor_size);  
  size_t local_feature_size = v_val.size() / config.factor_size;
  rowmajor_matrix_local<T> v(std::move(v_val));
  v.set_local_num(local_feature_size, config.factor_size);
  
  return fm_parameter<T>(w0, std::move(w), std::move(v));  
}

template <class T, class Matrix>
std::vector<T> compute_forward_prop(Matrix& data, Matrix& sq_data, fm_parameter<T>& parameter) {
  auto local_batch_size = data.local_num_row;
  // w0
  std::vector<T> y(local_batch_size, parameter.w0);
  auto* ptr_y = y.data();
  // w
  auto yw = data * parameter.w;
  auto* ptr_yw = yw.data();
  for (size_t ix = 0; ix < local_batch_size; ix++) {
    ptr_y[ix] += ptr_yw[ix];
  }
  // v
  std::vector<T> ones(parameter.factor_size(), 0.5);
  auto xv = data * parameter.v;
  auto sq_xv = xv.pow_val(2);
  auto sq_v = parameter.v.pow_val(2);
  auto sq_x_sq_v = sq_data * sq_v;
  auto yv = (sq_xv - sq_x_sq_v) * ones;
  
  auto* ptr_yv = yv.data();
  for (size_t ix = 0; ix < local_batch_size; ix++) {
    ptr_y[ix] += ptr_yv[ix];
  }  
  return y;
}

template <class T>
std::vector<T> compute_diff_of_loss(std::vector<T>& y, std::vector<T>& label, 
                                    bool is_regression) {
  assert(y.size() == label.size());
  
  auto local_batch_size = y.size();
  std::vector<T> diffloss(local_batch_size, 0);
  auto* ptr_diffloss = diffloss.data();
  auto* ptr_y = y.data();
  auto* ptr_lb = label.data();
  
  if (is_regression) {
    for (size_t i = 0; i < local_batch_size; i++) {
      ptr_diffloss[i] = (ptr_y[i] - ptr_lb[i]);
    }
  } else {
    for (size_t i = 0; i < local_batch_size; i++) {  
      ptr_diffloss[i] = - 1.0 * ptr_lb[i] / (1.0 + std::exp(ptr_y[i] * ptr_lb[i]));
    }
  }
  return diffloss;  
}


template <class T, class Matrix> 
fm_parameter<T> compute_backward_prop(Matrix& data, Matrix& trans_data, Matrix& trans_sq_data, 
                                      std::vector<T>& diffloss, fm_parameter<T>& parameter) {
  fm_parameter<T> grad;
  
  auto local_batch_size = diffloss.size();
  auto* ptr_diffloss = diffloss.data();
  // w0
  T sum_diffloss = 0;
  for (size_t i = 0; i < local_batch_size; i++) {
    sum_diffloss += ptr_diffloss[i];
  }
  grad.w0 = sum_diffloss;
  // w
  grad.w = trans_data * diffloss;
  // v
  auto xv = data * parameter.v;
  auto tr_sq_x_dl = trans_sq_data * diffloss;
  // auto diag_dl = moveto_diag_matrix_local<T>(diffloss);
  // grad.v = trans_data * (diag_dl * xv) - moveto_diag_matrix_local<T>(tr_sq_x_dl) * parameter.v;
  grad.v = trans_data * (moveto_diag_matrix_local<T>(diffloss) * xv) \
    - moveto_diag_matrix_local<T>(tr_sq_x_dl) * parameter.v;
  grad.v.local_num_row = grad.w.size();
  grad.v.local_num_col = grad.v.val.size() / grad.w.size();

  return grad;
}

template <class T, class Matrix>
fm_parameter<T> compute_gradient(Matrix& data, Matrix& sq_data, Matrix& trans_data, Matrix& trans_sq_data, 
                                 std::vector<T>& label, fm_parameter<T>& computed_parameter, fm_config<T>& config) {
  assert(data.local_num_col == computed_parameter.feature_size());
  
  if (data.local_num_row == 0) {
    return fm_parameter<T>();
  } else {
    auto y = compute_forward_prop(data, sq_data, computed_parameter);  
    auto diffloss = compute_diff_of_loss(y, label, config.is_regression);
    auto grad = compute_backward_prop(data, trans_data, trans_sq_data, diffloss, computed_parameter);
    return grad;
  }
}

template <class T, class I, class O, class Matrix>
std::tuple< std::vector<Matrix>, std::vector<Matrix>, std::vector<Matrix>, std::vector<Matrix> >
prepare_data_to_multiply(std::vector<crs_matrix_local<T,I,O>>& crs_data_batches) {

  auto pow_vec = +[](std::vector<crs_matrix_local<T,I,O>>& batches, T exponent) {
    std::vector<crs_matrix_local<T,I,O>> powed(batches.size());
    for (size_t i = 0; i < batches.size(); i++) {
      powed[i] = batches[i].pow_val(exponent);
    }
    return powed;
  };

#if defined(_SX) || defined(__ve__)
  auto sq_crs_data_batches = pow_vec(crs_data_batches, 2);  

  auto data_batches = to_jds_crs_vec<T,I,O>(crs_data_batches);
  auto sq_data_batches = to_jds_crs_vec<T,I,O>(sq_crs_data_batches);
  auto trans_data_batches = to_trans_jds_crs_vec<T,I,O>(crs_data_batches);
  auto trans_sq_data_batches = to_trans_jds_crs_vec<T,I,O>(sq_crs_data_batches);
#else
  auto data_batches = crs_data_batches;
  auto sq_data_batches = pow_vec(crs_data_batches, 2);
  auto trans_data_batches = to_trans_crs_vec(data_batches);
  auto trans_sq_data_batches = to_trans_crs_vec(sq_data_batches);
#endif

  return std::make_tuple(std::move(data_batches), std::move(sq_data_batches),
                         std::move(trans_data_batches), std::move(trans_sq_data_batches));                         
}

template <class T, class I, class O>
void optimize_sgd_parallel(std::vector<crs_matrix_local<T,I,O>>& crs_data_batches, std::vector<std::vector<T>>& label_batches, 
                           chain_schedule<I>& schedule, fm_parameter<T>& managed_parameter, fm_config<T>& config) {    

  extern time_spent time_agg, time_update, time_comp, time_bcast;

#if defined(_SX) || defined(__ve__)
  auto batches_tuple = prepare_data_to_multiply<T,I,O,jds_crs_hybrid_local<T,I,O>>(crs_data_batches);
#else  
  auto batches_tuple = prepare_data_to_multiply<T,I,O,crs_matrix_local<T,I,O>>(crs_data_batches);
#endif
  {  // release memory
    std::vector<crs_matrix_local<T,I,O>> tmp;
    tmp.swap(crs_data_batches);
  }
  auto& data_batches = std::get<0>(batches_tuple);
  auto& sq_data_batches = std::get<1>(batches_tuple);
  auto& trans_data_batches = std::get<2>(batches_tuple);
  auto& trans_sq_data_batches = std::get<3>(batches_tuple);

  fm_parameter<T> grad_accumulator(managed_parameter.feature_size(), 
                                   managed_parameter.factor_size());

  auto batch_count = data_batches.size();
  for (size_t epoch = 0; epoch < config.iteration; epoch++) {
    LOG(DEBUG) << "epoch: " << epoch << std::endl;

    //config.learn_rate = config.init_learn_rate / std::pow(epoch + 1, 0.5);
    
    for (size_t batch_id = 0; batch_id < batch_count; batch_id++) {
      // broadcast
      time_bcast.lap_start();
      auto computed_parameter = broadcast_fm_parameter_from_managing_node(
        schedule, batch_id, managed_parameter, config);
      time_bcast.lap_stop();
      
      // compute gradient
      time_comp.lap_start();
      auto local_grad = compute_gradient(data_batches[batch_id], sq_data_batches[batch_id], 
                                         trans_data_batches[batch_id], trans_sq_data_batches[batch_id],
                                         label_batches[batch_id], computed_parameter, config);
      time_comp.lap_stop();
      
      // aggregate
      time_agg.lap_start();
      aggregate_fm_parameter_to_managing_node(
        schedule, batch_id, local_grad, grad_accumulator, config);
      time_agg.lap_stop();
  
      // update 
      time_update.lap_start();
      update_managed_fm_parameter(schedule, batch_id, managed_parameter, grad_accumulator, config);          
      time_update.lap_stop();
      
    }
  }
  LOG(DEBUG) << "Training finished." << std::endl;
}

}  // namespace fm
}  // namespace frovedis

#endif  // _FM_SGD_HPP_
