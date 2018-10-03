#!/usr/bin/env python

from model_util import *
from ..exrpc.server import *
from ..exrpc.rpclib import *
from ..matrix.crs import FrovedisCRSMatrix
from ..matrix.dtype import DTYPE
import numpy as np

class ALS:
  "A python wrapper of Frovedis ALS"

  def __init__(cls, max_iter=100, alpha=0.01, regParam=0.01, 
               seed=0, verbose=0):
     cls.max_iter = max_iter
     cls.alpha = alpha
     cls.regParam = regParam
     cls.seed = seed
     cls.verbose = verbose
     # extra
     cls.rank = 0
     cls.nrow = 0
     cls.ncol = 0
     cls.__mid = None
     cls.__mdtype = None
     cls.__mkind = M_KIND.MFM

  def fit(cls,X,rank):
     cls.release()
     X = FrovedisCRSMatrix.asCRS(X)
     dtype = X.get_dtype()
     itype = X.get_itype()
     if (dtype != DTYPE.FLOAT and dtype != DTYPE.DOUBLE):
       raise TypeError("Expected training data either of float or double type!")
     else:
       cls.__mdtype = dtype
     if (itype != DTYPE.INT and itype != DTYPE.LONG):
       raise TypeError("Expected training sparse data itype to be either int or long!")
     if rank <= 0: raise ValueError("Rank must be a possitive integer")
     cls.rank = rank
     cls.nrow = X.numRows()
     cls.ncol = X.numCols()
     cls.__mid = ModelID.get()
     (host,port) = FrovedisServer.getServerInstance()
     rpclib.als_train(host,port,X.get(),rank,cls.max_iter,cls.alpha,
                      cls.regParam,cls.seed,cls.verbose,cls.__mid,dtype,itype)
     excpt = rpclib.check_server_exception()
     if excpt["status"]: raise RuntimeError(excpt["info"]) 
     return cls

  def predict(cls,ids):
     if cls.__mid is not None:
        if isinstance(ids,(list,tuple)) == False: 
           raise TypeError("Expected: List, Got: " + str(type(ids)))
        ids = np.asarray(ids,dtype=np.int32).flatten()
        sz = (ids.size / 2)
        (host,port) = FrovedisServer.getServerInstance()
        if cls.__mdtype is None: raise TypeError("input model for predict is typeless!")
        if(cls.__mdtype == DTYPE.FLOAT):
          ret = np.zeros(sz,dtype=np.float32)
          rpclib.als_float_predict(host,port,cls.__mid,ids,ret,sz)
        elif(cls.__mdtype == DTYPE.DOUBLE):
          ret = np.zeros(sz,dtype=np.float64)
          rpclib.als_double_predict(host,port,cls.__mid,ids,ret,sz)
        else: raise TypeError("model type can be either float or double!")
        excpt = rpclib.check_server_exception()
        if excpt["status"]: raise RuntimeError(excpt["info"]) 
        return ret
     else:
        raise ValueError("predict is called before calling fit, or the model is released.")

  def recommend_users(cls,pid,k):
     if cls.__mid is not None:  
        if k > cls.nrow: k = cls.nrow
        (host,port) = FrovedisServer.getServerInstance()
        uids = np.zeros(k,dtype=np.int32)
        if cls.__mdtype is None: raise TypeError("input model for recommend is typeless!")
        if(cls.__mdtype == DTYPE.FLOAT):
          scores = np.zeros(k,dtype=np.float32)
          rpclib.als_float_rec_users(host,port,cls.__mid,pid,k,uids,scores)
        elif(cls.__mdtype == DTYPE.DOUBLE):
          scores = np.zeros(k,dtype=np.float64)
          rpclib.als_double_rec_users(host,port,cls.__mid,pid,k,uids,scores)
        else: raise TypeError("model type can be either float or double!")
        excpt = rpclib.check_server_exception()
        if excpt["status"]: raise RuntimeError(excpt["info"]) 
        return [(i,j) for i,j in zip(uids,scores)]
     else:
        raise ValueError("recommend_users is called before calling fit, or the model is released.")
        
  def recommend_products(cls,uid,k):
     if cls.__mid is not None:
        if k > cls.ncol: k = cls.ncol
        (host,port) = FrovedisServer.getServerInstance()
        pids = np.zeros(k,dtype=np.int32)
        if cls.__mdtype is None: raise TypeError("input model for recommend is typeless!")
        if(cls.__mdtype == DTYPE.FLOAT):
          scores = np.zeros(k,dtype=np.float32)
          rpclib.als_float_rec_prods(host,port,cls.__mid,uid,k,pids,scores)
        elif(cls.__mdtype == DTYPE.DOUBLE):
          scores = np.zeros(k,dtype=np.float64)
          rpclib.als_double_rec_prods(host,port,cls.__mid,uid,k,pids,scores)
        else: raise TypeError("model type can be either float or double!")
        excpt = rpclib.check_server_exception()
        if excpt["status"]: raise RuntimeError(excpt["info"]) 
        return [(i,j) for i,j in zip(pids,scores)]
     else:
        raise ValueError("recommend_products is called before calling fit, or the model is released.")

  def load(cls,fname,dtype=None):
     cls.release()
     if isinstance(fname, str) == False: 
        raise TypeError("Expected: String, Got: " + str(type(fname)))
     if dtype is None:
       if cls.__mdtype is None:
         raise TypeError("model type should be specified for loading from file!")
     else: cls.__mdtype = TypeUtil.to_id_dtype(dtype)
     cls.__mid = ModelID.get()
     (host,port) = FrovedisServer.getServerInstance()
     ret = rpclib.load_frovedis_mfm(host,port,cls.__mid,cls.__mdtype,fname)
     excpt = rpclib.check_server_exception()
     if excpt["status"]: raise RuntimeError(excpt["info"]) 
     cls.rank = ret['rank']
     cls.nrow = ret['nrow']
     cls.ncol = ret['ncol']

  def save(cls,fname):
     if cls.__mid is not None: GLM.save(cls.__mid,cls.__mkind,cls.__mdtype,fname)

  def debug_print(cls):
     if cls.__mid is not None: GLM.debug_print(cls.__mid,cls.__mkind,cls.__mdtype)

  def release(cls):
     if cls.__mid is not None:
        GLM.release(cls.__mid,cls.__mkind,cls.__mdtype)
        cls.rank = 0
        cls.nrow = 0
        cls.ncol = 0
        cls.__mid = None

  def __del__(cls):
    if FrovedisServer.isUP(): cls.release()
