#!/usr/bin/env python

from .model_util import *
from ..exrpc.rpclib import *
from ..exrpc.server import *
from ..matrix.ml_data import FrovedisLabeledPoint
from ..matrix.dtype import TypeUtil
from .metrics import *

# Factorization Machine Classifier class
class FactorizationMachineClassifier:
  "A python wrapper of Frovedis Factorization Machine Classifier"

  """
  parameter               :    default value
  iteration               :    100
  init_stdev              :    0.1
  init_learn_rate         :    0.01
  optimizer               :    SGD
  global_bias             :    True
  dim_one_interactions    :    True
  dim_factors_no          :    8
  reg_intercept           :    0 
  reg_one_interactions    :    0
  reg_factors_no          :    0
  verbose                 :    0
  batch_size_pernode      :    100
  """

  # defaults are as per Frovedis/scikit-learn
  # Factorization Machine Classifier constructor
  def __init__(cls, iteration = 100, init_stdev = 0.1,
               init_learn_rate = 0.01, optimizer="SGD", dim = (True, True, 8),
               reg = (0, 0, 0), batch_size_pernode = 100, verbose = 0):
    cls.iteration = iteration
    cls.init_stdev = init_stdev
    cls.init_learn_rate = init_learn_rate
    cls.optimizer = optimizer
    cls.batch_size_pernode = batch_size_pernode
    cls.global_bias = dim[0]                       #global bias term 
    cls.dim_one_interactions = dim[1]              #one-way interactions
    cls.dim_factors_no = dim[2]                    #number of factors that are used for pairwise interactions
    cls.reg_intercept = reg[0]                     #regularization parameters of intercept
    cls.reg_one_interactions = reg[1]              #one-way interactions
    cls.reg_factors_no = reg[2]                    #number of factors that are used for pairwise interactions
    cls.verbose = verbose
    # extra
    cls.__mid = None
    cls.__mdtype = None
    cls.__mkind = M_KIND.FMM
    cls.isregressor = False
  
  def validate(cls):
    support = ['SGD', 'SGDA', 'ALS', 'MCMC']
    if cls.init_stdev < 0:
      raise ValueError("Initial standard deviation should be positive")
    elif cls.init_learn_rate < 0.00001 or cls.init_learn_rate > 1.0:
      raise ValueError("init_learn_rate should be in range [0.00001,1.0]")
    elif cls.iteration < 0:
      raise ValueError("iteration should be positive")
    elif cls.batch_size_pernode < 0:
      raise ValueError("BatchSize should be positive")
    elif cls.reg_intercept < 0 and cls.reg_one_interactions < 0 and cls.reg_factors_no<0:
      raise ValueError("Reg values should be positive")
    elif cls.dim_factors_no < 1:
      raise ValueError("Factor should be greater than 0")
    elif cls.optimizer not in support:
      raise ValueError("Unknown optimizer ", cls.optimizer, "!")
    elif (cls.global_bias is None) or (cls.dim_one_interactions is None) or (
        cls.dim_factors_no is None):
      raise ValueError ("Dimension parameters can not be None !")
    elif (cls.reg_intercept is None) or (cls.reg_one_interactions is None) or (
          cls.reg_factors_no is None): 
      raise ValueError ("Regularization parameters can not be None")
    elif cls.verbose < 0:
      raise ValueError("verbose should be positive")

  # Fit Factorization Machine classifier according to X (input data), y (Label)
  def fit(cls, X, y, sample_weight=None):
    cls.validate()
    cls.release()
    cls.__mid = ModelID.get()
    inp_data = FrovedisLabeledPoint(X,y)
    (X, y) = inp_data.get()
    dtype = inp_data.get_dtype()
    itype = inp_data.get_itype()
    dense = inp_data.is_dense()
    cls.__mdtype = dtype
    if(dense): raise TypeError("Expected sparse matrix data for training!")
    
    (host,port) = FrovedisServer.getServerInstance()
    rpclib.fm_train(host, port, X.get(),
                    y.get(), cls.init_stdev, cls.iteration, 
                    cls.init_learn_rate, cls.optimizer.encode('ascii'), cls.global_bias,
                    cls.dim_one_interactions, cls.dim_factors_no, 
                    cls.reg_intercept,
                    cls.reg_one_interactions, cls.reg_factors_no, 
                    cls.batch_size_pernode,
                    cls.__mid, cls.isregressor, cls.verbose, 
                    dtype, itype)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 
    return cls

  # Perform classification on an array of test vectors X.
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

# Factorization Machine Regressor class	
class FactorizationMachineRegressor:
  "A python wrapper of Frovedis Factorization Machine Regressor"

  """
  parameter               :    default value
  iteration               :    100
  init_stdev              :    0.1
  init_learn_rate         :    0.01
  optimizer               :    SGD
  global_bias             :    True
  dim_one_interactions    :    True
  dim_factors_no          :    8
  reg_intercept           :    0 
  reg_one_interactions    :    0
  reg_factors_no          :    0
  verbose                 :    0
  batch_size_pernode      :    100
  """

  # defaults are as per Frovedis/scikit-learn
  # Factorization Machine Regressor constructor
  def __init__(cls, iteration = 100, init_stdev = 0.1,
               init_learn_rate = 0.01, optimizer="SGD", dim = (True, True, 8),
               reg = (0, 0, 0), batch_size_pernode = 100, verbose = 0):
    cls.iteration = iteration
    cls.init_stdev = init_stdev
    cls.init_learn_rate = init_learn_rate
    cls.optimizer = optimizer
    cls.batch_size_pernode = batch_size_pernode
    cls.global_bias = dim[0]                       #global bias term
    cls.dim_one_interactions = dim[1]              #one-way interactions
    cls.dim_factors_no = dim[2]                    #number of factors that are used for pairwise interactions
    cls.reg_intercept = reg[0]                     #regularization parameters of intercept
    cls.reg_one_interactions = reg[1]              #one-way interactions
    cls.reg_factors_no = reg[2]                    #number of factors that are used for pairwise interactions
    cls.verbose = verbose
    # extra
    cls.__mid = None
    cls.__mdtype = None
    cls.__mkind = M_KIND.FMM
    cls.isregressor = True
  
  def validate(cls):
    support = ['SGD', 'SGDA', 'ALS', 'MCMC']
    if cls.init_stdev < 0:
      raise ValueError("Initial standard deviation should be positive")
    elif cls.init_learn_rate < 0.00001 or cls.init_learn_rate > 1.0:
      raise ValueError("init_learn_rate should be in range [0.00001,1.0]")
    elif cls.iteration < 0:
      raise ValueError("iteration should be positive")
    elif cls.batch_size_pernode < 0:
      raise ValueError("BatchSize should be positive")
    elif cls.reg_intercept < 0 and cls.reg_one_interactions < 0 and cls.reg_factors_no<0:
      raise ValueError("Reg values should be positive")
    elif cls.dim_factors_no < 1:
      raise ValueError("Factor should be greater than 0")
    elif cls.optimizer not in support:
      raise ValueError("Unknown optimizer ", cls.optimizer, "!")
    elif (cls.global_bias is None) or (cls.dim_one_interactions is None) or (
        cls.dim_factors_no is None):
      raise ValueError ("Dimension parameters can not be None !")
    elif (cls.reg_intercept is None) or (cls.reg_one_interactions is None) or (
          cls.reg_factors_no is None): 
      raise ValueError ("Regularization parameters can not be None")
    elif cls.verbose < 0:
      raise ValueError("verbose should be positive")

  # Fit Factorization Machine Regressor according to X (input data), y (Label)
  def fit(cls, X, y, sample_weight=None):
    cls.validate()
    cls.release()
    cls.__mid = ModelID.get()
    inp_data = FrovedisLabeledPoint(X,y)
    (X, y) = inp_data.get()
    dtype = inp_data.get_dtype()
    itype = inp_data.get_itype()
    dense = inp_data.is_dense()
    cls.__mdtype = dtype
    if(dense): raise TypeError("Expected Sparse matrix data for training!")
    
    (host,port) = FrovedisServer.getServerInstance()
    rpclib.fm_train(host, port, X.get(),
              y.get(), cls.init_stdev, cls.iteration, 
              cls.init_learn_rate, cls.optimizer.encode('ascii'), cls.global_bias,
              cls.dim_one_interactions, cls.dim_factors_no, cls.reg_intercept,
              cls.reg_one_interactions, cls.reg_factors_no, 
              cls.batch_size_pernode,
              cls.__mid, cls.isregressor, cls.verbose,
              dtype, itype)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 
    return cls

  # Perform classification on an array of test vectors X.
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
