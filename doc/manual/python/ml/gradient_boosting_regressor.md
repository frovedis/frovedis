% GradientBoostingRegressor  

# NAME  
GradientBoostingRegressor - It is a machine learning algorithm, used for regression. 
It works on the principle that many weak learners (multiple decision trees) can together 
make a more accurate predictor.  

# SYNOPSIS  

    class frovedis.mllib.ensemble.gbtree.GradientBoostingRegressor(loss="ls",  
                                                                   learning_rate=0.1,  
                                                                   n_estimators=100,  
                                                                   subsample=1.0,  
                                                                   criterion="friedman_mse",  
                                                                   min_samples_split=2,  
                                                                   min_samples_leaf=1,  
                                                                   min_weight_fraction_leaf=0.,  
                                                                   max_depth=3,  
                                                                   min_impurity_decrease=0.,  
                                                                   min_impurity_split=None,  
                                                                   init=None, random_state=None,  
                                                                   max_features=None, alpha=0.9,  
                                                                   verbose=0,  
                                                                   max_leaf_nodes=None,  
                                                                   warm_start=False,  
                                                                   presort="deprecated",  
                                                                   validation_fraction=0.1,  
                                                                   n_iter_no_change=None,  
                                                                   tol=1e-4, ccp_alpha=0.0,  
                                                                   max_bins=32)  
 
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
GradientBoostingRegressor is a supervised machine learning algorithm used for regression 
using decision trees. In gradient boosting decision trees, we combine many weak learners 
to come up with one strong learner. The weak learners here are the individual decision 
trees. All the trees are connected in series and each tree tries to minimise the error of the 
previous tree. Due to this sequential connection, boosting algorithms are usually slow to 
learn, but also highly accurate.  

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis interface is almost same as Scikit-learn
GradientBoostingRegressor interface, but it doesn't have any dependency with Scikit-learn. 
It can be used simply even if the system doesn't have Scikit-learn installed.
Thus in this implementation, a python client can interact with a frovedis server
sending the required python data for training at frovedis side. Python data is
converted into frovedis compatible data internally and the python ML call is
linked with the respective frovedis ML call to get the job done at frovedis server. 

Python side calls for GradientBoostingRegressor on the frovedis server. Once the training
is completed with the input data at the frovedis server, it returns an abstract 
model with a unique model ID to the client python program.  

When prediction-like request would be made on the trained model, python program 
will send the same request to the frovedis server. After the request is served at 
the frovedis server, the output would be sent back to the python client.  

## Detailed Description  

### 1. GradientBoostingRegressor()  

__Parameters__  
**_loss_**: A string object parameter that specifies the loss function to be optimized. Currently, 
it supports ‘ls’ and 'lad' loss. (Default: 'ls')  
- **'ls'**: refers to least squares regression.  
- **'lad' (least absolute deviation)**: it is a highly robust loss function solely based on order 
information of the input variables.  

**_learning\_rate_**: A positive double (float64) parameter that shrinks the contribution of each 
tree by 'learning_rate' value provided. (Default: 0.1)  
**_n\_estimators_**: A positive integer parameter that specifies the number of boosting 
stages to perform. (Default: 100)  
**_subsample_**: A positive double (float64) parameter that specifies the fraction of samples 
to be used for fitting the individual base learners. It must be in range **(0, 1.0]**. (Default: 1.0)  
**_criterion_**: A string object parameter that specifies the function to measure the 
quality of a split. (Default: 'friedman_mse')  
Currently, supported criteria are 'friedman_mse', 'mse' and 'mae'.  
- **'friedman_mse'**: the mean squared error with improvement score by Friedman.  
- **'mse'**: the mean squared error, which is equal to variance reduction as feature selection criterion.  
- **'mae'**: the mean absolute error uses reduction in Poisson deviance to find splits.  

**_min\_samples\_split_**: An unused parameter. (Default: 2)  
**_min\_samples\_leaf_**: An unused parameter. (Default: 1)  
**_min\_weight\_fraction\_leaf_**: An unused parameter. (Default: 0.0)  
**_max\_depth_**: A positive integer parameter that specifies the maximum depth of the 
individual regression estimators. It limits the number of nodes in the tree. (Default: 3)  
**_min\_impurity\_decrease_**: A positive double (float64) parameter. A node will be split 
if this split induces a decrease of the impurity greater than or equal to this value. (Default: 0.0)  
**_min\_impurity\_split_**: An unused parameter. (Default: None)  
**_init_**: An unused parameter. (Default: None)  
**_random\_state_**: An integer parameter that controls the random seed given to each 
tree estimator at each boosting iteration. In addition, it controls the random permutation 
of the features at each split. (Default: None)  
If it is None (not specified explicitly), then 'random_state' is set as -1.  
**_max\_features_**: A string object parameter that specifies the number of features to 
consider when looking for the best split:  
- If it is an integer, then it will be set as **(max_features * 1.0) / n_features_**.  
- If it is float, then it will be **'max_features'** number of features at each split.  
- If it is 'auto', then it will be set as **sqrt(n_features_)**.  
- If 'sqrt', then it will be set as **sqrt(n_features_)** (same as 'auto').  
- If 'log2', then it will be set as **log2(n_features_)**.  
- If None, then it will be set as **n_features_**. (Default: 'None')  

**_alpha_**: An unused parameter. (Default: 0.9)  
**_verbose_**: An integer parameter specifying the log level to use. Its value is set as 0 
by default (for INFO mode). But it can be set to 1 (for DEBUG mode) or 2 (for TRACE mode) for 
getting training time logs from frovedis server.  
**_max\_leaf\_nodes_**: An unused parameter. (Default: None)  
**_warm\_start_**: An unused parameter. (Default: False)  
**_presort_**: An unused parameter. (Default: 'deprecated')  
**_validation\_fraction_**: An unused parameter. (Default: 0.1)  
**_n\_iter\_no\_change_**: An unused parameter. (Default: None)  
**_tol_**: A double (float64) parameter that specifies the tolerance value for the early 
stopping. (Default: 1e-4)  
**_ccp\_alpha_**: An unused parameter. (Default: 0.0)  
**_max\_bins_**: A positive integer parameter that specifies the maximum number of bins 
created by ordered splits. (Default: 32)  

__Attributes__  
**_n\_estimators\__**: An integer value specifying the 'n_estimators' value.  
**_n\_features\__**: An integer value specifying the number of features when fitting the estimator.  

__Purpose__   
It initializes a GradientBoostingRegressor object with the given parameters.  

The parameters: "min_samples_split", "min_samples_leaf", "min_weight_fraction_leaf", 
"min_impurity_split", "init", "alpha", "max_leaf_nodes", "warm_start", "presort", "validation_fraction", 
"n_iter_no_change" and "ccp_alpha" are simply kept in to make the interface uniform to the 
Scikit-learn GradientBoostingRegressor module. They are not used anywhere within frovedis 
implementation.  

__Return Value__  
It simply returns "self" reference.  

### 2. fit(X, y)  
__Parameters__  
**_X_**: A numpy dense or any python array-like object or an instance of FrovedisColmajorMatrix 
for dense data. It has shape **(n_samples, n_features)**. Currently, it supports only dense data as input.  
**_y_**: Any python array-like object or an instance of FrovedisDvector containing the target values. 
It has shape **(n_samples,)**.  

__Purpose__  
It fits the gradient boosting model.  

For example,   

    # loading a sample matrix and labels dense data
    import numpy as np
    mat = np.array([[10, 0, 1, 0, 0, 1, 0],
                    [0, 1, 0, 1, 0, 1, 0],
                    [0, 1, 0, 0, 1, 0, 1],
                    [1, 0, 0, 1, 0, 1, 0]])
    lbl = np.array([1.2,0.3,1.1,1.9])  
    
    # fitting input matrix and label on GradientBoostingRegressor object
    from frovedis.mllib.ensemble import GradientBoostingRegressor
    gbr = GradientBoostingRegressor(n_estimators = 2)
    gbr.fit(mat, lbl)

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
                    [1, 0, 0, 1, 0, 1, 0]])
    lbl = np.array([1.2,0.3,1.1,1.9])  
    
    # Since "mat" is numpy dense data, we have created FrovedisColmajorMatrix.  
    from frovedis.matrix.dense import FrovedisColmajorMatrix
    from frovedis.matrix.dvector import FrovedisDvector 
    cmat = FrovedisColmajorMatrix(mat)
    dlbl = FrovedisDvector(lbl)
    
    # fitting input matrix and label on GradientBoostingRegressor object
    from frovedis.mllib.ensemble import GradientBoostingRegressor
    gbr = GradientBoostingRegressor(n_estimators = 2)
    gbr.fit(cmat, dlbl)

__Return Value__  
It simply returns "self" reference.  

### 3. predict(X)  
__Parameters__  
**_X_**: A numpy dense or any python array-like object or an instance of FrovedisRowmajorMatrix 
for dense data. It has shape **(n_samples, n_features)**. Currently, it supports only dense data 
as input.  

__Purpose__  
Predict regression target for X.  

For example,   

    # predicting on gradient boosting regressor model
    gbr.predict(mat)  

Output  

    [1.2 0.3 1.1 1.9]

Like in fit(), frovedis-like input can be used to speed-up the prediction making on 
the trained model at server side.  

For example,  

    # Since "cmat" is FrovedisColmajorMatrix, we have created FrovedisRowmajorMatrix. 
    # predicting on gradient boosting regressor model
    gbr.predict(cmat.to_frovedis_rowmatrix())  

Output  

    [1.2 0.3 1.1 1.9]

__Return Value__  
It returns a numpy array of float or double (float64) type and of shape **(n_samples,)** 
containing the predicted values.  

### 4. get_params(deep = True)  
__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If 
True, it will return the parameters for an estimator and contained subobjects that are 
estimators. (Default: True)  

__Purpose__    
This method belongs to the BaseEstimator class inherited by GradientBoostingRegressor. It is 
used to get parameters and their values of GradientBoostingRegressor class.  

For example, 
 
    print(gbr.get_params())

Output  

    {'alpha': 0.9, 'ccp_alpha': 0.0, 'criterion': 'friedman_mse', 'init': None, 
    'learning_rate': 0.1, 'loss': 'ls', 'max_bins': 32, 'max_depth': 3, 'max_features': None, 
    'max_leaf_nodes': None, 'min_impurity_decrease': 0.0, 'min_impurity_split': None, 
    'min_samples_leaf': 1, 'min_samples_split': 2, 'min_weight_fraction_leaf': 0.0, 
    'n_estimators': 2, 'n_iter_no_change': None, 'presort': 'deprecated', 'random_state': -1, 
    'subsample': 1.0, 'tol': 0.0001, 'validation_fraction': 0.1, 'verbose': 0, 
    'warm_start': False}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 5. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed to this function as dictionary. This 
dictionary contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by GradientBoostingRegressor, used 
to set parameter values.  

For example,   

    print("get parameters before setting:") 
    print(gbr.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    gbr.set_params(n_estimators = 5) 
    print("get parameters after setting:") 
    print(gbr.get_params())

Output  
     
    get parameters before setting:  
    {'alpha': 0.9, 'ccp_alpha': 0.0, 'criterion': 'friedman_mse', 'init': None, 
    'learning_rate': 0.1, 'loss': 'ls', 'max_bins': 32, 'max_depth': 3, 'max_features': None, 
    'max_leaf_nodes': None, 'min_impurity_decrease': 0.0, 'min_impurity_split': None, 
    'min_samples_leaf': 1, 'min_samples_split': 2, 'min_weight_fraction_leaf': 0.0, 
    'n_estimators': 2, 'n_iter_no_change': None, 'presort': 'deprecated', 'random_state': -1, 
    'subsample': 1.0, 'tol': 0.0001, 'validation_fraction': 0.1, 'verbose': 0, 
    'warm_start': False}  
    get parameters after setting:    
    {'alpha': 0.9, 'ccp_alpha': 0.0, 'criterion': 'friedman_mse', 'init': None, 
    'learning_rate': 0.1, 'loss': 'ls', 'max_bins': 32, 'max_depth': 3, 'max_features': None, 
    'max_leaf_nodes': None, 'min_impurity_decrease': 0.0, 'min_impurity_split': None, 
    'min_samples_leaf': 1, 'min_samples_split': 2, 'min_weight_fraction_leaf': 0.0, 
    'n_estimators': 5, 'n_iter_no_change': None, 'presort': 'deprecated', 'random_state': -1, 
    'subsample': 1.0, 'tol': 0.0001, 'validation_fraction': 0.1, 'verbose': 0, 
    'warm_start': False}  
    
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

    gbr.load("./out/gbt_regressor_model")  

__Return Value__  
It simply returns "self" reference.  

### 7. save(fname)  
__Parameters__  
**_fname_**: A string object containing the name of the file on which the target model 
is to be saved.  

__Purpose__  
On success, it writes the model information (metadata and model) in the specified file 
as little-endian binary data. Otherwise, it throws an exception.  

For example,   

    # To save the gradient boosting regressor model
    gbr.save("./out/gbt_regressor_model")  

This will save the gradient boosted regressor model on the path "/out/gbt_regressor_model". 
It would raise exception if the directory already exists with same name.  

The 'gbt_regressor_model' directory has  

**gbt_regressor_model**  
|-----metadata  
|-----model  

The 'metadata' file contains the model kind and input datatype used for trained model.  
The 'model' file contains the gradient boosting model saved in binary format.  

__Return Value__  
It returns nothing.  

### 8. score(X, y, sample_weight = None)  
__Parameters__  
**_X_**: A numpy dense or any python array-like object or an instance of FrovedisRowmajorMatrix 
for dense data. It has shape **(n_samples, n_features)**. Currently, it supports only dense 
data as input.  
**_y_**: Any python array-like object containing the true values for X. It has 
shape **(n_samples,)**.  
**_sample\_weight_**: A python ndarray containing the intended weights for each input 
samples and it should be the shape of **(n_samples,)**.  
When it is None (not specified explicitly), an uniform weight vector is assigned on each 
input sample. (Default: None)  

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
    gbr.score(mat,lbl)  

Output  

    1.00

__Return Value__  
It returns an R2 score of float type.  

### 9. debug_print()  

__Purpose__    
It shows the target model information on the server side user terminal. It is mainly 
used for debugging purpose.  

For example,

    gbr.debug_print()
    
Output

    -------- Regression Trees (GBTs):: --------
    # of trees: 2
    ---- [0] ----
        # of nodes: 7, height: 2
        <1> Split: feature[1] < 0.25, IG: 0.180625
         \_ <2> Split: feature[0] < 5.5, IG: 0.1225
         |  \_ (4) Predict: 1.9
         |  \_ (5) Predict: 1.2
         \_ <3> Split: feature[3] < 0.5, IG: 0.16
             \_ (6) Predict: 1.1
             \_ (7) Predict: 0.3
    ---- [1] ----
        # of nodes: 1, height: 0
         (1) Predict: 0

This output will be visible on server side. It displays the gradient boosting tree having maximum 
depth of 4 and total 2 decision trees.  

**No such output will be visible on client side.**  

__Return Value__  
It returns nothing.  

### 10. release()  

__Purpose__  
It can be used to release the in-memory model at frovedis server.  

For example,

    gbr.release()

This will reset the after-fit populated attributes (like n_features_) to None, 
along with releasing server side memory.  

__Return Value__  
It returns nothing.  

### 11. is_fitted()  

__Purpose__  
It can be used to confirm if the model is already fitted or not.  

__Return Value__  
It returns 'True', if the model is already fitted, otherwise, it returns 'False'.  

# SEE ALSO
dvector, rowmajor_matrix, colmajor_matrix, gradient_boost_tree_classifier, decision_tree_regressor