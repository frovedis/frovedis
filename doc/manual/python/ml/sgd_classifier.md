% SGDClassifier

# NAME

SGDClassifier - A classification algorithm used to predict the labels 
with various loss functions. This estimator implements regularized linear 
models with stochastic gradient descent (SGD) learning.  

# SYNOPSIS

    class frovedis.mllib.linear_model.SGDClassifier(loss='hinge', penalty='l2', alpha=0.0001,  
                                                    l1_ratio=0.15, fit_intercept=True,  
                                                    max_iter=1000, tol=1e-3,  
                                                    shuffle=True, verbose=0,  
                                                    epsilon=0.1, n_jobs=None,  
                                                    random_state=None,  
                                                    learning_rate="invscaling",  
                                                    eta0=1.0, power_t=0.5,  
                                                    early_stopping=False,  
                                                    validation_fraction=0.1,  
                                                    n_iter_no_change=5,  
                                                    class_weight=None,  
                                                    warm_start=False,  
                                                    average=False)  
                                                    
## Public Member Functions

fit(X, y, coef_init = None, intercept_init = None, sample_weight = None)  
predict(X)  
predict_proba(X)  
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
classifiers under convex loss functions such as SVM and Logistic regression. 
It has been successfully applied to large-scale datasets because the update 
to the coefficients is performed for each training instance, rather than at 
the end of instances. **Frovedis supports both binary and multinomial Stochastic 
Gradient Descent (SGD) classifier algorithms.**  

Stochastic Gradient Descent (SGD) classifier basically implements a plain SGD 
learning routine supporting various loss functions and penalties for classification. 
It implements regularized linear models with stochastic gradient descent (SGD) 
learning: the gradient of the loss is estimated each sample at a time and the model 
is updated along the way with a decreasing strength schedule.
It is a linear method which uses the following loss functions:  
**1) hinge**  
**2) log**  
**3) squared_loss**  

**It supports ZERO, L1 and L2 regularization to address the overfit problem.**  

During training, the input **X** is the training data and **y** are their corresponding 
label values (Frovedis supports any values as for labels, but internally it encodes the
input binary labels to -1 and 1, before training at Frovedis server) which we 
want to predict. 

This module provides a client-server implementation, where the client 
application is a normal python program. The frovedis interface is almost same 
as Scikit-learn SGDClassifier (Stochastic Gradient Descent Classification) 
interface, but it doesn't have any dependency with Scikit-learn. It can be 
used simply even if the system doesn't have Scikit-learn installed. Thus in 
this implementation, a python client can interact with a frovedis server 
sending the required python data for training at frovedis side. Python data is 
converted into frovedis compatible data internally and the python ML call is 
linked with the respective frovedis ML call to get the job done at frovedis 
server.  

Python side calls for SGDClassifier on the frovedis server. Once the training 
is completed with the input data at the frovedis server, it returns an abstract 
model with a unique model ID to the client python program. 

When prediction-like request would be made on the trained model, python program
will send the same request to the frovedis server. After the request is served 
at the frovedis server, the output would be sent back to the python client. 

## Detailed Description  

### 1. SGDClassifier()   

__Parameters__  
_**loss**_: A string object parameter containing the loss function type to use. Currently, 
frovedis supports ‘hinge’, ‘log’and ‘squared_loss’ functions. 
(Default: 'hinge')  
_**penalty**_: A string object parameter containing the regularizer type to use. Currently
none, l1 and l2 are supported by Frovedis. (Default: 'l2')  
If it is None (not specified explicitly), it will be set as 'ZERO' regularization 
type.  
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
_**epsilon**_: An unused parameter. (Default: 0.1)  
_**n\_jobs**_: An unused parameter. (Default: None)  
_**random\_state**_: An unused parameter. (Default: None)  
_**learning\_rate**_: A string object parameter containing the learning rate. 
(Default: 'invscaling')  
Unlike sklearn, Frovedis only supports 'invscaling' learning rate. 
'invscaling' gradually decreases the learning rate **'learning_rate_'** at each 
time step **'t'** using an inverse scaling exponent of **'power_t'**.  
**learning_rate_ = eta0 / pow(t, power_t)**  
_**eta0**_: A double (float64) parameter specifying the initial learning rate for 
the ‘invscaling’ schedules. (Default: 1.0)  
_**power\_t**_: A double (float64) parameter specifying the exponent for inverse 
scaling learning rate. (Default: 0.5)  
_**early\_stopping**_: An unused parameter. (Default: False)  
_**validation\_fraction**_: An unused parameter. (Default: 0.1)  
_**n\_iter\_no_change**_: An unused parameter. (Default: 5)  
_**class\_weight**_: An unused parameter. (Default: None)  
_**warm\_start**_: A boolean parameter which when set to True, reuses the solution of 
the previous call to fit as initialization, otherwise, just erase the previous solution. 
(Default: False)  
_**average**_: An unused parameter. (Default: False)  

__Attributes__  
_**coef\_**_: It is a python ndarray(containing float or double (float64) typed values 
depending on data-type of input matrix (X)). It is the weights assigned to the features.  
Shape of this attribute depends on the n_classes.  
- If 'classes_' is 2, then the shape **(1, n_features)**  
- If 'classes_' is more then 2, then the shape is **(n_classes, n_features)**.  

_**intercept\_**_: It is a python ndarray(float or double (float64) values depending on 
input matrix data type) and has shape **(1,)**. It specifies the constants in decision function.  
_**classes\_**_: It is a python ndarray(any type) of unique labels given to the classifier 
during training. It has shape **(n_classes,)**. This attribute is not available for **squared_loss**.  
_**n_iter**_: An integer value used to get the actual iteration point at which the problem 
is converged.  

__Purpose__    
It initializes a SGDClassifier object with the given parameters.   

The parameters: "l1_ratio", "shuffle", "epsilon", "n_jobs", "random_state", "early_stopping", 
"validation_fraction", "n_iter_no_change", "class_weight" and "average" are simply kept to 
make the interface uniform to Scikit-learn SGDClassifier module. They are not used anywhere 
within frovedis implementation.  

__Return Value__    
It simply returns "self" reference.  

### 2. fit(X, y, coef_init = None,  intercept_init = None, sample_weight = None)
__Parameters__   
_**X**_: A numpy dense or scipy sparse matrix or any python array-like object or an 
instance of FrovedisCRSMatrix for sparse data and FrovedisColmajorMatrix for 
dense data. It has shape **(n_samples, n_features)**.  
_**y**_: Any python array-like object or an instance of FrovedisDvector containing the target labels.
It has shape **(n_samples,)**.  
_**coef_init**_: An unused parameter that specifies the initial coefficients to warm-start 
the optimization. (Default: None)  
_**intercept_init**_: An unused parameter that specifies the initial intercept to warm-start 
the optimization. (Default: None)  
_**sample\_weight**_: A python ndarray containing the intended weights for each input
samples and it should be the shape of **(n_samples,)**.  
When it is None (not specified explicitly), an uniform weight vector is assigned on 
each input sample. (Default: None)  

__Purpose__    
It accepts the training feature matrix (X) and corresponding output labels (y) 
as inputs from the user and trains a SGDClassifier model with specified 
regularization with those data at frovedis server. 

For example,   

    # loading a sample matrix and labels data
    from sklearn.datasets import load_breast_cancer
    mat, lbl = load_breast_cancer(return_X_y = True)
    
    # fitting input matrix and label on SGDClassifier object
    from frovedis.mllib.linear_model import SGDClassifier
    sgd_clf = SGDClassifier().fit(mat, lbl)

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
    dlbl = FrovedisDvector(lbl)
    
    # Linear SVC with pre-constructed frovedis-like inputs
    from frovedis.mllib.linear_model import SGDClassifier
    sgd_clf = SGDClassifier().fit(cmat,dlbl)

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

    sgd_clf.predict(mat)  
    
Output:  

    [0 0 0 1 . . . . 0 0 0 1]

Like in fit(), frovedis-like input can be used to speed-up the prediction making on 
the trained model at server side.  

For example,  

    # Since "cmat" is FrovedisColmajorMatrix, we have created FrovedisRowmajorMatrix.
    # predicting on SGDClassifier using frovedis-like input 
    sgd_clf.predict(cmat.to_frovedis_rowmatrix())
 
Output  
 
    [0 0 0 1 . . . . 0 0 0 1]

__Return Value__  
For **squared_loss** loss, it returns a numpy array of **double (float64)** type and for other
loss functions it returns a numpy array of **int64** type containing the predicted outputs.
It has shape **(n_samples,)**.  

### 4. predict_proba(X)
  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for 
dense data. It has shape **(n_samples, n_features)**.  

__Purpose__  
Perform classification on an array and return probability estimates for the test 
vector X.  

It accepts the test feature matrix (X) in order to make prediction on the trained 
model at frovedis server. Unlike sklearn, it performs the classification on an 
array and returns the probability estimates for the test feature matrix (X).  

**This method is not available for "hinge" and "squared_loss" function.**  

For example,  

    # finds the probablity sample for each class in the model
    sgd_clf.predict_proba(mat)

Output

    [[0. 1.]
    [0. 1.]
    [0. 1.]
    ...
    [0. 1.]
    [0. 1.]
    [0. 1.]]

Like in fit(), frovedis-like input can be used to speed-up the prediction making on 
the trained model at server side.  

For example,  

    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix.
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(mat)
    
    # finds the probablity sample for each class in the model
    sgd_clf.predict_proba(rmat)
    
Output

    [[0. 1.]
    [0. 1.]
    [0. 1.]
    ...
    [0. 1.]
    [0. 1.]
    [0. 1.]]
    
__Return Value__  
It returns a numpy array of float or double (float64) type and of shape 
**(n_samples, n_classes)** containing the predicted probability values.  

### 5. score(X,  y,  sample_weight = None)
__Parameters__  
**X**: A numpy dense or scipy sparse matrix or any python array-like object or an 
instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense 
data. It has shape **(n_samples, n_features)**.  
_**y**_: Any python array-like object containing the target labels. It has shape 
**(n_samples,)**.  
**sample_weight**: A python narray containing the intended weights for each input
samples and it should be the shape of **(n_samples,)**.  
When it is None (not specified explicitly), an uniform weight vector is assigned 
on each input sample. (Default: None)  

__Purpose__    
Calculate mean accuracy on the given test data and labels i.e. mean accuracy of 
self.predict(X) wrt. y.  

**For 'log' and 'hinge' loss, 'accuracy_score' is calculated and for 'squared_loss',
'r2_score' is calculated.**  

For example,  

    # calculate mean accuracy score on given test data and labels
    sgd_clf.score(mat, lbl)
     
Output

    0.91  
    
__Return Value__  
For 'log' and 'hinge' loss, it returns 'accuracy_score' and for 'squared_loss',
it returns 'r2_score' of double (float64) type.  
 
### 6. load(fname, dtype = None)
__Parameters__  
**fname**: A string object containing the name of the file having model information 
to be loaded.  
**dtype**: A data-type is inferred from the input data. Currently, expected input 
data-type is either float or double (float64). (Default: None)  
**For 'squared_loss' this method doesn't load the saved file 'label_map'.**  

__Purpose__  
It loads the model from the specified file(having little-endian binary data).  

For example,  

    # loading the SGDClassifier model
    sgd_clf.load("./out/SCLFModel")

__Return Value__  
It simply returns "self" instance.   

### 7. save(fname)
__Parameters__   
**fname**: A string object containing the name of the file on which the target 
model is to be saved.    

__Purpose__    
On success, it writes the model information (label_map, metadata and model) in the 
specified file as little-endian binary data. Otherwise, it throws an exception.  

For example,  

    # saving the model
    sgd_clf.save("./out/SCLFModel")
    
The SCLFModel contains below directory structure:  
**SCLFModel**  
    |------label_map  
    |------metadata  
    |------model    

'label_map' contains information about labels mapped with their encoded value. 
**This information is not saved for 'squared_loss' loss function.**  

'metadata' represents the detail about loss function, model_kind and datatype of 
training vector.  
Here, the model file contains information about model_id, model_kind and datatype 
of training vector.  

This will save the SGDClassifier model on the path ‘/out/SCLFModel’. It would raise 
exception if the directory already exists with same name.  

__Return Value__  
It returns nothing.   

### 8. get_params(deep = True)  

__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. 
If True, will return the parameters for an estimator and contained subobjects that are 
estimators. (Default: True)

__Purpose__    
This method belongs to the BaseEstimator class inherited by SGDClassifier. It is used to get parameters
and their values of SGDClassifier class.  

For example,  
 
    print(sgd_clf.get_params())

Output  

    {'alpha': 0.0001, 'average': False, 'class_weight': None, 
    'early_stopping': False, 'epsilon': 0.1, 'eta0': 1.0, 'fit_intercept': True, 
    'l1_ratio': 0.15, 'learning_rate': 'invscaling', 'loss': 'hinge', 'max_iter': 1000, 
    'n_iter_no_change': 5, 'n_jobs': None, 'penalty': 'l2', 'power_t': 0.5, 
    'random_state': None, 'shuffle': True, 'tol': 0.001, 'validation_fraction': 0.1, 
    'verbose': 0, 'warm_start': False}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 9. set_params(\*\*params)  

__Parameters__   
_**\*\*params**_: All the keyword arguments are passed this function as dictionary. This dictionary contains 
parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by SGDClassifier, used to set parameter values.  

For example,   

    print("Get parameters before setting:")
    print(sgd_clf.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    sgd_clf.set_params(penalty = 'l1', fit_intercept = False)
    print("Get parameters before setting:")
    print(sgd_clf.get_params()) 

Output  

    Get parameters before setting: 
    {'alpha': 0.0001, 'average': False, 'class_weight': None, 'early_stopping': False, 
    'epsilon': 0.1, 'eta0': 1.0, 'fit_intercept': True, 'l1_ratio': 0.15, 
    'learning_rate': 'invscaling', 'loss': 'hinge', 'max_iter': 1000, 
    'n_iter_no_change': 5, 'n_jobs': None, 'penalty': 'l2', 'power_t': 0.5, 
    'random_state': None, 'shuffle': True, 'tol': 0.001, 'validation_fraction': 0.1, 
    'verbose': 0, 'warm_start': False}
    Get parameters before setting: 
    {'alpha': 0.0001, 'average': False, 'class_weight': None, 'early_stopping': False, 
    'epsilon': 0.1, 'eta0': 1.0, 'fit_intercept': False, 'l1_ratio': 0.15, 
    'learning_rate': 'invscaling', 'loss': 'hinge', 'max_iter': 1000, 
    'n_iter_no_change': 5, 'n_jobs': None, 'penalty': 'l1', 'power_t': 0.5, 
    'random_state': None, 'shuffle': True, 'tol': 0.001, 'validation_fraction': 0.1, 
    'verbose': 0, 'warm_start': False}

__Return Value__  
It simply returns "self" reference.  

### 10. debug_print()

__Purpose__    
It shows the target model information(weight values, intercept, etc.) on the server side 
user terminal. It is mainly used for debugging purpose.  

For example,

    sgd_clf.debug_print() 
    
Output:  
    
    -------- Weight Vector:: --------
    14072.2 22454.1 83330.6 45451.2 139.907 -12.3658 -191.893 -82.7364 265.627 109.118 
    46.0452 1510.72 -250.729 -38123.1 9.74564 -4.16035 -16.0092 0.837207 26.3526 3.59399 
    14821.1 29161.9 85356 -57710.4 183.668 -99.8164 -348.61 -70.554 386.649 113.928
    Intercept:: 1845.32
    Threshold:: 0

This output will be visible on server side. It displays the weights, intercept and threshold values 
on the trained model which is currently present on the server.  

**No such output will be visible on client side.**  

__Return Value__  
It returns nothing.   

### 11. release()

__Purpose__    
It can be used to release the in-memory model at frovedis server.  

For example,

    sgd_clf.release()
    
This will reset the after-fit populated attributes to None, along with 
releasing server side memory.  

__Return Value__  
It returns nothing.   

### 12. is_fitted()

__Purpose__   
It can be used to confirm if the model is already fitted or not. In case, 
predict() is used before training the model, then it can prompt the user to 
train the model first.

__Return Value__  
It returns 'True', if the model is already fitted otherwise, it returns 'False'.

# SEE ALSO  
rowmajor_matrix, colmajor_matrix, dvector, crs_matrix, sgd_regressor    
