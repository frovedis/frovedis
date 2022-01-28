% KMeans Clustering

# NAME

KMeans - is a clustering algorithm commonly used in EDA 
(exploratory data analysis).    

# SYNOPSIS

    class frovedis.mllib.cluster.KMeans(n_clusters=8, init='random', n_init=10, max_iter=300,  
                                        tol=1e-4, precompute_distances='auto', verbose=0,   
                                        random_state=None, copy_x=True, n_jobs=1,  
                                        algorithm='auto', use_shrink=False)  

## Public Member Functions

fit(X, y = None, sample_weight = None)  
fit_predict(X, y = None, sample_weight = None)  
fit_transform(X, y = None, sample_weight = None)  
transform(X)  
predict(X, sample_weight = None)  
score(X, y = None, sample_weight = None)  
load(fname, dtype = None)  
save(fname)  
get_params(deep = True)  
set_params(\*\*params)  
debug_print()  
release()  
is_fitted()  

# DESCRIPTION

Clustering is an unsupervised learning problem whereby we aim to group subsets 
of entities with one another based on some notion of similarity. Kmeans is one 
of the most commonly used clustering algorithms that clusters the data points 
into a predefined number of clusters.  

Under unsupervised learning, there are two clustering methods- 'k-means' and 
'k-means++'. The main difference between these two lies in the selection of
the centroids (we assume centroid is the center of the cluster) around which the 
clustering takes place.  

**Frovedis supports only k-means clustering method (i.e. init ='random')** which will 
randomly initialize the data points called centroid. Further, each data point is 
clustered to its nearest centroid and after every iteration the centroid is updated 
for each cluster. This cycle continues for a given number of repetitions and after 
that we have our final clusters.

This module provides a client-server implementation, where the client application 
is a normal python program. Frovedis is almost same as Scikit-learn clustering
module providing kmeans support, but it doesn’t have any dependency with 
Scikit-learn. It can be used simply even if the system doesn’t have Scikit-learn 
installed. Thus, in this implementation, a python client can interact with a 
frovedis server sending the required python data for training at frovedis side. 
Python data is converted into frovedis compatible data internally and the python 
ML call is linked with the respective frovedis ML call to get the job done at 
frovedis server.  

Python side calls for Kmeans on the frovedis server. Once the training is completed 
with the input data at the frovedis server, it returns an abstract model with a 
unique model ID to the client python program. 

When prediction-like request would be made on the trained model, the python program will 
send the same request to the frovedis server. After the request is served at the 
frovedis server, the output would be sent back to the python client.


## Detailed Description  

### 1. KMeans()  

__Parameters__  
_**n\_clusters**_: An integer parameter specifying the number of clusters. The number 
of clusters should be greater than zero and less than n_samples. (Default: 8)  
When it is None (specified explicitly), then it will be set as min(8, n_samples).  
_**init**_: A string object parameter specifies the method of initialization. (Default: 'random')  
Unlike Scikit-learn, currently it only supports 'random' initialization.  
_**n\_init**_: A positive integer specifying the number of times the kmeans algorithm 
will be run with different centroid seeds. (Default: 10)  
When it is None (specified explicitly), then it will be set as default 10.  
_**max\_iter**_: A positive integer parameter specifying the maximum iteration count. 
(Default: 300)   
_**tol**_: Zero or a positive double (float64) parameter specifying the convergence tolerance. (Default: 1e-4)    
_**precompute\_distances**_: A string object parameter. (unused)   
_**verbose**_: An integer parameter specifying the log level to use. Its value is set 
as 0 by default (for INFO mode). But it can be set to 1 (for DEBUG mode) or 2 (for TRACE 
mode) for getting training time logs from frovedis server.  
_**random\_state**_: A zero or positive integer parameter. When it is None (not specified explicitly), 
it will be set as 0. (unused)  
_**copy\_x**_: A boolean parameter. (unused)  
_**n\_jobs**_: An integer parameter. (unused)   
_**algorithm**_: A string object parameter, specifies the kmeans algorithm to use. (Default: auto)  
When it is 'auto', it will be set as 'full'. Unlike Scikit-learn, currently it supports only 'full'.  
_**use_shrink**_: A boolean parameter applicable only for "sparse" input (X). When set 
to True for sparse input, it can improve training performance by reducing communication 
overhead across participating processes. (Default: False)  

__Attribute__  
_**cluster\_centers\_**_: It is a python ndarray, containing float or double (float64) typed 
values and has shape **(n_clusters, n_features)**. These are the coordinates of cluster centers.  
_**labels\_**_: A python ndarray of int64 values and has shape **(n_clusters,)**. It contains predicted cluster 
labels for each point.  
_**inertia\_**_: A float parameter specifies the sum of squared distances of samples to their closest 
cluster center, weighted by the sample weights if provided.  
_**n_iter\_**_:  An integer parameter specifies the number of iterations to run.  

__Purpose__    
It initializes a Kmeans object with the given parameters.   

The parameters: "precompute_distances", "random_state", "copy_x" and "n_jobs" are simply kept to make 
the interface uniform to Scikit-learn cluster module. They are not used anywhere within 
frovedis implementation.  

__Return Value__    
It simply returns "self" reference. 

### 2. fit(X,  y = None, sample_weight = None)  
__Parameters__   
_**X**_: A numpy dense or scipy sparse matrix or any python array-like object or
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for 
dense data of float or double (float64) type. It has shape **(n_samples, n_features)**.   
_**y**_: None or any python array-like object (any shape). It is simply ignored 
in frovedis implementation, like in Scikit-learn.  
_**sample\_weight**_: An unused parameter whose default value is None. 
It is simply ignored in frovedis implementation, like in Scikit-learn.  

__Purpose__    
It clusters the given data points (X) into a predefined number of clusters (n_clusters).   

For example,   

    # loading sample matrix data
    train_mat = np.loadtxt("sample_data.txt")
    
    # fitting input matrix on kmeans object
    from frovedis.mllib.cluster import KMeans
    kmeans = KMeans(n_clusters = 2, n_init = 1).fit(train_mat)   

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
    
    # KMeans with pre-constructed frovedis-like inputs
    from frovedis.mllib.cluster import KMeans
    kmeans = KMeans(n_clusters = 2, n_init = 1).fit(rmat)
    
__Return Value__  
It simply returns "self" reference.  

### 3. fit_predict(X, y = None, sample_weight = None)  
__Parameters__   
_**X**_: A numpy dense or scipy sparse matrix or any python array-like object or
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for 
dense data of float or double (float64) type. It has shape **(n_samples, n_features)**.   
_**y**_: None or any python array-like object (any shape). It is simply ignored 
in frovedis implementation, like in Scikit-learn.  
_**sample\_weight**_: An unused parameter whose default value is None. It is 
simply ignored in frovedis implementation.  

__Purpose__    
It clusters the given data points (X) into a predefined number of clusters (n_clusters)
and predicts the cluster index for each sample.  

For example,   

    # loading sample matrix data
    train_mat = np.loadtxt("sample_data.txt")
    
    # fitting input matrix on KMeans object
    from frovedis.mllib.cluster import KMeans
    kmeans = KMeans(n_clusters = 2, n_init = 1)
    print(kmeans.fit_predict(train_mat))    
    
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
    from frovedis.mllib.cluster import KMeans
    kmeans = KMeans(n_clusters = 2, n_init = 1)
    print(kmeans.fit_predict(rmat))
    
Output

    [0 0 1 1 1]

__Return Value__  
It returns a numpy array of int64 type containing the cluster labels. 
It has a shape **(n_samples,)**.   

### 4. fit_transform(X, y = None, sample_weight = None)  
__Parameters__   
_**X**_: A numpy dense or scipy sparse matrix or any python array-like object or
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for 
dense data of float or double (float64) type. It has shape **(n_samples, n_features)**.  
_**y**_: None or any python array-like object (any shape). It is simply ignored 
in frovedis implementation, like in Scikit-learn.  
_**sample\_weight**_: An unused parameter whose default value is None and
simply ignored in frovedis implementation.  

__Purpose__    
It computes clustering and transforms training data (X) to cluster-distance space.  

For example,   

    # loading sample matrix data
    train_mat = np.loadtxt("sample_data.txt")
    
    # fitting input matrix on KMeans object
    from frovedis.mllib.cluster import KMeans
    kmeans = KMeans(n_clusters = 2, n_init = 1)
    print(kmeans.fit_transform(train_mat))
    
Output

    [[ 0.08660254 15.58845727]
    [ 0.08660254 15.41525219]
    [15.32864965  0.17320508]
    [15.50185473  0.        ]
    [15.67505981  0.17320508]]

If training data (X) is a numpy array or a scipy sparse matrix, it will return a
new numpy dense array.  

Like in fit() frovedis-like input can be used to speed-up training at server side.   

For example,   

    # loading sample matrix data
    train_mat = np.loadtxt("sample_data.txt")
    
    # Since "train_mat" is numpy dense data, we have created FrovedisRowmajorMatrix.
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(train_mat)

    # using pre-constructed input matrix
    from frovedis.mllib.cluster import KMeans
    kmeans = KMeans(n_clusters = 2, n_init = 1)
    # it returns a FrovedisRowmajorMatrix object
    kmeans.fit_transform(rmat)
    
If training data (X) is a frovedis-like input, it will return a FrovedisRowmajorMatrix 
object.  

__Return Value__  
- **When training data is native-python data**:  
Then the output will be a numpy array containing the transformed matrix.  
- **When training data is frovedis-like data**:  
Then the output will be a FrovedisRowmajorMatrix.  

In both cases output would be of float or double (float64) type (depending upon input dtype) and of 
shape **(n_samples, n_clusters)**.  

Note that even if training data (X) is sparse, the output would typically be dense.  

### 5. transform(X)  
__Parameters__   
_**X**_: A numpy dense or scipy sparse matrix or any python array-like object or
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for 
dense data of float or double (float64) type. It has shape **(n_samples, n_features)**.

__Purpose__    
It transforms the test data (X) to a cluster-distance space.

For example,   

    # loading sample matrix data
    test_mat = np.loadtxt("sample_data.txt")
    
    # fitting input matrix on kmeans object
    from frovedis.mllib.cluster import KMeans
    kmeans = KMeans(n_clusters = 2, n_init = 1).fit(test_mat)
    print(kmeans.transform(test_mat))    
    
Output

    [[ 0.08660254 15.58845727]
    [ 0.08660254 15.41525219]
    [15.32864965  0.17320508]
    [15.50185473  0.        ]
    [15.67505981  0.17320508]]

If test data (X) is a numpy array or a scipy sparse matrix, it  will return a new numpy 
dense array.  

Like in fit() frovedis-like input can be used to speed-up training the test data at server side.   

For example,   

    # loading sample matrix data
    test_mat = np.loadtxt("sample_data.txt")
    
    # Since "test_mat" is numpy dense data, we have created FrovedisRowmajorMatrix.
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.   
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    tr_mat = FrovedisRowmajorMatrix(test_mat)

    # using pre-constructed input matrix
    from frovedis.mllib.cluster import KMeans
    kmeans = KMeans(n_clusters = 2, n_init = 1).fit(tr_mat)
    kmeans.transform(tr_mat)

If test data (X) is a frovedis-like input, it  will return FrovedisRowmajorMatrix object.  

__Return Value__  
- **If native-python data is input:**  
Then it returns a numpy array containing the transformed matrix.  
- **If frovedis-like data is input:**  
Then it returns a FrovedisRowmajorMatrix.  

In both cases output would be of float or double (float64) type (depending upon input dtype) 
and of shape **(n_samples, n_clusters)**.  

Note that even if test data (X) is sparse, the output would typically be dense.  

### 6. predict(X, sample_weight = None)  
__Parameters__   
_**X**_: A numpy dense or scipy sparse matrix or any python array-like object or
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for 
dense data of float or double (float64) type. It has shape **(n_samples, n_features)**.  
_**sample\_weight**_: None or any python array-like object containing the 
intended weights for each input samples. It is simply ignored in frovedis 
implementation, like in Scikit-learn.  

__Purpose__  
It accepts the test data points (X) and returns the closest cluster each sample in X belongs to.  

For example,   

    # loading sample matrix data
    test_mat = np.loadtxt("sample_data.txt")
        
    # fitting input matrix on KMeans object
    from frovedis.mllib.cluster import KMeans
    kmeans = KMeans(n_clusters = 2, n_init = 1).fit(test_mat)
    print(kmeans.predict(test_mat))    
    
Output

    [0 0 1 1 1]

Like in fit() frovedis-like input can be used to speed-up prediction on the test data at server side.  

For example,   

    # loading sample matrix data
    test_mat = np.loadtxt("sample_data.txt")
    
    # Since "test_mat" is numpy dense data, we have created FrovedisRowmajorMatrix.
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    tr_mat = FrovedisRowmajorMatrix(test_mat)
    
    # using pre-constructed input matrix
    from frovedis.mllib.cluster import KMeans
    kmeans = KMeans(n_clusters = 2, n_init = 1).fit(tr_mat)
    print(kmeans.predict(tr_mat))
    
Output  

    [0 0 1 1 1]
    
__Return Value__  
It returns a numpy array of int32 type containing the centroid values. It has a 
shape **(n_samples,)**.  

### 7. score(X, y = None, sample_weight = None)  
__Parameters__   
_**X**_: A numpy dense or scipy sparse matrix or any python array-like object or
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for 
dense data of float or double (float64) type. It has shape **(n_samples, n_features)**.  
_**y**_: None or any python array-like object (any shape). It is simply ignored 
in frovedis implementation, like in Scikit-learn.  
_**sample\_weight**_: None or any python array-like object containing the 
intended weights for each input samples. It is simply ignored in frovedis 
implementation, like in Scikit-learn.  

__Purpose__  
It is calculated as "-1.0 * inertia", which an indication of how far the points 
are from the centroids. Bad scores will return a large negative number, whereas 
good scores return a value close to zero.  

For example,

    kmeans.score(test_mat)

Output

    -0.07499999552965164

__Return Value__  
It returns a score of float type.


### 8. load(fname, dtype = None)
__Parameters__   
_**fname**_: A string object containing the name of the file having model 
information to be loaded.    
_**dtype**_: A data-type is inferred from the input data. Currently, 
expected input data-type is either float or double (float64). (Default: None)  

__Purpose__    
It loads the model from the specified file(having little-endian binary data). 

For example,  

    # loading the same model
    kmeans.load("./out/MyKMeansModel",dtype=np.float64)

__Return Value__  
It simply returns "self" instance.   

### 9. save(fname)
__Parameters__   
_**fname**_: A string object containing the name of the file on which the target 
model is to be saved.    

__Purpose__    
On success, it writes the model information(metadata and model) in the specified 
file as little-endian binary data. Otherwise, it throws an exception. 

For example,   

    # saving the model
    kmeans.save("./out/MyKMeansModel")
    
The MyKMeansModel contains below directory structure:  
**MyKMeansModel**  
    |------metadata  
    |------**model**    

'metadata' represents the detail about model_id, model_kind and datatype of training vector.  
Here, the **model** directory contains information about n_clusters_, n_features, model_kind and datatype of training vector.  

This will save the Kmeans model on the path ‘/out/MyKMeansModel’. It would raise exception if the directory already 
exists with same name.  

__Return Value__  
It returns nothing.   

### 10. get_params(deep = True)  
__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If True, will return the 
parameters for an estimator and contained subobjects that are estimators. (Default: True)

__Purpose__    
This method belongs to the BaseEstimator class inherited by Kmeans. It is used to get parameters and their values of 
Kmeans class.  

For example, 
 
    print(kmeans.get_params())

Output  

    {'algorithm': 'auto', 'copy_x': True, 'init': 'random', 'max_iter': 300, 'n_clusters': 2, 
    'n_init': 1, 'n_jobs': 1, 'precompute_distances': 'auto', 'random_state': None, 
    'tol': 0.0001, 'use_shrink': False, 'verbose': 0}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 11. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed this function as dictionary. This dictionary 
contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by Kmeans, used to set parameter values.  

For example,   

    print("Get parameters before setting:") 
    print(kmeans.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user    
    kmeans.set_params(n_clusters = 4, n_init = 5)
    print("Get parameters after setting:") 
    print(kmeans.get_params())

Output  
     
    Get parameters before setting: 
    {'algorithm': 'auto', 'copy_x': True, 'init': 'random', 'max_iter': 300, 
    'n_clusters': 2, 'n_init': 1, 'n_jobs': 1,'precompute_distances': 'auto', 
    'random_state': None,'tol': 0.0001,'use_shrink': False, 'verbose': 0}
    Get parameters after setting: {'algorithm': 'auto', 'copy_x': True, 
    'init': 'random', 'max_iter': 300, 'n_clusters': 4, 'n_init': 5, 
    'n_jobs': 1, 'precompute_distances': 'auto', 'random_state': None, 
    'tol': 0.0001,'use_shrink': False, 'verbose': 0}

__Return Value__  
It simply returns "self" reference.  

### 12. debug_print()

__Purpose__  
It shows the target model information on the server side user terminal. 
It is mainly used for debugging purpose.   

For example,  

    kmeans.debug_print()
    
Output

    centroid:
    node = 0, local_num_row = 2, local_num_col = 3, val = 0.15 0.15 0.15 9.1 9.1 9.1
    
This output will be visible on server side. It displays the centroid information on the trained 
model which is currently present on the server.  

**No such output will be visible on client side.**  

__Return Value__  
It returns nothing.   

### 13. release()

__Purpose__    
It can be used to release the in-memory model at frovedis server.   

For example,  

    kmeans.release()
    
This will reset the after-fit populated attributes to None, along with 
releasing server side memory.  

__Return Value__  
It returns nothing.   

### 14. is_fitted()  

__Purpose__  
It can be used to confirm if the model is already fitted or not. In case, predict() 
is used before training the model, then it can prompt the user to train the clustering 
model first.  

__Return Value__  
It returns 'True', if the model is already fitted otherwise, it returns 'False'.  

## SEE ALSO  
rowmajor_matrix, crs_matrix, agglomerative_clustering, spectral_clustering, dbscan
