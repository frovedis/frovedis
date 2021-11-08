% Multinomial Naive Bayes

# NAME

Multinomial Naive Bayes- One of the variations of Naive Bayes algorithm. It is a 
multinomial classification algorithm.  

# SYNOPSIS

class frovedis.mllib.naive_bayes.MultinomialNB(alpha=1.0, fit_prior=True, class_prior=None, verbose=0)  


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
Naive Bayes classifier for multinomial models. The multinomial Naive Bayes
classifier is suitable for classification with discrete features (e.g., word counts for
text classification).  

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis interface is almost same as Scikit-learn 
MultinomialNB interface, but it doesn't have any dependency with Scikit-learn. 
It can be used simply even if the system doesn't have Scikit-learn installed. Thus, in 
this implementation, a python client can interact with a frovedis server sending the 
required python data for training at frovedis side. Python data is converted into 
frovedis compatible data internally and the python ML call is linked with the respective 
frovedis ML call to get the job done at frovedis server.  

Python side calls for MultinomialNB on the frovedis server. Once the training is 
completed with the input data at the frovedis server, it returns an abstract model with 
a unique model ID to the client python program.  

When prediction-like request would be made on the trained model, python program will 
send the same request to the frovedis server. After the request is served at the frovedis 
server, the output would be sent back to the python client.  

## Detailed Description  

### 1. MultinomialNB()  

__Parameters__  
**_alpha_**: A positive double(float64) smoothing parameter. It must be greater than or
equal to 1. (Default: 1.0)  
**_fit\_prior_**: A boolean parameter specifying whether to learn class prior probabilities 
or not. If False, a uniform prior will be used. (Default: True)  
**_class\_prior_**: A numpy ndarray of double(float64) type values and must be of shape 
**(n_classes,)**. It gives prior probabilities of the classes. (Default: None)  
When it is None (not specified explicitly), the priors are adjusted according to the data.  
**_verbose_**: An integer parameter specifying the log level to use. Its value is 0 by 
default (for INFO mode and not specified explicitly). But it can be set to 1 (for DEBUG mode) 
or 2 (for TRACE mode) for getting training time logs from frovedis server.  

__Attributes__  
**_class\_log\_prior\__**: A python ndarray of double(float64) type values and has shape (n_classes,). It
contains log probability of each class (smoothed).  
**_feature\_log\_prob\__**: A python ndarray of double(float64) type values and has shape (n_classes,
n_features). It contains empirical log probability of features given a class.  
**_class\_count\__**: A python ndarray of double(float64) type values and has shape (n_classes,). It contains
the number of samples encountered for each class during fitting. This value is weighted by the sample weight
when provided.  
**_classes\__**: A python ndarray of double(float64) type values and has shape (n_classes,). It contains the of
unique labels given to the classifier during training.  
**_feature\_count\__**: A python ndarray of double(float64) type values and has shape (n_classes,
n_features). It contains the number of samples encountered for each (class, feature) during fitting. This
value is weighted by the sample weight when provided.  
**_coef\__**: A python ndarray of double(float64) type values. If 'classes_' are 2 then the shape (1, n_features), 
otherwise, the shape is (n_classes,n_features). It mirrors ‘feature_log_prob_’ for interpreting MultinomialNB as a 
linear model.  
 **_intercept\__**:  A python ndarray of double(float64) type values. If 'classes_' are 2 then the shape (1,), 
otherwise, the shape is (n_classes,). It mirrors ‘class_log_prior_’ for interpreting MultinomialNB as a linear model.  

__Purpose__  
It initializes a MultinomialNB object with the given parameters.  

__Return Value__  
It simply returns "self" reference. 

### 2. fit(X, y, sample_weight = None)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data.  
**_y_**: Any python array-like object or an instance of FrovedisDvector.  
**_sample\_weight_**: A python ndarray containing the intended weights for each input samples 
and it should be the shape of (n_samples, ). (Default: None)  
When it is None (not specified explicitly), an uniform weight vector is assigned on each input 
sample.  

__Purpose__  
It accepts the training matrix (X) with labels (y) and trains a MultinomialNB model.  

For example,   

    # loading sample matrix and labels data
    from sklearn.datasets import load_breast_cancer
    mat, lbl = load_breast_cancer(return_X_y = True)
    
    # fitting input matrix and label on MultinomialNB object
    from frovedis.mllib.naive_bayes import MultinomialNB
    mnb = MultinomialNB().fit(mat,lbl)  

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,   
   
    # loading sample matrix and labels data
    from sklearn.datasets import load_breast_cancer
    mat, lbl = load_breast_cancer(return_X_y = True)
    
    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix. 
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    from frovedis.matrix.dvector import FrovedisDvector
    rmat = FrovedisRowmajorMatrix(mat)
    dlbl = FrovedisDvector(lbl)
    
    # MultinomialNB with pre-constructed frovedlis-like inputs
    from frovedis.mllib.naive_bayes import MultinomialNB
    mnb = MultinomialNB().fit(rmat, dlbl)  

__Return Value__  
It simply returns "self" reference.  

### 3. predict(X)
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data.  

__Purpose__  
It accepts the test feature matrix (X) in order to make prediction on the
trained model at frovedis server.  

For example,  

    # predicting on MultinomialNB model
    mnb.predict(mat)
    
Output  

    [1 1 1 1 1]

Like in fit(), frovedis-like input can be used to speed-up the prediction making on the trained 
model at server side.  

For example,  

    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix.
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(mat)
    
    # predicting on MultinomialNB model using pre-constructed input
    mnb.predict(rmat)

Output  

    [1 1 1 1 1]
    
__Return Value__  
It returns a numpy array of long (int64) type and of shape **(n_samples,)** containing the predicted 
outputs.  

### 4. predict_proba(X)
  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data.  

__Purpose__  
Perform classification on an array and return probability estimates for the test vector X.  

It accepts the test feature matrix (X) in order to make prediction on the trained model at frovedis 
server. Unlike sklearn, it performs the classification on an array and returns the probability estimates 
for the test feature matrix (X).  

For example,  

    # finds the probablity sample for each class in the model
    mnb.predict_proba(mat)

Output

    [[1.00000000e+00 1.31173349e-73]
    [1.00000000e+00 3.02940294e-46]
    [1.00000000e+00 1.60233859e-37]
    ...
    [9.99537526e-01 4.62474254e-04]
    [1.00000000e+00 2.74000590e-29]
    [3.87284789e-18 1.00000000e+00]]

Like in fit(), frovedis-like input can be used to speed-up the prediction making on the trained model 
at server side.  

For example,  

    # Since "mat" is numpy dense data, we have created FrovedisRowmajorMatrix.
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    rmat = FrovedisRowmajorMatrix(mat)
    
    # finds the probablity sample for each class in the model
    mnb.predict_proba(rmat)
    
Output

    [[1.00000000e+00 1.31173349e-73]
    [1.00000000e+00 3.02940294e-46]
    [1.00000000e+00 1.60233859e-37]
    ...
    [9.99537526e-01 4.62474254e-04]
    [1.00000000e+00 2.74000590e-29]
    [3.87284789e-18 1.00000000e+00]]

__Return Value__  
It returns a numpy array of float or double(float64) type and of shape **(n_samples, n_classes)** containing
the predicted probability values.  


### 5. score(X, y, sample_weight = None)  
__Parameters__   
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data.  
**_y_**: Any python array-like object or an instance of FrovedisDvector.  
**_sample\_weight_**: A python ndarray containing the intended weights for each input samples 
and it should be the shape of (n_samples, ). (Default: None)  
When it is None (not specified explicitly), an uniform weight vector is assigned on each input 
sample.  

__Purpose__  
Calculate mean accuracy on the given test data and labels i.e. mean accuracy of self.predict(X) wrt. y.  

For example,  

    mnb.score(mat,lbl)

Output

    0.89 

__Return Value__  
It returns an accuracy score of double(float64) type.  

### 6. get_params(deep = True)  

__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If True, will return the 
parameters for an estimator and contained subobjects that are estimators. (Default: True)

__Purpose__    
This method belongs to the BaseEstimator class inherited by MultinomialNB. It is used to get parameters
and their values of MultinomialNB class.  

For example, 
 
      print(mnb.get_params())

Output  

     {'alpha': 1.0, 'class_prior': None, 'fit_prior': True, 'verbose': 0}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 7. set_params(**params)  

__Parameters__   

_**params**_: All the keyword arguments are passed this function as dictionary. This dictionary contains 
parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by MultinomialNB, used to set parameter values.  

For example,   

    print("get parameters before setting:", mnb.get_params())
    mnb.set_params(fit_prior = False)
    print("get parameters after setting:", mnb.get_params())


Output  
     
    get parameters before setting: {'alpha': 1.0, 'class_prior': None, 'fit_prior': True, 'verbose': 0}
    get parameters after setting: {'alpha': 1.0, 'class_prior': None, 'fit_prior': False, 'verbose': 0}

__Return Value__  
It simply returns "self" reference.  

### 8. load(fname, dtype = None)
__Parameters__   
**_fname_**:  A string object containing the name of the file having model information such as theta, 
cls_count, feature_count, label, pi, type to be loaded.  
**_dtype_**: A data-type is inferred from the input data. Currently, expected input data-type is either 
float or double(float64). (Default: None)  

__Purpose__  
It loads the model stored previously from the specified file (having little-endian binary data).  

For example,  

    mnb.load("./out/MNBModel", dtype = np.float64)

__Return Value__  
It simply returns "self" reference.   

### 9. save(fname)
__Parameters__   
**_fname_**: A string object containing the name of the file on which the target 
model is to be saved.  

__Purpose__  
On success, it writes the model information(theta, cls_count, feature_count, label, pi, type) in the 
specified file as little-endian binary data. Otherwise, it throws an exception.  

For example,   

    # To save the MultinomailNB model
    mnb.save("./out/MNBModel")

The MNBModel contains below directory structure:  
**MNBModel**  
    |------label_map  
    |------metadata  
    |------**model**    
\ \ \ \ \ \ \ \ |------cls_count  
\ \ \ \ \ \ \ \ |------feature_count  
\ \ \ \ \ \ \ \ |------label  
\ \ \ \ \ \ \ \ |------pi  
\ \ \ \ \ \ \ \ |------**theta**  
\ \ \ \ \ \ \ \ |------type  

'label_map' contains information about labels mapped with their encoded value.  
'metadata' represents the detail about model_kind and datatype of training vector.  
Here, the **model** directory contains information about cls_count, feature_count, label, pi, **theta** and type.  

This will save the naive bayes model on the path ‘/out/MNBModel’. It would raise exception if the directory already 
exists with same name.  

__Return Value__  
It returns nothing.   

### 10. debug_print()

__Purpose__   
It shows the target model information on the server side user terminal. 
It is mainly used for debugging purpose.  

For example,  

    mnb.debug_print() 
    
Output  

    model_type: multinomial
    binarize: 0
    feature_count: 3702.12 4580.24 24457.5 207416 21.8145  ... 65.2141 59.3469 26.5766 96.4778 28.3608
    theta: node = 0, local_num_row = 2, local_num_col = 30, val = -5.08709 -4.8743 -3.19929 -1.06154 
    -10.1766     
    ... 
    -8.83939 -8.93218 -9.71532 -8.45266 -9.65263
    pi: -0.987294 -0.466145
    label: 0 1
    class count: 212 357

It displays the target model information like model_type, binarize, feature_count, theta, pi, etc. values on
the trained model which is currently present on the server.  

__Return Value__  
It returns nothing.  

### 11. release()  

__Purpose__  
It can be used to release the in-memory model at frovedis server.  

For example,
 
    mnb.release()

This will reset the after-fit populated attributes to None, along with releasing server 
side memory.  

__Return Value__  
It returns nothing.   

### 12. is_fitted()  

__Purpose__  
It can be used to confirm if the model is already fitted or not.  In case, predict() is used before training the
model, then it can prompt the user to train the naive bayes model first.   

__Return Value__  
It returns ‘True’, if the model is already fitted otherwise, it returns ‘False’.  

# SEE ALSO  
rowmajor_matrix, crs_matrix, dvector
