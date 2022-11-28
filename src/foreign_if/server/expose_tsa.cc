#include "exrpc_tsa.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"
#include "short_hand_model_type.hpp"

using namespace frovedis;

void expose_frovedis_tsa() {
  // (1) ARIMA fit()
  expose((frovedis_arima_fit<DT2>));  //for python
  expose((frovedis_arima_fit<DT1>));  //for python
  // ---- (2) fitted_value vector
  expose((get_fitted_vector<DT1>));  //for python
  expose((get_fitted_vector<DT2>));  //for python
  // ---- (3) ARIMA predict()
  expose(frovedis_arima_predict<DT2>);  //for python
  expose(frovedis_arima_predict<DT1>);  //for python
  // ---- (4) ARIMA forecast()
  expose(frovedis_arima_forecast<DT2>);  //for python
  expose(frovedis_arima_forecast<DT1>);  //for python
  // ---- (5) ARIMA release()
  expose(release_model<ARM1>);  //for python
  expose(release_model<ARM2>);  //for python
}
