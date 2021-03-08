"""
fpm.py
"""

#!/usr/bin/env python

from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from ..matrix.dtype import DTYPE
from .model_util import *

import numpy as np
from collections import Iterable
import pandas as pd
import frovedis.dataframe as fpd

class FPGrowth(object):
    """A python wrapper of Frovedis FP-Growth"""
    def __init__(self, min_support=0.3, tree_depth=None,
                 compression_point=4, mem_opt_level=0,
                 verbose=0, 
                 encode_string_input=False):
        self.min_support = min_support
        self.tree_depth = tree_depth
        self.compression_point = compression_point
        self.mem_opt_level = mem_opt_level
        self.verbose = verbose
        self.encode_string_input = encode_string_input
        self.__mid = None
        self.__mkind = M_KIND.FPM
        self.encode_logic = None

    def __encode_input(self, data):
        import itertools
        unq = np.unique(list(itertools.chain.from_iterable(data)))
        int_id = np.arange(1, len(unq) + 1, 1, dtype=np.int32)
        transmap = dict(zip(unq, int_id))
        ret = []
        for e in data:
            enc = [transmap[i] for i in e]
            ret.append(enc)
        self.encode_logic = transmap
        #print("### encoding logic: ")
        #print(transmap)
        return ret

    def __convert_iterable(self, data):
        """
        converts an iterable to frovedis DataFrame
        """
        tid = []
        item = []
        cur_id = 1
        #print("### iterable data: ")
        #print(data)
        if self.encode_string_input: 
            data = self.__encode_input(data)
            #print("### encoded iterable data: ")
            #print(data)
        for trans in data: # for each transaction in data
            for trans_it in trans: # for each item in each transaction
                tid.append(cur_id)
                item.append(trans_it)
            cur_id = cur_id + 1
        df_t = pd.DataFrame({'trans_id': tid, 'item': item}, \
                            columns=['trans_id', 'item'])
        #print("### converted pandas dataframe: ")
        #print(df_t)
        return fpd.DataFrame(df_t)

    def __convert_pandas_df(self, data):
        """
        converts a pandas DataFrame to frovedis DataFrame
        """
        item_list = []
        for ilist in data.values.tolist():
            item = [itm for itm in ilist if str(itm) != 'nan']
            item_list.append(item)
        return self.__convert_iterable(item_list)

    def __check_input(self, data):
        if self.min_support < 0 or self.min_support > 1:
            raise ValueError("fit: min_support value must be within 0 to 1.\n")
        if self.tree_depth is None:
            self.tree_depth = np.iinfo(np.int32).max
        if self.tree_depth < 1:
            raise ValueError("fit: tree_depth value must be >= 1.\n")
        if self.compression_point < 2:
            raise ValueError("fit: compression_point value must be >= 2.\n")
        if self.mem_opt_level != 0 and self.mem_opt_level != 1:
            raise ValueError("fit: mem_opt_level value must be 0 or 1.\n")
        if isinstance(data, pd.DataFrame):
            f_df = self.__convert_pandas_df(data)
        elif isinstance(data, fpd.DataFrame):
            f_df = data
        elif isinstance(data, Iterable):
            f_df = self.__convert_iterable(data)
        else:
            raise ValueError("fit: only dataframe and iterable " + 
                             "inputs are supported!\n")
        return f_df

    def fit(self, data):
        """
        NAME: fit
        """
        f_df = self.__check_input(data)
        self.release()
        self.__mid = ModelID.get()
        (host, port) = FrovedisServer.getServerInstance()
        self.fis_count = rpclib.fpgrowth_trainer(host, port, \
            f_df.get(), self.__mid, \
            self.min_support, self.tree_depth, self.compression_point, \
            self.mem_opt_level, self.verbose)
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
        count = rpclib.fpgrowth_fpr(host, port, self.__mid, midr, confidence)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return FPRules(midr, count)

    def load(self, fname):
        """
        NAME: load
        """
        self.release()
        self.__mid = ModelID.get()
        (host, port) = FrovedisServer.getServerInstance()
        self.fis_count = rpclib.load_fp_model(host, port, self.__mid, 
                         self.__mkind, fname.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    def save(self, fname):
        """
        NAME: save
        """
        if self.__mid is not None:
            GLM.save(self.__mid, self.__mkind, DTYPE.DOUBLE, fname)

    def debug_print(self):
        """
        NAME: debug_print
        """
        if self.__mid is not None:
            GLM.debug_print(self.__mid, self.__mkind, DTYPE.DOUBLE)

    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, DTYPE.DOUBLE)
            self.__mid = None
            self.encode_logic = None
            self.fis_count = None

    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
            self.release()

class FPRules(object):
    """
    FPRules
    """
    def __init__(self, mid=None, count=None):
        self.__mkind = M_KIND.FPR
        self.__mid = mid
        self.count = count

    def load(self, fname):
        """
        NAME: load
        """
        self.release()
        self.__mid = ModelID.get()
        (host, port) = FrovedisServer.getServerInstance()
        self.count = rpclib.load_fp_model(host, port, self.__mid, \
                              self.__mkind, fname.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    def save(self, fname):
        """
        NAME: save
        """
        if self.__mid is not None:
            GLM.save(self.__mid, self.__mkind, DTYPE.DOUBLE, fname)

    def debug_print(self):
        """
        NAME: debug_print
        """
        if self.__mid is not None:
            GLM.debug_print(self.__mid, self.__mkind, DTYPE.DOUBLE)

    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, DTYPE.DOUBLE)
            self.__mid = None
            self.count = None

    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
            self.release()

