% t-Distributed Stochastic Neighbor Embedding

# NAME

TSNE - It's full form is T-distributed Stochastic Neighbor Embedding. It is an unsupervised algorithm primarily used for data exploration and visualizing high-dimensional data.  

# SYNOPSIS

    frovedis.mllib.manifold.tsne(n_components=2, perplexity=30.0,
                                 early_exaggeration=12.0, learning_rate=200.0, 
                                 n_iter=1000, n_iter_without_progress=300, 
                                 min_grad_norm=1e-7, metric="euclidean", 
                                 init="random", verbose=0, random_state=None, 
                                 method="exact", angle=0.5, n_jobs=None)

## Public Member Functions  

fit(X, y = None)  
fit_transform(X, y = None)  
get_params(deep = True)  
set_params(\*\*params)  

# DESCRIPTION  
It is a nonlinear dimensionality reduction technique well-suited for embedding high-dimensional data for visualization
in a low-dimensional space of two or three dimensions. Specifically, it models each high-dimensional object by a two or 
three dimensional point in such a way that similar objects are modeled by nearby points and dissimilar objects are modeled 
by distant points with high probability.  

This module provides a client-server implementation, where the client application is a normal python program. The frovedis 
interface is almost same as Scikit-learn TSNE interface, but it doesn't have any dependency with Scikit-learn. It can be 
used simply even if the system doesn't have Scikit-learn installed. Thus in this implementation, a python client can 
interact with a frovedis server sending the required python data for training at frovedis side. Python data is converted 
into frovedis compatible data internally and the python ML call is linked with the respective frovedis ML call to get the 
job done at frovedis server.  

Python side calls for TSNE on the frovedis server. Once the training is completed with the input data at the frovedis 
server, it returns an abstract model with a unique model ID to the client python program.  

When tranform-like request would be made on the trained model, python program will send the same request to the frovedis 
server. After the request is served at the frovedis server, the output would be sent back to the python client.  

## Detailed Description  

### 1. TSNE()  
__Parameters__  
**_n\_components_**: It is an integer parameter that specifies the dimension of the embedded space. (Default: 2)  
Currently, it supports **'n_components = 2'** only as parameter value.  
**_perplexity_**: It must be a positive double (float64) parameter that specifies the number of nearest neighbors. Larger datasets usually require a larger perplexity. Consider selecting a value between 5 and 50. Different values can result in significantly 
different results. (Default: 30.0)  
**_early\_exaggeration_**: It must be a positive double (float64) parameter that controls how tight natural clusters in the original space are in the embedded space and how much space will be between them. For larger values, the space between natural clusters will be larger in the embedded space. (Default: 12.0)  
**_learning\_rate_**: It accepts the word 'auto' or a positive double (float64) as parameter value that controls the step size of the gradient updates. It is usually in the range **[10.0, 1000.0]**. **Currently, 'auto' as parameter value cannot be used.** (Default: 200.0)  

- **If the 'learning_rate' is too high**: then the data may look like a **'ball'** when plotted on a graph with any point approximately equidistant from its nearest neighbours.  
- **If the 'learning_rate' is too low**: then most data points may look compressed in a **dense cloud** with few outliers when plotted on a graph.  

**_n\_iter_**: It must be a positive integer value that specifies the maximum number of iterations for the optimization. It must be atleast 250. (Default: 1000)  
**_n\_iter\_without\_progress_**: It is an integer parameter that specifies the maximum number of iterations without progress before we abort the optimization. (Default: 300)  
**_min\_grad\_norm_**: It is a double (float64) parameter that specifies whether in case the gradient norm is below this threshold, then the optimization will be stopped. (Default: 1e-7)  
**_metric_**: It is a string object parameter that specifies the metric to use when calculating distance between instances in a 
feature array. It supports **'euclidean'** or **'precomputed'** distances. (Default: 'euclidean')  
**_init_**: It is a string object parameter that specifies the initialization of embedding. (Default: 'random')  
**Currently, only random initialization is supported for this method in frovedis**.  
**_verbose_**:  An integer parameter specifying the log level to use. Its value is set as 0 by default (for INFO mode). But it can be set to 1 (for DEBUG mode) or 2 (for TRACE mode) for getting training time logs from frovedis server.  
**_random\_state_**: This is an unused parameter. (Default: None)  
**_method_**: It is a string object parameter that specifies the t-SNE implementation method to use. (Default: 'exact')  

- **'exact'**: it calculates the pair-wise distance between every pair of data points. **Currently, only exact implementation of tsne is supported in frovedis.**  
- **'barnes_hut'**: it calculates the distance between each data point and its closest neighboring points only. **Currently, this implementation of tsne is not supported in frovedis.**  

**_angle_**: This is an unused parameter. (Default: 0.5)  
**_n\_jobs_**: This is an unused parameter. (Default: None)  

__Attributes__  
**_n\_iter\__**: It is a positive integer value that specifies the number of iterations run.  
**_kl\_divergence\__**: It is a double (float64) type value that specifies the Kullback-Leibler divergence after optimization.  
**_embedding\__**: It is a numpy ndarray of double (float64) type values or FrovedisRowmajorMatrix instance, having shape **(n_samples, n_components)**, where **n_samples** is the number of samples in the input matrix (X). It stores the embedding vectors.  

__Purpose__  
It initializes a TSNE object with the given parameters.  

The parameters: "random_state", "angle", "n_jobs" are simply kept in to make the interface 
uniform to the Scikit-learn TSNE module. They are not used anywhere within the frovedis implementation.  

__Return Value__  
It simply returns "self" reference.  

### 2. fit(X, y = None)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object of int, float or double (float64) type values. It 
can also be an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data of float or 
double (float64) type. It has shape **(n_samples, n_features)**. **If metric = 'precomputed'**, then input matrix (X) is 
assumed to be a **squared distance matrix**.  
**_y_**:  None or any python array-like object (any shape). It is simply ignored in frovedis implementation, like in Scikit-learn.  

__Purpose__  
It will fit input matrix (X) into an embedded space.  

For example,  

    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[0, 0, 0, 0], [0, 1, 1, 1], 
                     [1, 0, 1, 0], [1, 1, 1, 0], 
                     [1, 1, 1, 1]], dtype = np.float64)
    
    # fitting input matrix and label on TSNE object  
    from frovedis.mllib.manifold import TSNE
    tsne = TSNE().fit(mat)  

When native python data is provided, it is converted to frovedis-like inputs and sent to frovedis server which consumes 
some data transfer time. Pre-constructed frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,
    
    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[0, 0, 0, 0], [0, 1, 1, 1], 
                     [1, 0, 1, 0], [1, 1, 1, 0], 
                     [1, 1, 1, 1]], dtype = np.float64)    
    
    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix.  
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.  
    from frovedis.matrix.dense import FrovedisRowmajorMatrix  
    rmat = FrovedisRowmajorMatrix(mat)  
    
    # TSNE with pre-constructed frovedis-like inputs  
    from frovedis.mllib.manifold import TSNE
    tsne = TSNE().fit(rmat)  

__Return Value__  
It simply returns "self" reference.  

### 3. fit_transform(X, y = None)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object of int, float or double (float64) type values. It 
can also be an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data of float or 
double (float64) type. It has shape **(n_samples, n_features)**. **If metric = 'precomputed'**, then input matrix (X) is 
assumed to be a **squared distance matrix**.  
**_y_**:  None or any python array-like object (any shape). It is simply ignored in frovedis implementation, like in Scikit-learn.  

__Purpose__  
It will fit input matrix (X) into an embedded space and will return the transformed output.  

For example,  

    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[0, 0, 0, 0], [0, 1, 1, 1], 
                     [1, 0, 1, 0], [1, 1, 1, 0], 
                     [1, 1, 1, 1]], dtype = np.float64)
    
    # fitting input matrix and label on TSNE object  
    from frovedis.mllib.manifold import TSNE
    tsne = TSNE()
    print(tsne.fit_transform(mat))

Output  

    [[ -15.15708437 -218.36584687]
     [  31.18096218   62.89769738]
     [ -79.72509032  -59.56844016]
     [ 129.59496966 -137.71666144]
     [ 195.56724491   14.28454464]]

When native python data is provided, it is converted to frovedis-like inputs and sent to frovedis server which consumes 
some data transfer time. Pre-constructed frovedis-like inputs can be used to speed up the training time, especially when same 
data would be used for multiple executions.  

For example,
    
    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[0, 0, 0, 0], [0, 1, 1, 1], 
                     [1, 0, 1, 0], [1, 1, 1, 0], 
                     [1, 1, 1, 1]], dtype = np.float64)    
    
    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix.  
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.  
    from frovedis.matrix.dense import FrovedisRowmajorMatrix  
    rmat = FrovedisRowmajorMatrix(mat)  
    
    # TSNE with pre-constructed frovedis-like inputs  
    from frovedis.mllib.manifold import TSNE
    embedding = TSNE().fit_transform(rmat))   
    embedding.debug_print()

Output  

    matrix:
    num_row = 5, num_col = 2
    node 0
    node = 0, local_num_row = 5, local_num_col = 2, val = -15.1571 -218.366 31.181 62.8977 -79.7251 
    -59.5684 129.595 -137.717 195.567 14.2845

__Return Value__  

- **When X is python native input:**  
It returns a python ndarray of shape **(n_samples, n_components)** and double (float64) type values. It contains the 
embedding of the training data in low-dimensional space.  
- **When X is frovedis-like input:**  
It returns a FrovedisRowmajorMatrix instance of shape **(n_samples, n_components)** and double (float64) type values, containing 
the embedding of the training data in low-dimensional space.  

### 4. get_params(deep = True)  
__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If True, it will return the 
parameters for an estimator and contained subobjects that are estimators. (Default: True)  

__Purpose__    
This method belongs to the BaseEstimator class inherited by TSNE. It is used to get parameters and their values of TSNE class.  

For example,  
 
    print(tsne.get_params())  

Output  

    {'angle': 0.5, 'early_exaggeration': 12.0, 'init': 'random', 'learning_rate': 200.0, 
    'method': 'exact', 'metric': 'euclidean', 'min_grad_norm': 1e-07, 'n_components': 2, 
    'n_iter': 1000, 'n_iter_without_progress': 300, 'n_jobs': None, 'perplexity': 30.0, 
    'random_state': None, 'verbose': 0}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 5. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed to this function as dictionary. This dictionary contains parameters 
of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by TSNE, used to set parameter values.  

For example,   

    print("get parameters before setting:") 
    print(tsne.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    tsne.set_params(perplexity = 15, metric = 'precomputed')  
    print("get parameters after setting:") 
    print(tsne.get_params())

Output  
     
    get parameters before setting:
    {'angle': 0.5, 'early_exaggeration': 12.0, 'init': 'random', 'learning_rate': 200.0, 
    'method': 'exact', 'metric': 'euclidean', 'min_grad_norm': 1e-07, 'n_components': 2, 
    'n_iter': 1000, 'n_iter_without_progress': 300, 'n_jobs': None, 'perplexity': 30.0, 
    'random_state': None, 'verbose': 0}
    get parameters after setting:
    {'angle': 0.5, 'early_exaggeration': 12.0, 'init': 'random', 'learning_rate': 200.0, 
    'method': 'exact', 'metric': 'precomputed', 'min_grad_norm': 1e-07, 'n_components': 2, 
    'n_iter': 1000, 'n_iter_without_progress': 300, 'n_jobs': None, 'perplexity': 15.0, 
    'random_state': None, 'verbose': 0}
    
__Return Value__  
It simply returns "self" reference.  

**Note: In order to release the embedding vector from the server (generated by TSNE algorithm in frovedis), we can use release() of FrovedisRowmajorMatrix class.**  

For example,  

    tsne.embedding_.release()

This will remove the embedding vector from the server side memory.  

# SEE ALSO  

- **[Spectral Embedding in frovedis](./spectral_embedding.md)**  
- **[DVector](./dvector.md)**  
- **[FrovedisCRSMatrix](./crs_matrix.md)**  
- **[FrovedisRowmajorMatrix](./rowmajor_matrix.md)**  