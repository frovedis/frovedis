% Timeseries analysis using ARIMA  

# NAME  

ARIMA - Autoregressive Integrated Moving Average model is a time series model that is used
to forecast data based on the dataset of past to predict/forecast the future.  

# SYNOPSIS  

    class frovedis.mllib.tsa.arima.model.ARIMA(endog, exog=None, order=(1, 0, 0), 
                                               seasonal_order=(0, 0, 0, 0),  
                                               trend=None,  
                                               enforce_stationarity=True,  
                                               enforce_invertibility=True,  
                                               concentrate_scale=False, trend_offset=1,  
                                               dates=None,freq=None, missing='none',  
                                               validate_specification=True,  
                                               seasonal=None, auto_arima=False,  
                                               solver='lapack', verbose=0)  

## Public Member Functions  
  
fit()  
predict(start = None, end = None, dynamic = False, **kwargs)  
forecast(steps = 1, exog = None, alpha = 0.05)  
get_params(deep = True)  
set_params(\*\*params)  
release()  
is_fitted()  

# DESCRIPTION  
Frovedis provides a timeseries model in order to predict the future values based on the past 
values.  
Each component in ARIMA functions as a parameter with a standard notation. For ARIMA models, a 
standard notation would be ARIMA with p(AR), d(I), and q(MA) which indicate the type of ARIMA 
model to be used. A `0` value can be used as a parameter and would mean that the particular component 
should not be used in the model. This way, the ARIMA model can be constructed to perform the 
function of an ARMA model, or even simple AR (1,0,0), I(0,1,0), or MA(0,0,1) models. **However, the 
current implementaion cannot be used to construct a pure MA model.**  

Also, it provides the feature of auto ARIMA which can fit the best lag for AR and MA. This is a 
useful feature for the users who do not have knowledge about data analytics.  

Unlike statsmodel ARIMA, it does not use MLE (Maximum Likelihood Estimation), rather it uses
OLS (Ordinary Least Squares).  
**Note:-** Also, rather than converging around the mean after some number of predictions, it tends to follow
the trend i.e it diverges towards increasing or decreasing trend.  

This module provides a client-server implementation, where the client application is a normal 
python program. The frovedis interface is almost same as statsmodel ARIMA interface, but it does not 
have any dependency with statsmodel. It can be used simply even if the system does not have 
statsmodel installed. Thus in this implementation, a python client can interact with a frovedis 
server by sending the required python data for training at frovedis side. Python data is converted 
into frovedis compatible data internally and the python ML call is linked with the respective 
frovedis ML call to get the job done at frovedis server.  

Python side calls for ARIMA on the frovedis server. Once the training is completed with the 
input data at the frovedis server, it returns an abstract model with a unique model ID to the 
client python program.  

When prediction-like request would be made on the trained model, python program will 
send the same request to the frovedis server. After the request is served at the frovedis 
server, the output would be sent back to the python client.  

## Detailed Description  

### 1. ARIMA()  

__Parameters__   
**_endog_**: It contains the timeseries data.  
Currently, it accepts the below mentioned array-like inputs such as:  
- A numpy array of shape (n_samples,)  
- A list or tuple of shape (n_samples,)  
- A matrix of shape (n_samples, 1)  

Also, it accepts a pandas DataFrame having single column and a datetime index.  
**_exog_**: An unused parameter. (Default: =None)  
**_order_**: A tuple parameter having 3 elements (p,d,q) that specifies the order of the model 
for the autoregressive(p), differences(d), and moving average(q) components. (Default: (1, 0, 0))  
**Currently, autoregressive order cannot be 0 i.e it cannot be used to create pure MA model.**  
Also, these components (p, d, q) of the model cannot be negative values.  
**_seasonal\_order_**: An unused parameter. (Default: (0, 0, 0, 0))  
**_trend_**: An unused parameter. (Default: None)  
**_enforce\_stationarity_**: An unused parameter. (Default: True)  
**_enforce\_invertibility_**: An unused parameter. (Default: True)  
**_concentrate\_scale_**: An unused parameter. (Default: False)  
**_trend\_offset_**: An unused parameter. (Default: 1)  
**_dates_**: An unused parameter. (Default: None)  
**_freq_**: This parameter specifies the frequency of the timeseries. It is an offset string 
or Pandas offset. It is used only when time series data is a pandas dataframe having 
an index. For example, freq = '3D' or freq = to_offset('3D'). (Default: None)  
**_missing_**: An unused parameter. (Default: 'none')  
**_validate\_specification_**: An unused parameter. (Default: True)  
**_seasonal_**: A zero or a positive integer parameter that specifies the interval of seasonal 
differencing. In case the data has some seasonality , then it can handle it . (Default: None)  
**_auto\_arima_**: A boolean parameter that specifies whether to use auto (brute) ARIMA. (Default: False)  
If set to True, it treats the autoregressive and moving average component of the order parameter 
as the highest limit for its iteration and auto fits these components with the best RMSE score.  
**_solver_**: A string object parameter which specifies the solver to be used for linear 
regression. It supports `lapack`, `scalapack`, `lbfgs` and `sag` solvers. (Default: 'lapack')  
When specified, e.g `lbfgs`, then it uses lbfgs solver for linear regression.  
**Note:-** To get the best performance and accuracy from ARIMA, use solver='lapack'.  
**_verbose_**: An integer parameter specifying the log level to use. Its value is 0 by 
default (for INFO mode and not specified explicitly). But it can be set to 1 (for DEBUG mode) or 
2 (for TRACE mode) for getting training time logs from frovedis server. (Default: 0)  

__Attributes__  
**_fittedvalues_**: It is a python ndarray (containing float or double (float64) typed values 
depending on data-type of input array) of shape (n_samples,) or pandas Series containing the 
predicted values of the model after training is completed.  

__Purpose__  
It initializes the ARIMA instance with the given parameters.  

The parameters: "exog", "seasonal_order", "trend", "enforce_stationarity", "enforce_invertibility", 
"concentrate_scale", "trend_offset", "dates" and "missing" are simply kept in to to make the 
interface uniform to the statsmodel ARIMA module. They are not used anywhere within the 
frovedis implementation.  

Currently, the number of samples in the timeseries data must be greater than sum of ARIMA
order and seasonal parameter.  

    len(endog) >= (order[0] + order[1] + order[2] + seasonal)

__Return Value__    
It simply returns "self" reference.  

### 2. fit()  

__Purpose__   
It is used to fit the model parameters on the basis of given parameters and data provided
at frovedis server.   

For example,   

    # loading an array-like data
    import numpy as np
    data = np.array([266, 145.9, 183.1, 119.3, 180.3, 168.5,
                     231.8, 224.5, 192.8, 122.9, 336.5, 185.9,
                     194.3, 149.5, 210.1, 273.3, 191.4, 287,
                     226, 303.6, 289.9, 421.6, 264.5, 342.3,
                     339.7, 440.4, 315.9, 439.3, 401.3, 437.4,
                     575.5, 407.6, 682, 475.3, 581.3, 646.9])
    
    # fitting input array-like data on ARIMA object
    from frovedis.mllib.tsa.arima.model import ARIMA
    arima = ARIMA(data, order=(2,1,2)).fit()   
    
    # displaying the fittedvalues
    print(arima.fittedvalues)

Output  

    [  0.           0.           0.          34.80616713  82.04476428
     199.9205314  197.46227245 198.75614907 229.51624583 263.0131258
     270.18526491 192.05449775 272.3933017  284.84889246 266.61188466
     226.95908704 211.39748037 271.90632761 256.99495549 290.21370532
     281.66600813 303.1955228  304.63779347 393.97882909 375.17286126
     364.56323191 359.48470979 426.81421461 402.55377056 427.14878515
     442.66867406 432.34417321 540.18491169 486.09468617 596.96746024
     589.29064598]

Here, fittedvalues will be displayed after training is completed on array-like timeseries data.  

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like input can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,  
     
    # loading an array-like data
    import numpy as np
    data = np.array([266, 145.9, 183.1, 119.3, 180.3, 168.5,
                     231.8, 224.5, 192.8, 122.9, 336.5, 185.9,
                     194.3, 149.5, 210.1, 273.3, 191.4, 287,
                     226, 303.6, 289.9, 421.6, 264.5, 342.3,
                     339.7, 440.4, 315.9, 439.3, 401.3, 437.4,
                     575.5, 407.6, 682, 475.3, 581.3, 646.9])
    
    # Since "data" is numpy array, we have created FrovedisDvector. 
    from frovedis.matrix.dvector import FrovedisDvector
    data = FrovedisDvector(data)
    
    # ARIMA with pre-constructed frovedis-like input
    from frovedis.mllib.tsa.arima.model import ARIMA
    arima = ARIMA(data, order=(2,1,2)).fit()

    # displaying the fittedvalues
    print(arima.fittedvalues)

Output

    [  0.           0.           0.          34.80616713  82.04476428
     199.9205314  197.46227245 198.75614907 229.51624583 263.0131258
     270.18526491 192.05449775 272.3933017  284.84889246 266.61188466
     226.95908704 211.39748037 271.90632761 256.99495549 290.21370532
     281.66600813 303.1955228  304.63779347 393.97882909 375.17286126
     364.56323191 359.48470979 426.81421461 402.55377056 427.14878515
     442.66867406 432.34417321 540.18491169 486.09468617 596.96746024
     589.29064598]

Here, fittedvalues will be displayed after training is completed on frovedis-like timeseries data.  

When pandas dataframe having single column and a datetime index is provided, then training is 
done based on the given index type.  

If it is a numeric index with values 0, 1 , ..., N-1 (always incrementing indices), where N 
is n_samples or if it is (coerceable to) a DatetimeIndex or PeriodIndex with an associated 
frequency, then it is called a "Supported" index. Otherwise, it is called an "Unsupported" index.  

**Currently, support for DatetimeIndex or PeriodIndex instance is provided in frovedis.**  

NOTE: A warning will be given when unsupported indices are used for training.  

**When a supported index is provided:**  

For example,  

    # loading a pandas dataframe having datetime index
    # such indices should be monotonically increasing 
    # and have an associated frequency
    # also, such index is considered as a supported index
    import pandas as pd
    index = ["01-01-1981", "01-04-1981", "01-07-1981", "01-10-1981", "01-13-1981", "01-16-1981", 
             "01-19-1981", "01-22-1981", "01-25-1981", "01-28-1981", "01-31-1981", "02-03-1981", 
             "02-06-1981", "02-09-1981", "02-12-1981", "02-15-1981", "02-18-1981", "02-21-1981"] 

    index = pd.DatetimeIndex(data = index, name="Date", freq=None)
    data = np.array([266, 145.9, 183.1, 119.3, 180.3, 168.5, 231.8, 
                     224.5, 192.8, 122.9, 336.5, 185.9, 194.3, 149.5, 
                     210.1, 273.3, 191.4, 287])
    df = pd.DataFrame({'Temp': data}, index = index)

    # fitting input dataframe on ARIMA object
    from frovedis.mllib.tsa.arima.model import ARIMA
    arima = ARIMA(df, order=(2,1,2)).fit()

    # displaying the fittedvalues
    print(arima.fittedvalues)

Output  

    UserWarning: A date index has been provided, but it has no associated frequency information 
    and so will be ignored when e.g. forecasting.

    Date
    1981-01-01      0.000000
    1981-01-04      0.000000
    1981-01-07      0.000000
    1981-01-10      7.183812
    1981-01-13     50.686614
    1981-01-16    188.205611
    1981-01-19    171.793531
    1981-01-22    182.706200
    1981-01-25    200.586396
    1981-01-27    227.986895
    1981-01-31    233.641814
    1981-02-03    213.975698
    1981-02-06    213.622244
    1981-02-09    254.024920
    1981-02-12    242.193347
    1981-02-15    216.585066
    1981-02-18    198.927653
    1981-02-21    224.250625
    dtype: float64

Here, fittedvalues will be displayed after training is completed on dataframe having supported index.  

**Note:** For fittedvalues attribute, when endog is having an index, then after training is completed, 
    
    fittedvalues.index = endog.index

This will always be true even when endog is having supported or unsupported indices.  

Also, in above example frequency infromation was inferred based on given timeseries data.  

So, we can use **freq** parameter in frovedis ARIMA in order to set the frequency information of the timeseries 
data as well.  

For example,  

    # loading a pandas dataframe having datetime index
    import pandas as pd
    index = ["01-01-1981", "01-04-1981", "01-07-1981", "01-10-1981", "01-13-1981", "01-16-1981",
             "01-19-1981", "01-22-1981", "01-25-1981", "01-28-1981", "01-31-1981", "02-03-1981",
             "02-06-1981", "02-09-1981", "02-12-1981", "02-15-1981", "02-18-1981", "02-21-1981"]

    index = pd.DatetimeIndex(data = index, name="Date", freq=None)
    data = np.array([266, 145.9, 183.1, 119.3, 180.3, 168.5, 231.8,
                     224.5, 192.8, 122.9, 336.5, 185.9, 194.3, 149.5,
                     210.1, 273.3, 191.4, 287])
    df = pd.DataFrame({'Temp': data}, index = index)

    # fitting input dataframe on ARIMA object and using freq parameter
    from frovedis.mllib.tsa.arima.model import ARIMA
    arima = ARIMA(df, order=(2,1,2), freq = '3D').fit()

    # displaying the fittedvalues
    print(arima.fittedvalues)

Output  

    Date
    1981-01-01      0.000000
    1981-01-04      0.000000
    1981-01-07      0.000000
    1981-01-10      7.183812
    1981-01-13     50.686614
    1981-01-16    188.205611
    1981-01-19    171.793531
    1981-01-22    182.706200
    1981-01-25    200.586396
    1981-01-27    227.986895
    1981-01-31    233.641814
    1981-02-03    213.975698
    1981-02-06    213.622244
    1981-02-09    254.024920
    1981-02-12    242.193347
    1981-02-15    216.585066
    1981-02-18    198.927653
    1981-02-21    224.250625
    dtype: float64

**Note:** Here, the frequency information provided for freq parameter must be same as the frequency 
inferred for the timeseries data. Otherwise, it will raise an exception.  

**When an unsupported index is provided:**

For example,

    # loading a pandas dataframe having datetime index
    # and having no associated frequency
    # such index is considered as an unsupported index
    import pandas as pd
    index = ["01-01-1981", "01-05-1981", "01-07-1981", "01-10-1981", "01-12-1981", "01-16-1981",
             "01-19-1981", "01-21-1981", "01-25-1981", "01-28-1981", "01-31-1981", "02-03-1981",
             "02-06-1981", "02-10-1981", "02-12-1981", "02-16-1981", "02-18-1981", "02-22-1981"]

    index = pd.DatetimeIndex(data = index, name="Date", freq=None)
    data = np.array([266, 145.9, 183.1, 119.3, 180.3, 168.5, 231.8,
                     224.5, 192.8, 122.9, 336.5, 185.9, 194.3, 149.5,
                     210.1, 273.3, 191.4, 287])
    df = pd.DataFrame({'Temp': data}, index = index)

    # fitting input dataframe on ARIMA object
    from frovedis.mllib.tsa.arima.model import ARIMA
    arima = ARIMA(df, order=(2,1,2)).fit()

    # displaying the fittedvalues
    print(arima.fittedvalues)

Output  

    UserWarning: A date index has been provided, but it has no associated frequency information and so 
    will be ignored when e.g. forecasting.

    Date
    1981-01-01      0.000000
    1981-01-05      0.000000
    1981-01-07      0.000000
    1981-01-10      7.183812
    1981-01-12     50.686614
    1981-01-16    188.205611
    1981-01-19    171.793531
    1981-01-21    182.706200
    1981-01-25    200.586396
    1981-01-28    227.986895
    1981-01-31    233.641814
    1981-02-03    213.975698
    1981-02-06    213.622244
    1981-02-10    254.024920
    1981-02-12    242.193347
    1981-02-16    216.585066
    1981-02-18    198.927653
    1981-02-22    224.250625
    dtype: float64

Here, indices in the `fittedvalues` are same as indices present in the `endog` timeseries data.  

__Return Value__  
It simply returns "self" reference.  

### 3. predict(start = None, end = None, dynamic = False, **kwargs)  
__Parameters__   
**_start_**: This parameter can be an integer, string or datetime instance. It 
specifies the starting index from which the values are to be predicted.  
**_stop_**: This parameter can be an integer, string or datetime instance. It 
specifies the index till which the values are to be predicted.  
**_dynamic_**: An unused parameter. (Default: False)  
**_\*\*kwargs_**: An unused parameter.  

__Purpose__    
It is used to perform in-sample prediction and out-of-sample prediction at frovedis server.  

**During prediction, end index must not be less than start index.**  

Below mentioned examples show frovedis ARIMA to be used to perform in-sample and out-sample 
predictions.  

**When endog is array-like input, then start and end must only be integers to perform in-sample and 
out-sample predictions.**  

For example,  

    # loading an array-like data
    import numpy as np
    data = np.array([266, 145.9, 183.1, 119.3, 180.3, 168.5,
                     231.8, 224.5, 192.8, 122.9, 336.5, 185.9,
                     194.3, 149.5, 210.1, 273.3, 191.4, 287,
                     226, 303.6, 289.9, 421.6, 264.5, 342.3,
                     339.7, 440.4, 315.9, 439.3, 401.3, 437.4,
                     575.5, 407.6, 682, 475.3, 581.3, 646.9])

    # fitting input array-like data on ARIMA object
    from frovedis.mllib.tsa.arima.model import ARIMA
    arima = ARIMA(data, order=(2,1,2)).fit()

    # perform in-sample prediction with start and end as integers
    print('In-sample predictions: ', arima.predict(start=11, end=12))

Output  

    In-sample predictions:  [169.65455872 290.43805859]

**When start and end are negative indices to perform in-sample prediction:**  

For example, 
 
    # perfrom in-sample prediction with start and end as negative indices
    print('In-sample prediction with negative indices: ', arima.predict(start=-2, end=-1))

Output  

    In-sample prediction with negative indices:  [627.92270498 578.69942377]

**Note:** Here, negative indices can only be used to perform in-sample predictions.  

**When start and end are integers to perfrom out-sample prediction:**  

For example,  

    # perform out-sample prediction with start and end as integers
    print('Out-sample predictions: ', arima.predict(start=40, end=41))

Output  

    Out-sample predictions:  [672.66044638 683.2480911 ]

When endog is a dataframe having a supported index (monotonically increasing and associated 
frequency), then both `start` and `end` can be integer, dates as string or datetime instance to 
perform in-sample and out-sample predictions.  

For example,  

    # loading a pandas dataframe having a supported index
    import pandas as pd
    index = ["01-01-1981", "01-04-1981", "01-07-1981", "01-10-1981", "01-13-1981", "01-16-1981",  
             "01-19-1981", "01-22-1981", "01-25-1981", "01-28-1981", "01-31-1981", "02-03-1981",  
             "02-06-1981", "02-09-1981", "02-12-1981", "02-15-1981", "02-18-1981", "02-21-1981"]
    index = pd.DatetimeIndex(data = index, name="Date", freq=None)
    data = np.array([266, 145.9, 183.1, 119.3, 180.3, 168.5, 231.8,
                     224.5, 192.8, 122.9, 336.5, 185.9, 194.3, 149.5,
                     210.1, 273.3, 191.4, 287])
    df = pd.DataFrame({'Temp': data}, index = index)

    # fitting input dataframe on ARIMA object
    from frovedis.mllib.tsa.arima.model import ARIMA
    arima = ARIMA(df, order=(2,1,2)).fit()

    # perform in-sample prediction with start and end as integers
    print('In-sample predictions: ', arima.predict(start=11, end=12))

Output  

    UserWarning: No frequency information was provided, so inferred frequency 3D will be used.

    In-sample predictions:  Date
    1981-02-03    192.400565
    1981-02-06    255.586914
    Freq: 3D, dtype: float64

For example,  

    # perform out-sample prediction with start and end as integers
    print('Out-sample predictions: ', arima.predict(start=22, end=24))

Output  

    UserWarning: No frequency information was provided, so inferred frequency 3D will be used.
    
    Out-sample predictions:  1981-03-08    294.413814
    1981-03-11    297.292898
    1981-03-14    303.964832
    Freq: 3D, dtype: float64

**When start and end are both dates as string to perform predictions:**  

For example,  

    # perform in-sample prediction with start and end both are dates as strings
    print('In-sample predictions: ', arima.predict(start='01-28-1981', end='02-03-1981'))

Output  

    UserWarning: No frequency information was provided, so inferred frequency 3D will be used.
    
    In-sample predictions:  Date
    1981-01-28    243.389069
    1981-01-31    232.545345
    1981-02-03    235.522806
    Freq: 3D, dtype: float64

For example,  

    # perform out-sample prediction with start and end both are dates as strings
    print('Out-sample predictions: ', arima.predict(start='02-24-1981', end='02-24-1981'))

Output  

    UserWarning: No frequency information was provided, so inferred frequency 3D will be used.
    
    Out-sample predictions:  1981-02-24    259.295505
    Freq: 3D, dtype: float64

**When start and end are both datetime instance to perform predictions:**  

For example,  
 
    # perform in-sample prediction with start and end both are dates as datetime instance
    from datetime import datetime
    print('In-sample predictions: ', arima.predict(start=datetime(1981,1,28), end=datetime(1981,2,3)))

Output  

    UserWarning: No frequency information was provided, so inferred frequency 3D will be used.
    
    In-sample predictions:  Date
    1981-01-28    243.389069
    1981-01-31    232.545345
    1981-02-03    235.522806
    Freq: 3D, dtype: float64

For example,  

    # perform out-sample prediction with start and end both are dates as datetime instance
    from datetime import datetime
    print('Out-sample predictions: ', arima.predict(start=datetime(1981,2,24), end=datetime(1981,2,24)))    

Output  

    UserWarning: No frequency information was provided, so inferred frequency 3D will be used.
    
    Out-sample predictions:  1981-02-24    259.295505
    Freq: 3D, dtype: float64

When endog is a dataframe having an unsupported index, then only integer is used to perform in-sample and 
out-sample predictions. For using dates as string and datetime instance to perform predictions, then only 
in-sample prediction can be done. No out-sample prediction can be done when using dates as string or datetime 
instance.  

For example,

    # loading a pandas dataframe having an unsupported index
    import pandas as pd
    index = ["01-01-1981", "01-05-1981", "01-07-1981", "01-10-1981", "01-12-1981", "01-16-1981",
             "01-19-1981", "01-21-1981", "01-25-1981", "01-28-1981", "01-31-1981", "02-03-1981",  
             "02-06-1981", "02-10-1981", "02-12-1981", "02-16-1981", "02-18-1981", "02-22-1981"]
    index = pd.DatetimeIndex(data = index, name="Date", freq=None)
    data = np.array([266, 145.9, 183.1, 119.3, 180.3, 168.5, 231.8,
                     224.5, 192.8, 122.9, 336.5, 185.9, 194.3, 149.5,
                     210.1, 273.3, 191.4, 287])
    df = pd.DataFrame({'Temp': data}, index = index)

    # fitting input dataframe on ARIMA object
    from frovedis.mllib.tsa.arima.model import ARIMA
    arima = ARIMA(df, order=(2,1,2)).fit()

    # perform in-sample prediction with start and end as integers
    print('In-sample predictions: ', arima.predict(start=11, end=12))

Output  

    UserWarning: A date index has been provided, but it has no associated frequency information and 
    so will be ignored when e.g. forecasting.

    In-sample predictions:  Date
    1981-02-03    192.400565
    1981-02-06    255.586914
    dtype: float64

For example,  

    # perform out-sample prediction with start and end as integers
    print('Out-sample predictions: ', arima.predict(start=22, end=24))

Output  

    UserWarning: A date index has been provided, but it has no associated frequency information and 
    so will be ignored when e.g. forecasting.    
    
    Out-sample predictions:  22    294.413814
    23    297.292898
    24    303.964832
    dtype: float64

**When start and end are both dates as string to perform predictions:**  

For example,  

    # perform in-sample prediction with start and end both are dates as strings
    print('In-sample predictions: ', arima.predict(start='01-07-1981', end='01-16-1981'))

Output  

    UserWarning: A date index has been provided, but it has no associated frequency information and 
    so will be ignored when e.g. forecasting.

    In-sample predictions:  Date
    1981-01-07      0.000000
    1981-01-10      7.183812
    1981-01-12     50.686614
    1981-01-16    188.205611
    dtype: float64

**When start and end are both datetime instance to perform predictions:**  

For example,  

    # perform in-sample prediction with start and end both are dates as datetime instance
    from datetime import datetime
    print('In-sample predictions: ', arima.predict(start=datetime(1981,1,28), end=datetime(1981,2,3)))

Output  

    UserWarning: A date index has been provided, but it has no associated frequency information and 
    so will be ignored when e.g. forecasting.

    In-sample predictions:  Date
    1981-01-28    243.389069
    1981-01-31    232.545345
    1981-02-03    235.522806
    dtype: float64

__Return Value__  
**When endog is array-like:**  
- It returns a numpy array of shape (n_predictions,)  

**When endog is a dataframe:**  
- It returns a pandas Series having an index and data column. Number of samples in the pandas Series 
are equal to the number of number of predictions.  


### 4. forecast(steps = 1, exog = None, alpha = 0.05)  
__Parameters__  
**_steps_**: This parameter can be a positive integer, string or datetime 
instance. It specifies the number of out of sample values to be predicted. (Default: 1)  
**NOTE:** When endog is array-like, steps must be a positive integer and is greater than or equal to 1. If 
endog is a dataframe having date indices with associated frequency (known as supported index), 
then steps can also be dates as string or datetime instance during out-of-sample forecasting.  

When endog has unsupported indices, only integer must be used for out-of-sample forecasting. Also, 
a warning will be given when such indices are used for forecasting. For other types provided, it 
will raise an exception.  
**_exog_**: An unused parameter. (Default: None)  
**_alpha_**: An unused parameter. (Default: 0.05)  

__Purpose__  
It is used to perform out of sample forecasting.  

Below mentioned examples show frovedis ARIMA to be used to perform forecasting.  

**When endog is array-like input, then steps must only be an integer to perform forecasting.**  

For example,

    # loading an array-like data
    import numpy as np
    data = np.array([266, 145.9, 183.1, 119.3, 180.3, 168.5,
                     231.8, 224.5, 192.8, 122.9, 336.5, 185.9,
                     194.3, 149.5, 210.1, 273.3, 191.4, 287,
                     226, 303.6, 289.9, 421.6, 264.5, 342.3,
                     339.7, 440.4, 315.9, 439.3, 401.3, 437.4,
                     575.5, 407.6, 682, 475.3, 581.3, 646.9])

    # fitting input array-like data on ARIMA object
    from frovedis.mllib.tsa.arima.model import ARIMA
    arima = ARIMA(data, order=(2,1,2)).fit()

    # perform forecasting with steps as an integer
    print('forecast(): ', arima.forecast(steps=2))

Output

    forecast():  [578.26315696 654.88723312]

When endog is a dataframe having a supported index (monotonically increasing and associated
frequency), then `steps` can be an integer, dates as string or datetime instance to
perform forecasting.  

For example,

    # loading a pandas dataframe having a supported index
    import pandas as pd
    index = ["01-01-1981", "01-04-1981", "01-07-1981", "01-10-1981", "01-13-1981", "01-16-1981",
             "01-19-1981", "01-22-1981", "01-25-1981", "01-28-1981", "01-31-1981", "02-03-1981",
             "02-06-1981", "02-09-1981", "02-12-1981", "02-15-1981", "02-18-1981", "02-21-1981"]

    index = pd.DatetimeIndex(data = index, name="Date", freq=None)
    data = np.array([266, 145.9, 183.1, 119.3, 180.3, 168.5, 231.8,
                     224.5, 192.8, 122.9, 336.5, 185.9, 194.3, 149.5,
                     210.1, 273.3, 191.4, 287])
    df = pd.DataFrame({'Temp': data}, index = index)

    # fitting input dataframe on ARIMA object
    from frovedis.mllib.tsa.arima.model import ARIMA
    arima = ARIMA(df, order=(2,1,2)).fit()

    # perform forecasting with steps as an integer
    print('forecast(): ', arima.forecast(steps=2))

Output  

    UserWarning: No frequency information was provided, so inferred frequency 3D will be used.

    forecast():  1981-02-24    259.295505
    1981-02-27    280.647172
    Freq: 3D, dtype: float64

**When steps is a date as a string:**  

For example,

    # perform forecasting where steps is a date as a string
    print('forecast(): ', arima.forecast(steps='02-24-1981'))

Output

    forecast():  1981-02-24    259.295505  
    Freq: 3D, dtype: float64

**When steps is a date as a datetime instance:**  

For example,

    # perform forecasting where steps is a dates as a datetime instance
    from datetime import datetime
    print('forecast(): ', arima.forecast(steps=datetime(1981,2,24)))

Output

    forecast():  1981-02-24    259.295505
    Freq: 3D, dtype: float64

When endog is a dataframe having an unsupported index, then only integer is used to perform forecasting. No 
forecasting can be done when using dates as string or datetime instance.  

For example,

    # loading a pandas dataframe having a supported index
    import pandas as pd
    index = ["01-01-1981", "01-05-1981", "01-07-1981", "01-10-1981", "01-12-1981", "01-16-1981",
             "01-19-1981", "01-21-1981", "01-25-1981", "01-28-1981", "01-31-1981", "02-03-1981",
             "02-06-1981", "02-10-1981", "02-12-1981", "02-16-1981", "02-18-1981", "02-22-1981"]

    index = pd.DatetimeIndex(data = index, name="Date", freq=None)
    data = np.array([266, 145.9, 183.1, 119.3, 180.3, 168.5, 231.8,
                     224.5, 192.8, 122.9, 336.5, 185.9, 194.3, 149.5,
                     210.1, 273.3, 191.4, 287])
    df = pd.DataFrame({'Temp': data}, index = index)

    # fitting input dataframe on ARIMA object
    from frovedis.mllib.tsa.arima.model import ARIMA
    arima = ARIMA(df, order=(2,1,2)).fit()

    # perform forecasting with steps as an integer
    print('forecast(): ', arima.forecast(steps=2))

Output  

   UserWarning: A date index has been provided, but it has no associated frequency information and so 
   will be ignored when e.g. forecasting.
   
   forecast():  18    259.295505  
   19    280.647172  
   dtype: float64  

__Retur Value__  
**When endog is array-like:**  
- It returns a numpy array of shape (steps,)  

**When endog is a dataframe:**  
- It returns a pandas Series having an index and data column.  


### 5. get_params(deep = True)  
__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If True, 
it will return the parameters for an estimator and contained subobjects that are estimators. (Default: True)  

__Purpose__    
This method belongs to the BaseEstimator class inherited by ARIMA. It is used to get parameters 
and their values of ARIMA class.  

For example, 
 
    print(arima.get_params())

Output  

   {'auto_arima': False, 'concentrate_scale': False, 'dates': None, 
    'endog': array([266. , 145.9, 183.1, 119.3, 180.3, 168.5, 231.8, 224.5, 192.8,
                   122.9, 336.5, 185.9, 194.3, 149.5, 210.1, 273.3, 191.4, 287. ,
                   226. , 303.6, 289.9, 421.6, 264.5, 342.3, 339.7, 440.4, 315.9,
                   439.3, 401.3, 437.4, 575.5, 407.6, 682. , 475.3, 581.3, 646.9]), 
    'enforce_invertibility': True, 'enforce_stationarity': True, 'exog': None, 'freq': None, 
    'missing': 'none', 'order': (2, 1, 2), 'seasonal': 0, 'seasonal_order': (0, 0, 0, 0), 
    'solver': 'lapack', 'trend': None, 'trend_offset': 1, 'validate_specification': True, 
    'verbose': 0}
    
__Return Value__  
A dictionary of parameter names mapped to their values.  

### 6. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed to this function as dictionary. This dictionary 
contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by ARIMA, used to set 
parameter values.  

For example,   

    print("get parameters before setting:")  
    print(arima.get_params())  
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user  
    arima.set_params(order=(1,1,1), solver='lbfgs')  
    print("get parameters after setting:")  
    print(arima.get_params())  

Output  
     
    get parameters before setting:
    {'auto_arima': False, 'concentrate_scale': False, 'dates': None,
     'endog': array([266. , 145.9, 183.1, 119.3, 180.3, 168.5, 231.8, 224.5, 192.8,
                     122.9, 336.5, 185.9, 194.3, 149.5, 210.1, 273.3, 191.4, 287. ,
                     226. , 303.6, 289.9, 421.6, 264.5, 342.3, 339.7, 440.4, 315.9,
                     439.3, 401.3, 437.4, 575.5, 407.6, 682. , 475.3, 581.3, 646.9]),
     'enforce_invertibility': True, 'enforce_stationarity': True, 'exog': None, 'freq': None,
     'missing': 'none', 'order': (2, 1, 2), 'seasonal': 0, 'seasonal_order': (0, 0, 0, 0),
     'solver': 'lapack', 'trend': None, 'trend_offset': 1, 'validate_specification': True,
     'verbose': 0}

    get parameters after setting:
    {'auto_arima': False, 'concentrate_scale': False, 'dates': None,
     'endog': array([266. , 145.9, 183.1, 119.3, 180.3, 168.5, 231.8, 224.5, 192.8,
                     122.9, 336.5, 185.9, 194.3, 149.5, 210.1, 273.3, 191.4, 287. ,
                     226. , 303.6, 289.9, 421.6, 264.5, 342.3, 339.7, 440.4, 315.9,
                     439.3, 401.3, 437.4, 575.5, 407.6, 682. , 475.3, 581.3, 646.9]),
     'enforce_invertibility': True, 'enforce_stationarity': True, 'exog': None, 'freq': None,
     'missing': 'none', 'order': (1, 1, 1), 'seasonal': 0, 'seasonal_order': (0, 0, 0, 0),
     'solver': 'lbfgs', 'trend': None, 'trend_offset': 1, 'validate_specification': True,
     'verbose': 0}

__Return Value__  
It simply returns "self" reference.  

### 7. release()  

__Purpose__    
It can be used to release the in-memory model at frovedis server.  

For example,
 
    arima.release()

This will reset the after-fit populated attributes to None, along with releasing server 
side memory.  

__Return Value__  
It returns nothing.  

### 8. is_fitted()  

__Purpose__    
It can be used to confirm if the model is already fitted or not. In case, predict() is used 
before training the model, then it can prompt the user to train the ARIMA model first. 

__Return Value__  
It returns 'True', if the model is already fitted otherwise, it returns 'False'.  

# SEE ALSO  
- **[Introduction to FrovedisDvector](../matrix/dvector.md)**  
- **[DataFrame - Introduction](../dataframe/df_intro.md)**  
