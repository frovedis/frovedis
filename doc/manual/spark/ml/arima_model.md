% ARIMAModel 

# NAME

ARIMAModel -  A data  structure used for modeling  the output of ARIMA algorithm
from frovedis server to spark client wrapper.

# SYNOPSIS
import com.nec.frovedis.mllib.tsa.arima.ARIMAModel

## Public Member Functions
`org.apache.spark.sql.DataFrame` predict (Long start, Long end)   
`org.apache.spark.sql.DataFrame` forecast (Long steps)   
`org.apache.spark.sql.DataFrame` fittedvalues ()  
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

### 1. `org.apache.spark.sql.DataFrame` predict (Long start, Long end)   
__Parameters__   
**_start_**: This parameter can be an integer. It specifies the starting index  
from which the values are to be predicted. (Default: 0)  
**_stop_**: This parameter can be an integer. It specifies the index till which  
the values are to be predicted.  (Default: Index of last element in dataset)  

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
    predicted_values.show()

Output  

    In-sample predictions:
    +-----+------------------+
    |index|    predicted_mean|
    +-----+------------------+
    |    4|2.9856736597237106|
    |    5|39.075471086355975|
    +-----+------------------+

**When start and end are negative indices to perform in-sample prediction:**  

For example, 
 
    # perfrom in-sample prediction with start and end as negative indices
    val predicted_values = model.predict(start = -2, end = -1)
    println("In-sample prediction with negative indices:")
    predicted_values.show()

Output  

    In-sample prediction with negative indices:
    +-----+------------------+
    |index|    predicted_mean|
    +-----+------------------+
    |   10|20.085332914693463|
    |   11|15.014777413995532|
    +-----+------------------+


**Note:** Here,  negative  indices  can  only  be  used  to  perform  in-sample 
predictions.  

**When start and end are integers to perfrom out-sample prediction:**  

For example,  

    # perform out-sample prediction with start and end as integers
    val predicted_values = model.predict(start = 12, end = 13)
    println("Out-sample predictions:")
    predicted_values.show()

Output  

    Out-sample predictions:
    +-----+-----------------+
    |index|   predicted_mean|
    +-----+-----------------+
    |   12| 9.59804015203159|
    |   13|4.378840440410923|
    +-----+-----------------+

**Perform in-sample and out-sample  predictions  endog  being a spark dataframe**

For example,  

    # loading an spark dataframe
    import com.nec.frovedis.mllib.tsa.arima.ARIMA
    val endog = (Seq(2,4,6,8,10,12,14,16,18,20,22,24) zip 
                Seq(10.5,15.0,20.0,25.5,35.5,45.0,40.5,35.5,30.5,25.5,20.5,15.0))
               .toDF("Id", "MonthlyTemperature")
    # fitting input dataframe on ARIMA object
    var model = new ARIMA(order = List(2,1,2), targetCol = "MonthlyTemperature", indexCol = "Id")
                                  .fit(endog)
    # displaying the fittedvalues
    val fitted_values = model.fittedvalues()

    # perform in-sample prediction with start and end
    val predicted_values = model.predict(start = 4, end = 5)
    println("In-sample predictions:")
    predicted_values.show()

Output  

    In-sample predictions:
    +-----+------------------+
    |index|    predicted_mean|
    +-----+------------------+
    |   10|2.9856736597237106|
    |   12|39.075471086355975|
    +-----+------------------+

**When start and end are negative indices to perform in-sample prediction:**  

For example, 
 
    # perfrom in-sample prediction with start and end as negative indices
    val predicted_values = model.predict(start = -2, end = -1)
    println("In-sample prediction with negative indices:")
    predicted_values.show()

Output  

    In-sample prediction with negative indices:
    +-----+------------------+
    |index|    predicted_mean|
    +-----+------------------+
    |   22|20.085332914693463|
    |   24|15.014777413995532|
    +-----+------------------+


**Note:** Here,  negative  indices  can  only  be  used  to  perform  in-sample 
predictions.  

**When start and end are integers to perfrom out-sample prediction:**  

For example,  

    # perform out-sample prediction with start and end as integers
    val predicted_values = model.predict(start = 12, end = 13)
    println("Out-sample predictions:")
    predicted_values.show()

Output  

    Out-sample predictions:
    +-----+-----------------+
    |index|   predicted_mean|
    +-----+-----------------+
    |   26| 9.59804015203159|
    |   28|4.378840440410923|
    +-----+-----------------+

__Return Value__  
- It returns a spark dataframe. 


### 2. `org.apache.spark.sql.DataFrame` forecast(Long steps)  
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
    forecasted_values.show()

Output

    Forecast:
    +-----+-----------------+
    |index|   predicted_mean|
    +-----+-----------------+
    |   12| 9.59804015203159|
    |   13|4.378840440410923|
    +-----+-----------------+

**Performing forecasting when endog is spark dataframe.**  

For example,

    # loading an spark dataframe
    import com.nec.frovedis.mllib.tsa.arima.ARIMA
    val endog = (Seq(2,4,6,8,10,12,14,16,18,20,22,24) zip 
                Seq(10.5,15.0,20.0,25.5,35.5,45.0,40.5,35.5,30.5,25.5,20.5,15.0))
               .toDF("Id", "MonthlyTemperature")
    # fitting input dataframe on ARIMA object
    var model = new ARIMA(order = List(2,1,2), targetCol = "MonthlyTemperature", indexCol = "Id")
                                  .fit(endog)

    # perform forecasting
    val forecasted_values = model.forecast(steps = 2)
    println("Forecast:")
    forecasted_values.show()

Output

    Forecast:
    +-----+-----------------+
    |index|   predicted_mean|
    +-----+-----------------+
    |   26| 9.59804015203159|
    |   28|4.378840440410923|
    +-----+-----------------+

__Return Value__  
- It returns a spark dataframe.  

### 3. `org.apache.spark.sql.DataFrame` fittedvalues()  
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
    println("Fitted values")
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
- It returns a spark dataframe.  

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
