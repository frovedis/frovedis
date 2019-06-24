% spectral clustering

# NAME
spectral clustering - A clustering algorithm commonly used in EDA (exploratory data analysis), using the spectrum (eigenvalues) of the 
similarity matrix of the data to perform clustering.

# SYNOPSIS

class frovedis.mllib.cluster.SpectralClustering (n_clusters=8, eigen_solver=None, random_state=None, n_init=10, gamma=1.0, 
affinity='rbf', n_neighbors=10, eigen_tol=0.0, assign_labels='kmeans', degree=3, coef0=1, kernel_params=None, n_jobs=None, 
verbose=0, n_iter=100, eps=0.01, n_comp=None, norm_laplacian=True, mode=1, drop_first=False)

## Public Member Functions

fit(X, y=None)  
fit_predict(X)  
get_params()  
set_params(params)  
load(filename)  
get_affinity_matrix()  
save(filename)  
debug_print()  
release()  

# DESCRIPTION
Clustering is an unsupervised learning problem whereby we aim to group subsets of entities with one another based on some notion of similarity. 
In spectral clustering, the data points are treated as nodes of a graph. Thus, clustering is treated as a graph partitioning problem. 
The nodes are then mapped to a low-dimensional space that can be easily segregated to form clusters. The components or features are identified as per column order in matrix data. The nodes are then mapped to a low-dimensional space that can be easily segregated to form clusters.   

## Detailed Description
### SpectralClustering()

__Parameters__ 
 
__n_clusters:__ An integer parameter containing the number of required clusters.(Default: 2)  
__eigen_solver:__ The eigenvalue decomposition strategy to use. AMG requires pyamg to be installed. It can be faster on very large, sparse problems, but may also lead to instabilities(Default: None)[Internally skipped]  
__random_state:__ A pseudo random number generator used for the initialization of the lobpcg eigen vectors decomposition when eigen_solver == 'amg' and by the K-Means initialization.[Internally skipped]  
__n_init:__ An integer parameter containing the maximum number of iteration count (Default: 100)  
__gamma:__ The value required for computing nearby relational meaningful eigenvalues (Default: 1.0)  
__affinity:__ If a string, this may be one of 'nearest_neighbors', 'precomputed' according to input data.(Default: rbf)[Internally skipped]  
__n_neighbors:__ Number of neighbors to use when constructing the affinity matrix using the nearest neighbors method.[Internally skipped]  
__eigen_tol:__ Stopping criterion for eigen decomposition of the Laplacian matrix when using arpack eigen_solver.[Internally skipped]  
__assign_labels :__ The strategy to use to assign labels in the embedding space.[Internally skipped]   
__degree:__ Degree of the polynomial kernel. Ignored by other kernels.[Internally skipped]  
__coef0:__ Zero coefficient for polynomial and sigmoid kernels.[Internally skipped]  
__kernel_params:__ Parameters (keyword arguments) and values for kernel passed as callable object.[Internally skipped]  
__n_jobs:__ The number of parallel jobs to run.[Internally skipped]   
__verbose:__ An integer object specifying the log level to use. (Default: 0)  
__n_iter:__ An integer parameter containing the maximum number of iteration count for kmeans  
__eps:__ An double parameter containing the epsilon value for kmeans (Default: 0.01)  
__n_comp:__ An integer parameter containing the number of components for clusters (Default: 2)  
__norm_laplacian:__ A boolean parameter if set True, then compute normalized Laplacian else not (Default: true)  
__mode:__ A parameter required to set the eigen computation method. It can be either 1 or 3, 1 for generic and 3 for shift-invert(Default: 1)  
__drop_first:__ A boolean parameter if set True, then drops the first eigenvector. The first eigenvector of a normalized laplacian is full of constants, thus if drop_first is set true, compute (n_comp+1) eigenvectors and will drop the first vector. Otherwise it will calculate n_comp number of eigenvectors(Default: false)  


__Purpose__   

It clusters the given data points into a predefined number (n_clusters) of clusters. It simply returns "self" reference.

__Return Value__   

This is a non-blocking call. The control will return quickly, right after submitting the training request at frovedis server side with a SpectralModel object containing a array of labels.

### fit(X, y=None)
__Parameters__   

_X_: A scipy dense matrix or any python array-like object or an instance of FrovedisRowmajorMatrix.    
_y_: None (simply ignored in scikit-learn as well).    

__Purpose__    

It clusters the given data points (X) into a predefined number (n_clusters) of clusters.   

For example,   

    # loading sample Rowmajor data file
    mat = FrovedisRowmajorMatrix().load("./sample")
    
    # fitting input matrix on spectral object
    spectral = SpectralClustering(n_clusters=2, verbose=2).fit(mat)   

__Return Value__  

It simply returns "self" reference.   
Note that the call will return quickly, right after submitting the fit request 
at frovedis server side with a unique model ID for the fit request. It may be 
possible that the training is not completed at the frovedis server side even 
though the client scikit-learn side fit() returns. 

### fit_predict(X, y=None)    
__Parameters__   

_X_: A scipy dense matrix or any python array-like object or an instance of FrovedisRowmajorMatrix.    

__Purpose__    

It accepts the test data points (X) and returns the centroid information.  

__Return Value__  

It returns a numpy array of integer (int32) type containing the label values.   

### get_params()

__Purpose__  

It returns the values of parameters used for clustering.

__Return Value__  

It simply returns "self" instance.

### set_params()

__Purpose__  

It intialize the values of parameters the required for clustering.

__Return Value__  

It simply returns "self" instance.

### load(filename)
__Parameters__   

_filename_: A string object containing the name of the file having model information to be loaded.    

__Purpose__    

It loads the model from the specified file (having little-endian binary data).

__Return Value__  

It simply returns "self" instance.   

### get_affinity_matrix()

__Purpose__  

It returns the output value of the computed affinity matrix.

__Return Value__  

It returns FrovedisRowmajorMatrix instance.

### save(filename)
__Parameters__   

_filename_: A string object containing the name of the file on which the target model is to be saved.    

__Purpose__    

On success, it writes the model information in the specified file as little-endian binary data. Otherwise, it throws an exception. 

__Return Value__  

It returns nothing.   

### debug_print()

__Purpose__   

It shows the target model information on the server side user terminal. It is mainly used for debugging purpose.   

__Return Value__  

It returns nothing.  

### release()

__Purpose__   

It can be used to release the in-memory model at frovedis server. 

__Return Value__  

It returns nothing.   

