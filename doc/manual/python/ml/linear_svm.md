% Linear SVM  

# NAME

Linear SVM (Support Vector Machines) - A classification algorithm 
to predict the binary output with hinge loss.  

# SYNOPSIS

class frovedis.mllib.svm.LinearSVC (penalty='l2', loss='hinge', dual=True, tol=1e-4,   
\  \ \  \  \  \ C=0.01, multi_class='ovr', fit_intercept=True,    
\  \ \  \  \  \ intercept_scaling=1, class_weight=None, verbose=0,    
\  \ \  \  \  \ random_state=None, max_iter=1000, solver='sag')      

## Public Member Functions

fit(X, y, sample_weight=None)   
predict(X)  
predict_proba (X)
save(filename)   
load(filename)   
debug_print()   
release()   

# DESCRIPTION
Classification aims to divide items into categories. 
The most common classification type is binary classification, where there are 
two categories, usually named positive and negative. Frovedis supports binary 
classification algorithms only. 

The Linear SVM is a standard method for large-scale classification tasks. 
It is a linear method with the loss function given by the **hinge loss**:   

    L(w;x,y) := max{0, 1-ywTx}

Where the vectors x are the training data examples and y are their corresponding 
labels (Frovedis considers negative response as -1 and positive response as 1, but 
when calling from scikit-learn interface, user should pass 0 for negative response 
and 1 for positive response according to the scikit-learn requirement) which we 
want to predict. w is the linear model (also known as weight) which uses a 
single weighted sum of features to make a prediction. Linear SVM supports ZERO, 
L1 and L2 regularization to address the overfit problem.     

The gradient of the hinge loss is: -y.x, if ywTx < 1, 0 otherwise.    
The gradient of the L1 regularizer is: sign(w)     
And The gradient of the L2 regularizer is: w     

For binary classification problems, the algorithm outputs a binary svm 
model. Given a new data point, denoted by x, the model makes 
predictions based on the value of wTx. 

By default (threshold=0), if wTx >= 0, then the response is positive (1), 
else the response is negative (0).

Frovedis provides implementation of linear SVM with two different 
optimizers: (1) stochastic gradient descent with minibatch and (2) LBFGS 
optimizer. 

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
application is a normal python scikit-learn program. Scikit-learn has its own 
svm module providing the LinearSVC (Support Vector Classification) support. 
But that algorithm is non-distributed in nature. Hence it is slower when comparing 
with the equivalent Frovedis algorithm (see frovedis manual for ml/linear_svm) with 
big dataset. Thus in this implementation, a scikit-learn client can interact with 
a frovedis server sending the required python data for training at frovedis side. 
Python data is converted into frovedis compatible data internally and the 
scikit-learn ML call is linked with the respective frovedis ML call to get the 
job done at frovedis server. 

Scikit-learn side call for Linear SVC quickly returns, 
right after submitting the training request to the frovedis server with a unique 
model ID for the submitted training request. 

When operations like prediction will be required on the trained model, scikit-learn 
client sends the same request to frovedis server on the same model 
(containing the unique ID) and the request is served at frovedis server and output 
is sent back to the scikit-learn client. 

## Detailed Description  

### LinearSVC()   

__Parameters__       
_penalty_: A string object containing the regularizer type to use. (Default: 'l2')    
_loss_: A string object containing the loss function type to use. (Default: 'hinge')    
_dual_: A boolean parameter (unused)      
_tol_: A double parameter specifying the convergence tolerance value, (Default: 1e-4)     
_C_: A double parameter containing the learning rate. (Default: 0.01)    
_multi\_class_: A string object specifying type of classification. (Default: 'ovr')   
_fit\_intercept_: A boolean parameter specifying whether a constant (intercept) 
should be added to the decision function. (Default: True)     
_intercept\_scaling_: An integer parameter. (unused)    
_class\_weight_: A python dictionary or a string object. (unused)    
_verbose_: An integer object specifying the log level to use. (Default: 0)   
_random\_state_: An integer, None or RandomState instance. (unused)   
_max\_iter_: An integer parameter specifying maximum iteration count. (Default: 1000)    
_solver_: A string object specifying the solver to use. (Default: 'sag')   

__Purpose__    
It initialized a Lasso object with the given parameters.   

The parameters: "dual", "intercept_scaling", "class_weight", and 
"random_state" are not yet supported at frovedis side. 
Thus they don't have any significance in this call. They are simply provided 
for the compatibility with scikit-learn application.    

"penalty" can be either 'l1' or 'l2' (Default: 'l2').    
"loss" value can only be 'hinge'.   

"solver" can be either 'sag' for frovedis side stochastic gradient descent or 
'lbfgs' for frovedis side LBFGS optimizer when optimizing the linear regression 
model. 

"multi_class" can only be 'ovr' as frovedis suppots binary classification 
algorithms only at this moment.   

"verbose" value is set at 0 by default.
But it can be set to 1 (for DEBUG mode) or 2 (for TRACE mode) for getting 
training time logs from frovedis server.   

__Return Value__    
It simply returns "self" reference. 

### fit(X, y, sample_weight=None)
__Parameters__   
_X_: A scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix.    
_y_: Any python array-like object or an instance of FrovedisDvector.     
_sample\_weight_: Python array-like optional parameter. (unused)   

__Purpose__    
It accepts the training feature matrix (X) and corresponding output labels (y) 
as inputs from the user and trains a linear regression model with specifed 
regularization with those data at frovedis server. 

It doesn't support any initial weight to be passed as input at this moment. 
Thus the "sample_weight" parameter will simply be ignored. It starts with an 
initial guess of zeros for the model vector and keeps updating the model to 
minimize the cost function until convergence is achieved or maximum iteration count 
is reached.  

For example,   

    # loading sample CRS data file
    mat = FrovedisCRSMatrix().load("./sample")
    lbl = FrovedisDvector([1,0,1,1,1,0,1,1])
    
    # fitting input matrix and label on linear SVC object
    lr = LinearSVC(solver='sgd', verbose=2).fit(mat,lbl)

__Return Value__  
It simply returns "self" reference.   
Note that the call will return quickly, right after submitting the fit request 
at frovedis server side with a unique model ID for the fit request. It may be 
possible that the training is not completed at the frovedis server side even 
though the client scikit-learn side fit() returns. 

### predict(X)
__Parameters__   
_X_: A scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix.    

__Purpose__    
It accepts the test feature matrix (X) in order to make prediction on the 
trained model at frovedis server. 

__Return Value__  
It returns a numpy array of double (float64) type containing the predicted 
outputs. 

### predict_proba(X)
__Parameters__   
_X_: A scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix.    

__Purpose__    
It accepts the test feature matrix (X) in order to make prediction on the 
trained model at frovedis server. But unlike predict(), it returns the 
probability values against each input sample to be positive.   

__Return Value__  
It returns a numpy array of double (float64) type containing the prediction 
probability values. 

### save(filename)
__Parameters__   
_filename_: A string object containing the name of the file on which the target 
model is to be saved.    

__Purpose__    
On success, it writes the model information (weight values etc.) in the 
specified file as little-endian binary data. Otherwise, it throws an exception. 

__Return Value__  
It returns nothing.   

### load(filename)
__Parameters__   
_filename_: A string object containing the name of the file having model 
information to be loaded.    

__Purpose__    
It loads the model from the specified file (having little-endian binary data).

__Return Value__  
It simply returns "self" instance.   

### debug_print()

__Purpose__    
It shows the target model information (weight values etc.) on the server side 
user terminal. It is mainly used for debugging purpose.   

__Return Value__  
It returns nothing.   

### release()

__Purpose__    
It can be used to release the in-memory model at frovedis server. 

__Return Value__  
It returns nothing.   

# SEE ALSO  
logistic_regression, dvector, crs_matrix    
