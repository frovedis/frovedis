% FactorizationMachineRegressor

# NAME
 
FactorizationMachineRegressor - A factorization machine is a general-purpose supervised
learning algorithm that can be used for regression tasks. It is an extension of a linear 
model that is designed to capture interactions between features within high dimensional 
sparse datasets.  

# SYNOPSIS

    class frovedis.mllib.fm.FactorizationMachineRegressor(iteration=100, init_stdev=0.1,  
                                                          init_learn_rate=0.01, optimizer=“SGD”,  
                                                          dim=(True,True, 8), reg=(0, 0, 0),  
                                                          batch_size_pernode=100, verbose=0)  

## Public Member Functions

fit(X, y, sample_weight = None)  
predict(X)  
load(fname, dtype = None)  
save(fname)  
score(X, y, sample_weight = None)  
get_params(deep = True)  
set_params(\*\*params)  
debug_print()  
release()  
is_fitted()  

# DESCRIPTION

The FactorizationMachineRegressor (fmr) is a general predictor like SVMs but is also able
to estimate reliable parameters under very high sparsity. The factorization machine 
models all nested variable interactions (comparable to a polynomial kernel
in SVM), but uses a factorized parameterization instead of a dense parametrization 
like in SVMs. We show that the model equation of fmrs can be computed
in linear time and that it depends only on a linear number of parameters. This
allows direct optimization and storage of model parameters without the need of
storing any training data (e.g. support vectors) for prediction.  

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis interface is almost same as libFM. libFM 
is a software implementation for factorization machines that features stochastic 
gradient descent (SGD) and alternating least squares (ALS) optimization as well as 
Bayesian inference using Markov Chain Monte Carlo (MCMC). In this implementation, 
a python client can interact with a frovedis server sending the required python data 
for training at frovedis side. Python data is converted into frovedis compatible 
data internally and the python ML call is linked with the respective frovedis ML 
call to get the job done at frovedis server.  

Python side calls for FactorizationMachineRegressor on the frovedis server. Once the training is 
completed with the input data at the frovedis server, it returns an abstract model with 
a unique model ID to the client python program.  

When predict-like request would be made on the trained model, python program will 
send the same request to the frovedis server. After the request is served at the frovedis 
server, the output would be sent back to the python client.  

## Detailed Description  

### 1. FactorizationMachineRegressor()  

__Parameters__  
**_iteration_**: A positive integer parameter, specifying the maximum number of iteration
count. (Default: 100)  
**_init\_stdev_**: A positive double parameter specifying the standard deviation which is
used to initialize the model parameter of 2-way factors (Default: 0.1)  
**_init\_learn\_rate_**: A double parameter containing the learning rate for SGD
optimizer. (Default: 0.01)  
It should be in range from 0.00001 to 1.0.  
**_optimizer_**: A string object parameter that specifies which algorithms minimize or maximize a Loss
function E(x) using its Gradient values with respect to the parameters.  (Default: 'SGD')  
Only 'SGD' is supported.  
**_dim_**: A tuple that specifies three important parameters with default values- (True, True,8):  
- **_global\_bias_**: A boolean value that represents a switch to use bias. Currently, this parameter
is not used in Frovedis implementation.  
- **_dim\_one\_interactions_** : A boolean value that represents a switch to use 1-way interaction.  
- **_dim\_factors\_no_** : A positive integer that represents the dimension of 2-way interaction or 
number of factors that are used for pairwise interactions.  

When any of the three is None (not specified explicitly), then user experiences an Error.  
**_reg_**: An tuple of values that specifies three important parameters with default values- 
(Default: (0, 0, 0))  
- **_regularization\_intercept_** : A positive integer that represents the regularization 
parameters of intercept or bias regularization.  
- **_regularization\_one\_interactions_** : A positive integer that represents the switch 
to use 1-way regularization.  
- **_regularization\_factors\_no_** : A positive integer that represents the dimension of 
2-way interaction or number of factors that are used for pairwise regularization.  

When any of the three is None (not specified explicitly), then user experiences an Error.   
**_batch\_size\_pernode_**: A positive integer parameter specifies the size of minibatch
processed by one node. (Default: 100)  
**_verbose_**: An integer parameter specifying the log level to use. Its value is set as 0 
by default (for INFO mode). But it can be set to 1 (for DEBUG mode) or 2 (for TRACE mode) for 
getting training time logs from frovedis server.  

__Purpose__  
It initializes a FactorizationMachineRegressor object with the given parameters.   
__Return Value__    
It simply returns "self" reference. 

### 2. fit(X, y, sample_weight=None)  
__Parameters__   
**_X_**: A scipy sparse matrix or an instance of FrovedisCRSMatrix of float or double (float64) 
type. It has shape **(n_samples, n_features)**.  
_**y**_: Any python array-like object or an instance of FrovedisDvector containing the target values.
It has shape **(n_samples,)**.  
**_sample\_weight_**: A python ndarray containing the intended weights for each input samples 
and it should be the shape of **(n_samples,)**. (Default: None)  
When it is None (not specified explicitly), an uniform weight vector is assigned on each input 
sample.  

__Purpose__  
It accepts the training matrix (X) with labels (y) and trains a FactorizationMachineRegressor model.  

For example,   

    # loading sample data   
    row = np.array([0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5])
    col = np.array([0, 1, 4, 0, 1, 2, 4, 1, 2, 3, 2,3,4,5, 0, 1, 3, 4, 3, 5])
    data = np.asarray([ 2.0, 1.0,1.0, 1.0, 3.0,1.0, 1.0, 1.0, 2.0,1.0, 1.0, 3.0,
                        1.0,1.0,1.0, 1.0, 1.0, 3.0,1.0, 1.0])
    csr = csr_matrix((data, (row, col)), shape = (6, 6))
    lbl = [10.0, 10.0, 10.0, 20.0, 10.0, 20.0]
    
    # fitting input data on FactorizationMachineRegressor object
    from frovedis.mllib.fm import FactorizationMachineRegressor
    fmr = FactorizationMachineRegressor()
    fmr.fit(csr, lbl)

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,   

    # loading sample data
    row = np.array([0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5])
    col = np.array([0, 1, 4, 0, 1, 2, 4, 1, 2, 3, 2,3,4,5, 0, 1, 3, 4, 3, 5])
    data = np.asarray([ 2.0, 1.0,1.0, 1.0, 3.0,1.0, 1.0, 1.0, 2.0,1.0, 1.0, 3.0,
                        1.0,1.0,1.0, 1.0, 1.0, 3.0,1.0, 1.0])
    csr = csr_matrix((data, (row, col)), shape = (6, 6))
    lbl = [10.0, 10.0, 10.0, 20.0, 10.0, 20.0]
    
    # Since "csr" is scipy sparse data, we have created FrovedisCRSMatrix. 
    from frovedis.matrix.crs import FrovedisCRSMatrix
    from frovedis.matrix.dvector import FrovedisDvector
    cmat = FrovedisCRSMatrix(csr)
    dlbl = FrovedisDvector(lbl)
    
    # fitting input data on FactorizationMachineRegressor object
    from frovedis.mllib.fm import FactorizationMachineRegressor
    fmr = FactorizationMachineRegressor()
    fmr.fit(cmat, dlbl)

__Return Value__  
It simply returns "self" reference.   

### 3. predict(X)  
__Parameters__  
**_X_**: A scipy sparse matrix or an instance of FrovedisCRSMatrix of float or double (float64) 
type. It has shape **(n_samples, n_features)**.  

__Purpose__  
It accepts the test feature matrix (X) in order to make prediction on the trained
model at frovedis server.

For example,  

    # predicting on FactorizationMachineRegressor model
    fmr.predict(csr)
    
Output  

    [10.00277618 20.6134937  13.81870647 26.75876098 33.13530746  3.03828379]

Like in fit(), frovedis-like input can be used to speed-up the prediction making on the trained 
model at server side.  

For example,  

    # Since "csr" is scipy sparse data, we have created FrovedisCRSMatrix. 
    from frovedis.matrix.crs import FrovedisCRSMatrix
    cmat = FrovedisCRSMatrix(csr)
     
    # predicting on FactorizationMachineRegressor model using pre-constructed input
    fmr.predict(cmat)

Output  

    [10.00277618 20.6134937  13.81870647 26.75876098 33.13530746  3.03828379]
    
__Return Value__  
It returns a numpy array of double (float64) type containing the predicted outputs. It is of 
shape **(n_samples,)**.  

### 4. load(fname, dtype = None)
__Parameters__   
**_fname_**:  A string object containing the name of the file having model information to be loaded.  
**_dtype_**: A data-type is inferred from the input data. Currently, expected input data-type is either 
float or double (float64). (Default: None)  

__Purpose__   
Currently, this method is not supported for FactorizationMachineRegressor. It is simply kept in 
FactorizationMachineRegressor module to maintain uniform interface like other estimators in frovedis.  

__Return Value__  
It simply raises an AttributeError.  

### 5. score(X, y, sample_weight = None)  
__Parameters__   
**_X_**: A scipy sparse matrix or an instance of FrovedisCRSMatrix of float or double (float64) 
type. It has shape **(n_samples, n_features)**.  
_**y**_: Any python array-like object containing the target values. It has shape **(n_samples,)**.  
**_sample\_weight_**: A python ndarray containing the intended weights for each input samples 
and it should be the shape of **(n_samples,)**. (Default: None)  
When it is None (not specified explicitly), an uniform weight vector is assigned on each input 
sample.  

__Purpose__  
Calculate the root mean square value on the given test data and labels i.e. R2(r-squared) of 
self.predict(X) wrt. y.  

The coefficient 'R2' is defined as (1 - (u/v)),  
where 'u' is the residual sum of squares ((y_true - y_pred) ** 2).sum() and  
'v' is the total sum of squares ((y_true - y_true.mean()) ** 2).sum().  

The best possible score is 1.0 and it can be negative (because the model can be arbitrarily worse). 
A constant model that always predicts the expected value of y, disregarding the input features, 
would get a R2 score of 0.0.  

For example,  

    fmr.score(csr, lbl)

Output

    -0.10

__Return Value__  
It returns an accuracy score of double (float64) type.  

### 6. save(fname)
__Parameters__   
**_fname_**: A string object containing the name of the file on which the target 
model is to be saved.  

__Purpose__  
On success, it writes the model information(metadata and model) in the 
specified file as little-endian binary data. Otherwise, it throws an exception.  

For example,   

    # To save the FactorizationMachineRegressor model
    fmr.save("./out/FMRModel")

The FMRModel contains below directory structure:  
**FMRModel**  
    |------metadata  
    |------model    
 
'metadata' represents the detail about model_kind and datatype of training vector.  
Here, the model file contains information about trained model in binary format.  

This will save the FactorizationMachineRegressor model on the path ‘/out/FMRModel’. 
It would raise exception if the directory already exists with same name.  

__Return Value__  
It returns nothing.   

### 7. get_params(deep = True)  

__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If True, 
it will return the parameters for an estimator and contained subobjects that are estimators. (Default: True)  

__Purpose__    
This method belongs to the BaseEstimator class inherited by FactorizationMachineRegressor. 
It is used to get parameters and their values of FactorizationMachineRegressor class.  

For example, 
 
    print(fmr.get_params())

Output  

    {'batch_size_pernode': 100, 'dim': (True, True, 8), 'init_learn_rate': 0.01, 
    'init_stdev': 0.1, 'iteration': 100, 'optimizer': 'SGD', 'reg': (False, False, 0), 
    'verbose': 0}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 8. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed this function as dictionary. This dictionary 
contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by FactorizationMachineRegressor, 
used to set parameter values.  

For example,   

    print("Get parameters before setting:") 
    print(fmr.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    fmr.set_params(iteration=200) 
    print("Get parameters after setting:") 
    print(fmr.get_params())

Output  
        
    Get parameters before setting: {'batch_size_pernode': 100, 'dim': (True, True, 8), 
    'init_learn_rate': 0.01, 'init_stdev': 0.1, 'iteration': 100, 'optimizer': 'SGD',
    'reg': (False, False, 0), 'verbose': 0}
    Get parameters before setting: {'batch_size_pernode': 100, 'dim': (True, True, 8), 
    'init_learn_rate': 0.01, 'init_stdev': 0.1, 'iteration': 200, 'optimizer': 'SGD', 
    'reg': (0, 0, 0), 'verbose': 0}
    
__Return Value__  
It simply returns "self" reference.  

### 9. debug_print()

__Purpose__   
Currently, this method is not supported for FactorizationMachineRegressor. It is simply kept in 
FactorizationMachineRegressor module to maintain uniform interface like other estimators in frovedis.  

__Return Value__  
It simply raises an AttributeError.  

### 10. release()  

__Purpose__  
It can be used to release the in-memory model at frovedis server.  

For example,
 
    fmr.release()

This will reset the after-fit populated attributes to None, along with releasing server 
side memory.  

__Return Value__  
It returns nothing.   

### 11. is_fitted()  

__Purpose__  
It can be used to confirm if the model is already fitted or not. In case, predict() is used 
before training the model, then it can prompt the user to train the model first.    

__Return Value__  
It returns 'True', if the model is already fitted otherwise, it returns 'False'.  

# SEE ALSO  
crs_matrix, dvector, fm_classifier