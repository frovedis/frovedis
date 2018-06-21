#!/usr/bin/env python

from model_util import *
from ..exrpc.server import *
from ..exrpc.rpclib import *
from ..matrix.sparse import FrovedisCRSMatrix
import numpy as np

class KMeans:
  "A python wrapper of Frovedis kmeans"

  def __init__(cls, n_clusters=8, init='k-means++', n_init=10,
               max_iter=300, tol=1e-4, precompute_distances='auto',
               verbose=0, random_state=None, copy_x=True,
               n_jobs=1, algorithm='auto'):
    cls.n_clusters = n_clusters
    cls.init = init
    cls.max_iter = max_iter
    cls.tol = tol
    cls.precompute_distances = precompute_distances
    cls.n_init = n_init
    cls.verbose = verbose
    cls.random_state = random_state
    cls.copy_x = copy_x
    cls.n_jobs = n_jobs
    cls.algorithm = algorithm
    #extra
    cls.__mid = None
    cls.__mkind = M_KIND.KMEANS

  def fit(cls, X, y=None):
    cls.release()
    cls.__mid = ModelID.get()
    X = FrovedisCRSMatrix.asCRS(X)
    eps = 0.01
    seed = 0
    if cls.random_state is not None: seed = cls.random_state
    (host,port) = FrovedisServer.getServerInstance()
    rpclib.kmeans_train(host,port,X.get(),cls.n_clusters,
                        cls.max_iter,seed,eps,cls.verbose,cls.__mid)
    return cls

  def predict(cls, X):
    if cls.__mid is not None:
       X = FrovedisCRSMatrix.asCRS(X)
       (host,port) = FrovedisServer.getServerInstance()
       len = X.numRows()
       ret = np.zeros(len,dtype=np.int32)
       rpclib.parallel_kmeans_predict(host,port,cls.__mid,cls.__mkind,
                                      X.get(),ret,len)
       return ret
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
       #print("Frovedis Kmeans model with " + str(cls.__mid) + " is released")
       cls.__mid = None

  def __del__(cls):
    if FrovedisServer.isUP(): cls.release()


