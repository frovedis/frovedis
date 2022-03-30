% KNeighborsRegressor

# NAME

KNeighborsRegressor - Regression based on k-nearest neighbors.  

# SYNOPSIS

    class frovedis.mllib.neighbors.KNeighborsRegressor(n_neighbors=5, weights=‘uniform’,  
                                                       algorithm='auto', leaf_size=30, p=2,  
                                                       metric='euclidean', metric_params=None,  
                                                       n_jobs=None, verbose=0,  
                                                       chunk_size=1.0, batch_fraction=None)  

## Public Member Functions

fit(X, y)  
kneighbors(X = None, n_neighbors = None, return_distance = True)  
kneighbors_graph(X = None, n_neighbors = None, mode = 'connectivity')  
save(fname)  
load(fname)  
predict(X)  
score(X, y, sample_weight = None)  
get_params(deep = True)  
set_params(\*\*params)  
debug_print()  
release()  
is_fitted()  

# DESCRIPTION

Neighbors-based regression can be used in cases where the data labels are
continuous rather than discrete variables. The label assigned to a query point is
computed based on the mean of the labels of its nearest neighbors.   

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis interface is almost same as Scikit-learn 
KNeighborsRegressor interface, but it doesn't have any dependency with Scikit-learn. 
It can be used simply even if the system doesn't have Scikit-learn installed. Thus, in 
this implementation, a python client can interact with a frovedis server sending the 
required python data for training at frovedis side. Python data is converted into 
frovedis compatible data internally and the python ML call is linked with the respective 
frovedis ML call to get the job done at frovedis server.  

Python side calls for KNeighborsRegressor on the frovedis server. Once the training is 
completed with the input data at the frovedis server, it returns an abstract model with 
a unique model ID to the client python program.  

When kneighbors-like request would be made on the trained model, python program will 
send the same request to the frovedis server. After the request is served at the frovedis 
server, the output would be sent back to the python client.  

## Detailed Description  

### 1. KNeighborsRegressor()  

__Parameters__  
**_n\_neighbors_**: A positive integer parameter, specifying the number of neighbors to 
use by default for 'kneighbors' queries. It must be within the range of 0 and n_samples. (Default: 5)  
**_weights_**: An unused parameter.  (Default: uniform)  
**_algorithm_**: A string object parameter, specifying the algorithm used to compute the 
nearest neighbors. (Default: auto)  
When it is ‘auto’, it will be set as ‘brute’(brute-force search approach). Unlike Scikit-learn, 
currently it supports only ‘brute’.    
**_leaf\_size_**: An unused parameter. (Default: 30)  
**_p_**: An unused parameter. (Default: 2)  
**_metric_**: A string object parameter specifying the distance metric to use for the 
tree. (Default: ‘euclidean’)  
Currenlty it only supports ‘euclidean’, 'seuclidean' and 'cosine' distance.  
**_metric\_params_**: An unused parameter. (Default: None)  
**_n\_jobs_**: An unused parameter. (Default: None)  
**_verbose_**: An integer parameter specifying the log level to use. Its value is set as 0 
by default (for INFO mode). But it can be set to 1 (for DEBUG mode) or 2 (for TRACE mode) for 
getting training time logs from frovedis server.  
**_chunk\_size_**: A positive float parameter specifying the amount of data (in megabytes) to 
be processed in one time. (Default: 1.0)  
**_batch\_fraction_**: A positive double (float64) parameter used to calculate the batches 
of specific size. These batches are used to construct the distance matrix. It must be within 
the range of 0.0 to 1.0. (Default: None)  
When it is None (not specified explicitly), it will be set as np.finfo(np.float64).max value.  

__Purpose__  
It initializes a KNeighborsRegressor object with the given parameters.   

The parameters: "weights","leaf_size", "p","metric_params" and "n_jobs" are simply kept in to make the 
interface uniform to the Scikit-learn KNeighborsRegressor module. They are not used anywhere 
within frovedis implementation.  

__Return Value__    
It simply returns "self" reference. 

### 2. fit(X, y)  
__Parameters__   
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data of float or 
double (float64) type. It has shape **(n_samples, n_features)**.  
_**y**_: Any python array-like object or an instance of FrovedisDvector containing the target labels.
It has shape **(n_samples,)**. 

__Purpose__  
It accepts the training matrix (X) with labels (y) and trains a KNeighborsRegressor model.  

For example,   

    # loading sample data 
    samples = np.loadtxt("./input/knr_data.txt", dtype = np.float64)
    lbl = [10, 10, 10, 20, 10, 20]
    
    # fitting input data on KNeighborsRegressor object
    from frovedis.mllib.neighbors import KNeighborsRegressor
    knr = KNeighborsRegressor(n_neighbors = 3)
    knr.fit(samples, lbl)

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,   

    # loading sample data
    samples = np.loadtxt("./input/knr_data.txt", dtype = np.float64)
    lbl = [10, 10, 10, 20, 10, 20]
    
    # Since "samples" is numpy dense data, we have created FrovedisRowmajorMatrix. 
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    from frovedis.matrix.dvector import FrovedisDvector
    rmat = FrovedisRowmajorMatrix(samples)
    dlbl = FrovedisDvector(lbl)
    
    # fitting input data on KNeighborsRegressor object
    from frovedis.mllib.neighbors import KNeighborsRegressor
    knr = KNeighborsRegressor(n_neighbors = 3)
    knr.fit(rmat, dlbl)

__Return Value__  
It simply returns "self" reference.   

### 3. kneighbors(X = None, n_neighbors = None, return_distance = True)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data of float or 
double (float64) type. It has shape **(n_queries, n_features)**, where ‘n_queries’ is the 
number of rows in the test data. (Default: None)  
When it is None (not specified explicitly), it will be training data (X) used as input in fit().  
**_n\_neighbors_**: A positive integer parameter, specifying the number of neighbors to 
use by default for 'kneighbors' queries. It must be within the range of 0 and n_samples. (Default: None)  
When it is None (not specified explicitly), it will be 'n_neighbors' value used during 
KNeighborsRegressor object creation.  
**_return\_distance_**: A boolean parameter specifying whether or not to return the distances. (Default: True)  
If set to False, it will not return distances. Then, only indices are returned by this method.  

__Purpose__    
Finds the k-Neighbors of a point and returns the indices of neighbors and distances to the 
neighbors of each point.  

For example,   

    distances, indices = knr.kneighbors(samples)
    print('distances')
    print(distances)
    print('indices')
    print(indices)

Output
    
    distances
    [[0.         1.         2.23606798]
     [0.         1.         1.41421356]
     [0.         1.41421356 2.23606798]
     [0.         1.         2.23606798]
     [0.         1.         1.41421356]
     [0.         1.41421356 2.23606798]]
    indices
    [[0 1 2]
     [1 0 2]
     [2 1 0]
     [3 4 5]
     [4 3 5]
     [5 4 3]]

Like in fit(), frovedis-like input can be used to speed-up the computation of indices and distances 
at server side. 

For example,   

    # Since "samples" is numpy dense data, we have created FrovedisRowmajorMatrix. 
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(samples)
   
    # fitting input data on KNeighborsRegressor object
    from frovedis.mllib.neighbors import KNeighborsRegressor
    knr = KNeighborsRegressor(n_neighbors = 3)
    distances, indices = knr.kneighbors(rmat)
    
    # Here FrovedisRowmajorMatrix().debug_print() is used
    print('distances')
    distances.debug_print()
    
    # Here FrovedisRowmajorMatrix().debug_print() is used   
    print('indices')
    indices.debug_print()
    
Output
    
    distances
    matrix:
    num_row = 6, num_col = 3
    node 0
    node = 0, local_num_row = 6, local_num_col = 3, val = 0 1 2.23607 0 1 1.41421 0 1.41421 
    2.23607 0 1 2.23607 0 1 1.41421 0 1.41421 2.23607
    indices
    matrix:
    num_row = 6, num_col = 3
    node 0
    node = 0, local_num_row = 6, local_num_col = 3, val = 0 1 2 1 0 2 2 1 0 3 4 5 4 3 5 5 4 3

It returns distances and indices as FrovedisRowmajorMatrix objects.  

__Return Value__  
1. **When test data and training data used by fitted model are python native input :**  
     - **_distances_**: A numpy array of float or double (float64) type values. It has 
     shape **(n_queries, n_neighbors)**, where ‘n_queries’ is the number of rows in the test data.
     It is only returned by kneighbors(), if return_distance = True.  
     - **_indices_**: A numpy array of int64 type values. It has shape **(n_queries, n_neighbors)**.  

2. **When test data and training data used by fitted model is frovedis-like input::**  
     - **_distances_**: A FrovedisRowmajorMatrix of float or double (float64) type values. It has 
     shape **(n_queries, n_neighbors)**, where ‘n_queries’ is the number of rows in the test data.
     It is only returned by kneighbors(), if return_distance = True.  
     - **_indices_**: A FrovedisRowmajorMatrix of int64 typess values. It has shape **(n_queries, n_neighbors)**.  

### 4. kneighbors_graph(X = None, n_neighbors = None, mode = 'connectivity')
__Parameters__   
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data of float or 
double (float64) type. It has shape **(n_queries, n_features)**, where ‘n_queries’ is the 
number of rows in the test data. (Default: None)  
When it is None (not specified explicitly), it will be training data (X) used as input in fit().  
**_n\_neighbors_**: A positive integer parameter, specifying the number of neighbors to 
use by default for 'kneighbors' queries. It must be within the range of 0 and n_samples. (Default: None)  
When it is None (not specified explicitly), it will be 'n_neighbors' value used during 
KNeighborsRegressor object creation.  
**_mode_**:  A string object parameter which can be either 'connectivity' or 'distance'. It 
specifies the type of returned matrix.  
For 'connectivity', it will return the connectivity matrix with ones and zeros, whereas for 
‘distance’, the edges are euclidean distance between points, type of distance depends on the 
selected 'metric' value in KNeighborsRegressor class. (Default: 'connectivity')  

__Purpose__    
It computes the (weighted) graph of k-Neighbors for points in X.

For example,   

    # Here 'mode = connectivity' by default
    graph = knr.kneighbors_graph(samples)
    print('kneighbors graph')
    print(graph)

Output 
    
    kneighbors graph
    (0, 0)        1.0
    (0, 1)        1.0
    (0, 2)        1.0
    (1, 1)        1.0
    (1, 0)        1.0
    (1, 2)        1.0
    (2, 2)        1.0
    (2, 1)        1.0
    (2, 0)        1.0
    (3, 3)        1.0
    (3, 4)        1.0
    (3, 5)        1.0
    (4, 4)        1.0
    (4, 3)        1.0
    (4, 5)        1.0
    (5, 5)        1.0
    (5, 4)        1.0
    (5, 3)        1.0
    
For example, when mode = ‘distance’  

    # Here 'mode = distance'
    graph = knr.kneighbors_graph(samples, mode = 'distance')
    print('kneighbors graph')
    print(graph)

Output 
    
    kneighbors graph
    (0, 0)        0.0
    (0, 1)        1.0
    (0, 2)        2.23606797749979
    (1, 1)        0.0
    (1, 0)        1.0
    (1, 2)        1.4142135623730951
    (2, 2)        0.0
    (2, 1)        1.4142135623730951
    (2, 0)        2.23606797749979
    (3, 3)        0.0
    (3, 4)        1.0
    (3, 5)        2.23606797749979
    (4, 4)        0.0
    (4, 3)        1.0
    (4, 5)        1.4142135623730951
    (5, 5)        0.0
    (5, 4)        1.4142135623730951
    (5, 3)        2.23606797749979
    
Like in fit(), frovedis-like input can be used to speed-up the graph making at server side. 

For example,   

    # Since "samples" is numpy dense data, we have created FrovedisRowmajorMatrix. 
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(samples)

    # fitting input data on KNeighborsRegressor object
    from frovedis.mllib.neighbors import KNeighborsRegressor
    knr = KNeighborsRegressor(n_neighbors = 3)
    
    # Here 'mode = connectivty' by default
    graph = knr.kneighbors_graph(rmat)
    
    # Here FrovedisCRSMatrix().debug_print() is used    
    print('graph')
    graph.debug_print()

Output 
    
    graph
    Active Elements:  18
    matrix:
    num_row = 6, num_col = 6
    node 0
    local_num_row = 6, local_num_col = 6
    val : 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
    idx : 0 1 2 1 0 2 2 1 0 3 4 5 4 3 5 5 4 3
    off : 0 3 6 9 12 15 18

It returns a FrovedisCRSMatrix object for frovedis-like input.  
    
__Return Value__  
- **When test data and training data used by fitted model are python native input:**  
It returns a scipy sparse matrix of float or double (float64) type values.
It has shape **(n_queries, n_samples_fit)**, where ‘n_queries’ is the number of 
rows in the test data and 'n_samples_fit' is the number of samples in the fitted data.  
- **When either test data or training data used by fitted model is frovedis-like input:**  
It returns a FrovedisCRSMatrix of float or double (float64) type values. It has shape 
**(n_queries, n_samples_fit)**, where ‘n_queries’ is the number of rows in the test data 
and 'n_samples_fit' is the number of samples in the fitted data.  

### 5. save(fname)
__Parameters__   
**_fname_**: A string object containing the name of the file on which the target 
model is to be saved.  

__Purpose__  
Currently, this method is not supported for KNeighborsRegressor. It is simply kept 
in KNeighborsRegressor module to maintain uniform interface like other estimators in frovedis.  

__Return Value__  
It simply raises an AttributeError.  

### 6. load(fname)
__Parameters__   
**_fname_**:  A string object containing the name of the file having model information
to be loaded.  

__Purpose__  
Currently, this method is not supported for KNeighborsRegressor. 
It is simply kept in KNeighborsRegressor module to maintain uniform interface like other 
estimators in frovedis.  

__Return Value__  
It simply raises an AttributeError.  

### 7. predict(X)  

__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data of float or 
double (float64) type. It has shape **(n_queries, n_features)**, where ‘n_queries’ is the 
number of rows in the test data.  

__Purpose__  
Predict the class labels for the provided data.  

For example,  

    # predicting on KNeighborsRegressor model
    knr.predict(samples)
    
Output  

    [10.         10.         10.         16.66666667 16.66666667 16.66666667]

Like in fit(), frovedis-like input can be used to speed-up the prediction making on the trained 
model at server side.  

For example,  

    # Since "samples" is numpy dense data, we have created FrovedisRowmajorMatrix.
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(samples)
    
    # predicting on KNeighborsRegressor model using pre-constructed input
    knr.predict(rmat)

Output  

    [10.         10.         10.         16.66666667 16.66666667 16.66666667]
    
__Return Value__  
It returns a numpy array of long (int64) type containing the predicted outputs. It is of 
shape **(n_queries,)**, where 'n_queries' is the number of rows in the test data.  

### 8. score(X, y, sample_weight = None)  
__Parameters__   
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance of 
FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data of float or double (float64) 
type. It has shape **(n_samples, n_features)**.    
_**y**_: Any python array-like object or an instance of FrovedisDvector containing the target values.
It has shape **(n_samples,)**.  
**_sample\_weight_**: An unused parameter whose default value is None. It is simply ignored in frovedis
implementation, like in Scikit-learn.  

__Purpose__  
Calculate the root mean square value on the given test data and labels i.e. R2(r-squared) of self.predict(X) 
wrt. y.

The coefficient 'R2' is defined as (1 - (u/v)),  
where 'u' is the residual sum of squares ((y_true - y_pred) ** 2).sum() and,  
'v' is the total sum of squares ((y_true - y_true.mean()) ** 2).sum().  
The best possible score is 1.0 and it can be negative (because the model can be arbitrarily worse). A constant 
model that always predicts the expected value of y, disregarding the input features, would get a R2 score of 0.0.  

For example,  

    knr.score(samples, lbl)

Output

    0.5 

__Return Value__  
It returns an R2 score of float type.  

### 9. get_params(deep = True)  

__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If True, 
it will return the parameters for an estimator and contained subobjects that are estimators. (Default: True)  

__Purpose__    
This method belongs to the BaseEstimator class inherited by KNeighborsRegressor. It is used to get parameters 
and their values of KNeighborsRegressor class.  

For example, 
 
    print(knr.get_params())

Output  

    {'algorithm': 'auto', 'batch_fraction': None, 'chunk_size': 1.0, 'leaf_size': 30, 
    'metric': 'euclidean', 'metric_params': None, 'n_jobs': None, 'n_neighbors': 3, 
    'p': 2, 'verbose': 0, 'weights': 'uniform '}
    
__Return Value__  
A dictionary of parameter names mapped to their values.  

### 10. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed this function as dictionary. This dictionary 
contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by KNeighborsRegressor, used to set parameter values.  

For example,   

    print("Get parameters before setting:") 
    print(knr.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    knr.set_params(n_neighbors = 5) 
    print("Get parameters after setting:") 
    print(knr.get_params())

Output  
     
    Get parameters before setting: 
    {'algorithm': 'auto', 'batch_fraction': None, 'chunk_size': 1.0, 'leaf_size': 30, 
    'metric': 'euclidean', 'metric_params': None, 'n_jobs': None, 'n_neighbors': 3, 
    'p': 2, 'verbose': 0, 'weights': 'uniform '}
    Get parameters after setting: 
    {'algorithm': 'auto', 'batch_fraction': None, 'chunk_size': 1.0, 'leaf_size': 30, 
    'metric': 'euclidean', 'metric_params': None, 'n_jobs': None, 'n_neighbors': 5, 
    'p': 2, 'verbose': 0, 'weights': 'uniform '}
    
__Return Value__  
It simply returns "self" reference.  

### 11. debug_print()

__Purpose__   
 Currently, this method is not supported for KNeighborsRegressor. 
It is simply kept in KNeighborsRegressor module to maintain uniform interface like other 
estimators in frovedis.  

__Return Value__  
It simply raises an AttributeError.  

### 12. release()  

__Purpose__  
It can be used to release the in-memory model at frovedis server.  

For example,
 
    knr.release()

This will remove the trained model, model-id present on server, along with releasing server 
side memory.  

__Return Value__  
It returns nothing.   

### 13. is_fitted()  

__Purpose__  
It can be used to confirm if the model is already fitted or not.  

__Return Value__  
It returns 'True', if the model is already fitted otherwise, it returns 'False'.  

# SEE ALSO  
- **[Introduction to FrovedisRowmajorMatrix](../matrix/rowmajor_matrix.md)**  
- **[Introduction to FrovedisCRSMatrix](../matrix/crs_matrix.md)**  
- **[Introduction to FrovedisDvector](../matrix/dvector.md)**  
- **[KNeighbors Classifier in Frovedis](./kneighbors_classifier.md)**  
