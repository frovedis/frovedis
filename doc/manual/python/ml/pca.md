% Principal Component Analysis  

# NAME  

PCA - It's full form is Principal Component Analysis. It is an unsupervised learning algorithm that is used for dimensionality reduction in machine learning.  

# SYNOPSIS  

    frovedis.mllib.decomposition.pca(n_components=None, copy=True, whiten=False,
                                     svd_solver='auto', tol=0.0, iterated_power='auto',
                                     random_state=None)  

## Public Member Functions  

fit(X, y = None)  
fit_transform(X)  
get_params(deep = True)  
inverse_transform(X)  
is_fitted()  
load(path, dtype)  
load_binary(path, dtype)  
release()  
save(path)  
save_binary(path)  
set_params(\*\*params)  
transform(X)  

# DESCRIPTION  
It is one of the popular tools that is used for exploratory data analysis and predictive modeling. It is a technique to draw strong patterns from the given dataset by reducing the variances.  

PCA generally tries to find the lower-dimensional surface to project the high-dimensional data.  

For dimensionality reduction, it uses Singular Value Decomposition of the data. The input data is centered but not scaled for each feature before applying the SVD.  

This module provides a client-server implementation, where the client application is a normal python program. The frovedis 
interface is almost same as Scikit-learn PCA interface, but it doesn't have any dependency with Scikit-learn. It can be 
used simply even if the system doesn't have Scikit-learn installed. Thus in this implementation, a python client can 
interact with a frovedis server sending the required python data for training at frovedis side. Python data is converted 
into frovedis compatible data internally and the python ML call is linked with the respective frovedis ML call to get the 
job done at frovedis server.  

Python side calls for PCA on the frovedis server. Once the training is completed with the input data at the frovedis 
server, it returns an abstract model with a unique model ID to the client python program.  

When transform-like request would be made on the trained model, python program will send the same request to the frovedis 
server. After the request is served at the frovedis server, the output would be sent back to the python client.  

**Like Scikit-learn, PCA in frovedis only supports dense input data. However, when sparse input is provided in frovedis, internally, it will be converted into frovedis-like dense input before sending it to frovedis server.**  

## Detailed Description  

### 1. PCA()  
__Parameters__  
**_n\_components_**: It accepts a positive integer value as parameter that specifies the number of components to keep. It must be in range **\[1, min(n_samples_, n_features_) - 1)**. (Default: None)  
When it is None (not specified explicitly), it wil be set as **min(n_samples_, n_features_) - 1**.  
**_copy_**: It is a boolean parameter that specifies whether input data passed is overwritten or not during fitting. (Default: True)  
When it is False (specified explicitly), data passed to fit() is overwritten and running fit(X).transform(X) will not yield the expected results, so fit_transform(X) is to be used instead.  
**_whiten_**: It is a boolean parameter. It removes some information from the transformed signal (the relative variance scales of the components) but can sometime improve the predictive accuracy of the downstream estimators. (Default: False)  
When it is True (specified explicitly), the **components_** vectors are multiplied by the square root of **n_samples** and then divided by the singular values to ensure uncorrelated outputs with unit component-wise variances.  
**_svd\_solver_**: It accepts a string value as parameter that specifies which solver to use. **Currently it supports only 'arpack' implementation of svd.** (Default: 'auto')  
When it is auto, then **svd_solver = 'arpack'.**  
**_tol_**: This is an unused parameter. (Default: 0.0)  
**_iterated\_power_**: This is an unused parameter. (Default: 'auto')  
**_random\_state_**: This is an unused parameter. (Default: None)  

__Attributes__  
**_components\__**: It is a numpy ndarray of shape **(n_components, n_features)**. It specifies the principal axes in feature space, representing the directions of maximum variance in the data.  
**_explained\_variance\__**: It is a numpy ndarray of shape **(n_components,)**. It specifies the variance of the training samples transformed by a projection to each component.  
**_explained\_variance\_ratio\__**: It is a numpy ndarray of shape **(n_components,)**. It specifies the percentage of variance explained by each of the selected components. If **'n_components'** is not set, then all components are stored and the sum of the ratios 
is equal to 1.0.  
**_mean\__**: It is a numpy ndarray of shape **(n_features,)**. It specifies the per-feature empirical mean, estimated from the training set. It is equal to **X.mean(axis=0)**.  
**_n\_components\__**: It is a positive integer value that specifies the estimated number of components.  
**_n\_features\__**: It is a positive integer value that specifies the number of features in the training data (X).  
**_n\_samples\__**: It is a positive integer value that specifies the number of samples in the training data (X).  
**_noise\_varaince\__**: It is a float (float32) value that specifies the estimated noise covariance. It is required to compute the estimated data covariance and score samples. It is equal to the **average of (min(n_features, n_samples) - n_components)** smallest eigenvalues of the covariance matrix of input matrix (X).  
**_singular\_values\__**: It is a numpy ndarray of shape **(n_components,)** that specifies the singular values corresponding to each of the selected components.  

__Purpose__  
It initializes a PCA object with the given parameters.  

The parameters: "tol", "iterated_power" and "random_state" are simply kept in to make the interface 
uniform to the Scikit-learn PCA module. They are not used in frovedis implementation internally.  

__Return Value__  
It simply returns "self" reference.  

### 2. fit(X, y = None)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object of int, float or double (float64) type values. It can also 
be an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data of float or double (float64) type. It 
has shape **(n_samples, n_features)**.  
**_y_**:  None or any python array-like object (any shape). It is simply ignored in frovedis implementation, like in Scikit-learn.  

__Purpose__  
It will fit the model with input matrix (X).  

For example,  

    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[1.0, 0.0, 7.0, 0.0, 0.0], 
                     [2.0, 0.0, 3.0, 4.0, 5.0], 
                     [4.0, 0.0, 0.0, 6.0, 7.0]], 
                    dtype = np.float64)
                
    # fitting input matrix on PCA object  
    from frovedis.mllib.decomposition import PCA
    pca = PCA().fit(mat)  

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,
    
    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[1.0, 0.0, 7.0, 0.0, 0.0], 
                     [2.0, 0.0, 3.0, 4.0, 5.0], 
                     [4.0, 0.0, 0.0, 6.0, 7.0]], 
                    dtype = np.float64)  
    
    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix.  
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.  
    from frovedis.matrix.dense import FrovedisRowmajorMatrix  
    rmat = FrovedisRowmajorMatrix(mat)  
    
    # PCA with pre-constructed frovedis-like inputs  
    from frovedis.mllib.decomposition import PCA
    pca = PCA().fit(rmat)  

__Return Value__  
It simply returns "self" reference.  

### 3. fit_transform(X, y = None)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object of int, float or double (float64) type values. It can also be an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data of float or double (float64) type. It has shape **(n_samples, n_features)**.  
**_y_**:  None or any python array-like object (any shape). It is simply ignored in frovedis implementation, like in Scikit-learn.  

__Purpose__  
It will fit the model with input matrix (X) and apply the dimensionality reduction on input matrix (X).  

For example,  

    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[1.0, 0.0, 7.0, 0.0, 0.0], 
                     [2.0, 0.0, 3.0, 4.0, 5.0], 
                     [4.0, 0.0, 0.0, 6.0, 7.0]], 
                    dtype = np.float64)
    
    # fitting input matrix on PCA object and perform transform 
    from frovedis.mllib.decomposition import PCA
    pca = PCA()
    print(pca.fit_transform(mat))

Output  

    [[-6.50037234  0.28016556]
     [ 1.0441383  -0.75923769]
     [ 5.45623404  0.47907213]]

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,
    
    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[1.0, 0.0, 7.0, 0.0, 0.0], 
                     [2.0, 0.0, 3.0, 4.0, 5.0], 
                     [4.0, 0.0, 0.0, 6.0, 7.0]], 
                    dtype = np.float64)    
    
    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix.  
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.  
    from frovedis.matrix.dense import FrovedisRowmajorMatrix  
    rmat = FrovedisRowmajorMatrix(mat)  
    
    # Fitting PCA with pre-constructed frovedis-like inputs and perform transform  
    from frovedis.mllib.decomposition import PCA
    pca = PCA()
    print(pca.fit_transform(rmat)) 

Output  

    [[-6.50037234  0.28016556]
     [ 1.0441383  -0.75923769]
     [ 5.45623404  0.47907213]]

__Return Value__  
For both frovedis-like input and python input, it returns a numpy ndarray of shape **(n_samples, n_components)** and double (float64) type values. It contains transformed values.  

### 4. get_params(deep = True)  
__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If 
True, it will return the parameters for an estimator and contained subobjects that are 
estimators. (Default: True)  

__Purpose__    
This method belongs to the BaseEstimator class inherited by PCA. It is used to get parameters and their 
values of PCA class.  

For example,  
 
    print(pca.get_params())  

Output  

    {'copy': True, 'iterated_power': 'auto', 'n_components': 2, 'random_state': None, 
    'svd_solver': 'arpack', 'tol': 0.0, 'whiten': False}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 5. inverse_transform(X, y = None)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object of int, float or double (float64) type values. It can also be an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data of float or double (float64) type. It has shape **(n_samples, n_features)**.  
**_y_**:  None or any python array-like object (any shape). It is simply ignored in frovedis implementation, like in Scikit-learn.  

__Purpose__  
It transforms data back to its original space.  

In other words, it returns an input matrix (X_original) whose transform would be input matrix (X).  

For example,  

    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[1.0, 0.0, 7.0, 0.0, 0.0], 
                     [2.0, 0.0, 3.0, 4.0, 5.0], 
                     [4.0, 0.0, 0.0, 6.0, 7.0]], 
                    dtype = np.float64)
    
    # fitting input matrix on PCA object and perform transform 
    from frovedis.mllib.decomposition import PCA
    pca = PCA().fit(mat)
    X1 = pca.transform(mat)
    
    # inverse_transform() demo to get original input data
    print(pca.inverse_transform(X1))

Output  

    [[ 1.00000000e+00  0.00000000e+00  7.00000000e+00  5.77315973e-15
       1.24344979e-14]
     [ 2.00000000e+00  0.00000000e+00  3.00000000e+00  4.00000000e+00
       5.00000000e+00]
     [ 4.00000000e+00  0.00000000e+00 -7.99360578e-15  6.00000000e+00
       7.00000000e+00]]

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,
    
    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[1.0, 0.0, 7.0, 0.0, 0.0], 
                     [2.0, 0.0, 3.0, 4.0, 5.0], 
                     [4.0, 0.0, 0.0, 6.0, 7.0]], 
                    dtype = np.float64)    
    
    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix.  
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.  
    from frovedis.matrix.dense import FrovedisRowmajorMatrix  
    rmat = FrovedisRowmajorMatrix(mat)  
    
    # PCA with pre-constructed frovedis-like inputs and perform transform
    from frovedis.mllib.decomposition import PCA
    pca = PCA().fit(rmat)
    X1 = pca.transform(rmat)
    
    # inverse_transform() demo to get original frovedis-like input data
    X2 = pca.inverse_transform(X1)
    X2.debug_print()

Output  

    matrix:
    num_row = 3, num_col = 5
    node 0
    node = 0, local_num_row = 3, local_num_col = 5, val = 1 0 7 5.77316e-15 1.24345e-14 
    2 0 3 4 5 4 0 -7.99361e-15 6 7

__Return Value__  

- **When X is python native input:**  
It returns a numpy ndarray of shape **(n_samples, n_components)** and double (float64) type values. It contains original input 
data values.  
- **When X is frovedis-like input:**  
It returns a FrovedisRowmajorMatrix instance of shape **(n_samples, n_components)** and double (float64) type values. It contains original input data values.  

### 6. is_fitted()

__Purpose__    
It can be used to confirm if the model is already fitted or not. In case, transform() is used before training the model, then it 
can prompt the user to train the pca model first.  

__Return Value__  
It returns 'True', if the model is already fitted otherwise, it returns 'False'.  

### 7. load(path, dtype)  
__Parameters__   
**_path_**: A string object containing the name of the file having model information to be loaded.  
**_dtype_**: It is the data-type of the loaded model with trainig data samples. Currently, expected input data-type is either float (float32) or double (float64).  

__Purpose__  
It loads the model from the specified file.  

For example,   

    pca.load("./out/PCAModel", dtype = np.float64)

__Return Value__  
It simply returns "self" reference.  

### 8. load_binary(path, dtype)  
__Parameters__  
**_path_**: A string object containing the name of the binary file having model information to be loaded.  
**_dtype_**: It is the data-type of the loaded model with trainig data samples. Currently, expected input data-type is double (float64).  

__Purpose__    
It loads the model from the specified file (having binary data).  

For example,   

    pca.load("./out/PCA_BinaryModel", dtype = np.float64)  

__Return Value__  
It simply returns "self" reference.   

### 9. save(path)  
__Parameters__   
**_path_**: A string object containing the name of the file on which the target model is to be saved.  

__Purpose__  
On success, it writes the model information (components, mean, score, etc.) in the specified file. Otherwise, it throws an exception.  

For example,   

    # To save the pca model
    pca.save("./out/PCAModel")    

This will save the pca model on the path '/out/PCAModel'. It would raise exception if the directory already exists with same name.  

The 'PCAModel' directory has  

**PCAModel**  
|----components  
|----mean  
|----score  
|----singular_values  
|----variance  
|----variance_ratio  

The saved model contains the after-fitted attribute values.  

__Return Value__  
It returns nothing.  

### 10. save_binary(path)  
__Parameters__   
**_path_**: A string object containing the name of the binary file on which the target model is to be saved.  

__Purpose__  
On success, it writes the model information (components, mean, score, etc.) in binary format in the specified file. Otherwise, it throws an exception.  

For example,  

    # To save the pca binary model
    pca.save("./out/PCA_BinaryModel")    

This will save the pca model on the path '/out/PCA_BinaryModel'. It would raise exception if the directory already exists with same name.  

The 'PCA_BinaryModel' directory has  

**PCA_BinaryModel**  
|----**components**  
|----mean  
|----**score**  
|----singular_values  
|----variance  
|----variance_ratio  

The saved binary model contains the after-fitted attribute values.  

__Return Value__  
It returns nothing.  

### 11. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed to this function as dictionary. This dictionary contains 
parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by PCA, used to set parameter values.  

For example,   

    print("get parameters before setting:") 
    print(pca.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    pca.set_params(whiten=True)  
    print("get parameters after setting:") 
    print(pca.get_params())

Output  
     
    get parameters before setting:
    {'copy': True, 'iterated_power': 'auto', 'n_components': 2, 'random_state': None, 
    'svd_solver': 'arpack', 'tol': 0.0, 'whiten': False}
    get parameters after setting:
    {'copy': True, 'iterated_power': 'auto', 'n_components': 2, 'random_state': None, 
    'svd_solver': 'arpack', 'tol': 0.0, 'whiten': True}
    
__Return Value__  
It simply returns "self" reference.  

### 12. transform(X, y = None)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object of int, float or double (float64) type values. It can also be an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data of float or double (float64) type. It has shape **(n_samples, n_features)**.  
**_y_**:  None or any python array-like object (any shape). It is simply ignored in frovedis implementation, like in Scikit-learn.  

__Purpose__  
It applies the dimensionality reduction on input matrix (X).  

For example,  

    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[1.0, 0.0, 7.0, 0.0, 0.0], 
                     [2.0, 0.0, 3.0, 4.0, 5.0], 
                     [4.0, 0.0, 0.0, 6.0, 7.0]], 
                    dtype = np.float64)
    
    # fitting input matrix on PCA object and perform transform 
    from frovedis.mllib.decomposition import PCA
    pca = PCA().fit(mat)
    print(pca.transform(mat))

Output  

    [[-6.50037234  0.28016556]
     [ 1.0441383  -0.75923769]
     [ 5.45623404  0.47907213]]
 
When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,
    
    # loading a sample numpy dense data    
    import numpy as np
    mat = np.matrix([[1.0, 0.0, 7.0, 0.0, 0.0], 
                     [2.0, 0.0, 3.0, 4.0, 5.0], 
                     [4.0, 0.0, 0.0, 6.0, 7.0]], 
                    dtype = np.float64)    
    
    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix.  
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.  
    from frovedis.matrix.dense import FrovedisRowmajorMatrix  
    rmat = FrovedisRowmajorMatrix(mat)  
    
    # Fitting PCA with pre-constructed frovedis-like inputs and perform transform  
    from frovedis.mllib.decomposition import PCA
    pca = PCA().fit(rmat)
    X1 = pca.transform(rmat)
    X1.debug_print()

Output  

    matrix:
    num_row = 3, num_col = 2
    node 0
    node = 0, local_num_row = 3, local_num_col = 2, val = -6.50037 0.280166 1.04414 
    -0.759238 5.45623 0.479072

__Return Value__  

- **When X is python native input:**  
It returns a numpy ndarray of shape **(n_samples, n_components)** and double (float64) type values. It contains the projection of input matrix (X) in the first principal components.  
- **When X is frovedis-like input:**  
It returns a FrovedisRowmajorMatrix instance of shape **(n_samples, n_components)** and double (float64) type values. It contains the projection of input matrix (X) in the first principal components.  

# SEE ALSO  
- **[Introduction to FrovedisRowmajorMatrix](../matrix/rowmajor_matrix.md)**  
- **[Introduction to FrovedisCRSMatrix](../matrix/crs_matrix.md)**  
- **[Latent Dirichlet Allocation in frovedis](./lda.md)**  
