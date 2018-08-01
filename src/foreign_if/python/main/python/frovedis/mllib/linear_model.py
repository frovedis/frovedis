#!/usr/bin/env python

from model_util import *
from ..exrpc.rpclib import *
from ..exrpc.server import *
from ..matrix.sparse import FrovedisCRSMatrix
from ..matrix.dvector import FrovedisDoubleDvector

class LogisticRegression:
   "A python wrapper of Frovedis Logistic Regression"

   # defaults are as per Frovedis
   # C (alpha): Frovedis: 0.01, Sklearn: 1.0
   # max_iter: Frovedis: 1000, Sklearn: 100
   # solver: Frovedis: sag (SGD), Sklearn: liblinear
   def __init__(cls, penalty='l2', dual=False, tol=1e-4, C=0.01,
                fit_intercept=True, intercept_scaling=1, class_weight=None,
                random_state=None, solver='sag', max_iter=1000,
                multi_class='ovr', verbose=0, warm_start=False, n_jobs=1):
      cls.penalty = penalty
      cls.dual = dual
      cls.tol = tol
      cls.C = C
      cls.fit_intercept = fit_intercept
      cls.intercept_scaling = intercept_scaling
      cls.class_weight = class_weight
      cls.random_state = random_state
      cls.solver = solver
      cls.max_iter = max_iter
      cls.multi_class = multi_class
      cls.verbose = verbose
      cls.warm_start = warm_start
      cls.n_jobs = n_jobs
      # extra
      cls.__mid = None
      cls.__mkind = M_KIND.LRM

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
         raise ValueError, "Frovedis doesn't support multinomial Logistic Regression currently."
      else: raise ValueError, "Invalid multi_class input is provided: " + cls.multi_class
      
      (host,port) = FrovedisServer.getServerInstance()
      sv = ['newton-cg', 'liblinear']
      if cls.solver in sv:
         raise ValueError, "Frovedis doesn't support solver %s for Logistic Regression currently." % cls.solver

      if cls.solver == 'sag':
         rpclib.lr_sgd(host,port,X.get(),y.get(),cls.max_iter,cls.C,
                       regTyp,cls.fit_intercept,cls.tol,cls.verbose,cls.__mid)
      elif cls.solver == 'lbfgs':
         rpclib.lr_lbfgs(host,port,X.get(),y.get(),cls.max_iter,cls.C,
                         regTyp,cls.fit_intercept,cls.tol,cls.verbose,cls.__mid)
      else: raise ValueError, "Unknown solver %s for Logistic Regression." % cls.solver

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
         #print("Frovedis LR model with " + str(cls.__mid) + " is released")
         cls.__mid = None

   def __del__(cls): 
      if FrovedisServer.isUP(): cls.release()


class LinearRegression:
   "A python wrapper of Frovedis Linear Regression"

   def __init__(cls, fit_intercept=True, normalize=False, copy_X=True,
                n_jobs=1, solver='sag', verbose=0):
      cls.fit_intercept = fit_intercept
      cls.normalize = normalize
      cls.copy_X = copy_X
      cls.n_jobs = n_jobs
      # extra
      cls.solver = solver
      cls.verbose = verbose
      cls.__mid = None
      cls.__mkind = M_KIND.LNRM

   def fit(cls, X, y, sample_weight=None):
      cls.release()
      cls.__mid = ModelID.get()
      X = FrovedisCRSMatrix.asCRS(X)
      y = FrovedisDoubleDvector.asDvec(y)

      (host,port) = FrovedisServer.getServerInstance()
      if cls.solver == 'sag':
         rpclib.lnr_sgd(host,port,X.get(),y.get(),cls.fit_intercept,
                        cls.verbose,cls.__mid)
      elif cls.solver == 'lbfgs':
         rpclib.lnr_lbfgs(host,port,X.get(),y.get(),cls.fit_intercept,
                          cls.verbose,cls.__mid)
      else: raise ValueError, "Unknown solver %s for Linear Regression." % cls.solver

      return cls

   def predict(cls,X):
      if cls.__mid is not None:
         return GLM.predict(X,cls.__mid,cls.__mkind,False)
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
         #print("Frovedis LNR model with " + str(cls.__mid) + " is released")
         cls.__mid = None

   def __del__(cls):
      if FrovedisServer.isUP(): cls.release()


class Lasso:
   "A python wrapper of Frovedis Lasso Regression"

   # defaults are as per Frovedis
   # alpha: Frovedis: 0.01, Sklearn: 1.0
   def __init__(cls, alpha=0.01, fit_intercept=True, normalize=False,
                precompute=False, copy_X=True, max_iter=1000,
                tol=1e-4, warm_start=False, positive=False,
                random_state=None, selection='cyclic',
                verbose=0, solver='sag'):
      cls.alpha = alpha
      cls.fit_intercept = fit_intercept
      cls.normalize = normalize
      cls.precompute = precompute
      cls.copy_X = copy_X
      cls.max_iter = max_iter
      cls.tol = tol
      cls.warm_start = warm_start
      cls.positive = positive
      cls.random_state = random_state
      cls.selection = selection,
      # extra
      cls.verbose = verbose
      cls.solver = solver
      cls.__mid = None
      cls.__mkind = M_KIND.LNRM

   def fit(cls, X, y, sample_weight=None):
      cls.release()
      cls.__mid = ModelID.get()
      X = FrovedisCRSMatrix.asCRS(X)
      y = FrovedisDoubleDvector.asDvec(y)

      (host,port) = FrovedisServer.getServerInstance()
      if cls.solver == 'sag':
         rpclib.lasso_sgd(host,port,X.get(),y.get(),cls.max_iter,cls.alpha,
                          cls.fit_intercept,cls.tol,cls.verbose,cls.__mid)
      elif cls.solver == 'lbfgs':
         rpclib.lasso_lbfgs(host,port,X.get(),y.get(),cls.max_iter,cls.alpha,
                            cls.fit_intercept,cls.tol,cls.verbose,cls.__mid)
      else: raise ValueError, "Unknown solver %s for Lasso Regression." % cls.solver

      return cls

   def predict(cls,X):
      if cls.__mid is not None:
         return GLM.predict(X,cls.__mid,cls.__mkind,False)
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
         #print("Frovedis LNR (lasso) model with " + str(cls.__mid) + " is released")
         cls.__mid = None

   def __del__(cls):
      if FrovedisServer.isUP(): cls.release()


class Ridge:
   "A python wrapper of Frovedis Ridge Regression"

   # defaults are as per Frovedis
   # alpha: Frovedis: 0.01, Sklearn: 1.0
   def __init__(cls, alpha=0.01, fit_intercept=True, normalize=False,
                copy_X=True, max_iter=1000, tol=1e-3, solver='sag',
                random_state=None, verbose=0):
      cls.alpha = alpha
      cls.fit_intercept = fit_intercept
      cls.normalize = normalize
      cls.copy_X = copy_X
      cls.max_iter = max_iter
      cls.tol = tol
      cls.solver = solver
      cls.random_state = random_state
      # extra
      cls.verbose = verbose
      cls.__mid = None
      cls.__mkind = M_KIND.LNRM

   def fit(cls, X, y, sample_weight=None):
      cls.release()
      cls.__mid = ModelID.get()
      X = FrovedisCRSMatrix.asCRS(X)
      y = FrovedisDoubleDvector.asDvec(y)

      sv = ['svd', 'cholesky', 'lsqr', 'sparse_cg']
      if cls.solver in sv: 
         raise ValueError, "Frovedis doesn't support solver %s for Ridge Regression currently." % cls.solver

      (host,port) = FrovedisServer.getServerInstance()
      if cls.solver == 'sag' or cls.solver == 'auto':
         rpclib.ridge_sgd(host,port,X.get(),y.get(),cls.max_iter,cls.alpha,
                          cls.fit_intercept,cls.tol,cls.verbose,cls.__mid)
      elif cls.solver == 'lbfgs':
         rpclib.ridge_lbfgs(host,port,X.get(),y.get(),cls.max_iter,cls.alpha,
                            cls.fit_intercept,cls.tol,cls.verbose,cls.__mid)
      else:
         raise ValueError, "Unknown solver %s for Ridge Regression." % cls.solver

      return cls

   def predict(cls,X):
      if cls.__mid is not None:
         return GLM.predict(X,cls.__mid,cls.__mkind,False)
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
         #print("Frovedis LNR (ridge) model with " + str(cls.__mid) + " is released")
         cls.__mid = None

   def __del__(cls):
      if FrovedisServer.isUP(): cls.release()

