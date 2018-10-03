#!/usr/bin/env python

from model_util import *
from ..exrpc.rpclib import *
from ..exrpc.server import *
from ..matrix.ml_data import FrovedisLabeledPoint
from ..matrix.dtype import TypeUtil
from metrics import *
import numpy as np

# Decision Tree Regressor Class
class DecisionTreeRegressor:
  "A python wrapper of Frovedis Decision Tree Regressor"

  """
  parameter   			:   default value
  criterion or impurity         :   'mse'
  plitter    			:   'best'
  in_impurity_decrease		:   0.0
  in_samples_split  		:   2
  min_samples_leaf   		:   1
  in_weight_fraction_leaf 	:   0.0
  presort     			:   False
  verbose      			:   0
  """

  # defaults are as per Frovedis/scikit-learn
  # Decision Tree Regressor constructor
  def __init__(cls, criterion='mse', splitter='best',
               max_depth=None, min_samples_split=2, min_samples_leaf=1,
               min_weight_fraction_leaf=0.0, max_features=None, random_state=None,
               max_leaf_nodes=1, min_impurity_decrease=0.0, min_impurity_split=None,
               class_weight=None, presort=False, 
               min_info_gain = 0.0, max_bins = 32, verbose = 0):
    cls.criterion = criterion.upper()
    cls.splitter = splitter
    if max_depth is None: cls.max_depth = 5
    else: cls.max_depth = max_depth
    cls.min_samples_split = min_samples_split
    cls.min_samples_leaf = min_samples_leaf
    cls.min_weight_fraction_leaf = min_weight_fraction_leaf
    cls.max_features = max_features
    cls.random_state = random_state
    cls.max_leaf_nodes = max_leaf_nodes
    cls.min_impurity_decrease = min_impurity_decrease
    cls.min_impurity_split = min_impurity_split
    cls.class_weight = class_weight
    cls.presort = presort
    cls.verbose = verbose
    # extra
    cls.__mid = None
    cls.__mdtype = None
    cls.__mkind = M_KIND.DTM
    # Frovedis side parameters
    cls.min_info_gain = min_info_gain 
    cls.max_bins = max_bins
    cls.algo = "Regression"
    cls.n_classes_ = 0

  #To validate the input parameters
  def validate(cls):
    if cls.criterion != "MSE":
      raise ValueError("Invalid criterion for Decision Tree Regressor!")
    elif cls.max_depth < 0:
      raise ValueError("max depth can not be negative !")
    elif cls.min_info_gain < 0:
      raise ValueError("Value of min_info_gain should be greater than 0")
    elif cls.max_bins < 0:
      raise ValueError("Value of max_bin should be greater than 0")
    elif cls.n_classes_ < 0:
      raise ValueError("Value of number of classes should be +ve integer or zero!")
    elif cls.min_samples_leaf < 0:
      raise ValueError("Value of min_samples_leaf should be greater than 0!")

  # Fit Decision Tree Regressor according to X (input data), y (Label)
  def fit(cls, X, y):
    cls.validate()
    cls.release()
    cls.__mid = ModelID.get()
    inp_data = FrovedisLabeledPoint(X,y)
    (X, y) = inp_data.get()
    dtype = inp_data.get_dtype()
    itype = inp_data.get_itype()
    dense = inp_data.is_dense()
    cls.__mdtype = dtype

    (host,port) = FrovedisServer.getServerInstance()
    rpclib.dt_train(host,port,X.get(),y.get(), 
                    cls.algo, cls.criterion, cls.max_depth,
                    cls.n_classes_, cls.max_bins, cls.min_samples_leaf, 
                    cls.min_info_gain, cls.verbose, cls.__mid, 
                    dtype, itype, dense)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 
    return cls
    
  # Perform prediction on an array of test vectors X.
  def predict(cls,X):
    if cls.__mid is not None:
      return GLM.predict(X,cls.__mid,cls.__mkind,cls.__mdtype,False)
    else: 
      raise ValueError("predict is called before calling fit, or the model is released.")
 
  # Load the model from a file
  def load(cls,fname,dtype=None):
    cls.release()
    cls.__mid = ModelID.get()
    if dtype is None: 
      if cls.__mdtype is None:
        raise TypeError("model type should be specified for loading from file!")
    else: cls.__mdtype = TypeUtil.to_id_dtype(dtype)
    GLM.load(cls.__mid,cls.__mkind,cls.__mdtype,fname)
    return cls

  # Save model to a file
  def save(cls,fname):
    if cls.__mid is not None: GLM.save(cls.__mid,cls.__mkind,cls.__mdtype,fname)


  # calculate the root mean square value on the given test data and labels.
  def score(cls, X, y):
    if cls.__mid is not None:
      return r2_score(y, cls.predict(X))

  # Show the model	
  def debug_print(cls):
    if cls.__mid is not None: GLM.debug_print(cls.__mid,cls.__mkind,cls.__mdtype)

  # Release the model-id to generate new model-id
  def release(cls):
    if cls.__mid is not None:
      GLM.release(cls.__mid,cls.__mkind,cls.__mdtype)
      cls.__mid = None

  # Check FrovedisServer is up then release	  
  def __del__(cls):
    if FrovedisServer.isUP(): cls.release()

# Decision Tree Classifier Class   
class DecisionTreeClassifier:
  "A python wrapper of Frovedis Decision Tree Classifier"

  """
  parameter   			:   default value
  criterion or impurity   	:   'gini'
  splitter    			:   'best'
  min_impurity_decrease		:   0.0
  min_samples_split  		:   2
  min_samples_leaf   		:   1
  min_weight_fraction_leaf 	:   0.0
  presort     			:   False
  verbose      			:   0
  """

  # defaults are as per Frovedis/scikit-learn
  # Decision Tree Regressor constructor
  def __init__(cls, criterion='gini', splitter='best',
               max_depth=None, min_samples_split=2, min_samples_leaf=1,
               min_weight_fraction_leaf=0.0, max_features=None, random_state=None,
               max_leaf_nodes=1, min_impurity_decrease=0.0, min_impurity_split=None,
               class_weight=None, presort=False,min_info_gain=0.0,
               max_bins=32,verbose=0):
    cls.criterion = criterion.upper()
    cls.splitter = splitter
    if max_depth is None: cls.max_depth = 5
    else: cls.max_depth = max_depth
    cls.min_samples_split = min_samples_split
    cls.min_samples_leaf = min_samples_leaf
    cls.min_weight_fraction_leaf = min_weight_fraction_leaf
    cls.max_features = max_features
    cls.random_state = random_state
    cls.min_impurity_decrease = min_impurity_decrease
    cls.min_impurity_split = min_impurity_split
    cls.class_weight = class_weight
    cls.max_leaf_nodes = max_leaf_nodes
    cls.presort = presort
    cls.verbose = verbose
    # extra
    cls.__mid = None
    cls.__mdtype = None
    cls.__mkind = M_KIND.DTM
    # Frovedis side parameters
    cls.min_info_gain = min_info_gain
    cls.max_bins = max_bins
    cls.algo = "Classification"
  
  def validate(cls):
    if cls.criterion != "GINI" and cls.criterion != "ENTROPY":
      raise ValueError("Invalid criterion for Decision Tree Regressor!")
    elif cls.max_depth < 0:
      raise ValueError("max depth can not be negative !")
    elif cls.min_info_gain < 0:
      raise ValueError("Value of min_info_gain should be greater than 0")
    elif cls.max_bins < 0:
      raise ValueError("Value of max_bin should be greater than 0")
    elif cls.min_samples_leaf < 0:
      raise ValueError("Value of min_samples_leaf should be greater than 0!")
    elif cls.n_classes_ < 0:
      raise ValueError("Value of number of classes should be +ve integer or zero!")

  # Fit Decision Tree classifier according to X (input data), y (Label)
  def fit(cls, X, y):
    # compute number of classes in given label vector (y)
    n_labels = []
    for e in y: 
      if e not in n_labels: n_labels.append(e)
    cls.n_classes_ = len(n_labels)
    # validate hyper-parameters
    cls.validate()
    # release old model, if any
    cls.release()
    # perform the fit
    cls.__mid = ModelID.get()
    inp_data = FrovedisLabeledPoint(X,y)
    (X, y) = inp_data.get()
    dtype = inp_data.get_dtype()
    itype = inp_data.get_itype()
    dense = inp_data.is_dense()
    cls.__mdtype = dtype

    (host,port) = FrovedisServer.getServerInstance()
    rpclib.dt_train(host,port,X.get(),y.get(), 
                    cls.algo, cls.criterion, cls.max_depth,
                    cls.n_classes_, cls.max_bins, cls.min_samples_leaf, 
                    cls.min_info_gain, cls.verbose, cls.__mid,
                    dtype, itype, dense)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 
    return cls
    
  # Perform classification on an array of test vectors X.
  def predict(cls,X):
    if cls.__mid is not None:
      return GLM.predict(X,cls.__mid,cls.__mkind,cls.__mdtype,False)
    else: 
      raise ValueError("predict is called before calling fit, or the model is released.")
 
  # Perform classification on an array and return probability estimates for the test vector X.
  def predict_proba(cls,X):
    if cls.__mid is not None: 
      return GLM.predict(X,cls.__mid,cls.__mkind,cls.__mdtype,True)
    else: 
      raise ValueError("predict is called before calling fit, or the model is released.")
  
  # Load the model from a file
  def load(cls,fname,dtype=None):
    cls.release()
    cls.__mid = ModelID.get()
    if dtype is None: 
      if cls.__mdtype is None:
        raise TypeError("model type should be specified for loading from file!")
    else: cls.__mdtype = TypeUtil.to_id_dtype(dtype)
    GLM.load(cls.__mid,cls.__mkind,cls.__mdtype,fname)
    return cls

  # calculate the mean accuracy on the given test data and labels.
  def score(cls,X,y):
    if cls.__mid is not None:
      return accuracy_score(y, cls.predict(X))


  # Save model to a file
  def save(cls,fname):
    if cls.__mid is not None: GLM.save(cls.__mid,cls.__mkind,cls.__mdtype,fname)

  # Show the model	
  def debug_print(cls):
    if cls.__mid is not None: GLM.debug_print(cls.__mid,cls.__mkind,cls.__mdtype)

  # Release the model-id to generate new model-id
  def release(cls):
    if cls.__mid is not None:
      GLM.release(cls.__mid,cls.__mkind,cls.__mdtype)
      cls.__mid = None

  # Check FrovedisServer is up then release	  
  def __del__(cls):
    if FrovedisServer.isUP(): cls.release()
