% SGDRegressor

# NAME

SGDRegressor - A regression algorithm used to predict the labels 
with various loss functions. This estimator implements regularized linear 
models with stochastic gradient descent (SGD) learning.  

# SYNOPSIS

    class frovedis.mllib.linear_model.SGDRegressor(loss='squared_loss', penalty='l2', alpha=0.0001,  
                                                   l1_ratio=0.15, fit_intercept=True,  
                                                   max_iter=1000, tol=1e-3,  
                                                   shuffle=True, verbose=0,  
                                                   epsilon=0.1, random_state=None,  
                                                   learning_rate="invscaling",  
                                                   eta0=0.001, power_t=0.25,  
                                                   early_stopping=False,  
                                                   validation_fraction=0.1,  
                                                   n_iter_no_change=5,
                                                   warm_start=False,  
                                                   average=False)  
                                                    
## Public Member Functions

fit(X, y, coef_init = None, intercept_init = None, sample_weight = None)  
predict(X)  
score(X, y, sample_weight = None)  
load(fname, dtype = None)  
save(fname)  
get_params(deep = True)  
set_params(\*\*params)  
debug_print()     
release()  
is_fitted()  

# DESCRIPTION

Stochastic Gradient Descent (SGD) is used for discriminative learning of linear 
regressors  under convex loss functions such as SVM and Logistic regression. 
It has been successfully applied to large-scale datasets because the update 
to the coefficients is performed for each training instance, rather than at 
the end of instances.  

Stochastic Gradient Descent (SGD) regressor basically implements a plain SGD 
learning routine supporting various loss functions and penalties for regression. 

It implements regularized linear models with stochastic gradient descent (SGD) 
learning: the gradient of the loss is estimated each sample at a time and the model 
is updated along the way with a decreasing strength schedule.
It is a linear method which uses the following loss functions:  
**1) squared_loss**  
**2) epsilon_insensitive**  
**3) squared_epsilon_insensitive**  

**It supports ZERO, L1 and L2 regularization to address the overfit problem.**  

This module provides a client-server implementation, where the client 
application is a normal python program. The frovedis interface is almost same 
as Scikit-learn SGDRegressor (Stochastic Gradient Descent Regression) 
interface, but it doesn't have any dependency with Scikit-learn. It can be 
used simply even if the system doesn't have Scikit-learn installed. Thus in 
this implementation, a python client can interact with a frovedis server 
sending the required python data for training at frovedis side. Python data is 
converted into frovedis compatible data internally and the python ML call is 
linked with the respective frovedis ML call to get the job done at frovedis 
server.  

Python side calls for SGDRegressor on the frovedis server. Once the training 
is completed with the input data at the frovedis server, it returns an abstract 
model with a unique model ID to the client python program. 

When prediction-like request would be made on the trained model, python program
will send the same request to the frovedis server. After the request is served 
at the frovedis server, the output would be sent back to the python client. 

## Detailed Description  

### 1. SGDRegressor()   

__Parameters__  
_**loss**_: A string object parameter containing the loss function type to use. Currently, 
frovedis supports 'squared_loss', 'epsilon_insensitive'and 'squared_epsilon_insensitive' functions. 
(Default: 'squared_loss')  
_**penalty**_: A string object parameter containing the regularizer type to use. Currently
none, l1 and l2 are supported by Frovedis. (Default: 'l2')  
If it is None (not specified explicitly), it will be set as 'ZERO' regularization type.  
_**alpha**_: Zero or a positive double (float64) smoothing parameter. (Default: 0.0001)  
_**l1\_ratio**_: An unused parameter. (Default: 0.15)  
_**fit\_intercept**_: A boolean parameter specifying whether a constant (intercept) 
should be added to the decision function. (Default: True)  
_**max\_iter**_: A positive integer parameter specifying maximum iteration count. 
(Default: 1000)  
_**tol**_: A double (float64) parameter specifying the convergence tolerance value. 
It must be zero or a positive value. (Default: 1e-3)  
_**shuffle**_: An unused parameter. (Default: True)  
_**verbose**_: An integer parameter specifying the log level to use. Its value 
is set as 0 by default (for INFO mode). But it can be set to 1 (for DEBUG mode) or 
2 (for TRACE mode) for getting training time logs from frovedis server.  
_**epsilon**_: A zero or positive double (float64) parameter used in the 
epsilon-insensitive loss function. (Default: 0.1)  
_**random\_state**_: An unused parameter. (Default: None)  
_**learning\_rate**_: A string object parameter containing the learning rate. (Default: 'invscaling')  
Unlike sklearn, Frovedis only supports 'invscaling' learning rate. 'invscaling' gradually decreases 
the learning rate **'learning_rate_'** at each time step **'t'** using an inverse scaling exponent 
of **'power_t'**.  
**learning_rate_ = eta0 / pow(t, power_t)**  
_**eta0**_: A double (float64) parameter specifying the initial learning rate for 
the ‘invscaling’ and 'optimal' schedules. (Default: 0.001)  
_**power\_t**_: An unused parameter which specifies the exponent for inverse scaling 
learning rate. Although, this parameter is unused in frovedis but it must be of 
double (float64) type. This is simply done to keep the behavior consistent with 
Scikit-learn. (Default: 0.25)  
_**early\_stopping**_: An unused parameter. (Default: False)  
_**validation\_fraction**_: An unused parameter. (Default: 0.1)  
_**n\_iter\_no_change**_: An unused parameter. (Default: 5)  
_**warm\_start**_: A boolean parameter which when set to True, reuses the solution of 
the previous call to fit as initialization, otherwise, just erase the previous solution. 
(Default: False)  
_**average**_: An unused parameter. (Default: False)  

__Attributes__  
_**coef\_**_: It is a python ndarray (containing float or double (float64) typed values 
depending on data-type of input matrix (X)). It is the weights assigned to the features. 
It is of shape **(n_features,)**.  
_**intercept\_**_: It is a python ndarray (float or double (float64) values depending on 
input matrix data type) and has shape **(1,)**. It specifies the constants in decision 
function.  
_**n\_iter**\_: An integer value used to get the actual iteration point at which the problem 
is converged.  

__Purpose__    
It initializes a SGDRegressor object with the given parameters.   

The parameters: "l1_ratio", "shuffle", "random_state", "early_stopping", 
"validation_fraction", "n_iter_no_change" and "average" are simply kept to 
make the interface uniform to Scikit-learn SGDRegressor module. They are not used anywhere 
within frovedis implementation.  

__Return Value__  
It simply returns "self" reference.  

### 2. fit(X, y, coef_init = None,  intercept_init = None, sample_weight = None)  
__Parameters__   
_**X**_: A numpy dense or scipy sparse matrix or any python ndarray or an instance of 
FrovedisCRSMatrix for sparse data and FrovedisColmajorMatrix for dense data. It has 
shape **(n_samples, n_features)**.  
_**y**_: Any python ndarray or an instance of FrovedisDvector containing the target values.
It has shape **(n_samples,)**.  
_**coef_init**_: An unused parameter that specifies the initial coefficients to warm-start 
the optimization. (Default: None)  
_**intercept_init**_: An unused parameter that specifies the initial intercept to warm-start 
the optimization. (Default: None)  
_**sample\_weight**_: A python ndarray containing the intended weights for each input
samples and it should be the shape of **(n_samples,)**. (Default: None)  
When it is None (not specified explicitly), an uniform weight vector is assigned on 
each input sample.  

__Purpose__    
It accepts the training feature matrix (X) and corresponding target values (y) 
as inputs from the user and trains a SGDRegressor model with specified 
regularization with those data at frovedis server.  

For example,   

    # loading a sample matrix and labels data
    from sklearn.datasets import load_diabetes
    mat, lbl = load_diabetes(return_X_y = True)

    # fitting input matrix and label on SGDRegressor object
    from frovedis.mllib.linear_model import SGDRegressor
    sgd_reg = SGDRegressor(max_iter = 50000000, loss = "squared_epsilon_insensitive",
                           penalty = 'none', tol = 0.0000001).fit(mat, lbl)

When native python data is provided, it is converted to frovedis-like inputs
and sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when same 
data would be used for multiple executions.  

For example,

    # loading a sample matrix and labels data
    from sklearn.datasets import load_diabetes
    mat, lbl = load_diabetes(return_X_y = True)
    
    # Since "mat" is numpy dense data, we have created FrovedisColmajorMatrix.
    and for scipy sparse data, FrovedisCRSMatrix should be used. 
    from frovedis.matrix.dense import FrovedisColmajorMatrix
    from frovedis.matrix.dvector import FrovedisDvector
    cmat = FrovedisColmajorMatrix(mat)
    dlbl = FrovedisDvector(lbl)
    
    # SGDRegressor with pre-constructed frovedis-like inputs
    from frovedis.mllib.linear_model import SGDRegressor
    sgd_reg = SGDRegressor(max_iter = 50000000, loss = "squared_epsilon_insensitive",
                           penalty = 'none', tol = 0.0000001).fit(cmat,dlbl)
                           
__Return Value__  
It simply returns "self" reference.   

### 3. predict(X)
__Parameters__   
_**X**_: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix 
for dense data. It has shape **(n_samples, n_features)**.  

__Purpose__    
It accepts the test feature matrix (X) in order to make prediction on the 
trained model at frovedis server.

For example,

    sgd_reg.predict(mat)  
    
Output:  

    [187.30000503 79.39151311 168.59790769 155.75933185 129.99656706
     89.71373962 96.12127367 152.37303585 163.56868847 181.57865709
     .....
     .....
     123.18827921 64.13627628 185.64720615 136.37181437 139.34641795
     185.9899156 72.21133803]

Like in fit(), frovedis-like input can be used to speed-up the prediction making on 
the trained model at server side.  

For example,  

    # Since "cmat" is FrovedisColmajorMatrix, we have created FrovedisRowmajorMatrix.
    # predicting on SGDRegressor using frovedis-like input 
    sgd_reg.predict(cmat.to_frovedis_rowmatrix())
 
Output  
 
    [187.30000503 79.39151311 168.59790769 155.75933185 129.99656706
     89.71373962 96.12127367 152.37303585 163.56868847 181.57865709
     .....
     .....
     123.18827921 64.13627628 185.64720615 136.37181437 139.34641795
     185.9899156 72.21133803]
     
__Return Value__  
It returns a numpy array of double (float64) type containing the predicted values. 
It is of shape **(n_samples,)**.  

### 4. score(X,  y,  sample_weight = None)
__Parameters__  
**X**: A numpy dense or scipy sparse matrix or any python ndarray or an instance of 
FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data. It has 
shape **(n_samples, n_features)**.  
_**y**_: Any python ndarray containing the target values. It has shape **(n_samples,)**.  
**sample_weight**: A python ndarray containing the intended weights for each input
samples and it should be the shape of **(n_samples,)**. (Default: None)  
When it is None (not specified explicitly), an uniform weight vector is assigned 
on each input sample.  

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

    # calculate R2 score on given test data and labels
    sgd_reg.score(mat, lbl)
     
Output

    0.52  
    
__Return Value__  
It returns an R2 score of double (float64) type.  
 
### 5. load(fname, dtype = None)
__Parameters__  
**fname**: A string object containing the name of the file having model information 
to be loaded.  
**dtype**: A data-type is inferred from the input data. Currently, expected input 
data-type is either float or double (float64). (Default: None)  

__Purpose__  
It loads the model from the specified file(having little-endian binary data).  

For example,  

    # loading the SGDRegressor model
    sgd_reg.load("./out/SGDRegressorModel")

__Return Value__  
It simply returns "self" reference.   

### 6. save(fname)
__Parameters__   
**fname**: A string object containing the name of the file on which the target 
model is to be saved.    

__Purpose__    
On success, it writes the model information (metadata and model) in the 
specified file as little-endian binary data. Otherwise, it throws an exception.  

For example,  

    # saving the model
    sgd_reg.save("./out/SGDRegressorModel")
    
The **SGDRegressorModel** contains below directory structure:  

**SGDRegressorModel**  
    |------metadata  
    |------model    

'metadata' represents the detail about loss function, model_kind and datatype of 
training vector.  
The 'model' file contains the SGDRegressor model saved in binary format.  

This will save the SGDRegressor model on the path '/out/SGDRegressorModel'. It would raise 
exception if the directory already exists with same name.  

__Return Value__  
It returns nothing.   

### 7. get_params(deep = True)  

__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. 
If True, will return the parameters for an estimator and contained subobjects that are 
estimators. (Default: True)

__Purpose__    
This method belongs to the BaseEstimator class inherited by SGDRegressor. It is used to get parameters
and their values of SGDRegressor class.  

For example,  
 
    print(sgd_reg.get_params())

Output  

    {'alpha': 0.0001, 'average': False, 'early_stopping': False, 'epsilon': 0.1, 
    'eta0': 0.001, 'fit_intercept': True, 'l1_ratio': 0.15, 'learning_rate': 'invscaling', 
    'loss': 'squared_loss', 'max_iter': 1000, 'n_iter_no_change': 5, 'penalty': 'l2', 
    'power_t': 0.25, 'random_state': None, 'shuffle': True, 'tol': 0.001, 
    'validation_fraction': 0.1, 'verbose': 0, 'warm_start': False}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 8. set_params(\*\*params)  

__Parameters__   
_**\*\*params**_: All the keyword arguments are passed this function as dictionary. This dictionary contains 
parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by SGDRegressor, used to set parameter values.  

For example,   

    print("get parameters before setting:")
    print(sgd_reg.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    sgd_reg.set_params(penalty = 'l1', fit_intercept = False)
    print("get parameters before setting:")
    print(sgd_reg.get_params()) 

Output  

    get parameters before setting: 
    {'alpha': 0.0001, 'average': False, 'early_stopping': False, 'epsilon': 0.1, 
    'eta0': 0.001, 'fit_intercept': True, 'l1_ratio': 0.15, 'learning_rate': 'invscaling', 
    'loss': 'squared_loss', 'max_iter': 1000, 'n_iter_no_change': 5, 'penalty': 'l2', 
    'power_t': 0.25, 'random_state': None, 'shuffle': True, 'tol': 0.001, 
    'validation_fraction': 0.1, 'verbose': 0, 'warm_start': False}
    get parameters before setting: 
    {'alpha': 0.0001, 'average': False, 'early_stopping': False, 'epsilon': 0.1, 
    'eta0': 0.001, 'fit_intercept': False, 'l1_ratio': 0.15, 'learning_rate': 'invscaling', 
    'loss': 'squared_loss', 'max_iter': 1000, 'n_iter_no_change': 5, 'penalty': 'l1', 
    'power_t': 0.25, 'random_state': None, 'shuffle': True, 'tol': 0.001, 
    'validation_fraction': 0.1, 'verbose': 0, 'warm_start': False}
    
__Return Value__  
It simply returns "self" reference.  

### 9. debug_print()

__Purpose__    
It shows the target model information (weight values, intercept, etc.) on the server side 
user terminal. It is mainly used for debugging purpose.  

For example,

    sgd_reg.debug_print() 
    
Output:  
    
    -------- Weight Vector:: --------
    -8.26881e+10 -1.23538e+11 -1.81876e+11 -9.355e+08 -8.27686e+09 -8.80985e+10 -1.0433e+12 
    -4.84075e+10 -1.69485e+11 -6.51606e+12 -2.66758e+11 -4.63121e+12 -2.0491e+11
    Intercept:: -1.42647e+10

This output will be visible on server side. It displays the weights and intercept values 
on the trained model which is currently present on the server.  

**No such output will be visible on client side.**  

__Return Value__  
It returns nothing.   

### 10. release()

__Purpose__    
It can be used to release the in-memory model at frovedis server.  

For example,

    sgd_reg.release()
    
This will reset the after-fit populated attributes (like coef_, intercept_, n_iter_) to 
None, along with releasing server side memory.  

__Return Value__  
It returns nothing.   

### 11. is_fitted()

__Purpose__   
It can be used to confirm if the model is already fitted or not. In case, 
predict() is used before training the model, then it can prompt the user to 
train the model first.

__Return Value__  
It returns 'True', if the model is already fitted otherwise, it returns 'False'.

# SEE ALSO  
rowmajor_matrix, colmajor_matrix, dvector, crs_matrix, sgd_classifier   
