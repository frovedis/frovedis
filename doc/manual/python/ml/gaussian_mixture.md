% GaussianMixture

# NAME  

GaussianMixture - It is a representation of a Gaussian mixture model probability 
distribution. This class allows to estimate the parameters of a Gaussian mixture distribution.  

# SYNOPSIS  

    class frovedis.mllib.mixture.gmm.GaussianMixture(n_components=1, covariance_type='full', 
                                                     tol=1e-3, reg_covar=1e-6, max_iter=100, 
                                                     n_init=1, init_params='kmeans', 
                                                     weights_init=None, means_init=None, 
                                                     precisions_init=None, random_state=None,  
                                                     warm_start=False, verbose=0, 
                                                     verbose_interval=10)  

## Public Member Functions  

fit(X, y = None)  
fit_predict(X, y = None)  
predict(X)  
predict_proba(X)  
sample(n_samples = 1)  
score(X, y = None)  
score_samples(X)  
get_params(deep = True)  
set_params(\*\*params)  
load(fname, dtype = None)  
save(fname)  
bic(X)  
aic(X)  
debug_print()  
release()  
is_fitted()  

# DESCRIPTION  
A Gaussian mixture model is a probabilistic model that assumes all the data points 
are generated from a mixture of a finite number of Gaussian distributions with unknown 
parameters. One can think of mixture models as generalizing k-means clustering to 
incorporate information about the covariance structure of the data as well as the 
centers of the latent Gaussians.  

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis interface is almost same as Scikit-learn 
GaussianMixture interface, but it doesn’t have any dependency with Scikit-learn. It 
can be used simply even if the system doesn’t have Scikit-learn installed. Thus, in 
this implementation, a python client can interact with a frovedis server sending the 
required python data for training at frovedis side. Python data is converted into 
frovedis compatible data nternally and the python ML call is linked with the 
respective frovedis ML call to get the job done at frovedis server.  

Python side calls for GaussianMixture on the frovedis server. Once the training is completed 
with the input data at the frovedis server, it returns an abstract model with a unique 
model ID to the client python program.  

When prediction-like request would be made on the trained model, the python program will 
send the same request to the frovedis server. After the request is served at the frovedis 
server, the output would be sent back to the python client.  

## Detailed Description  

### 1. GaussianMixture()  

__Parameters__  
_**n\_components**_: A positive integer parameter specifying the number of mixture components. (Default: 1)  
_**covariance\_type**_: A string object parameter specifying the type of covariance 
parameters to use.  
Currently, **'full'** is supported in frovedis. In 'full' covariance type, each component has its 
own general covariance matrix. (Default: 'full')  
_**tol**_: Zero or a positive double (float64) parameter specifying the convergence tolerance. 
EM (expectation-maximization) iterations will stop when the lower bound average gain is 
below this threshold. (Default: 1e-3)  
_**reg\_covar**_: An unused parameter. (Default: 1e-6)  
_**max\_iter**_: A positive integer parameter specifying the number of EM (expectation-maximization) 
iterations to perform. (Default: 100)  
_**n\_init**_: A positive integer parameter specifying the number of initializations to perform. (Default: 1)  
If it is None (not specified explicitly), it will be set as 1.  
_**init\_params**_: A string object parameter specifying the method used to initialize the 
weights, the means and the precisions. (Default: 'kmeans')  
Must be one of the following:  
- **'kmeans'** : responsibilities are initialized using kmeans.  
- **'random'** : responsibilities are initialized randomly.  

_**weights\_init**_: An unused parameter. (Default: None)  
_**means\_init**_: An unused parameter. (Default: None)  
_**precisions\_init**_: An unused parameter. (Default: None)  
_**random\_state**_: An integer, float parameter or a RandomState instance that controls 
the random seed given to the method chosen to initialize the parameters. (Default: None)  
If it is None (not specified explicitly) or a RandomState instance, it will be set as 0.  
_**warm\_start**_: An unused parameter. (Default: False)  
**_verbose_**: An integer parameter specifying the log level to use. Its value is set as 0 
by default (for INFO mode). But it can be set to 1 (for DEBUG mode) or 2 (for TRACE mode) for 
getting training time logs from frovedis server.  
_**verbose\_interval**_: An unused parameter. (Default: 10)  

__Attribute__  
_**weights\_**_: It is a python ndarray, containing double (float64) typed values and 
has shape **(n_components,)**. It stores the weights of each mixture components.  
_**covariances\_**_: It is a python ndarray, containing double (float64) typed values. It 
stores the covariance of each mixture component. The shape depends on **covariance_type**:  
- if **'full'**, then the shape is **(n_components, n_features, n_features)**.  

_**means\_**_: It is a python ndarray, containing double (float64) typed values and has 
shape **(n_components, n_features)**. It stores the mean of each mixture component.  
_**converged\_**_: A boolean value. If True, then convergence was reached in fit(), otherwise 
it will be False.  
_**n\_iter\_int**_: An integer value specifying the number of step used by the best fit 
of EM to reach the convergence.  
_**lower\_bound\_**_: A float value specifying the lower bound value on the log-likelihood 
(of the training data with respect to the model) of the best fit of EM.  

__Purpose__  
It initializes a GaussianMixture object with the given parameters.  

The parameters: "reg_covar", "weights_init, "means_init", "precisions_init", "warm_start", 
and "verbose_interval" are simply kept to make the interface uniform to Scikit-learn 
GaussianMixture module. They are not used anywhere within the frovedis implementation.  

__Return Value__    
It simply returns "self" reference.  

### 2. fit(X,  y = None)  
__Parameters__   
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data. It has 
shape **(n_samples, n_features)**. During training, **n_samples >= n_components**.  
**_y_**: None or any python array-like object (any shape). It is simply ignored in frovedis 
implementation, like in Scikit-learn.  

__Purpose__  
It estimates the model parameters with the EM algorithm.  

The method fits the model **'n_init'** times and sets the parameters with which the model has 
the largest likelihood or lower bound. Within each trial, the method iterates between E-step 
and M-step for **'max_iter'** times until the change of likelihood or lower bound is less than 
**'tol'**.  

For example,  

    # loading sample matrix dense data
    # train_mat = np.array([[1., 2.], 
                           [1., 4.], 
                           [1., 0.], 
                           [10., 2.], 
                           [10., 4.], 
                           [10., 0.]])
    
    # fitting input matrix on GaussianMixture object
    from frovedis.mllib.mixture import GaussianMixture
    gmm_model = GaussianMixture(n_components = 2)
    gmm_model.fit(train_mat)  

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.   

For example,   

    # loading sample matrix dense data
    # train_mat = np.array([[1., 2.], 
                           [1., 4.], 
                           [1., 0.], 
                           [10., 2.], 
                           [10., 4.], 
                           [10., 0.]])
    
    # Since "train_mat" is numpy dense data, we have created FrovedisRowmajorMatrix.
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(train_mat)
    
    # fitting GaussianMixture object with pre-constructed input
    from frovedis.mllib.mixture import GaussianMixture
    gmm_model = GaussianMixture(n_components = 2)
    gmm_model.fit(rmat)  
    
__Return Value__  
It simply returns "self" reference.  

### 3. fit_predict(X, y = None)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data. It has 
shape **(n_samples, n_features)**. During training, **n_samples >= n_components**.  
**_y_**: None or any python array-like object (any shape). It is simply ignored in frovedis 
implementation, like in Scikit-learn.  

__Purpose__    
It estimates the model parameters using X and predict the labels for X.  

The method fits the model **'n_init'** times and sets the parameters with which the model 
has the largest likelihood or lower bound. Within each trial, the method iterates between 
E-step and M-step for **'max_iter'** times until the change of likelihood or lower bound is 
less than **'tol'**. After fitting, it predicts the most probable label for the input 
data points.  

For example,   

    # loading sample matrix dense data
    # train_mat = np.array([[1., 2.], 
                           [1., 4.], 
                           [1., 0.], 
                           [10., 2.], 
                           [10., 4.], 
                           [10., 0.]])
    
    # fitting input matrix on GaussianMixture object and perform predictions
    from frovedis.mllib.mixture import GaussianMixture
    gmm_model = GaussianMixture(n_components = 2)
    print(gmm_model.fit_predict(train_mat))  
    
Output

    [0 0 0 1 1 1]
 
Like in fit() frovedis-like input can be used to speed-up training at server side.  

For example,   

    # loading sample matrix dense data
    # train_mat = np.array([[1., 2.], 
                           [1., 4.], 
                           [1., 0.], 
                           [10., 2.], 
                           [10., 4.], 
                           [10., 0.]])
    
    # Since "train_mat" is numpy dense data, we have created FrovedisRowmajorMatrix.
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(train_mat)

    # fitting GaussianMixture object with pre-constructed input and perform predictions
    from frovedis.mllib.mixture import GaussianMixture
    gmm_model = GaussianMixture(n_components = 2)
    print(gmm_model.fit_predict(rmat))  
    
Output

    [0 0 0 1 1 1]

__Return Value__  
It returns a numpy array of int64 type values containing the components labels. It has 
a shape **(n_samples,)**.  

### 4. predict(X)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data. It has 
shape **(n_samples, n_features)**.  

__Purpose__  

It predict the labels for the data samples in X using trained model.  

For example,  

    # loading sample matrix dense data
    # train_mat = np.array([[1., 2.], 
                           [1., 4.], 
                           [1., 0.], 
                           [10., 2.], 
                           [10., 4.], 
                           [10., 0.]])
    
    # predicting on GaussianMixture model
    from frovedis.mllib.mixture import GaussianMixture
    gmm_model = GaussianMixture(n_components = 2).fit(train_mat)
    print(gmm_model.predict(train_mat))  
    
Output

    [0 0 0 1 1 1]
 
Like in fit(), frovedis-like input can be used to speed-up the prediction making on 
the trained model at server side.  

For example,   

    # loading sample matrix dense data
    # train_mat = np.array([[1., 2.], 
                           [1., 4.], 
                           [1., 0.], 
                           [10., 2.], 
                           [10., 4.], 
                           [10., 0.]])
    
    # Since "train_mat" is numpy dense data, we have created FrovedisRowmajorMatrix.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(train_mat)

    # predicting on GaussianMixture model using pre-constructed input
    from frovedis.mllib.mixture import GaussianMixture
    gmm_model = GaussianMixture(n_components = 2).fit(rmat)
    print(gmm_model.predict(rmat))  
    
Output

    [0 0 0 1 1 1]

__Return Value__  
It returns a numpy array of int64 type values containing the components labels. It has 
a shape **(n_samples,)**.  

### 5. sample(n_samples = 1)  
__Parameters__  
_**n\_samples**_: A positive integer value that specifies the number of samples to 
generate. (Default: 1)  

__Purpose__  
Currently this method is not supported for GaussianMixture in frovedis.  

__Return Value__  
It simply raises a NotImplementedError.  

### 6. score(X)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data. It has 
shape **(n_samples, n_features)**.  

__Purpose__  
It computes the per-sample average log-likelihood of the given data X.  

For example,

    # calculate log-likelihood on given test data X
    gmm_model.score(train_mat)

Output

    3.386

__Return Value__  
It returns a score of double (float64) type.  

### 7. score_samples(X)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data. It has 
shape **(n_samples, n_features)**.  

__Purpose__  
It computes the log-likelihood of each sample.  

For example,

    gmm_model.score_samples(train_mat)

Output

    [3.88631622 3.1363165  3.1363165  3.88631622 3.1363165  3.1363165 ]

__Return Value__  
It returns a numpy array of double (float64) type containing log-likelihood of each sample 
in 'X' under the current model. It has a shape **(n_samples,)**.  

### 8. get_params(deep = True)  
__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an 
estimator. If True, will return the parameters for an estimator and contained subobjects 
that are estimators. (Default: True)  

__Purpose__    
This method belongs to the BaseEstimator class inherited by GaussianMixture. It is used 
to get parameters and their values of GaussianMixture class.  

For example, 
 
    print(gmm_model.get_params())

Output  

    {'covariance_type': 'full', 'init_params': 'kmeans', 'max_iter': 100, 'means_init': None, 
    'n_components': 2, 'n_init': 1, 'precisions_init': None, 'random_state': None, 
    'reg_covar': None, 'tol': 0.001, 'verbose': 0, 'verbose_interval': None, 'warm_start': None, 
    'weights_init': None}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 9. set_params(\*\*params)  
__Parameters__   
_**\*\*params**_: All the keyword arguments are passed this function as dictionary. This 
dictionary contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by GaussianMixture, used to 
set parameter values.  

For example,   

    print("get parameters before setting:")
    print(gmm_model.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    gmm_model.set_params(n_components = 4)
    print("get parameters after setting:")
    print(gmm_model.get_params())

Output  
     
    get parameters before setting:
    {'covariance_type': 'full', 'init_params': 'kmeans', 'max_iter': 100, 'means_init': None, 
    'n_components': 2, 'n_init': 1, 'precisions_init': None, 'random_state': None, 
    'reg_covar': None, 'tol': 0.001, 'verbose': 0, 'verbose_interval': None, 'warm_start': None, 
    'weights_init': None}
    get parameters after setting:
    {'covariance_type': 'full', 'init_params': 'kmeans', 'max_iter': 100, 'means_init': None, 
    'n_components': 4, 'n_init': 1, 'precisions_init': None, 'random_state': None, 
    'reg_covar': None, 'tol': 0.001, 'verbose': 0, 'verbose_interval': None, 'warm_start': None, 
    'weights_init': None}

__Return Value__  
It simply returns "self" reference.  

### 10. load(fname, dtype = None)
__Parameters__   
_**fname**_: A string object containing the name of the file having model 
information to be loaded.    
_**dtype**_: A data-type is inferred from the input data. Currently, 
expected input data-type is either float or double (float64). (Default: None)  

__Purpose__    
It loads the model from the specified file(having little-endian binary data). 

For example,  

    # loading the same model
    gmm_model.load("./out/MyGmmModel",dtype = np.float64)

__Return Value__  
It simply returns "self" reference.  

### 11. save(fname)  
__Parameters__  
_**fname**_: A string object containing the name of the file on which the target 
model is to be saved.  

__Purpose__  
On success, it writes the model information (metadata and model) in the specified file 
as little-endian binary data. Otherwise, it throws an exception.  

For example,  

    # saving the model
    gmm_model.save("./out/MyGmmModel")

This will save the random forest classifier model on the path "/out/MyGmmModel". It would 
raise exception if the directory already exists with same name.  

The MyGmmModel contains below directory structure:  

**MyGmmModel**  
    |------metadata  
    |------model  

'metadata' represents the detail about n_components, n_features, converged_, n_iter_, 
lower_bound_, model_kind and datatype of training vector.  
Here, the 'model' file contains information about gaussian mixture model in binary format.  

__Return Value__  
It returns nothing.  

### 12. bic(X)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data. It has 
shape **(n_samples, n_features)**.  

__Purpose__  
It is the Bayesian information criterion for the current model on the input X.  

For example,  

    gmm_model.bic(train_mat)

Output  

    -20.926

__Return Value__  
It returns a bayesian information criterion of double (float64) type.  

### 13. aic(X)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data. It has 
shape **(n_samples, n_features)**.  

__Purpose__  
It is the Akaike information criterion for the current model on the input X.  

For example,  

    gmm_model.aic(train_mat)

Output  

    -18.636

__Return Value__  
It returns an akaike information criterion of double (float64) type.  

### 14. debug_print()

__Purpose__  
It shows the target model information on the server side user terminal. It is mainly 
used for debugging purpose.   

For example,  

    gmm_model.debug_print()
    
Output

    Gaussian Mixture Model:
    Means:
    node = 0, local_num_row = 2, local_num_col = 2, val = 1 2 10 2

    Weights:
    node = 0, local_num_row = 2, local_num_col = 1, val = 0.5 0.5

    Covariances:
    node = 0, local_num_row = 2, local_num_col = 4, val = 0 0 0 2.66667 0 0 0 2.66667
    
It displays the information on the trained model such as means, weights, covariances which 
is currently present on the server.  

__Return Value__  
It returns nothing.   

### 15. release()

__Purpose__    
It can be used to release the in-memory model at frovedis server.   

For example,  

    gmm_model.release()
    
This will reset the after-fit populated attributes (like weights_, means_, etc.) to 
None, along with releasing server side memory.  

__Return Value__  
It returns nothing.   

### 16. is_fitted()  

__Purpose__  
It can be used to confirm if the model is already fitted or not. In case, predict() 
is used before training the model, then it can prompt the user to train the model first.  

__Return Value__  
It returns 'True', if the model is already fitted otherwise, it returns 'False'.  

## SEE ALSO  
rowmajor_matrix, crs_matrix, kmeans