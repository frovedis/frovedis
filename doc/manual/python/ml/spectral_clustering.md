% Spectral Clustering

# NAME

Spectral Clustering - A clustering algorithm commonly used in EDA (exploratory data analysis). 
It uses the spectrum (eigenvalues) of the similarity matrix of the data to perform clustering.  

# SYNOPSIS

class frovedis.mllib.cluster.SpectralClustering(n_clusters=8, eigen_solver=None, n_components=None,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ random_state=None, n_init=10, gamma=1.0,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ affinity='rbf', n_neighbors=10, eigen_tol=0.0,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ assign_labels='kmeans', degree=3, coef0=1,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ kernel_params=None, n_jobs=None, verbose=0,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ max_iter=300, eps=1e-4, norm_laplacian=True,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ mode=3, drop_first=True)  

## Public Member Functions

fit(X, y = None)  
fit_predict(X, y = None)  
score(X, y, sample_weight = None)  
load(fname, dtype = None)  
save(fname)  
debug_print()  
release()  
is_fitted()  

# DESCRIPTION
Clustering is an unsupervised learning problem where we aim to group subsets of 
entities with one another based on some notion of similarity.  

In Spectral Clustering, the data points are treated as nodes of a graph. Thus, 
clustering is treated as a graph partitioning problem. The nodes are then mapped to 
a low-dimensional space that can be easily segregated to form clusters.  

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis interface is almost same as Scikit-learn 
Spectral Clustering interface, but it doesn't have any dependency with Scikit-learn. 
It can be used simply even if the system doesn't have Scikit-learn installed. Thus, in 
this implementation, a python client can interact with a frovedis server sending the 
required python data for training at frovedis side. Python data is converted into 
frovedis compatible data internally and the python ML call is linked with the respective 
frovedis ML call to get the job done at frovedis server.  

Python side calls for Spectral Clustering on the frovedis server. Once the training is 
completed with the input data at the frovedis server, it returns an abstract model with 
a unique model ID to the client python program.  

When prediction-like request would be made on the trained model, python program will 
send the same request to the frovedis server. After the request is served at the frovedis 
server, the output would be sent back to the python client.  

## Detailed Description

### SpectralClustering()  

__Parameters__  
 **_n\_clusters_**: A positive integer parameter specifying the number of clusters. The 
number of clusters should be greater than 0 and less than or equal to n_samples. (Default: 8)  
**_eigen\_solver_**: A string object parameter. It is the eigenvalue decomposition strategy 
to use. (Default: None)  
When it is None (not specified explicitly), it will be set as 'arpack'. Only 'arpack' 
eigen solver is supported. 
**_n\_components_**: A positive integer parameter containing the number of components 
for clusters. It is used to compute the number of eigenvectors for spectral embedding. 
The number of components should be in between 1 to n_features. (Default: None)  
When it is None (not specified explicitly), it will be equal to the number of clusters.  
**_random\_state_**: Zero or positive integer parameter. It is None by default. When it is 
None (not specified explicitly), it will be set as 0. (unused)  
**_n\_init_**: A positive integer parameter is the number of times the k-means algorithm 
will be run with different centroid seeds. (Default: 10)  
**_gamma_**: The double (float64) parameter required for computing nearby relational 
meaningful eigenvalues. (Default: 1.0)  
When it is None (specified explicitly), it will be set as 1.0.  
Kernel coefficient for rbf is "[np.exp(-gamma * d(X,X) ** 2)]" kernel. Ignored for 
affinity='nearest_neighbors'.  
**_affinity_**: A string object parameter which tells how to construct the affinity matrix. (Default: 'rbf')  
When it is None (specified explicitly), it will be set as 'rbf'. Only 'rbf', 
'nearest_neighbors' and 'precomputed' are supported.  
'nearest_neighbors': construct the affinity matrix by computing a graph of nearest neighbors.  
'rbf': construct the affinity matrix using a radial basis function (RBF) kernel.  
'precomputed': interpret X as a precomputed affinity matrix, where larger values indicate greater 
similarity between instances. One of the kernels supported by pairwise_kernels.  
**_n\_neighbors_**: A positive integer parameter, is the number of neighbors to be used when 
constructing the affinity matrix using the nearest neighbors method. It must be in between 
1 to n_samples. It is applicable only when affinity = 'nearest_neighbors'. (Default: 10)  
**_eigen\_tol_**: Stopping criterion for eigen decomposition of the Laplacian matrix when 
using 'arpack' eigen_solver. (unused)  
**_assign\_labels_**: A string object parameter, is the strategy to use to assign labels in the 
embedding space. When it is None (specified explicitly), it will be set as 'kmeans'. Only 
'kmeans' is supported. (Default: 'kmeans')  
**_degree_**: Degree of the polynomial kernel. (unused)  
**_coef0_**: Zero coefficient for polynomial and sigmoid kernels. (unused)  
**_kernel\_params_**: Parameters (keyword arguments) and values for kernel. (unused)  
**_n\_jobs_**: The number of parallel jobs to run. (unused)  
**_verbose_**: An integer parameter specifying the log level to use. Its value is 0 by 
default (for INFO mode and not specified explicitly). But it can be set to 1 (for DEBUG mode) 
or 2 (for TRACE mode) for getting training time logs from frovedis server.  
**_max\_iter_**: A positive integer parameter containing the maximum number of iteration count 
for kmeans assignment. (Default: 300)  
**_eps_**: Zero or a positive double parameter containing the tolerance value for kmeans. (Default: 1e-4)  
**_norm\_laplacian_**: A boolean parameter if set to True, then compute normalized Laplacian, 
else not. (Default: True)  
**_mode_**: An integer parameter required to set the eigen computation method. It can be either 
1 (for generic) or 3 (for shift-invert). It is applicable only for dense data. For more details 
refer ARPACK computation modes. (Default: 3)  
**_drop\_first_**: A boolean parameter if set to True, then drops the first eigenvector. The 
first eigenvector of a normalized Laplacian is full of constants, thus if it is set to true, 
then (n_components + 1) eigenvectors are computed and will drop the first vector. Otherwise, 
it will calculate 'n_components' number of eigenvectors. (Default: True)  

__Attributes__  
**_affinity\_matrix\__**:  
For python native dense input:  
  - When affinity = 'precomputed/rbf', it returns a numpy array  
  - When affinity = 'nearest_neighbors', it returns a scipy matrix  
For frovedis-like dense input:  
  - When affinity = 'precomputed/rbf', returns a FrovedisRowmajorMatrix  
  - When affinity = 'nearest_neighbors', returns a FrovedisCRSMatrix  
For python native sparse input:  
  - When affinity = 'precomputed/nearest_neighbors', it returns a scipy matrix  
  - When affinity = 'rbf', it returns a numpy array  
For frovedis-like sparse input:  
  - When affinity = 'precomputed/nearest_neighbors', it a returns FrovedisCRSMatrix  
  - When affinity = 'rbf', it returns a FrovedisRowmajorMatrix  
  
In all cases, the output is of float or double (float64) type and of shape (n_samples, n_samples).  
**_labels\__**: A python ndarray of int64 values and has shape(n_clusters,). It contains 
predicted cluster labels for each point.  

__Purpose__  
It initializes a Spectral Clustering object with the given parameters.  

The parameters: "eigen_tol", "degree", "coef0", "kernel_params" and "n_jobs", "random_state" are simply kept 
in to make the interface uniform to the Scikit-learn Spectral Clustering module. They are 
not used anywhere within frovedis implementation.  

__Return Value__  
It simply returns "self" reference. 

### fit(X, y = None)
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data. If 
affinity="precomputed", it needs to be of shape (n_samples, n_samples).  
**_y_**: None or any python array-like object (any shape). It is simply ignored in frovedis
implementation, as in Scikit-learn as well.  

__Purpose__  
It clusters the given data points (X) into a predefined number (n_clusters) of clusters.   

For example,   

    # loading sample matrix data
    mat = np.loadtxt("./input/spectral_data.txt")
    
    # fitting input matrix on Spectral Clustering object
    from frovedis.mllib.cluster import SpectralClustering
    spec = SpectralClustering(n_clusters = 2).fit(mat)   

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedlis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,   

    # loading sample matrix data
    mat = np.loadtxt("./input/spectral_data.txt")
    
    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix. 
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(mat)
    
    # Spectral Clustering with pre-constructed frovedlis-like inputs
    from frovedis.mllib.cluster import SpectralClustering
    spec = SpectralClustering(n_clusters = 2).fit(rmat)

__Return Value__  
It simply returns "self" reference.  

### fit_predict(X, y = None)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data. If 
affinity="precomputed", it needs to be of shape (n_samples, n_samples).  
**_y_**: None or any python array-like object (any shape). It is simply ignored in frovedis
implementation, as in Scikit-learn as well.  

__Purpose__  
It fits the given data points (X) and returns the predicted labels based on cluster formed 
during the fit.  

For example,  

    # loading sample matrix data
    mat = np.loadtxt("./input/spectral_data.txt")
    
    # fitting input matrix on Spectral Clustering object
    from frovedis.mllib.cluster import SpectralClustering
    spec = SpectralClustering(n_clusters = 2)
    print(spec.fit_predict(mat)) 

Output

    [0 0 1 1 1]

It prints the predicted cluster lables after training is completed.  

Like in fit(), we can also provide frovedis-like input in fit_predict() for faster computation.  

For example,

    # loading sample matrix data
    mat = np.loadtxt("./input/sample_data.txt")
    
    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix. 
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(mat)
    
    # using pre-constructed input matrix
    from frovedis.mllib.cluster import SpectralClustering
    spec = SpectralClustering(n_clusters = 2)
    print(spec.fit_predict(rmat))
    
Output

    [0 0 1 1 1]

It prints the predicted cluster lables after training is completed.  

__Return Value__  
It returns a numpy array of int32 type containing the cluster labels. It has a shape(n_samples,).   

### score(X, y, sample_weight = None)  
__Parameters__   
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data. If 
affinity="precomputed", it needs to be of shape (n_samples, n_samples).  
**_y_**: A python ndarray of shape (n_samples,).  
**_sample\_weight_**: An unused parameter whose default value is None. It is simply ignored 
in frovedis implementation.  

__Purpose__  
It uses homogeneity score on given true labels and predicted labels i.e homogeneity score 
of self.predict(X, y) wrt. y.  

For example,

    spec.score(train_mat, [0, 0, 1, 1, 1])  

Output

    1.0 

__Return Value__  
It returns a homogeneity score of float type.

### load(fname, dtype = None)
__Parameters__   
**_fname_**:  A string object containing the name of the file having model information
to be loaded.  
**_dtype_**: A data-type is inferred from the input data. Currently, expected input 
data-type is either float or double(float64). (Default: None)  

__Purpose__  
It loads a spectral clustering model stored previously from the specified 
file (having little-endian binary data).  

For example,  

    spec.load("./out/MySpecClusteringModel", dtype = np.float64)

__Return Value__  
It simply returns "self" instance.   

### save(fname)
__Parameters__   
**_fname_**: A string object containing the name of the file on which the target 
model is to be saved.  

__Purpose__  
On success, it writes the model information (after-fit populated attributes) in the 
specified file as little-endian binary data. Otherwise, it throws an exception.  

For example,   

    # To save the spectral clustering model
    spec.save("./out/MySpecClusteringModel")  

This will save the spectral clustering model on the path "/out/MySpecClusteringModel".  

__Return Value__  
It returns nothing.   

### debug_print()

__Purpose__   
It shows the target model information (affinity matrix) on the server side user terminal. 
It is mainly used for debugging purpose.  

For example,  

    spec.debug_print() 
    
Output  

    affinity matrix:
    num_row = 5, num_col = 5
    node 0
    node = 0, local_num_row = 5, local_num_col = 5, val = 1 0.970446 6.2893e-104 2.92712e-106 
    1.28299e-108 0.970446 1 1.27264e-101 6.2893e-104 2.92712e-106 6.2893e-104 1.27264e-101 1 
    0.970446 0.88692 2.92712e-106 6.2893e-104 0.970446 1 0.970446 1.28299e-108 2.92712e-106 
    0.88692 0.970446 1
    labels:
    0 0 1 1 1
    ncluster: 2

__Return Value__  
It returns nothing.  

### release()  

__Purpose__  
It can be used to release the in-memory model at frovedis server.  

For example,
 
    spec.release()

This will reset the after-fit populated attributes to None, along with releasing server 
side memory.  

__Return Value__  
It returns nothing.   

### is_fitted()  

__Purpose__  
It can be used to confirm if the model is already fitted or not.  

__Return Value__  
It returns ‘True’, if the model is already fitted otherwise, it returns ‘False’.  

# SEE ALSO  
rowmajor_matrix, crs_matrix  