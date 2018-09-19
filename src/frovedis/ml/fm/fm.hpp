#ifndef _FM_HPP_
#define _FM_HPP_

#include <frovedis/ml/fm/main.hpp>

namespace frovedis {

using FmOptimizer = fm::FmOptimizer;

template <class T, class I = size_t, class O = size_t>
fm::fm_model<T> fm_train(bool dim_0, bool dim_1, size_t dim_2, 
                         T init_stdev, size_t iteration, T init_learn_rate, FmOptimizer optimizer,
                         T regular_0, T regular_1, T regular_2, bool is_regression,
                         crs_matrix<T,I,O>& nl_data, dvector<T>& dv_label, size_t batch_size_pernode, int random_seed = 1) {
   
  //auto nl_label = dv_label.moveto_node_local();
  auto nl_label = dv_label.viewas_node_local();

  return fm::train(
    dim_0, dim_1, dim_2, init_stdev, iteration, 
    init_learn_rate, optimizer, regular_0, regular_1, regular_2, 
    is_regression, nl_data, nl_label, batch_size_pernode, random_seed
  );                        
}

template <class T, class I = size_t, class O = size_t>
T fm_test(fm::fm_model<T>& trained_model, crs_matrix_local<T,I,O>& test_data, std::vector<T>& label) {

  return fm::test(trained_model, test_data, label);
}

template <class T>
fm::fm_model<T> load_fm_model(const std::string& input) {
  return fm::load_fm_model<T>(input);
}


}  // namespace frovedis

#endif  // _FM_MAIN_HPP_
