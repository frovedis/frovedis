% GradientBoostingClassifier  

# NAME  
GradientBoostingClassifier - It is a machine learning algorithm, used for binary 
classification problems. It works on the principle that many weak learners 
(multiple decision trees) can together make a more accurate predictor.  

# SYNOPSIS  

    class frovedis.mllib.ensemble.gbtree.GradientBoostingClassifier(loss="deviance",  
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
                                                                    max_features=None, verbose=0,  
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
GradientBoostingClassifier is a supervised machine learning algorithm used for classification 
using decision trees. In gradient boosting decision trees, we combine many weak 
learners to come up with one strong learner. The weak learners here are the individual decision 
trees. All the trees are connected in series and each tree tries to minimise the error of the 
previous tree. Due to this sequential connection, boosting algorithms are usually slow to 
learn, but also highly accurate. **Currently, frovedis supports only binary gradient boosting 
classification algorithm.**  

During training, the input **X** is the training data and **y** is the corresponding 
label values (Frovedis supports any values for labels, but internally it encodes the 
input binary labels to -1 and 1) before training at Frovedis server) which we want 
to predict.  

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis interface is almost same as Scikit-learn
GradientBoostingClassifier interface, but it doesn't have any dependency with Scikit-learn. 
It can be used simply even if the system doesn't have Scikit-learn installed.
Thus in this implementation, a python client can interact with a frovedis server
sending the required python data for training at frovedis side. Python data is
converted into frovedis compatible data internally and the python ML call is
linked with the respective frovedis ML call to get the job done at frovedis server. 

Python side calls for GradientBoostingClassifier on the frovedis server. Once the training
is completed with the input data at the frovedis server, it returns an abstract 
model with a unique model ID to the client python program.  

When prediction-like request would be made on the trained model, python program 
will send the same request to the frovedis server. After the request is served at 
the frovedis server, the output would be sent back to the python client.  

## Detailed Description  

### 1. GradientBoostingClassifier()  

__Parameters__  
**_loss_**: A string object parameter that specifies the loss function to be optimized. Currently, 
it supports only ‘deviance’ loss (logloss as in logistic regression) for classification. (Default: 'deviance')  
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
individual estimators. (Default: 3)  
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
**_classes\__**: It is a python ndarray (any type) of unique labels given to the classifier 
during training. It has shape **(n_classes,)**.  
**_n\_estimators\__**: An integer value specifying the 'n_estimators' value.  
**_n\_features\__**: An integer value specifying the number of features when fitting the estimator.  

__Purpose__   
It initializes a GradientBoostingClassifier object with the given parameters.  

The parameters: "min_samples_split", "min_samples_leaf", "min_weight_fraction_leaf", 
"min_impurity_split", "init", "max_leaf_nodes", "warm_start", "presort", "validation_fraction", 
"n_iter_no_change" and "ccp_alpha" are simply kept in to make the interface uniform to the 
Scikit-learn GradientBoostingClassifier module. They are not used anywhere within frovedis 
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
    lbl = np.array([1, 0, 1, 0])
    
    # fitting input matrix and label on GradientBoostingClassifier object
    from frovedis.mllib.ensemble import GradientBoostingClassifier
    gbc = GradientBoostingClassifier(n_estimators = 2)
    gbc.fit(mat, lbl)

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
    lbl = np.array([1, 0, 1, 0])  
    
    # Since "mat" is numpy dense data, we have created FrovedisColmajorMatrix.  
    from frovedis.matrix.dense import FrovedisColmajorMatrix
    from frovedis.matrix.dvector import FrovedisDvector 
    cmat = FrovedisColmajorMatrix(mat)
    dlbl = FrovedisDvector(lbl)
    
    # fitting input matrix and label on GradientBoostingClassifier object
    from frovedis.mllib.ensemble import GradientBoostingClassifier
    gbc = GradientBoostingClassifier(n_estimators = 2)
    gbc.fit(cmat, dlbl)

__Return Value__  
It simply returns "self" reference.  

### 3. predict(X)  
__Parameters__  
**_X_**: A numpy dense or any python array-like object or an instance of FrovedisRowmajorMatrix 
for dense data. It has shape **(n_samples, n_features)**. Currently, it supports only dense data as input.  

__Purpose__  
Predict class for X.  

For example,   

    # predicting on gradient boosting classifier model
    gbc.predict(mat)  

Output  

    [1 0 1 0]

Like in fit(), frovedis-like input can be used to speed-up the prediction making on 
the trained model at server side.  

For example,  

    # Since "cmat" is FrovedisColmajorMatrix, we have created FrovedisRowmajorMatrix. 
    # predicting on gradient boosting classifier model
    gbc.predict(cmat.to_frovedis_rowmatrix())  

Output  

    [1 0 1 0]

__Return Value__  
It returns a numpy array of int64 type and of shape **(n_samples,)** containing 
the predicted classes.  

### 4. get_params(deep = True)  
__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If True, 
it will return the parameters for an estimator and contained subobjects that are estimators. (Default: True)  

__Purpose__    
This method belongs to the BaseEstimator class inherited by GradientBoostingClassifier. It 
is used to get parameters and their values of GradientBoostingClassifier class.  

For example, 
 
    print(gbc.get_params())

Output  

    {'ccp_alpha': 0.0, 'criterion': 'friedman_mse', 'init': None, 'learning_rate': 0.1, 
    'loss': 'deviance', 'max_bins': 32, 'max_depth': 3, 'max_features': None, 
    'max_leaf_nodes': None, 'min_impurity_decrease': 0.0, 'min_impurity_split': None, 
    'min_samples_leaf': 1, 'min_samples_split': 2, 'min_weight_fraction_leaf': 0.0, 
    'n_estimators': 2, 'n_iter_no_change': None, 'presort': 'deprecated', 'random_state': -1, 
    'subsample': 1.0, 'tol': 0.0001, 'validation_fraction': 0.1, 'verbose': 0, 
    'warm_start': False}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 5. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed to this function as dictionary. This dictionary 
contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by GradientBoostingClassifier, used 
to set parameter values.  

For example,   

    print("get parameters before setting:") 
    print(gbc.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    gbc.set_params(n_estimators = 5) 
    print("get parameters after setting:") 
    print(gbc.get_params())

Output  
     
    get parameters before setting:  
    {'ccp_alpha': 0.0, 'criterion': 'friedman_mse', 'init': None, 'learning_rate': 0.1, 
    'loss': 'deviance', 'max_bins': 32, 'max_depth': 3, 'max_features': None, 
    'max_leaf_nodes': None, 'min_impurity_decrease': 0.0, 'min_impurity_split': None, 
    'min_samples_leaf': 1, 'min_samples_split': 2, 'min_weight_fraction_leaf': 0.0, 
    'n_estimators': 2, 'n_iter_no_change': None, 'presort': 'deprecated', 'random_state': -1, 
    'subsample': 1.0, 'tol': 0.0001, 'validation_fraction': 0.1, 'verbose': 0, 
    'warm_start': False}  
    get parameters after setting:    
    {'ccp_alpha': 0.0, 'criterion': 'friedman_mse', 'init': None, 'learning_rate': 0.1, 
    'loss': 'deviance', 'max_bins': 32, 'max_depth': 3, 'max_features': None, 
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

    gbc.load("./out/gbt_classifier_model")  

__Return Value__  
It simply returns "self" reference.  

### 7. save(fname)  
__Parameters__  
**_fname_**: A string object containing the name of the file on which the target model 
is to be saved.  

__Purpose__  
On success, it writes the model information (metadata and model) in the specified 
file as little-endian binary data. Otherwise, it throws an exception.  

For example,   

    # To save the gradient boosting classifier model
    gbc.save("./out/gbt_classifier_model")  

This will save the gradient boosted classifier model on the path "/out/gbt_classifier_model". 
It would raise exception if the directory already exists with same name.  

The 'gbt_classifier_model' directory has  

**gbt_classifier_model**  
|-----label_map  
|-----metadata  
|-----model  

‘label_map’ file contains information about labels mapped with their encoded value.  
The 'metadata' file contains the number of classes, model kind and input datatype used for 
trained model.  
The 'model' file contains the gradient boosting model saved in binary format.  

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
When it is None (not specified explicitly), an uniform weight vector is assigned on each 
input sample. (Default: None)  

__Purpose__  
Calculate mean accuracy on the given test data and labels i.e. mean accuracy of 
self.predict(X) wrt. y.  

For example,   

    # calculate mean accuracy score on given test data and labels
    gbc.score(mat,lbl)  

Output  

    1.00

__Return Value__  
It returns an accuracy score of float type.   

### 9. debug_print()  

__Purpose__    
It shows the target model information on the server side user terminal. It is mainly 
used for debugging purpose.  

For example,

    gbc.debug_print()
    
Output

    -------- Classification Trees (GBTs):: --------
    # of trees: 2
    ---- [0] ----
      # of nodes: 3, height: 1
      <1> Split: feature[3] < 0.25, IG: 1
       \_ (2) Predict: 1
       \_ (3) Predict: -1
    ---- [1] ----
      # of nodes: 3, height: 1
      <1> Split: feature[3] < 0.25, IG: 0.227349
       \_ (2) Predict: 0.476812
       \_ (3) Predict: -0.476812

This output will be visible on server side. It displays the gradient boosting tree having maximum 
depth of 4 and total 2 decision trees.  

**No such output will be visible on client side.**  

__Return Value__  
It returns nothing.  

### 10. release()  

__Purpose__  
It can be used to release the in-memory model at frovedis server.  

For example,

    gbc.release()

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
- **[Introduction to FrovedisRowmajorMatrix](../matrix/rowmajor_matrix.md)**  
- **[Introduction to FrovedisColmajorMatrix](../matrix/colmajor_matrix.md)**  
- **[Introduction to FrovedisCRSMatrix](../matrix/crs_matrix.md)**  
- **[Introduction to FrovedisDvector](../matrix/dvector.md)**  
- **[Gradient Boosting Regressor in Frovedis](./gradient_boosting_regressor.md)**  
- **[Decision Tree Classifier in Frovedis](./decision_tree_classifier.md)**  
