#!/usr/bin/env python

from .model_util import *
from ..exrpc.server import *
from ..exrpc.rpclib import *
from ..matrix.ml_data import FrovedisFeatureData
from ..matrix.dense import FrovedisRowmajorMatrix
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


class SpectralClustering:
  "A python wrapper of Frovedis Spectral clustering"

  def __init__(cls, n_clusters=8, eigen_solver=None, random_state=None, n_init=10,
        gamma=1.0, affinity='rbf', n_neighbors=10, eigen_tol=0.0, assign_labels='kmeans',
        degree=3, coef0=1, kernel_params=None, n_jobs=None,
        verbose=0, n_iter=100, eps=0.01, n_comp=None, norm_laplacian=True,
        mode=1, drop_first=False):

    cls.n_clusters = n_clusters
    cls.eigen_solver = eigen_solver
    cls.random_state = random_state
    cls.n_init = n_init
    cls.gamma = gamma
    cls.affinity = affinity
    cls.n_neighbors = n_neighbors
    cls.eigen_tol = eigen_tol
    cls.assign_labels = assign_labels
    cls.degree = degree
    cls.coef0 = coef0
    cls.kernel_params = kernel_params
    cls.n_jobs = n_jobs
    cls.verbose = verbose
    #extra
    cls.__mid = None
    cls.__mdtype = None
    cls.__mkind = M_KIND.SCM
    cls.n_iter = n_iter
    cls.eps = eps
    cls.n_comp = n_clusters if n_comp is None else n_comp
    cls.norm_laplacian = norm_laplacian
    cls.mode = mode
    cls.drop_first = drop_first

  def fit(cls, X, y=None):
    cls.release()
    cls.__mid = ModelID.get()
    # if X is not a sparse data, it would be loaded as rowmajor matrix
    inp_data = FrovedisFeatureData(X,dense_kind='rowmajor')
    X = inp_data.get()
    dtype = inp_data.get_dtype()
    itype = inp_data.get_itype()
    dense = inp_data.is_dense()
    if (cls.affinity == "precomputed"): precomputed = True
    else: precomputed = False
    cls.__mdtype = dtype
    len = X.numRows()
    (host,port) = FrovedisServer.getServerInstance()
    ret = np.zeros(len,dtype=np.int32)
    rpclib.sca_train(host,port,X.get(),cls.n_clusters,cls.n_comp,
                     cls.n_iter,cls.eps,cls.gamma,
                     precomputed,cls.norm_laplacian,cls.mode,cls.drop_first,
                     ret,len,cls.verbose,cls.__mid,dtype,itype,dense)
    cls.labels_ = ret
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
    return cls

  def fit_predict(cls, X, y=None):
    cls.fit(X,y)
    return cls.labels_

  def get_params(cls):
    d = { 'n_clusters': cls.n_clusters,
    'eigen_solver' : cls.eigen_solver,
    'random_state' : cls.random_state,
    'n_init' : cls.n_init,
    'gamma' : cls.gamma,
    'affinity' : cls.affinity,
    'n_neighbors' : cls.n_neighbors,
    'eigen_tol' : cls.eigen_tol,
    'assign_labels' : cls.assign_labels,
    'degree' : cls.degree,
    'coef0' : cls.coef0,
    'kernel_params' : cls.kernel_params,
    'n_jobs' : cls.n_jobs,
    'n_iter' : cls.n_iter,
    'eps' : cls.eps,
    'n_comp' : cls.n_comp,
    'norm_laplacian' : cls.norm_laplacian, 
    'mode' : cls.mode,
    'drop_first' : cls.drop_first }
    return d

  def __str__(cls):
    return str(cls.get_params())

  def set_params(cls, **params):
    d = cls.get_params()
    valid_params = set(d.keys())
    given_params = set(params.keys())
    if given_params <= valid_params:
      #print "Valid params"
      extra_params = {'mid': cls.__mid,
                      'mdtype': cls.__mdtype,
                      'mkind': cls.__mkind }

      cls.__init__(**params)
      cls.__mid = extra_params['mid']
      cls.__mdtype = extra_params['mdtype']
      cls.__mkind = extra_params['mkind']
      return cls
    else:
      raise ValueError("Invalid parameters passed")


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
    cls.labels_ = rpclib.load_frovedis_scm(host,port,cls.__mid,cls.__mdtype,fname.encode('ascii'))
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
    return cls

  def get_affinity_matrix(cls):
    if cls.__mid is not None:
      (host,port) = FrovedisServer.getServerInstance()
      dmat = rpclib.get_scm_affinity_matrix(host,port,cls.__mid,cls.__mdtype)
      return FrovedisRowmajorMatrix(mat=dmat, dtype=TypeUtil.to_numpy_dtype(cls.__mdtype))

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


class SpectralEmbedding:
  "A python wrapper of Frovedis Spectral clustering"

  def __init__(cls, n_components=2, affinity='nearest_neighbors', gamma=1.0,
               random_state=None, eigen_solver=None, n_neighbors=None, n_jobs=None,
               verbose=0, norm_laplacian=True, mode=1, drop_first=True):
    cls.n_components = n_components
    cls.affinity = affinity
    cls.gamma = gamma
    cls.random_state = random_state
    cls.eigen_solver = eigen_solver
    cls.n_neighbors = n_neighbors
    cls.n_jobs = n_jobs
    cls.verbose = verbose
    #extra
    cls.__mid = None
    cls.__mdtype = None
    cls.__mkind = M_KIND.SEM
    cls.norm_laplacian = norm_laplacian
    cls.mode = mode
    cls.drop_first = drop_first
 

  def fit(cls, X, y=None):
    cls.release()
    cls.__mid = ModelID.get()
    # if X is not a sparse data, it would be loaded as rowmajor matrix
    inp_data = FrovedisFeatureData(X,dense_kind='rowmajor')
    X = inp_data.get()
    dtype = inp_data.get_dtype()
    itype = inp_data.get_itype()
    dense = inp_data.is_dense()
    cls.__mdtype = dtype
    if (cls.affinity == "precomputed"): precomputed = True
    else: precomputed = False
    (host,port) = FrovedisServer.getServerInstance()
    rpclib.sea_train(host,port,X.get(),
                     cls.n_components, cls.gamma,
                     precomputed,cls.norm_laplacian,cls.mode,
                     cls.drop_first,cls.verbose,cls.__mid,dtype,
                     itype, dense)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
    return cls

  def get_params(cls):
    d = { 'n_components': cls.n_components,
    'affinity' : cls.affinity,
    'gamma' : cls.gamma,
    'random_state' : cls.random_state,
    'eigen_solver' : cls.eigen_solver,
    'n_neighbors' : cls.n_neighbors,
    'n_jobs' : cls.n_jobs,
    'norm_laplacian' : cls.norm_laplacian,
    'mode' : cls.mode,
    'drop_first' : cls.drop_first 
    }
    return d

  def __str__(cls):
    return str(cls.get_params())

  def set_params(cls, **params):
    d = cls.get_params()
    valid_params = set(d.keys())
    given_params = set(params.keys())
    if given_params <= valid_params:
      #print "Valid params"
      extra_params = {'mid': cls.__mid,
                      'mdtype': cls.__mdtype,
                      'mkind': cls.__mkind }

      cls.__init__(**params)
      cls.__mid = extra_params['mid']
      cls.__mdtype = extra_params['mdtype']
      cls.__mkind = extra_params['mkind']
      return cls
    else:
      raise ValueError("Invalid parameters passed")

  def get_affinity_matrix(cls):
    if cls.__mid is not None:
      (host,port) = FrovedisServer.getServerInstance()
      dmat = rpclib.get_sem_affinity_matrix(host,port,cls.__mid,cls.__mdtype)
      return FrovedisRowmajorMatrix(mat=dmat, dtype=TypeUtil.to_numpy_dtype(cls.__mdtype))

  def get_embedding_matrix(cls):
    if cls.__mid is not None:
      (host,port) = FrovedisServer.getServerInstance()
      dmat = rpclib.get_sem_embedding_matrix(host,port,cls.__mid,cls.__mdtype)
      return FrovedisRowmajorMatrix(mat=dmat, dtype=TypeUtil.to_numpy_dtype(cls.__mdtype))

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


class AgglomerativeClustering:
  "A python wrapper of Frovedis Agglomerative Clustering"

  def __init__(cls, n_clusters=2, affinity='euclidean', memory=None,
               connectivity=None, compute_full_tree='auto', linkage='average',
               pooling_func='deprecated', verbose=0):
    cls.n_clusters = n_clusters
    cls.affinity = affinity
    cls.memory = memory
    cls.connectivity = connectivity
    cls.compute_full_tree = compute_full_tree
    cls.linkage = linkage
    cls.verbose = verbose
    cls.pooling_func = pooling_func
    # extra
    cls.__mid = None
    cls.__mdtype = None
    cls.__mkind = M_KIND.ACM

  def fit(cls, X, y=None):
    cls.release()
    cls.__mid = ModelID.get()
    inp_data = FrovedisFeatureData(X,dense_kind='rowmajor')
    X = inp_data.get()
    dtype = inp_data.get_dtype()
    itype = inp_data.get_itype()
    dense = inp_data.is_dense()
    cls.__mdtype = dtype
    nsamples = X.numRows()
    (host,port) = FrovedisServer.getServerInstance()
    ret = np.zeros(nsamples,dtype=np.int32)
    rpclib.aca_train(host,port,X.get(),cls.n_clusters,
                     cls.linkage,ret,nsamples,cls.verbose,cls.__mid,
                     dtype,itype,dense)
    cls.labels_ = ret
    cls.__nsamples = nsamples
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
    return cls

  def fit_predict(cls, X, y=None):
    cls.fit(X,y)
    return cls.labels_

  # added for predicting with different nclusters on same model
  def predict(cls, ncluster=None):
    if ncluster is not None: cls.n_clusters = ncluster
    (host,port) = FrovedisServer.getServerInstance()
    ret = np.zeros(cls.__nsamples,dtype=np.int32)
    rpclib.acm_predict(host,port,cls.__mid,cls.__mdtype,
                       cls.n_clusters,ret,cls.__nsamples)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
    cls.labels_ = ret
    return cls.labels_

  def get_params(cls):
    d = {'n_clusters': cls.n_clusters,
         'affinity': cls.affinity,
         'memory': cls.memory,
         'connectivity': cls.connectivity,
         'compute_full_tree': cls.compute_full_tree,
         'linkage': cls.linkage,
         'pooling_func': cls.pooling_func,
         'verbose': cls.verbose}
    return d

  def __str__(cls):
    return str(cls.get_params())

  def set_params(cls, **params):
    d = cls.get_params()
    valid_params = set(d.keys())
    given_params = set(params.keys())
    if given_params <= valid_params:
      #print "Valid params"
      extra_params = {'mid': cls.__mid,
                      'mdtype': cls.__mdtype,
                      'mkind': cls.__mkind }
      cls.__init__(**params)
      cls.__mid = extra_params['mid']
      cls.__mdtype = extra_params['mdtype']
      cls.__mkind = extra_params['mkind']
      return cls
    else:
      raise ValueError("Invalid parameters passed")

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
    cls.__nsamples = rpclib.load_frovedis_acm(host,port,cls.__mid,cls.__mdtype,fname.encode('ascii'))
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
    # get labels
    ret = np.zeros(cls.__nsamples,dtype=np.int32)
    rpclib.acm_predict(host,port,cls.__mid,cls.__mdtype,
                       cls.n_clusters,ret,cls.__nsamples)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
    cls.labels_ = ret
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


