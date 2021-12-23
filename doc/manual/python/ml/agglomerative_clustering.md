% Agglomerative Clustering

# NAME

Agglomerative Clustering - The most common type of hierarchical clustering used to group objects 
in clusters based on their similarities.   

# SYNOPSIS

    class frovedis.mllib.cluster.AgglomerativeClustering(n_clusters=2, affinity='euclidean',  
                                                         memory=None, connectivity=None,  
                                                         compute_full_tree='auto', 
                                                         linkage='average',  
                                                         distance_threshold=None, 
                                                         compute_distances=False,  
                                                         verbose=0)  

## Public Member Functions

fit(X, y = None)  
fit_predict(X, y = None)  
reassign(ncluster = None)  
score(X, y, sample_weight = None)  
get_params(deep = True)  
set_params(\*\*params)  
load(fname, dtype = None)  
save(fname)  
debug_print()  
release()  
is_fitted()  


# DESCRIPTION
Clustering is a machine learning technique that involves the grouping of data points. 
Hierarchical clustering is a general family of clustering algorithms that build nested 
clusters by merging or splitting them successively. 

The Agglomerative Clustering object performs a hierarchical clustering using a bottom-up 
approach. Each observation starts in its own cluster, and clusters are successively merged 
together. 

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis interface is almost same as Scikit-learn
Agglomerative Clustering interface, but it doesn't have any dependency with Scikit-learn. 
It can be used simply even if the system doesn't have Scikit-learn installed. Thus, in
this implementation, a python client can interact with a frovedis server sending the
required python data for training at frovedis side. Python data is converted into
frovedis compatible data internally and the python ML call is linked with the respective
frovedis ML call to get the job done at frovedis server. 

Python side calls for Agglomerative Clustering on the frovedis server. Once the training is
completed with the input data at the frovedis server, it returns an abstract model with
a unique model ID to the client python program. 

When prediction-like request would be made on the trained model, python program will
send the same request to the frovedis server. After the request is served at the frovedis
server, the output would be sent back to the python client.  

## Detailed Description  

### 1. AgglomerativeClustering()   

__Parameters__  
**_n\_clusters_**: An integer parameter specifying the number of clusters. The number of
clusters should be greater than 0 and less than n_samples. (Default: 2)  
**_affinity_**: An unused parameter. (Default: 'euclidean')  
**_memory_**: An unused parameter. (Default: None)  
**_connectivity_**: An unused parameter. (Default: None)  
**_compute\_full\_tree_**: An unused parameter. (Default: 'auto')  
**_linkage_**: A string parameter used to specify linkage criterion. It determines 
which distance to use between sets of observation. The algorithm will merge the pairs of 
clusters that minimize this criterion.   
- **'average'**: uses the average of the distances of each observation of the two sets.  
- **'complete'**: linkage uses the maximum distances between all observations of the 
two sets.  
- **'single'**: uses the minimum of the distances between all observations of the two sets.  

Only 'average', 'complete' and 'single' are supported. (Default: 'average')  
**_distance\_threshold_**: A float or double(float64) type parameter, is the linkage distance
threshold above which the clusters will not be merged. It must be zero or positive value. (Default: None)  
When it is None (not specified explicitly), it will be set as 0.0.  
**_compute\_distances_**: Unlike Scikit-learn, it is alwats True for frovedis. Hence, this parameter 
is left unused. (Default: False)  
**_verbose_**: An integer parameter specifying the log level to use. Its value is 0 by 
default (for INFO mode and not specified explicitly). But it can be set to 1 (for DEBUG mode) 
or 2 (for TRACE mode) for getting training time logs from frovedis server.  

__Attributes__  
**_n\_clusters\__**: A positive integer value specifying the number of clusters found by 
the algorithm.  
**_labels\__**: A python ndarray of int64 type values and has shape **(n_clusters,)**. It contains 
cluster labels for each point.  
**_children\__**: A python ndarray of int64 type values and has shape **(n_samples - 1, 2)**. It
contains the children of each non-leaf node.  
**_distances\__**: A python ndarray of float or double(float64) values and has 
shape **(n_samples - 1,)**. It specifies the distances between nodes in the corresponding place 
in "children_".  
**_n\_connected\_components\__**: An integer value used to provide the estimated number of 
connected components in the graph.  

__Purpose__  
It initializes an Agglomerative Clustering object with the given parameters.  

The parameters: "affinity", "memory", "connectivity", "compute_full_tree" and "compute_distances" 
are simply kept in to to make the interface uniform to the Scikit-learn Agglomerative Clustering 
module. They are not used anywhere within the frovedis implementation.  

__Return Value__  
It simply returns "self" reference. 

### 2. fit(X, y = None)  
__Parameters__   
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data.  
**_y_**: None or any python array-like object (any shape). It is simply ignored in frovedis
implementation, like in Scikit-learn as well.  

__Purpose__  
It clusters the given data points (X) into a predefined number of clusters.  

For example,   

    # loading sample matrix data
    mat = np.loadtxt("./input/sample_data.txt")
    
    # fitting input matrix on AgglomerativeClustering object
    from frovedis.mllib.cluster import AgglomerativeClustering
    acm = AgglomerativeClustering(n_clusters = 2).fit(mat)  

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,   

    # loading sample matrix data
    mat = np.loadtxt("./input/sample_data.txt")
    
    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix. 
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(mat)
    
    # Agglomerative Clustering with pre-constructed frovedis-like inputs
    from frovedis.mllib.cluster import AgglomerativeClustering
    acm = AgglomerativeClustering(n_clusters = 2).fit(rmat)
    
__Return Value__  
It simply returns "self" reference.  

### 3. fit_predict(X, y = None)  
__Parameters__   
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data.  
**_y_**: None or any python array-like object (any shape). It is simply ignored in frovedis
implementation, like in Scikit-learn as well.  

__Purpose__  
It clusters the given data points (X) into a predefined number of clusters. In addition to 
fitting, it returns the cluster labels for each sample in the training set.  

For example,  

    # loading sample matrix data
    mat = np.loadtxt("./input/sample_data.txt")
    
    # fitting input matrix on AgglomerativeClustering object
    from frovedis.mllib.cluster import AgglomerativeClustering
    acm = AgglomerativeClustering(n_clusters = 2)
    print(acm.fit_predict(mat)) 

Output

    [1 1 0 0 0]

Like in fit(), frovedis-like input can be used to speed-up the trainng at server side.  

For example,

    # loading sample matrix data
    mat = np.loadtxt("./input/sample_data.txt")
    
    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix. 
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(mat)
    
    # using pre-constructed input matrix
    from frovedis.mllib.cluster import AgglomerativeClustering
    acm = AgglomerativeClustering(n_clusters = 2)
    print(acm.fit_predict(rmat))
    
Output

    [1 1 0 0 0]
    
__Return Value__  
It returns a numpy array of int64 type values containing the cluster labels. It has a shape **(n_samples,)**.  

### 4. reassign(ncluster = None)  
__Parameters__   
**_nclusters_**: An integer parameter specifying the number of clusters to be reassigned 
for the fitted data without computing the tree again. The number of clusters should be 
greater than 0 and less than n_samples. (Default: None)  
When it is None (not specified explicitly), it simply returns the same cluster labels of 
already fitted clustering model. In this case, 'ncluster' becomes the 'n_cluster' value used 
during fit().

__Purpose__    
It accepts the number of clusters (nclusters) in order to make prediction with different 
"nclusters" on same model at frovedis server.  

For example, 

    # On the same AgglomerativeClustering object, predicting labels with new nclusters
    print(acm.reassign(nclusters = 3))

Output

    [0 0 1 1 2]  

__Return Value__  
It returns a numpy array of int64 type values containing the cluster labels. It has a shape **(n_samples,)**.  

### 5. score(X, y, sample_weight = None)  
__Parameters__   
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data.  
**_y_**: A python ndarray or an instance of FrovedisVector containing the true labels for 
X. It has shape **(n_samples, 1)**.  
**_sample\_weight_**: An unused parameter whose default value is None. It is simply ignored 
in frovedis implementation.  

__Purpose__  
It uses homogeneity score on given test data and labels i.e homogeneity score of 
self.predict(X, y) wrt. y.  

For example,

    acm.score(train_mat, [0, 0, 1, 1 1]) 

Output

    1.0  

__Return Value__  
It returns a homogeneity score of float type.

### 6. get_params(deep = True)  
__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If True, 
it will return the parameters for an estimator and contained subobjects that are estimators. (Default: True)  

__Purpose__    
This method belongs to the BaseEstimator class inherited by AgglomerativeClustering. It is used to get 
parameters and their values of AgglomerativeClustering class.  

For example, 
 
    print(acm.get_params())

Output  

    {'affinity': 'euclidean', 'compute_distances': True, 'compute_full_tree': 'auto',  
    'connectivity': None, 'distance_threshold': None, 'linkage': 'average', 'memory': None,
    'n_clusters': 3, 'verbose': 0}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 7. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed to this function as dictionary. This dictionary 
contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by AgglomerativeClustering, used to set 
parameter values.  

For example,   

    print("get parameters before setting:") 
    print(acm.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    acm.set_params(n_clusters = 4) 
    print("get parameters after setting:") 
    print(acm.get_params())

Output  
     
    get parameters before setting:
    {'affinity': 'euclidean', 'compute_distances': True, 'compute_full_tree': 'auto',  
    'connectivity': None, 'distance_threshold': None, 'linkage': 'average', 'memory': None,
    'n_clusters': 3, 'verbose': 0}
    get parameters after setting:
    {'affinity': 'euclidean', 'compute_distances': True, 'compute_full_tree': 'auto',  
    'connectivity': None, 'distance_threshold': None, 'linkage': 'average', 'memory': None,
    'n_clusters': 4, 'verbose': 0}
    
__Return Value__  
It simply returns "self" reference.  

### 8. load(fname, dtype = None)  
__Parameters__   
**_fname_**:  A string object containing the name of the file having model information
to be loaded.  
**_dtype_**: A data-type is inferred from the input data. Currently, expected input data-type 
is either float or double (float64). (Default: None)

__Purpose__    
It loads an agglomerative clustering model stored previously from the specified 
file (having little-endian binary data).  

For example,   

    acm.load("./out/MyAcmClusteringModel", dtype = np.float64)

__Return Value__  
It simply returns "self" reference.  

### 9. save(fname)  
__Parameters__  
**_fname_**: A string object containing the name of the file on which the target 
model is to be saved.  

__Purpose__  
On success, it writes the model information (metadata and model) in the 
specified file as little-endian binary data. Otherwise, it throws an exception.  

For example,   

    # To save the agglomerative clustering model
    acm.save("./out/MyAcmClusteringModel")  

This will save the agglomerative clustering model on the path "/out/MyAcmClusteringModel".  
It would raise exception if the directory already exists with same name.  

The 'MyAcmClusteringModel' directory has  

**MyAcmClusteringModel**  
|-------- metadata  
|-------- **model**  

The metadata file contains the number of clusters, number of samples, model kind, input datatype 
used for trained model.  
Here, the **model** directory contains information about dendogram.  

__Return Value__  
It returns nothing.

### 10. debug_print()  

__Purpose__  
It shows the target model information(dendogram) on the server side user terminal. 
It is mainly used for debugging purpose.  

For example,  

    acm.debug_print() 
    
Output  

    --- dendrogram ---
            X       Y       distance        size
    5:      2       3       0.173205        2
    6:      0       1       0.173205        2
    7:      4       5       0.259808        3
    8:      6       7       15.5019 5

It displays the dendrogram on the trained model which is currently present on the server. 
Using the dendrogram, the desired number of clusters may be found. 

__Return Value__  
It returns nothing.  

### 11. release()  

__Purpose__  
It can be used to release the in-memory model at frovedis server.  

For example,
 
    acm.release()

This will reset the after-fit populated attributes to None, along with releasing server 
side memory.  

__Return Value__  
It returns nothing.  

### 12. is_fitted()  

__Purpose__  
It can be used to confirm if the model is already fitted or not. In case, reassign() is used 
before training the model, then it can prompt the user to train the clustering model first.  

__Return Value__  
It returns 'True', if the model is already fitted otherwise, it returns 'False'.  

# SEE ALSO  
spectral_clustering, dbscan, kmeans, rowmajor_matrix, crs_matrix  