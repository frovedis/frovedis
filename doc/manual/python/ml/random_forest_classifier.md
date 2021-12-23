% RandomForestClassifier  

# NAME  
RandomForestClassifier - A classification algorithm that contains multiple decision trees 
on various subsets of the given dataset. It contains the predictions of multiple decision trees 
and based on the majority votes of predictions, it predicts the final output.  

# SYNOPSIS  

    class frovedis.mllib.ensemble.forest.RandomForestClassifier(n_estimators=100, criterion='gini',  
                                                                max_depth=None,  
                                                                min_samples_split=2,  
                                                                min_samples_leaf=1,  
                                                                min_weight_fraction_leaf=0.0,  
                                                                max_features='auto',  
                                                                max_leaf_nodes=None,  
                                                                min_impurity_decrease=0.0,  
                                                                min_impurity_split=None,  
                                                                bootstrap=True, oob_score=False,  
                                                                n_jobs=None, random_state=None,  
                                                                verbose=0, warm_start=False,  
                                                                class_weight=None, ccp_alpha=0.0,  
                                                                max_samples=None, max_bins=32)  
 
## Public Member Functions  

fit(X, y)  
predict(X)  
get_params(deep = True)  
set_params(\*\*params)  
load(fname, dtype = None)  
score(X, y, sample_weight = None)  
save(fname)  
debug_print()  
release()  
is_fitted()  

# DESCRIPTION  
Random Forest Classifier is a supervised machine learning algorithm used for classification 
using decision trees. The classifier creates a set of decision trees from randomly selected 
subsets of the training set. It is basically a set of decision trees from a randomly 
selected subset of the training set and then it collects the votes from different decision 
trees to decide the final prediction. **Frovedis supports both binary and multinomial 
random forest classification algorithms.**  

During training, the input **X** is the training data and **y** is the corresponding 
label values (Frovedis supports any values for labels, but internally it encodes the 
input binary labels to 0 and 1, and input multinomial labels to 0, 1, 2, ..., N-1 
(where N is the no. of classes) before training at Frovedis server) which we want 
to predict.  

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis interface is almost same as Scikit-learn
RandomForestClassifier interface, but it doesn't have any dependency with Scikit-learn. 
It can be used simply even if the system doesn't have Scikit-learn installed.
Thus in this implementation, a python client can interact with a frovedis server
sending the required python data for training at frovedis side. Python data is
converted into frovedis compatible data internally and the python ML call is
linked with the respective frovedis ML call to get the job done at frovedis server. 

Python side calls for RandomForestClassifier on the frovedis server. Once the training
is completed with the input data at the frovedis server, it returns an abstract 
model with a unique model ID to the client python program.  

When prediction-like request would be made on the trained model, python program 
will send the same request to the frovedis server. After the request is served at 
the frovedis server, the output would be sent back to the python client.  

## Detailed Description  

### 1. RandomForestClassifier()  

__Parameters__  
**_n\_estimators_**: A positive integer parameter that specifies the number of trees 
in the forest. (Default: 100)  
**_criterion_**: A string object parameter that specifies the function to measure the 
quality of a split. Supported criteria are 'gini' and 'entropy'. (Default: 'gini')  
- **'gini' impurity**: calculates the amount of probability of a specific feature that is 
classified incorrectly when selected randomly.  
- **'entropy' (information gain)**: it is applied to quantify which feature provides maximal 
information about the classification based on the notion of entropy.  

**_max\_depth_**: A positive integer parameter that specifies the maximum depth of the 
tree. (Default: None)  
If it is None (not specified explicitly), then 'max_depth' is set to 4.  
**_min\_samples\_split_**: An integer or float value that specifies the minimum number 
of samples required to split an internal node. (Default: 2)  
**_min\_samples\_leaf_**: A positive integer or float value that specifies the minimum 
number of samples required to be at a leaf node. A split point at any depth will only be 
considered if it leaves at least 'min_samples_leaf' training samples in each of the left 
and right branches. (Default: 1)  
- If it is an integer, then 'min_samples_leaf' should be greater than 0.  
- If it is float, then it is set as **int(np.ceil(self.min_samples_split * self.n_samples_))**  

**_min\_weight\_fraction\_leaf_**: An unused parameter. (Default: 0.0)  
**_max\_features_**: A string object parameter that specifies the number of features to 
consider when looking for the best split:  
- If it is an integer, then it will be set as **(max_features * 1.0) / n_features_**.  
- If it is float, then it will be **'max_features'** number of features at each split.  
- If it is 'auto', then it will be set as **sqrt(n_features_)**.  
- If 'sqrt', then it will be set as **sqrt(n_features_)** (same as 'auto').  
- If 'log2', then it will be set as **log2(n_features_)**.  
- If None, then it will be set as **n_features_**. (Default: 'auto')  

**_max\_leaf\_nodes_**: An unused parameter. (Default: None)  
**_min\_impurity\_decrease_**: A positive double (float64) parameter. A node will be split 
if this split induces a decrease of the impurity greater than or equal to this value. (Default: 0.0)  
**_min\_impurity\_split_**: An unused parameter. (Default: None)  
**bootstrap**: An unused parameter. (Default: True)  
**oob\_score**: An unused parameter. (Default: False)  
**n\_jobs**: An unused parameter. (Default: None)  
**_random\_state_**: An integer parameter that controls the sampling of the features to consider 
when looking for the best split at each node (if max_features < n_features). (Default: None)  
If it is None (not specified explicitly), then 'random_state' is set as -1.  
**_verbose_**: An integer parameter specifying the log level to use. Its value is set as 0 
by default (for INFO mode). But it can be set to 1 (for DEBUG mode) or 2 (for TRACE mode) for 
getting training time logs from frovedis server.  
**_warm\_start_**: An unused parameter. (Default: False)  
**_class\_weight_**: An unused parameter. (Default: None)  
**_ccp\_alpha_**: An unused parameter. (Default: 0.0)  
**_max\_samples_**: An unused parameter. (Default: None)  
**_max\_bins_**: A positive integer parameter that specifies the maximum number of bins 
created by ordered splits. (Default: 32)  

__Attributes__  
**_classes\__**: It is a python ndarray (any type) of unique labels given to the classifier 
during training. It has shape **(n_classes,)**.  
**_n\_features\__**: An integer value specifying the number of features when fitting the estimator.  

__Purpose__   
It initializes a RandomForestClassifier object with the given parameters.  

The parameters: "min_weight_fraction_leaf", "max_leaf_nodes", "min_impurity_split", "bootstrap", 
"oob_score", "n_jobs", "warm_start", "class_weight", "ccp_alpha" and "max_samples" are simply kept 
in to make the interface uniform to the Scikit-learn RandomForestClassifier module. They are not 
used anywhere within frovedis implementation.  

__Return Value__  
It simply returns "self" reference.  

### 2. fit(X, y)  
__Parameters__  
**_X_**: A numpy dense or any python array-like object or an instance of FrovedisColmajorMatrix 
for dense data. It has shape **(n_samples, n_features)**. Currently, it supports only dense data as input.  
**_y_**: Any python array-like object or an instance of FrovedisDvector containing the target values. 
It has shape **(n_samples,)**.  

__Purpose__  
It builds a forest of trees from the training data X and labels y.  

For example,   

    # loading a sample matrix and labels dense data
    import numpy as np
    mat = np.array([[10, 0, 1, 0, 0, 1, 0],
                    [0, 1, 0, 1, 0, 1, 0],
                    [0, 1, 0, 0, 1, 0, 1],
                    [1, 0, 0, 1, 0, 1, 0]], dtype = np.float64)
    lbl = np.array([100, 500, 100, 600], dtype = np.float64) 
    
    # fitting input matrix and label on RandomForestClassifier object
    from frovedis.mllib.ensemble import RandomForestClassifier
    rfc = RandomForestClassifier(n_estimators = 10, max_depth = 4, min_samples_split = 0.5
                                 min_samples_leaf = 1.2, random_state = 324)  
    rfc.fit(mat,lbl)

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,  
    
    # loading a sample matrix and labels dense data
    import numpy as np
    mat = np.array([[10, 0, 1, 0, 0, 1, 0],
                    [0, 1, 0, 1, 0, 1, 0],
                    [0, 1, 0, 0, 1, 0, 1],
                    [1, 0, 0, 1, 0, 1, 0]], dtype = np.float64)
    lbl = np.array([100, 500, 100, 600], dtype = np.float64) 
    
    # Since "mat" is numpy dense data, we have created FrovedisColmajorMatrix. 
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisColmajorMatrix
    from frovedis.matrix.dvector import FrovedisDvector 
    cmat = FrovedisColmajorMatrix(mat)
    dlbl = FrovedisDvector(lbl)
    
    # fitting input matrix and label on RandomForestClassifier object
    from frovedis.mllib.ensemble import RandomForestClassifier
    rfc = RandomForestClassifier(n_estimators = 10, max_depth = 4,
                                 min_samples_split = 0.5, min_samples_leaf = 1.2, 
                                 random_state = 324)  
    rfc.fit(cmat,dlbl)


__Return Value__  
It simply returns "self" reference.  

### 3. predict(X)  
__Parameters__  
**_X_**: A numpy dense or any python array-like object or an instance of FrovedisRowmajorMatrix 
for dense data. It has shape **(n_samples, n_features)**. Currently, it supports only dense data as input.  

__Purpose__  
Predict class for X.  

The predicted class of an input sample is a vote by the trees in the forest, weighted 
by their probability estimates. That is, the predicted class is the one with highest 
mean probability estimate across the trees.  

For example,   

    # predicting on random forest classifier model
    rfc.predict(mat)  

Output  

    [100. 500. 100. 500.]

Like in fit(), frovedis-like input can be used to speed-up the prediction making on 
the trained model at server side.  

For example,  

    # Since "cmat" is FrovedisColmajorMatrix, we have created FrovedisRowmajorMatrix.
    # predicting on random forest classifier model
    rfc.predict(cmat.to_frovedis_rowmatrix())  

Output  

    [100. 500. 100. 500.]

__Return Value__  
It returns a numpy array of float or double (float64) type and of shape **(n_samples,)** 
containing the predicted classes.  

### 4. get_params(deep = True)  
__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If True, 
it will return the parameters for an estimator and contained subobjects that are estimators. (Default: True)  

__Purpose__    
This method belongs to the BaseEstimator class inherited by RandomForestClassifier. It is used 
to get parameters and their values of RandomForestClassifier class.  

For example, 
 
    print(rfc.get_params())

Output  

    {'bootstrap': True, 'ccp_alpha': 0.0, 'class_weight': None, 'criterion': 'gini', 
    'max_bins': 32, 'max_depth': 4, 'max_features': 'auto', 'max_leaf_nodes': None, 
    'max_samples': None, 'min_impurity_decrease': 0.0, 'min_impurity_split': None, 
    'min_samples_leaf': 2, 'min_samples_split': 0.5, 'min_weight_fraction_leaf': 0.0, 
    'n_estimators': 10, 'n_jobs': None, 'oob_score': False, 'random_state': 324, 
    'verbose': 0, 'warm_start': False}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 5. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed to this function as dictionary. This dictionary 
contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by RandomForestClassifier, used to set 
parameter values.  

For example,   

    print("get parameters before setting:") 
    print(rfc.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    rfc.set_params(criterion = 'entropy', max_depth = 5) 
    print("get parameters after setting:") 
    print(rfc.get_params())

Output  
     
    get parameters before setting:  
    {'bootstrap': True, 'ccp_alpha': 0.0, 'class_weight': None, 'criterion': 'gini', 
    'max_bins': 32, 'max_depth': 4, 'max_features': 'auto', 'max_leaf_nodes': None, 
    'max_samples': None, 'min_impurity_decrease': 0.0, 'min_impurity_split': None, 
    'min_samples_leaf': 2, 'min_samples_split': 0.5, 'min_weight_fraction_leaf': 0.0, 
    'n_estimators': 10, 'n_jobs': None, 'oob_score': False, 'random_state': 324, 
    'verbose': 0, 'warm_start': False}  
    get parameters after setting:    
    {'bootstrap': True, 'ccp_alpha': 0.0, 'class_weight': None, 'criterion': 'entropy', 
    'max_bins': 32, 'max_depth': 5, 'max_features': 'auto', 'max_leaf_nodes': None, 
    'max_samples': None, 'min_impurity_decrease': 0.0, 'min_impurity_split': None, 
    'min_samples_leaf': 2, 'min_samples_split': 0.5, 'min_weight_fraction_leaf': 0.0, 
    'n_estimators': 10, 'n_jobs': None, 'oob_score': False, 'random_state': 324, 
    'verbose': 0, 'warm_start': False}  
    
__Return Value__  
It simply returns "self" reference.  

### 6. load(fname, dtype = None)  
__Parameters__  
**_fname_**:  A string object containing the name of the file having model information
to be loaded.  
**_dtype_**: A data-type is inferred from the input data. Currently, expected input 
data-type is either float or double (float64). (Default: None)  

__Purpose__  
It loads the model from the specified file (having little-endian binary data).  

For example,  

    rfc.load("./out/rf_classifier_model")  

__Return Value__  
It simply returns "self" reference.  

### 7. save(fname)  
__Parameters__  
**_fname_**: A string object containing the name of the file on which the target model 
is to be saved.  

__Purpose__  
On success, it writes the model information (label_map, metadata and model) in the 
specified file as little-endian binary data. Otherwise, it throws an exception.  

For example,   

    # To save the random forest classifier model
    dtc.save("./out/rf_classifier_model")  

This will save the random forest classifier model on the path "/out/rf_classifier_model". It would 
raise exception if the directory already exists with same name.  

The 'rf_classifier_model' directory has  

**rf_classifier_model**  
|-----label_map  
|-----metadata  
|-----model  

‘label_map’ file contains information about labels mapped with their encoded value.  
The 'metadata' file contains the number of classes, model kind and input datatype used for 
trained model.  
The 'model' file contains the random forest model saved in binary format.  

__Return Value__  
It returns nothing.  

### 8. score(X, y, sample_weight = None)  
__Parameters__  
**_X_**: A numpy dense or any python array-like object or an instance of FrovedisRowmajorMatrix 
for dense data. It has shape **(n_samples, n_features)**. Currently, it supports only dense data as input.  
**_y_**: Any python array-like object containing the true labels for X. It has 
shape **(n_samples,)**.  
**_sample\_weight_**: A python ndarray containing the intended weights for each input 
samples and it should be the shape of **(n_samples,)**.  
When it is None (not specified explicitly), an uniform weight vector is assigned on 
each input sample. (Default: None)  

__Purpose__  
Calculate mean accuracy on the given test data and labels i.e. mean accuracy of 
self.predict(X) wrt. y.  

For example,   

    # calculate mean accuracy score on given test data and labels
    rfc.score(mat,lbl)  

Output  

    0.75

__Return Value__  
It returns an accuracy score of float type.   

### 9. debug_print()  

__Purpose__    
It shows the target model information on the server side user terminal. It is mainly 
used for debugging purpose.  

For example,

    rfc.debug_print()
    
Output

    -------- Classification Trees (Random Forest):: --------
    # of trees: 10
    ---- [0] ----
      # of nodes: 3, height: 1
      <1> Split: feature[2] < 0.25, IG: 0.5
       \_ (2) Predict: 2 (100%)
       \_ (3) Predict: 0 (100%)
    ---- [1] ----
      # of nodes: 3, height: 1
      <1> Split: feature[6] < 0.25, IG: 0.375
       \_ (2) Predict: 1 (50%)
       \_ (3) Predict: 0 (100%)
    ---- [2] ----
      # of nodes: 1, height: 0
      (1) Predict: 0 (50%)
    ---- [3] ----
      # of nodes: 3, height: 1
      <1> Split: feature[6] < 0.25, IG: 0.125
       \_ (2) Predict: 0 (50%)
       \_ (3) Predict: 0 (100%)
    ---- [4] ----
      # of nodes: 3, height: 1
      <1> Split: feature[3] < 0.25, IG: 0.375
       \_ (2) Predict: 0 (100%)
       \_ (3) Predict: 1 (50%)
    ---- [5] ----
      # of nodes: 3, height: 1
      <1> Split: feature[1] < 0.25, IG: 0.5
       \_ (2) Predict: 2 (100%)
       \_ (3) Predict: 1 (100%)
    ---- [6] ----
      # of nodes: 1, height: 0
      (1) Predict: 0 (100%)
    ---- [7] ----
      # of nodes: 1, height: 0
      (1) Predict: 2 (75%)
    ---- [8] ----
      # of nodes: 1, height: 0
      (1) Predict: 1 (50%)
    ---- [9] ----
      # of nodes: 3, height: 1
      <1> Split: feature[5] < 0.25, IG: 0.375
       \_ (2) Predict: 0 (100%)
       \_ (3) Predict: 1 (50%)

It displays the random forest having maximum depth of 4 and total 10 decision trees.  

__Return Value__  
It returns nothing.  

### 10. release()  

__Purpose__  
It can be used to release the in-memory model at frovedis server.  

For example,

    rfc.release()

This will reset the after-fit populated attributes (like classes_, n_features_) to None, 
along with releasing server side memory.  

__Return Value__  
It returns nothing.  

### 11. is_fitted()  

__Purpose__  
It can be used to confirm if the model is already fitted or not.  

__Return Value__  
It returns 'True', if the model is already fitted, otherwise, it returns 'False'.  

# SEE ALSO
dvector, rowmajor_matrix, colmajor_matrix, random_forest_regressor, decision_tree_classifier