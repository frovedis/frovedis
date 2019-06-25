% spectral embedding

# NAME
spectral embedding - Spectral embedding is the accurate method for extraction of meaningful patterns in high dimensional data. It forms an affinity matrix given by the specified function and applies spectral decomposition to the corresponding graph laplacian. The resulting transformation is given by the value of the normalized eigenvectors for each data point.

# SYNOPSIS

class frovedis.mllib.cluster.SpectralEmbedding (n_components=2, affinity='rbf', gamma=1.0, random_state=None, eigen_solver=None, n_neighbors=None, n_jobs=None, verbose=0, norm_laplacian=True, mode=1, drop_first=True)

## Public Member Functions

fit(X, y=None)  
get_params()  
set_params(params)  
get_affinity_matrix()   
get_embedding_matrix()  
load(filename)  
save(filename)  
debug_print()  
release()  

# DESCRIPTION
Spectral embedding is particularly useful for reducing the dimensionality of data that is expected to lie on a low-dimensional manifold contained within a high-dimensional space, it yields a low-dimensional representation of the data that best preserves the structure of the original manifold in the sense that points that are close to each other on the original manifold will also be close after embedding. At the same time, the embedding emphasizes clusters in the original data.
   

## Detailed Description
### SpectralEmbedding()

__Parameters__   

__n_components:__ An integer parameter containing the  number of component count (Default: 2)  
__affinity:__ If a string, this may be one of 'nearest_neighbors', 'precomputed' according to input data.(Default: rbf)[Internally skipped]  
__gamma:__ The value required for computing nearby relational meaningful eigenvalues(Default: 1.0)  
__random_state:__ A pseudo random number generator used for the initialization of the lobpcg eigen vectors decomposition when eigen_solver == 'amg' and by the K-Means initialization.[Internally skipped]  
__eigen_solver:__ The eigenvalue decomposition strategy to use. AMG requires pyamg to be installed. It can be faster on very large, sparse problems, but may also lead to instabilities(Default: None)[Internally skipped]   
__n_neighbors:__ Number of neighbors to use when constructing the affinity matrix using the nearest neighbors method.[Internally skipped]   
__n_jobs:__ The number of parallel jobs to run.[Internally skipped]   
__verbose:__ An integer object specifying the log level to use. (Default: 0)  
__norm_laplacian:__ A boolean parameter if set True, then compute normalized Laplacian else not (Default: true)  
__mode:__ A parameter required to set the eigen computation method. It can be either 1 or 3, 1 for generic and 3 for shift-invert(Default: 1)   
__drop_first:__ A boolean parameter if set True, then drops the first eigenvector. The first eigenvector of a normalized laplacian is full of constants, thus if drop_first is set true, compute (n_comp+1) eigenvectors and will drop the first vector. Otherwise it will calculate n_comp number of eigenvectors(Default: false)  


__Purpose__  

After getting the affinity matrix by computing distance co-relation, this is used to extract meaningful patterns in high dimensional data.
After the successful embedding, It returns a FrovedisRowmajorMatrix containing the assigned values.

__Return Value__  

This is a non-blocking call. The control will return quickly, right after submitting the training request at frovedis server 
side with a SpectralEmbedding object containing a FrovedisRowmajorMatrix with meaningful or co-related patterns obtained from eigenvectors.

### fit(X, y=None)

__Parameters__   

_X_: A scipy dense matrix or any python array-like object or an instance of FrovedisRowmajorMatrix.    
_y_: None (simply ignored in scikit-learn as well).    

__Purpose__    

It extracts meaningful or co-related patterns obtained from normalized eigenvector computation.   

For example,   

    # loading sample Rowmajor data file
    mat = FrovedisRowmajorMatrix().load("./sample")
    
    # fitting input matrix on embedding object
    embedding = SpectralEmbedding(n_components=2, gamma=1.0, mode=1, verbose=2).fit(mat)   

__Return Value__  

It simply returns "self" reference.   
Note that the call will return quickly, right after submitting the fit request 
at frovedis server side with a unique model ID for the fit request. It may be 
possible that the training is not completed at the frovedis server side even 
though the client scikit-learn side fit() returns. 

### get_params()  

__Purpose__  

It returns the values of parameters used for embedding.

__Return Value__  

It simply returns "self" instance.

### set_params()  

__Purpose__  

It intialize the values of parameters the required for embedding.

__Return Value__  

It simply returns "self" instance.

### get_affinity_matrix()  

__Purpose__  

It returns the output value of the computed affinity matrix.

__Return Value__  

It returns FrovedisRowmajorMatrix instance.

### get_embedding_matrix()  

__Purpose__  

It returns the output value of the computed normalized embedding matrix.

__Return Value__  

It returns FrovedisRowmajorMatrix instance.

### load(filename)  

__Parameters__   

_filename_: A string object containing the name of the file having model information to be loaded.    

__Purpose__    

It loads the model from the specified file (having little-endian binary data).

__Return Value__  

It returns nothing.

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

