#!/usr/bin/env python

from model_util import *
from ..exrpc.rpclib import *
from ..exrpc.server import *
from ..matrix.ml_data import FrovedisLabeledPoint
from ..matrix.dtype import TypeUtil
from metrics import *
import numpy as np

# NaiveBayes multinomial class
class MultinomialNB:
  "A python wrapper of Frovedis Multinomial Naive Bayes"
  """
  parameter      :    default value
  alpha (lambda) :    1.0
  fit_prior      :    True
  class_prior    :    None
  verbose        :    0
  """

  # defaults are as per Frovedis/scikit-learn
  # NaiveBayes multinomial constructor
  def __init__(cls, alpha=1.0, fit_prior=True, class_prior=None, verbose = 0):
    cls.alpha = alpha
    cls.fit_prior = fit_prior
    cls.class_prior = class_prior
    cls.verbose = verbose
    # extra
    cls.__mid = None
    cls.__mdtype = None
    cls.__mkind = M_KIND.NBM
    cls.Algo = "multinomial"

  def validate(cls):
    if(cls.alpha < 0):
      raise ValueError("alpha should be greater than or equal to 1")
	  
  # Fit NaiveBayes multinomial classifier according to X (input data), y (Label).
  def fit(cls, X, y):
    cls.validate()
    cls.release()
    cls.__mid = ModelID.get()
    inp_data = FrovedisLabeledPoint(X,y)
    (X,y) = inp_data.get()
    (data, label) = inp_data.get()
    dtype = inp_data.get_dtype() 
    itype = inp_data.get_itype() 
    dense = inp_data.is_dense()
    cls.__mdtype = dtype
    (host,port) = FrovedisServer.getServerInstance()
    rpclib.nb_train(host, port, data.get(),
                    label.get(), cls.alpha, cls.__mid,
                    cls.Algo, cls.verbose, dtype, itype, dense)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 
    return cls

  # Perform classification on an array of test vectors X.
  def predict(cls,X):
    if cls.__mid is not None:
      return GLM.predict(X,cls.__mid,cls.__mkind,cls.__mdtype,False)
    else:
      raise ValueError("predict is called before calling fit, or the model is released.\n")

  # Perform classification on an array and return probability estimates for the test vector X.
  def predict_proba(cls,X):
    if cls.__mid is not None:
      return GLM.predict(X,cls.__mid,cls.__mkind,cls.__mdtype,True)
    else:
      raise ValueError("predict is called before calling fit, or the model is released.")
  
  # calculate the mean accuracy on the given test data and labels.
  def score(cls,X,y):
    if cls.__mid is not None:
      return accuracy_score(y, cls.predict(X))
  
  # Show the model
  def debug_print(cls):
    if cls.__mid is not None: GLM.debug_print(cls.__mid,cls.__mkind,cls.__mdtype)

  # Load the model from a file
  def load(cls,fname,dtype=None):
    cls.release()
    if dtype is None: 
      if cls.__mdtype is None:
        raise TypeError("model type should be specified for loading from file!")
    else: cls.__mdtype = TypeUtil.to_id_dtype(dtype)
    if isinstance(fname, str) == False: 
      raise TypeError("Expected: String, Got: " + str(type(fname)))
    cls.__mid = ModelID.get()
    (host,port) = FrovedisServer.getServerInstance()
    algo = rpclib.load_frovedis_nbm(host,port,cls.__mid,cls.__mdtype,fname)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 
    if(algo != cls.Algo):
      msg = "incorrect data file is loaded! "
      msg += "expected model type: " + cls.Algo + ", loaded model type: " + algo
      raise ValueError(msg)
    return cls
    
  # Save model to a file
  def save(cls,fname):
    if cls.__mid is not None: GLM.save(cls.__mid,cls.__mkind,cls.__mdtype,fname)
  
  # Release the model-id to generate new model-id
  def release(cls):
    if cls.__mid is not None:
      GLM.release(cls.__mid,cls.__mkind,cls.__mdtype)
      cls.__mid = None

  # Check FrovedisServer is up then release
  def __del__(cls):
    if FrovedisServer.isUP(): cls.release()

# Not implemented yet in Frovedis
# NaiveBayes Gaussian class
class GaussianNB:
  "A python wrapper of Frovedis Gaussian Naive Bayes"
  """
  parameter      :    default value
  alpha (lambda) :    1.0
  priors         :    None
  verbose        :    0
  """

  # defaults are as per Frovedis/scikit-learn
  # NaiveBayes Gaussian constructor
  def __init__(cls, priors = None, verbose = 0):
    cls.priors = priors
    # extra
    cls.alpha = 1.0
    cls.__mid = None
    cls.__mdtype = None
    cls.__mkind = M_KIND.NBM
    cls.Algo = "gaussian"
    cls.verbose = verbose    

  def validate(cls):
    if(cls.alpha < 0):
      raise ValueError("alpha should be greater than or equal to 1")

  # Fit Gaussian NaiveBayes classifier according to X (input data), y (Label)
  def fit(cls, X, y):
    cls.validate()
    cls.release()
    cls.__mid = ModelID.get()
    inp_data = FrovedisLabeledPoint(X,y)
    (data, label) = inp_data.get()
    dtype = inp_data.get_dtype()
    itype = inp_data.get_itype()
    dense = inp_data.is_dense()
    cls.__mdtype = dtype
    (host,port) = FrovedisServer.getServerInstance()
    rpclib.nb_train(host, port, data.get(),
                    label.get(), cls.alpha, cls.__mid,
                    cls.Algo, cls.verbose, dtype, itype, dense)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 

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
  
  # calculate the mean accuracy on the given test data and labels.
  def score(cls,X,y):
    if cls.__mid is not None:
      return accuracy_score(y, cls.predict(X))

  # Show the model
  def debug_print(cls):
    if cls.__mid is not None: GLM.debug_print(cls.__mid,cls.__mkind,cls.__mdtype)

  # Load the model from a file
  def load(cls,fname,dtype=None):
    cls.release()
    cls.__mid = ModelID.get()
    if dtype is None: 
      if cls.__mdtype is None:
        raise TypeError("model type should be specified for loading from file!")
    else: cls.__mdtype = TypeUtil.to_id_dtype(dtype)
    (host,port) = FrovedisServer.getServerInstance()
    algo = rpclib.load_frovedis_nbm(host,port,cls.__mid,cls.__mdtype,fname)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 
    if(algo != cls.Algo):
      msg = "incorrect data file is loaded! "
      msg += "expected model type: " + cls.Algo + ", loaded model type: " + algo
      raise ValueError(msg)
    return cls
    
  # Save model to a file
  def save(cls,fname):
    if cls.__mid is not None: GLM.save(cls.__mid,cls.__mkind,cls.__mdtype,fname)
  
  # Release the model-id to generate new model-id
  def release(cls):
    if cls.__mid is not None:
      GLM.release(cls.__mid,cls.__mkind,cls.__mdtype)
      cls.__mid = None

  # Check FrovedisServer is up then release
  def __del__(cls):
    if FrovedisServer.isUP(): cls.release()

# NaiveBayes Bernoulli class
class BernoulliNB:
  "A python wrapper of Frovedis Bernouli Naive Bayes"
  """
  parameter      :    default value
  alpha (lambda) :    1.0 
  fit_prior      :    True
  class_prior    :    None
  binarize       :    0.0
  verbose        :    0
  """

  # defaults are as per Frovedis/scikit-learn
  # NaiveBayes Bernoulli constructor
  def __init__(cls, alpha = 1.0, fit_prior = True, class_prior = None, binarize = 0.0, verbose = 0):
    cls.alpha = alpha
    cls.fit_prior = fit_prior
    cls.class_prior = class_prior
    cls.verbose = verbose
    cls.binarize = binarize
    # extra
    cls.__mid = None
    cls.__mdtype = None
    cls.__mkind = M_KIND.NBM
    cls.Algo = "bernoulli"

  def validate(cls):
    if(cls.alpha < 0):
      raise ValueError("alpha should be greater than or equal to 1")

  # Fit NaiveBayes multinomial classifier according to X (input data), y (Label).
  def fit(cls, X, y):
    cls.validate()
    cls.release()
    cls.__mid = ModelID.get()
    inp_data = FrovedisLabeledPoint(X,y)
    (data, label) = inp_data.get()
    dtype = inp_data.get_dtype()
    itype = inp_data.get_itype()
    dense = inp_data.is_dense()
    cls.__mdtype = dtype
    (host,port) = FrovedisServer.getServerInstance()
    rpclib.nb_train(host, port, data.get(),
                    label.get(), cls.alpha, cls.__mid,
                    cls.Algo, cls.verbose, dtype, itype, dense)
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

  # calculate the mean accuracy on the given test data and labels.
  def score(cls,X,y):
    if cls.__mid is not None:
      return accuracy_score(y, cls.predict(X))

  # Show the model
  def debug_print(cls):
    if cls.__mid is not None: GLM.debug_print(cls.__mid,cls.__mkind,cls.__mdtype)

  # Load the model from a file
  def load(cls,fname,dtype=None):
    cls.release()
    cls.__mid = ModelID.get()
    if dtype is None: 
      if cls.__mdtype is None:
        raise TypeError("model type should be specified for loading from file!")
    else: cls.__mdtype = TypeUtil.to_id_dtype(dtype)
    (host,port) = FrovedisServer.getServerInstance()
    algo = rpclib.load_frovedis_nbm(host,port,cls.__mid,cls.__mdtype,fname)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 
    if(algo != cls.Algo):
      msg = "incorrect data file is loaded! "
      msg += "expected model type: " + cls.Algo + ", loaded model type: " + algo
      raise ValueError(msg)
    return cls
 
  # Save model to a file
  def save(cls,fname):
    if cls.__mid is not None: GLM.save(cls.__mid,cls.__mkind,cls.__mdtype,fname)
  
  # Release the model-id to generate new model-id
  def release(cls):
    if cls.__mid is not None:
      GLM.release(cls.__mid,cls.__mkind,cls.__mdtype)
      cls.__mid = None

  # Check FrovedisServer is up then release
  def __del__(cls):
    if FrovedisServer.isUP(): cls.release()

