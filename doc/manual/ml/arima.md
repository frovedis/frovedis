% Timeseries analysis using ARIMA 

# NAME  

ARIMA - Autoregressive Integrated Moving Average model is a time series model that is used 
to forecast data based on the dataset of past to predict/forecast the future.  

# SYNOPSIS  

`#include <frovedis/ml/tsa/arima.hpp>`  

## Constructors  

Arima(const node_local<std::vector<T>>& sample_data_l,  
      size_t ar_lag_l = 1,  
      size_t diff_order_l = 0,  
      size_t ma_lag_l = 0,  
      size_t seasonal_l = 0,  
      bool auto_arima_l = false,  
      std::string solver_l = "lapack")  

## Public Member Functions  

void fit()  

std::vector<T>  
predict(size_t start_step, size_t stop_step)  

std::vector<T>  
forecast(size_t steps = 1)  

# DESCRIPTION  
Frovedis provides a timeseries model in order to predict the future values based on the past 
values.  
Each component in ARIMA functions as a parameter with a standard notation. For ARIMA models, a 
standard notation would be ARIMA with p(AR), d(I), and q(MA) which indicate the type of ARIMA 
model to be used. A `0` value can be used as a parameter and would mean that the particular component 
should not be used in the model. This way, the ARIMA model can be constructed to perform the 
function of an ARMA model, or even simple AR (1,0,0), I(0,1,0), or MA(0,0,1) models. **However, the 
current implementaion cannot be used to construct a pure MA model.**  

Frovedis ARIMA is comparatively faster than statsmodel ARIMA for large timeseries dataset as it is 
light weighted and having accuracy as good as statsmodel. Also, it provides the feature of auto ARIMA 
which can fit the best lag for AR and MA. This is a useful feature for the users who do not have knowledge 
about data analytics and those who do not want to use ACF, PACF and ADfuller tests in order to judge the 
data first before calling ARIMA.  

Unlike statsmodel ARIMA, it does not use MLE (Maximum Likelihood Estimation), rather it uses 
OLS (Ordinary Least Squares).  
**Note:-** Also, rather than converging around the mean after some number of predictions, it tends to follow 
the trend i.e it diverges towards increasing or decreasing trend.  

## Detailed Description 
 
### 1. Arima()   
__Parameters__   
**_sample\_data\_l_**: A `node_local<std::vector<T>>` containing the timeseries data.  
**_ar\_lag\_l_**: A `size_t` parameter which specifies the order (lags used) of the Auto-Regressive 
Model. Currently, this parameter must be greater than or equal to 1.  
**_diff\_order\_l_**: A `size_t` parameter which specifies the order of differencing. It must be a 
positive parameter.  
**_ma\_lag\_l_**: A `size_t` parameter which specifies the order (lags used) of the Moving-Average 
Model. It must be a positive parameter.  
**_seasonal_**: A `size_t` parameter which specifies the interval of  seasonal differencing. In case the 
data has some seasonality , then it can  handle it . It must be a positive parameter. This is an 
optional parameter.  
**_auto\_arima_**: A `bool` parameter which specifies whether to use auto (brute) ARIMA. If set to `true`, it 
treats the ar_lag_l and ma_lag_l parameter as the highest limit for its iteration and auto fits the lags with 
the best RMSE score. This is an optional parameter.  
**_solver_**: A `string` parameter which specifies the solver to be used for linear regression. By default, 
`lapack` solver will be used. It supports `lapack`, `scalapack`, `lbfgs` and `sgd` solver. When 
specified, e.g `lbfgs`, then it uses lbfgs solver for linear regression.  
**Note:-** To get the best performance and accuracy from ARIMA, use solver='lapack'.  

__Purpose__  
It initializes the Arima instance with the given parameters.  

Currently, the number of samples in the timeseries data must be greater than sum of ARIMA 
order (ar_lag_l, diff_order_l, ma_lag_l, seasonal).  

    `N_samples >= (ar_lag_l + diff_order_l + ma_lag_l + seasonal)`

__Return Value__  
It simply returns the instance of Arima class.  

### 2. void fit()  

__Purpose__  
It is used to fit the model parameters on the basis of given parameters and data provided 
in Arima constructor.  

__Return Value__  
void  

### 3. std::vector<T>  predict(size_t start_step, size_t stop_step)  
__Parameters__ 
**_start\_step_**: A `size_t` parameter which specifies the staring index from which the values 
are to be predicted. This parameter must be greater than or eqaul to 0.  
**_stop\_step_**: A `size_t` parameter which specifies the index till which the values are to 
be predicted. This parameter must be greater than or equal to 0.  

__Purpose__  
It is used to perform in-sample prediction and out-of-sample prediction.  

During prediction, `stop_step` must not be less than `start_step`.  

For prediction:  
- When `start_step` and `stop_step` are both less than the order, then it will return a 
`std::vector<T>` having only the fitted values as predictions.  
- When `start_step` is less than the sum of ARIMA order but `stop_step` is not, then it will return 
a `std::vector<T>` having a combination of the fitted values from `start_step` till `order value` 
and will use new prediction from `order + 1` to `stop_step`.  
- When `start_step` and `stop_step` both are greater than the number of sample in timeseries data, then 
it will return a `std::vector<T>` having newly generated predictions.  

__Return Value__  
It returns a `std::vector<T>` of predicted values.  

### 4. std::vector<T>  forecast(size_t steps = 1)  
__Parameters__  
**_steps_**: A `size_t` parameter which specifies the number of out of sample values to be 
predicted. By default, it will be set as 1. It must be greater than or equal to 1.  

__Purpose__  
It is used to perform out of sample forecasting. It is a wrapper function around predict().  

__Return Value__  
It returns a `std::vector<T>` of out of sample forecasts.  
