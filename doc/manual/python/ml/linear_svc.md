% LinearSVC  

# NAME

LinearSVC (Support Vector Classification) - A classification algorithm used 
to predict the binary output with hinge loss.  

# SYNOPSIS

    class frovedis.mllib.svm.LinearSVC(penalty='l2', loss='hinge', dual=True, tol=1e-4,  
                                       C=1.0, multi_class='ovr', fit_intercept=True,  
                                       intercept_scaling=1, class_weight=None,  
                                       verbose=0, random_state=None,  
                                       max_iter=1000, lr_rate=0.01,  
                                       solver='sag', warm_start=False)  
                                       

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
Classification aims to divide items into categories. 
The most common classification type is binary classification, where there are 
two categories, usually named positive and negative. **Frovedis supports only binary 
Linear SVM classification algorithm**. 

The Linear SVM is a standard method for large-scale classification tasks. 
It is a linear method with the loss function given by the **hinge loss**:   

    L(w;x,y) := max{0, 1-ywTx}

During training, the input **X** is the training data and **y** are their corresponding 
label values (Frovedis supports any values as for labels, but internally it encodes the
input binary labels to -1 and 1, before training at Frovedis server) which we 
want to predict. w is the linear model (also known as weight) which uses a 
single weighted sum of features to make a prediction. Linear SVM supports ZERO, 
L1 and L2 regularization to address the overfit problem.     

The gradient of the hinge loss is: -y.x, if ywTx < 1, 0 otherwise.    
The gradient of the L1 regularizer is: **sign(w)**     
And The gradient of the L2 regularizer is: **w**     

For binary classification problems, the algorithm outputs a binary svm 
model. Given a new data point, denoted by x, the model makes 
predictions based on the value of wTx. 

By default (threshold=0), if wTx >= 0, then the response is positive (1), 
else the response is negative (0).

Frovedis provides implementation of linear SVM with **stochastic gradient 
descent with minibatch**. 

The simplest method to solve optimization problems of the form **min f(w)** 
is gradient descent. Such first-order optimization methods well-suited for 
large-scale and distributed computation. 

This module provides a client-server implementation, where the client 
application is a normal python program. The frovedis interface is almost same 
as Scikit-learn LinearSVC (Support Vector Classification) interface, but 
it doesn't have any dependency with Scikit-learn. It can be used simply even if 
the system doesn't have Scikit-learn installed. Thus in this implementation, a 
python client can interact with a frovedis server sending the required python
data for training at frovedis side. Python data is converted into frovedis 
compatible data internally and the python ML call is linked with the respective
frovedis ML call to get the job done at frovedis server.  

Python side calls for LinearSVC on the frovedis server. Once the training 
is completed with the input data at the frovedis server, it returns an abstract 
model with a unique model ID to the client python program. 

When prediction-like request would be made on the trained model, python program
will send the same request to the frovedis server. After the request is served 
at the frovedis server, the output would be sent back to the python client. 

## Detailed Description  

### 1. LinearSVC()   

__Parameters__       
_**penalty**_: A string object containing the regularizer type to use. Currently
none, l1 and l2 are supported by Frovedis. (Default: 'l2')  
_**loss**_: A string object containing the loss function type to use. Unlike
 Scikit-learn, currently it supports only hinge loss. (Default: 'hinge')  
_**dual**_: An unused parameter. (Default: True)  
_**tol**_: A double (float64) parameter specifying the convergence tolerance value. 
It must be zero or a positive value. (Default: 1e-4)  
_**C**_: A positive float parameter, also called as inverse of regularization 
strength. (Default: 1.0)  
_**multi\_class**_: An unused parameter. (Default: 'ovr')  
_**fit\_intercept**_: A boolean parameter specifying whether a constant (intercept) 
should be added to the decision function. (Default: True)  
_**intercept\_scaling**_: An unused parameter. (Default: 1)  
_**class\_weight**_: An unused parameter. (Default: 'None')  
_**verbose**_: An integer parameter specifying the log level to use. Its value 
is set as 0 by default (for INFO mode). But it can be set to 1 (for DEBUG mode) or 
2 (for TRACE mode) for getting training time logs from frovedis server.  
_**random\_state**_: An unused parameter. (Default: 'None')  
_**max\_iter**_: A positive integer parameter specifying maximum iteration count. (Default: 1000)  
_**lr_rate**_: A double (float64) parameter containing the learning rate. (Default: 0.01)  
_**solver**_: A string object specifying the solver to use. (Default: 'sag')  
“sag” can handle L1, L2 or no penalty.  
_**warm_start**_: A boolean parameter which when set to True, reuses the solution of 
the previous call to fit as initialization, otherwise, just erase the previous solution. 
(Default: False)  

__Attributes__  
_**coef\_**_: It is a python ndarray(containing float or double (float64) typed values 
depending on data-type of input matrix (X)). It is the weights assigned to the features. 
It has shape **(1, n_features)**.  
_**classes\_**_: It is a python ndarray(any type) of unique labels given to the classifier 
during training. It has shape **(n_classes,)**.  
_**intercept\_**_: It is a python ndarray(float or double (float64) values depending on 
input matrix data type) and has shape **(1,)**.   
_**n_iter**_: It is a python ndarray of shape **(1,)** and has integer data. It is used to 
get the actual iteration point at which the problem is converged.  

__Purpose__    
It initializes a LinearSVC object with the given parameters.   

The parameters: "dual", "intercept_scaling", "class_weight", "multi_class"and 
"random_state" are simply kept to make the interface uniform to Scikit-learn LinearSVC 
module. They are not used anywhere within frovedis implementation.  

__Return Value__    
It simply returns "self" reference.  

### 2. fit(X, y, sample_weight = None)
__Parameters__   
_**X**_: A numpy dense or scipy sparse matrix or any python array-like object or an 
instance of FrovedisCRSMatrix for sparse data and FrovedisColmajorMatrix for 
dense data. It has shape **(n_samples, n_features)**.  
_**y**_: Any python array-like object or an instance of FrovedisDvector containing 
the target labels. It has shape **(n_samples,)**.  
_**sample\_weight**_: A python ndarray containing the intended weights for each input
samples and it should be the shape of **(n_samples, )**.  
When it is None (not specified explicitly), an uniform weight vector is assigned on 
each input sample. (Default: None)  

__Purpose__    
It accepts the training feature matrix (X) and corresponding output labels (y) 
as inputs from the user and trains a linear svm model with specifed 
regularization with those data at frovedis server. 

For example,   

    # loading a sample matrix and labels data
    from sklearn.datasets import load_breast_cancer
    mat, lbl = load_breast_cancer(return_X_y = True)
    
    # fitting input matrix and label on LinearSVC object
    from frovedis.mllib.svm import LinearSVC
    svm = LinearSVC().fit(mat, lbl)

When native python data is provided, it is converted to frovedis-like inputs
and sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when same 
data would be used for multiple executions.  

For example,

    # loading a sample matrix and labels data
    from sklearn.datasets import load_breast_cancer
    mat, lbl = load_breast_cancer(return_X_y = True)
    
    # Since "mat" is numpy dense data, we have created FrovedisColmajorMatrix.
    and for scipy sparse data, FrovedisCRSMatrix should be used. 
    from frovedis.matrix.dense import FrovedisColmajorMatrix
    from frovedis.matrix.dvector import FrovedisDvector
    cmat = FrovedisColmajorMatrix(mat)
    
    # Linear SVC with pre-constructed frovedis-like inputs
    from frovedis.mllib.svm import LinearSVC
    svm = LinearSVC().fit(cmat,dlbl)
    
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

    svm.predict(mat)  
    
Output:  

    [0 0 0 ... 0 0 1]  

Like in fit(), frovedis-like input can be used to speed-up the prediction making on 
the trained model at server side.  

For example,  

    # Since "cmat" is FrovedisColmajorMatrix, we have created FrovedisRowmajorMatrix.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix

    # predicting on LinearSVC using frovedis-like input 
    svm.predict(cmat.to_frovedis_rowmatrix())
 
Output  
 
    [0 0 0 ... 0 0 1]    
    
__Return Value__  
It returns a numpy array of double (float64) type containing the predicted 
outputs. It has shape **(n_samples,)**.

### 4. load(fname, dtype = None)
__Parameters__  
**fname**: A string object containing the name of the file having model information 
to be loaded.  
**dtype**: A data-type is inferred from the input data. Currently, expected input 
data-type is either float or double (float64). (Default: None)  

__Purpose__  
It loads the model from the specified file(having little-endian binary data).  

For example,  

    # loading the LinearSVC model
    svm.load("./out/SVMModel")

__Return Value__  
It simply returns "self" instance.   


### 5. save(fname)
__Parameters__   
**fname**: A string object containing the name of the file on which the target 
model is to be saved.    

__Purpose__    
On success, it writes the model information (label_map, metadata and model) in the 
specified file as little-endian binary data. Otherwise, it throws an exception.  

For example,  

    # saving the model
    svm.save("./out/SVMModel")
    
The SVMModel contains below directory structure:  
**SVMModel**  
    |------label_map  
    |------metadata  
    |------model    

'label_map' contains information about labels mapped with their encoded value.  
'metadata' represents the detail about model_kind and datatype of training vector.  
Here, the model file contains information about model_id, model_kind and datatype of training vector.  

This will save the LinearSVC model on the path ‘/out/SVMModel’. It would raise exception if the directory already 
exists with same name.  

__Return Value__  
It returns nothing.   

### 6. score(X,  y,  sample_weight = None)
__Parameters__  
**X**: A numpy dense or scipy sparse matrix or any python array-like object or an 
instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense 
data. It has shape **(n_samples, n_features)**.  
_**y**_: Any python array-like object containing the target labels. It has shape 
**(n_samples,)**.  
**sample_weight**: A python narray containing the intended weights for each input
samples and it should be the shape of **(n_samples, )**.  
When it is None (not specified explicitly), an uniform weight vector is assigned on 
each input sample. (Default: None)  

__Purpose__    
Calculate mean accuracy on the given test data and labels i.e. mean accuracy of 
self.predict(X) wrt. y.  

For example,  

     # calculate mean accuracy score on given test data and labels
     svm.score(mat, lbl)
     
Output

    0.63  
    
__Return Value__  
It returns an accuracy score of float type.


### 7. get_params(deep = True)  

__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If True, will return the 
parameters for an estimator and contained subobjects that are estimators. (Default: True)

__Purpose__    
This method belongs to the BaseEstimator class inherited by LinearSVC. It is used to get parameters
and their values of LinearSVC class.  

For example, 
 
    print(svm.get_params())

Output  

    {'C': 1.0, 'class_weight': None, 'dual': True, 'fit_intercept': True, 'intercept_scaling': 1,
    'loss': 'hinge', 'lr_rate': 0.01, 'max_iter': 1000,'multi_class': 'ovr', 'penalty': 'l2',
    'random_state': None, 'solver': 'sag','tol': 0.0001, 'verbose': 0,'warm_start': False}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 8. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed this function as dictionary. This dictionary 
contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by LinearSVC, used to set parameter values.  

For example,   

    print("Get parameters before setting:") 
    print(svm.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    svm.set_params( penalty='l1', dual=False)
    print("Get parameters after setting:") 
    print(svm.get_params())

Output  
     
    Get parameters before setting: 
    {'C': 1.0, 'class_weight': None, 'dual': True, 'fit_intercept': True, 
    'intercept_scaling': 1,'loss': 'hinge', 'lr_rate': 0.01, 'max_iter': 1000, 
    'multi_class': 'ovr','penalty': 'l2', 'random_state': None, 'solver': 'sag', 
    'tol': 0.0001, 'verbose': 0,'warm_start': False}
    Get parameters after setting: 
    {'C': 1.0, 'class_weight': None, 'dual': False, 'fit_intercept': True, 
    'intercept_scaling': 1,'loss': 'hinge', 'lr_rate': 0.01, 'max_iter': 1000, 
    'multi_class': 'ovr','penalty': 'l1', 'random_state': None, 'solver': 'sag', 
    'tol': 0.0001, 'verbose': 0,'warm_start': False}

__Return Value__  
It simply returns "self" reference.  

### 9. debug_print()

__Purpose__    
It shows the target model information(weight values, intercept, etc.) on the server side 
user terminal. It is mainly used for debugging purpose.  

For example,

    svm.debug_print() 
    
Output:  
    
    -------- Weight Vector:: --------
     83.7418 122.163 486.84 211.922 1.32991 0.287324 -0.867741 -0.0505454 
     2.04889 1.16388 0.750738 8.61861 -2.13628 -234.118 0.582984 0.445561 
     0.353854 0.519177 0.667717 0.547778 89.3196 157.824 499.367 
     -293.736 1.56023 -0.636429 -2.30027 -0.061839 2.66517 1.15244  
     Intercept:: 19.3242
     Threshold:: 0

This output will be visible on server side. It displays the weights, intercept and threshold values 
on the trained model which is currently present on the server.  

**No such output will be visible on client side.**  

__Return Value__  
It returns nothing.   

### 10. release()

__Purpose__    
It can be used to release the in-memory model at frovedis server.  

For example,

    svm.release()
    
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
It returns 'True', if the model is already fitted otherwise, it returns 'False'.  

# SEE ALSO  
- **[Introduction to FrovedisRowmajorMatrix](../matrix/rowmajor_matrix.md)**  
- **[Introduction to FrovedisCRSMatrix](../matrix/crs_matrix.md)**  
- **[Introduction to FrovedisDvector](../matrix/dvector.md)**  
- **[LinearSVR in Frovedis](./linear_svr.md)**  
- **[SVC in Frovedis](./svc.md)**  
