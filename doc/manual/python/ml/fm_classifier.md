% FactorizationMachineClassifier

# NAME
 
FactorizationMachineClassifier - A factorization machine is a general-purpose supervised
learning algorithm that can be used for classification tasks. It is an extension of a 
linear model that is designed to capture interactions between features within high 
dimensional sparse datasets.  

# SYNOPSIS

    class frovedis.mllib.fm.FactorizationMachineClassifier(iteration=100, init_stdev=0.1,  
                                                           init_learn_rate=0.01, optimizer=“SGD”,  
                                                           dim=(True,True, 8), reg=(0, 0, 0),  
                                                           batch_size_pernode=100, verbose=0)  

## Public Member Functions

fit(X, y, sample_weight = None)  
predict(X)  
load(fname, dtype = None)  
score(X, y, sample_weight = None)  
save(fname) 
get_params(deep = True)  
set_params(\*\*params)  
debug_print()  
release()  
is_fitted()  

# DESCRIPTION

The FactorizationMachineClassifier (fmc) is a general predictor like SVMs but is also able
to estimate reliable parameters under very high sparsity. The factorization machine 
models all nested variable interactions (comparable to a polynomial kernel
in SVM), but uses a factorized parameterization instead of a dense parametrization 
like in SVMs. We show that the model equation of fmcs can be computed
in linear time and that it depends only on a linear number of parameters. This
allows direct optimization and storage of model parameters without the need of
storing any training data (e.g. support vectors) for prediction. **Frovedis supports 
both binary and multinomial labels.**  

During training, the input **X** is the training data and **y** are their corresponding 
label values (Frovedis supports any values as for labels, but internally it encodes the
input binary labels to -1 and 1, before training at Frovedis server) which we 
want to predict.  

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis interface is almost same as libFM. libFM 
is a software implementation for factorization machines that features stochastic 
gradient descent (SGD) and alternating least squares (ALS) optimization as well as 
Bayesian inference using Markov Chain Monte Carlo (MCMC). In this implementation, 
a python client can interact with a frovedis server sending the required python data 
for training at frovedis side. Python data is converted into frovedis compatible 
data internally and the python ML call is linked with the respective frovedis ML 
call to get the job done at frovedis server.  

Python side calls for FactorizationMachineClassifier on the frovedis server. Once the training is 
completed with the input data at the frovedis server, it returns an abstract model with 
a unique model ID to the client python program.  

When predict-like request would be made on the trained model, python program will 
send the same request to the frovedis server. After the request is served at the frovedis 
server, the output would be sent back to the python client.  

## Detailed Description  

### 1. FactorizationMachineClassifier()  

__Parameters__  
**_iteration_**: A positive integer parameter, specifying the maximum number of iteration
count. (Default: 100)  
**_init\_stdev_**: A positive double parameter specifying the standard deviation which is
used to initialize the model parameter of 2-way factors. (Default: 0.1)  
**_init\_learn\_rate_**: A double parameter containing the learning rate for SGD
optimizer. (Default: 0.01)  
It should be in range from 0.00001 to 1.0.  
**_optimizer_**: A string object parameter that specifies which algorithms minimize or maximize a Loss
function E(x) using its gradient values with respect to the parameters.  (Default: 'SGD')  
Only 'SGD' is supported.  
**_dim_**: A tuple that specifies three important parameters with default values- (True, True,8):  
- **_global\_bias_**: A boolean value that represents a switch to use bias. Currently, this parameter
is not used in Frovedis implementation.  
- **_dim\_one\_interactions_** : A boolean value represents a switch to use 1-way interaction.  
- **_dim\_factors\_no_** : A positive integer that represents the dimension of 2-way interaction or 
number of factors that are used for pairwise interactions.  

When any of the three is None (not specified explicitly), then user experiences an Error.  
**_reg_**: A tuple that specifies three important parameters with default values- 
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

__Attributes__  
**_classes\__**: A numpy array of long (int64) type value that specifies unique labels given 
to the classifier during training. It has shape **(n_classes,)**, where n_classes is the 
unique number of classes.  

__Purpose__  
It initializes a FactorizationMachineClassifier object with the given parameters.  

__Return Value__    
It simply returns "self" reference. 

### 2. fit(X, y, sample_weight=None)  
__Parameters__   
**_X_**: A scipy sparse matrix or an instance of FrovedisCRSMatrix of float or double (float64) type. 
It has shape **(n_samples, n_features)**.  
_**y**_: Any python array-like object or an instance of FrovedisDvector containing the target labels.
It has shape **(n_samples,)**.  
**_sample\_weight_**: A python ndarray containing the intended weights for each input samples 
and it should be the shape of **(n_samples,)**. (Default: None)  
When it is None (not specified explicitly), an uniform weight vector is assigned on each input 
sample.    

__Purpose__  
It accepts the training matrix (X) with labels (y) and trains a FactorizationMachineClassifier model.  

For example,   

    # loading sample data   
    row = np.array([0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5])
    col = np.array([0, 1, 4, 0, 1, 2, 4, 1, 2, 3, 2,3,4,5, 0, 1, 3, 4, 3, 5])
    data = np.asarray([ 2.0, 1.0,1.0, 1.0, 3.0,1.0, 1.0, 1.0, 2.0,1.0, 1.0, 3.0,
                        1.0,1.0,1.0, 1.0, 1.0, 3.0,1.0, 1.0])
    csr = csr_matrix((data, (row, col)), shape = (6, 6))
    lbl = [10.0, 10.0, 10.0, 20.0, 10.0, 20.0]
    
    # fitting input data on FactorizationMachineClassifier object
    from frovedis.mllib.fm import FactorizationMachineClassifier
    fmc = FactorizationMachineClassifier()
    fmc.fit(csr, lbl)

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
    
    # fitting input data on FactorizationMachineClassifier object
    from frovedis.mllib.fm import FactorizationMachineClassifier
    fmc = FactorizationMachineClassifier()
    fmc.fit(cmat, dlbl)

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

    # predicting on FactorizationMachineClassifier model
    fmc.predict(csr)
    
Output  

    [20. 20. 20. 20. 20. 20.]

Like in fit(), frovedis-like input can be used to speed-up the prediction making on the trained 
model at server side.  

For example,  

    # Since "csr" is scipy sparse data, we have created FrovedisCRSMatrix. 
    from frovedis.matrix.crs import FrovedisCRSMatrix
    cmat = FrovedisCRSMatrix(csr)
     
    # predicting on FactorizationMachineClassifier model using pre-constructed input
    fmc.predict(cmat)

Output  

    [20. 20. 20. 20. 20. 20.]
    
__Return Value__  
It returns a numpy array of double (float64) type containing the predicted outputs. It is of 
shape **(n_samples,)**.  

### 4. load(fname, dtype = None)
__Parameters__   
**_fname_**:  A string object containing the name of the file having model information to be loaded.  
**_dtype_**: A data-type is inferred from the input data. Currently, expected input data-type is either 
float or double (float64). (Default: None)  

__Purpose__   
Currently, this method is not supported for FactorizationMachineClassifier. It is simply kept in 
FactorizationMachineClassifier module to maintain uniform interface like other estimators in frovedis.  

__Return Value__  
It simply raises an AttributeError.  

### 5. score(X, y, sample_weight = None)  
__Parameters__   
**_X_**: A scipy sparse matrix or an instance of FrovedisCRSMatrix of float or double (float64) type. 
It has shape **(n_samples, n_features)**.  
**_y_**: Any python array-like object containing class labels. It is of shape **(n_samples,)**.  
**_sample\_weight_**: A python ndarray containing the intended weights for each input samples 
and it should be the shape of **(n_samples,)**. (Default: None)  
When it is None (not specified explicitly), an uniform weight vector is assigned on each input 
sample.  

__Purpose__  
Calculate mean accuracy on the given test data and labels i.e. mean accuracy of self.predict(X) wrt. y.  

For example,  

    fmc.score(csr, lbl)

Output

    0.83

__Return Value__  
It returns an accuracy score of double (float64) type.  

### 6. save(fname)
__Parameters__   
**_fname_**: A string object containing the name of the file on which the target model is to be saved.  

__Purpose__  
On success, it writes the model information(label_map, metadata and model) in the 
specified file as little-endian binary data. Otherwise, it throws an exception.  

For example,   

    # To save the FactorizationMachineClassifier model
    fmc.save("./out/FMCModel")

The FMCModel contains below directory structure:  
**FMCModel**  
    |------label_map  
    |------metadata  
    |------model    
 
'label_map' contains information about labels mapped with their encoded value.  
'metadata' represents the detail about model_kind and datatype of training vector.  
Here, the model file contains information about trained model in binary format.  

This will save the FactorizationMachineClassifier model on the path ‘/out/FMCModel’. 
It would raise exception if the directory already exists with same name.  

__Return Value__  
It returns nothing.   


### 7. get_params(deep = True)  

__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If True, 
it will return the parameters for an estimator and contained subobjects that are estimators. (Default: True)  

__Purpose__    
This method belongs to the BaseEstimator class inherited by FactorizationMachineClassifier. 
It is used to get parameters and their values of FactorizationMachineClassifier class.  

For example, 
 
    print(fmc.get_params())

Output  

    {'batch_size_pernode': 100, 'dim': (True, True, 8), 'init_learn_rate': 0.01, 
    'init_stdev': 0.1, 'iteration': 100, 'optimizer': 'SGD', 'reg': (0, 0, 0), 
    'verbose': 0}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 8. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed this function as dictionary. This dictionary 
contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by FactorizationMachineClassifier, 
used to set parameter values.  

For example,   

    print("Get parameters before setting:") 
    print(fmc.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    fmc.set_params(iteration = 200) 
    print("Get parameters after setting:") 
    print(fmc.get_params())

Output  
     
    Get parameters before setting: 
    {'batch_size_pernode': 100, 'dim': (True, True, 8), 'init_learn_rate': 0.01, 
    'init_stdev': 0.1, 'iteration': 100, 'optimizer': 'SGD', 'reg': (0, 0, 0), 
    'verbose': 0}
    Get parameters after setting: 
    {'batch_size_pernode': 100, 'dim': (True, True, 8), 'init_learn_rate': 0.01, 
    'init_stdev': 0.1, 'iteration': 200, 'optimizer': 'SGD', 'reg': (0, 0, 0), 
    'verbose': 0}
    
__Return Value__  
It simply returns "self" reference.  

### 9. debug_print()

__Purpose__   
Currently, this method is not supported for FactorizationMachineClassifier. It is simply kept in 
FactorizationMachineClassifier module to maintain uniform interface like other estimators in frovedis.  

__Return Value__  
It simply raises an AttributeError.  

### 10. release()  

__Purpose__  
It can be used to release the in-memory model at frovedis server.  

For example,
 
    fmc.release()

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
- **[Introduction to FrovedisCRSMatrix](../matrix/crs_matrix.md)**  
- **[Introduction to FrovedisDvector](../matrix/dvector.md)**  
- **[Factorization Machine Regressor in Frovedis](./fm_regressor.md)**  
