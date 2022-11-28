#ifndef _EXRPC_TSA_HPP_
#define _EXRPC_TSA_HPP_

#include "frovedis.hpp"
#include "frovedis/core/exceptions.hpp"
#include "frovedis/ml/tsa/arima.hpp"
#include "./exrpc_ml.hpp"
#include "./exrpc_model.hpp"
#include "../exrpc/exrpc_expose.hpp"

using namespace frovedis;

template <class T>
void frovedis_arima_fit(exrpc_ptr_t& data_ptr, unsigned long& ar_lag,
                        unsigned long& diff_order, unsigned long& ma_lag, 
                        unsigned long& seasonal,
                        bool& auto_arima, std::string& solver,
                        int& verbose, int& mid) {
  dvector<T>& data = *reinterpret_cast<dvector<T>*>(data_ptr);
  set_verbose_level(verbose);
  frovedis::Arima<T> estimator(data.viewas_node_local(), ar_lag, diff_order,
                               ma_lag, seasonal, auto_arima, solver);
  estimator.fit();
  reset_verbose_level();
  handle_trained_model<Arima<T>>(mid, ARM, estimator);
}

template <class T>
std::vector<T> 
get_fitted_vector(int& mid) {
  auto model = get_model_ptr<Arima<T>>(mid);
  return model->fitted_values.template as_dvector<T>().gather();
}

template <class T>
std::vector<T>
frovedis_arima_predict(int& mid, unsigned long& start, unsigned long& stop) {
  auto model = get_model_ptr<Arima<T>>(mid);
  return model->predict(start, stop);
}

template <class T>
std::vector<T>
frovedis_arima_forecast(int& mid, unsigned long& steps) {
  auto model = get_model_ptr<Arima<T>>(mid);
  return model->forecast(steps);
}
template <class T>
std::vector<T>
  diff_helper(std::vector<T>& vec) {
    auto vecp = vec.data();
    auto sz = vec.size();
    if (sz < 2) return std::vector<T>();
    std::vector<T> diff_vec(sz - 1);
    auto diff_vecp = diff_vec.data();
    for (size_t i = 0; i < sz - 1; ++i) {
        diff_vecp[i] = vecp[i+1] - vecp[i];
    }
    return diff_vec;
}

template <class T>
T
distinct_check_helper(std::vector<T>& data) {
  auto datap = data.data();
  auto sz = data.size();
  T distinct_val = datap[0];
  if (sz > 1) {
    for (size_t i = 1; i < sz; ++i) {
      if (distinct_val != datap[i]) {
        distinct_val = std::numeric_limits<T>::max();
        break;
      }
    }
  }
  return distinct_val;
}

template <class T>
size_t sum (size_t a, size_t b) {
  return a + b;
}

template <class T>
size_t vec_count (std::vector<T>& vec) {
  return vec.size();
}

template <class T>
node_local<std::vector<T>>
nlvector_diff(exrpc_ptr_t& data_ptr, std::string& col) {
  dftable& df = *reinterpret_cast<dftable*>(data_ptr);
  auto dfcol = df.column(col);
  use_dfcolumn use(dfcol);
  auto data = std::dynamic_pointer_cast<typed_dfcolumn<T>>(dfcol);
  auto vx = data->val.template as_dvector<T>().gather();
  auto s = data->val.map(vec_count<T>).template reduce(sum<T>);
  if (s < 2) {
    std::cout << "Size of given input is less than 2. Minimum required size is" 
              << "two. \nExiting.\n";
    exit(-1);
  }
  auto l_shadow = extend_lower_shadow(data->val, 1);
  node_local<std::vector<T>> ret = l_shadow.map(diff_helper<T>);
  return ret;
}

template <class T>
long frovedis_get_frequency(exrpc_ptr_t& data_ptr, std::string& col) {
  auto diff_nlv = nlvector_diff<T>(data_ptr, col);
  std::vector<T> diff_v = diff_nlv.map(distinct_check_helper<long>).gather();
  auto x = distinct_check_helper<long>(diff_v);
  return x;
}


#endif
