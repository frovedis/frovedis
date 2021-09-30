% Linear Regression

# NAME

Linear Regression - A regression algorithm to predict 
the continuous output without any regularization.

# SYNOPSIS

class frovedis.mllib.linear_model.LinearRegression(fit_intercept=True, normalize=False,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ copy_X=True, n_jobs=None,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ max_iter=None, tol=0.0001,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ lr_rate=1e-8, solver=None,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ verbose=0, warm_start = False)

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
is called linear since it can be expressed as a function of wTx and y. Linear 
Regression does not use any regularizer. 

The gradient of the squared loss is: (wTx-y).x   

Frovedis provides implementation of linear regression with the following 
optimizers:  
(1) stochastic gradient descent with minibatch  
(2) LBFGS optimizer  
(3) least-square  

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

For least-square solver, we have used LAPACK routine "gelsd" and ScaLAPACK routine
"gels" when input data is dense in nature. For the sparse-input we have provided a
least-square implementation, similar to scipy.sparse.linalg.lsqr.  

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis interface is almost same as Scikit-learn
Linear Regression interface, but it doesn't have any dependency with Scikit-learn. 
It can be used simply even if the system doesn't have Scikit-learn installed. Thus
in this implementation, a python client can interact with a frovedis server by sending
the required python data for training at frovedis side. Python data is converted
into frovedis compatible data internally and the python ML call is linked with the
respective frovedis ML call to get the job done at frovedis server. 

Python side calls for Linear Regression on the frovedis server. Once the training is
completed with the input data at the frovedis server, it returns an abstract model 
with a unique model ID to the client python program.  

When prediction-like request would be made on the trained model, python program will
send the same request to the frovedis server. After the request is served at the frovedis
server, the output would be sent back to the python client.  

## Detailed Description  

### LinearRegression()

__Parameters__   
**_fit\_intercept_**: A boolean parameter specifying whether a constant(intercept) 
should be added to the decision function. (Default: True)  
**_normalize_**: A boolean parameter. (unused)  
**_copy\_X_**: A boolean parameter. (unused)    
**_n\_jobs_**: An integer parameter. (unused)  
**_max\_iter_**: A positive integer value used to set the maximum number of iterations.
When it is None(not specified explicitly), it will be set as 1000 for "sag", "lbfgs",
"lapack" and "scalapack" solvers and for "sparse_lsqr" solver, it will be 2 * (n_features). (Default: None)  
**_tol_**: Zero or a positive value of double(float64) type specifying the convergence tolerance 
value. (Default: 0.001)  
**_lr\_rate_**: A positive value of double(float64) type containing the learning rate. (Default: 1e-8)  
**_solver_**: A string parameter specifying the solver to use. (Default: None). In case 
it is None (not explicitly specified), the value will be set to "lapack" 
when dense input matrix (X) is provided and for sparse input matrix (X), it is set as
"sparse_lsqr". Frovedis supports "sag", "lbfgs", "lapack", "scalapack", "sparse_lsqr"
solvers.  
"lapack" and "scalapack" solvers can only work with dense data.  
"sparse_lsqr" solver can only work with sparse data.  
**_verbose_**: An integer parameter specifying the log level to use. Its value is 0 by 
default(for INFO mode and not specified explicitly). But it can be set to 1 (for DEBUG mode)
or 2 (for TRACE mode) for getting training time logs from frovedis server.  
**_warm\_start_**: A boolean parameter which when set to True, reuses the solution of the 
previous call to fit as initialization, otherwise, just erase the previous solution. 
Only supported by "sag" and "lbfgs" solvers. (Default: False)  

__Attributes__  
**_coef\__**: It is a python ndarray(containing float or double(float64) typed values depending 
on data-type of input matrix (X)) of estimated coefficients for the linear regression problem. 
It has shape (n_features,).  
**_rank\__**: An integer value used to store rank of matrix (X). It is only available when
matrix (X) is dense and "lapack" solver is used.  
**_singular\__**: It is a python ndarray(contaning float or double(float64) typed values depending
on data-type of input matrix (X)) and of shape(min(X,y),) which is  used to store singular 
values of X. It is only available when X is dense and "lapack" solver is used.  
**_intercept(bias)\__**: It is a python ndarray(contaning float or double(float64) typed values 
depending on data-type of input matrix (X)). If fit_intercept is set to False, the intercept 
is set to zero. It has shape (1,).  
**_n\_iter\__**: A positive integer value used to get the actual iteration point at which the 
problem is converged. It is only available for "sag", "lbfgs" and "sparse-lsqr" solvers.  

__Purpose__    
It initializes a Linear Regression object with the given parameters.   

The parameters: "normalize", "copy_X" and "n_jobs" are simply kept in to to make the 
interface uniform to the Scikit-learn Linear Regression module. They are not used anywhere 
within frovedis implementation.  

"solver" can be "sag" for frovedis side stochastic gradient descent, 
"lbfgs" for frovedis side LBFGS optimizer, "sparse_lsqr", "lapack" and "scalapack" when 
optimizing the linear regression model. 

"max_iter" can be used to set the maximum interations to achieve the convergence. In case 
the convergence is not achieved, it displays a warning for the same.  

__Return Value__    
It simply returns "self" reference. 

### fit(X, y, sample_weight = None)
__Parameters__   
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisColmajorMatrix for dense data.      
**_y_**: Any python array-like object or an instance of FrovedisDvector.     
**_sample\_weight_**: Python array-like containing the intended weights for each
input samples and it should be the shape of (nsamples, ). When it is None (not specified),
an uniform weight vector is assigned on each input sample.  

__Purpose__    
It accepts the training feature matrix (X) and corresponding output labels (y) 
as inputs from the user and trains a linear regression model with those data 
at frovedis server. 

For example,   

    # loading a sample matrix and labels data
    from sklearn.datasets import load_boston
    mat, label = load_boston(return_X_y = True)
    
    # fitting input matrix and label on linear regression object
    from frovedis.mllib.linear_model import LinearRegression
    lr = LinearRegression(solver = 'sag').fit(mat,label)

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
    
    # Linear Regression with pre-constructed frovedlis-like inputs
    from frovedis.mllib.linear_model import LinearRegression
    lr = LinearRegression(solver = 'sag').fit(cmat, dlbl)  

__Return Value__  
It simply returns "self" reference.  

### predict(X)
__Parameters__   
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data.  

__Purpose__    
It accepts the test feature matrix (X) in order to make prediction on the 
trained model at frovedis server.  

For example,   

    # predicting on linear regression model
    lr.predict(mat[:10])  

Output  

    [30.00384338 25.02556238 30.56759672 28.60703649 27.94352423 25.25628446
     23.00180827 19.53598843 11.52363685 18.92026211]

If the above pre-constructed training data (cmat) is to be used during prediction, the
same can be used as follows:

    # predicting on sag linear regression model using pre-constructed input
    lr.predict(cmat.to_frovedis_rowmatrix())

Output  

    [30.00384338 25.02556238 30.56759672 28.60703649 27.94352423 25.25628446 
     23.00180827 19.53598843 11.52363685 18.92026211]

__Return Value__  
It returns a numpy array of float or double(float64) type and has shape (n_samples,) 
containing the predicted outputs. 

### score(X, y, sample_weight = None)
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an 
instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data.  
**_y_**: Any python array-like object or an instance of FrovedisDvector.     
**_sample\_weight_**: Python array-like containing the intended weights for each
input samples and it should be the shape of (nsamples, ). When it is None (not specified),
an uniform weight vector is assigned on each input sample.  

__Purpose__  
Calculate the root mean square value on the given test data and labels i.e. 
R2(r-squared) of self.predict(X) wrt. y.

The coefficient 'R2' is defined as (1 - (u/v)),  
where 'u' is the residual sum of squares ((y_true - y_pred) ** 2).sum() and,  
'v' is the total sum of squares ((y_true - y_true.mean()) ** 2).sum().  
The best possible score is 1.0 and it can be negative (because the model can 
be arbitrarily worse). A constant model that always predicts the expected value 
of y, disregarding the input features, would get a R2 score of 0.0.  

For example,   

    # calculate R2 score on given test data and labels
    lr.score(mat[:10], label[:10]) 

Output  

    0.40

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

    lr.load("./out/LNRModel")

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

    # To save the linear regression model
    lr.save("./out/LNRModel")    

This will save the linear regression model on the path "/out/LNRModel".  

__Return Value__  
It returns nothing.  

### debug_print()

__Purpose__    
It shows the target model information (weight values, intercept) on the server side 
user terminal. It is mainly used for debugging purpose.  

For example,  

    lr.debug_print() 
    
Output  

    -------- Weight Vector:: --------
    -0.108011 0.0464205 0.0205586 2.68673 -17.7666 3.80987 0.000692225 -1.47557 0.306049 
    -0.0123346 -0.952747 0.00931168 -0.524758
    Intercept:: 36.4595

It displays the weights and intercept values on the trained model which is currently present 
on the server.

__Return Value__  
It returns nothing.  

### release()

__Purpose__    
It can be used to release the in-memory model at frovedis server.  

For example,
 
    lr.release()

This will reset the after-fit populated attributes to None, along with releasing server 
side memory.  

__Return Value__  
It returns nothing.  

### is_fitted()

__Purpose__    
It can be used to confirm if the model is already fitted or not. In case, predict() is used 
before training the model, then it can prompt the user to train the linear regression model first. 

__Return Value__  
It returns ‘True’, if the model is already fitted otherwise, it returns ‘False’.

# SEE ALSO  
lasso_regression, ridge_regression, dvector, crs_matrix      