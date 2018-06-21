% LogisticRegressionModel 

# NAME

LogisticRegressionModel -  A data structure used in modeling the output 
of the frovedis server side logistic regression algorithm at client spark side.

# SYNOPSIS

import com.nec.frovedis.mllib.classification.LogisticRegressionModel    

## Public Member Functions
Double predict (Vector data)   
`RDD[Double]` predict (`RDD[Vector]` data)   
Unit save(String path)   
Unit save(SparkContext sc, String path)   
LogisticRegressionModel LogisticRegressionModel.load(String path)    
LogisticRegressionModel LogisticRegressionModel.load(SparkContext sc, String path)    
Unit debug_print()   
Unit release()    

# DESCRIPTION

LogisticRegressionModel models the output of the frovedis logistic regression 
algorithm, the trainer interface of which aims to optimize an initial 
model and outputs the same after optimization. 

Note that the actual model with weight parameter etc. is created at frovedis 
server side only. Spark side LogisticRegressionModel contains a unique ID 
associated with the frovedis server side model, along with some generic 
information like number of features etc. It simply works like a pointer to 
the in-memory model at frovedis server. 

Any operations, like prediction etc. on a LogisticRegressionModel makes a request 
to the frovedis server along with the unique model ID and the actual job is served 
by the frovedis server. For functions which returns some output, the result is sent 
back from frovedis server to the spark client.

## Pubic Member Function Documentation

### Double predict (Vector data)
This function can be used when prediction is to be made on the trained model 
for a single sample. It returns with the predicted value from the frovedis server. 

### `RDD[Double]` predict (`RDD[Vector]` data)
This function can be used when prediction is to be made on the trained model 
for more than one samples distributed among spark workers. 

It is performed by all the worker nodes in parallel and on success 
the function returns a `RDD[Double]` object containing the distributed predicted 
values at worker nodes. 

### LogisticRegressionModel LogisticRegressionModel.load(String path)
This static function is used to load the target model with data in given 
filename stored at frovedis server side at specified location (filename with 
relative/absolute path) as little-endian binary data.
On success, it returns the loaded model.

### LogisticRegressionModel LogisticRegressionModel.load(SparkContext sc, String path)
This is Spark like static API provided for compatibility with spark code. 
But the "sc" parameter is simply ignored in this case and internally it calls 
the above load() method as "LogisticRegressionModel.load(path)".

### Unit save(String path)
This function is used to save the target model with given filename. 
Note that the target model is saved at frovedis server side at specified location 
(filename with relative/absolute path) as little-endian binary data.

### Unit save(SparkContext sc, String path)
This is Spark like API provided for compatibility with spark code. 
But the "sc" parameter is simply ignored in this case and internally it calls 
the above save() method as "save(path)".

### Unit debug_print()  
It prints the contents of the server side model on the server side user terminal. 
It is mainly useful for debugging purpose.

### Unit release()  
This function can be used to release the existing in-memory model at frovedis 
server side.

# SEE ALSO
linear_regression_model, svm_model   
