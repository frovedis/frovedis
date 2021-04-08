"""
recommendation.py: wrapper of matrix factorization using ALS
"""

from __future__ import division
import os.path
import pickle
import numpy as np
from ..base import BaseEstimator
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer, check_server_state
from ..matrix.crs import FrovedisCRSMatrix
from ..matrix.dtype import DTYPE
from .model_util import *

class ALS(BaseEstimator):
    """A python wrapper of Frovedis ALS"""
    def __init__(self, rank=None, max_iter=100, alpha=0.01, 
                 reg_param=0.01, similarity_factor=0.1,
                 seed=0, verbose=0):
        self.rank = rank
        self.max_iter = max_iter
        self.alpha = alpha
        self.reg_param = reg_param
        self.sim_factor = similarity_factor
        self.seed = seed
        self.verbose = verbose
        # extra populated after fit()
        self.__mid = None
        self.__sid = None
        self.__mdtype = None
        self.n_samples_ = None
        self.n_features_ = None
        self.__mkind = M_KIND.MFM

    def fit(self, X):
        """
        NAME: fit
        """
        self.release()
        X = FrovedisCRSMatrix.asCRS(X)
        dtype = X.get_dtype()
        itype = X.get_itype()
        self.n_samples_ = X.numRows()
        self.n_features_ = X.numCols()
        if dtype != DTYPE.FLOAT and dtype != DTYPE.DOUBLE:
            raise TypeError( \
            "fit: expected training data either of float or double type!")
        else:
            self.__mdtype = dtype
        if itype != DTYPE.INT and itype != DTYPE.LONG:
            raise TypeError("fit: expected training sparse data " 
                            "itype to be either int or long!")
        if not (self.sim_factor >= 0.0 and self.sim_factor <= 1.0):
            raise ValueError(\
            "fit: similarity factor must be in between 0.0 and 1.0!")
        if self.rank is None: 
            self.rank = min(256, min(self.n_samples_, self.n_features_))
        if self.rank <= 0:
            raise ValueError("fit: rank must be a possitive integer!")

        (host, port) = FrovedisServer.getServerInstance()
        self.__mid = ModelID.get()
        self.__sid = FrovedisServer.getID()
        rpclib.als_train(host, port, X.get(), self.rank, self.max_iter, \
                         self.alpha, self.reg_param, self.sim_factor, \
                         self.seed, self.verbose, \
                         self.__mid, dtype, itype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    def predict(self, ids):
        """
        NAME: predict
        """
        if self.__mid is None:
            raise AttributeError(\
            "predict is called before calling fit, or the model is released!")
        if not isinstance(ids, (list, tuple)):
            raise TypeError("predict: expected: list/tuple, Got: " + str(type(ids)))
        check_server_state(self.__sid, self.__class__.__name__)

        ids = np.asarray(ids, dtype=np.int32).flatten()
        sz = (ids.size // 2)
        (host, port) = FrovedisServer.getServerInstance()
        if self.__mdtype == DTYPE.FLOAT:
            ret = np.zeros(sz, dtype=np.float32)
            rpclib.als_float_predict(host, port, self.__mid, ids, ret, sz)
        elif self.__mdtype == DTYPE.DOUBLE:
            ret = np.zeros(sz, dtype=np.float64)
            rpclib.als_double_predict(host, port, self.__mid, ids, ret, sz)
        else:
            raise TypeError(\
            "predict: model type can be either float or double!")
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return ret

    def recommend_users(self, pid, k):
        """
        NAME: recommend_users
        """
        if self.__mid is None:
            raise AttributeError(\
            "recommend_users is called before calling fit, " \
            "or the model is released!")
        check_server_state(self.__sid, self.__class__.__name__)

        if k > self.n_samples_:
            k = self.n_samples_
        (host, port) = FrovedisServer.getServerInstance()
        uids = np.zeros(k, dtype=np.int32)
        if self.__mdtype == DTYPE.FLOAT:
            scores = np.zeros(k, dtype=np.float32)
            rpclib.als_float_rec_users(host, port, self.__mid, pid, k, \
                                       uids, scores)
        elif self.__mdtype == DTYPE.DOUBLE:
            scores = np.zeros(k, dtype=np.float64)
            rpclib.als_double_rec_users(host, port, self.__mid, pid, k, \
                                        uids, scores)
        else:
            raise TypeError("recommend_users: model type can be " \
                            "either float or double!")
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return [(i, j) for i, j in zip(uids, scores)]

    def recommend_products(self, uid, k):
        """
        NAME: recommend_products
        """
        if self.__mid is None:
            raise AttributeError(\
            "recommend_products is called before calling fit, " \
            "or the model is released!")
        check_server_state(self.__sid, self.__class__.__name__)

        if k > self.n_features_:
            k = self.n_features_
        (host, port) = FrovedisServer.getServerInstance()
        pids = np.zeros(k, dtype=np.int32)
        if self.__mdtype == DTYPE.FLOAT:
            scores = np.zeros(k, dtype=np.float32)
            rpclib.als_float_rec_prods(host, port, self.__mid, uid, k, \
                                       pids, scores)
        elif self.__mdtype == DTYPE.DOUBLE:
            scores = np.zeros(k, dtype=np.float64)
            rpclib.als_double_rec_prods(host, port, self.__mid, uid, k, \
                                        pids, scores)
        else:
            raise TypeError("recommend_products: model type can be " \
                            "either float or double!")
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return [(i, j) for i, j in zip(pids, scores)]

    def load(self, fname, dtype=None):
        """
        NAME: load
        """
        if not isinstance(fname, str):
            raise TypeError("load: expected: String, Got: " + str(type(fname)))
        if not os.path.exists(fname):
            raise ValueError(\
                "the model with name %s does not exist!" % fname)
        self.release()
        metadata = open(fname + "/metadata", "rb")
        self.__mkind, self.__mdtype = pickle.load(metadata)
        metadata.close()
        if dtype is not None:
            mdt = TypeUtil.to_numpy_dtype(self.__mdtype)
            if dtype != mdt:
                raise ValueError("load: type mismatches detected! " + \
                                 "expected type: " + str(mdt) + \
                                 "; given type: " + str(dtype))
        (host, port) = FrovedisServer.getServerInstance()
        self.__mid = ModelID.get()
        self.__sid = FrovedisServer.getID()
        model_file = fname + "/model"
        ret = rpclib.load_frovedis_mfm(host, port, self.__mid, self.__mdtype,
                                       model_file.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        self.rank = ret['rank']
        self.n_samples_ = ret['nrow']
        self.n_features_ = ret['ncol']

    def save(self, fname):
        """
        NAME: save
        """
        if self.__mid is None:
            raise AttributeError("save: the requested model might have " \
                                 "been released or called before fit!")
        check_server_state(self.__sid, self.__class__.__name__)
        if os.path.exists(fname):
            raise ValueError(\
                "another model with %s name already exists!" % fname)
        else:
            os.makedirs(fname)
        GLM.save(self.__mid, self.__mkind, self.__mdtype, fname + "/model")
        metadata = open(fname + "/metadata", "wb")
        pickle.dump((self.__mkind, self.__mdtype), metadata)
        metadata.close()

    def debug_print(self):
        """
        NAME: debug_print
        """
        if self.__mid is not None:
            check_server_state(self.__sid, self.__class__.__name__)
            GLM.debug_print(self.__mid, self.__mkind, self.__mdtype)

    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            if FrovedisServer.isUP(self.__sid):
                GLM.release(self.__mid, self.__mkind, self.__mdtype)
        self.__mid = None
        self.__sid = None
        self.n_samples_ = None 
        self.n_features_ = None

    def __del__(self):
        """
        NAME: __del__
        """
        self.release()

