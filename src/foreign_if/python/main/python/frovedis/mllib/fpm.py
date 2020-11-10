"""
fpm.py
"""

#!/usr/bin/env python

from .model_util import *
from ..exrpc import rpclib
from ..matrix.dtype import DTYPE
from frovedis.exrpc.server import FrovedisServer
import pandas as pd
import frovedis.dataframe as fpd

class FPGrowth(object):
    """A python wrapper of Frovedis GPGrowth"""
    def __init__(self, minSupport=0.3, verbose=0):
        self.minSupport = minSupport
        self.verbose = verbose
        self.__mid = None
        self.__mkind = M_KIND.FPM

    def __convert_to_df(self, data):
        """
        NAME: __convert_to_df
        """
        tid = []
        item = []
        cur_id = 0
        for trans in data: # for each transaction in data
            for trans_it in trans: # for each item in each transaction
                tid.append(cur_id)
                item.append(trans_it)
            cur_id = cur_id + 1
        tid = np.asarray(tid, dtype=np.int32)
        item = np.asarray(item, dtype=np.int32)
        df_t = pd.DataFrame({'trans_id': tid, 'item': item}, \
                columns=['trans_id', 'item'])
        return fpd.DataFrame(df_t)

    def fit(self, data):
        """
        NAME: fit
        """
        if self.minSupport < 0:
            raise ValueError("Negative minsupport factor!")
        self.release()
        self.__mid = ModelID.get()
        f_df = self.__convert_to_df(data)
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.fpgrowth_trainer(host, port, f_df.get(), self.__mid, \
            self.minSupport, self.verbose)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    def generate_rules(self, confidence=0.8):
        """
        NAME: generate_rules
        """
        midr = ModelID.get()
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.fpgrowth_fpr(host, port, self.__mid, midr, confidence)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return Fp_rules(midr)

    def load(self, fname):
        """
        NAME: load
        """
        self.release()
        self.__mid = ModelID.get()
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.load_frovedis_model(host, port, self.__mid, self.__mkind,
                                   DTYPE.DOUBLE, fname.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    def save(self, fname):
        """
        NAME: save
        """
        if self.__mid is not None:
            GLM.save(self.__mid, self.__mkind,
                     DTYPE.DOUBLE, fname)

    def debug_print(self):
        """
        NAME: debug_print
        """
        if self.__mid is not None:
            GLM.debug_print(self.__mid, self.__mkind,
                            DTYPE.DOUBLE)

    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, DTYPE.DOUBLE)
            self.__mid = None

    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
            self.release()

class Fp_rules(object):
    """
    Fp_rules
    """
    def __init__(self, mid=None):
        self.__mkind = M_KIND.FPR
        self.__mid = mid

    def debug_print(self):
        """
        NAME: debug_print
        """
        if self.__mid is not None:
            GLM.debug_print(self.__mid, self.__mkind,
                            DTYPE.DOUBLE)
