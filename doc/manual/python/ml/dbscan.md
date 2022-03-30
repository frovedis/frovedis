% DBSCAN

# NAME

DBSCAN clustering - Density-Based Spatial Clustering of Applications with 
Noise (DBSCAN) is a clustering algorithm commonly used in EDA (exploratory 
data analysis). It is a base algorithm for density-based clustering, which 
can discover clusters of different shapes and sizes from a large amount of 
data, containing noise and outliers.

# SYNOPSIS

    class frovedis.mllib.cluster.DBSCAN(eps=0.5, min_samples=5, metric=‘euclidean’,  
                                        metric_params=None, algorithm=‘auto’,  
                                        leaf_size=30, p=None, n_jobs=None,  
                                        batch_fraction=None, verbose=0)  

## Public Member Functions

fit(X, y = None, sample_weight = None)  
fit_predict(X, sample_weight = None)  
score(X, y, sample_weight = None)  
get_params(deep = True)  
set_params(\*\*params)  
release()  
is_fitted()  

# DESCRIPTION

DBSCAN is a Density-Based Clustering unsupervised learning algorithm that identifies 
distinctive groups/clusters in the data, based on the idea that a cluster in 
data space is a contiguous region of high point-density, separated from other 
such clusters by contiguous regions of low point-density.

It is able to find arbitrary shaped clusters and clusters with noise (i.e. 
outliers). The main idea behind DBSCAN is that a point belongs to a cluster if 
it is close to many points from that cluster.  
DBSCAN algorithm requires two key parameters: **eps** and **min_samples**.  

Based on these two parameters, the points are classified as core point, border point, 
or outlier:  
- **Core point**: A point is a core point if there are at least minPts number of points 
(including the point itself) in its surrounding area with radius eps.  
- **Border point**: A point is a border point if it is reachable from a core point 
and there are less than minPts number of points within its surrounding area.  
- **Outlier**: A point is an outlier if it is not a core point and not reachable 
from any core points.  

The main advantage of using DBSCAN is that it is able to find arbitrarily size and 
arbitrarily shaped clusters and identify noise data while clustering.  

This module provides a client-server implementation, where the client application is 
a normal python program. Frovedis is almost same as Scikit-learn clustering module 
providing DBSCAN support, but it doesn’t have any dependency with Scikit-learn. It 
can be used simply even if the system doesn’t have Scikit-learn installed. Thus, in 
this implementation, a python client can interact with a frovedis server sending the 
required python data for training at frovedis side. Python data is converted into 
frovedis compatible data internally and the python ML call is linked with the 
respective frovedis ML call to get the job done at frovedis server.  

Python side calls for DBSCAN on the frovedis server. Once the training is completed 
with the input data at the frovedis server, it returns an abstract model with a 
unique model ID to the client python program.  

When prediction-like request would be made on the trained model, the python program 
will send the same request to the frovedis server. After the request is served at 
the frovedis server, the output would be sent back to the python client.

## Detailed Description  

### 1. DBSCAN()  

__Parameters__  

_**eps**_: A positive double (float64) parameter containing the epsilon value or distance 
that specifies the neighborhoods. (Default: 0.5)  
Two points are considered to be neighbors if the distance between them is less than or equal to eps.  
_**min_samples**_: A positive integer parameter which specifies the number of samples in a 
neighborhood for a point to be considered as a core point.  
This includes the point itself. (Default: 5)  
_**metric**_: A string object parameter used when calculating distance between instances 
in a feature array. (Default: 'euclidean')  
It only supports 'euclidean' distance.  
_**metric_params**_: It is unused parameter which is an additional keyword arguments for the 
metric function. (Default: None)  
_**algorithm**_:  A string object parameter, used to compute pointwise distances and find nearest 
neighbors. (Default: 'auto')  
When it is 'auto', it will be set as 'brute'(brute-force search approach). Unlike Scikit-learn, 
currently it supports only 'brute'.  
_**leaf_size**_: An unused parameter, which is used to affect the speed of the 
construction and query. (Default: 30)  
_**p**_: An unused parameter specifying the power of the Minkowski metric to be 
used to calculate distance between points. (Default: None)  
_**n_jobs**_:  An unused parameter specifying the number of parallel jobs to run. (Default: None)  
_**batch_fraction**_: A positive double (float64) parameter used to calculate the batches of specific 
size. These batches are used to construct the distance matrix. (Default: None)  
It must be within the range of 0.0 to 1.0. When it is None (not specified explicitly), 
it will be set as np.finfo(np.float64).max value.  
_**verbose**_: An integer parameter specifying the log level to use. Its value is set 
as 0 by default (for INFO mode). But it can be set to 1 (for DEBUG mode) or 2 (for TRACE 
mode) for getting training time logs from frovedis server.  

__Attributes__  
_**labels\_**_: It is a python ndarray, containing int64 typed values and has shape 
**(n_samples,)**. These are the coordinates of cluster centers.  
_**core_sample_indices\_**_:  It is a python ndarray, containing int32 or int64 typed values
and has shape **(n_samples,)**.  These are the core samples indices.  
_**components\_**_:  It is a numpy array or FrovedisRowmajorMatrix, containing float 
or double (float64) typed values and has shape **(n_samples, n_features)**.  
These are the copy of each core sample found by training.

__Purpose__  
It initializes a DBSCAN object with the given parameters.   

The parameters: "metric_params", "leaf_size", "p" and "n_jobs" are simply kept to make 
the interface uniform to Scikit-learn DBSCAN clustering module. They are not used 
anywhere within frovedis implementation.  

__Return Value__    
It simply returns "self" reference. 

### 2. fit(X,  y = None, sample_weight = None)  
__Parameters__   
_**X**_: A numpy dense or scipy sparse matrix or any python array-like object or
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for 
dense data of float or double (float64) type. It has shape **(n_samples, n_features)**.   
_**y**_: None or any python array-like object (any shape). It is simply ignored 
in frovedis implementation, like in Scikit-learn.  
_**sample\_weight**_: Python array-like containing the intended weights for each input 
samples and it should be the shape of **(n_samples, )**.  
When it is None (not specified explicitly), an uniform weight vector is assigned on 
each input sample.  

__Purpose__    

It clusters the given data points (X). 

For example,   

    # loading sample matrix data
    train_mat = np.loadtxt("sample_data.txt")
    
    # fitting input matrix on DBSCAN object
    from frovedis.mllib.cluster import DBSCAN
    dbm = DBSCAN(eps = 5, min_samples = 2).fit(train_mat)   

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.   

For example,   

    # loading sample matrix data
    train_mat = np.loadtxt("sample_data.txt")
    
    # Since "train_mat" is numpy dense data, we have created FrovedisRowmajorMatrix.
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.   
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(train_mat)
    
    # DBSCAN with pre-constructed Frovedis-like inputs
    from frovedis.mllib.cluster import DBSCAN
    dbm = DBSCAN(eps = 5, min_samples = 2).fit(rmat)
    
__Return Value__  
It simply returns "self" reference.  

### 3. fit_predict(X, sample_weight = None)  
__Parameters__   
_**X**_: A numpy dense or scipy sparse matrix or any python array-like object or
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for 
dense data of float or double (float64) type. It has shape **(n_samples, n_features)**.   
_**sample\_weight**_: Python array-like containing the intended weights for each input 
samples and it should be the shape of **(n_samples, )**.  
When it is None (not specified explicitly), an uniform weight vector is assigned on 
each input sample.  

__Purpose__    

It computes the clusters from the given data points (X) or distance matrix and predict 
labels.  

For example,   

    # loading sample matrix data
    train_mat = np.loadtxt("sample_data.txt")
    
    # fitting input matrix on DBSCAN object
    from frovedis.mllib.cluster import DBSCAN
    dbm = DBSCAN(eps = 5, min_samples = 2)
    print(dbm.fit_predict(train_mat))    
    
Output

    [0 0 1 1 1]
    
Like in fit() frovedis-like input can be used to speed-up training at server side.  

For example,   

    # loading sample matrix data
    train_mat = np.loadtxt("sample_data.txt")
    
    # Since "train_mat" is numpy dense data, we have created FrovedisRowmajorMatrix.
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.   
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(train_mat)

    # using pre-constructed input matrix
    from frovedis.mllib.cluster import DBSCAN
    dbm = DBSCAN(eps = 5, min_samples = 2)
    print(dbm.fit_predict(rmat))    
    
Output

    [0 0 1 1 1]

__Return Value__  
It returns a numpy array of int64 type containing the cluster labels,
with shape **(n_samples,)**. 

### 4. score(X, y, sample_weight = None)  
_**X**_: A numpy dense or scipy sparse matrix or any python array-like object or
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for 
dense data of float or double (float64) type. It has shape **(n_samples, n_features)**.  
_**y**_: A python ndarray or an instance of FrovedisVector containing the true 
labels for X. It has shape **(n_samples,)**.  
_**sample\_weight**_: An unused parameter whose default value is None. It is simply 
ignored in frovedis implementation, like in Scikit-learn as well.  

__Purpose__  
It calculates homogeneity score on given test data and labels i.e homogeneity score 
of self.predict(X, y) wrt. y.

For example,

    dbm.score(train_mat, [0 0 1 1 1])

Output

    1.0

__Return Value__  
It returns a score of float type.


### 5. get_params(deep = True)  

__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If True, will return the 
parameters for an estimator and contained subobjects that are estimators. (Default: True)

__Purpose__    
This method belongs to the BaseEstimator class inherited by DBSCAN. It is used to get parameters and their values of 
DBSCAN class.  

For example, 
 
    print(dbm.get_params())

Output  

    {'algorithm': 'auto', 'batch_fraction': None, 'eps': 5, 'leaf_size': 30, 
    'metric': 'euclidean', 'metric_params': None, 'min_samples': 2, 'n_jobs': None, 
    'p': None, 'verbose': 0}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 6. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed to this function as dictionary. This dictionary 
contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by DBSCAN, used to set parameter values.  

For example,   

    print("Get parameters before setting:") 
    print(kmeans.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    kmeans.set_params(n_clusters=4, n_init=5)
    print("Get parameters after setting:") 
    print(kmeans.get_params())

Output  
     
    Get parameters before setting: 
    {'algorithm': 'auto', 'copy_x': True, 'init':'random', 'max_iter': 300, 
    'n_clusters': 2, 'n_init': 1, 'n_jobs': 1, 'precompute_distances': 'auto',
    'random_state': None, 'tol': 0.0001, 'use_shrink': False, 'verbose': 0}
    Get parameters after setting: 
    {'algorithm': 'auto', 'copy_x': True, 'init': 'random', 'max_iter': 300, 
    'n_clusters': 4, 'n_init': 5, 'n_jobs': 1, 'precompute_distances': 'auto', 
    'random_state': None, 'tol': 0.0001, 'use_shrink': False, 'verbose': 0}

__Return Value__  
It simply returns "self" reference.  


### 7. release()

__Purpose__    
It can be used to release the in-memory model at frovedis server.   

For example,  

    dbm.release()
    
This will reset the after-fit populated attributes to None, along with 
releasing server side memory.  

__Return Value__  
It returns nothing.   

### 8. is_fitted()  

__Purpose__  
It can be used to confirm if the model is already fitted or not. In case, fit_predict() 
is used before training the model, then it can prompt the user to train the clustering 
model first.

__Return Value__  
It returns ‘True’, if the model is already fitted otherwise, it returns ‘False’.  

# SEE ALSO  
- **[Introduction to FrovedisRowmajorMatrix](../matrix/rowmajor_matrix.md)**  
- **[Introduction to FrovedisCRSMatrix](../matrix/crs_matrix.md)**  
- **[Agglomerative Clustering in Frovedis](./agglomerative_clustering.md)**  
- **[Spectral Clustering in Frovedis](./spectral_clustering.md)**  
- **[KMeans in Frovedis](./kmeans.md)**  
