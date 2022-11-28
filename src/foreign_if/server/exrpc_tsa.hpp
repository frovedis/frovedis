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

#endif
