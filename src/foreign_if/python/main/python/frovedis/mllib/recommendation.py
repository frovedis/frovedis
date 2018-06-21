#!/usr/bin/env python

from model_util import *
from ..exrpc.server import *
from ..exrpc.rpclib import *
from ..matrix.sparse import FrovedisCRSMatrix
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
     cls.__mkind = M_KIND.MFM

  def fit(cls,X,rank):
     cls.release()
     X = FrovedisCRSMatrix.asCRS(X)
     if rank <= 0: raise ValueError, "Rank must be a possitive integer"
     cls.rank = rank
     cls.nrow = X.numRows()
     cls.ncol = X.numCols()
     cls.__mid = ModelID.get()
     (host,port) = FrovedisServer.getServerInstance()
     rpclib.als_train(host,port,X.get(),rank,cls.max_iter,cls.alpha,
                      cls.regParam,cls.seed,cls.verbose,cls.__mid)
     return cls

  def predict(cls,ids):
     if cls.__mid is not None:
        if isinstance(ids,(list,tuple)) == False: 
           raise TypeError, "Expected: List, Got: " + str(type(ids))
        ids = np.asarray(ids,dtype=np.int32).flatten()
        sz = (ids.size / 2)
        ret = np.zeros(sz,dtype=np.float64)
        (host,port) = FrovedisServer.getServerInstance()
        rpclib.als_predict(host,port,cls.__mid,ids,ret,sz)
        return ret
     else:
        raise ValueError, "predict is called before calling fit, or the model is released."

  def recommend_users(cls,pid,k):
     if cls.__mid is not None:  
        if k > cls.nrow: k = cls.nrow
        uids = np.zeros(k,dtype=np.int32)
        scores = np.zeros(k,dtype=np.float64)
        (host,port) = FrovedisServer.getServerInstance()
        rpclib.als_rec_users(host,port,cls.__mid,pid,k,uids,scores)
        return [(i,j) for i,j in zip(uids,scores)]
     else:
        raise ValueError, "recommend_users is called before calling fit, or the model is released."
        
  def recommend_products(cls,uid,k):
     if cls.__mid is not None:
        if k > cls.ncol: k = cls.ncol
        pids = np.zeros(k,dtype=np.int32)
        scores = np.zeros(k,dtype=np.float64)
        (host,port) = FrovedisServer.getServerInstance()
        rpclib.als_rec_prods(host,port,cls.__mid,uid,k,pids,scores)
        return [(i,j) for i,j in zip(pids,scores)]
     else:
        raise ValueError, "recommend_products is called before calling fit, or the model is released."

  def load(cls,fname):
     cls.release()
     if isinstance(fname, str) == False: 
        raise TypeError, "Expected: String, Got: " + str(type(gname))
     cls.__mid = ModelID.get()
     (host,port) = FrovedisServer.getServerInstance()
     ret = rpclib.load_frovedis_mfm(host,port,cls.__mid,cls.__mkind,fname)
     cls.rank = ret['rank']
     cls.nrow = ret['nrow']
     cls.ncol = ret['ncol']

  def save(cls,fname):
     if cls.__mid is not None: GLM.save(cls.__mid,cls.__mkind,fname)

  def debug_print(cls):
     if cls.__mid is not None: GLM.debug_print(cls.__mid,cls.__mkind)

  def release(cls):
     if cls.__mid is not None:
        GLM.release(cls.__mid,cls.__mkind)
        #print("Frovedis matrix factorization model with " + str(cls.__mid) + " is released")
        cls.rank = 0
        cls.nrow = 0
        cls.ncol = 0
        cls.__mid = None

  def __del__(cls):
    if FrovedisServer.isUP(): cls.release()

 
