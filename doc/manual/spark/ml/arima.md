% ARIMA  

# NAME  

ARIMA - Autoregressive Integrated Moving Average algorithm is a time series 
model that is used to forecast data based on the dataset of past to 
predict/forecast the future.  

# SYNOPSIS  

    import com.nec.frovedis.mllib.tsa.arima.ARIMA
## Public Member Functions  
    ARIMA(List order,  
          Long seasonal, 
          Boolean autoArima,  
          String solver, 
          String targetCol,  
          String indexCol,  
          Int verbose)  
    ARIMAModel fit(RDD[Float] endog)  
    ARIMAModel fit(RDD[Double] endog)  
    ARIMAModel fit(org.apache.spark.sql.DataFrame endog)  

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
**_seasonal_**: A zero or a positive integer parameter that specifies the interval of seasonal 
differencing. In case the data has some seasonality , then it can handle it . (Default: 0)  
**_auto\_arima_**: A boolean parameter that specifies whether to use auto (brute) ARIMA. (Default: false)  
If set to True, it treats the autoregressive and moving average component of the order parameter 
as the highest limit for its iteration and auto fits these components with the best RMSE score.  
**_solver_**: A string object parameter which specifies the solver to be used for linear 
regression. It supports `lapack`, `scalapack`, `lbfgs` and `sag` solvers. (Default: 'lapack')  
When specified, e.g `lbfgs`, then it uses lbfgs solver for linear regression.  
**Note:-** To get the best performance and accuracy from ARIMA, use solver="lapack".  
**_targetCol_** An optional parameter for target/data column name. (Default: null)  
**_indexCol_** An optional parameter for index column name. (Default: null)  
**Note:-**  
  In case targetColumn and indexColumn names are not specified explicitly then following behavior is followed:  
    Case 1: Input dataframe has one column, then the column would be considered as target column with a zero-based index column.  
    Case 2: Input dataframe has two columns, then the first column would be interpreted as index column and second one as target column.  
    Case 3: Input dataframe has more than two columns, then an error is raised specifying to set index and target columns explicitly before calling fit.  
**_verbose_**: An integer parameter specifying the log level to use. Its value is 0 by 
default (for INFO mode and not specified explicitly). But it can be set to 1 (for DEBUG mode) or 
2 (for TRACE mode) for getting training time logs from frovedis server. (Default: 0)  

__Purpose__  
It initializes the ARIMA instance with the given parameters.  

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
    fitted_values.show()

Output

    Fitted values:
    +-----+------------------+
    |index|                  |
    +-----+------------------+
    |    0|               0.0|
    |    1|               0.0|
    |    2|               0.0|
    |    3| 2.656586170196533|
    |    4|  2.98567271232605|
    |    5|39.075469970703125|
    |    6| 46.74116134643555|
    |    7| 33.34402084350586|
    |    8| 32.46163558959961|
    |    9| 24.82413673400879|
    |   10| 20.51268196105957|
    |   11|  15.5408935546875|
    +-----+------------------+

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
    fitted_values.show()

Output

    Fitted values:
    +-----+------------------+
    |index|                  |
    +-----+------------------+
    |    0|               0.0|
    |    1|               0.0|
    |    2|               0.0|
    |    3| 2.656587032568032|
    |    4|2.9856736597237106|
    |    5|39.075471086355975|
    |    6| 46.74116210912707|
    |    7| 33.34402185820186|
    |    8| 32.46163584927344|
    |    9|24.824135902546068|
    |   10|20.512681101065784|
    |   11|  15.5408920934298|
    +-----+------------------+

__Return Value__  
- It returns an ARIMAModel object.  

### 4. ARIMAModel fit(org.apache.spark.sql.DataFrame endog)  
__Parameters__   
**_endog_**: It contains the timeseries data.  
It accepts a org.apache.spark.sql.DataFrame as input data.  
**Note:-**
  - Input dataframe may or may not contain an index column.  
  - In case an index column is provided in dataframe then its frequency must be constant, it may be a positive or negative integer.  
  - In case an index column is not provided or the provided index column is invalid then a default zero based index is used with frequency = 1 and relevant warning message is thrown.  
__Purpose__   
It is used to fit the model parameters on the basis of given parameters and data
provided at frovedis server.   

For example,   

    # loading a spark dataframe
    import com.nec.frovedis.mllib.tsa.arima.ARIMA
    val endog = (Seq(2,4,6,8,10,12,14,16,18,20,22,24) zip 
                Seq(10.5,15.0,20.0,25.5,35.5,45.0,40.5,35.5,30.5,25.5,20.5,15.0))
               .toDF("Id", "MonthlyTemperature")
    # fitting input dataframe on ARIMA object
    var model = new ARIMA(order = List(2,1,2), targetCol = "MonthlyTemperature", indexCol = "Id")
                                  .fit(endog)
    # displaying the fittedvalues
    val fitted_values = model.fittedvalues()
    println("Fitted values:")
    fitted_values.show()

Output

    Fitted values:
    +-----+------------------+
    |index|                  |
    +-----+------------------+
    |    2|               0.0|
    |    4|               0.0|
    |    6|               0.0|
    |    8| 2.656587032568032|
    |   10|2.9856736597237106|
    |   12|39.075471086355975|
    |   14| 46.74116210912707|
    |   16| 33.34402185820186|
    |   18| 32.46163584927344|
    |   20|24.824135902546068|
    |   22|20.512681101065784|
    |   24|  15.5408920934298|
    +-----+------------------+

__Return Value__  
- It returns an ARIMAModel object.  

# SEE ALSO
**[ARIMAModel](./arima_model.md)**
