% LinearSVR  

# NAME

LinearSVR (Support Vector Regression) - A regression algorithm used to predict the binary output 
with L1 and L2 loss.  

# SYNOPSIS

class frovedis.mllib.svm.LinearSVR(epsilon=0.0, tol=1e-4, C=1.0, loss='epsilon_insensitive',  
\  \  \  \  \  \  \  \  \  \  \  \  \  \  \  \  \  \  
\  \  \  \  \  \ fit_intercept=True, intercept_scaling=1, dual=True,   
\  \  \  \  \  \  \  \  \  \  \  \  \  \  \  \  \  \  
\  \  \  \  \  \ verbose=0, random_state=None, max_iter=1000,  
\  \  \  \  \  \  \  \  \  \  \  \  \  \  \  \  \  \  
\  \  \  \  \  \ penalty='l2', lr_rate=0.01,  solver='sag',  
\  \  \  \  \  \  \  \  \  \  \  \  \  \  \  \  \  \  
\  \  \  \  \  \ warm_start=False)  


## Public Member Functions

fit(X, y, sample_weight = None)   
predict(X)  
load(fname, dtype = None)done  
save(fname)  
score(X, y, sample_weight = None)  
get_params(deep = True)  
set_params(\*\*params)  
debug_print()     
release()  
is_fitted()  

# DESCRIPTION 
Based on support vector machines method, the Linear SVR is an algorithm to solve 
the regression problems. The Linear SVR algorithm applies linear kernel method and 
it works well with large datasets. L1 or L2 method can be specified as a loss 
function in this model.

The model produced by Support Vector Regression depends only on a subset of the 
training data, because the cost function ignores samples whose prediction is close 
to their target.  

LinearSVR supports ZERO, L1 and L2 regularization to address the overfit problem.    

Frovedis provides implementation of LinearSVR with **stochastic gradient 
descent with minibatch**. 

The simplest method to solve optimization problems of the form **min f(w)** 
is gradient descent. Such first-order optimization methods well-suited for 
large-scale and distributed computation. 

This module provides a client-server implementation, where the client 
application is a normal python program. The frovedis interface is almost same 
as Scikit-learn LinearSVR (Support Vector Regression) interface, but 
it doesn't have any dependency with Scikit-learn. It can be used simply even if 
the system doesn't have Scikit-learn installed. Thus in this implementation, a 
python client can interact with a frovedis server sending the required python
data for training at frovedis side. Python data is converted into frovedis 
compatible data internally and the python ML call is linked with the respective
frovedis ML call to get the job done at frovedis server.  

Python side calls for LinearSVR on the frovedis server. Once the training 
is completed with the input data at the frovedis server, it returns an abstract 
model with a unique model ID to the client python program. 

When prediction-like request would be made on the trained model, python program
will send the same request to the frovedis server. After the request is served 
at the frovedis server, the output would be sent back to the python client. 

## Detailed Description  

### 1. LinearSVR()   

__Parameters__       
_**epsilon**_: A zero or positive double(float64) parameter used in the 
epsilon-insensitive loss function.  (Default: 0.0)  
_**tol**_: A double(float64) parameter specifying the convergence tolerance value. 
It must be zero or a positive value. (Default: 1e-4)     
_**C**_: A positive float parameter, it is inversely proportional to regularization strength. (Default: 1.0)  
_**loss**_: A string object containing the loss function type to use.  
Currently, frovedis supports ‘epsilon_insensitive’ and ‘squared_epsilon_insensitive’ 
loss function.  
The ‘epsilon-insensitive’ loss (standard SVR) is the L1 loss, while the squared 
epsilon-insensitive loss (‘squared_epsilon_insensitive’) is the L2 loss.
(Default: 'epsilon_insensitive')  
_**fit\_intercept**_: A boolean parameter specifying whether a constant (intercept) 
should be added to the decision function. (Default: True)  
_**intercept\_scaling**_: An unused parameter. (Default: 1)  
_**dual**_: An unused parameter. (Default: True)  
_**verbose**_: An integer parameter specifying the log level to use. Its value 
is set as 0 by default(for INFO mode). But it can be set to 1(for DEBUG mode) or 
2(for TRACE mode) for getting training time logs from frovedis server.  
_**random\_state**_: An unused parameter. (Default: None)  
_**max\_iter**_: A positive integer parameter specifying maximum iteration count. 
(Default: 1000)  
_**penalty**_: A string object containing the regularizer type to use. Currently
none, l1 and l2 are supported by Frovedis. (Default: 'l2')  
If it is None (not specified explicitly), it will be set as 'ZERO' regularization type.  
_**lr\_rate**_: A positive double(float64) value of parameter containing the learning rate. 
(Default: 0.01)  
_**solver**_: A string object specifying the solver to use. (Default: 'sag')  
Currenlty, it only supports 'sag'.  
_**warm_start**_: A boolean parameter which when set to True, reuses the solution of 
the previous call to fit as initialization, otherwise, just erase the previous solution. 
(Default: False)  

__Attributes__  
_**coef\_**_: It is a python ndarray(containing float or double(float64) typed values 
depending on data-type of input matrix (X)). It is the weights assigned to the features. 
It has shape **(1, n_features)**.  
_**intercept\_**_: It is a python ndarray(float or double(float64) values depending on 
input matrix data type) and has shape **(1,)**.  
_**n_iter\_**_: An integer value used to get the actual iteration point at which the 
problem is converged.  

__Purpose__    
It initializes a LinearSVR object with the given parameters.   

The parameters: "intercept_scaling", "dual" and "random_state" are simply kept to make the 
interface uniform to Scikit-learn LinearSVR module. They are not used anywhere within 
frovedis implementation.  

__Return Value__    
It simply returns "self" reference.  

### 2. fit(X, y, sample_weight = None)
__Parameters__   
_**X**_: A numpy dense or scipy sparse matrix or any python array-like object or an 
instance of FrovedisCRSMatrix for sparse data and FrovedisColmajorMatrix for 
dense data. It has shape **(n_samples, n_features)**.  
_**y**_: Any python array-like object or an instance of FrovedisDvector. It has shape 
**(n_samples,)**.  
_**sample\_weight**_: A python ndarray containing the intended weights for each input
samples and it should be the shape of **(nsamples, )**.  
When it is None (not specified explicitly), an uniform weight vector is assigned on 
each input sample. (Default: None)  

__Purpose__    
It accepts the training feature matrix (X) and corresponding output labels (y) 
as inputs from the user and trains a LinearSVR model with specifed 
regularization with those data at frovedis server. 

For example,   

    # let a sample matrix be 'mat 'and labels be 'lbl' 
    mat = np.array([[ 5.5, 0.0, 6.5, 2.3],
                    [ 0.0, 7.2, 0.0, 8.4],
                    [ 8.5, 0.0, 6.5, 4.3],
                    [ 0.0, 8.5, 0.1, 9.1]], dtype=np.float64)
    lbl = np.array([1.1, 6.2,1.2, 5.9])


    # fitting input matrix and label on LinearSVR object
    from frovedis.mllib.svm import LinearSVR
    svr = LinearSVR().fit(mat, lbl)

When native python data is provided, it is converted to frovedis-like inputs
and sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when same 
data would be used for multiple executions.  

For example,

    # Since "mat" is numpy dense data, we have created FrovedisColmajorMatrix.
    and for scipy sparse data, FrovedisCRSMatrix should be used. 
    from frovedis.matrix.dense import FrovedisColmajorMatrix
    from frovedis.matrix.dvector import FrovedisDvector
    cmat = FrovedisColmajorMatrix(mat)
    dlbl = FrovedisDvector(lbl)
    
    # LinearSVR with pre-constructed frovedis-like inputs
    from frovedis.mllib.svm import LinearSVR
    svr = LinearSVR().fit(cmat, dlbl)
    
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

    svr.predict(mat)  
    
Output:  

    [-181.66076961 -162.62098062 -166.05339001 ... -170.80953572 -169.6636383 -171.76112166] 

Like in fit(), frovedis-like input can be used to speed-up the prediction making on 
the trained model at server side.  

For example,  

    # Since "cmat" is FrovedisColmajorMatrix, we have created FrovedisRowmajorMatrix.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix

    # predicting on LinearSVR using frovedis-like input 
    svr.predict(cmat.to_frovedis_rowmatrix())


Output  
 
    [-181.66076961 -162.62098062 -166.05339001 ... -170.80953572 -169.6636383 -171.76112166]
   
__Return Value__  
It returns a numpy array of double(float64) type containing the predicted 
outputs. It has shape **(n_samples,)**.  

### 4. load(fname, dtype = None)
__Parameters__  
**fname**: A string object containing the name of the file having model information 
to be loaded.  
**dtype**: A data-type is inferred from the input data. Currently, expected input 
data-type is either float or double(float64). (Default: None)  

__Purpose__  
It loads the model from the specified file(having little-endian binary data).  

For example,  

    # loading the LinearSVR model
    svr.load("./out/SVRModel")

__Return Value__  
It simply returns "self" reference.   

### 5. save(fname)
__Parameters__   
**fname**: A string object containing the name of the file on which the target 
model is to be saved.    

__Purpose__    
On success, it writes the model information (metadata and model) in the 
specified file as little-endian binary data. Otherwise, it throws an exception.  

For example,  

    # saving the model
    svr.save("./out/SVRModel")
    
The SVRModel contains below directory structure:  
**SVRModel**  
    |------metadata  
    |------model    

'metadata' represents the detail about model_kind and datatype of training vector.  
Here, the model file contains information about model_id, model_kind and datatype 
of training vector.  

This will save the LinearSVR model on the path ‘/out/SVRModel’. It would raise 
exception if the directory already exists with same name.  

__Return Value__  
It returns nothing.   

### 6. score(X,  y,  sample_weight = None)
__Parameters__  
**X**: A numpy dense or scipy sparse matrix or any python array-like object or an 
instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense 
data. It has shape **(n_samples, n_features)**.  
**y**: Any python array-like object or an instance of FrovedisDvector. It has shape 
**(n_samples,)**.    
**sample_weight**: A python narray containing the intended weights for each input
samples and it should be the shape of **(nsamples, )**.  
When it is None (not specified explicitly), an uniform weight vector is assigned 
on each input sample. (Default: None)  

__Purpose__    
Calculate the root mean square value on the given test data and labels i.e. 
R2(r-squared) of self.predict(X) wrt. y.  

The coefficient 'R2' is defined as (1 - (u/v)),  
where 'u' is the residual sum of squares ((y_true - y_pred) ** 2).sum() and
'v' is the total sum of squares ((y_true - y_true.mean()) ** 2).sum().  

The best possible score is 1.0 and it can be negative (because the model can be 
arbitrarily worse). 
A constant model that always predicts the expected value of y, disregarding the 
input features, would get a R2 score of 0.0.  

For example,  

     # calculate R2 score on given test data and labels
     svr.score(mat, lbl)
     
Output

    0.97 
    
__Return Value__  
It returns an R2 score of float type.


### 7. get_params(deep = True)  

__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an 
estimator. If True, will return the parameters for an estimator and contained 
subobjects that are estimators. (Default: True)  

__Purpose__    
This method belongs to the BaseEstimator class inherited by LinearSVR. It is 
used to get parameters and their values of LinearSVR class.  

For example, 
 
    print(svr.get_params())

Output  

    {'C': 1.0, 'dual': True, 'epsilon': 0.0, 'fit_intercept': True, 'intercept_scaling': 1, 
    'loss': 'epsilon_insensitive', 'lr_rate': 0.01, 'max_iter': 1000, 'penalty': 'l2', 
    'random_state': None, 'solver': 'sag', 'tol': 0.0001, 'verbose': 0, 'warm_start': False}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 8. set_params(**params)  

__Parameters__   

_**params**_: All the keyword arguments are passed to this function as dictionary. 
This dictionary contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by LinearSVR, used to set 
parameter values.  

For example,   

    print("Get parameters before setting:", svr.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    svr.set_params( penalty = 'l1', dual = False)
    print("Get parameters after setting:", svr.get_params())

Output  

    Get parameters before setting: {'C': 1.0, 'dual': True, 'epsilon': 0.0, 'fit_intercept': True, 
    'intercept_scaling': 1, 'loss': 'epsilon_insensitive', 'lr_rate': 0.01, 'max_iter': 1000, 
    'penalty': 'l2', 'random_state': None, 'solver': 'sag', 'tol': 0.0001, 'verbose': 0, 
    'warm_start': False}
    Get parameters before setting: {'C': 1.0, 'dual': False, 'epsilon': 0.0, 'fit_intercept': True, 
    'intercept_scaling': 1, 'loss': 'epsilon_insensitive', 'lr_rate': 0.01, 'max_iter': 1000, 
    'penalty': 'l1', 'random_state': None, 'solver': 'sag', 'tol': 0.0001, 'verbose': 0, 
    'warm_start': False}

__Return Value__  
It simply returns "self" reference.  

### 9. debug_print()

__Purpose__    
It shows the target model information(weight values and intercept) on the server side 
user terminal. It is mainly used for debugging purpose.  

For example,

    svr.debug_print() 
    
Output:  
     
    -------- Weight Vector:: --------
    0.406715 0.413736 0.440404 0.539082 0.536528 0.51577 0.157345 0.520861 
    0.513352 -0.341082 0.460518 -0.292171 0.433433
    Intercept:: 1.00832

__Return Value__  
It returns nothing.   

### 10. release()

__Purpose__    
It can be used to release the in-memory model at frovedis server.  

For example,

    svr.release()
    
This will reset the after-fit populated attributes to None, along with 
releasing server side memory.  

__Return Value__  
It returns nothing.   

### 11. is_fitted()

__Purpose__   
It can be used to confirm if the model is already fitted or not. In case, 
predict() is used before training the model, then it can prompt the user to 
train the model first.

__Return Value__  
It returns ‘True’, if the model is already fitted otherwise, it returns ‘False’.

# SEE ALSO  
SVM, LinearSVC, dvector, rowmajor_matrix, colmajor_matrix, crs_matrix