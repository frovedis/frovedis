#!/usr/bin/env python

from model_util import *
from ..exrpc.rpclib import *
from ..exrpc.server import *
from ..matrix.sparse import FrovedisCRSMatrix
from ..matrix.dvector import FrovedisDoubleDvector

class LinearSVC:
   "A python wrapper of Frovedis Linear SVM"

   # defaults are as per Frovedis
   # C (alpha): Frovedis: 0.01, Sklearn: 1.0
   # loss: Frovedis: 'hinge', Sklearn: 'squared_hinge'
   def __init__(cls, penalty='l2', loss='hinge', dual=True, tol=1e-4,
                C=0.01, multi_class='ovr', fit_intercept=True,
                intercept_scaling=1, class_weight=None, verbose=0,
                random_state=None, max_iter=1000, solver='sag'):
      cls.penalty = penalty
      cls.loss = loss
      cls.dual = dual
      cls.tol = tol
      cls.C = C
      cls.multi_class = multi_class
      cls.fit_intercept = fit_intercept
      cls.intercept_scaling = intercept_scaling
      cls.class_weight = class_weight
      cls.verbose = verbose
      cls.random_state = random_state
      cls.max_iter = max_iter
      # extra
      cls.solver = solver
      cls.__mid = None
      cls.__mkind = M_KIND.SVM

   def fit(cls, X, y, sample_weight=None):
      cls.release()
      cls.__mid = ModelID.get()
      X = FrovedisCRSMatrix.asCRS(X)
      y = FrovedisDoubleDvector.asDvec(y)

      regTyp = 0
      if cls.penalty == 'l1': regTyp = 1
      elif cls.penalty == 'l2': reTyp = 2
      else: raise ValueError, "Invalid penalty is provided: " + cls.penalty

      if cls.multi_class == 'ovr': pass
      elif cls.multi_class == 'multinomial':
         raise ValueError, "Frovedis doesn't support multinomial SVM currently."
      else: raise ValueError, "Invalid multi_class input is provided: " + cls.multi_class

      (host,port) = FrovedisServer.getServerInstance()
      if cls.solver == 'sag':
        rpclib.svm_sgd(host,port,X.get(),y.get(),cls.max_iter,cls.C,
                       regTyp,cls.fit_intercept,cls.tol,cls.verbose,cls.__mid)

      elif cls.solver == 'lbfgs':
        rpclib.svm_lbfgs(host,port,X.get(),y.get(),cls.max_iter,cls.C,
                         regTyp,cls.fit_intercept,cls.tol,cls.verbose,cls.__mid)
      else: raise ValueError, "Unknown solver %s for Linear SVM." % cls.solver

      return cls
      
   def predict(cls,X):
      if cls.__mid is not None:
         return GLM.predict(X,cls.__mid,cls.__mkind,False)
      else: 
         raise ValueError, "predict is called before calling fit, or the model is released."
 
   def predict_proba(cls,X):
      if cls.__mid is not None: 
         return GLM.predict(X,cls.__mid,cls.__mkind,True)
      else: 
         raise ValueError, "predict is called before calling fit, or the model is released."

   def load(cls,fname):
      cls.release()
      cls.__mid = ModelID.get()
      GLM.load(cls.__mid,cls.__mkind,fname)
      return cls

   def save(cls,fname):
      if cls.__mid is not None: GLM.save(cls.__mid,cls.__mkind,fname)

   def debug_print(cls):
      if cls.__mid is not None: GLM.debug_print(cls.__mid,cls.__mkind)
   
   def release(cls):
      if cls.__mid is not None:
         GLM.release(cls.__mid,cls.__mkind)
         #print("Frovedis SVM model with " + str(cls.__mid) + " is released")
         cls.__mid = None

   def __del__(cls): 
      if FrovedisServer.isUP(): cls.release()
     

