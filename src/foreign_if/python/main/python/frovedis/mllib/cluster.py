#!/usr/bin/env python

from .model_util import *
from ..exrpc.server import *
from ..exrpc.rpclib import *
from ..matrix.ml_data import FrovedisFeatureData
from ..matrix.dtype import TypeUtil
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
    cls.__mdtype = None
    cls.__mkind = M_KIND.KMEANS

  def fit(cls, X, y=None):
    cls.release()
    cls.__mid = ModelID.get()
    eps = 0.01
    seed = 0
    if cls.random_state is not None: seed = cls.random_state
    # if X is not a sparse data, it would be loaded as rowmajor matrix
    inp_data = FrovedisFeatureData(X,dense_kind='rowmajor')
    X = inp_data.get()
    dtype = inp_data.get_dtype()
    itype = inp_data.get_itype()
    dense = inp_data.is_dense()
    cls.__mdtype = dtype

    (host,port) = FrovedisServer.getServerInstance()
    rpclib.kmeans_train(host,port,X.get(),cls.n_clusters,
                        cls.max_iter,seed,eps,cls.verbose,cls.__mid,
                        dtype, itype, dense)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 
    return cls

  def predict(cls, X):
    if cls.__mid is not None:
       # if X is not a sparse data, it would be loaded as rowmajor matrix
       inp_data = FrovedisFeatureData(X,dense_kind='rowmajor')
       X = inp_data.get()
       dtype = inp_data.get_dtype()
       itype = inp_data.get_itype()
       dense = inp_data.is_dense()
       if (dtype != cls.__mdtype):
         raise TypeError("Input test data dtype is different than model dtype!")
       (host,port) = FrovedisServer.getServerInstance()
       len = X.numRows()
       ret = np.zeros(len,dtype=np.int32)
       rpclib.parallel_kmeans_predict(host,port,cls.__mid,
                                      cls.__mdtype,X.get(),ret,len,
                                      itype,dense)
       excpt = rpclib.check_server_exception()
       if excpt["status"]: raise RuntimeError(excpt["info"]) 
       return ret
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


