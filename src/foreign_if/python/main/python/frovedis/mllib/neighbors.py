"""
neighbors.py: wrapper of KNN related algorithms
"""
import os.path
import pickle
from .model_util import *
from ..base import BaseEstimator
from ..exrpc.server import FrovedisServer
from ..exrpc.server import FrovedisServer, set_association, \
                           check_association, do_if_active_association
from ..exrpc import rpclib
from ..matrix.dense import FrovedisRowmajorMatrix
from ..matrix.crs import FrovedisCRSMatrix
from ..matrix.ml_data import FrovedisFeatureData, FrovedisLabeledPoint
from ..matrix.dtype import DTYPE, TypeUtil
import numpy as np

class NearestNeighbors(BaseEstimator):
    """
    A python wrapper of Frovedis Nearest  Neigbors
    """
    def __init__(self, n_neighbors=5, radius=1.0,
                 algorithm='auto', leaf_size=30, metric='euclidean',
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
        # extra
        self.chunk_size = chunk_size
        self._X = None
        self._X_movable = None
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.KNN

    @set_association
    def fit(self, X, y=None):
        """
        fit for Nearest Neighbors
        """
        if self.algorithm == "auto":
            self.algorithm = "brute"
        supported_algorithms = ['brute']
        supported_metrics = ['euclidean', 'seuclidean']
        if self.algorithm not in supported_algorithms:
            raise AttributeError("Frovedis KNN currently supports following\
                                algorithms : \n", supported_algorithms,\
                                "\n Given algorithm: ", self.algorithm,\
                                "not supported \n")
        if self.metric not in supported_metrics:
            raise AttributeError("Frovedis KNN currently supports following\
                                metrics : \n", supported_metrics,\
                                "\n Given metric: ", self.metric,\
                                "not supported \n")
        self.release()
        train_data = FrovedisFeatureData(X, dense_kind='rowmajor', \
                       caller = "[" + self.__class__.__name__ + "] fit: ", \
                       densify=True)
        self._X = train_data 
        self._X_movable = train_data.is_movable()
        X = train_data.get()
        dtype = train_data.get_dtype()
        itype = train_data.get_itype()
        dense = train_data.is_dense()
        self.__mid = ModelID.get()
        self.__mdtype = dtype
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.knn_fit(host, port, X.get(), self.n_neighbors, self.radius,
                       self.algorithm.encode("ascii"), 
                       self.metric.encode("ascii"), 
                       self.chunk_size,
                       self.verbose, self.__mid,
                       dtype, itype, dense)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    @check_association
    def kneighbors(self, X=None, n_neighbors=None, return_distance=True):
        if n_neighbors is None:
            n_neighbors = self.n_neighbors
        if X is None:
            test_data = self._X # FrovedisFeatureData
        else:
            test_data = FrovedisFeatureData(X, dense_kind='rowmajor', \
                          caller = "[" + self.__class__.__name__ + \
                                   "] kneighbors: ", \
                          densify=True)
        X = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("kneighbors data dtype is different than " + \
                            "fitted data dtype")
        itype = test_data.get_itype() #TODO: pass itype, dense to client cpp
        dense = test_data.is_dense()
        (host, port) = FrovedisServer.getServerInstance()
        knn_res = rpclib.knn_kneighbors(host, port, X.get(), n_neighbors, 
                                        self.__mid, 
                                        return_distance, dtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
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
 
    @check_association
    def kneighbors_graph(self, X=None, n_neighbors=None, mode="connectivity"):
        if n_neighbors is None:
            n_neighbors = self.n_neighbors
        if X is None:
            test_data = self._X # FrovedisFeatureData
        else:
            test_data = FrovedisFeatureData(X, dense_kind='rowmajor', \
                          caller = "[" + self.__class__.__name__ + \
                                   "] kneighbors_graph: ", \
                          densify=True)
        X = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("kneighbors_graph data dtype is different " + \
                            "than fitted data dtype")
        itype = test_data.get_itype() #TODO: pass itype, dense to client cpp
        dense = test_data.is_dense()
        (host, port) = FrovedisServer.getServerInstance()
        graph = rpclib.knn_kneighbors_graph(host, port, X.get(), n_neighbors, 
                                            self.__mid, 
                                            mode.encode("ascii"), 
                                            dtype) #allow type, dense
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        ret = FrovedisCRSMatrix(mat=graph, 
                                dtype=TypeUtil.to_numpy_dtype(dtype), 
                                itype=np.int64)
        if test_data.is_movable() and self._X_movable:
            return ret.to_scipy_matrix()
        else: 
            return ret

    @check_association
    def radius_neighbors(self, X=None, radius=None, return_distance=True):
        """Finds the neighbors within a given radius of a point or points."""
        if radius is None:
            radius = self.radius
        if X is None:
            test_data = self._X # FrovedisFeatureData
        else:
            test_data = FrovedisFeatureData(X, dense_kind='rowmajor', \
                          caller = "[" + self.__class__.__name__ + \
                                   "] radius_neighbors: ", \
                          densify=True)
        X = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("radius_neighbors data dtype is different " + \
                            "than fitted data dtype")
        itype = test_data.get_itype()
        dense = test_data.is_dense()
        (host, port) = FrovedisServer.getServerInstance()
        dmat = rpclib.knn_radius_neighbors(host, port, X.get(), radius, 
                                           self.__mid, 
                                           dtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        fmat = FrovedisCRSMatrix(mat=dmat, 
                                 dtype=TypeUtil.to_numpy_dtype(dtype), 
                                 itype=np.int64)
        if test_data.is_movable() and self._X_movable:
            csr_mat = fmat.to_scipy_matrix()
            if return_distance:
                dist = []
                ind = []
                for i in range(csr_mat.shape[0]):
                    dist.append(csr_mat[i].data)
                    ind.append(csr_mat[i].indices)
                return dist, ind
            else:
                ind = []
                for i in range(csr_mat.shape[0]):
                    ind.append(csr_mat[i].indices)
                return ind
        else:
            return fmat

    @check_association
    def radius_neighbors_graph(self, X=None, radius=None, mode='connectivity'):
        """Computes the radius neighbor graph"""
        if radius is None:
            radius = self.radius
        if X is None:
            test_data = self._X # FrovedisFeatureData
        else:
            test_data = FrovedisFeatureData(X, dense_kind='rowmajor', \
                          caller = "[" + self.__class__.__name__ + \
                                   "] radius_neighbors_graph: ", \
                          densify=True)
        X = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("radius_neighbors_graph data dtype is " + \
                            "different than fitted data dtype")
        itype = test_data.get_itype() #TODO: pass itype, dense to cpp client
        dense = test_data.is_dense()
        (host, port) = FrovedisServer.getServerInstance()
        dmat = rpclib.knn_radius_neighbors_graph(host, port, X.get(), radius, 
                                                 self.__mid, 
                                                 mode.encode("ascii"), 
                                                 dtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        ret = FrovedisCRSMatrix(mat=dmat, 
                                dtype=TypeUtil.to_numpy_dtype(dtype), 
                                itype=np.int64)
        if test_data.is_movable() and self._X_movable:
            return ret.to_scipy_matrix()
        else:
            return ret

    #@check_association
    def save(self, fname):
        """
        NAME: save
        """
        raise AttributeError("save: not supported for NearestNeighbors!")

    #@set_association
    def load(self, fname):
        """
        NAME: load
        """
        raise AttributeError("load: not supported for NearestNeighbors!")

    #@check_association
    def debug_print(self):
        """
        NAME: debug_print
        """
        raise AttributeError("debug_print: not supported for " \
                             "NearestNeighbors!")

    def release(self):
        """
        resets after-fit populated attributes to None
        """
        self.__release_server_heap()
        self._X = None
        self._X_movable = None
        self.__mid = None
        self.__mdtype = None

    @do_if_active_association
    def __release_server_heap(self):
        """
        to release model pointer from server heap
        """
        GLM.release(self.__mid, self.__mkind, self.__mdtype)

    def __del__(self):
        """
        destructs the python object
        """
        self.release()

    def is_fitted(self):
        """ function to confirm if the model is already fitted """
        return self.__mid is not None

class KNeighborsClassifier(BaseEstimator):
    """
    A python wrapper of KNeigborsClassifiers
    """
    def __init__(self, n_neighbors=5, weights='uniform', algorithm='auto',
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
        # extra
        self.chunk_size = chunk_size
        self._X = None
        self._X_movable = None
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.KNC

    @set_association
    def fit(self, X, y):
        """
        fit for Nearest Neighbors
        """
        if self.algorithm == "auto":
            self.algorithm = "brute"
        supported_algorithms = ['brute']
        supported_metrics = ['euclidean', 'seuclidean']
        if self.algorithm not in supported_algorithms:
            raise AttributeError("Frovedis KNN currently supports following\
                                algorithms : \n", supported_algorithms,\
                                "\n Given algorithm: ", self.algorithm,\
                                "not supported \n")
        if self.metric not in supported_metrics:
            raise AttributeError("Frovedis KNN currently supports following\
                                metrics : \n", supported_metrics,\
                                "\n Given metric: ", self.metric,\
                                "not supported \n")
        self.release()
        train_data = FrovedisLabeledPoint(X, y, \
                   caller = "[" + self.__class__.__name__ + "] fit: ",\
                   encode_label = True, binary_encoder=[0, 1], \
                   dense_kind = 'rowmajor', densify=True)
        self._X = train_data 
        self._X_movable = train_data.is_movable()
        X, y, logic = train_data.get()
        self._classes = train_data.get_distinct_labels()
        self.n_classes = len(self._classes)
        self.label_map = logic
        dtype = train_data.get_dtype()
        itype = train_data.get_itype()
        dense = train_data.is_dense()
        self.__mid = ModelID.get()
        self.__mdtype = dtype
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.knc_fit(host, port, X.get(), y.get(), self.n_neighbors,
                       self.algorithm.encode("ascii"), 
                       self.metric.encode("ascii"), 
                       self.chunk_size,
                       self.verbose, self.__mid,
                       dtype, itype, dense)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    @check_association
    def kneighbors(self, X=None, n_neighbors=None, return_distance=True):
        if n_neighbors is None:
            n_neighbors = self.n_neighbors
        if X is None:
            test_data = self._X # FrovedisLabeledPoint
            X = test_data.get()[0]
        else:
            test_data = FrovedisFeatureData(X, dense_kind='rowmajor', \
                          caller = "[" + self.__class__.__name__ + \
                                   "] kneighbors: ", \
                          densify=True)
            X = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("kneighbors data dtype is different than " + \
                            "fitted data dtype")
        itype = test_data.get_itype() #TODO: pass itype, dense to client cpp
        dense = test_data.is_dense()
        (host, port) = FrovedisServer.getServerInstance()
        knn_res = rpclib.knc_kneighbors(host, port, X.get(), n_neighbors, 
                                        self.__mid, 
                                        return_distance, dtype) 
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
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

    @check_association 
    def kneighbors_graph(self, X=None, n_neighbors=None, mode="connectivity"):
        if n_neighbors is None:
            n_neighbors = self.n_neighbors
        if X is None:
            test_data = self._X # FrovedisLabeledPoint
            X = test_data.get()[0]
        else:
            test_data = FrovedisFeatureData(X, dense_kind='rowmajor', \
                          caller = "[" + self.__class__.__name__ + \
                                   "] kneighbors_graph: ", \
                          densify=True)
            X = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("kneighbors_graph data dtype is different " + \
                            "than fitted data dtype")
        itype = test_data.get_itype() #TODO: pass itype, dense to client cpp
        dense = test_data.is_dense()
        (host, port) = FrovedisServer.getServerInstance()
        graph = rpclib.knc_kneighbors_graph(host, port, X.get(), n_neighbors, 
                                            self.__mid, 
                                            mode.encode("ascii"), 
                                            dtype) 
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        ret = FrovedisCRSMatrix(mat=graph, 
                                dtype=TypeUtil.to_numpy_dtype(dtype), 
                                itype=np.int64)
        if test_data.is_movable() and self._X_movable:
            return ret.to_scipy_matrix()
        else:
            return ret

    #@check_association
    def save(self, fname):
        """
        NAME: save
        """
        raise AttributeError("save: not supported for KNeighborsClassifier!")

    #@set_association
    def load(self, fname):
        """
        NAME: load
        """
        raise AttributeError("load: not supported for KNeighborsClassifier!")
 
    @check_association
    def predict(self, X, save_proba=False):
        """
        NAME: predict for KNeighbors classifier
        """
        test_data = FrovedisFeatureData(X, dense_kind='rowmajor', \
                          caller = "[" + self.__class__.__name__ + \
                                   "] predict: ", \
                          densify=True)
        X = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("predict data dtype is different than fitted " + \
                            "data dtype")
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
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return np.asarray([self.label_map[ret[i]] \
                          for i in range(0, len(ret))])

    @check_association
    def predict_proba(self, X):
        """
        NAME: predict_proba
        """
        test_data = FrovedisFeatureData(X, dense_kind='rowmajor', \
                          caller = "[" + self.__class__.__name__ + \
                                   "] predict_proba: ", \
                          densify=True)
        X = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("predict_proba data dtype is different than " + \
                            "fitted data dtype")
        (host, port) = FrovedisServer.getServerInstance()
        dmat = rpclib.knc_predict_proba(host, port, X.get(), self.__mid, dtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        ret = FrovedisRowmajorMatrix(mat=dmat, 
                                     dtype=TypeUtil.to_numpy_dtype(dtype))
        if test_data.is_movable() and self._X_movable:
            return ret.to_numpy_array()
        else:
            return ret

    # TODO: support sample_weight
    @check_association
    def score(self, X, y, sample_weight=None):
        """
        NAME: score function for KNeighbors Regressor
        """
        test_data = FrovedisLabeledPoint(X, y, \
                   caller = "[" + self.__class__.__name__ + "] score: ",\
                   encode_label = True, binary_encoder=[0, 1], \
                   dense_kind = 'rowmajor', densify=True)
        X, y, logic = test_data.get()
        dtype = test_data.get_dtype()
        itype = test_data.get_itype() # TODO pass itype, dense to client cpp
        dense = test_data.is_dense()
        if (dtype != self.__mdtype):
            raise TypeError("score data dtype is different than fitted " + \
                            "data dtype")
        (host, port) = FrovedisServer.getServerInstance()
        res = rpclib.knc_model_score(host, port, X.get(), y.get(), 
                                     self.__mid, dtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return res

    @property
    @check_association
    def classes_(self):
        """classes_ getter"""
        if self._classes is None:
            self._classes = np.sort(list(self.label_map.values()))
        return self._classes

    @classes_.setter
    def classes_(self, val):
        """classes_ setter"""
        raise AttributeError(\
            "attribute 'classes_' of KNeighborsClassifier object is not writable")

    #@check_association
    def debug_print(self):
        """
        NAME: debug_print
        """
        raise AttributeError("debug_print: not supported for " \
                             "KNeighborsClassifier!")

    def release(self):
        """
        resets after-fit populated attributes to None
        """
        self.__release_server_heap()
        self._X = None
        self._X_movable = None
        self.__mid = None
        self.__mdtype = None

   @do_if_active_association
    def __release_server_heap(self):
        """
        to release model pointer from server heap
        """
        GLM.release(self.__mid, self.__mkind, self.__mdtype)

    def __del__(self):
        """
        destructs the python object
        """
        self.release()

    def is_fitted(self):
        """ function to confirm if the model is already fitted """
        return self.__mid is not None

class KNeighborsRegressor(BaseEstimator):
    """
    A python wrapper of Kneigbors Regressor
    """
    def __init__(self, n_neighbors=5, weights='uniform', algorithm='auto',
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
        # extra
        self.chunk_size = chunk_size
        self._X = None
        self._X_movable = None
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.KNR

    @set_association
    def fit(self, X, y):
        """
        fit for KNeighborsRegressor
        """
        if self.algorithm == "auto":
            self.algorithm = "brute"
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
        train_data = FrovedisLabeledPoint(X, y, \
                   caller = "[" + self.__class__.__name__ + "] fit: ",\
                   dense_kind = 'rowmajor', densify=True)
        self._X = train_data 
        self._X_movable = train_data.is_movable()
        X, y = train_data.get()
        dtype = train_data.get_dtype()
        itype = train_data.get_itype()
        dense = train_data.is_dense()
        self.__mid = ModelID.get()
        self.__mdtype = dtype

        (host, port) = FrovedisServer.getServerInstance()
        rpclib.knr_fit(host, port, X.get(), y.get(), self.n_neighbors,
                       self.algorithm.encode("ascii"), 
                       self.metric.encode("ascii"), 
                       self.chunk_size,
                       self.verbose, self.__mid,
                       dtype, itype, dense)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    @check_association
    def kneighbors(self, X=None, n_neighbors=None, return_distance=True):
        if n_neighbors is None:
            n_neighbors = self.n_neighbors
        if X is None:
            test_data = self._X # FrovedisLabeledPoint
            X = test_data.get()[0]
        else:
            test_data = FrovedisFeatureData(X, dense_kind='rowmajor', \
                          caller = "[" + self.__class__.__name__ + \
                                   "] kneighbors: ", \
                          densify=True)
            X = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("kneighbors data dtype is different " + \
                            "than fitted data dtype")
        itype = test_data.get_itype() #TODO: pass itype, dense to client cpp
        dense = test_data.is_dense()
        (host, port) = FrovedisServer.getServerInstance()
        knn_res = rpclib.knr_kneighbors(host, port, X.get(), n_neighbors, 
                                        self.__mid, return_distance, dtype) 
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
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
 
    @check_association
    def kneighbors_graph(self, X=None, n_neighbors=None, mode="connectivity"):
        if n_neighbors is None:
            n_neighbors = self.n_neighbors
        if X is None:
            test_data = self._X # FrovedisLabeledPoint
            X = test_data.get()[0]
        else:
            test_data = FrovedisFeatureData(X, dense_kind='rowmajor', \
                          caller = "[" + self.__class__.__name__ + \
                                   "] kneighbors_graph: ", \
                          densify=True)
            X = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("kneighbors_graph data dtype is " + \
                            "different than fitted data dtype")
        itype = test_data.get_itype() #TODO: pass itype, dense to client cpp
        dense = test_data.is_dense()
        (host, port) = FrovedisServer.getServerInstance()
        graph = rpclib.knr_kneighbors_graph(host, port, X.get(), n_neighbors, 
                                            self.__mid, 
                                            mode.encode("ascii"), 
                                            dtype) 
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        ret = FrovedisCRSMatrix(mat=graph, 
                                dtype=TypeUtil.to_numpy_dtype(dtype), 
                                itype=np.int64)
        if test_data.is_movable() and self._X_movable:
            return ret.to_scipy_matrix()
        else:
            return ret

    #@check_association
    def save(self, fname):
        """
        NAME: save
        """
        raise AttributeError("save: not supported for KNeighborsRegressor!")

    #@set_association
    def load(self, fname):
        """
        NAME: load
        """
        raise AttributeError("load: not supported for KNeighborsRegressor!")

    @check_association
    def predict(self, X):
        """
        NAME: predict for KNeighbors Regressor
        """
        test_data = FrovedisFeatureData(X, dense_kind='rowmajor', \
                          caller = "[" + self.__class__.__name__ + \
                                   "] predict: ", \
                          densify=True)
        X = test_data.get()
        dtype = test_data.get_dtype()
        if (dtype != self.__mdtype):
            raise TypeError("predict data dtype is different " + \
                            "than fitted data dtype")
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
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return np.asarray(ret, dtype=np.float64)

    # TODO: support sample_weight
    @check_association
    def score(self, X, y, sample_weight=None):
        """
        NAME: score function for KNeighbors Regressor
        """
        test_data = FrovedisLabeledPoint(X, y, \
                   caller = "[" + self.__class__.__name__ + "] score: ",\
                   dense_kind = 'rowmajor', densify=True)
        X, y = test_data.get()
        dtype = test_data.get_dtype()
        itype = test_data.get_itype() # TODO pass itype, dense to client cpp
        dense = test_data.is_dense()
        if (dtype != self.__mdtype):
            raise TypeError("score data dtype is different than " + \
                            "fitted data dtype")
        (host, port) = FrovedisServer.getServerInstance()
        res = rpclib.knr_model_score(host, port, X.get(), y.get(), 
                                     self.__mid, dtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return res
 
    #@check_association
    def debug_print(self):
        """
        NAME: debug_print
        """
        raise AttributeError("debug_print: not supported for " \
                             "KNeighborsRegressor!")

    def release(self):
        """
        resets after-fit populated attributes to None
        """
        self.__release_server_heap()
        self._X = None
        self._X_movable = None
        self.__mid = None
        self.__mdtype = None

    @do_if_active_association
    def __release_server_heap(self):
        """
        to release model pointer from server heap
        """
        GLM.release(self.__mid, self.__mkind, self.__mdtype)

    def __del__(self):
        """
        destructs the python object
        """
        self.release()

    def is_fitted(self):
        """ function to confirm if the model is already fitted """
        return self.__mid is not None

