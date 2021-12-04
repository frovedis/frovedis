% DecisionTreeClassifier  

# NAME  
DecisionTreeClassifier - A classification algorithm that predicts the binary and 
multi-class output using conditional control statements. A decision tree is a 
decision support tool that uses a tree-like model of decisions and their possible 
consequences, including chance, event, outcomes, resource costs, and utility.  

# SYNOPSIS  

class frovedis.mllib.tree.DecisionTreeClassifier(criterion='gini', splitter='best', max_depth=None,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ min_samples_split=2, min_samples_leaf=1,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ min_weight_fraction_leaf=0.0, max_features=None,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ random_state=None, max_leaf_nodes=None,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ min_impurity_decrease=0.0, class_weight=None,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ presort='deprecated', ccp_alpha=0.0,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ max_bins=32, verbose=0, categorical_info={})  
 
## Public Member Functions  

fit(X, y)  
predict(X)  
predict_proba(X)  
get_params(deep = True)  
set_params(\*\*params)  
load(fname, dtype = None)  
save(fname)  
score(X, y, sample_weight = None)  
debug_print()  
release()  
is_fitted()  

# DESCRIPTION  
Decision Tree Classifier is a supervised learning method used for classification 
problems. The aim is to create a model that predicts the value of a target variable by 
learning simple decision rules inferred from the data features. Like any other tree 
representation, it has a root node, internal nodes, and leaf nodes. The internal node 
represents condition on attributes, the branches represent the results of the condition 
and the leaf node represents the class label. **Frovedis supports both binary and 
multinomial decision tree classification algorithms.**  

During training, the input **X** is the training data and **y** is the corresponding 
label values (Frovedis supports any values for labels, but internally it encodes the 
input binary labels to 0 and 1, and input multinomial labels to 0, 1, 2, ..., N-1 
(where N is the no. of classes) before training at Frovedis server) which we want 
to predict.  

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis interface is almost same as Scikit-learn
DecisionTreeClassifier interface, but it doesn't have any dependency with Scikit-learn. 
It can be used simply even if the system doesn't have Scikit-learn installed.
Thus in this implementation, a python client can interact with a frovedis server
sending the required python data for training at frovedis side. Python data is
converted into frovedis compatible data internally and the python ML call is
linked with the respective frovedis ML call to get the job done at frovedis server. 

Python side calls for DecisionTreeClassifier on the frovedis server. Once the training
is completed with the input data at the frovedis server, it returns an abstract 
model with a unique model ID to the client python program.  

When prediction-like request would be made on the trained model, python program 
will send the same request to the frovedis server. After the request is served at 
the frovedis server, the output would be sent back to the python client.  

## Detailed Description  

### 1. DecisionTreeClassifier()  

__Parameters__  
**_criterion_**: A string object parameter that specifies the function to measure the 
quality of a split. Supported criteria are 'gini' and 'entropy'. (Deault: 'gini')  
'gini' impurity: calculates the amount of probability of a specific feature that is 
classified incorrectly when selected randomly.  
'entropy' (information gain): it is applied to quantify which feature provides maximal 
information about the classification based on the notion of entropy.  
**_splitter_**: An unused parameter. (Default: 'best')  
**_max\_depth_**: A positive integer parameter that specifies the maximum depth of the 
tree.  
If it is None (not specified explicitly), then 'max_depth' is set to 5. (Default: None)  
**_min\_samples\_split_**: An unused parameter. (Default: 2)  
**_min\_samples\_leaf_**: A positive integer or float value that specifies the minimum 
number of samples required to be at a leaf node. A split point at any depth will only be 
considered if it leaves at least 'min_samples_leaf' training samples in each of the left 
and right branches. (Default: 1)  
If it is an integer, then 'min_samples_leaf' should be greater than or equal to 1.  
If it is float, then 'min_samples_leaf' should be in range (0,0.5].  
**_min\_weight\_fraction\_leaf_**: An unused parameter. (Default: 0.0)  
**_max\_features_**: An unused parameter. (Default: None)  
**_random\_state_**: An unused parameter. (Default: None)  
**_max\_leaf\_nodes_**: An unused parameter. (Default: None)  
**_min\_impurity\_decrease_**: A positive double (float64) parameter. A node will be split 
if this split induces a decrease of the impurity greater than or equal to this value. (Default: 0.0)  
**_class\_weight_**: An unused parameter. (Default: None)  
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
**_classes\__**: It is a python ndarray(any type) of unique labels given to the classifier 
during training. It has shape **(n_classes,)**.  

__Purpose__   
It initializes a DecisionTreeClassifier object with the given parameters.  

The parameters: "splitter", "min_samples_split", "min_weight_fraction_leaf", "max_features", 
"random_state", "max_leaf_nodes", "class_weight", "presort" and "ccp_alpha" are simply kept 
in to make the interface uniform to the Scikit-learn DecisionTreeClassifier module. They are 
not used anywhere within frovedis implementation.  

__Return Value__  
It simply returns "self" reference.  

### 2. fit(X, y)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisColmajorMatrix for dense data. 
It has shape **(n_samples, n_features)**.  
**_y_**: Any python array-like object or an instance of FrovedisDvector. It has shape **(n_samples,)**.  

__Purpose__  
It builds a decision tree classifier from the training data X and labels y.  

For example,   

    # loading a sample matrix and labels data
    from sklearn.datasets import load_breast_cancer
    mat, lbl = load_breast_cancer(return_X_y = True)
    
    # fitting input matrix and label on DecisionTreeClassifier object
    from frovedis.mllib.tree import DecisionTreeClassifier
    dtc = DecisionTreeClassifier(max_depth = 5)
    dtc.fit(mat,lbl)

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,  
    
    # loading a sample matrix and labels data
    from sklearn.datasets import load_breast_cancer
    mat, lbl = load_breast_cancer(return_X_y = True)
    
    # Since "mat" is numpy dense data, we have created FrovedisColmajorMatrix. 
    # For scipy sparse data, FrovedisCRSMatrix should be used instead.
    from frovedis.matrix.dense import FrovedisColmajorMatrix
    from frovedis.matrix.dvector import FrovedisDvector 
    cmat = FrovedisColmajorMatrix(mat)
    dlbl = FrovedisDvector(lbl)
    
    # DecisionTreeClassifier with pre-constructed frovedis-like inputs
    from frovedis.mllib.tree import DecisionTreeClassifier
    dtc = DecisionTreeClassifier(max_depth = 5)
    dtc.fit(cmat,dlbl)

__Return Value__  
It simply returns "self" reference.  

### 3. predict(X)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data. 
It has shape **(n_samples, n_features)**.  

__Purpose__  
Predict class for X.  

For a classification model, the predicted class value for each sample in X is returned.  

For example,   

    # predicting on decision tree classifier model
    dtc.predict(mat)  

Output  

    [0 0 0 ... 0 0 1]

Like in fit(), frovedis-like input can be used to speed-up the prediction making on 
the trained model at server side.  

For example,  

    # Since "cmat" is FrovedisColmajorMatrix, we have created FrovedisRowmajorMatrix.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix
    
    # predicting on decision tree classifier model using pre-constructed input
    dtc.predict(cmat.to_frovedis_rowmatrix())

Output  

    [0 0 0 ... 0 0 1]

__Return Value__  
It returns a numpy array of float or double (float64) type and of shape **(n_samples,)** 
containing the predicted classes.  

### 4. predict_proba(X)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data. 
It has shape **(n_samples, n_features)**.  

__Purpose__   
Predict class log-probabilities of the input samples X.  

**Currently, this method is not supported for multinomial classification problems.**  

For example,   

    # finds the probability sample for each class in the model
    dtc.predict_proba(mat)  

Output
    
    [[1.       0.      ]
     [1.       0.      ]
     [1.       0.      ]
     ...
     [1.       0.      ]
     [1.       0.      ]
     [0.009375 0.990625]]

Like in fit(), frovedis-like input can be used to speed-up the prediction making on 
the trained model at server side.  

For example,  

    # Since "cmat" is FrovedisColmajorMatrix, we have created FrovedisRowmajorMatrix.
    from frovedis.matrix.dense import FrovedisRowmajorMatrix

    # finds the probability sample for each class in the model
    dtc.predict_proba(cmat.to_frovedis_rowmatrix())  

Output
    
    [[1.       0.      ]
     [1.       0.      ]
     [1.       0.      ]
     ...
     [1.       0.      ]
     [1.       0.      ]
     [0.009375 0.990625]]

__Return Value__  
It returns a numpy array of float or double(float64) type and of shape **(n_samples, n_classes)** 
containing the prediction probability values. Here, **n_classes = 2**, since only binary classification 
problems are supported for this method.

### 5. get_params(deep = True)  
__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If True, 
it will return the parameters for an estimator and contained subobjects that are estimators. (Default: True)  

__Purpose__    
This method belongs to the BaseEstimator class inherited by DecisionTreeClassifier. It is used 
to get parameters and their values of DecisionTreeClassifier class.  

For example, 
 
      print(dtc.get_params())

Output  

    {'categorical_info': {}, 'ccp_alpha': 0.0, 'class_weight': None, 'criterion': 'GINI', 
    'max_bins': 32, 'max_depth': 5, 'max_features': None, 'max_leaf_nodes': None, 
    'min_impurity_decrease': 0.0, 'min_samples_leaf': 1, 'min_samples_split': 2, 
    'min_weight_fraction_leaf': 0.0, 'presort': 'deprecated', 'random_state': None, 
    'splitter': 'best', 'verbose': 0}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 6. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed to this function as dictionary. This dictionary 
contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by DecisionTreeClassifier, used to set 
parameter values.  

For example,   

    print("get parameters before setting:") 
    print(dtc.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    dtc.set_params(criterion = 'entropy', max_depth = 5) 
    print("get parameters after setting:") 
    print(dtc.get_params())

Output  
     
    get parameters before setting:
    {'categorical_info': {}, 'ccp_alpha': 0.0, 'class_weight': None, 'criterion': 'GINI', 
    'max_bins': 32, 'max_depth': 5, 'max_features': None, 'max_leaf_nodes': None, 
    'min_impurity_decrease': 0.0, 'min_samples_leaf': 1, 'min_samples_split': 2, 
    'min_weight_fraction_leaf': 0.0, 'presort': 'deprecated', 'random_state': None, 
    'splitter': 'best', 'verbose': 0}
    get parameters after setting:
    {'categorical_info': {}, 'ccp_alpha': 0.0, 'class_weight': None, 'criterion': 'entropy', 
    'max_bins': 32, 'max_depth': 5, 'max_features': None, 'max_leaf_nodes': None, 
    'min_impurity_decrease': 0.0, 'min_samples_leaf': 1, 'min_samples_split': 2, 
    'min_weight_fraction_leaf': 0.0, 'presort': 'deprecated', 'random_state': None, 
    'splitter': 'best', 'verbose': 0}
    
__Return Value__  
It simply returns "self" reference.  

### 7. load(fname, dtype = None)  
__Parameters__  
**_fname_**:  A string object containing the name of the file having model information
to be loaded.  
**_dtype_**: A data-type is inferred from the input data. Currently, expected input 
data-type is either float or double (float64). (Default: None)  

__Purpose__  
It loads the model from the specified file (having little-endian binary data).  

For example,  

    dtc.load("./out/MyDecisionTreeClassifierModel")

__Return Value__  
It simply returns "self" reference.  

### 8. save(fname)  
__Parameters__  
**_fname_**: A string object containing the name of the file on which the target model 
is to be saved.  

__Purpose__  
On success, it writes the model information (after-fit populated attributes) in the 
specified file as little-endian binary data. Otherwise, it throws an exception.  

For example,   

    # To save the decision tree classifier model
    dtc.save("./out/MyDecisionTreeClassifierModel")  

This will save the decision tree classifier model on the path "/out/MyDecisionTreeClassifierModel". 
It would raise exception if the directory already exists with same name.  

The 'MyDecisionTreeClassifierModel' directory has  

**MyDecisionTreeClassifierModel**  
|-----label_map  
|-----metadata  
|-----model  

‘label_map’ file contains information about labels mapped with their encoded value.  
The 'metadata' file contains the number of classes, model kind and input datatype used for 
trained model.  
The 'model' file contains the decision tree model saved in binary format.  

__Return Value__  
It returns nothing.  

### 9. score(X, y, sample_weight = None)  
__Parameters__  
**_X_**: A numpy dense or scipy sparse matrix or any python array-like object or 
an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data. 
It has shape **(n_samples, n_features)**.  
**_y_**: Any python array-like object or an instance of FrovedisDvector. It has shape **(n_samples,)**.  
**_sample\_weight_**: Python ndarray containing the intended weights for each input
samples and it should be the shape of **(n_samples, )**. When it is None (not specified explicitly), an
uniform weight vector is assigned on each input sample. (Default: None)  

__Purpose__  
Calculate mean accuracy on the given test data and labels i.e. mean accuracy of 
self.predict(X) wrt. y.  

For example,   

    # calculate mean accuracy score on given test data and labels
    dtc.score(mat,lbl)  

Output  

    0.9895

__Return Value__  
It returns an accuracy score of float type.   

### 10. debug_print()  

__Purpose__    
It shows the target model information on the server side user terminal. It is mainly 
used for debugging purpose.  

For example,

    dtc.debug_print()
    
Output

        -------- Classification Tree:: --------
    # of nodes: 35, height: 5
    <1> Split: feature[22] < 113.157, IG: 0.319406
     \_ <2> Split: feature[27] < 0.144844, IG: 0.0626618
     |  \_ <4> Split: feature[22] < 107.223, IG: 0.00955662
     |  |  \_ <8> Split: feature[10] < 0.904737, IG: 0.00582386
     |  |  |  \_ <16> Split: feature[27] < 0.133781, IG: 0.00558482
     |  |  |  |  \_ (32) Predict: 1 (99.0625%)
     |  |  |  |  \_ (33) Predict: 1 (66.6667%)
     |  |  |  \_ (17) Predict: 0 (100%)
     |  |  \_ <9> Split: feature[14] < 0.00570775, IG: 0.184704
     |  |     \_ <18> Split: feature[0] < 14.0963, IG: 0.165289
     |  |     |  \_ (36) Predict: 0 (100%)
     |  |     |  \_ (37) Predict: 1 (100%)
     |  |     \_ <19> Split: feature[21] < 21.9025, IG: 0.408163
     |  |        \_ (38) Predict: 1 (100%)
     |  |        \_ (39) Predict: 0 (100%)
     |  \_ <5> Split: feature[21] < 24.13, IG: 0.189372
     |     \_ <10> Split: feature[4] < 0.109085, IG: 0.32
     |     |  \_ (20) Predict: 1 (100%)
     |     |  \_ (21) Predict: 0 (100%)
     |     \_ <11> Split: feature[9] < 0.0609525, IG: 0.144027
     |        \_ (22) Predict: 1 (100%)
     |        \_ <23> Split: feature[7] < 0.0404469, IG: 0.0907029
     |           \_ (46) Predict: 1 (100%)
     |           \_ (47) Predict: 0 (100%)
     \_ <3> Split: feature[7] < 0.0509028, IG: 0.0364914
        \_ <6> Split: feature[17] < 0.00995862, IG: 0.259286
        |  \_ <12> Split: feature[1] < 15.855, IG: 0.336735
        |  |  \_ (24) Predict: 1 (100%)
        |  |  \_ (25) Predict: 0 (100%)
        |  \_ (13) Predict: 1 (100%)
        \_ <7> Split: feature[1] < 13.9925, IG: 0.0116213
           \_ <14> Split: feature[1] < 12.18, IG: 0.5
           |  \_ (28) Predict: 0 (100%)
           |  \_ (29) Predict: 1 (100%)
           \_ (15) Predict: 0 (100%)

It displays the decision tree having maximum depth of 5 and total 35 nodes in the tree.  

__Return Value__  
It returns nothing.  

### 11. release()  

__Purpose__  
It can be used to release the in-memory model at frovedis server.  

For example,

    dtc.release()

This will reset the after-fit populated attributes (like classes_) to None, along 
with releasing server side memory.  

__Return Value__  
It returns nothing.  

### 12. is_fitted()  

__Purpose__  
It can be used to confirm if the model is already fitted or not.  

__Return Value__  
It returns ‘True’, if the model is already fitted, otherwise, it returns ‘False’.  

# SEE ALSO
dvector, crs_matrix, rowmajor_matrix, colmajor_matrix