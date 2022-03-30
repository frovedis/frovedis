% DecisionTreeRegressor  

# NAME  
DecisionTreeRegressor - A regression algorithm used to predict the output using conditional 
control statements. A decision tree is a decision support tool that uses a tree-like 
model of decisions and their possible consequences, including chance, event, outcomes, 
resource costs, and utility.  

# SYNOPSIS  

    class frovedis.mllib.tree.DecisionTreeRegressor(criterion='mse', splitter='best',  
                                                    max_depth=None, min_samples_split=2,  
                                                    min_samples_leaf=1,  
                                                    min_weight_fraction_leaf=0.0,  
                                                    max_features=None, random_state=None,  
                                                    max_leaf_nodes=None,  
                                                    min_impurity_decrease=0.0,  
                                                    presort='deprecated', ccp_alpha=0.0,  
                                                    max_bins=32, verbose=0,  
                                                    categorical_info={})  

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
Decision Tree Regressor is a supervised learning method used for regression 
problems. The aim is to create a model that predicts the value of a target variable by 
learning simple decision rules inferred from the data features. Like any other tree 
representation, it has a root node, internal nodes, and leaf nodes. The internal node 
represents condition on attributes, the branches represent the results of the condition 
and the leaf node represents the class label.  

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis interface is almost same as Scikit-learn
DecisionTreeRegressor interface, but it doesn't have any dependency with Scikit-learn. 
It can be used simply even if the system doesn't have Scikit-learn installed.
Thus in this implementation, a python client can interact with a frovedis server
sending the required python data for training at frovedis side. Python data is
converted into frovedis compatible data internally and the python ML call is
linked with the respective frovedis ML call to get the job done at frovedis server. 

Python side calls for DecisionTreeRegressor on the frovedis server. Once the training
is completed with the input data at the frovedis server, it returns an abstract 
model with a unique model ID to the client python program.  

When prediction-like request would be made on the trained model, python program 
will send the same request to the frovedis server. After the request is served at 
the frovedis server, the output would be sent back to the python client.  

## Detailed Description  

### 1. DecisionTreeRegressor()  

__Parameters__  
**_criterion_**: A string object parameter that specifies the function to measure the 
quality of a split.  
Currently, supported criteria is 'mse'. The mean squared error (mse), is equal to 
variance reduction as a feature selection criterion and minimizes the L2 loss using 
the mean of each terminal node. (Default: 'mse')  
**_splitter_**: An unused parameter. (Default: 'best')  
**_max\_depth_**: A positive integer parameter that specifies the maximum depth of the 
tree. (Default: None)  
When it is None (not specified explicitly), then 'max_depth' is set to 5.  
**_min\_samples\_split_**: An unused parameter. (Default: 2)  
**_min\_samples\_leaf_**: A positive integer or float value that specifies the minimum 
number of samples required to be at a leaf node. A split point at any depth will only be 
considered if it leaves at least 'min_samples_leaf' training samples in each of the left 
and right branches. This may have the effect of smoothing the model, especially in regression. (Default: 1)  
- If it is an integer, then 'min_samples_leaf' should be greater than or equal to 1.  
- If it is float, then 'min_samples_leaf' should be in range **(0,0.5]**.  

**_min\_weight\_fraction\_leaf_**: An unused parameter. (Default: 0.0)  
**_max\_features_**: An unused parameter. (Default: None)  
**_random\_state_**: An unused parameter. (Default: None)  
**_max\_leaf\_nodes_**: An unused parameter. (Default: None)  
**_min\_impurity\_decrease_**: A positive float parameter. A node will be split if this 
split induces a decrease of the impurity greater than or equal to this value. (Default: 0.0)  
**_presort_**: An unused parameter. (Default: 'deprecated')  
**_ccp\_alpha_**: An unused parameter. (Default: 0.0)  
**_max\_bins_**: A positive integer parameter that specifies the maximum number of bins 
created by ordered splits. (Default: 32)  
**_verbose_**: An integer parameter specifying the log level to use. Its value is set as 0 
by default (for INFO mode). But it can be set to 1 (for DEBUG mode) or 2 (for TRACE mode) for 
getting training time logs from frovedis server.  
**_categorical\_info_**: A dictionary that specifies categorical features information. Here, 
it gives column indices of categorical features and the number of categories for those features.  
For example, a dictionary { {0, 2}, {4, 5} } means that the feature[0] takes values 0 or 1 (binary) 
and the feature[4] has five categories (values 0, 1, 2, 3 or 4). Note that feature indices 
and category assignments are 0-based. (Default: {})  

__Attributes__  
**_n\_features\__**: An integer value specifying the number of features when fitting the estimator.  

__Purpose__   
It initializes a DecisionTreeRegressor object with the given parameters.  

The parameters: "splitter", "min_samples_split", "min_weight_fraction_leaf", "max_features", 
"random_state", "max_leaf_nodes", "presort" and "ccp_alpha" are simply kept in to make the 
interface uniform to the Scikit-learn DecisionTreeRegressor module. They are not used 
anywhere within frovedis implementation.  

__Return Value__  
It simply returns "self" reference.  

### 2. fit(X, y)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisColmajorMatrix for dense data. 
It has shape **(n_samples, n_features)**.  
**_y_**: Any python array-like object or an instance of FrovedisDvector containing the target 
values for X. It has shape **(n_samples,)**.  

__Purpose__  
It builds a decision tree regressor from the training data X and labels y.  

For example,   

    # loading a sample matrix and labels data
    from sklearn.datasets import load_boston
    mat, lbl = load_boston(return_X_y = True)
    
    # fitting input matrix and label on DecisionTreeRegressor object
    from frovedis.mllib.tree import DecisionTreeRegressor
    dtr = DecisionTreeRegressor(max_depth = 5)
    dtr.fit(mat,lbl)

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,  
    
    # loading a sample matrix and labels data
    from sklearn.datasets import load_boston
    mat, lbl = load_boston(return_X_y = True)
    
    # Since "mat" is numpy dense data, we have created FrovedisColmajorMatrix. 
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisColmajorMatrix
    from frovedis.matrix.dvector import FrovedisDvector 
    cmat = FrovedisColmajorMatrix(mat)
    dlbl = FrovedisDvector(lbl)
    
    # DecisionTreeRegressor with pre-constructed frovedis-like inputs
    from frovedis.mllib.tree import DecisionTreeRegressor
    dtr = DecisionTreeRegressor(max_depth = 5)
    dtr.fit(cmat,dlbl)

__Return Value__  
It simply returns "self" reference.  

### 3. predict(X)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data. 
It has shape **(n_samples, n_features)**.  

__Purpose__  
Predict regression value for X.  

For a regression model, the predicted value based on X is returned.  

For example,   

    # predicting on decision tree regressor model
    dtr.predict(mat)  

Output  

    [24.64166667 24.64166667 34.80666667 ... 27.83636364 27.83636364 20.98461538]  

Like in fit(), frovedis-like input can be used to speed-up the prediction making on 
the trained model at server side.  

For example,  

    # Since "cmat" is FrovedisColmajorMatrix, we have created FrovedisRowmajorMatrix.
    # predicting on decision tree regressor model using pre-constructed input
    dtr.predict(cmat.to_frovedis_rowmatrix())

Output  

    [24.64166667 24.64166667 34.80666667 ... 27.83636364 27.83636364 20.98461538]  

__Return Value__  
It returns a numpy array of float or double (float64) type and of shape **(n_samples,)** 
containing the predicted values.  

### 4. get_params(deep = True)  
__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If True, 
it will return the parameters for an estimator and contained subobjects that are estimators. (Default: True)  

__Purpose__    
This method belongs to the BaseEstimator class inherited by DecisionTreeRegressor. It is used 
to get parameters and their values of DecisionTreeRegressor class.  

For example, 
 
    print(dtr.get_params())

Output  

    {'categorical_info': {}, 'ccp_alpha': 0.0, 'criterion': 'MSE', 'max_bins': 32, 
    'max_depth': 5, 'max_features': None, 'max_leaf_nodes': None, 'min_impurity_decrease': 0.0, 
    'min_samples_leaf': 1, 'min_samples_split': 2, 'min_weight_fraction_leaf': 0.0, 
    'presort': 'deprecated', 'random_state': None, 'splitter': 'best', 'verbose': 0}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 5. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed to this function as dictionary. This dictionary 
contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by DecisionTreeRegressor, used to set 
parameter values.  

For example,   

    print("get parameters before setting:") 
    print(dtr.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    dtr.set_params(max_depth = 4) 
    print("get parameters after setting:") 
    print(dtr.get_params())

Output  
     
    get parameters before setting:
    {'categorical_info': {}, 'ccp_alpha': 0.0, 'criterion': 'MSE', 'max_bins': 32, 
    'max_depth': 5, 'max_features': None, 'max_leaf_nodes': None, 'min_impurity_decrease': 0.0, 
    'min_samples_leaf': 1, 'min_samples_split': 2, 'min_weight_fraction_leaf': 0.0, 
    'presort': 'deprecated', 'random_state': None, 'splitter': 'best', 'verbose': 0}
    get parameters after setting:
    {'categorical_info': {}, 'ccp_alpha': 0.0, 'criterion': 'MSE', 'max_bins': 32, 
    'max_depth': 4, 'max_features': None, 'max_leaf_nodes': None, 'min_impurity_decrease': 0.0, 
    'min_samples_leaf': 1, 'min_samples_split': 2, 'min_weight_fraction_leaf': 0.0, 
    'presort': 'deprecated', 'random_state': None, 'splitter': 'best', 'verbose': 0}
    
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

    dtr.load("./out/MyDecisionTreeRegressorModel")

__Return Value__  
It simply returns "self" reference.  

### 7. save(fname)  
__Parameters__  
**_fname_**: A string object containing the name of the file on which the target model 
is to be saved.  

__Purpose__  
On success, it writes the model information (metadata and model) in the specified file as 
little-endian binary data. Otherwise, it throws an exception.  

For example,   

    # To save the decision tree regressor model
    dtr.save("./out/MyDecisionTreeRegressorModel")  

This will save the decision tree regressor model on the path "/out/MyDecisionTreeRegressorModel". 
It would raise exception if the directory already exists with same name.  

The 'MyDecisionTreeRegressorModel' directory has  

**MyDecisionTreeRegressorModel**   
|-----metadata  
|-----model  

The 'metadata' file contains the information on model kind and input datatype used for trained model.  
The 'model' file contains the decision tree model saved in binary format.  

__Return Value__  
It returns nothing.  

### 8. score(X, y, sample_weight = None)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data. 
It has shape **(n_samples, n_features)**.  
**_y_**: Any python array-like object or an instance of FrovedisDvector containing the true values 
for X. It has shape **(n_samples,)**.  
**_sample\_weight_**: Python ndarray containing the intended weights for each input
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
    dtr.score(mat,lbl)  

Output  

    0.9109

__Return Value__  
It returns an R2 score of float type.   

### 9. debug_print()  

__Purpose__    
It shows the target model information on the server side user terminal. It is mainly 
used for debugging purpose.  

For example,

    dtr.debug_print()
    
Output

    -------- Regression Tree:: --------
    # of nodes: 51, height: 5
    <1> Split: feature[12] < 9.6575, IG: 37.2288
     \_ <2> Split: feature[5] < 7.14906, IG: 37.6417
     |  \_ <4> Split: feature[7] < 1.47325, IG: 17.8515
     |  |  \_ (8) Predict: 50
     |  |  \_ <9> Split: feature[5] < 6.69962, IG: 9.19518
     |  |     \_ <18> Split: feature[5] < 6.10819, IG: 2.41396
     |  |     |  \_ (36) Predict: 20.9846
     |  |     |  \_ (37) Predict: 24.6417
     |  |     \_ <19> Split: feature[12] < 5.27, IG: 6.57304
     |  |        \_ (38) Predict: 32.9652
     |  |        \_ (39) Predict: 27.8364
     |  \_ <5> Split: feature[5] < 7.45969, IG: 25.6196
     |     \_ <10> Split: feature[5] < 7.43531, IG: 4.71838
     |     |  \_ <20> Split: feature[8] < 7.125, IG: 1.42815
     |     |  |  \_ (40) Predict: 34.8067
     |     |  |  \_ (41) Predict: 31.6
     |     |  \_ (21) Predict: 44
     |     \_ <11> Split: feature[0] < 2.1766, IG: 19.28
     |        \_ <22> Split: feature[10] < 17.9563, IG: 5.68288
     |        |  \_ (44) Predict: 46.9375
     |        |  \_ (45) Predict: 40.125
     |        \_ (23) Predict: 21.9
     \_ <3> Split: feature[12] < 14.9763, IG: 9.38095
        \_ <6> Split: feature[5] < 6.56925, IG: 1.74534
        |  \_ <12> Split: feature[11] < 101.175, IG: 0.921292
        |  |  \_ <24> Split: feature[4] < 0.5995, IG: 6.02083
        |  |  |  \_ (48) Predict: 16.5667
        |  |  |  \_ (49) Predict: 10.9
        |  |  \_ <25> Split: feature[2] < 2.57187, IG: 0.586878
        |  |     \_ (50) Predict: 25.3667
        |  |     \_ (51) Predict: 20.4598
        |  \_ <13> Split: feature[0] < 4.94925, IG: 10.4783
        |     \_ <26> Split: feature[4] < 0.59225, IG: 7.70469
        |     |  \_ (52) Predict: 24.4429
        |     |  \_ (53) Predict: 30.5
        |     \_ (27) Predict: 15
        \_ <7> Split: feature[0] < 5.64819, IG: 5.7942
           \_ <14> Split: feature[4] < 0.52725, IG: 2.64736
           |  \_ <28> Split: feature[12] < 29.8562, IG: 2.20522
           |  |  \_ (56) Predict: 20.4
           |  |  \_ (57) Predict: 15.45
           |  \_ <29> Split: feature[12] < 18.0497, IG: 1.97256
           |     \_ (58) Predict: 17.5667
           |     \_ (59) Predict: 14.7563
           \_ <15> Split: feature[12] < 20.0091, IG: 3.36396
              \_ <30> Split: feature[5] < 5.92387, IG: 2.19683
              |  \_ (60) Predict: 11.4714
              |  \_ (61) Predict: 15.137
              \_ <31> Split: feature[4] < 0.67175, IG: 4.84222
                 \_ (62) Predict: 13.88
                 \_ (63) Predict: 9.15938

This output will be visible on server side. It displays the decision tree having maximum depth 
of 5 and total 51 nodes in the tree.  

**No such output will be visible on client side.**  

__Return Value__  
It returns nothing.  

### 10. release()  

__Purpose__  
It can be used to release the in-memory model at frovedis server.  

For example,

    dtr.release()

This will remove the trained model, model-id present on server, along with releasing 
server side memory.  

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
- **[Decision Tree Classifier in Frovedis](./decision_tree_classifier.md)**  
