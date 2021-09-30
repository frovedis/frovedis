% Ridge Regression

# NAME

Ridge Regression - A regression algorithm to predict 
the continuous output with L2 regularization.   

# SYNOPSIS

class frovedis.mllib.linear_model.Ridge(alpha=0.01, fit_intercept=True,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ normalize=False, copy_X=True,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ max_iter=None, tol=1e-3,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ solver='auto', random_state=None,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ lr_rate=1e-8, verbose=0,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ warm_start = False)  

## Public Member Functions

fit(X, y, sample_weight = None)  
predict(X)  
score(X, y, sample_weight = None)  
load(fname, dtype = None)  
save(fname)  
debug_print()  
release()  
is_fitted()  

# DESCRIPTION
Linear least squares is the most common formulation for regression problems. 
It is a linear method with the loss function given by the **squared loss**:

    L(w;x,y) := 1/2(wTx-y)^2

Where the vectors x are the training data examples and y are their corresponding 
labels which we want to predict. w is the linear model (also known as weight) 
which uses a single weighted sum of features to make a prediction. The method 
is called linear since it can be expressed as a function of wTx and y. Ridge 
regression uses L2 regularization to address the overfit problem.    

The gradient of the squared loss is: (wTx-y).x   
The gradient of the regularizer is: w   

Frovedis provides implementation of ridge regression with two different 
optimizers:  
(1) stochastic gradient descent with minibatch  
(2) LBFGS optimizer  

The simplest method to solve optimization problems of the form **min f(w)** 
is gradient descent. Such first-order optimization methods well-suited for 
large-scale and distributed computation. Whereas, L-BFGS is an optimization 
algorithm in the family of quasi-Newton methods to solve the optimization 
problems of the similar form.  

Like the original BFGS, L-BFGS (Limited Memory BFGS) uses an estimation to 
the inverse Hessian matrix to steer its search through feature space, 
but where BFGS stores a dense nxn approximation to the inverse Hessian 
(n being the number of features in the problem), L-BFGS stores only a few 
vectors that represent the approximation implicitly. L-BFGS often achieves 
rapider convergence compared with other first-order optimization.  

This module provides a client-server implementation, where the client 
application is a normal python program. The frovedis interface is almost same
as Scikit-learn Ridge Regression interface, but it doesn't have any dependency
with Scikit-learn. It can be used simply even if the system doesn't have Scikit-learn
installed. Thus in this implementation, a python client can 
interact with a frovedis server sending the required python data for training
at frovedis side. Python data is converted into frovedis compatible data 
internally and the python ML call is linked with the respective frovedis ML 
call to get the job done at frovedis server.  

Python side calls for Ridge Regression on the frovedis server. Once the training is
completed with the input data at the frovedis server, it returns an abstract model 
with a unique model ID to the client python program.  

When prediction-like request would be made on the trained model, python program will
send the same request to the frovedis server. After the request is served at the frovedis
server, the output would be sent back to the python client.   

## Detailed Description  

### Ridge()

__Parameters__   
**_alpha_**: A postive value of double(float64) type is called the regularization 
strength parameter. (Default: 0.01)  
**_fit\_intercept_**: A boolean parameter specifying whether a constant (intercept) 
should be added to the decision function. (Default: True)  
**_normalize_**: A boolean parameter (unused)  
**_copy\_X_**: A boolean parameter (unsed)  
**_max\_iter_**: An integer parameter specifying maximum iteration count. (Default: None)
When it is None(not specified explicitly), it will be set as 1000.  
**_tol_**: Zero or a positive value of double(float64) type specifying the convergence 
tolerance value. (Default: 1e-3)  
**_solver_**: A string object specifying the solver to use. It can be "sag" for frovedis 
side stochastic gradient descent or "lbfgs" for frovedis side LBFGS optimizer 
when optimizing the ridge regression model. Initilaly solver is "auto" by default. 
In such cases, it will select "sag" solver. Both "sag" and "lbfgs" handle L2 penalty.  
**_random\_state_**: An integer, None or RandomState instance. (unused)  
**_lr\_rate_**: Zero or a positive value of double(float64) type containing the learning 
rate. (Default: 1e-8)  
**_verbose_**: An integer parameter specifying the log level to use. Its value is 0 by 
default(for INFO mode and not specified explicitly). But it can be set to 1 (for DEBUG mode)
or 2 (for TRACE mode) for getting training time logs from frovedis server.  
**_warm\_start_**: A boolean parameter which when set to True, reuses the solution of 
the previous call to fit as initialization, otherwise, just erase the previous 
solution. (Default: False)  

__Attributes__  
**_coef\__**: It is a python ndarray(containing float or double(float64) typed values depending 
on data-type of input matrix (X)) of estimated coefficients for the ridge regression problem. 
It has shape (n_features,).  
**_intercept(bias)\__**: It is a python ndarray(contaning float or double(float64) typed values 
depending on data-type of input matrix (X)). If fit_intercept is set to False, the intercept 
is set to zero. It has shape (1,).  
**_n\_iter\__**: A positive integer value used to get the actual iteration point at which the 
problem is converged.   

__Purpose__    
It initialized a Ridge object with the given parameters.   

The parameters: "normalize", "copy_X" and "random_state" are simply kept in to to make the
interface uniform to the Scikit-learn Ridge Regression module. They are not used anywhere
within frovedis implementation.  

__Return Value__    
It simply returns "self" reference. 

### fit(X, y, sample_weight = None)
__Parameters__   
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an 
instance of FrovedisCRSMatrix for sparse data and FrovedisColmajorMatrix for dense data.  
**_y_**: Any python array-like object or an instance of FrovedisDvector.  
**_sample\_weight_**: Python array-like containing the intended weights for each input
samples and it should be the shape of (nsamples, ). When it is None (not specified),
an uniform weight vector is assigned on each input sample.   

__Purpose__    
It accepts the training feature matrix (X) and corresponding output labels (y) 
as inputs from the user and trains a ridge regression model with L2 regularization 
with those data at frovedis server.  

For example,  

    # loading a sample matrix and labels data
    from sklearn.datasets import load_boston
    mat, label = load_boston(return_X_y = True)
    
    # fitting input matrix and label on ridge regression object
    from frovedis.mllib.linear_model import Ridge
    rr = Ridge(solver = 'lbfgs').fit(mat, label)  

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedlis-like inputs can be used to speed up the training time, specially when 
same data would be used for multiple executions.  

For example,
    
    # loading a sample matrix and labels data
    from sklearn.datasets import load_boston
    mat, label = load_boston(return_X_y = True)
    
    # Since "mat" is numpy dense data, we have created FrovedisColmajorMatrix. 
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisColmajorMatrix
    from frovedis.matrix.dvector import FrovedisDvector 
    cmat = FrovedisColmajorMatrix(mat)
    dlbl = FrovedisDvector(lbl)
    
    # Ridge Regression with pre-constructed frovedlis-like inputs
    from frovedis.mllib.linear_model import Ridge
    rr = Ridge(solver = 'lbfgs').fit(cmat, dlbl)

__Return Value__  
It simply returns "self" reference.  

### predict(X)
__Parameters__   
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix as 
for dense data.  

__Purpose__    
It accepts the test feature matrix (X) in order to make prediction on the 
trained model at frovedis server. 

For example,   

    # predicting on ridge model
    rr.predict(mat) 

Output  

    [28.71021961 23.76178249 30.4133597  28.97666397 28.67315792 24.66094155
     20.57933251 17.64381951  9.47525481 16.75502233 17.6363261  19.15371596
     18.82699337 20.73240606 20.44989254 20.31470466 21.85948797 18.28896663
     ...
     16.35934999 20.59831667 21.23383603 17.60978119 14.01230367 19.28714569
     21.74889541 18.19015854 20.88375846 26.19279806 24.06841151 30.327149
     28.62134714 23.7732722 ]

If the above pre-constructed training data (cmat) is to be used during prediction, 
the same can be used as follows:

    # predicting on ridge regression model using pre-constructed input
    rr.predict(cmat.to_frovedis_rowmatrix())

Output  

    [28.71021961 23.76178249 30.4133597  28.97666397 28.67315792 24.66094155
     20.57933251 17.64381951  9.47525481 16.75502233 17.6363261  19.15371596
     18.82699337 20.73240606 20.44989254 20.31470466 21.85948797 18.28896663
     ...
     16.35934999 20.59831667 21.23383603 17.60978119 14.01230367 19.28714569
     21.74889541 18.19015854 20.88375846 26.19279806 24.06841151 30.327149
     28.62134714 23.7732722 ]

__Return Value__  
It returns a numpy array of double(float64) type type and has shape (n_samples,) 
containing the predicted outputs. 

### score(X, y, sample_weight = None)
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data.  
**_y_**: Any python array-like object or an instance of FrovedisDvector.     
**_sample\_weight_**: Python array-like containing the intended weights for each input
samples and it should be the shape of (nsamples, ). When it is None (not specified),
an uniform weight vector is assigned on each input sample.  

__Purpose__  
Calculate the root mean square value on the given test data and labels i.e. 
R2(r-squared) of self.predict(X) wrt. y.

The coefficient 'R2' is defined as (1 - (u/v)),  
where 'u' is the residual sum of squares ((y_true - y_pred) ** 2).sum() and  
'v' is the total sum of squares ((y_true - y_true.mean()) ** 2).sum(). The best 
possible score is 1.0 and it can be negative (because the model can be arbitrarily worse). 
A constant model that always predicts the expected value of y, disregarding the input 
features, would get a R2 score of 0.0.  

For example,   

    # calculate R2 score on given test data and labels
    rr.score(mat, label) 

Output  

    0.70

__Return Value__  
It returns an R2 score of float type.  

### load(fname, dtype = None)
__Parameters__   
**_fname_**: A string object containing the name of the file having model 
information to be loaded.  
**_dtype_**: A data-type is inferred from the input data. Currently, expected input 
data-type is either float or double(float64). (Default: None)  

__Purpose__    
It loads the model from the specified file (having little-endian binary data).

For example,   

    rr.load("./out/RidgeModel")

__Return Value__  
It simply returns "self" instance.  

### save(fname)
__Parameters__   
**_fname_**: A string object containing the name of the file on which the target 
model is to be saved.  

__Purpose__    
On success, it writes the model information (weight values etc.) in the 
specified file as little-endian binary data. Otherwise, it throws an exception. 

For example,   

    # To save the ridge regression model
    rr.save("./out/RidgeModel")  

This will save the ridge regression model on the path "/out/RidgeModel".  

__Return Value__  
It returns nothing.  

### debug_print()

__Purpose__    
It shows the target model information (weight values, intercept) on the server side 
user terminal. It is mainly used for debugging purpose.   

For example,  

    rr.debug_print() 
    
Output  

    -------- Weight Vector:: --------
    -0.092909 0.0669578 -0.013893 -0.0253964 -0.314699 5.54217 -0.0117369 -1.29332
     0.215405 -0.0133328 -0.253942 0.0153361 -0.444395
    Intercept:: 1.06476  

It displays the weights and intercept values on the trained model which is currently
present on the server.

__Return Value__  
It returns nothing.  

### release()

__Purpose__    
It can be used to release the in-memory model at frovedis server. 

For example,
 
    rr.release()

This will reset the after-fit populated attributes to None, along with releasing 
server side memory.  

__Return Value__  
It returns nothing.  

### is_fitted()

__Purpose__    
It can be used to confirm if the model is already fitted or not. In case, predict() is used 
before training the model, then it can prompt the user to train the ridge regression 
model first. 

__Return Value__  
It returns ‘True’, if the model is already fitted otherwise, it returns ‘False’.

# SEE ALSO  
linear_regression, lasso_regression, dvector, crs_matrix    
