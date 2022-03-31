% StandardScaler  

# NAME  

StandardScaler - It is a preprocessing technique which is used to perform scaling of distribution values so that mean of observed values is zero and the standard deviation is unit variance.  

# SYNOPSIS  

    frovedis.mllib.preprocessing.StandardScaler(copy = True, with_mean= True, 
                                                with_std=True, sam_std = False, verbose=0)  

## Public Member Functions  

fit(X, y = None, sample_weight = None)  
fit_transform(X, y = None)  
get_params(deep = True)  
inverse_transform(X, copy = None)  
is_fitted()  
partial_fit(X, y = None, sample_weight = None)  
release()  
set_params(\*\*params)  
transform(X, copy = None)  

# DESCRIPTION  
This machine learning algorithm standardizes the features by removing the mean and scaling to unit variance.  

The standard score of a sample x is calculated as:  

    z = (x - u) / s

where,  
- u is the mean of the training samples or zero if with_mean = False.  
- s is the standard deviation of the training samples or one if with_std = False.  

Centering and scaling happens independently on each feature by computing the relevant statistics on the samples in the training set. Mean and standard deviation are then stored to be used later on during **transform()**.  

**Currently, this class in frovedis will first create a copy of input data and then will perform standard scaling. It does not support inplace scaling yet.**  

This module provides a client-server implementation, where the client application is a normal python program. The frovedis 
interface is almost same as Scikit-learn StandardScaler interface, but it doesn't have any dependency with Scikit-learn. It can be 
used simply even if the system doesn't have Scikit-learn installed. Thus in this implementation, a python client can 
interact with a frovedis server sending the required python data for training at frovedis side. Python data is converted 
into frovedis compatible data internally and the python ML call is linked with the respective frovedis ML call to get the 
job done at frovedis server.  

Python side calls for StandardScaler on the frovedis server. Once the training is completed with the input data at the frovedis 
server, it returns an abstract model with a unique model ID to the client python program.  

When transform-like request would be made on the trained model, python program will send the same request to the frovedis 
server. After the request is served at the frovedis server, the output would be sent back to the python client.  

## Detailed Description  

### 1. StandardScaler()  
__Parameters__  
**_copy_**: This is an unused parameter. (Default: True)  
**_with\_mean_**: It is a boolean parameter that specifies whether to perform centering before scaling of data. (Default: True)  
When it is True (not specified explicitly), it will center the data before scaling.  
**Note: Also, use 'with_mean = False' when attempting centering on sparse matrices. Otherwise it raises an exception.**  
When it is set as None (specified explicitly), it will be set as False.  
**_with\_std_**: It is a boolean parameter that specifies whether to scale the data to unit variance or not. (Default: True)  
When it is True (not specified explicitly), it will scale the data to unit variance.  
When it is set as None (specified explicitly), it will be set as False.  
**_sam\_std_**: It is a boolean parameter that specifies whether to enable unbiased or biased sample standard deviation. (Default: False)  

- If it is False (not specified explicitly), it will compute biased standard deviation (where **1 / n_samples** is used).  
- If it is True (specified explicitly), it will compute unbiased standard deviation (where **1 / ('n_samples' - 1)** is used).  

**_verbose_**: An integer parameter specifying the log level to use. Its value is set as 0 by default (for INFO mode). But it can 
be set to 1 (for DEBUG mode) or 2 (for TRACE mode) for getting training time logs from frovedis server.  

__Attributes__  
**_mean\__**: It is a numpy ndarray of shape **(n_features,)** and having double (float64) type values. It specifies the mean value for each feature in the training set.  
**_var\__**: It is a numpy ndarray of shape **(n_features,)** and having double (float64) type values. It specifies the variance for each feature in the training set.  
**_scale\__**: It is a numpy ndarray of shape **(n_features,)** and having double (float64) type values. It specifies the per feature relative scaling of the data to achieve zero mean and unit variance.  

__Purpose__  
It initializes a StandardScaler object with the given parameters.  

The parameters: "copy" is simply kept in to make the interface uniform to the Scikit-learn StandardScaler module. They are not used in frovedis implementation internally.  

__Return Value__  
It simply returns "self" reference.  

### 2. fit(X, y = None, sample_weight = None)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object of int, float (float32) or double (float64) type values. It can also be an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data of float (float32) or double (float64) type. It has shape **(n_samples, n_features)**.  
**_y_**:  None or any python array-like object (any shape). It is simply ignored in frovedis implementation, like in Scikit-learn.  
**_sample\_weight_**:  None or any python array-like object (any shape). It is simply ignored in frovedis implementation.  

__Purpose__  
It computes the mean and standard deviation to be used for later scaling.  

For example,  

    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[0.1, 0.1, 0.1],
                     [0.2, 0.2, 0.2],
                     [9., 9., 9. ],
                     [9.1, 9.1, 9.1],
                     [9.2, 9.2, 9.2]])
                
    # fitting input matrix on StandardScaler object  
    from frovedis.mllib.preprocessing import StandardScaler
    ss = StandardScaler().fit(mat)  

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,
    
    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[0.1, 0.1, 0.1],
                     [0.2, 0.2, 0.2],
                     [9., 9., 9. ],
                     [9.1, 9.1, 9.1],
                     [9.2, 9.2, 9.2]])
    
    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix.  
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.  
    from frovedis.matrix.dense import FrovedisRowmajorMatrix  
    rmat = FrovedisRowmajorMatrix(mat)  
    
    # StandardScaler with pre-constructed frovedis-like inputs  
    from frovedis.mllib.preprocessing import StandardScaler
    ss = StandardScaler().fit(rmat)  

__Return Value__  
It simply returns "self" reference.  

### 3. fit_transform(X, y = None)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object of int, float (float32) or double (float64) type values. It can also be an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data of float (float32) or double (float64) type. It has shape **(n_samples, n_features)**.  
**_y_**:  None or any python array-like object (any shape). It is simply ignored in frovedis implementation.  

__Purpose__  
It will fit the model with input matrix (X) and then returns a transformed version of input matrix (X).  

For example,  

    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[0.1, 0.1, 0.1],
                     [0.2, 0.2, 0.2],
                     [9., 9., 9. ],
                     [9.1, 9.1, 9.1],
                     [9.2, 9.2, 9.2]])
    
    # fitting input matrix on StandardScaler object and perform transform 
    from frovedis.mllib.preprocessing import StandardScaler
    ss = StandardScaler()
    print(ss.fit_transform(mat))

Output  

    [[-1.23598774 -1.23598774 -1.23598774]
     [-1.21318353 -1.21318353 -1.21318353]
     [ 0.79358622  0.79358622  0.79358622]
     [ 0.81639042  0.81639042  0.81639042]
     [ 0.83919462  0.83919462  0.83919462]]    

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,
    
    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[0.1, 0.1, 0.1],
                     [0.2, 0.2, 0.2],
                     [9., 9., 9. ],
                     [9.1, 9.1, 9.1],
                     [9.2, 9.2, 9.2]])   
    
    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix.  
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.  
    from frovedis.matrix.dense import FrovedisRowmajorMatrix  
    rmat = FrovedisRowmajorMatrix(mat)  
    
    # Fitting StandardScaler with pre-constructed frovedis-like inputs and perform transform  
    from frovedis.mllib.preprocessing import StandardScaler
    ss = StandardScaler()
    print(ss.fit_transform(rmat)) 

Output  

    [[-1.23598774 -1.23598774 -1.23598774]
     [-1.21318353 -1.21318353 -1.21318353]
     [ 0.79358622  0.79358622  0.79358622]
     [ 0.81639042  0.81639042  0.81639042]
     [ 0.83919462  0.83919462  0.83919462]]    

__Return Value__  

- **When dense data is used as input:**  
For both frovedis-like input and python input, it returns a numpy matrix of shape **(n_samples, n_features)** and double (float64) type values. It contains transformed values.  
- **When sparse data is used as input:**  
For both frovedis-like input and python input, it returns a scipy sparse matrix of shape **(n_samples, n_features)** and double (float64) type values. It contains transformed values.  

### 4. get_params(deep = True)  
__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If 
True, it will return the parameters for an estimator and contained subobjects that are 
estimators. (Default: True)  

__Purpose__    
This method belongs to the BaseEstimator class inherited by StandardScaler. It is used to get parameters and their 
values of StandardScaler class.  

For example,  
 
    print(ss.get_params())  

Output  

    {'copy': True, 'sam_std': False, 'verbose': 0, 'with_mean': True, 'with_std': True}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 5. inverse_transform(X, copy = None)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object of int, float (float32) or double (float64) type values. It can also be an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data of float (float32) or double (float64) type. It has shape **(n_samples, n_features)**.  
**_copy_**: This is an unsed parameter.  

__Purpose__  
It scales back the data to the original representation.  

For example,  

    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[0.1, 0.1, 0.1],
                     [0.2, 0.2, 0.2],
                     [9., 9., 9. ],
                     [9.1, 9.1, 9.1],
                     [9.2, 9.2, 9.2]])
    
    # fitting input matrix on StandardScaler object and perform transform 
    from frovedis.mllib.preprocessing import StandardScaler
    ss = StandardScaler()
    ss.fit(mat)
    X1 = ss.transform(mat)
    
    # inverse_transform() demo to get original input data
    print(ss.inverse_transform(X1))

Output  

    [[0.1 0.1 0.1]
     [0.2 0.2 0.2]
     [9.  9.  9. ]
     [9.1 9.1 9.1]
     [9.2 9.2 9.2]]

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,
    
    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[0.1, 0.1, 0.1],
                     [0.2, 0.2, 0.2],
                     [9., 9., 9. ],
                     [9.1, 9.1, 9.1],
                     [9.2, 9.2, 9.2]])   
    
    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix.  
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.  
    from frovedis.matrix.dense import FrovedisRowmajorMatrix  
    rmat = FrovedisRowmajorMatrix(mat)  
    
    # StandardScaler with pre-constructed frovedis-like inputs and perform transform
    from frovedis.mllib.preprocessing import StandardScaler
    ss = StandardScaler()
    ss.fit(rmat)
    X1 = ss.transform(rmat)
    
    # inverse_transform() demo to get original frovedis-like input data
    print(ss.inverse_transform(X1))

Output  

    [[0.1 0.1 0.1]
     [0.2 0.2 0.2]
     [9.  9.  9. ]
     [9.1 9.1 9.1]
     [9.2 9.2 9.2]]

__Return Value__  

- **When dense data is used as input:**  
For both frovedis-like input and python input, it returns a numpy matrix of shape **(n_samples, n_features)** and double (float64) type values. It contains transformed values (original data).  
- **When sparse data is used as input:**  
For both frovedis-like input and python input, it returns a scipy sparse matrix of shape **(n_samples, n_features)** and double (float64) type values. It contains transformed values (original data).  

### 6. is_fitted()

__Purpose__    
It can be used to confirm if the model is already fitted or not. In case, transform() is used before training the model, then it 
can prompt the user to train the pca model first.  

__Return Value__  
It returns 'True', if the model is already fitted otherwise, it returns 'False'.  

### 7. partial_fit(X, y = None, sample_weight = None)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object of int, float (float32) or double (float64) type values. It can also be an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data of float (float32) or double (float64) type. It has shape **(n_samples, n_features)**.  
**_y_**:  None or any python array-like object (any shape). It is simply ignored in frovedis implementation, like in Scikit-learn.  
**_sample\_weight_**:  None or any python array-like object (any shape). It is simply ignored in frovedis implementation.  

__Purpose__  
It performs incremental computation of mean and standard deviation on each new batch of samples present in input matrix (X) and the finalised mean and standard deviation computed is later used for scaling.  

All of input matrix (X) is processed as a single batch. This is intended for cases when fit is not feasible due to very large number of 'n_samples' or because input matrix (X) is read from a continuous stream.  

For example,  

    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[0.1, 0.1, 0.1],
                     [0.2, 0.2, 0.2],
                     [9., 9., 9. ],
                     [9.1, 9.1, 9.1],
                     [9.2, 9.2, 9.2]])
                
    # partial_fit() demo with input matrix on StandardScaler object  
    from frovedis.mllib.preprocessing import StandardScaler
    ss = StandardScaler().partial_fit(mat)  

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,
    
    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[0.1, 0.1, 0.1],
                     [0.2, 0.2, 0.2],
                     [9., 9., 9. ],
                     [9.1, 9.1, 9.1],
                     [9.2, 9.2, 9.2]])
    
    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix.  
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.  
    from frovedis.matrix.dense import FrovedisRowmajorMatrix  
    rmat = FrovedisRowmajorMatrix(mat)  
    
    # StandardScaler with pre-constructed frovedis-like inputs  
    from frovedis.mllib.preprocessing import StandardScaler
    ss = StandardScaler().partial_fit(rmat)  

__Return Value__  
It simply returns "self" reference.  

### 8. release()  

__Purpose__  
It can be used to release the in-memory model at frovedis server.  

For example,
 
    ss.release()

This will reset the after-fit populated attributes to None, along with releasing server side memory.  

__Return Value__  
It returns nothing.  

### 9. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed to this function as dictionary. This dictionary contains 
parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by StandardScaler, used to set parameter values.  

For example,   

    print("get parameters before setting:") 
    print(ss.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    ss.set_params(with_mean = False, sam_std = False, verbose = 1)  
    print("get parameters after setting:") 
    print(ss.get_params())

Output  
     
    get parameters before setting:
    {'copy': True, 'sam_std': False, 'verbose': 0, 'with_mean': True, 'with_std': True}
    get parameters after setting:
    {'copy': True, 'sam_std': False, 'verbose': 1, 'with_mean': False, 'with_std': True}
    
__Return Value__  
It simply returns "self" reference.  

### 10. transform(X, copy = None)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object of int, float (float32) or double (float64) type values. It can also be an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data of float (float32) or double (float64) type. It has shape **(n_samples, n_features)**.  
**_copy_**: This is an unsed parameter.  

__Purpose__  
It performs standardization by centering and scaling.  

It will always perform transform on a copy of input matrix (X).  

For example,  

    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[0.1, 0.1, 0.1],
                     [0.2, 0.2, 0.2],
                     [9., 9., 9. ],
                     [9.1, 9.1, 9.1],
                     [9.2, 9.2, 9.2]])
    
    # fitting input matrix on StandardScaler object and perform transform 
    from frovedis.mllib.preprocessing import StandardScaler
    ss = StandardScaler()
    ss.fit(mat)
    print(ss.transform(mat))

Output  

    [[-1.23598774 -1.23598774 -1.23598774]
     [-1.21318353 -1.21318353 -1.21318353]
     [ 0.79358622  0.79358622  0.79358622]
     [ 0.81639042  0.81639042  0.81639042]
     [ 0.83919462  0.83919462  0.83919462]]  
 
When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,
    
    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[0.1, 0.1, 0.1],
                     [0.2, 0.2, 0.2],
                     [9., 9., 9. ],
                     [9.1, 9.1, 9.1],
                     [9.2, 9.2, 9.2]])   
    
    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix.  
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.  
    from frovedis.matrix.dense import FrovedisRowmajorMatrix  
    rmat = FrovedisRowmajorMatrix(mat)  
    
    # Fitting StandardScaler with pre-constructed frovedis-like inputs and perform transform  
    from frovedis.mllib.preprocessing import StandardScaler
    ss = StandardScaler()
    ss.fit(rmat)
    print(ss.transform(rmat)) 

Output  

    [[-1.23598774 -1.23598774 -1.23598774]
     [-1.21318353 -1.21318353 -1.21318353]
     [ 0.79358622  0.79358622  0.79358622]
     [ 0.81639042  0.81639042  0.81639042]
     [ 0.83919462  0.83919462  0.83919462]]  

__Return Value__  

- **When dense data is used as input:**  
For both frovedis-like input and python input, it returns a numpy matrix of shape **(n_samples, n_features)** and double (float64) type values. It contains transformed values.  
- **When sparse data is used as input:**  
For both frovedis-like input and python input, it returns a scipy sparse matrix of shape **(n_samples, n_features)** and double (float64) type values. It contains transformed values.  


# SEE ALSO  
- **[Introduction to FrovedisRowmajorMatrix](../matrix/rowmajor_matrix.md)**  
- **[Introduction to FrovedisCRSMatrix](../matrix/crs_matrix.md)**  