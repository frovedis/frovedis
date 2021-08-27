"""
preprocessing.py: module containing wrapper for Standard Scaler

"""
import sys
import os.path
import pickle
import numbers
import numpy as np
from ..base import *
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer, set_association, \
                           check_association, do_if_active_association
from ..matrix.ml_data import FrovedisFeatureData
from ..matrix.dense import FrovedisRowmajorMatrix
from ..matrix.crs import FrovedisCRSMatrix
from ..matrix.dtype import TypeUtil
from .model_util import *

class StandardScaler(BaseEstimator):
    """
    A python wrapper of Frovedis Standard Scaler
    """
    def __init__(self, copy = True, with_mean= True, with_std=True, 
                 sam_std = False, verbose=0):
        self.copy = copy
        self.with_mean = with_mean
        self.with_std = with_std
        self.sam_std = sam_std
        self.verbose = verbose
        self.__mkind = M_KIND.STANDARDSCALER
        # extra
        self._mean = None
        self._var = None
        self._scale = None
        self.__mid = None
        self.__mdtype = None

    def check_input(self, X, F):
        inp_data = FrovedisFeatureData(X, \
                     caller = "[" + self.__class__.__name__ + "] "+ F +": ",\
                     dense_kind='rowmajor', densify= False)
        X = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        if self.with_mean and not dense:
            raise ValueError(F + ": To centerize sparse data" \
                             + " use with_mean= False!")
        if self.with_mean is None:
            self.with_mean = False
        if self.with_std is None:
            self.with_std = False
        return X, dtype, itype, dense

    def fit(self, X, y = None, sample_weight = None):
        """
        NAME: fit
        """
        self.partial_fit(X)

    @set_association
    def partial_fit(self, X, y = None, sample_weight = None):
        """
        NAME: partial_fit
        """
        self.release()
        X, dtype, itype, \
        dense = self.check_input(X, "partial_fit")
        self.__mid = ModelID.get()
        self.__mdtype = dtype
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.scaler_partial_fit(host, port, X.get(), \
                                  self.with_mean, self.with_std, \
                                  self.sam_std, self.verbose, \
                                  self.__mid, dtype, itype, dense)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    @check_association
    def transform(self, X, copy=None):
        """
        NAME: transform X
        """
        X, dtype, itype,  \
        dense = self.check_input(X, "transform")
        (host, port) = FrovedisServer.getServerInstance()
        trans_mat = rpclib.scaler_transform(host, port, \
                            X.get(),self.with_mean, self.with_std, \
                            self.__mid, dtype, itype, dense)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        if dense:
            ret = FrovedisRowmajorMatrix(mat=trans_mat, \
                                         dtype=TypeUtil.to_numpy_dtype(dtype))
            return ret.to_numpy_matrix()
        else:
            ret = FrovedisCRSMatrix(mat= trans_mat, \
                                    dtype=TypeUtil.to_numpy_dtype(dtype),\
                                    itype=TypeUtil.to_numpy_dtype(itype))
            return ret.to_scipy_matrix()

    @check_association
    def inverse_transform(self, X, copy=None):
        """inverse_transform"""
        X, dtype, itype,  \
        dense = self.check_input( X, "inverse_transform")
        (host, port) = FrovedisServer.getServerInstance()
        inverse_trans_mat = rpclib.scaler_inverse_transform(host, port, \
                            X.get(), self.__mid, dtype, itype, dense)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        if dense:
            ret = FrovedisRowmajorMatrix(mat= inverse_trans_mat, \
                                     dtype=TypeUtil.to_numpy_dtype(dtype))
            return ret.to_numpy_matrix()
        else:
            ret = FrovedisCRSMatrix(mat=inverse_trans_mat,\
                                     dtype=TypeUtil.to_numpy_dtype(dtype),\
                                     itype=TypeUtil.to_numpy_dtype(itype))
            return ret.to_scipy_matrix()

    def fit_transform(self, X, y=None):
        """fit_transform"""
        self.fit(X)
        return self.transform(X)

    @property
    @check_association
    def mean_(self):
        """
        NAME: mean_
        """
        if self._mean is None:
            (host, port) = FrovedisServer.getServerInstance()
            mean_vector = rpclib.get_scaler_mean(host, port, self.__mid, \
                                                  self.__mdtype)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            self._mean = np.asarray(mean_vector, dtype=np.float64)
        return self._mean

    @mean_.setter
    def mean_(self, val):
        """Setter method for mean__ """
        raise AttributeError(\
        "attribute 'mean_' of Standard Scaler is not writable")

    @property
    @check_association
    def var_(self):
        """
        NAME: var_
        """
        if self._var is None:
            (host, port) = FrovedisServer.getServerInstance()
            var_vector = rpclib.get_scaler_var(host, port, self.__mid, \
                                                  self.__mdtype)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            self._var = np.asarray(var_vector, dtype=np.float64)
        return self._var

    @var_.setter
    def var_(self, val):
        """Setter method for var__ """
        raise AttributeError(\
        "attribute 'var_' of Standard Scaler is not writable")

    @property
    @check_association
    def scale_(self):
        """
        NAME: scale_
        """
        if self._scale is None:
            (host, port) = FrovedisServer.getServerInstance()
            std_vector = rpclib.get_scaler_std(host, port, self.__mid, \
                                                  self.__mdtype)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            self._scale = np.asarray(std_vector, dtype=np.float64)
        return self._scale

    @scale_.setter
    def scale_(self, val):
        """Setter method for var__ """
        raise AttributeError(\
        "attribute 'scale_' of Standard Scaler is not writable")        

    def release(self):
        """
        resets after-fit populated attributes to None
        """
        self.__release_server_heap()
        self._mean = None
        self._var = None
        self._scale = None
        self.__mid = None
        self.__mdtype = None
        self.__itype = None
        self.__dense = None

    @do_if_active_association
    def __release_server_heap(self):
        """
        to release model pointer from server heap
        """
        GLM.release(self.__mid, self.__mkind, self.__mdtype)

    def is_fitted(self):
        """ function to confirm if the model is already fitted """
        return self.__mid is not None
