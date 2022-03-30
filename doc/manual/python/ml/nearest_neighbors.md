% NearestNeighbors

# NAME

NearestNeighbors - Unsupervised learner for implementing neighbor searches.   

# SYNOPSIS

    class frovedis.mllib.neighbors.NearestNeighbors(n_neighbors=5, radius=1.0, algorithm='auto',
                                                    leaf_size=30, metric='euclidean', p=2,
                                                    metric_params=None, n_jobs=None, verbose=0,
                                                    chunk_size=1.0, batch_fraction=None)  

## Public Member Functions

fit(X, y = None)  
kneighbors(X = None, n_neighbors = None, return_distance = True)  
kneighbors_graph(X = None, n_neighbors = None, mode = 'connectivity')  
radius_neighbors(X = None, radius = None, return_distance = True)  
radius_neighbors_graph(X = None, radius = None, mode = 'connectivity')  
get_params(deep = True)  
set_params(\*\*params)  
save(fname)  
load(fname)  
debug_print()  
release()  
is_fitted()  

# DESCRIPTION

Unsupervised nearest neighbors is the foundation of many other learning methods, 
notably manifold learning and spectral clustering.  
 
The principle behind nearest neighbor methods is to find a predefined number of training 
samples closest in distance to the new point and predict the label from these. The number 
of samples can be a user-defined constant (k-nearest neighbor learning), or vary based on 
the local density of points (radius-based neighbor learning). In general, the distance can 
be any metric measure: standard Euclidean distance is the most common choice. Neighbors-based 
methods are known as non-generalizing machine learning methods, since they simply “remember” 
all of its training data.

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis interface is almost same as Scikit-learn 
NearestNeighbors interface, but it doesn't have any dependency with Scikit-learn. 
It can be used simply even if the system doesn't have Scikit-learn installed. Thus in 
this implementation, a python client can interact with a frovedis server sending the 
required python data for training at frovedis side. Python data is converted into 
frovedis compatible data internally and the python ML call is linked with the respective 
frovedis ML call to get the job done at frovedis server.  

Python side calls for NearestNeighbors on the frovedis server. Once the training is completed 
with the input data at the frovedis server, it returns an abstract model with a 
unique model ID to the client python program.  

When operations like kneighbors() will be required on the trained model, python program 
will send the same request to the frovedis server. After the request is served at 
the frovedis server, the output would be sent back to the python client.  

## Detailed Description  

### 1. NearestNeighbors()   

__Parameters__  
**_n\_neighbors_**: A positive integer parameter, specifying the number of neighbors to 
use by default for 'kneighbors' queries. It must be within the range of 0 and n_samples. (Default: 5)  
**_radius_**: A positive float parameter, specifying the range of parameter space to use 
by default for 'radius_neighbors' queries. (Default: 1.0)  
**_algorithm_**: A string object parameter, specifying the algorithm used to compute the 
nearest neighbors. (Default: 'auto')  
When it is ‘auto’, it will be set as ‘brute’(brute-force search approach). Unlike 
Scikit-learn, currently it supports only ‘brute’.  
**_leaf\_size_**: An unsed parameter. (Default: 30)  
**_metric_**: A string object parameter, specifying the distance metric to use for the 
tree. (Default: ‘euclidean’)  
Currenlty it only supports ‘euclidean’, 'seuclidean' and 'cosine' distance.  
**_p_**: An unused parameter. (Default: 2)  
**_metric\_params_**: An unsed parameter. (Default: None)  
**_n\_jobs_**: An unsed parameter. (Default: None)  
**_verbose_**: An integer parameter specifying the log level to use. Its value is set as 0 
by default (for INFO mode). But it can be set to 1(for DEBUG mode) or 2(for TRACE mode) for 
getting training time logs from frovedis server.  
**_chunk\_size_**: A positive float parameter, specifying the amount of data (in megabytes) to 
be processed in one time. (Default: 1.0)  
**_batch\_fraction_**: A positive double (float64) parameter used to calculate the batches 
of specific size. These batches are used to construct the distance matrix. It must be within 
the range of 0.0 to 1.0. (Default: None)  
When it is None (not specified explicitly), it will be set as np.finfo(np.float64).max value.  

__Purpose__    
It initializes a NearestNeighbors object with the given parameters.   

The parameters: "leaf_size", "p", "metric_params" and "n_jobs" are simply kept in to make the 
interface uniform to the Scikit-learn NearestNeighbors module. They are not used anywhere 
within the frovedis implementation.  

__Return Value__    
It simply returns "self" reference. 

### 2. fit(X, y = None)
__Parameters__   
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data of float or 
double (float64) type. It has shape **(n_samples, n_features)**.  
**_y_**:  None or any python array-like object (any shape). It is simply ignored in frovedis 
implementation, like in Scikit-learn.  

__Purpose__    
Fit the model using X as training data

For example,   

    # loading sample data 
    samples = np.loadtxt("./input/knn_data.txt", dtype = np.float64)
    
    # fitting input data on NearestNeighbors object
    from frovedis.mllib.neighbors import NearestNeighbors
    knn = NearestNeighbors(n_neighbors = 3, radius = 2.0, algorithm = 'brute', 
                           metric = 'euclidean')
    knn.fit(samples)

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,   

    # loading sample data
    samples = np.loadtxt("./input/knn_data.txt", dtype = np.float64)
    
    # Since "samples" is numpy dense data, we have created FrovedisRowmajorMatrix. 
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(samples)
    
    # fitting input data on NearestNeighbors object
    from frovedis.mllib.neighbors import NearestNeighbors
    knn = NearestNeighbors(n_neighbors = 3, radius = 2.0, algorithm = 'brute', 
                           metric = 'euclidean')
    knn.fit(rmat)


__Return Value__  
It simply returns "self" reference.   

### 3. kneighbors(X = None, n_neighbors = None, return_distance = True)  
__Parameters__   
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data of float or 
double (float64) type. It has shape **(n_queries, n_features)**, where 'n_queries' is the number 
of rows in the test data. (Default: None)  
When it is None (not specified explicitly), it will be training data (X) used as input in fit().  
**_n\_neighbors_**: A positive integer parameter, specifying the number of neighbors to 
use by default for 'kneighbors' queries. It must be within the range of 0 and n_queries. (Default: None)  
When it is None (not specified explicitly), it will be 'n_neighbors' value used during 
NearestNeighbors object creation.  
**_return\_distance_**: A boolean parameter specifying whether or not to return the distances. (Default: True)  
If set to False, it will not return distances. Then, only indices are returned by this method.  

__Purpose__    
It finds the k-Neighbors of a point and returns the indices of neighbors and distances to the 
neighbors of each point.  

For example,   

    distances, indices = knn.kneighbors(samples)
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
     [[0.         1.         2.23606798]
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
    
    distances, indices = knn.kneighbors(rmat)
    
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

__Return Value__  
1. **When test data and training data used by fitted model are python native input:**  
     - **_distances_**: A numpy array of float or double(float64) type values. It has 
     shape **(n_queries, n_neighbors)**, where 'n_queries' is the number of rows in the test 
     data. It is only returned by kneighbors() if return_distance = True.  
     - **_indices_**: A numpy array of int64 type values. It has shape **(n_queries, n_neighbors)**, 
     where 'n_queries' is the number of rows in the test data.  
2. **When either test data or training data used by fitted model is frovedis-like input:**  
     - **_distances_**: A FrovedisRowmajorMatrix of float or double(float64) type values. It has 
     shape **(n_queries, n_neighbors)**, where 'n_queries' is the number of rows in the test data. 
     It is only returned by kneighbors() if return_distance = True.  
     - **_indices_**: A FrovedisRowmajorMatrix of int64 type values. It has shape 
     **(n_queries, n_neighbors)**, where 'n_queries' is the number of rows in the test data.  


### 4. kneighbors_graph(X = None, n_neighbors = None, mode = 'connectivity')
__Parameters__   
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data of float or 
double (float64) type. It has shape **(n_queries, n_features)**, where 'n_queries' is the 
number of rows in the test data. (Default: None)  
When it is None (not specified explicitly), it will be training data (X) used as input in fit().  
**_n\_neighbors_**: A positive integer parameter, specifying the number of neighbors to 
use by default for 'kneighbors' queries. It must be within the range of 0 and n_queries. (Default: None)  
When it is None (not specified explicitly), it will be 'n_neighbors' value used during 
NearestNeighbors object creation.  
**_mode_**:  A string object parameter which can be either 'connectivity' or 'distance'. It 
specifies the type of returned matrix.  
For 'connectivity', it will return the connectivity matrix with ones and zeros, whereas for 
‘distance’, the edges are euclidean distance between points. Type of distance depends on the 
selected 'metric' value in NearestNeighbors class. (Default: 'connectivity')  

__Purpose__    
It computes the (weighted) graph of k-Neighbors for points in X.

For example, when mode = 'connectivity'  

    # Here 'mode = connectivity' by default
    graph = knn.kneighbors_graph(samples)
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


For example, when mode = 'distance'  

    # Here 'mode = distance'  
    graph = knn.kneighbors_graph(samples, mode = 'distance')
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

Like in fit(), frovedis-like input can be used to speed-up the graph construction at server side. 

For example,   

    # Since "samples" is numpy dense data, we have created FrovedisRowmajorMatrix. 
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(samples)
    
    # Here 'mode = connectivity' by default
    graph = knn.kneighbors_graph(rmat)
    print('kneighbors graph')

    # Here FrovedisCRSMatrix().debug_print() is used 
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
    
__Return Value__  
- **When test data and training data used by fitted model are python native input :**  
It returns a scipy sparse csr matrix of float or double (float64) type values. It has 
shape **(n_queries, n_samples_fit)**, where 'n_queries' is the number of rows in the test data 
and 'n_samples_fit' is the number of samples in the fitted data.  
- **When either test data or training data used by fitted model is frovedis-like input:**  
It returns a FrovedisCRSMatrix of float or double(float64) type values. It has 
shape **(n_queries, n_samples_fit)**, where 'n_queries' is the number of rows in the test data 
and 'n_samples_fit' is the number of samples in the fitted data.  


### 5. radius_neighbors(X = None, radius = None, return_distance = True)
__Parameters__   
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data of float or 
double (float64) type. It has shape **(n_samples, n_features)**. (Default: None)  
When it is None (not specified explicitly), it will be training data (X) used as input in fit().  
**_radius_**: A positive float parameter, specifying the limiting distance of neighbors to 
return. (Default: None)  
When it is None (not specified explicitly), it will be 'radius' value used in NearestNeighbors object 
creation.  
**_return\_distance_**: A boolean parameter specifying whether or not to return the distances. (Default: True)  
If set to False, it will not return distances. Then, only indices are returned by this method.  

__Purpose__    
It finds the neighbors within a given radius of a point or points and returns indices and distances 
to the neighbors of each point.  

For example,   

    dist, ind = knn.radius_neighbors(samples)
    print('radius neighbors')
    for i in range(len(dist)):
        print(dist[i])
        print(ind[i])
Output

    radius neighbors
    [0. 1.]
    [0 1]
    [1.         0.         1.41421356]
    [0 1 2]
    [1.41421356 0.        ]
    [1 2]
    [0. 1.]
    [3 4]
    [1.         0.         1.41421356]
    [3 4 5]
    [1.41421356 0.        ]
    [4 5]

Like in fit(), frovedis-like input can be used to speed-up the computation of indices and distances 
at server side. 

For example,   

    # Since "samples" is numpy dense data, we have created FrovedisRowmajorMatrix. 
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(samples)
    
    rad_nbs = knn.radius_neighbors(rmat)
    print('radius neighbors')
    
    # Here FrovedisCRSMatrix().debug_print() is used  
    rad_nbs.debug_print()

Output

    radius neighbors
    Active Elements:  14
    matrix:
    num_row = 6, num_col = 6
    node 0
    local_num_row = 6, local_num_col = 6
    val : 0 1 1 0 1.41421 1.41421 0 0 1 1 0 1.41421 1.41421 0
    idx : 0 1 0 1 2 1 2 3 4 3 4 5 4 5
    off : 0 2 5 7 9 12 14
    
__Return Value__  
1. **When test data and training data used by fitted model are python native input :**  
     - **_distance_**: A python list of float or double(float64) type values and has length **n_samples**. It 
     is only returned by radius_neighbors() if return_distance = True.  
     - **_indices_**: A python list of float or double(float64) type values and has length **n_samples**.  
2. **When either test data or training data used by fitted model is frovedis-like input:**  
It returns a FrovedisCRSMatrix of shape **(n_samples, n_samples_fit)**, where 'n_samples_fit' 
is the number of samples in the fitted data.  


### 6. radius_neighbors_graph(X = None, radius = None, mode = 'connectivity')
__Parameters__   
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data of float or 
double (float64) type. It has shape **(n_samples, n_features)**. (Default: None)  
When it is None (not specified explicitly), it will be training data (X) used as input in fit().  
**_radius_**: A positive float parameter, specifying the limiting distance of neighbors to 
return. (Default: None)  
When it is None (not specified explicitly), it will be 'radius' value used in NearestNeighbors object 
creation.  
**_mode_**:  A string object parameter which can be either 'connectivity' or 'distance'. It 
specifies the type of returned matrix.  
For 'connectivity', it will return the connectivity matrix with ones and zeros, whereas for 
‘distance’, the edges are euclidean distance between points. Type of distance depends on the 
selected 'metric' value in NearestNeighbors class. (Default: 'connectivity')  

__Purpose__    
It computes the (weighted) graph of Neighbors for points in X.  

For example, when mode = 'connectivity'  

    # Here 'mode = connectivity' by default
    rad_graph = knn.radius_neighbors_graph(samples)
    print('radius neighbors graph')
    print(rad_graph)

Output

    radius neighbors graph
    (0, 0)        1.0
    (0, 1)        1.0
    (1, 0)        1.0
    (1, 1)        1.0
    (1, 2)        1.0
    (2, 1)        1.0
    (2, 2)        1.0
    (3, 3)        1.0
    (3, 4)        1.0
    (4, 3)        1.0
    (4, 4)        1.0
    (4, 5)        1.0
    (5, 4)        1.0
    (5, 5)        1.0

For example, when mode = 'distance'  

    # Here 'mode = distance'
    rad_graph = knn.radius_neighbors_graph(samples, mode = 'distance')
    print('radius neighbors graph')
    print(rad_graph)

Output

    radius neighbors graph
    (0, 0)        0.0
    (0, 1)        1.0
    (1, 0)        1.0
    (1, 1)        0.0
    (1, 2)        1.4142135623730951
    (2, 1)        1.4142135623730951
    (2, 2)        0.0
    (3, 3)        0.0
    (3, 4)        1.0
    (4, 3)        1.0
    (4, 4)        0.0
    (4, 5)        1.4142135623730951
    (5, 4)        1.4142135623730951
    (5, 5)        0.0

Like in fit(), frovedis-like input can be used to speed-up the computation of graph construction 
at server side.  

For example,   

    # Since "samples" is numpy dense data, we have created FrovedisRowmajorMatrix. 
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(samples)
    
    # Here 'mode = connectivity' by default
    rad_graph = knn.radius_neighbors_graph(samples)
    print('radius neighbors graph')
    
    # Here FrovedisCRSMatrix().debug_print() is used  
    rad_graph.debug_print()

Output

    radius neighbors graph
    Active Elements:  14
    matrix:
    num_row = 6, num_col = 6
    node 0
    local_num_row = 6, local_num_col = 6
    val : 1 1 1 1 1 1 1 1 1 1 1 1 1 1
    idx : 0 1 0 1 2 1 2 3 4 3 4 5 4 5
    off : 0 2 5 7 9 12 14  
    
__Return Value__  
- **When test data and training data used by fitted model are python native input :**  
It returns a scipy sparse csr matrix of float or double(float64) type values. It has 
shape **(n_queries, n_samples_fit)**, where 'n_queries' is the number of rows in the test data 
and 'n_samples_fit' is the number of samples in the fitted data.  
- **When either test data or training data used by fitted model is frovedis-like input:**  
It returns a FrovedisCRSMatrix of float or double(float64) type values. It has 
shape **(n_queries, n_samples_fit)**, where 'n_queries' is the number of rows in the test data and 
'n_samples_fit' is the number of samples in the fitted data.  


### 7. get_params(deep = True)  

__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If True, 
it will return the parameters for an estimator and contained subobjects that are estimators. (Default: True)  

__Purpose__    
This method belongs to the BaseEstimator class inherited by NearestNeighbors. It is used to get parameters 
and their values of NearestNeighbors class.  

For example, 
 
    print(knn.get_params())

Output  

    {'algorithm': 'brute', 'batch_fraction': 1.7976931348623157e+308, 'chunk_size': 1.0, 
    'leaf_size': 30, 'metric': 'euclidean', 'metric_params': None, 'n_jobs': None, 
    'n_neighbors': 3, 'p': 2, 'radius': 2.0, 'verbose': 0}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 8. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed to this function as dictionary. This dictionary 
contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by NearestNeighbors, used to set parameter values.  

For example,   

    print("get parameters before setting:") 
    print(knn.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    knn.set_params(n_neighbors = 4, radius = 1.0) 
    print("get parameters after setting:") 
    print(knn.get_params())

Output  
     
    get parameters before setting: 
    {'algorithm': 'brute', 'batch_fraction': 1.7976931348623157e+308, 'chunk_size': 1.0,
    'leaf_size': 30, 'metric': 'euclidean', 'metric_params': None, 'n_jobs': None, 
    'n_neighbors': 3, 'p': 2, 'radius': 2.0, 'verbose': 0}
    get parameters after setting: 
    {'algorithm': 'brute', 'batch_fraction': 1.7976931348623157e+308, 'chunk_size': 1.0,
    'leaf_size': 30, 'metric': 'euclidean', 'metric_params': None, 'n_jobs': None, 
    'n_neighbors': 4, 'p': 2, 'radius': 1.0, 'verbose': 0} 
    
__Return Value__  
It simply returns "self" reference.  

### 9. save(fname)
__Parameters__   
**_fname_**: A string object containing the name of the file on which the target 
model is to be saved.  

__Purpose__  
Currently this method is not supported for NearestNeighbors. It is simply kept in NearestNeighbors
module to maintain uniform interface for all estimators in frovedis.  

__Return Value__  
It simply raises an AttributeError.  

### 10. load(fname)
__Parameters__   
**_fname_**:  A string object containing the name of the file having model information
to be loaded.  

__Purpose__  
Currently this method is not supported for NearestNeighbors. It is simply kept in NearestNeighbors
module to maintain uniform interface for all estimators in frovedis.  

__Return Value__  
It simply raises an AttributeError.  

### 11. debug_print()

__Purpose__   
Currently this method is not supported for NearestNeighbors. It is simply kept in NearestNeighbors
module to maintain uniform interface for all estimators in frovedis.  

__Return Value__  
It simply raises an AttributeError.  

### 12. release()  

__Purpose__  
It can be used to release the in-memory model at frovedis server.  

For example,  
 
    knn.release()
    
This will remove the trained model, model-id present on server, along with releasing server
side memory.  

__Return Value__  
It returns nothing.   

### 13. is_fitted()  

__Purpose__  
It can be used to confirm if the model is already fitted or not.  

__Return Value__  
It returns 'True', if the model is already fitted, otherwise, it returns 'False'.  

# SEE ALSO  
- **[Introduction to FrovedisRowmajorMatrix](../matrix/rowmajor_matrix.md)**  
- **[Introduction to FrovedisCRSMatrix](../matrix/crs_matrix.md)**  
- **[KNeighbors Classifier in Frovedis](./kneighborsclassifier.md)**  
- **[KNeighbors Regressor in Frovedis](./kneighborsregressor.md)**  
