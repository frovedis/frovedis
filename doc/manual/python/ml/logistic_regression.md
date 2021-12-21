% Logistic Regression

# NAME

Logistic Regression - A classification algorithm to predict the binary and 
multi-class output with logistic loss.  

# SYNOPSIS

    class frovedis.mllib.linear_model.LogisticRegression(penalty='l2', dual=False, tol=1e-4,  
                                                         C=100.0, fit_intercept=True,
                                                         intercept_scaling=1, class_weight=None,
                                                         random_state=None, solver='lbfgs',
                                                         max_iter=1000, multi_class='auto',
                                                         verbose=0, warm_start=False,
                                                         n_jobs=1, l1_ratio=None,
                                                         lr_rate=0.01, use_shrink=False)  

## Public Member Functions

fit(X, y, sample_weight = None)   
predict(X)  
predict_proba(X)  
score(X, y, sample_weight = None)  
get_params(deep = True)  
set_params(\*\*params)  
load(fname, dtype = None)  
save(fname)  
debug_print()   
release()   
is_fitted()  

# DESCRIPTION  
Classification aims to divide the items into categories. The most common classification 
type is binary classification, where there are two categories, usually named positive 
and negative. The other is multinomial classification, where there are more than two 
categories. **Frovedis supports both binary and multinomial logistic regression algorithms.** 
For multinomial classification, it uses softmax probability.  

Logistic regression is widely used to predict a binary response. 
It is a linear method with the loss function given by the **logistic loss**:  

    L(w;x,y) := log(1 + exp(-ywTx))    

During training, the input **X** is the training data and **y** is the corresponding 
label values (Frovedis supports any values as for labels, but internally it encodes the 
input binary labels to -1 and 1, and input multinomial labels to 0, 1, 2, ..., N-1 
(where N is the no. of classes) before training at Frovedis server) which we want 
to predict. **w** is the linear model (also called as weight) which uses a single weighted 
sum of features to make a prediction. **Frovedis Logistic Regression supports ZERO, 
L1 and L2 regularization to address the overfit problem. However, LBFGS solver 
supports only L2 regularization.**  

The gradient of the logistic loss is: **-y( 1 - 1 / (1 + exp(-ywTx))).x**    
The gradient of the L1 regularizer is: **sign(w)**     
And, the gradient of the L2 regularizer is: **w**  

For binary classification problems, the algorithm outputs a binary logistic 
regression model. Given a new data point, denoted by x, the model makes 
predictions by applying the logistic function:   

    f(z) := 1 / 1 + exp(-z)  

Where z = wTx. By default (threshold=0.5), if f(wTx) > 0.5, the response is 
positive (1), else the response is negative (0).   

In the multiclass case, the training algorithm uses the one-vs-rest (OvR) scheme. 
Currently the "multinomial" option is supported only by the "sag" solvers.  
Frovedis provides implementation of logistic regression with two different 
optimizers:  
(1) stochastic gradient descent with minibatch  
(2) LBFGS optimizer  
They can handle both dense and sparse input.

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

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis interface is almost same as Scikit-learn
Logistic Regression interface, but it doesn't have any dependency with Scikit-learn. 
It can be used simply even if the system doesn't have Scikit-learn installed.
Thus in this implementation, a python client can interact with a frovedis server
sending the required python data for training at frovedis side. Python data is
converted into frovedis compatible data internally and the python ML call is
linked with the respective frovedis ML call to get the job done at frovedis server. 

Python side calls for Logistic Regression on the frovedis server. Once the training
is completed with the input data at the frovedis server, it returns an abstract 
model with a unique model ID to the client python program.  

When prediction-like request would be made on the trained model, python program 
will send the same request to the frovedis server. After the request is served at 
the frovedis server, the output would be sent back to the python client.  

## Detailed Description  

### 1. LogisticRegression()   

__Parameters__  
**_penalty_**: A string object containing the regularizer type to use. Currenlty
none, l1 and l2 are supported by Frovedis. (Default: 'l2')  
**_dual_**: A boolean parameter. (unused)  
**_tol_**: A double (float64) type value specifying the convergence tolerance value.
It must be zero or a postive value. (Default: 1e-4)  
**_C_**: A positive float parameter, it is the inverse of regularization strength. 
Like in support vector machines, smaller values specify stronger regularization. (Default: 100.0)  
**_fit\_intercept_**: A boolean parameter specifying whether a constant (intercept)
should be added to the decision function. (Default: True)  
**_intercept\_scaling_**: An unused parameter. (Default: 1)  
**_class\_weight_**: An unused parameter. (Default: None)  
**_random\_state_**: An unused parameter. (Default: None)  
**_solver_**: A string object specifying the solver to use. (Default: 'lbfgs')  
It can be "sag" for frovedis side stochastic gradient descent or "lbfgs" for frovedis
side LBFGS optimizer when optimizing the logistic regression model.  
"sag" can handle L1, L2 or no penalty.  
"lbfgs" can handle only L2 penalty.  
**_max\_iter_**: A positive integer value specifying maximum iteration count. (Default: 1000)  
**_multi\_class_**: A string object specifying the type of classification. 
- If it is "auto" or "ovr", then a binary classification is selected when N = 2, otherwise 
multinomial classification is selected (where N is the no. of classes in training labels). 
- If it is "multinomial", then it always selects a multinomial problem (even when N = 2). 
Only "sag" solvers support multinomial classification currently. (Default: 'auto')  

**_verbose_**: An integer parameter specifying the log level to use. Its value is 0 
by default (for INFO mode and not explicitly specified). But it can be set to 1 (for DEBUG mode)
or 2 (for TRACE mode) for getting training time logs from frovedis server.  
**_warm\_start_**: A boolean parameter which when set to True, reuses the solution of the
previous call to fit as initialization, otherwise, just erase the previous solution. (Default: False)     
**_n\_jobs_**: An unused parameter. (Default: 1)  
**_l1\_ratio_**: An unused parameter. (Default: None)  
**_lr\_rate(alpha)_**: A double (float64) parameter containing the learning rate. (Default: 0.01)  
**_use\_shrink_**: A boolean parameter applicable only for "sag" solver with "sparse"
input (X). When set to True for sparse input, it can improve training performance by 
reducing communication overhead across participating processes. (Default: False)  

__Attributes__  
**_coef\__**: It is a python ndarray (float or double (float64) values depending 
on input matrix data type) of coefficient of the features in the decision function.
It has shape **(1, n_features)** when the given problem is "binary" and **(n_classes, n_features)** 
when it is a "multinomial" problem.  
**_intercept\_(bias)_**: It is a python ndarray(float or double (float64) values 
depending on input matrix data type) If fit_intercept is set to False, the intercept
is set to zero. It has shape **(1,)** when the given problem is "binary" and (n_classes) 
when its "multinomial" problem.  
**_classes\__**: It is a python ndarray (any type) of unique labels given to the classifier
during training. It has shape **(n_classes,)**.  
**_n\_iter\__**: It is a python ndarray of shape **(1,)** and has integer data. It is used to
get the actual iteration point at which the problem is converged.  

__Purpose__    
It initializes a Logistic Regression object with the given parameters.   

The parameters: "dual", "intercept_scaling", "class_weight", "random_state", "n_jobs" 
and "l1_ratio" are simply kept to make the interface uniform to the Scikit-learn 
Logistic Regression module. They are not used anywhere within the frovedis implementation.   

__Return Value__    
It simply returns "self" reference. 

### 2. fit(X, y, sample_weight = None)
__Parameters__   
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisColmajorMatrix for dense data.  
**_y_**: Any python array-like object or an instance of FrovedisDvector containing target 
labels. It has shape **(n_samples,)**.  
**_sample\_weight_**: A python ndarray containing the intended weights for each input
samples and it should be the shape of **(n_samples,)**.  
When it is None (not specified explicitly), an uniform weight vector is assigned on 
each input sample. (Default: None)  

__Purpose__    
It accepts the training feature matrix (X) and corresponding output labels (y) 
as inputs from the user and trains a logistic regression model with specified 
regularization with those data at frovedis server.  

For example,   

    # loading a sample matrix and labels data
    from sklearn.datasets import load_breast_cancer
    mat, lbl = load_breast_cancer(return_X_y = True)
    
    # fitting input matrix and label on LogisticRegression object
    from frovedis.mllib.linear_model import LogisticRegression
    lr = LogisticRegression(solver = 'lbfgs').fit(mat,lbl)

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,
    
    # loading a sample matrix and labels data
    from sklearn.datasets import load_breast_cancer
    mat, lbl = load_breast_cancer(return_X_y = True)
    
    # Since "mat" is numpy dense data, we have created FrovedisColmajorMatrix. 
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisColmajorMatrix
    from frovedis.matrix.dvector import FrovedisDvector 
    cmat = FrovedisColmajorMatrix(mat)
    dlbl = FrovedisDvector(lbl)
    
    # Logistic Regression with pre-constructed frovedis-like inputs
    from frovedis.mllib.linear_model import LogisticRegression
    lr = LogisticRegression(solver = 'lbfgs').fit(cmat, dlbl)

__Return Value__  
It simply returns "self" reference.    

### 3. predict(X)
__Parameters__   
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an
instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data.  

__Purpose__    
It accepts the test feature matrix (X) in order to make prediction on the 
trained model at frovedis server.  

For example,   

    # predicting on lbfgs logistic regression model
    lr.predict(mat)  

Output  

    [0 0 0 ... 0 0 1]

Like in fit(), frovedis-like input can be used to speed-up the prediction making on 
the trained model at server side.  

For example,  

    # Since "cmat" is FrovedisColmajorMatrix, we have created FrovedisRowmajorMatrix.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    
    # predicting on lbfgs logistic regression model using pre-constructed input
    lr.predict(cmat.to_frovedis_rowmatrix())

Output  

    [0 0 0 ... 0 0 1]

__Return Value__  
It returns a numpy array of float or double (float64) type and of shape **(n_samples,)** 
containing the predicted outputs.  

### 4. predict_proba(X)  
__Parameters__   
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data.   

__Purpose__    
It accepts the test feature matrix (X) in order to make prediction on the trained model 
at frovedis server. But unlike predict(), it returns the softmax probability matrix of 
shape **(n_samples, n_classes)** containing the probability of each class in each sample.  

For example,   

    # finds the probability sample for each class in the model
    lr.predict_proba(mat)  

Output
    
    [[1.46990588e-19 1.00000000e+00]
     [7.23344224e-10 9.99999999e-01]
     [8.43160984e-10 9.99999999e-01]
     ...
     [4.03499383e-04 9.99596501e-01]
     [3.03132738e-13 1.00000000e+00]
     [6.14030540e-03 9.93859695e-01]]

Like in fit(), frovedis-like input can be used to speed-up the prediction making on 
the trained model at server side.  

For example,  

    # Since "cmat" is FrovedisColmajorMatrix, we have created FrovedisRowmajorMatrix.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix

    # finds the probability sample for each class in the model
    lr.predict_proba(mat)  

Output
    
    [[1.46990588e-19 1.00000000e+00]
     [7.23344224e-10 9.99999999e-01]
     [8.43160984e-10 9.99999999e-01]
     ...
     [4.03499383e-04 9.99596501e-01]
     [3.03132738e-13 1.00000000e+00]
     [6.14030540e-03 9.93859695e-01]]

__Return Value__  
It returns an ndarray of float or double (float64) type and of shape **(n_samples, n_classes)** 
containing the prediction probability values. 

### 5. score(X, y, sample_weight = None)
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data.  
**_y_**: Any python array-like object containing true labels for X. It has shape **(n_samples,)**.  
**_sample\_weight_**: A python ndarray containing the intended weights for each input
samples and it should be the shape of **(n_samples,)**.  
When it is None (not specified explicitly), an uniform weight vector is assigned on each 
input sample. (Default: None)  

__Purpose__  
Calculate mean accuracy on the given test data and labels i.e. mean accuracy of 
self.predict(X) wrt. y.  

For example,   

    # calculate mean accuracy score on given test data and labels
    lr.score(mat,lbl)  

Output  

    0.96

__Return Value__  
It returns an accuracy score of float type.   

### 6. get_params(deep = True)  
__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If True, 
it will return the parameters for an estimator and contained subobjects that are estimators. (Default: True)  

__Purpose__    
This method belongs to the BaseEstimator class inherited by LogisticRegression. It is used to get parameters 
and their values of LogisticRegression class.  

For example, 
 
    print(lr.get_params())

Output  

    {'C': 100.0, 'class_weight': None, 'dual': False, 'fit_intercept': True, 
    'intercept_scaling': 1, 'l1_ratio': None, 'lr_rate': 0.01, 'max_iter': 1000, 
    'multi_class': 'auto', 'n_jobs': 1, 'penalty': 'l2', 'random_state': None, 
    'solver': 'sag', 'tol': 0.0001, 'use_shrink': False, 'verbose': 0, 
    'warm_start': False}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 7. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed to this function as dictionary. This dictionary 
contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by LogisticRegression, used to set 
parameter values.  

For example,   

    print("get parameters before setting:") 
    print(lr.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    lr.set_params(solver='lbfgs', max_iter = 10000) 
    print("get parameters after setting:") 
    print(lr.get_params())

Output  
     
    get parameters before setting:
    {'C': 100.0, 'class_weight': None, 'dual': False, 'fit_intercept': True, 
    'intercept_scaling': 1, 'l1_ratio': None, 'lr_rate': 0.01, 'max_iter': 1000, 
    'multi_class': 'auto', 'n_jobs': 1, 'penalty': 'l2', 'random_state': None, 
    'solver': 'sag', 'tol': 0.0001, 'use_shrink': False, 'verbose': 0, 
    'warm_start': False}
    get parameters after setting:
    {'C': 100.0, 'class_weight': None, 'dual': False, 'fit_intercept': True, 
    'intercept_scaling': 1, 'l1_ratio': None, 'lr_rate': 0.01, 'max_iter': 10000, 
    'multi_class': 'auto', 'n_jobs': 1, 'penalty': 'l2', 'random_state': None, 
    'solver': 'lbfgs', 'tol': 0.0001, 'use_shrink': False, 'verbose': 0, 
    'warm_start': False}
    
__Return Value__  
It simply returns "self" reference.  

### 8. load(fname, dtype = None)
__Parameters__   
**_fname_**: A string object containing the name of the file having model 
information to be loaded.    
**_dtype_**: A data-type is inferred from the input data. Currently, expected input 
data-type is either float or double (float64). (Default: None)

__Purpose__    
It loads the model from the specified file (having little-endian binary data).  

For example,   

    lr.load("./out/LRModel")

__Return Value__  
It simply returns "self" reference.   

### 9. save(fname)
__Parameters__   
**_fname_**: A string object containing the name of the file on which the target 
model is to be saved.    

__Purpose__    
On success, it writes the model information (label_map, metadata and model) in the 
specified file as little-endian binary data. Otherwise, it throws an exception.  

For example,   

    # To save the logistic regression model
    lr.save("./out/LRModel")    

This will save the logistic regression model on the path '/out/LRModel'. It would raise exception if 
the directory already exists with same name.  

The 'LRModel' directory has  

**LRModel**  
|----label_map  
|----metadata  
|----**model**  
\ \ \ \ \ |------model  
\ \ \ \ \ |------type  

‘label_map’ contains information about labels mapped with their encoded value.  
The metadata file contains the number of classes, model kind, input datatype used for trained model.  
Here, the **model** directory contains information about weights, intercept, threshold and thier datatype.  

__Return Value__  
It returns nothing.

### 10. debug_print()

__Purpose__    
It shows the target model information (weight values, intercept, etc.) on the server side 
user terminal. It is mainly used for debugging purpose.  

For example,  

    lr.debug_print()  

Output  

    -------- Weight Vector:: --------
    25.4745 47.8416 155.732 190.863 0.271114 0.0911008 -0.151433 -0.0785512 0.511576
    0.203452 0.199293 3.8659 1.22203 -42.3556 0.0239707 0.0395711 0.0389786 0.017432
    0.0647208 0.0105295 24.7162 60.7113 150.789 -148.921 0.354222 0.104251 -0.202345
    -0.0363726 0.734499 0.22635
    Intercept:: 60.7742
    Threshold:: 0.5

It displays the weights, intercept, etc. values on the trained model which is currently present 
on the server.

__Return Value__  
It returns nothing.  

### 11. release()

__Purpose__    
It can be used to release the in-memory model at frovedis server.  

For example,
 
    lr.release()

This will reset the after-fit populated attributes to None, along with releasing server 
side memory.  

__Return Value__  
It returns nothing.  

### 12. is_fitted()

__Purpose__    
It can be used to confirm if the model is already fitted or not. In case, predict() is used 
before training the model, then it can prompt the user to train the logistic regression model first. 

__Return Value__  
It returns 'True', if the model is already fitted otherwise, it returns 'False'.

# SEE ALSO  
linear_regression, dvector, crs_matrix, colmajor_matrix, rowmajor_matrix   