"""
cluster.py: module containing wrapper for kmeans, agglomerative
            and spectral clustering
"""
#!/usr/bin/env python

import os.path
import pickle
from .model_util import *
from ..base import *
from ..exrpc.server import FrovedisServer
from ..exrpc import rpclib
from ..matrix.ml_data import FrovedisFeatureData
from ..matrix.dense import FrovedisRowmajorMatrix
from ..matrix.dtype import TypeUtil
import numpy as np

def clustering_score(labels_true, labels_pred):
  try:
      from sklearn.metrics.cluster import homogeneity_score
      return homogeneity_score(labels_true, labels_pred)
  except: #for system without sklearn
      raise AttributeError("score: needs scikit-learn to use this method!")

class KMeans(BaseEstimator):
    """
    A python wrapper of Frovedis kmeans
    """
    def __init__(self, n_clusters=8, init='k-means++', n_init=10,
                 max_iter=300, tol=1e-4, precompute_distances='auto',
                 verbose=0, random_state=None, copy_x=True,
                 n_jobs=1, algorithm='auto'):
        self.n_clusters = n_clusters
        self.init = init
        self.max_iter = max_iter
        self.tol = tol
        self.precompute_distances = precompute_distances
        self.n_init = n_init
        self.verbose = verbose
        self.random_state = random_state
        self.copy_x = copy_x
        self.n_jobs = n_jobs
        self.algorithm = algorithm
        #extra
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.KMEANS

    def fit(self, X, y=None, sample_weight=None):
        """Compute k-means clustering."""
        self.release()
        self.__mid = ModelID.get()
        eps = 0.01
        seed = 0
        if self.random_state is not None:
            seed = self.random_state
        # if X is not a sparse data, it would be loaded as rowmajor matrix
        inp_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        X = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        self.__mdtype = dtype

        (host, port) = FrovedisServer.getServerInstance()
        rpclib.kmeans_train(host, port, X.get(), self.n_clusters,\
                            self.max_iter, seed, eps, self.verbose, \
                            self.__mid, dtype, itype, dense)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    def predict(self, X, sample_weight=None):
        """Predict the closest cluster each sample in X belongs to."""
        if self.__mid is not None:
            # if X is not a sparse data, it would be loaded as
            #rowmajor matrix
            inp_data = FrovedisFeatureData(X, dense_kind='rowmajor')
            X = inp_data.get()
            dtype = inp_data.get_dtype()
            itype = inp_data.get_itype()
            dense = inp_data.is_dense()
            if dtype != self.__mdtype:
                raise TypeError( \
                "Input test data dtype is different than model dtype!")
            (host, port) = FrovedisServer.getServerInstance()
            len_l = X.numRows()
            ret = np.zeros(len_l, dtype=np.int32)
            rpclib.parallel_kmeans_predict(host, port, self.__mid,
                                           self.__mdtype, X.get(),
                                           ret, len_l, itype, dense)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return ret
        else:
            raise ValueError( \
            "predict is called before calling fit, or the model is released.")

    def fit_predict(self, X, y=None, sample_weight=None):
        """Compute cluster centers and predict cluster index for each sample"""
        return self.fit(X, y, sample_weight).predict(X, sample_weight)

    def score(self, X, y, sample_weight=None):
        """uses scikit-learn homogeneity_score for scoring"""
        if self.__mid is not None:
            #TODO: implement inertia
            return clustering_score(y, self.predict(X, sample_weight))

    def load(self, fname, dtype=None):
        """
        NAME: load
        """
        if not os.path.exists(fname):
            raise ValueError(\
                "the model with name %s does not exist!" % fname)
        self.release()
        metadata = open(fname+"/metadata", "rb")
        self.n_clusters, self.__mkind, self.__mdtype = pickle.load(metadata)
        metadata.close()
        if dtype is not None:
            mdt = TypeUtil.to_numpy_dtype(self.__mdtype)
            if dtype != mdt:
                raise ValueError("load: type mismatches detected!" + \
                                 "expected type: " + str(mdt) + \
                                 "; given type: " + str(dtype))
        self.__mid = ModelID.get()
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
        return self

    def save(self, fname):
        """
        NAME: save
        """
        if self.__mid is not None:
            if os.path.exists(fname):
                raise ValueError(\
                    "another model with %s name already exists!" % fname)
            else:
                os.makedirs(fname)
            GLM.save(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
            metadata = open(fname+"/metadata", "wb")
            pickle.dump((self.n_clusters, self.__mkind, self.__mdtype), \
                            metadata)
            metadata.close()
        else:
            raise ValueError(\
                "save: the requested model might have been released!")

    def debug_print(self):
        """
        NAME: debug_print
        """
        if self.__mid is not None:
            GLM.debug_print(self.__mid, self.__mkind, self.__mdtype)

    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, self.__mdtype)
            self.__mid = None

    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
            self.release()

class SpectralClustering(BaseEstimator):
    """
    A python wrapper of Frovedis Spectral clustering
    """
    def __init__(self, n_clusters=8, eigen_solver=None, n_components=None, 
                 random_state=None, n_init=10, gamma=1.0, affinity='rbf', 
                 n_neighbors=10, eigen_tol=0.0, assign_labels='kmeans', 
                 degree=3, coef0=1, kernel_params=None, n_jobs=None, 
                 verbose=0, n_iter=100, eps=0.01, norm_laplacian=True, 
                 mode=1, drop_first=False):
        self.n_clusters = n_clusters
        self.eigen_solver = eigen_solver
        self.n_components = n_clusters if n_components is None else n_components
        self.random_state = random_state
        self.n_init = n_init
        self.gamma = gamma
        self.affinity = affinity
        self.n_neighbors = n_neighbors
        self.eigen_tol = eigen_tol
        self.assign_labels = assign_labels
        self.degree = degree
        self.coef0 = coef0
        self.kernel_params = kernel_params
        self.n_jobs = n_jobs
        self.verbose = verbose
        #extra
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.SCM
        self.n_iter = n_iter
        self.eps = eps
        self.norm_laplacian = norm_laplacian
        self.mode = mode
        self.drop_first = drop_first
        self.labels_ = None

    def fit(self, X, y=None):
        """
        NAME: fit
        """
        self.release()
        self.__mid = ModelID.get()
        # if X is not a sparse data, it would be loaded as rowmajor matrix
        inp_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        X = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        if self.affinity == "precomputed":
            precomputed = True
        else:
            precomputed = False
        self.__mdtype = dtype
        len_l = X.numRows()
        (host, port) = FrovedisServer.getServerInstance()
        ret = np.zeros(len_l, dtype=np.int32)
        rpclib.sca_train(host, port, X.get(), self.n_clusters, self.n_components,
                         self.n_iter, self.eps, self.gamma,
                         precomputed, self.norm_laplacian, self.mode,
                         self.drop_first, ret, len_l, self.verbose, self.__mid,
                         dtype, itype, dense)
        self.labels_ = ret
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    def fit_predict(self, X, y=None):
        """
        NAME: fit_predict
        """
        self.fit(X, y)
        return self.labels_

    def score(self, X, y, sample_weight=None):
        """uses scikit-learn homogeneity_score for scoring"""
        if self.__mid is not None:
            return clustering_score(y, self.fit_predict(X, y))

    def __str__(self):
        """
        NAME: __str__
        """
        return str(self.get_params())

    def load(self, fname, dtype=None):
        """
        NAME: load
        """
        if isinstance(fname, str) == False:
            raise TypeError("Expected: String, Got: " + str(type(fname)))
        if not os.path.exists(fname):
            raise ValueError(\
                "the model with name %s does not exist!" % fname)
        self.release()
        metadata = open(fname+"/metadata", "rb")
        self.n_clusters, self.n_components, self.__mkind, self.__mdtype = \
            pickle.load(metadata)
        metadata.close()
        if dtype is not None:
            mdt = TypeUtil.to_numpy_dtype(self.__mdtype)
            if dtype != mdt:
                raise ValueError("load: type mismatches detected!" + \
                                 "expected type: " + str(mdt) + \
                                 "; given type: " + str(dtype))
        self.__mid = ModelID.get()
        (host, port) = FrovedisServer.getServerInstance()
        model_file = fname + "/model"
        self.labels_ = rpclib.load_frovedis_scm(host, port, self.__mid, \
                        self.__mdtype, model_file.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    def get_affinity_matrix(self):
        """
        NAME: get_affinity_matrix
        """
        if self.__mid is not None:
            (host, port) = FrovedisServer.getServerInstance()
            dmat = rpclib.get_scm_affinity_matrix(host, port, self.__mid, \
                                                    self.__mdtype)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return FrovedisRowmajorMatrix(mat=dmat, dtype=TypeUtil. \
                        to_numpy_dtype(self.__mdtype))

    def save(self, fname):
        """
        NAME: save
        """
        if self.__mid is not None:
            if os.path.exists(fname):
                raise ValueError(\
                    "another model with %s name already exists!" % fname)
            else:
                os.makedirs(fname)
            GLM.save(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
            metadata = open(fname+"/metadata", "wb")
            pickle.dump((self.n_clusters, self.n_components, \
                self.__mkind, self.__mdtype), metadata)
            metadata.close()
        else:
            raise ValueError(\
                "save: the requested model might have been released!")

    def debug_print(self):
        """
        NAME: debug_print
        """
        if self.__mid is not None:
            GLM.debug_print(self.__mid, self.__mkind, self.__mdtype)

    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, self.__mdtype)
            self.__mid = None
            self.labels_ = None

    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
            self.release()

class SpectralEmbedding(BaseEstimator):
    """
    A python wrapper of Frovedis Spectral clustering
    """
    def __init__(self, n_components=2, affinity='nearest_neighbors', gamma=1.0,
                 random_state=None, eigen_solver=None, n_neighbors=None,
                 n_jobs=None, verbose=0, norm_laplacian=True, mode=1,
                 drop_first=True):
        self.n_components = n_components
        self.affinity = affinity
        self.gamma = gamma
        self.random_state = random_state
        self.eigen_solver = eigen_solver
        self.n_neighbors = n_neighbors
        self.n_jobs = n_jobs
        self.verbose = verbose
        #extra
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.SEM
        self.norm_laplacian = norm_laplacian
        self.mode = mode
        self.drop_first = drop_first

    def fit(self, X, y=None):
        """
        NAME: fir
        """
        self.release()
        self.__mid = ModelID.get()
        # if X is not a sparse data, it would be loaded as rowmajor matrix
        inp_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        X = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        self.__mdtype = dtype
        if self.affinity == "precomputed":
            precomputed = True
        else:
            precomputed = False
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.sea_train(host, port, X.get(),
                         self.n_components, self.gamma,
                         precomputed, self.norm_laplacian, self.mode,
                         self.drop_first, self.verbose, self.__mid, dtype,
                         itype, dense)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    def __str__(self):
        """
        NAME: __str__
        """
        return str(self.get_params())

    def get_affinity_matrix(self):
        """
        NAME: get_affinity_matrix
        """
        if self.__mid is not None:
            (host, port) = FrovedisServer.getServerInstance()
            dmat = rpclib.get_sem_affinity_matrix(host, port, self.__mid, \
                                                    self.__mdtype)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return FrovedisRowmajorMatrix(mat=dmat, dtype=TypeUtil. \
                        to_numpy_dtype(self.__mdtype))

    def get_embedding_matrix(self):
        """
        NAME: get_embedding_matrix
        """
        if self.__mid is not None:
            (host, port) = FrovedisServer.getServerInstance()
            dmat = rpclib.get_sem_embedding_matrix(host, port, self.__mid, \
                                                            self.__mdtype)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return FrovedisRowmajorMatrix(mat=dmat, dtype=TypeUtil. \
                             to_numpy_dtype(self.__mdtype))

    def load(self, fname, dtype=None):
        """
        NAME: load
        """
        if not os.path.exists(fname):
            raise ValueError(\
                "the model with name %s does not exist!" % fname)
        self.release()
        metadata = open(fname+"/metadata", "rb")
        self.n_components, self.__mkind, self.__mdtype = pickle.load(metadata)
        metadata.close()
        if dtype is not None:
            mdt = TypeUtil.to_numpy_dtype(self.__mdtype)
            if dtype != mdt:
                raise ValueError("load: type mismatches detected!" + \
                                 "expected type: " + str(mdt) + \
                                 "; given type: " + str(dtype))
        self.__mid = ModelID.get()
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
        return self

    def save(self, fname):
        """
        NAME: save
        """
        if self.__mid is not None:
            if os.path.exists(fname):
                raise ValueError(\
                    "another model with %s name already exists!" % fname)
            else:
                os.makedirs(fname)
            GLM.save(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
            metadata = open(fname+"/metadata", "wb")
            pickle.dump((self.n_components, self.__mkind, \
                self.__mdtype), metadata)
            metadata.close()
        else:
            raise ValueError(\
                "save: the requested model might have been released!")

    def debug_print(self):
        """
        NAME: debug_print
        """
        if self.__mid is not None:
            GLM.debug_print(self.__mid, self.__mkind, self.__mdtype)

    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, self.__mdtype)
            self.__mid = None

    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
            self.release()

class AgglomerativeClustering(BaseEstimator):
    """
    A python wrapper of Frovedis Agglomerative Clustering
    """
    def __init__(self, n_clusters=2, affinity='euclidean', memory=None,
                 connectivity=None, compute_full_tree='auto',
                 linkage='average', pooling_func='deprecated', verbose=0):
        self.n_clusters = n_clusters
        self.affinity = affinity
        self.memory = memory
        self.connectivity = connectivity
        self.compute_full_tree = compute_full_tree
        self.linkage = linkage
        self.verbose = verbose
        self.pooling_func = pooling_func
        # extra
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.ACM
        self.__nsamples = None
        self.labels_ = None

    def fit(self, X, y=None):
        """
        NAME: fit
        """
        self.release()
        supported_linkages = {'average', 'complete', 'single'}
        if self.linkage not in supported_linkages:
            raise ValueError("linkage: Frovedis doesn't support the "\
                              + "given linkage!")
        self.__mid = ModelID.get()
        inp_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        X = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        self.__mdtype = dtype
        nsamples = X.numRows()
        (host, port) = FrovedisServer.getServerInstance()
        ret = np.zeros(nsamples, dtype=np.int32)
        rpclib.aca_train(host, port, X.get(), self.n_clusters,
                         self.linkage.encode('ascii'), ret, nsamples, 
                         self.verbose, self.__mid,
                         dtype, itype, dense)
        self.labels_ = ret
        self.__nsamples = nsamples
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    def fit_predict(self, X, y=None):
        """
        NAME: fit_predict
        """
        self.fit(X, y)
        return self.labels_

    # added for predicting with different nclusters on same model
    def predict(self, ncluster=None):
        """
        recomputes cluster indices when input 'ncluster' is different 
        than self.n_clusters
        """
        if ncluster is not None and ncluster != self.n_clusters:
            if (ncluster <= 0): 
                raise ValueError("predict: ncluster must be a positive integer!")
            self.n_clusters = ncluster
            (host, port) = FrovedisServer.getServerInstance()
            ret = np.zeros(self.__nsamples, dtype=np.int32)
            rpclib.acm_predict(host, port, self.__mid, self.__mdtype,
                               self.n_clusters, ret, self.__nsamples)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            self.labels_ = ret
        return self.labels_
    
    def score(self, X, y, sample_weight=None):
        """uses scikit-learn homogeneity_score for scoring"""
        if self.__mid is not None:
            return clustering_score(y, self.fit_predict(X, y))

    def __str__(self):
        """
        NAME: __str__
        """
        return str(self.get_params())
   
    def load(self, fname, dtype=None):
        """
        NAME: load
        """
        if isinstance(fname, str) == False:
            raise TypeError("Expected: String, Got: " + str(type(fname)))
        if not os.path.exists(fname):
            raise ValueError(\
                "the model with name %s does not exist!" % fname)
        self.release()
        metadata = open(fname+"/metadata", "rb")
        self.n_clusters, self.__mkind, self.__mdtype = pickle.load(metadata)
        metadata.close()
        if dtype is not None:
            mdt = TypeUtil.to_numpy_dtype(self.__mdtype)
            if dtype != mdt:
                raise ValueError("load: type mismatches detected!" + \
                                 "expected type: " + str(mdt) + \
                                 "; given type: " + str(dtype))
        self.__mid = ModelID.get()
        (host, port) = FrovedisServer.getServerInstance()
        model_file = fname + "/model"
        self.__nsamples = rpclib.load_frovedis_acm(host, port, self.__mid, \
                                self.__mdtype, model_file.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        # get labels
        ret = np.zeros(self.__nsamples, dtype=np.int32)
        rpclib.acm_predict(host, port, self.__mid, self.__mdtype,
                           self.n_clusters, ret, self.__nsamples)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        self.labels_ = ret
        return self

    def save(self, fname):
        """
        NAME: save
        """
        if self.__mid is not None:
            if os.path.exists(fname):
                raise ValueError(\
                    "another model with %s name already exists!" % fname)
            else:
                os.makedirs(fname)
            GLM.save(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
            metadata = open(fname+"/metadata", "wb")
            pickle.dump((self.n_clusters, self.__mkind, \
                self.__mdtype), metadata)
            metadata.close()
        else:
            raise ValueError(\
                "save: the requested model might have been released!")

    def debug_print(self):
        """
        NAME: debug_print
        """
        if self.__mid is not None:
            GLM.debug_print(self.__mid, self.__mkind, self.__mdtype)

    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, self.__mdtype)
            self.__mid = None
            self.labels_ = None

    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
            self.release()

class DBSCAN(BaseEstimator):
    """
    A python wrapper of Frovedis dbcsan
    """
    def __init__(self, eps=0.5, min_samples=5, metric='euclidean', 
                 metric_params=None, algorithm='brute', leaf_size=30, 
                 p=None, n_jobs=None, verbose=0):
        self.eps = eps
        self.min_samples = min_samples
        self.metric = metric
        self.metric_params = metric_params
        self.algorithm = algorithm
        self.leaf_size = leaf_size
        self.p = p
        self.n_jobs = n_jobs
        self.verbose = verbose
        #extra
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.DBSCAN 
        self._labels = None

    def fit(self, X, y=None):
        """
        DESC: fit method for dbscan
        """
        self.release()
        self.__mid = ModelID.get()
        # Currently Frovedis DBSCAN does not support sparse data, 
        # it would be loaded as rowmajor matrix
        inp_data = FrovedisFeatureData(X, dense_kind='rowmajor')

        X = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        self.__mdtype = dtype
        n_samples = X.numRows()

        if self.eps <= 0:
            raise ValueError(\
                "Invalid parameter value passed for eps")
        if self.min_samples < 1:
            raise ValueError(\
                "Invalid parameter value passed for min_samples")
        if self.metric is not "euclidean":
            raise ValueError(\
                "Currently Frovedis DBSCAN does not support %s metric!" \
                % self.metric)
        if self.algorithm is not "brute":
            raise ValueError(\
                "Currently Frovedis DBSCAN does not support %s algorithm!" \
                % self.algorithm)

        (host, port) = FrovedisServer.getServerInstance()
        ret = np.zeros(n_samples, dtype=np.int32)
        rpclib.dbscan_train(host, port, X.get(), self.eps, self.min_samples, \
                            ret, n_samples, self.verbose, self.__mid, dtype, \
                            itype, dense)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        self._labels = ret
        return self

    def fit_predict(self, X):
        """
        NAME: fit_predict
        """
        self.fit(X)
        return self._labels

    def score(self, X, y, sample_weight=None):
        """uses scikit-learn homogeneity_score for scoring"""
        if self.__mid is not None:
            return clustering_score(y, self.fit_predict(X))

    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, self.__mdtype)
            #print (self.__mid, " model is released")
            self.__mid = None
            self._labels= None

    @property
    def labels_(self):
        """labels_ getter"""
        if self.__mid is not None:
            if self._labels is not None:
                return self._labels
        else:
            raise AttributeError(\
            "attribute 'labels_' might have been released or called before fit")

    @labels_.setter
    def labels_(self, val):
        """labels_ setter"""
        raise AttributeError(\
            "attribute 'labels_' of DBSCAN object is not writable")

