% Spectral Embedding

# NAME
Spectral Embedding - One of the accurate method for extraction of meaningful patterns in high 
dimensional data. It forms an affinity matrix given by the specified function and applies 
spectral decomposition to the corresponding graph laplacian. The resulting transformation 
is given by the value of the normalized eigenvectors for each data point.

# SYNOPSIS

    class frovedis.mllib.manifold.SpectralEmbedding(n_components=2, affinity='nearest_neighbors',  
                                                    gamma=1.0, random_state=None, eigen_solver=None,  
                                                    n_neighbors=None, n_jobs=None, verbose=0,  
                                                    norm_laplacian=True, mode=3, drop_first=True)  

## Public Member Functions

fit(X, y = None)  
load(fname, dtype = None)  
save(fname)  
get_params(deep = True)  
set_params(\*\*params)  
debug_print()  
release()  
is_fitted()  

# DESCRIPTION
Spectral embedding is particularly useful for reducing the dimensionality of data that is 
expected to lie on a low-dimensional manifold contained within a high-dimensional space. 
It yields a low-dimensional representation of the data that best preserves the structure 
of the original manifold in the sense that points that are close to each other on the 
original manifold will also be close after embedding. At the same time, the embedding 
emphasizes clusters in the original data.  

This module provides a client-server implementation, where the client application 
is a normal python program. Frovedis is almost same as Scikit-learn manifold
module providing Spectral Embedding support, but it doesn’t have any dependency with 
Scikit-learn. It can be used simply even if the system doesn’t have Scikit-learn 
installed. Thus, in this implementation, a python client can interact with a 
frovedis server sending the required python data for training at frovedis side. 
Python data is converted into frovedis compatible data internally and the python 
ML call is linked with the respective frovedis ML call to get the job done at 
frovedis server.  

Python side calls for Spectral Embedding on the frovedis server. Once the training is completed 
with the input data at the frovedis server, it returns an abstract model with a 
unique model ID to the client python program. 

When prediction-like request would be made on the trained model, the python program will 
send the same request to the frovedis server. After the request is served at the 
frovedis server, the output would be sent back to the python client.  

## Detailed Description  

### 1. SpectralEmbedding()

__Parameters__   
_**n\_components**_: An integer parameter containing the number of component count. 
(Default: 2)  
_**affinity**_: A string object parameter which specifies how to construct the affinity matrix. 
(Default: 'nearest_neighbors')  
- **'nearest_neighbors'**: construct the affinity matrix by computing a graph of nearest neighbors.  
- **'precomputed'**: interpret X as a precomputed affinity matrix, where larger values indicate greater 
similarity between instances.

Only 'nearest_neighbors' and 'precomputed' are supported.  
_**gamma**_: The double (float64) parameter required for computing nearby relational meaningful 
eigenvalues. (Default: 1.0)  
_**random_state**_: An unused parameter. (Default: None)  
_**eigen\_solver**_: An unused parameter. (Default: None)  
_**n_neighbors**_: An unused parameter. (Default: None)  
_**n\_jobs**_: An unused parameter. (Default: None)  
_**verbose**_: An integer parameter specifying the log level to use. Its value is 0 by default 
(for INFO mode and not specified explicitly). But it can be set to 1 (for DEBUG mode) or 2 (for 
TRACE mode) for getting training time logs from frovedis server.  
_**norm\_laplacian**_: A boolean parameter if set to True, then computes the normalized Laplacian. 
(Default: True)  
_**mode**_: An integer parameter required to set the eigen computation method. It can be either 
1 (for generic) or 3 (for shift-invert). (Default: 3)  
_**drop\_first**_: A boolean parameter if set to True, then drops the first eigenvector. The first 
eigenvector of a normalized Laplacian is full of constants, thus if it is set to true, then 
(n_components + 1) eigenvectors are computed and will drop the first vector. Otherwise, it will 
calculate ‘n_components’ number of eigenvectors. (Default: True) 

__Attribute__  
_**affinity\_matrix**_:  

1. **For python native dense input:**  
     - When affinity = ‘precomputed’, it returns a numpy array  
     - When affinity = ‘nearest_neighbors’, it returns a numpy array  

2. **For frovedis-like dense input:**  
     - When affinity = ‘precomputed', returns a FrovedisRowmajorMatrix  
     - When affinity = ‘nearest_neighbors’, returns a FrovedisRowmajorMatrix  

3. **For python native sparse input:**  
     - When affinity = ‘precomputed’, it returns a scipy matrix  
     - When affinity = 'nearest_neighbors', it returns a numpy array  

4. **For frovedis-like sparse input:**  
     - When affinity = ‘nearest_neighbors’, it a returns a FrovedisRowmajorMatrix  
     - When affinity = ‘precomputed’, it returns a FrovedisRowmajorMatrix  

In all cases, the output is of float or double (float64) type and of shape **(n_samples, n_samples)**.  

_**embedding\_**_:  

1. **For python native dense input:**  
     - When affinity = ‘precomputed’, it returns a numpy array  
     - When affinity = ‘nearest_neighbors’, it returns a numpy array  

2. **For frovedis-like dense input:**  
     - When affinity = ‘precomputed', returns a FrovedisRowmajorMatrix  
     - When affinity = ‘nearest_neighbors’, returns a FrovedisRowmajorMatrix  

3. **For python native sparse input:**  
     - When affinity = ‘precomputed’, it returns a numpy array  
     - When affinity = 'nearest_neighbors', it returns a numpy array  

4. **For frovedis-like sparse input:**  
     - When affinity = ‘nearest_neighbors’, it a returns a FrovedisRowmajorMatrix  
     - When affinity = ‘precomputed’, it returns a FrovedisRowmajorMatrix  

In all cases, the output is of float or double (float64) type and of shape **(n_samples, n_components)**.  

**Note: affinity = ‘precomputed’ should be used with square matrix input only, Otherwise, it throws an exception.**  

__Purpose__  
It initializes a Spectral Embedding object with the given parameters.  

The parameters: 'random\_state', 'eigen\_solver', 'n_neighbors' and 'n_jobs' are simply kept in to make the 
interface uniform to the Scikit-learn Spectral Embedding module. They are not used anywhere within
frovedis implementation.  

After getting the affinity matrix by computing distance co-relation, this is used to extract 
meaningful patterns in high dimensional data.  

__Return Value__  
It simply returns “self” reference.  


### 2. fit(X, y = None)

__Parameters__   
_**X**_: A numpy dense or scipy sparse matrix or any python array-like object or
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for 
dense data of float or double (float64) type. It has shape **(n_samples, n_features)**.   
_**y**_: None or any python array-like object (any shape). It is simply ignored 
in frovedis implementation, like in Scikit-learn.  

__Purpose__    

It extracts meaningful or co-related patterns obtained from normalized eigenvector computation.   

For example,   

    # loading sample matrix data
    train_mat = np.loadtxt("spectral_data.txt")
    
    # fitting input matrix on Spectral Embedding object
    from frovedis.mllib.manifold import SpectralEmbedding
    sem = SpectralEmbedding(n_components = 2, drop_first = True).fit(train_mat)  

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.   

For example,   

    # loading sample matrix data
    train_mat = np.loadtxt("spectral_data.txt")
    
    # Since "train_mat" is numpy dense data, we have created FrovedisRowmajorMatrix.
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.   
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(train_mat)
    
    # fitting input matrix on Spectral Embedding object
    from frovedis.mllib.manifold import SpectralEmbedding
    sem = SpectralEmbedding(n_components = 2, drop_first = True).fit(rmat)  
    
__Return Value__  
It simply returns "self" reference.  

### 3. load(fname, dtype = None)  

__Parameters__   

_**fname**_: A string object containing the name of the file having model 
information to be loaded.  
_**dtype**_: A data-type is inferred from the input data. Currently, 
expected input data-type is either float or double (float64). (Default: None)  

__Purpose__    
It loads the model from the specified file(having little-endian binary data).

For example,  

    # loading the same model
    sem.load("./out/MySemModel",dtype=np.float64)
    
__Return Value__  
It simply returns "self" instance.   

### 4. save(fname)

__Parameters__   

_**fname**_: A string object containing the name of the file on which the target 
model is to be saved.    

__Purpose__    

On success, it writes the model information(metadata and model) in the specified 
file as little-endian binary data. Otherwise, it throws an exception. 

For example,   

    # saving the model
    sem.save("./out/MySemModel")

The MySemModel contains below directory structure:  
**MySemModel**  
    |------metadata  
    |------**model**    
\ \ \ \ \ \ \ \ |------aff_type  
\ \ \ \ \ \ \ \ |------**affinity**  
\ \ \ \ \ \ \ \ |------**embedding**  
             
'metadata' represents the detail about n_components, model_kind and datatype of training vector.
Here, the **model** directory contains information about type of affinity matrix, affinity matrix 
and embedding matrix.  

If the directory already exists with the same name then it will raise an exception.  

__Return Value__  
It returns nothing.  

### 5. get_params(deep = True)  

__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If True, will return the 
parameters for an estimator and contained subobjects that are estimators. (Default: True)

__Purpose__    
This method belongs to the BaseEstimator class inherited by Spectral Embedding. It is used to get parameters
and their values of Spectral Embedding class.  

For example, 
 
    print(sem.get_params())

Output  

    {'affinity': 'nearest_neighbors', 'drop_first': True, 'eigen_solver': None, 'gamma': 1.0, 
    'mode': 3, 'n_components': 2, 'n_jobs': None, 'n_neighbors': None, 'norm_laplacian': True, 
    'random_state': None, 'verbose': 0}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 6. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed this function as dictionary. This dictionary 
contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by Spectral Embedding, used to set parameter values.  

For example,   

    print("get parameters before setting:")
    print(sem.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    sem.set_params(n_components = 3, drop_first = False)
    print("get parameters after setting:")
    print(sem.get_params())

Output  
     
    get parameters before setting: 
    {'affinity': 'nearest_neighbors', 'drop_first': True, 'eigen_solver': None, 
    'gamma': 1.0, 'mode': 3, 'n_components': 2, 'n_jobs': None, 'n_neighbors': None, 
    'norm_laplacian': True, 'random_state': None, 'verbose': 0}
    get parameters after setting: 
    {'affinity': 'nearest_neighbors', 'drop_first': False, 'eigen_solver': None, 
    'gamma': 1.0, 'mode': 3, 'n_components': 3, 'n_jobs': None, 'n_neighbors': None, 
    'norm_laplacian': True, random_state': None, 'verbose': 0}  
    
__Return Value__  
It simply returns "self" reference.  


### 7. debug_print()

__Purpose__   
It shows the target model information (affinity and embed matrix) on the server side user terminal. 
It is mainly used for debugging purpose.   

For example,  

    sem.debug_print()
    
Output

    affinity matrix:
    num_row = 5, num_col = 5
    node 0
    node = 0, local_num_row = 5, local_num_col = 5, val = 1 0.970446 6.2893e-104 2.92712e-106 
    1.28299e-108 0.970446 1 1.27264e-101 6.2893e-104 2.92712e-106 6.2893e-104 1.27264e-101 
    1 0.970446 0.88692 2.92712e-106 6.2893e-104 0.970446 1 0.970446 1.28299e-108 2.92712e-106 
    0.88692 0.970446 1
    embed matrix:
    num_row = 5, num_col = 2
    node 0
    node = 0, local_num_row = 5, local_num_col = 2, val = -0.628988 -0.345834 -0.628988 
    -0.345834 -0.202594 0.368471 -0.202594 0.368471 -0.202594 0.368471

This output will be visible on server side. It displays the affinity matrix and embedding matrix 
on the trained model which is currently present on the server.  

**No such output will be visible on client side.**  

__Return Value__  
It returns nothing.  


### 8. release()

__Purpose__   
It can be used to release the in-memory model at frovedis server. 

For example,  

    sem.release()
    
This will reset the after-fit populated attributes to None, along with 
releasing server side memory.  

__Return Value__  
It returns nothing.   


### 9. is_fitted()  

__Purpose__  
It can be used to confirm if the model is already fitted or not. In case, affinity_matrix 
is used before training the model, then it can prompt the user to train the clustering 
model first.  

__Return Value__  
It returns 'True', if the model is already fitted otherwise, it returns 'False'.  

# SEE ALSO  
- **[Introduction to FrovedisRowmajorMatrix](../matrix/rowmajor_matrix.md)**  
- **[Introduction to FrovedisCRSMatrix](../matrix/crs_matrix.md)**  
- **[Spectral Clustering in Frovedis](./spectral_clustering.md)**  
