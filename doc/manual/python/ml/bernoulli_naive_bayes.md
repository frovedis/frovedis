% Bernoulli Naive Bayes

# NAME

BernoulliNB - One of the variations of Naive Bayes algorithm. It is a classification algorithm 
to predict only binary output.  

# SYNOPSIS

frovedis.mllib.naive_bayes.BernoulliNB(alpha=1.0, fit_prior=True,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ class_prior=None, binarize=0.0,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ verbose=0)  

## Public Member Functions  

fit(X, y, sample_weight = None)  
predict(X)  
predict_proba(X)  
score(X, y, sample_weight = None)  
get_params(deep = True)  
set_params(\*\*params)  
debug_print()  
load(fname, dtype = None)  
save(fname)  
release()  
is_fitted()  

# DESCRIPTION  
Naive Bayes classifier for bernoulli models.  

The Bernoulli Naive Bayes classifier is suitable for classification with 
binary/boolean features. In the multivariate Bernoulli event model, features 
are independent booleans (binary variables) describing inputs.  

This model is popular for document classification tasks, where binary term 
occurrence features (i.e a word occurs in a document or not) are used rather than 
finding the frequency of a word in document.  

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis interface is almost same as Scikit-learn
BernoulliNB interface, but it doesn't have any dependency with Scikit-learn. 
It can be used simply even if the system doesn't have Scikit-learn installed.
Thus in this implementation, a python client can interact with a frovedis server
sending the required python data for training at frovedis side. Python data is
converted into frovedis compatible data internally and the python ML call is
linked with the respective frovedis ML call to get the job done at frovedis server. 

Python side calls for BernoulliNB on the frovedis server. Once the training
is completed with the input data at the frovedis server, it returns an abstract 
model with a unique model ID to the client python program.  

When prediction-like request would be made on the trained model, python program 
will send the same request to the frovedis server. After the request is served at 
the frovedis server, the output would be sent back to the python client. 

## Detailed Description  

### 1. BernoulliNB()  
__Parameters__  
**_alpha_**: A positive double(float64) smoothing parameter (0 for no smoothing). It must be 
greater than or equal to 1. (Default: 1.0)  
**_fit\_prior_**: A boolean parameter specifying whether to learn class prior probabilities or 
not. If False, a uniform prior will be used. (Default: True)  
**_class\_prior_**: A numpy ndarray of double(float64) type values and must be of shape **(n_classes,)**. 
It gives prior probabilities of the classes. (Default: None)  
When it is None (not specified explicitly), the priors are adjusted according to the data.  
**_binarize_**: A double(float64) parameter specifying the threshold for binarizing sample features. (Default: 0.0)  
**_verbose_**: An integer parameter specifying the log level to use. Its value is 0 by 
default(for INFO mode). But it can be set to 1 (for DEBUG mode) or 2 (for TRACE mode) for getting 
training time logs from frovedis server.  

__Attributes__  
**_class\_log\_prior\__**: A python ndarray of double(float64) type values and has shape **(n_classes,)**. It 
contains log probability of each class (smoothed).  
**_feature\_log\_prob\__**: A python ndarray of double(float64) type values and has shape **(n_classes, n_features)**. 
It contains empirical log probability of features given a class, P(x_i|y).  
**_class\_count\__**: A python ndarray of double(float64) type values and has shape **(n_classes,)**. It 
contains the number of samples encountered for each class during fitting. This value is weighted 
by the sample weight when provided.  
**_classes\__**: A python ndarray of double(float64) type values and has shape **(n_classes,)**. It 
contains the of unique labels given to the classifier during training.  
**_feature\_count\__**: A python ndarray of double(float64) type values and has shape **(n_classes, n_features)**. 
It contains the number of samples encountered for each (class, feature) during fitting. This 
value is weighted by the sample weight when provided.  
**_coef\__**: A python ndarray of double(float64) type values. If 'classess_' is 2, then its shape **(1, n_features)**, 
otherwise, the shape is (n_classes, n_features). It mirrors 'feature_log_prob_' for interpreting BernoulliNB 
as a linear model.  
**_intercept\__**: A python ndarray of double(float64) type values. If 'classes_' is 2, the its shape **(1,)**, 
otherwise, the shape is (n_classes,). It mirrors 'class_log_prior_' for interpreting BernoulliNB as a linear model.  

__Purpose__  
It initializes a BernoulliNB object with the given parameters.  

__Return Value__  
It simply returns "self" reference.  

### 2. fit(X, y, sample_weight = None)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data.  
**_y_**: Any python array-like object or an instance of FrovedisDvector.  
**_sample\_weight_**: A python ndarray containing the intended weights for each input
samples and it should be the shape of **(n_samples, )**. When it is None (not specified explicitly), an 
uniform weight vector is assigned on each input sample. (Default: None)  

__Purpose__  
It accepts the training matrix (X) with labels (y) and trains a BernoulliNB model.  

For example,  

    # loading a sample matrix and labels data  
    from sklearn.datasets import load_breast_cancer  
    mat, lbl = load_breast_cancer(return_X_y = True)  
    
    # fitting input matrix and label on BernoulliNB object  
    from frovedis.mllib.linear_model import BernoulliNB  
    bnb = BernoulliNB(alpha = 1.0).fit(mat,lbl)  

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, specially when 
same data would be used for multiple executions.  

For example,
    
    # loading a sample matrix and labels data  
    from sklearn.datasets import load_breast_cancer  
    mat, lbl = load_breast_cancer(return_X_y = True)  
    
    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix.  
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.  
    from frovedis.matrix.dense import FrovedisRowmajorMatrix  
    from frovedis.matrix.dvector import FrovedisDvector  
    cmat = FrovedisRowmajorMatrix(mat)  
    dlbl = FrovedisDvector(lbl)  
    
    # BernoulliNB with pre-constructed frovedlis-like inputs  
    from frovedis.mllib.linear_model import BernoulliNB  
    bnb = BernoulliNB(alpha = 1.0).fit(cmat, dlbl)  

__Return Value__  
It simply returns "self" reference.  

### 3. predict(X)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data.  

__Purpose__  
It accepts the test feature matrix (X) in order to make prediction on the trained model at 
frovedis server.  

For example,   

    # predicting on BernoulliNB model
    bnb.predict(mat)  

Output  

    [1 1 1 ... 1 1 1]

Like in fit(), frovedis-like input can be used to speed-up the prediction making on 
the trained model at server side.  

For example,  

    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix. 
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.  
    from frovedis.matrix.dense import FrovedisRowmajorMatrix  
    rmat = FrovedisRowmajorMatrix(mat)  
    
    # predicting on BernoulliNB model using pre-constructed input  
    bnb.predict(rmat)  

Output  

    [1 1 1 ... 1 1 1]

__Return Value__  
It returns a numpy array of float or double(float64) type and of shape **(n_samples,)** 
containing the predicted outputs.  

### 4. predict_proba(X)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data.  

__Purpose__  
It accepts the test feature matrix (X) in order to make prediction on the trained model at 
frovedis server. Unlike sklearn, it performs the classification on an array and returns the 
probability estimates for the test feature matrix (X).  

For example,   

    # finds the probablity sample for each class in the model
    bnb.predict_proba(mat)  

Output

     [[0.35939685 0.64060315]
      [0.35939685 0.64060315]
      [0.35939685 0.64060315]
      ...
      [0.35939685 0.64060315]
      [0.35939685 0.64060315]
      [0.35939685 0.64060315]]

Like in fit(), frovedis-like input can be used to speed-up the prediction making on 
the trained model at server side.  

For example,  

    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix. 
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.  
    from frovedis.matrix.dense import FrovedisRowmajorMatrix  
    rmat = FrovedisRowmajorMatrix(mat)  
    
    # finds the probablity sample for each class in the model
    bnb.predict_proba(rmat)  

Output
    
    [[0.35939685 0.64060315]
     [0.35939685 0.64060315]
     [0.35939685 0.64060315]
     ...
     [0.35939685 0.64060315]
     [0.35939685 0.64060315]
     [0.35939685 0.64060315]]
     
__Return Value__  
It returns a numpy array of float or double(float64) type and of shape **(n_samples, n_classes)** 
containing the prediction probability values.  

### 5. score(X, y)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data.  
**_y_**: Any python array-like object or an instance of FrovedisDvector.     

__Purpose__  
Calculate mean accuracy on the given test data and labels i.e. mean accuracy of 
self.predict(X) wrt. y.  

For example,   

    # calculate mean accuracy score on given test data and labels
    bnb.score(mat,lbl)  

Output  

    0.6274

__Return Value__  
It returns an accuracy score of float type. 

### 6. debug_print()  

__Purpose__    
It shows the target model information on the server side user terminal. It is mainly used 
for debugging purpose.  

For example,  

    bnb.debug_print()  

Output  

    model_type: bernoulli
    binarize: -1
    feature_count: 212 212 212 212 212  ... 357 357 357 357 357
    theta: node = 0, local_num_row = 2, local_num_col = 30, val = -0.00468385 -0.00468385 -0.00468385 
    -0.00468385 -0.00468385 ... -0.0027894 -0.0027894 -0.0027894 -0.0027894 -0.0027894
    pi: -0.987294 -0.466145
    label: 0 1
    class count: 212 357
    theta_minus_negtheta: node = 0, local_num_row = 2, local_num_col = 30, val = 5.36129 5.36129 5.36129 
    5.36129 5.36129 ... 5.88053 5.88053 5.88053 5.88053 5.88053
    negtheta_sum: -160.979 -176.5
    
It displays the target model information like model_type, binarize, feature_count, theta, pi, etc. values on 
the trained model which is currently present on the server.

__Return Value__  
It returns nothing.  

### 7. get_params(deep = True)  
__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If True, 
it will return the parameters for an estimator and contained subobjects that are estimators. (Default: True)  

__Purpose__    
This method belongs to the BaseEstimator class inherited by BernoulliNB. It is used to get 
parameters and their values of BernoulliNB class.  

For example, 
 
      print(bnb.get_params())

Output  

    {'alpha': 1.0, 'binarize': -1.0, 'class_prior': None, 'fit_prior': True, 'verbose': 0}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 8. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed this function as dictionary. This dictionary 
contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by BernoulliNB, used to set 
parameter values.  

For example,   

    print("get parameters before setting:") 
    print(bnb.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    bnb.set_params(n_clusters = 4) 
    print("get parameters after setting:") 
    print(bnb.get_params())

Output  
     
    get parameters before setting:
    {'alpha': 1.0, 'binarize': -1.0, 'class_prior': None, 'fit_prior': True, 'verbose': 0}
    get parameters after setting:
    {'alpha': 1.0, 'binarize': 0.5, 'class_prior': None, 'fit_prior': True, 'verbose': 0}
    
__Return Value__  
It simply returns "self" reference.  


### 9. load(fname, dtype=None)  
__Parameters__  
**_fname_**: A string object containing the name of the file having model information such as 
theta, cls_count, feature_count, label, pi, type to be loaded.    
**_dtype_**: A data-type is inferred from the input data. Currently, expected input 
data-type is either float or double(float64). (Default: None)  

__Purpose__  
It loads the model from the specified file (having little-endian binary data).  

For example,   

    bnb.load("./out/BNBModel")

__Return Value__  
It simply returns "self" reference.  

### 10. save(fname)  
__Parameters__  
**_fname_**: A string object containing the name of the file on which the target 
model is to be saved.  

__Purpose__  
On success, it writes the model information (theta, cls_count, feature_count, label, pi, type) 
in the specified file as little-endian binary data. Otherwise, it throws an exception.  

For example,   

    # To save the bernoulli naive bayes model
    bnb.save("./out/BNBModel")    

This will save the naive bayes model on the path '/out/BNBModel'. It would raise exception 
if the directory already exists with same name.  

The 'BNBModel' directory has  

**BNBModel**  
|----label_map  
|----metadata  
|----**model**  
\ \ \ \ \ |------cls_count  
\ \ \ \ \ |------feature_count  
\ \ \ \ \ |------label  
\ \ \ \ \ |------pi  
\ \ \ \ \ |------**theta**  
\ \ \ \ \ |------type  

‘label_map’ contains information about labels mapped with their encoded value.  
The metadata file contains the  model kind, input datatype used for trained model.  
Here, the **model** directory contains information about class count, feature count, labels, pi, **theta** 
and thier datatype.  

__Return Value__  
It returns nothing

### 11. release()  

__Purpose__    
It can be used to release the in-memory model at frovedis server.  

For example,
 
    bnb.release()

This will reset the after-fit populated attributes to None, along with releasing server 
side memory.  

__Return Value__  
It returns nothing.  

### 12. is_fitted()  

__Purpose__    
It can be used to confirm if the model is already fitted or not. In case, predict() is used 
before training the model, then it can prompt the user to train the naive bayes model first.  

__Return Value__  
It returns ‘True’, if the model is already fitted otherwise, it returns ‘False’.

# SEE ALSO  
rowmajor_matrix, dvector, crs_matrix   