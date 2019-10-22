"""
grouped_df
"""
#!/usr/bin/env python

import numpy as np
import pandas as pd
import copy
from ctypes import c_char_p
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from ..matrix.dtype import DTYPE
from .frovedisColumn import FrovedisColumn
from . import df

class FrovedisGroupedDataframe(object):
    """
    A python container for holding Frovedis side created grouped dataframe
    """
    def __init__(self, df=None):
        """
        __init__
        """
        self.__fdata = None
        self.__cols = None
        self.__types = None
        self.__p_cols = None
        self.__p_types = None

    def load_dummy(self, fdata, cols, types, p_cols, p_types):
        """
        load_dummy
        """
        self.__fdata = fdata
        self.__cols = copy.deepcopy(cols)
        self.__types = copy.deepcopy(types)
        self.__p_cols = copy.deepcopy(p_cols)
        self.__p_types = copy.deepcopy(p_types)
        for i in range(0, len(p_cols)):
            cname = p_cols[i]
            dt = p_types[i]
            self.__dict__[cname] = FrovedisColumn(cname, dt)
        return self

    def release(self):
        """
        release
        """
        if self.__fdata is not None:
            (host, port) = FrovedisServer.getServerInstance()
            rpclib.release_frovedis_dataframe(host, port, self.__fdata)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            for cname in self.__cols:
                del self.__dict__[cname]
            self.__fdata = None
            self.__cols = None
            self.__types = None
            self.__p_cols = None
            self.__p_types = None

    #def __del__(self):
    #  if FrovedisServer.isUP(): self.release()

    def agg(self, func, *args, **kwargs):
        """
        agg
        """
        return self.aggregate(func, args, kwargs)

    def aggregate(self, func, *args, **kwargs):
        """
        aggregate
        """
        if self.__fdata is not None:
            if isinstance(func, str):
                return self.__agg_with_list([func])
            elif isinstance(func, list):
                return self.__agg_with_list(func)
            elif isinstance(func, dict):
                return self.__agg_with_dict(func)
            else:
                raise TypeError("Unsupported input type for aggregation")
        else:
            raise ValueError("Operation on invalid frovedis grouped dataframe!")

    def __agg_with_list(self, func):
        """
        __agg_with_list
        """
        num_cols = self.__get_numeric_columns()
        args = {}
        for col in num_cols:
            args[col] = func
        if 'count' in func:
            n_num_cols = self.__get_non_numeric_columns()
            for col in n_num_cols:
                args[col] = ['count']
        return self.__agg_with_dict(args)

    def __agg_with_dict(self, func):
        """
        __agg_with_dict
        """
        agg_func = []
        agg_col = []
        agg_col_as = []
        agg_col_as_types = []
        for col, aggfuncs in func.items():
            if col not in self.__dict__:
                raise ValueError("No column named: ", col)
            else: tid = self.__dict__[col].dtype
            for f in aggfuncs:
                if tid == DTYPE.STRING and f != 'count':
                    raise ValueError("Currently Frovedis doesn't support \
                                      aggregator %s to be applied on \
                                      string-typed column %s" %(f, col))
                else:
                    agg_func.append(f)
                    agg_col.append(col)
                    new_col = f + '(' + col + ')'
                    agg_col_as.append(new_col)
                    if f == 'count':
                        col_as_tid = DTYPE.LONG
                    elif f == 'mean':
                        col_as_tid = DTYPE.DOUBLE
                    else:
                        col_as_tid = tid
                    agg_col_as_types.append(col_as_tid)
        #print(agg_func)
        #print(agg_col)
        #print(agg_col_as)
        #print(agg_col_as_types)
        g_cols = np.asarray(self.__cols)
        sz1 = g_cols.size
        g_cols_arr = (c_char_p * sz1)()
        g_cols_arr[:] = np.array([e.encode('ascii') for e in g_cols.T])

        a_func = np.asarray(agg_func)
        a_col = np.asarray(agg_col)
        a_col_as = np.asarray(agg_col_as)
        sz2 = a_func.size
        a_func_arr = (c_char_p * sz2)()
        a_col_arr = (c_char_p * sz2)()
        a_col_as_arr = (c_char_p * sz2)()
        a_func_arr[:] = np.array([e.encode('ascii') for e in a_func.T])
        a_col_arr[:] = np.array([e.encode('ascii') for e in a_col.T])
        a_col_as_arr[:] = np.array([e.encode('ascii') for e in a_col_as.T])

        (host, port) = FrovedisServer.getServerInstance()
        fdata = rpclib.agg_grouped_dataframe(host, port, self.__fdata,
                                             g_cols_arr, sz1,
                                             a_func_arr, a_col_arr,
                                             a_col_as_arr, sz2)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        cols = self.__cols + agg_col_as
        types = self.__types + agg_col_as_types
        return df.FrovedisDataframe().load_dummy(fdata, cols, types)

    def __get_numeric_columns(self):
        """
        __get_numeric_columns
        """
        cols = []
        for i in range(0, len(self.__p_cols)):
            if self.__p_types[i] != DTYPE.STRING:
                cols.append(self.__p_cols[i])
        return cols

    def __get_non_numeric_columns(self):
        """
        __get_non_numeric_columns
        """
        cols = []
        for i in range(0, len(self.__p_cols)):
            if self.__p_types[i] == DTYPE.STRING:
                cols.append(self.__p_cols[i])
        return cols

    def get(self):
        """
        get
        """
        return self.__fdata
