% ARIMAModel 

# NAME

ARIMAModel -  A data  structure used for modeling  the output of ARIMA algorithm
from frovedis server to spark client wrapper.

# SYNOPSIS
import com.nec.frovedis.mllib.tsa.arima.ARIMAModel

## Public Member Functions
`Array[Double]` predict (Long start, Long end, Boolean dynamic)   
`Array[Double]` forecast (Long steps)  
`Array[Double]` fittedvalues ()  
Unit release()    

# DESCRIPTION

ARIMAModel  models  the  output  of  the  frovedis  ARIMA  timeseries prediction
algorithm, the fit interface of ARIMA  aims to  optimize  an  initial  model and
outputs the same after optimization. 
 
Note that the actual model with given parameters like order  etc. is  created at
frovedis server only. Spark ARIMAModel contains a unique ID associated  with the
frovedis server side model. It simply works  like  a  pointer to  the  in-memory
model at frovedis server. 

Any operations, like  prediction  etc. on a  ARIMAModel  makes a request  to the
frovedis server along with the unique model ID and the actual job is  served  by
the frovedis server. For functions which returns some output, the result is sent 
back from frovedis server to the spark client.

## Public Member Function Documentation

### 1. `Array[Double]` predict (Long start, Long end, Boolean dynamic)   
__Parameters__   
**_start_**: This parameter can be an integer. It 
specifies the starting index from which the values are to be predicted. (Default: 0)  
**_stop_**: This parameter can be an integer. It 
specifies the index till which the values are to be predicted.  (Default: Index of last element in dataset)  
**_dynamic_**: An unused parameter. (Default: false)  

__Purpose__    
It  is  used  to  perform in-sample prediction and out-of-sample prediction  at 
frovedis server.

**During prediction, end index must not be less than start index.**  

Below mentioned examples show frovedis ARIMAModel being used to perform in-sample
and out-sample predictions.  

**Perform in-sample and out-sample  predictions  endog  being an RDD of Float or
Double input.**  

For example,  

    # loading an RDD[Double] data
    import com.nec.frovedis.mllib.tsa.arima.ARIMA
    val endog:RDD[Double] = sc.parallelize(Array(10.5,15.0,20.0,25.5,35.5,45.0,
                                40.5,35.5,30.5,25.5,20.5,15.0))

    # fitting RDD data on ARIMA object
    val model = new ARIMA(order = List(2,1,2)).fit(endog)

    # perform in-sample prediction with start and end
    val predicted_values = model.predict(start = 4, end = 5)
    println("In-sample predictions:")
    predicted_values.foreach(println)

Output  

    In-sample predictions:
    2.9856736597237106
    39.075471086355975

**When start and end are negative indices to perform in-sample prediction:**  

For example, 
 
    # perfrom in-sample prediction with start and end as negative indices
    val predicted_values = model.predict(start = -2, end = -1)
    println("In-sample prediction with negative indices:")
    predicted_values.foreach(println)

Output  

    In-sample prediction with negative indices:
    20.085332914693463
    15.014777413995532


**Note:** Here,  negative  indices  can  only  be  used  to  perform  in-sample 
predictions.  

**When start and end are integers to perfrom out-sample prediction:**  

For example,  

    # perform out-sample prediction with start and end as integers
    val predicted_values = model.predict(start = 12, end = 13)
    println("Out-sample predictions:")
    predicted_values.foreach(println)

Output  

    Out-sample predictions:
    9.59804015203159
    4.378840440410923

__Return Value__  
- It returns an array of doubles. 


### 2. `Array[Double]` forecast(Long steps)  
__Parameters__  
**_steps_**: This parameter is a positive integer. It specifies the number of out of sample values to be predicted. (Default: 1)  
**NOTE:** steps must be atleast 1. 

__Purpose__  
It is used to perform out of sample forecasting.  

Below mentioned examples show frovedis ARIMAModel to be used to perform forecasting.  

**Performing forecasting when endog is RDD input.**  

For example,

    # loading an RDD[Double] data
    import com.nec.frovedis.mllib.tsa.arima.ARIMA
    val endog:RDD[Double] = sc.parallelize(Array(10.5,15.0,20.0,25.5,35.5,45.0,
                                40.5,35.5,30.5,25.5,20.5,15.0))

    # fitting RDD data on ARIMA object
    val model = new ARIMA(order = List(2,1,2)).fit(endog)

    # perform forecasting
    val forecasted_values = model.forecast(steps = 2)
    println("Forecast:")
    forecasted_values.foreach(println)

Output

    Forecast:
    9.59804015203159
    4.378840440410923

__Return Value__  
- It returns an array of doubles.  

### 3. `Array[Double]` fittedvalues()  
__Purpose__  
It is used to get the generated fitted values after fitting.  

Below mentioned example shows use of fittedvalues method.  


For example,

    # loading an RDD[Double] data
    import com.nec.frovedis.mllib.tsa.arima.ARIMA
    val endog:RDD[Double] = sc.parallelize(Array(10.5,15.0,20.0,25.5,35.5,45.0,
                                40.5,35.5,30.5,25.5,20.5,15.0))

    # fitting RDD data on ARIMA object
    val model = new ARIMA(order = List(2,1,2)).fit(endog)

    # get fitted values
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
- It returns an array of doubles.  

### 4. Unit release()  
This function can be used to release the existing in-memory model in frovedis 
server.

Below mentioned example shows use of release method.  


For example,

    # loading an RDD[Double] data
    import com.nec.frovedis.mllib.tsa.arima.ARIMA
    val endog:RDD[Double] = sc.parallelize(Array(10.5,15.0,20.0,25.5,35.5,45.0,
                                40.5,35.5,30.5,25.5,20.5,15.0))

    # fitting RDD data on ARIMA object
    val model = new ARIMA(order = List(2,1,2)).fit(endog)

    # get fitted values
    val fitted_values = model.fittedvalues()

    # release model
    model.release()

# SEE ALSO
**[ARIMA](./arima.md)**
