#ifndef _FM_MAIN_HPP_
#define _FM_MAIN_HPP_

#include <frovedis/ml/fm/sgd.hpp>
#include <frovedis/ml/fm/common.hpp>

#include <frovedis/matrix/shrink_matrix.hpp>
#include <frovedis/ml/utility/batch_chain_communicator.hpp>


namespace frovedis {

namespace fm {

extern time_spent time_agg, time_update, time_comp, time_bcast;

enum struct FmOptimizer {
  SGD = 0,
  SGDA,
  ALS,
  MCMC
};

template <class T, class I = size_t, class O = size_t>
fm_model<T> train(crs_matrix<T,I,O>& nl_data,
                  node_local<std::vector<T>>& nl_label,
                  FmOptimizer optimizer,
                  fm_config<T>& conf,
                  int random_seed = 1) {
  conf.feature_size = nl_data.num_col;
  size_t feature_size = conf.feature_size;
  size_t factor_size = conf.factor_size;
  size_t batch_size_pernode = conf.batch_size_pernode;
  T init_stdev = conf.init_stdev;
  
  // divide data, label
  auto nl_data_batches = broadcast(std::vector<crs_matrix_local<T,I,O>>());
  auto nl_label_batches = broadcast(std::vector<std::vector<T>>());
  nl_data.data.mapv(divide_to_minibatch_with_size<T,I,O>, nl_label,
                    nl_data_batches, nl_label_batches,
                    broadcast(batch_size_pernode));

  // create schedule
  node_local<chain_schedule<I>> nl_schedule = create_schedule_from_batches(nl_data_batches, factor_size);
  std::vector<size_t> local_feature_sizes = nl_schedule.map(
    +[](chain_schedule<I>& sched){ return sched.managed_counts; }
  ).get(0);

  // omit indices not appeared
  auto present_level = get_loglevel();
  set_loglevel(INFO);
  
  nl_data_batches.mapv(+[](std::vector<crs_matrix_local<T,I,O>>& data_batches){
    auto count = data_batches.size();
    for (size_t i = 0; i < count; i++) {
      shrink_column_local(data_batches[i]);
    }
  });
  
  set_loglevel(present_level);
  
  auto nl_managed_parameter = make_local_fm_parameter<T>(local_feature_sizes, factor_size);
  nl_managed_parameter.mapv(init_fm_parameter_with_stdev<T>, broadcast(init_stdev), broadcast(random_seed));
  
  auto nl_conf = broadcast(conf);
  switch (optimizer) {
    case FmOptimizer::SGD :
      nl_data_batches.mapv(optimize_sgd_parallel<T,I,O>, nl_label_batches, 
                           nl_schedule, nl_managed_parameter, nl_conf);  
      break;
      
    default:
      REPORT_ERROR(INTERNAL_ERROR, "This optimizer is not implemented yet.");
      break;
  }
  
  time_comp.show("Compute: ");
  time_agg.show("Aggregate: ");
  time_bcast.show("Broadcast: ");
  time_update.show("Update: ");

  auto opt_parameter = concatenate_fm_parameter<T>(nl_managed_parameter);
  assert(feature_size == opt_parameter.feature_size());
  return fm_model<T>(conf, opt_parameter);
}

template <class T, class I = size_t, class O = size_t>
fm_model<T> train(bool dim_0, bool dim_1, size_t dim_2, 
		  T init_stdev, size_t iteration, T init_learn_rate, FmOptimizer optimizer,
		  T regular_0, T regular_1, T regular_2, bool is_regression,
		  crs_matrix<T,I,O>& nl_data, node_local<std::vector<T>>& nl_label, size_t batch_size_pernode, int random_seed) {
                                              
  fm_config<T> conf(dim_0, dim_1, dim_2, init_stdev, iteration, init_learn_rate, 
                    regular_0, regular_1, regular_2, is_regression, batch_size_pernode);
  return train(nl_data, nl_label, optimizer, conf, random_seed);
}

template <class T, class I = size_t, class O = size_t>
T test(fm_model<T>& trained_model, crs_matrix_local<T,I,O>& test_data, std::vector<T>& label) {
  if (test_data.local_num_row != label.size()) {
    throw std::runtime_error("inconsistent the size of test data and label.");
  }
  size_t datasize = test_data.local_num_row;
  
  auto pred = trained_model.predict(test_data);
  bool is_regression = trained_model.config.is_regression;
  
  auto* ptr_pred = pred.data();
  auto* ptr_label = label.data();
  if (is_regression) {
    T rmse = 0;
    for (size_t ix = 0; ix < datasize; ix++) {
      rmse += std::pow(ptr_pred[ix] - ptr_label[ix], 2.0);
    }
    return std::pow(rmse / datasize, 0.5);
  } else {
    T accuracy;
    size_t num_correct = 0;
    for (size_t ix = 0; ix < datasize; ix++) {
      bool is_correct = (ptr_pred[ix] * ptr_label[ix] > 0);
      if (is_correct) num_correct++;
    }    
    accuracy = 1.0 * num_correct / datasize;
    return accuracy;
  }
}

}  // namespace fm
}  // namespace frovedis

#endif  // _FM_MAIN_HPP_
