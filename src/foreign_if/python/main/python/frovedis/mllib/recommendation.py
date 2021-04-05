"""
recommendation.py: wrapper of matrix factorization using ALS
"""

#!/usr/bin/env python
from __future__ import division
import os.path
import pickle
from .model_util import *
from ..base import BaseEstimator
from ..exrpc.server import FrovedisServer
from ..exrpc import rpclib
from ..matrix.crs import FrovedisCRSMatrix
from ..matrix.dtype import DTYPE
import numpy as np

class ALS(BaseEstimator):
    """A python wrapper of Frovedis ALS"""
    def __init__(self, max_iter=100, alpha=0.01, regParam=0.01,
                 similarity_factor = 0.1, seed=0, verbose=0):
        self.max_iter = max_iter
        self.alpha = alpha
        self.regParam = regParam
        self.sim_factor = similarity_factor
        self.seed = seed
        self.verbose = verbose
        # extra
        self.rank = 0
        self.nrow = 0
        self.ncol = 0
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.MFM

    def fit(self, X, rank):
        """
        NAME: fit
        """
        self.release()
        X = FrovedisCRSMatrix.asCRS(X)
        dtype = X.get_dtype()
        itype = X.get_itype()
        if dtype != DTYPE.FLOAT and dtype != DTYPE.DOUBLE:
            raise TypeError( \
            "Expected training data either of float or double type!")
        else:
            self.__mdtype = dtype
        if itype != DTYPE.INT and itype != DTYPE.LONG:
            raise TypeError( \
            "Expected training sparse data itype to be either int or long!")
        if rank <= 0:
            raise ValueError("Rank must be a possitive integer")
        if not (self.sim_factor >= 0.0 and self.sim_factor <= 1.0):
            raise ValueError("Similarity factor must be in between 0.0 and 1.0")            
        self.rank = rank
        self.nrow = X.numRows()
        self.ncol = X.numCols()
        self.__mid = ModelID.get()
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.als_train(host, port, X.get(), rank, self.max_iter, self.alpha,
                         self.regParam, self.sim_factor, self.seed, self.verbose,
                         self.__mid, dtype, itype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    def predict(self, ids):
        """
        NAME: predict
        """
        if self.__mid is not None:
            if isinstance(ids, (list, tuple)) == False:
                raise TypeError("Expected: List, Got: " + str(type(ids)))
            ids = np.asarray(ids, dtype=np.int32).flatten()
            sz = (ids.size // 2)
            (host, port) = FrovedisServer.getServerInstance()
            if self.__mdtype is None:
                raise TypeError("input model for predict is typeless!")
            if self.__mdtype == DTYPE.FLOAT:
                ret = np.zeros(sz, dtype=np.float32)
                rpclib.als_float_predict(host, port, self.__mid, ids, ret, sz)
            elif self.__mdtype == DTYPE.DOUBLE:
                ret = np.zeros(sz, dtype=np.float64)
                rpclib.als_double_predict(host, port, self.__mid, ids, ret, sz)
            else:
                raise TypeError("model type can be either float or double!")
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return ret
        else:
            raise ValueError( \
            "predict is called before calling fit, or the model is released.")

    def recommend_users(self, pid, k):
        """
        NAME: recommend_users
        """
        if self.__mid is not None:
            if k > self.nrow:
                k = self.nrow
            (host, port) = FrovedisServer.getServerInstance()
            uids = np.zeros(k, dtype=np.int32)
            if self.__mdtype is None:
                raise TypeError("input model for recommend is typeless!")
            if self.__mdtype == DTYPE.FLOAT:
                scores = np.zeros(k, dtype=np.float32)
                rpclib.als_float_rec_users(host, port, self.__mid, pid, k, \
                    uids, scores)
            elif self.__mdtype == DTYPE.DOUBLE:
                scores = np.zeros(k, dtype=np.float64)
                rpclib.als_double_rec_users(host, port, self.__mid, pid, k, \
                    uids, scores)
            else:
                raise TypeError("model type can be either float or double!")
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return [(i, j) for i, j in zip(uids, scores)]
        else:
            raise \
            ValueError( \
            "recommend_users is called before calling fit, \
            or the model is released.")

    def recommend_products(self, uid, k):
        """
        NAME: recommend_products
        """
        if self.__mid is not None:
            if k > self.ncol:
                k = self.ncol
            (host, port) = FrovedisServer.getServerInstance()
            pids = np.zeros(k, dtype=np.int32)
            if self.__mdtype is None:
                raise TypeError("input model for recommend is typeless!")
            if self.__mdtype == DTYPE.FLOAT:
                scores = np.zeros(k, dtype=np.float32)
                rpclib.als_float_rec_prods(host, port, self.__mid, uid, k, \
                    pids, scores)
            elif self.__mdtype == DTYPE.DOUBLE:
                scores = np.zeros(k, dtype=np.float64)
                rpclib.als_double_rec_prods(host, port, self.__mid, uid, k, \
                    pids, scores)
            else:
                raise TypeError("model type can be either float or double!")
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return [(i, j) for i, j in zip(pids, scores)]
        else:
            raise ValueError( \
            "recommend_products is called before calling fit, \
            or the model is released.")

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
        self.__mkind, self.__mdtype = pickle.load(metadata)
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
        ret = rpclib.load_frovedis_mfm(host, port, self.__mid, self.__mdtype,
                                       model_file.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        self.rank = ret['rank']
        self.nrow = ret['nrow']
        self.ncol = ret['ncol']

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
            pickle.dump((self.__mkind, self.__mdtype), metadata)
            metadata.close()
        else:
            raise ValueError("save: the requested model might have \
                been released!")

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
            self.rank = 0
            self.nrow = 0
            self.ncol = 0
            self.__mid = None

    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
            self.release()

