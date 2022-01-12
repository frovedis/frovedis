% SVC  

# NAME

SVC (Support Vector Classification) - A classification algorithm used 
to predict the binary output with different kernel functions.  

# SYNOPSIS
                                       
    class frovedis.mllib.svm.SVC(C=1.0, kernel='rbf', degree=3, gamma='scale', coef0=0.0,  
                                 shrinking=True, probability=False, tol=0.001, cache_size=128,  
                                 class_weight=None, verbose=False, max_iter=-1,  
                                 decision_function_shape='ovr', break_ties=False,  
                                 random_state=None)  

## Public Member Functions

fit(X, y, sample_weight = None)   
predict(X)  
predict_proba(X)
load(fname, dtype = None)  
save(fname)  
score(X, y, sample_weight = None)  
get_params(deep = True)  
set_params(\*\*params)  
release()  
is_fitted()  

# DESCRIPTION
Support vector machines (SVMs) are a set of supervised learning methods used for 
classification and regression.

During training, the input X is the training data and y are their corresponding label 
values (Frovedis supports any values as for labels, but internally it encodes the input
binary labels to -1 and 1, before training at Frovedis server) which we want to predict.
**Frovedis supports only binary SVC classification algorithm**.  

This module provides a client-server implementation, where the client 
application is a normal python program. The frovedis interface is almost same 
as Scikit-learn SVC (Support Vector Classification) interface, but 
it doesn't have any dependency with Scikit-learn. It can be used simply even if 
the system doesn't have Scikit-learn installed. Thus in this implementation, a 
python client can interact with a frovedis server sending the required python
data for training at frovedis side. Python data is converted into frovedis 
compatible data internally and the python ML call is linked with the respective
frovedis ML call to get the job done at frovedis server.  

Python side calls for SVC on the frovedis server. Once the training 
is completed with the input data at the frovedis server, it returns an abstract 
model with a unique model ID to the client python program. 

When prediction-like request would be made on the trained model, python program
will send the same request to the frovedis server. After the request is served 
at the frovedis server, the output would be sent back to the python client. 

## Detailed Description  

### 1. SVC()   

__Parameters__       
_**C**_: A positive double (float64) parameter, also called as inverse of regularization 
strength. (Default: 1.0)  
_**kernel**_: A string object parameter that specifies the kernel type to be used. 
Unlike sklearn, frovedis supports 'linear', 'poly', 'rbf' and 'sigmoid' kernel function. 
 (Default: 'rbf')  
_**degree**_: A positive integer parameter specifying the degree of the polynomial kernel. 
 (Default: 3)  
_**gamma**_: A string object parameter that specifies the kernel coefficient. (Default: 'scale')  
For all python like input gamma ='scale' will be used, otherwise 'auto' will be used.  
Depending on the **kernel coefficient**, **gamma** will be calculated as:  
 - If **gamma = 'scale'** (default) is used then **gamma = 1.0 / (n_features * variance),**  
 - If **gamma = 'auto'** is used then **gamma = 1.0 / n_features**  

_**coef0**_: A double (float64) parameter which is independent term in kernel function. 
 (Default: 0.0)  
_**shrinking**_: An unused parameter. (Default: 'True')  
_**probability**_: An unused parameter. (Default: 'False')  
_**tol**_: A zero or positive double (float64) parameter specifying the convergence tolerance value. 
(Default: 0.001)  
_**cache\_size**_: An interger parameter that specifies the size of the kernel cache(in megabytes). 
It must be greater than 2. (Default: 128)  
_**class\_weight**_: An unused parameter. (Default: 'None')  
_**verbose**_: A boolean parameter that specifies the log level to use.  (Default: 'False')  
Its value is False by default (for INFO mode). But it can be set to True (for DEBUG mode or TRACE mode) for 
getting training time logs from frovedis server.  
_**max\_iter**_: A positive integer parameter specifying the hard limit on iterations within 
solver, or -1 for no limit. (Default: -1)  
_**decision\_function\_shape**_: An unused parameter. (Default: 'ovr')  
_**break\_ties**_: An unused parameter. (Default: 'False')  
_**random\_state**_: An unused parameter. (Default: 'None')  

__Attributes__  
_**coef\_**_: It is a python ndarray(containing float or double (float64) typed values 
depending on data-type of input matrix (X)). It is the weights assigned to the features. 
It has shape **(1, n_features)**.  
This attribute is supported only for 'linear' kernel, otherwise it will be None.  
_**intercept\_**_: It is a python ndarray(float or double (float64) values depending on 
input matrix data type) and has shape **(1,)**. It specifies the constants in decision function.  
This attribute is supported only for 'linear' kernel, otherwise it will be None.  
_**classes\_**_: It is an int64 type python ndarray of unique labels given to the classifier 
during training. It has shape **(n_classes,)**.  
_**support\_**_: It is an int32 type python ndarray that specifies the support vectors indices. 
It has shape **(n_support\_vectors\_,)**, where **n_support\_vectors\_ = len(support vectors) / n_features**  
_**support\_vectors\_**_: It is double (float64) type python ndarray specifies the support vectors. 
It has shape **(n_support\_vectors\_, n_features)**.   

__Purpose__    
It initializes a SVC object with the given parameters.   

The parameters: "shrinking", "probability", "class\_weight", "decision\_function\_shape", "break\_ties "and 
"random\_state" are simply kept to make the interface uniform to Scikit-learn SVC 
module. They are not used anywhere within frovedis implementation.  

__Return Value__    
It simply returns "self" reference.  

### 2. fit(X, y, sample_weight = None)
__Parameters__   
_**X**_: A numpy dense or scipy sparse matrix or any python ndarray. It has shape **(n_samples, n_features)**.  
For Frovedis input data:  
- if kernel = 'linear', then it can be an instance of FrovedisColmajorMatrix for dense data and 
FrovedisCRSMatrix for sparse data.  
- if kernel != 'linear', then it can be an instance of FrovedisRowmajorMatrix.  

_**y**_: Any python array-like object or an instance of FrovedisDvector containing 
the target labels. It has shape **(n_samples,)**.  
_**sample\_weight**_: A python ndarray containing the intended weights for each input
samples and it should be the shape of **(n_samples,)**. (Default: None)  
When it is None (not specified explicitly), an uniform weight vector is assigned on 
each input sample.  

__Purpose__    
It accepts the training feature matrix (X) and corresponding output labels (y) 
as inputs from the user and trains a SVC model at frovedis server.  

Depending upon the input data, usage of of 'gamma' will vary:  
- For all **python** data, **gamma = 'scale'** and **gamma = 'auto'** will be used.  
- For all **frovedis** like input, **gamma = 'auto'** only will be used.  

For example,   

    # loading a sample matrix and labels data
    mat = np.array([[-1.0, -1.0], [-2.0, -1.0], [1.0, 1.0], [2.0, 1.0]])
    lbl = np.array([1.0, 1.0, 2.0, 2.0])
    
    # fitting input matrix and label on SVC object
    from frovedis.mllib.svc import SVC
    svc = SVC(gamma ='auto').fit(mat, lbl)

When native python data is provided, it is converted to frovedis-like inputs
and sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when same 
data would be used for multiple executions.  

For example,

    # loading a sample matrix and labels data
    mat = np.array([[-1.0, -1.0], [-2.0, -1.0], [1.0, 1.0], [2.0, 1.0]])
    lbl = np.array([1.0, 1.0, 2.0, 2.0])
    
    # Since "mat" is numpy dense data and kernel = 'rbf' by default, 
    we have created FrovedisRowmajorMatrix.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    from frovedis.matrix.dvector import FrovedisDvector
    rmat = FrovedisRowmajorMatrix(mat)
    dlbl = FrovedisDvector(lbl)
    
    # SVC with pre-constructed frovedis-like inputs
    from frovedis.mllib.svm import SVC
    svc = SVC(gamma ='auto').fit(rmat, dlbl)
    
User can also provide FrovedisColmajorMatrix or FrovedisCRSMatrix as a
training data but only with **kernel = 'linear'** and **gamma ='auto'**.  

    # loading a sample matrix and labels data
    mat = np.array([[-1.0, -1.0], [-2.0, -1.0], [1.0, 1.0], [2.0, 1.0]])
    lbl = np.array([1.0, 1.0, 2.0, 2.0])
    
    # Since "mat" is numpy dense data and kernel = 'linear', we have created FrovedisColmajorMatrix
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisColmajorMatrix
    from frovedis.matrix.dvector import FrovedisDvector
    cmat = FrovedisColmajorMatrix(mat)
    dlbl = FrovedisDvector(lbl)
    
    # SVC with pre-constructed frovedis-like inputs
    from frovedis.mllib.svm import SVC
    svc = SVC(kernel = 'linear', gamma = 'auto').fit(cmat, dlbl)
    
__Return Value__  
It simply returns "self" reference.   

### 3. predict(X)
__Parameters__   
_**X**_: A numpy dense or scipy sparse matrix or any python ndarray or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data. 
It has shape **(n_samples, n_features)**.  

__Purpose__    
It accepts the test feature matrix (X) in order to make prediction on the 
trained model at frovedis server.  

For example,  

    svc.predict(mat)  
    
Output:  

    [1. 1. 2. 2.]  

Like in fit(), frovedis-like input can be used to speed-up the prediction making on 
the trained model at server side.  

For example,  

    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(mat)

    # predicting on SVC using frovedis-like input 
    svc.predict(rmat)
 
Output  
 
    [1. 1. 2. 2.]    
    
__Return Value__  
It returns a numpy array of int64, float or double (float64) type containing the predicted outputs. 
It has shape **(n_samples,)**.  

### 4. predict_proba(X)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python ndarray or an instance of 
FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data. It has 
shape **(n_samples, n_features)**.  

__Purpose__  
Perform classification on an array and return probability estimates for the test 
vector X.  

It accepts the test feature matrix (X) in order to make prediction on the trained 
model at frovedis server. Unlike sklearn, it performs the classification on an 
array and returns the probability estimates for the test feature matrix (X).  

**This method is not available for kernel = 'linear'.**  

For example,  

    # finds the probablity sample for each class in the SVC model
    svc.predict_proba(mat)

Output

    [[6.17597327e-04 9.99382403e-01]
     [1.98150882e-24 1.00000000e+00]
     [7.31058579e-01 2.68941421e-01]
     [9.38039081e-01 6.19609192e-02]]
    
Like in fit(), frovedis-like input can be used to speed-up the prediction making on 
the trained model at server side.  

For example,  

    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(mat)
    
    # finds the probablity sample for each class in the SVC model
    svc.predict_proba(rmat)
    
Output

    [[6.17597327e-04 9.99382403e-01]
     [1.98150882e-24 1.00000000e+00]
     [7.31058579e-01 2.68941421e-01]
     [9.38039081e-01 6.19609192e-02]]
   
__Return Value__  
It returns a numpy ndarray of float or double (float64) type and of shape 
**(n_samples, n_classes)** containing the predicted probability values.  

### 5. load(fname, dtype = None)
__Parameters__  
**fname**: A string object containing the name of the file having model information 
to be loaded.  
**dtype**: A data-type is inferred from the input data. Currently, expected input 
data-type is either float or double (float64). (Default: None)  

__Purpose__  
It loads the model from the specified file(having little-endian binary data).  

For example,  

    # loading the SVC model
    svc.load("./out/SVCModel")

__Return Value__  
It simply returns "self" reference.   

### 6. save(fname)
__Parameters__   
**fname**: A string object containing the name of the file on which the target 
model is to be saved.  

__Purpose__  
On success, it writes the model information (label_map, metadata and model) in the 
specified file as little-endian binary data. Otherwise, it throws an exception.  

For example,  

    # saving the model
    svc.save("./out/SVCModel")

The **SVCModel** contains below directory structure:  

**SVCModel**  
    |------label_map  
    |------metadata  
    |------model  

'label_map' contains information about labels mapped with their encoded value.  
'metadata' represents the detail about numbre of classes, model_kind and datatype of training vector.  
Here, the model file contains information about model in binary format.  

This will save the SVC model on the path '/out/SVCModel'. It would raise exception if the directory already 
exists with same name.  

__Return Value__  
It returns nothing.   

### 7. score(X, y, sample_weight = None)
__Parameters__  
**X**: A numpy dense or scipy sparse matrix or any python ndarray or an 
instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense 
data. It has shape **(n_samples, n_features)**.  
_**y**_: Any python ndarray containing the target labels. It has shape **(n_samples,)**.  
**sample_weight**: A python ndarray containing the intended weights for each input
samples and it should be the shape of **(n_samples,)**. (Default: None)  
When it is None (not specified explicitly), an uniform weight vector is assigned on 
each input sample.    

__Purpose__    
Calculate mean accuracy on the given test data and labels i.e. mean accuracy of 
self.predict(X) wrt. y.  

For example,  

    # calculate mean accuracy score on given test data and labels
    svc.score(mat, lbl)
    
Output

    1.0  
    
__Return Value__  
It returns an accuracy score of double (float64) type.

### 8. get_params(deep = True)  
__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. 
If True, will return the parameters for an estimator and contained subobjects that are 
estimators. (Default: True)

__Purpose__    
This method belongs to the BaseEstimator class inherited by SVC. It is used to get parameters
and their values of SVC class.  

For example, 
 
    print(svc.get_params())

Output  

    {'C': 1.0, 'break_ties': False, 'cache_size': 128, 'class_weight': None, 'coef0': 0.0, 
    'decision_function_shape': 'ovr', 'degree': 3, 'gamma': 0.5, 'kernel': 'rbf', 
    'max_iter': -1, 'probability': False, 'random_state': None, 'shrinking': True, 
    'tol': 0.001, 'verbose': False}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 9. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed this function as dictionary. This dictionary 
contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by SVC, used to set parameter values.  

For example,   

    print("get parameters before setting:") 
    print(svc.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    svc.set_params( kernel = "linear", shrinking = False)
    print("get parameters after setting:") 
    print(svc.get_params())

Output  
     
    get parameters before setting:  
    {'C': 1.0, 'break_ties': False, 'cache_size': 128, 'class_weight': None, 'coef0': 0.0, 
    'decision_function_shape': 'ovr', 'degree': 3, 'gamma': 0.5, 'kernel': 'rbf', 
    'max_iter': -1, 'probability': False, 'random_state': None, 'shrinking': True, 
    'tol': 0.001, 'verbose': False}  
    get parameters after setting:  
    {'C': 1.0, 'break_ties': False, 'cache_size': 128, 'class_weight': None, 'coef0': 0.0, 
    'decision_function_shape': 'ovr', 'degree': 3, 'gamma': 0.5, 'kernel': 'linear', 
    'max_iter': -1, 'probability': False, 'random_state': None, 'shrinking': False, 
    'tol': 0.001, 'verbose': False}  

__Return Value__  
It simply returns "self" reference.  

### 10. release()  

__Purpose__    
It can be used to release the in-memory model at frovedis server.  

For example,  

    svc.release()  

This will reset the after-fit populated attributes (like coef_, intercept_, classes_, etc) to 
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
dvector, crs_matrix, rowmajor_matrix, colmajor_matrix, linear_svc, linear_svr