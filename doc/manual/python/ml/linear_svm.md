% Linear SVM  

# NAME

Linear SVM (Support Vector Machines) - A classification algorithm 
to predict the binary output with hinge loss.  

# SYNOPSIS

class frovedis.mllib.svm.LinearSVC(penalty='l2', loss='hinge', dual=True,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \  tol=1e-4, C=1.0, multi_class='ovr',   
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \  fit_intercept=True, intercept_scaling=1,   
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \  class_weight=None, verbose=0,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \  random_state=None, max_iter=1000,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \  lr_rate=0.01, solver='sag',  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \  warm_start=False)  

## Public Member Functions

fit(X, y, sample_weight = None)   
predict(X)  
load(fname, dtype = None)  
save(fname)  
score(X, y, sample_weight = None)  
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

Where the vectors x are the training data examples and y are their corresponding 
labels (Frovedis supports any values as for labels, but internally it encodes the
input binary labels to -1 and 1, before training at Frovedis server) which we 
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

Frovedis provides implementation of linear SVM with **stochastic gradient 
descent with minibatch**. 

The simplest method to solve optimization problems of the form **min f(w)** 
is gradient descent. Such first-order optimization methods well-suited for 
large-scale and distributed computation. 

This module provides a client-server implementation, where the client 
application is a normal python program. Frovedis is almost same as Scikit-learn
svm module providing the LinearSVC (Support Vector Classification) support, but 
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

### LinearSVC()   

__Parameters__       
_**penalty**_: A string object containing the regularizer type to use. Currently
none, l1 and l2 are supported by Frovedis. (Default: 'l2')   
_**loss**_: A string object containing the loss function type to use. Currently 
svm supports only hinge loss. (Default: 'hinge')    
_**dual**_: A boolean parameter (unused)      
_**tol**_: A double(float64) parameter specifying the convergence tolerance value. 
It must be zero or a positive value. (Default: 1e-4)     
_**C**_: A float parameter, also called as inverse of regularization strength. 
It must be positive. (Default: 1.0)   
_**multi\_class**_: A string object specifying type of classification. (unused)   
_**fit\_intercept**_: A boolean parameter specifying whether a constant (intercept) 
should be added to the decision function. (Default: True)     
_**intercept\_scaling**_: An integer parameter. (unused)    
_**class\_weight**_: A python dictionary or a string object. (unused)  
_**verbose**_: An integer parameter specifying the log level to use. Its value 
is set as 0 by default(for INFO mode). But it can be set to 1(for DEBUG mode) or 
2(for TRACE mode) for getting training time logs from frovedis server.  
_**random\_state**_: An integer, None or RandomState instance. (unused)   
_**max\_iter**_: An integer parameter specifying maximum iteration count. It is 
positive interger. (Default: 1000)    
_**lr_rate**_: A double(float64) parameter containing the learning rate. (Default: 0.01)  
_**solver**_: A string object specifying the solver to use. (Default: 'sag')  
“sag” handle L1, L2 or no penalty.  
_**warm_start**_: A boolean parameter which when set to True, reuses the solution of 
the previous call to fit as initialization, otherwise, just erase the previous solution. 
(Default: False)

__Attributes__  
_**coef\_**_: It is a python ndarray(containing float or double(float64) typed values 
depending on data-type of input matrix (X)). It is the weights assigned to the features. 
It has shape (1, n_features).  
_**classes\_**_: It is a python ndarray(any type) of unique labels given to the classifier 
during training. It has shape
(n_classes,).  
_**intercept\_**_: It is a python ndarray(float or double(float64) values depending on 
input matrix data type) and has shape(1,).   
_**n_iter**_: It is a python ndarray of shape(1,) and has integer data. It is used to 
get the actual iteration point at which the problem is converged.  

__Purpose__    
It initializes a LinearSVC object with the given parameters.   

The parameters: "dual", "intercept_scaling", "class_weight", "multi_class"and 
"random_state" are simply kept to make the interface uniform to Scikit-learn LinearSVC 
module. They are not used anywhere within frovedis implementation. 


__Return Value__    
It simply returns "self" reference. 

### fit(X, y, sample_weight = None)
__Parameters__   
_**X**_: A numpy dense or scipy sparse matrix or any python array-like object or an 
instance of FrovedisCRSMatrix for sparse data and FrovedisColmajorMatrix for 
dense data. It has shape(n_samples, n_features).  
_**y**_: Any python array-like object or an instance of FrovedisDvector.     
_**sample\_weight**_: Python array-like containing the intended weights for each input
samples and it should be the shape of (nsamples, ). When it is None (not specified), an
uniform weight vector is assigned on each input sample.  

__Purpose__    
It accepts the training feature matrix (X) and corresponding output labels (y) 
as inputs from the user and trains a linear svm model with specifed 
regularization with those data at frovedis server. 

For example,   

    # loading a sample matrix and labels data
    from sklearn.datasets import load_breast_cancer
    mat, lbl = load_breast_cancer(return_X_y = True)
    
    # fitting input matrix and label on linear SVC object
    from frovedis.mllib.svm import LinearSVC
    svm = LinearSVC().fit(mat, lbl)

When native python data is provided, it is converted to frovedis-like inputs
and sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, specially when same 
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

### predict(X)
__Parameters__   
_**X**_: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix 
for dense data. It has shape(n_samples, n_features).  

__Purpose__    
It accepts the test feature matrix (X) in order to make prediction on the 
trained model at frovedis server.

For example,

    svm.predict(mat)  
    
Output:  

    [0 0 0 ... 0 0 1]  
    
If the above pre-constructed training data (cmat) is to be used during prediction, 
the same can be used as follows:
 
    # predicting on LinearSVC using pre-constructed input
    svm.predict(cmat.to_frovedis_rowmatrix())
 
Output  
 
    [0 0 0 ... 0 0 1]    
    
__Return Value__  
It returns a numpy array of double(float64) type containing the predicted 
outputs. It has shape(n_samples,).

### load(fname, dtype = None)
__Parameters__  
**fname**: A string object containing the name of the file having model information 
to be loaded.  
**dtype**: A data-type is inferred from the input data. Currently, expected input 
data-type is either float or double(float64). (Default: None)  

__Purpose__  
It loads the model from the specified file(having little-endian binary data).  

For example,  

    # loading the svc model
    svm.load("./out/SVMModel")

__Return Value__  
It simply returns "self" instance.   


### save(fname)
__Parameters__   
**fname**: A string object containing the name of the file on which the target 
model is to be saved.    

__Purpose__    
On success, it writes the model information (weight values etc.) in the 
specified file as little-endian binary data. Otherwise, it throws an exception.  

For example,  

    # saving the model
    svm.save("./out/SVMModel")

__Return Value__  
It returns nothing.   

### score(X,  y,  sample_weight = None)
__Parameters__  
**X**: A numpy dense or scipy sparse matrix or any python array-like object or an 
instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense 
data. It has shape(n_samples, n_features).  
**y**: Any python array-like object or an instance of FrovedisDvector.  
**sample_weight**: Python array-like containing the intended weights for each input
samples and it should be the shape of (nsamples, ). When it is None (not specified), an
uniform weight vector is assigned on each input sample.  

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

### debug_print()

__Purpose__    
It shows the target model information(weight values etc.) on the server side 
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

__Return Value__  
It returns nothing.   

### release()

__Purpose__    
It can be used to release the in-memory model at frovedis server.  

For example,

    svm.release()
    
This will reset the after-fit populated attributes to None, along with 
releasing server side memory.  

__Return Value__  
It returns nothing.   

### is_fitted()

__Purpose__   
It can be used to confirm if the model is already fitted or not. In case, 
predict() is used before training the model, then it can prompt the user to 
train the model first.

__Return Value__  
It returns ‘True’, if the model is already fitted otherwise, it returns ‘False’.

# SEE ALSO  
logistic_regression, dvector, crs_matrix    
