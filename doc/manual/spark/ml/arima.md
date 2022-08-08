% ARIMA  

# NAME  

ARIMA - Autoregressive Integrated Moving Average algorithm is a time series 
model that is used to forecast data based on the dataset of past to 
predict/forecast the future.  

# SYNOPSIS  

    import com.nec.frovedis.mllib.tsa.arima.ARIMA
## Public Member Functions  
    ARIMA(List order, Any dates, String freq,  
          Boolean validate_specification,  
          Long seasonal, Boolean auto_arima,  
          String solver, Int verbose)  
    ARIMAModel fit(RDD[Float] endog)  
    ARIMAModel fit(RDD[Double] endog)  

# DESCRIPTION  

Frovedis  provides a timeseries  model in order to predict  the  future  values 
based on the past values. Each component in ARIMA functions as a parameter with 
a standard notation. For ARIMA models, a standard notation would be ARIMA  with 
p(AR), d(I), and q(MA) which indicate the type of ARIMA model to be used. A `0`
value can be used as a parameter and would mean that the  particular  component 
should not be used in the  model. This way, the ARIMA model  can be constructed 
to perform  the function of an ARMA model, or even simple AR (1,0,0), I(0,1,0), 
or  MA(0,0,1)  models. **However,  the current  implementaion cannot be used to 
construct a pure MA model.**  

Also, it provides the feature of auto ARIMA which can fit the  best lag for  AR 
and MA. This is a useful feature for the users who do not have knowledge  about 
data analytics.  

Frovedis ARIMA uses OLS (Ordinary Least Squares) estimation.  
**Note:-** Also, rather  than converging  around the mean  after some number of 
predictions, it tends to follow the trend i.e it diverges towards increasing or 
decreasing trend.  

This  module  provides  a  client-server  implementation,  where  the  client 
application is a normal spark program. In this implementation, a  spark  client 
can interact with a frovedis  server  by  sending  the  required spark data for 
training  at frovedis side. Spark data is converted  into  frovedis  compatible 
data internally and the spark ML call is linked  with the  respective  frovedis 
ML call to get the job done at frovedis server.  

## Public Member Function Documentation  

### 1. ARIMA()  

__Parameters__   
**_order_**: A list having 3 elements (p,d,q) that specifies the order of the model 
for the autoregressive(p), differences(d), and moving average(q) components. (Default: (1, 0, 0))  
**Currently, autoregressive order cannot be 0 i.e it cannot be used to create pure MA model.**  
Also, these components (p, d, q) of the model cannot be negative values.  
**_dates_**: An unused parameter. (Default: None)  
**_freq_**: An unused parameter. (Default: "")  
**_validate\_specification_**: An unused parameter. (Default: true)  
**_seasonal_**: A zero or a positive integer parameter that specifies the interval of seasonal 
differencing. In case the data has some seasonality , then it can handle it . (Default: 0)  
**_auto\_arima_**: A boolean parameter that specifies whether to use auto (brute) ARIMA. (Default: false)  
If set to True, it treats the autoregressive and moving average component of the order parameter 
as the highest limit for its iteration and auto fits these components with the best RMSE score.  
**_solver_**: A string object parameter which specifies the solver to be used for linear 
regression. It supports `lapack`, `scalapack`, `lbfgs` and `sag` solvers. (Default: 'lapack')  
When specified, e.g `lbfgs`, then it uses lbfgs solver for linear regression.  
**Note:-** To get the best performance and accuracy from ARIMA, use solver="lapack".  
**_verbose_**: An integer parameter specifying the log level to use. Its value is 0 by 
default (for INFO mode and not specified explicitly). But it can be set to 1 (for DEBUG mode) or 
2 (for TRACE mode) for getting training time logs from frovedis server. (Default: 0)  

__Purpose__  
It initializes the ARIMA instance with the given parameters.  

The parameters: "dates", "freq" and "validate_specification" are kept for future
enhancements. 

Currently, the number of samples in the timeseries data must be greater than sum
of ARIMA order and seasonal parameter.  

    len(endog) >= (order[0] + order[1] + order[2] + seasonal)

__Return Value__    
It simply returns "this" reference.  

### 2. ARIMAModel fit(RDD[Float] endog)  
__Parameters__   
**_endog_**: It contains the timeseries data.  
It accepts an RDD of Float as input data.  

__Purpose__   
It is used to fit the model parameters on the basis of given parameters and data
provided at frovedis server.   

For example,   

    # loading an RDD float data
    import com.nec.frovedis.mllib.tsa.arima.ARIMA
    val arr:Array[Float] = Array(10.5f,15.0f,20.0f,25.5f,35.5f,45.0f,40.5f,35.5f,30.5f,25.5f,20.5f,15.0f)
    val endog:RDD[Float] = sc.parallelize(arr)
    
    # fitting input RDD data on ARIMA object
    val model = new ARIMA(order = List(2,1,2)).fit(endog)
    
    # displaying the fittedvalues
    val fitted_values = model.fittedvalues()
    println("Fitted values:")
    fitted_values.foreach(println)

Output

    Fitted values:
    0.0
    0.0
    0.0
    2.656587032568032
    2.9856736597237106
    39.075471086355975
    46.74116210912707
    33.34402185820186
    32.46163584927344
    24.824135902546068
    20.512681101065784
    15.5408920934298

__Return Value__  
- It returns an ARIMAModel object.  

### 3. ARIMAModel fit(RDD[Double] endog)  
__Parameters__   
**_endog_**: It contains the timeseries data.  
It accepts an RDD of Double as input data.  

__Purpose__   
It is used to fit the model parameters on the basis of given parameters and data
provided at frovedis server.   

For example,   

    # loading an RDD double data
    import com.nec.frovedis.mllib.tsa.arima.ARIMA
    val arr:Array[Double] = Array(10.5,15.0,20.0,25.5,35.5,45.0,40.5,35.5,30.5,25.5,20.5,15.0)
    val endog:RDD[Double] = sc.parallelize(arr)
    
    # fitting input RDD data on ARIMA object
    val model = new ARIMA(order = List(2,1,2)).fit(endog)
    
    # displaying the fittedvalues
    val fitted_values = model.fittedvalues()
    println("Fitted values:")
    fitted_values.foreach(println)

Output

    Fitted values:
    0.0
    0.0
    0.0
    2.656587032568032
    2.9856736597237106
    39.075471086355975
    46.74116210912707
    33.34402185820186
    32.46163584927344
    24.824135902546068
    20.512681101065784
    15.5408920934298

__Return Value__  
- It returns an ARIMAModel object.  

# SEE ALSO
**[ARIMAModel](./arima_model.md)**
