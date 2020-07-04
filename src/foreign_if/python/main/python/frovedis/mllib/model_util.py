"""
model_util.py
"""

#!/usr/bin/env python

import numpy as np
from ..exrpc.server import FrovedisServer
from ..exrpc import rpclib
from ..matrix.ml_data import FrovedisFeatureData
from ..matrix.dtype import DTYPE, TypeUtil

class M_KIND(object):
    """A python enumerator for wrapping model kinds"""
    GLM = 0
    LRM = 1
    SVM = 2
    LNRM = 3
    MFM = 4
    KMEANS = 5
    DTM = 6
    NBM = 7
    FMM = 8
    FPM = 9
    FPR = 10
    ACM = 11
    SCM = 12
    SEM = 13
    SPARSE_CONV_INFO = 14
    MLR = 15
    W2V = 16
    DBSCAN = 17
    KNN = 18
    KNC = 19
    KNR = 20
    LDA = 21
    LDASP = 22 #for spark
    RFM = 23
    GBT = 24
    SVR = 25

class ModelID(object):
    """A python container for generating model IDs for ML"""
    # initial model ID for python's tracking [spark generates 1 to (1 << 15)]
    __initial = (1 << 15)
    __mid = __initial
    # A threshold value, assuming it is safe to re-iterate
    # ModelID after reaching this value without affecting the
    # registered models at Frovedis server side.
    __max_id = (1 << 31)

    @staticmethod
    def get():
        """
        NAME: get
        """
        ModelID.__mid = (ModelID.__mid + 1) % ModelID.__max_id
        if ModelID.__mid == 0:
            ModelID.__mid = ModelID.__initial + 1
        return ModelID.__mid

# Defines very generic functionaties of a Frovedis model
class GLM(object):
    """A python util for common operations on Generic Model"""
    @staticmethod
    def predict(X, mid, mkind, mdtype, prob, n_classes = 1):
        """
        NAME: predict
        """
        if mdtype is None:
            raise ValueError("model for predict is typeless!")
        inp_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        X = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        if dtype != mdtype:
            raise TypeError( \
            "Input CRS matrix dtype is different than model dtype!")
        (host, port) = FrovedisServer.getServerInstance()
        len_l = X.numRows() * n_classes
        if mdtype == DTYPE.FLOAT:
            ret = np.empty(len_l, dtype=np.float32)
            rpclib.parallel_float_glm_predict(host, port, mid, mkind, \
                X.get(), ret, len_l, prob, itype, dense)
        elif mdtype == DTYPE.DOUBLE:
            ret = np.empty(len_l, dtype=np.float64)
            rpclib.parallel_double_glm_predict(host, port, mid, mkind, \
                X.get(), ret, len_l, prob, itype, dense)
        else:
            raise TypeError("model type should be either float or double!")
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return ret

    @staticmethod
    def release(mid, mkind, mdtype):
        """
        NAME: release
        """
        if mdtype is None:
            raise ValueError("model for release is typeless!")
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.release_frovedis_model(host, port, mid, mkind, mdtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

    @staticmethod
    def load(mid, mkind, mdtype, fname):
        """
        NAME: load
        """
        if mdtype is None:
            raise ValueError("model for load is typeless!")
        if isinstance(fname, str) == False:
            raise TypeError("Expected: String, Got: " + str(type(fname)))
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.load_frovedis_model(host, port, mid, mkind, mdtype, \
            fname.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

    @staticmethod
    def save(mid, mkind, mdtype, fname):
        """
        NAME: save
        """
        if mdtype is None:
            raise ValueError("model for save is typeless!")
        if isinstance(fname, str) == False:
            raise TypeError("Expected: String, Got: " + str(type(fname)))
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.save_frovedis_model(host, port, mid, mkind, mdtype, \
            fname.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

    @staticmethod
    def debug_print(mid, mkind, mdtype):
        """
        NAME: debug_print
        """
        if mdtype is None:
            raise ValueError("model for print is typeless!")
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.show_frovedis_model(host, port, mid, mkind, mdtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

