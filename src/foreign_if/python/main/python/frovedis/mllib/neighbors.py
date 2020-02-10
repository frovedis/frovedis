#!/usr/bin/env python

import os.path
import pickle
from .model_util import *
from ..exrpc.server import FrovedisServer
from ..exrpc import rpclib
from ..matrix.ml_data import FrovedisFeatureData
from ..matrix.dense import FrovedisRowmajorMatrix
from ..matrix.dvector import FrovedisDvector
from ..matrix.crs import FrovedisCRSMatrix
from ..matrix.dtype import DTYPE, TypeUtil
import numpy as np

class NearestNeighbors(object):
    """
    A python wrapper of Frovedis Nearest  Neigbors
    """
    def __init__(self, n_neighbors=5, radius=1.0,
                 algorithm='brute', leaf_size=30, metric='euclidean',
                 p=2, metric_params=None, n_jobs=None, verbose=0,
                 chunk_size=1.0):
        self.n_neighbors = n_neighbors
        self.radius = radius
        self.algorithm = algorithm
        self.leaf_size = leaf_size
        self.metric = metric
        self.p = p
        self.metric_params = metric_params
        self.n_jobs = n_jobs
        self.verbose = verbose
        self.chunk_size = chunk_size
        # extra
        self._X = None
        self._X_movable = None
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.KNN

    def fit(self, X, y=None):
        """
        fit for Nearest Neighbors
        """
        supported_algorithms = ['brute']
        supported_metrics = ['euclidean', 'seuclidean']
        if self.algorithm not in supported_algorithms:
            raise AttributeError("Frovedis currently supports following\
                                algorithms : \n", supported_algorithms,\
                                "\n Given algorithm: ", self.algorithm,\
                                "not supported \n")
        if self.metric not in supported_metrics:
            raise AttributeError("Frovedis currently supports following\
                                metrics : \n", supported_metrics,\
                                "\n Given metric: ", self.metric,\
                                "not supported \n")

        self.release()
        self.__mid = ModelID.get()
        train_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        self._X = train_data 
        self._X_movable = train_data.is_movable()
        x = train_data.get()
        dtype = train_data.get_dtype()
        itype = train_data.get_itype()
        dense = train_data.is_dense()
        self.__mdtype = dtype
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.knn_fit(host, port, x.get(), self.n_neighbors, self.radius,
                       self.algorithm.encode("ascii"), 
                       self.metric.encode("ascii"), 
                       self.chunk_size,
                       self.verbose, self.__mid,
                       dtype, itype, dense)
        return self

    def kneighbors(self, X=None, n_neighbors=None, return_distance=True):
        if self.__mid is None:
            raise AttributeError("kneighbors called before fit")
        if n_neighbors is None:
            n_neighbors = self.n_neighbors
        if X is None:
            test_data = self._X
        else:
            test_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        x = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("kneighbors data dtype is different than fitted \
            data dtype")
        itype = test_data.get_itype()
        dense = test_data.is_dense()
        (host, port) = FrovedisServer.getServerInstance()
        knn_res = rpclib.knn_kneighbors(host, port, x.get(), n_neighbors, 
                                        self.__mid, 
                                        return_distance, dtype)
        dummy_dist = {'dptr': knn_res['distances_ptr'], 
                      'nrow': knn_res['nrow_dist'],
                      'ncol': knn_res['ncol_dist'] }
        dummy_ind = { 'dptr': knn_res['indices_ptr'], 
                      'nrow': knn_res['nrow_ind'], 
                      'ncol': knn_res['ncol_ind'] }
        distances = FrovedisRowmajorMatrix(mat=dummy_dist, 
                                           dtype=\
                                               TypeUtil.to_numpy_dtype(dtype))
        indices = FrovedisRowmajorMatrix(mat=dummy_ind, dtype=np.int64)
        if test_data.is_movable() and self._X_movable:
            if return_distance == True:
                return distances.to_numpy_array(), indices.to_numpy_array()
            else:
                return indices.to_numpy_array()
        else:
            if return_distance == True:
                return distances, indices
            else:
                return indices
 
    def kneighbors_graph(self, X=None, n_neighbors=None, mode="connectivity"):
        if self.__mid is None:
            raise AttributeError("kneighbors_graph called before fit")
        if n_neighbors is None:
            n_neighbors = self.n_neighbors
        if X is None:
            test_data = self._X
        else:
            test_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        x = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("kneighbors_graph data dtype is different than \
                           fitted data dtype")
        itype = test_data.get_itype()
        dense = test_data.is_dense()
        (host, port) = FrovedisServer.getServerInstance()
        graph = rpclib.knn_kneighbors_graph(host, port, x.get(), n_neighbors, 
                                            self.__mid, 
                                            mode.encode("ascii"), 
                                            dtype) #allow type, dense
        ret = FrovedisCRSMatrix(mat=graph, 
                                dtype=TypeUtil.to_numpy_dtype(dtype), 
                                itype=np.int64)
        if test_data.is_movable() and self._X_movable:
            return ret.to_scipy_matrix()
        else: 
            return ret

    def radius_neighbors(self, X=None, radius=None, return_distance=True):
        """Finds the neighbors within a given radius of a point or points."""
        if self.__mid is None:
            raise AttributeError("radius_neighbors called before fit")
        if radius is None:
            radius = self.radius
        if X is None:
            test_data = self._X
        else:
            test_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        x = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("radius_neighbors data dtype is different than\
                           fitted data dtype")
        itype = test_data.get_itype()
        dense = test_data.is_dense()
        (host, port) = FrovedisServer.getServerInstance()
        dmat = rpclib.knn_radius_neighbors(host, port, x.get(), radius, 
                                           self.__mid, 
                                           return_distance, dtype)
        fmat = FrovedisCRSMatrix(mat=dmat, 
                                 dtype=TypeUtil.to_numpy_dtype(dtype), 
                                 itype=np.int64)
        if test_data.is_movable() and self._X_movable:
            csr_mat = fmat.to_scipy_matrix()
            dist = []
            ind = []
            for i in range(csr_mat.shape[0]):
                dist.append(csr_mat[i].data)
                ind.append(csr_mat[i].indices)
            if return_distance:
                return dist, ind
            else:
                return ind
        else:
            return fmat

    def radius_neighbors_graph(self, X=None, radius=None, mode='connectivity'):
        """Computes the radius neighbor graph"""
        if self.__mid is None:
            raise AttributeError("radius_neighbors_graph called before fit")
        if radius is None:
            radius = self.radius
        if X is None:
            test_data = self._X
        else:
            test_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        x = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("radius_neighbors_graph data dtype is different\
                           than fitted data dtype")
        itype = test_data.get_itype()
        dense = test_data.is_dense()
        (host, port) = FrovedisServer.getServerInstance()
        dmat = rpclib.knn_radius_neighbors_graph(host, port, x.get(), radius, 
                                                 self.__mid, 
                                                 mode.encode("ascii"), 
                                                 dtype)
        #TODO: pass itype, dense
        ret = FrovedisCRSMatrix(mat=dmat, 
                                dtype=TypeUtil.to_numpy_dtype(dtype), 
                                itype=np.int64)
        if test_data.is_movable() and self._X_movable:
            return ret.to_scipy_matrix()
        else:
            return ret

    def save(self, fname):
        """
        NAME: save
        """
        pass

    def load(self, fname):
        """
        NAME: load
        """
        pass

    def debug_print(self):
        """
        NAME: debug_print
        """
        pass

    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, self.__mdtype)
            self._X = None
            self._X_movable = None
            self.__mid = None
            self.__mdtype = None

    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
            self.release()

class KNeighborsClassifier(object):
    """
    A python wrapper of KNeigborsClassifiers
    """
    def __init__(self, n_neighbors=5, weights='uniform', algorithm='brute',
                 leaf_size=30, p=2, metric='euclidean', metric_params=None, 
                 n_jobs=None, verbose=0, chunk_size=1.0):
        self.n_neighbors = n_neighbors
        self.weights = weights
        self.algorithm = algorithm
        self.leaf_size = leaf_size
        self.metric = metric
        self.p = p
        self.metric_params = metric_params
        self.n_jobs = n_jobs
        self.verbose = verbose
        self.chunk_size = chunk_size
        # extra
        self._X = None
        self._X_movable = None
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.KNC

    def fit(self, X, y):
        """
        fit for Nearest Neighbors
        """
        supported_algorithms = ['brute']
        supported_metrics = ['euclidean', 'seuclidean']
        if self.algorithm not in supported_algorithms:
            raise AttributeError("Frovedis currently supports following\
                                algorithms : \n", supported_algorithms,\
                                "\n Given algorithm: ", self.algorithm,\
                                "not supported \n")
        if self.metric not in supported_metrics:
            raise AttributeError("Frovedis currently supports following\
                                metrics : \n", supported_metrics,\
                                "\n Given metric: ", self.metric,\
                                "not supported \n")
        self.release()
        self.__mid = ModelID.get()
        train_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        self._X = train_data 
        self._X_movable = train_data.is_movable()
        x = train_data.get()
        dtype = train_data.get_dtype()
        itype = train_data.get_itype()
        dense = train_data.is_dense()
        y = FrovedisDvector.as_dvec(y,dtype=TypeUtil.to_numpy_dtype(dtype))
        if (dtype != y.get_dtype()):
            raise TypeError("fit: different dtypes are encountered for \
                            X and y!")
        self.__mdtype = dtype
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.knc_fit(host, port, x.get(), y.get(), self.n_neighbors,
                       self.algorithm.encode("ascii"), 
                       self.metric.encode("ascii"), 
                       self.chunk_size,
                       self.verbose, self.__mid,
                       dtype, itype, dense)
        return self

    def kneighbors(self, X=None, n_neighbors=None, return_distance=True):
        if self.__mid is None:
            raise AttributeError("kneighbors called before fit")
        if n_neighbors is None:
            n_neighbors = self.n_neighbors
        if X is None:
            test_data = self._X
        else:
            test_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        x = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("kneighbors data dtype is different than \
                            fitted data dtype")
        itype = test_data.get_itype()
        dense = test_data.is_dense()
        (host, port) = FrovedisServer.getServerInstance()
        knn_res = rpclib.knc_kneighbors(host, port, x.get(), n_neighbors, 
                                        self.__mid, 
                                        return_distance, dtype) 
        dummy_dist = {'dptr': knn_res['distances_ptr'], 
                      'nrow': knn_res['nrow_dist'],
                      'ncol': knn_res['ncol_dist'] }
        dummy_ind = { 'dptr': knn_res['indices_ptr'], 
                      'nrow': knn_res['nrow_ind'], 
                      'ncol': knn_res['ncol_ind'] }
        distances = FrovedisRowmajorMatrix(mat=dummy_dist, 
                                           dtype=TypeUtil.to_numpy_dtype(dtype))
        indices = FrovedisRowmajorMatrix(mat=dummy_ind, dtype=np.int64)
        if test_data.is_movable() and self._X_movable:
            if return_distance == True:
                return distances.to_numpy_array(), indices.to_numpy_array()
            else:
                return indices.to_numpy_array()
        else:
            if return_distance == True:
                return distances, indices
            else:
                return indices
 
    def kneighbors_graph(self, X=None, n_neighbors=None, mode="connectivity"):
        if self.__mid is None:
            raise AttributeError("kneighbors_graph called before fit")
        if n_neighbors is None:
            n_neighbors = self.n_neighbors
        if X is None:
            test_data = self._X
        else:
            test_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        x = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("kneighbors_graph data dtype is different\
                            than fitted data dtype")
        itype = test_data.get_itype()
        dense = test_data.is_dense()
        (host, port) = FrovedisServer.getServerInstance()
        graph = rpclib.knc_kneighbors_graph(host, port, x.get(), n_neighbors, 
                                            self.__mid, 
                                            mode.encode("ascii"), 
                                            dtype) 
        ret = FrovedisCRSMatrix(mat=graph, 
                                dtype=TypeUtil.to_numpy_dtype(dtype), 
                                itype=np.int64)
        if test_data.is_movable() and self._X_movable:
            return ret.to_scipy_matrix()
        else:
            return ret

    def save(self, fname):
        """
        NAME: save
        """
        pass

    def load(self, fname):
        """
        NAME: load
        """
        pass
 
    def predict(self, X, save_proba=False):
        """
        NAME: predict for KNeighbors classifier
        """
        if self.__mid is None:
            raise ValueError( \
            "predict is called before calling fit, or the model is released.")
        test_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        X = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("predict data dtype is different than fitted \
                            data dtype")
        nsamples = test_data.numRows()
        (host, port) = FrovedisServer.getServerInstance()
        if dtype == DTYPE.FLOAT:
            ret = np.empty(nsamples, dtype=np.float32)
            rpclib.knc_float_predict(host, port, X.get(), self.__mid,
                                     save_proba, ret, nsamples)
        elif dtype == DTYPE.DOUBLE:
            ret = np.empty(nsamples, dtype=np.float64)
            rpclib.knc_double_predict(host, port, X.get(), self.__mid,
                                      save_proba, ret, nsamples)
        else:
            raise TypeError("input type should be either float or double!")
        return ret

    def predict_proba(self, X):
        """
        NAME: predict_proba
        """
        if self.__mid is None:
            raise ValueError( \
            "predict is called before calling fit, or the model is released.")
        test_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        X = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("predict_proba data dtype is different than \
                             fitted data dtype")
        (host, port) = FrovedisServer.getServerInstance()
        dmat = rpclib.knc_predict_proba(host, port, X.get(), self.__mid, dtype) 
        ret =FrovedisRowmajorMatrix(mat=dmat, 
                                    dtype=TypeUtil.to_numpy_dtype(dtype))
        if test_data.is_movable() and self._X_movable:
            return ret.to_numpy_array()
        else:
            return ret

    def score(self, X, y, sample_weight=None):
        """
        NAME: score function for KNeighbors Regressor
        """
        if self.__mid is None:
            raise ValueError( \
            "predict is called before calling fit, or the model is released.")
        test_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        x = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("score data dtype is different than fitted \
                            data dtype")
        itype = test_data.get_itype()
        dense = test_data.is_dense()
        y = FrovedisDvector.as_dvec(y,dtype=TypeUtil.to_numpy_dtype(dtype))
        if (dtype != y.get_dtype()):
            raise TypeError("score: different dtypes are encountered for \
                            X and y!")
        (host, port) = FrovedisServer.getServerInstance()
        res = rpclib.knc_model_score(host, port, x.get(), y.get(), 
                                     self.__mid, dtype)
        return res
    
    def debug_print(self):
        """
        NAME: debug_print
        """
        pass

    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, self.__mdtype)
            self._X = None
            self._X_movable = None
            self.__mid = None
            self.__mdtype = None

    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
            self.release()

class KNeighborsRegressor(object):
    """
    A python wrapper of Kneigbors Regressor
    """
    def __init__(self, n_neighbors=5, weights='uniform', algorithm='brute',
                 leaf_size=30, p=2, metric='euclidean', metric_params=None, 
                 n_jobs=None, verbose=0, chunk_size=1.0):
        self.n_neighbors = n_neighbors
        self.weights = weights
        self.algorithm = algorithm
        self.leaf_size = leaf_size
        self.metric = metric
        self.p = p
        self.metric_params = metric_params
        self.n_jobs = n_jobs
        self.verbose = verbose
        self.chunk_size = chunk_size
        # extra
        self._X = None
        self._X_movable = None
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.KNR

    def fit(self, X, y):
        """
        fit for KNeighborsRegressor
        """
        supported_algorithms = ['brute']
        supported_metrics = ['euclidean', 'seuclidean']
        if self.algorithm not in supported_algorithms:
            raise AttributeError("Frovedis currently supports following\
                                algorithms : \n", supported_algorithms,\
                                "\n Given algorithm: ", self.algorithm,\
                                "not supported \n")
        if self.metric not in supported_metrics:
            raise AttributeError("Frovedis currently supports following\
                                metrics : \n", supported_metrics,\
                                "\n Given metric: ", self.metric,\
                                "not supported \n")
        self.release()
        self.__mid = ModelID.get()
        train_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        self._X = train_data 
        self._X_movable = train_data.is_movable()
        x = train_data.get()
        dtype = train_data.get_dtype()
        itype = train_data.get_itype()
        dense = train_data.is_dense()
        y = FrovedisDvector.as_dvec(y,dtype=TypeUtil.to_numpy_dtype(dtype))
        if (dtype != y.get_dtype()):
            raise TypeError("fit: different dtypes are encountered\
                            for X and y!")
        self.__mdtype = dtype
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.knr_fit(host, port, x.get(), y.get(), self.n_neighbors,
                       self.algorithm.encode("ascii"), 
                       self.metric.encode("ascii"), 
                       self.chunk_size,
                       self.verbose, self.__mid,
                       dtype, itype, dense)
        return self

    def kneighbors(self, X=None, n_neighbors=None, return_distance=True):
        if self.__mid is None:
            raise AttributeError("kneighbors called before fit")
        if n_neighbors is None:
            n_neighbors = self.n_neighbors
        if X is None:
            test_data = self._X
        else:
            test_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        x = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("kneighbors data dtype is different \
                            than fitted data dtype")
        itype = test_data.get_itype()
        dense = test_data.is_dense()
        (host, port) = FrovedisServer.getServerInstance()
        knn_res = rpclib.knr_kneighbors(host, port, x.get(), n_neighbors, 
                                        self.__mid, return_distance, dtype) 
        dummy_dist = {'dptr': knn_res['distances_ptr'], 
                      'nrow': knn_res['nrow_dist'],
                      'ncol': knn_res['ncol_dist'] }
        dummy_ind = { 'dptr': knn_res['indices_ptr'], 
                      'nrow': knn_res['nrow_ind'], 
                      'ncol': knn_res['ncol_ind'] }
        distances = FrovedisRowmajorMatrix(mat=dummy_dist, 
                                           dtype=TypeUtil.to_numpy_dtype(dtype))
        indices = FrovedisRowmajorMatrix(mat=dummy_ind, dtype=np.int64)
        if test_data.is_movable() and self._X_movable:
            if return_distance == True:
                return distances.to_numpy_array(), indices.to_numpy_array()
            else:
                return indices.to_numpy_array()
        else:
            if return_distance == True:
                return distances, indices
            else:
                return indices
 
    def kneighbors_graph(self, X=None, n_neighbors=None, mode="connectivity"):
        if self.__mid is None:
            raise AttributeError("kneighbors_graph called before fit")
        if n_neighbors is None:
            n_neighbors = self.n_neighbors
        if X is None:
            test_data = self._X
        else:
            test_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        x = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("kneighbors_graph data dtype is \
                            different than fitted data dtype")
        itype = test_data.get_itype()
        dense = test_data.is_dense()
        (host, port) = FrovedisServer.getServerInstance()
        graph = rpclib.knr_kneighbors_graph(host, port, x.get(), n_neighbors, 
                                            self.__mid, 
                                            mode.encode("ascii"), 
                                            dtype) 
        ret = FrovedisCRSMatrix(mat=graph, 
                                dtype=TypeUtil.to_numpy_dtype(dtype), 
                                itype=np.int64)
        if test_data.is_movable() and self._X_movable:
            return ret.to_scipy_matrix()
        else:
            return ret

    def save(self, fname):
        """
        NAME: save
        """
        pass

    def load(self, fname):
        """
        NAME: load
        """
        pass

    def predict(self, X, save_proba=True):
        """
        NAME: predict for KNeighbors Regressor
        """
        if self.__mid is None:
            raise ValueError( \
            "predict is called before calling fit, or the model is released.")
        test_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        X = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("predict data dtype is different\
                            than fitted data dtype")
        nsamples = test_data.numRows()
        (host, port) = FrovedisServer.getServerInstance()
        if dtype == DTYPE.FLOAT:
            ret = np.empty(nsamples, dtype=np.float32)
            rpclib.knr_float_predict(host, port, X.get(), self.__mid, 
                                     ret, nsamples)
        elif dtype == DTYPE.DOUBLE:
            ret = np.empty(nsamples, dtype=np.float64)
            rpclib.knr_double_predict(host, port, X.get(), self.__mid, 
                                      ret, nsamples)
        else:
            raise TypeError("input type should be either float or double!")
        return ret

    def score(self, X, y, sample_weight=None):
        """
        NAME: score function for KNeighbors Regressor
        """
        if self.__mid is None:
            raise ValueError( \
            "predict is called before calling fit, or the model is released.")
        test_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        x = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("score data dtype is different than \
                            fitted data dtype")
        itype = test_data.get_itype()
        dense = test_data.is_dense()
        y = FrovedisDvector.as_dvec(y,dtype=TypeUtil.to_numpy_dtype(dtype))
        if (dtype != y.get_dtype()):
            raise TypeError("score: different dtypes are encountered \
                           for X and y!")
        (host, port) = FrovedisServer.getServerInstance()
        res = rpclib.knr_model_score(host, port, x.get(), y.get(), 
                                     self.__mid, dtype)
        return res
  
    def debug_print(self):
        """
        NAME: debug_print
        """
        pass

    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, self.__mdtype)
            self._X = None
            self._X_movable = None
            self.__mid = None
            self.__mdtype = None

    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
            self.release()
