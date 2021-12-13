% RandomForestRegressor  

# NAME  
RandomForestRegressor - A regression algorithm used for predicting the final output by taking the 
mean of all the predictions from multiple decision trees.  

# SYNOPSIS  

    class frovedis.mllib.ensemble.forest.RandomForestRegressor(n_estimators=100, criterion='mse',  
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
                                                               ccp_alpha=0.0, max_samples=None,  
                                                               max_bins=32)  
 
## Public Member Functions  

fit(X, y)  
predict(X)  
get_params(deep = True)  
set_params(\*\*params)  
load(fname, dtype = None)  
save(fname)  
score(X, y, sample_weight = None)  
debug_print()  
release()  
is_fitted()  

# DESCRIPTION  

Random Forest Regressor is a supervised machine learning algorithm used for regression 
using decision trees. Every decision tree has high variance, but when we combine all of 
them together in parallel then the resultant variance is low as each decision tree gets 
perfectly trained on that particular sample data and hence the output doesn’t depend on 
one decision tree but on multiple decision trees. The regressor creates a set of decision 
trees from randomly selected subsets of the training set. Then, it takes the average to 
improve the predictive accuracy of that dataset. So, instead of relying on one decision 
tree, it combines multiple decision trees in determining the final output.  

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis interface is almost same as Scikit-learn
RandomForestRegressor interface, but it doesn't have any dependency with Scikit-learn. 
It can be used simply even if the system doesn't have Scikit-learn installed.
Thus in this implementation, a python client can interact with a frovedis server
sending the required python data for training at frovedis side. Python data is
converted into frovedis compatible data internally and the python ML call is
linked with the respective frovedis ML call to get the job done at frovedis server. 

Python side calls for RandomForestRegressor on the frovedis server. Once the training
is completed with the input data at the frovedis server, it returns an abstract 
model with a unique model ID to the client python program.  

When prediction-like request would be made on the trained model, python program 
will send the same request to the frovedis server. After the request is served at 
the frovedis server, the output would be sent back to the python client.  

## Detailed Description  

### 1. RandomForestRegressor()  

__Parameters__  
**_n\_estimators_**: A positive integer parameter that specifies the number of trees 
in the forest. (Default: 100)  
**_criterion_**: A string object parameter that specifies the function to measure the 
quality of a split. (Default: 'mse')  
Currently, supported criteria are 'mse' and 'mae'. The mean squared error (mse), which 
is equal to variance reduction as feature selection criterion. The mean absolute 
error (mae) uses reduction in Poisson deviance to find splits. Training using 'mae' is 
significantly slower than when using 'mse'.  
**_max\_depth_**: A positive integer parameter that specifies the maximum depth of the 
tree. (Default: None)  
If it is None (not specified explicitly), then 'max_depth' is set to 4.  
**_min\_samples\_split_**: An integer or float value that specifies the minimum number 
of samples required to split an internal node. (Default: 2)  
**_min\_samples\_leaf_**: A positive integer or float value that specifies the minimum 
number of samples required to be at a leaf node. A split point at any depth will only be 
considered if it leaves at least 'min_samples_leaf' training samples in each of the left 
and right branches. (Default: 1)  
If it is an integer, then 'min_samples_leaf' should be greater than 0.  
If it is float, then it is set as **int(np.ceil(self.min_samples_split * self.n_samples_))**  
**_min\_weight\_fraction\_leaf_**: An unused parameter. (Default: 0.0)  
**_max\_features_**: A string object parameter that specifies the number of features to 
consider when looking for the best split:  
If it is an integer, then it will be set as **(max_features * 1.0) / n_features**.  
If it is float, then it will be **'max_features'** number of features at each split.  
If it is 'auto', then it will be set as **sqrt(n_features)**.  
If 'sqrt', then it will be set as **sqrt(n_features)** (same as 'auto').  
If 'log2', then it will be set as **log2(n_features)**.  
If None, then it will be set as **n_features**. (Default: 'auto')  
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
**_ccp\_alpha_**: An unused parameter. (Default: 0.0)  
**_max\_samples_**: An unused parameter. (Default: None)  
**_max\_bins_**: A positive integer parameter that specifies the maximum number of bins 
created by ordered splits. (Default: 32)  

__Attributes__  
**_n\_features\__**: An integer value specifying the number of features when fitting the estimator.  

__Purpose__   
It initializes a RandomForestRegressor object with the given parameters.  

The parameters: "min_weight_fraction_leaf", "max_leaf_nodes", "min_impurity_split", "bootstrap", 
"oob_score", "n_jobs", "warm_start", "ccp_alpha" and "max_samples" are simply kept in to make the 
interface uniform to the Scikit-learn RandomForestRegressor module. They are not used anywhere within 
frovedis implementation.  

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
    lbl = np.array([1.2,0.3,1.1,1.9])  
    
    # fitting input matrix and label on RandomForestRegressor object
    from frovedis.mllib.ensemble import RandomForestRegressor
    rfr = RandomForestRegressor(n_estimators = 10, criterion = 'mae', max_depth = 5)  
    rfr.fit(mat,lbl)

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
    lbl = np.array([1.2,0.3,1.1,1.9])  
    
    # Since "mat" is numpy dense data, we have created FrovedisColmajorMatrix. 
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisColmajorMatrix
    from frovedis.matrix.dvector import FrovedisDvector 
    cmat = FrovedisColmajorMatrix(mat)
    dlbl = FrovedisDvector(lbl)
    
    # fitting input matrix and label on RandomForestRegressor object
    from frovedis.mllib.ensemble import RandomForestRegressor
    rfr = RandomForestRegressor(n_estimators = 10, criterion = 'mae', max_depth = 5)  
    rfr.fit(cmat,dlbl)


__Return Value__  
It simply returns "self" reference.  

### 3. predict(X)  
__Parameters__  
**_X_**: A numpy dense or any python array-like object or an instance of FrovedisRowmajorMatrix 
for dense data. It has shape **(n_samples, n_features)**. Currently, it supports only dense data as input.  

__Purpose__  
Predict regression value for X.  

The predicted regression target of an input sample is computed as the mean predicted regression targets of the trees in the forest.  

For example,   

    # predicting on random forest regressor model
    rfr.predict(mat)  

Output  

    [1.29083333 0.8075     1.0475     1.48083333]  

Like in fit(), frovedis-like input can be used to speed-up the prediction making on 
the trained model at server side.  

For example,  

    # Since "cmat" is FrovedisColmajorMatrix, we have created FrovedisRowmajorMatrix.
    # predicting on random forest regressor model using pre-constructed input
    rfr.predict(cmat.to_frovedis_rowmatrix())

Output  

    [1.29083333 0.8075     1.0475     1.48083333] 

__Return Value__  
It returns a numpy array of float or double (float64) type and of shape **(n_samples,)** 
containing the predicted values.  

### 4. get_params(deep = True)  
__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If True, 
it will return the parameters for an estimator and contained subobjects that are estimators. (Default: True)  

__Purpose__    
This method belongs to the BaseEstimator class inherited by RandomForestRegressor. It is used 
to get parameters and their values of RandomForestRegressor class.  

For example, 
 
    print(rfr.get_params())

Output  

    {'bootstrap': True, 'ccp_alpha': 0.0, 'criterion': 'mae', 'max_bins': 32, 
    'max_depth': 5, 'max_features': 'auto', 'max_leaf_nodes': None, 
    'max_samples': None, 'min_impurity_decrease': 0.0, 'min_impurity_split': None, 
    'min_samples_leaf': 1, 'min_samples_split': 2, 'min_weight_fraction_leaf': 0.0, 
    'n_estimators': 10, 'n_jobs': None, 'oob_score': False, 'random_state': -1, 
    'verbose': 0, 'warm_start': False}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 5. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed to this function as dictionary. This dictionary 
contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by RandomForestRegressor, used to set 
parameter values.  

For example,   

    print("get parameters before setting:") 
    print(rfr.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    rfr.set_params(max_depth = 4) 
    print("get parameters after setting:") 
    print(rfr.get_params())

Output  
     
    get parameters before setting:
    {'bootstrap': True, 'ccp_alpha': 0.0, 'criterion': 'mae', 'max_bins': 32, 
    'max_depth': 5, 'max_features': 'auto', 'max_leaf_nodes': None, 
    'max_samples': None, 'min_impurity_decrease': 0.0, 'min_impurity_split': None, 
    'min_samples_leaf': 1, 'min_samples_split': 2, 'min_weight_fraction_leaf': 0.0, 
    'n_estimators': 10, 'n_jobs': None, 'oob_score': False, 'random_state': -1, 
    'verbose': 0, 'warm_start': False}
    get parameters after setting:
    {'bootstrap': True, 'ccp_alpha': 0.0, 'criterion': 'mae', 'max_bins': 32, 
    'max_depth': 4, 'max_features': 'auto', 'max_leaf_nodes': None, 
    'max_samples': None, 'min_impurity_decrease': 0.0, 'min_impurity_split': None, 
    'min_samples_leaf': 1, 'min_samples_split': 2, 'min_weight_fraction_leaf': 0.0, 
    'n_estimators': 10, 'n_jobs': None, 'oob_score': False, 'random_state': -1, 
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

    rfc.load("./out/rf_regressor_model")  

__Return Value__  
It simply returns "self" reference.  

### 7. save(fname)  
__Parameters__  
**_fname_**: A string object containing the name of the file on which the target model 
is to be saved.  

__Purpose__  
On success, it writes the model information (after-fit populated attributes) in the 
specified file as little-endian binary data. Otherwise, it throws an exception.  

For example,   

    # To save the random forest regressor model
    dtc.save("./out/rf_regressor_model")  

This will save the random forest regressor model on the path "/out/rf_regressor_model". It would 
raise exception if the directory already exists with same name.  

The 'rf_regressor_model' directory has  

**rf_regressor_model**   
|-----metadata  
|-----model  

The 'metadata' file contains the model kind and input datatype used for trained model.  
The 'model' file contains the random forest model saved in binary format.  

__Return Value__  
It returns nothing.  

### 8. score(X, y, sample_weight = None)  
__Parameters__  
**_X_**: A numpy dense or any python array-like object or an instance of FrovedisRowmajorMatrix 
for dense data. It has shape **(n_samples, n_features)**. Currently, it supports only dense data as input.  
**_y_**: Any python array-like object containing the true values for X. It has 
shape **(n_samples,)**.  
**_sample\_weight_**: A python ndarray containing the intended weights for each input 
samples and it should be the shape of **(n_samples, )**. When it is None (not specified 
explicitly), an uniform weight vector is assigned on each input sample. (Default: None)  

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
    rfr.score(mat,lbl)  

Output  

    0.65

__Return Value__  
It returns an R2 score of float type.   

### 9. debug_print()  

__Purpose__    
It shows the target model information on the server side user terminal. It is mainly 
used for debugging purpose.  

For example,

    rfr.debug_print()
    
Output

    -------- Regression Trees (Random Forest):: --------
    # of trees: 10
    ---- [0] ----
      # of nodes: 3, height: 1
      <1> Split: feature[1] < 0.25, IG: 0.191667
       \_ (2) Predict: 1.43333
       \_ (3) Predict: 0.3
    ---- [1] ----
      # of nodes: 5, height: 2
      <1> Split: feature[2] < 0.25, IG: 0.0416667
       \_ <2> Split: feature[1] < 0.5, IG: 0.711111
       |  \_ (4) Predict: 1.9
       |  \_ (5) Predict: 0.3
       \_ (3) Predict: 1.2
    ---- [2] ----
      # of nodes: 1, height: 0
      (1) Predict: 1.375
    ---- [3] ----
      # of nodes: 5, height: 2
      <1> Split: feature[1] < 0.25, IG: 0.4
       \_ (2) Predict: 1.9
       \_ <3> Split: feature[3] < 0.5, IG: 0.4
          \_ (6) Predict: 1.1
          \_ (7) Predict: 0.3
    ---- [4] ----
      # of nodes: 3, height: 1
      <1> Split: feature[1] < 0.25, IG: 0.4
       \_ (2) Predict: 1.9
       \_ (3) Predict: 1.1
    ---- [5] ----
      # of nodes: 3, height: 1
      <1> Split: feature[4] < 0.25, IG: 0.4
       \_ (2) Predict: 0.3
       \_ (3) Predict: 1.1
    ---- [6] ----
      # of nodes: 5, height: 2
      <1> Split: feature[5] < 0.25, IG: 0.141667
       \_ (2) Predict: 1.1
       \_ <3> Split: feature[2] < 0.5, IG: 0.311111
          \_ (6) Predict: 1.9
          \_ (7) Predict: 1.2
    ---- [7] ----
      # of nodes: 7, height: 3
      <1> Split: feature[0] < 2.5, IG: 0.025
       \_ <2> Split: feature[6] < 0.5, IG: 1.11022e-16
       |  \_ <4> Split: feature[1] < 0.5, IG: 0.8
       |  |  \_ (8) Predict: 1.9
       |  |  \_ (9) Predict: 0.3
       |  \_ (5) Predict: 1.1
       \_ (3) Predict: 1.2
    ---- [8] ----
      # of nodes: 3, height: 1
      <1> Split: feature[2] < 0.25, IG: 0.2625
       \_ (2) Predict: 1.9
       \_ (3) Predict: 1.2
    ---- [9] ----
      # of nodes: 5, height: 2
      <1> Split: feature[3] < 0.25, IG: 0.279167
       \_ <2> Split: feature[2] < 0.5, IG: 0.0444444
       |  \_ (4) Predict: 1.1
       |  \_ (5) Predict: 1.2
       \_ (3) Predict: 0.3

It displays the random forest having maximum depth of 5 and total 10 decision trees.  

__Return Value__  
It returns nothing.  

### 10. release()  

__Purpose__  
It can be used to release the in-memory model at frovedis server.  

For example,

    rfr.release()

This will reset the after-fit populated attributes (like n_features_) to None, along 
with releasing server side memory.  

__Return Value__  
It returns nothing.  

### 11. is_fitted()  

__Purpose__  
It can be used to confirm if the model is already fitted or not.  

__Return Value__  
It returns ‘True’, if the model is already fitted, otherwise, it returns ‘False’.  

# SEE ALSO
dvector, rowmajor_matrix, colmajor_matrix, random_forest_classifer, decision_tree_regressor