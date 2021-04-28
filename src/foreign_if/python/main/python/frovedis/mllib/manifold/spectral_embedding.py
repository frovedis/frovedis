"""
main/python/frovedis/mllib/manifold/_spectral_embedding.py: 
module containing wrapper for spectral embedding
"""

import os.path
import pickle
from ..model_util import *
from ...base import *
from ...exrpc.server import FrovedisServer, set_association, \
                           check_association, do_if_active_association
from ...exrpc import rpclib
from ...matrix.ml_data import FrovedisFeatureData
from ...matrix.dense import FrovedisRowmajorMatrix
from ...matrix.dtype import TypeUtil

class SpectralEmbedding(BaseEstimator):
    """
    A python wrapper of Frovedis Spectral Embedding
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

    @set_association
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
        self.__mdtype = dtype
        self.__X_movable = inp_data.is_movable()
        if self.affinity == "precomputed":
            precomputed = True
        else:
            precomputed = False
        (host, port) = FrovedisServer.getServerInstance()
        if dense and X.get_mtype() != 'R':
            raise TypeError("fit: please provide row-major points " +
                            "for frovedis spectral embedding!")
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

    @property
    @check_association
    def affinity_matrix_(self):
        """
        NAME: get_affinity_matrix
        """
        if self._affinity is None:
            (host, port) = FrovedisServer.getServerInstance()
            dmat = rpclib.get_sem_affinity_matrix(host, port, self.__mid, \
                                                    self.__mdtype)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            rmat = FrovedisRowmajorMatrix(mat=dmat, dtype=TypeUtil. \
                                          to_numpy_dtype(self.__mdtype))
            if self.__X_movable:
                self._affinity = rmat.to_numpy_array()
            else:
                self._affinity = rmat
        return self._affinity
         
    @affinity_matrix_.setter
    def affinity_matrix_(self, val):
        """Setter method for affinity_matrix_ """
        raise AttributeError(\
        "attribute 'affinity_matrix_' of SpectralEmbedding is not writable")

    @property
    @check_association
    def embedding_(self):
        """
        NAME: get_embedding_matrix
        """
        if self._embedding is None:
            (host, port) = FrovedisServer.getServerInstance()
            dmat = rpclib.get_sem_embedding_matrix(host, port, self.__mid, \
                                                            self.__mdtype)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            rmat = FrovedisRowmajorMatrix(mat=dmat, dtype=TypeUtil. \
                                          to_numpy_dtype(self.__mdtype))
            if self.__X_movable:
                self._embedding = rmat.to_numpy_array()
            else:
                self._embedding = rmat
        return self._embedding

    @embedding_.setter
    def embedding_(self, val):
        """Setter method for embedding_ """
        raise AttributeError(\
        "attribute 'embedding_' of SpectralEmbedding is not writable")

    @set_association
    def load(self, fname, dtype=None):
        """
        NAME: load
        """
        if not os.path.exists(fname):
            raise ValueError(\
                "the model with name %s does not exist!" % fname)
        self.release()
        metadata = open(fname + "/metadata", "rb")
        self.n_components, self.__mkind, self.__mdtype = pickle.load(metadata)
        metadata.close()
        if dtype is not None:
            mdt = TypeUtil.to_numpy_dtype(self.__mdtype)
            if dtype != mdt:
                raise ValueError("load: type mismatches detected! " + \
                                 "expected type: " + str(mdt) + \
                                 "; given type: " + str(dtype))
        self.__mid = ModelID.get()
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname + "/model")
        return self

    @check_association
    def save(self, fname):
        """
        NAME: save
        """
        if os.path.exists(fname):
            raise ValueError(\
                "another model with %s name already exists!" % fname)
        else:
            os.makedirs(fname)
        GLM.save(self.__mid, self.__mkind, self.__mdtype, fname + "/model")
        metadata = open(fname + "/metadata", "wb")
        pickle.dump((self.n_components, self.__mkind, \
            self.__mdtype), metadata)
        metadata.close()

    @check_association
    def debug_print(self):
        """
        NAME: debug_print
        """
        GLM.debug_print(self.__mid, self.__mkind, self.__mdtype)

    def release(self):
        """
        resets after-fit populated attributes to None
        """
        self.__release_server_heap()
        self.__mid = None
        self._affinity = None
        self._embedding = None

    @do_if_active_association
    def __release_server_heap(self):
        """
        to release model pointer from server heap
        """
        GLM.release(self.__mid, self.__mkind, self.__mdtype)

    def __del__(self):
        """
        NAME: __del__
        """
        self.release()

    def is_fitted(self):
        """ function to confirm if the model is already fitted """
        return self.__mid is not None

