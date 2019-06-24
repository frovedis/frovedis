#!/usr/bin/env python

from .model_util import *
from ..exrpc.rpclib import *
from ..exrpc.server import FrovedisServer 
from ..matrix.ml_data import FrovedisLabeledPoint
from ..matrix.dtype import TypeUtil

class LogisticRegression:
   "A python wrapper of Frovedis Logistic Regression"

   # defaults are as per Frovedis
   # lr_rate: Frovedis: 0.01 (added)
   # max_iter: Frovedis: 1000, Sklearn: 100
   # solver: Frovedis: sag (SGD), Sklearn: liblinear
   def __init__(cls, penalty='l2', dual=False, tol=1e-4, C=1.0,
                fit_intercept=True, intercept_scaling=1, class_weight=None,
                random_state=None, solver='sag', max_iter=1000,
                multi_class='ovr', verbose=0, warm_start=False,
                n_jobs=1, l1_ratio=None, lr_rate=0.01):
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
      cls.l1_ratio = l1_ratio
      cls.lr_rate = lr_rate
      # extra
      cls.__mid = None
      cls.__mdtype = None
      cls.__mkind = None

   def fit(cls, X, y, sample_weight=None):
      cls.release()
      inp_data = FrovedisLabeledPoint(X,y)
      (X, y) = inp_data.get()
      dtype = inp_data.get_dtype()
      itype = inp_data.get_itype()
      dense = inp_data.is_dense()
      cls.n_classes = inp_data.get_distinct_label_count()
      cls.__mid = ModelID.get()
      cls.__mdtype = dtype

      if cls.multi_class == 'auto':
        if cls.n_classes == 2 :
          cls.multi_class = 'ovr'
        else:
          cls.multi_class = 'multinomial'

      if cls.penalty == 'l1': regTyp = 1
      elif cls.penalty == 'l2': regTyp = 2
      elif cls.penalty == 'none': regTyp = 0
      else: raise ValueError("Unsupported penalty is provided: ", cls.penalty)

      rparam = 1.0 / cls.C

      sv = ['newton-cg', 'liblinear', 'saga']
      if cls.solver in sv:
         raise ValueError("Frovedis doesn't support solver %s for Logistic Regression currently." % cls.solver)
      
      if cls.multi_class == 'ovr':
         isMult = False 
         cls.__mkind = M_KIND.LRM
      elif cls.multi_class == 'multinomial':
         isMult = True 
         cls.__mkind = M_KIND.MLR
      else: raise ValueError("Unknown multi_class type!")

      (host,port) = FrovedisServer.getServerInstance()
      if cls.solver == 'sag':
        rpclib.lr_sgd(host,port,X.get(),y.get(),cls.max_iter,cls.lr_rate,
                      regTyp,rparam,isMult,cls.fit_intercept,
                      cls.tol,cls.verbose,
                      cls.__mid,dtype,itype,dense)
      elif cls.solver == 'lbfgs':
        rpclib.lr_lbfgs(host,port,X.get(),y.get(),cls.max_iter,cls.lr_rate,
                        regTyp,rparam,isMult,cls.fit_intercept,
                        cls.tol,cls.verbose,
                        cls.__mid,dtype,itype,dense)
      else: raise ValueError("Unknown solver %s for Logistic Regression." % cls.solver)
      
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"]) 
		 
      return cls
      
   def predict(cls,X):
      if cls.__mid is not None:
         return GLM.predict(X,cls.__mid,cls.__mkind,cls.__mdtype,False)
      else: 
         raise ValueError("predict is called before calling fit, or the model is released.")
 
   def predict_proba(cls,X):
      if cls.__mid is not None: 
         return GLM.predict(X,cls.__mid,cls.__mkind,cls.__mdtype,True)
      else: 
         raise ValueError("predict is called before calling fit, or the model is released.")

   def load(cls, fname, is_multinomial=False, dtype=None):
      cls.release()
      cls.__mid = ModelID.get()
      if cls.__mkind is None: #in case called before fit()
        if is_multinomial: cls.__mkind = M_KIND.MLR
        else:              cls.__mkind = M_KIND.LRM
      if dtype is None: 
        if cls.__mdtype is None:
          raise TypeError("model type should be specified for loading from file!")
      else: cls.__mdtype = TypeUtil.to_id_dtype(dtype)
      GLM.load(cls.__mid,cls.__mkind,cls.__mdtype,fname)
      return cls

   def save(cls,fname):
      if cls.__mid is not None: GLM.save(cls.__mid,cls.__mkind,cls.__mdtype,fname)

   def debug_print(cls):
      if cls.__mid is not None: GLM.debug_print(cls.__mid,cls.__mkind,cls.__mdtype)
   
   def release(cls):
      if cls.__mid is not None:
         GLM.release(cls.__mid,cls.__mkind,cls.__mdtype)
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
      cls.__mdtype = None
      cls.__mkind = M_KIND.LNRM

   def fit(cls, X, y, sample_weight=None):
      cls.release()
      cls.__mid = ModelID.get()
      inp_data = FrovedisLabeledPoint(X,y)
      (X, y) = inp_data.get()
      dtype = inp_data.get_dtype()
      itype = inp_data.get_itype()
      dense = inp_data.is_dense()
      cls.__mdtype = dtype

      (host,port) = FrovedisServer.getServerInstance()
      if cls.solver == 'sag':
         rpclib.lnr_sgd(host,port,X.get(),y.get(),cls.fit_intercept,
                        cls.verbose,cls.__mid,dtype,itype,dense)
      elif cls.solver == 'lbfgs':
         rpclib.lnr_lbfgs(host,port,X.get(),y.get(),cls.fit_intercept,
                          cls.verbose,cls.__mid,dtype,itype,dense)
      else: raise ValueError("Unknown solver %s for Linear Regression." % cls.solver)
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"]) 
      return cls

   def predict(cls,X):
      if cls.__mid is not None:
         return GLM.predict(X,cls.__mid,cls.__mkind,cls.__mdtype,False)
      else:
         raise ValueError("predict is called before calling fit, or the model is released.")

   def load(cls,fname,dtype=None):
      cls.release()
      cls.__mid = ModelID.get()
      if dtype is None: 
        if cls.__mdtype is None:
          raise TypeError("model type should be specified for loading from file!")
      else: cls.__mdtype = TypeUtil.to_id_dtype(dtype)
      GLM.load(cls.__mid,cls.__mkind,cls.__mdtype,fname)
      return cls

   def save(cls,fname):
      if cls.__mid is not None: GLM.save(cls.__mid,cls.__mkind,cls.__mdtype,fname)

   def debug_print(cls):
      if cls.__mid is not None: GLM.debug_print(cls.__mid,cls.__mkind,cls.__mdtype)

   def release(cls):
      if cls.__mid is not None:
         GLM.release(cls.__mid,cls.__mkind,cls.__mdtype)
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
      cls.selection = selection
      # extra
      cls.verbose = verbose
      cls.solver = solver
      cls.__mid = None
      cls.__mdtype = None
      cls.__mkind = M_KIND.LNRM

   def fit(cls, X, y, sample_weight=None):
      cls.release()
      cls.__mid = ModelID.get()
      inp_data = FrovedisLabeledPoint(X,y)
      (X, y) = inp_data.get()
      dtype = inp_data.get_dtype()
      itype = inp_data.get_itype()
      dense = inp_data.is_dense()
      cls.__mdtype = dtype

      (host,port) = FrovedisServer.getServerInstance()
      if cls.solver == 'sag':
         rpclib.lasso_sgd(host,port,X.get(),y.get(),cls.max_iter,cls.alpha,
                          cls.fit_intercept,cls.tol,cls.verbose,cls.__mid,
			  dtype,itype,dense)
      elif cls.solver == 'lbfgs':
         rpclib.lasso_lbfgs(host,port,X.get(),y.get(),cls.max_iter,cls.alpha,
                            cls.fit_intercept,cls.tol,cls.verbose,cls.__mid,
			    dtype,itype,dense)
      else: raise ValueError("Unknown solver %s for Lasso Regression." % cls.solver)
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"]) 

      return cls

   def predict(cls,X):
      if cls.__mid is not None:
         return GLM.predict(X,cls.__mid,cls.__mkind,cls.__mdtype,False)
      else:
         raise ValueError("predict is called before calling fit, or the model is released.")

   def load(cls,fname,dtype=None):
      cls.release()
      cls.__mid = ModelID.get()
      if dtype is None: 
        if cls.__mdtype is None:
          raise TypeError("model type should be specified for loading from file!")
      else: cls.__mdtype = TypeUtil.to_id_dtype(dtype)
      GLM.load(cls.__mid,cls.__mkind,cls.__mdtype,fname)
      return cls

   def save(cls,fname):
      if cls.__mid is not None: GLM.save(cls.__mid,cls.__mkind,cls.__mdtype,fname)

   def debug_print(cls):
      if cls.__mid is not None: GLM.debug_print(cls.__mid,cls.__mkind,cls.__mdtype)

   def release(cls):
      if cls.__mid is not None:
         GLM.release(cls.__mid,cls.__mkind,cls.__mdtype)
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
      cls.__mdtype = None
      cls.__mkind = M_KIND.LNRM

   def fit(cls, X, y, sample_weight=None):
      cls.release()
      cls.__mid = ModelID.get()
      inp_data = FrovedisLabeledPoint(X,y)
      (X, y) = inp_data.get()
      dtype = inp_data.get_dtype()
      itype = inp_data.get_itype()
      dense = inp_data.is_dense()
      cls.__mdtype = dtype

      sv = ['svd', 'cholesky', 'lsqr', 'sparse_cg']
      if cls.solver in sv: 
         raise ValueError("Frovedis doesn't support solver %s for Ridge Regression currently." % cls.solver)

      (host,port) = FrovedisServer.getServerInstance()
      if cls.solver == 'sag' or cls.solver == 'auto':
         rpclib.ridge_sgd(host,port,X.get(),y.get(),cls.max_iter,cls.alpha,
                          cls.fit_intercept,cls.tol,cls.verbose,cls.__mid,
			  dtype,itype,dense)
      elif cls.solver == 'lbfgs':
         rpclib.ridge_lbfgs(host,port,X.get(),y.get(),cls.max_iter,cls.alpha,
                            cls.fit_intercept,cls.tol,cls.verbose,cls.__mid,
			    dtype,itype,dense)
      else:
         raise ValueError("Unknown solver %s for Ridge Regression." % cls.solver)
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"]) 

      return cls

   def predict(cls,X):
      if cls.__mid is not None:
         return GLM.predict(X,cls.__mid,cls.__mkind,cls.__mdtype,False)
      else:
         raise ValueError("predict is called before calling fit, or the model is released.")

   def load(cls,fname,dtype=None):
      cls.release()
      cls.__mid = ModelID.get()
      if dtype is None: 
        if cls.__mdtype is None:
          raise TypeError("model type should be specified for loading from file!")
      else: cls.__mdtype = TypeUtil.to_id_dtype(dtype)
      GLM.load(cls.__mid,cls.__mkind,cls.__mdtype,fname)
      return cls

   def save(cls,fname):
      if cls.__mid is not None: GLM.save(cls.__mid,cls.__mkind,cls.__mdtype,fname)

   def debug_print(cls):
      if cls.__mid is not None: GLM.debug_print(cls.__mid,cls.__mkind,cls.__mdtype)

   def release(cls):
      if cls.__mid is not None:
         GLM.release(cls.__mid,cls.__mkind,cls.__mdtype)
         cls.__mid = None

   def __del__(cls):
      if FrovedisServer.isUP(): cls.release()

