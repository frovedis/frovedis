"""
grouped_df
"""
#!/usr/bin/env python

import copy
from ctypes import c_char_p
import numpy as np
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from ..matrix.dtype import DTYPE
from .frovedisColumn import FrovedisColumn
from .df import DataFrame

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

    def __getitem__(self, target):
        """  __getitem__  """
        if self.__fdata is not None:
            if isinstance(target, str):
                return self.__select_gdf([target])
            elif isinstance(target, list):
                return self.__select_gdf(target)
            else:
                raise TypeError("Unsupported indexing input type!")
        else:
            raise ValueError("Operation on invalid frovedis grouped dataframe!")

    def __select_gdf(self, cols):
        """ To select grouped columns """
        sz = len(cols)
        types = [0] * sz
        for i in range(sz):
            col = cols[i]
            if col not in self.__dict__:
                raise ValueError("No column named: ", col)
            types[i] = self.__dict__[col].dtype
        tcols = np.asarray(cols)
        tcols_arr = (c_char_p * sz)()
        tcols_arr[:] = np.array([e.encode('ascii') for e in tcols.T])
        (host, port) = FrovedisServer.getServerInstance()
        fdata = rpclib.select_grouped_dataframe(host, port, self.__fdata,
                                                tcols_arr, sz)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return DataFrame().load_dummy(fdata, cols, types)

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
    
    def min(self, numeric_only=True, min_count=-1):
        if numeric_only == False:
            raise ValueError("min: Currently supported only for numeric columns!")
        return self.agg("min")

    def max(self, numeric_only=True, min_count=-1):
        if numeric_only == False:
            raise ValueError("max: Currently supported only for numeric columns!")
        return self.agg("max")

    def mean(self, numeric_only=True):
        if numeric_only == False:
            raise ValueError("mean: Currently supported only for numeric columns!")
        return self.agg("mean")

    def sum(self, numeric_only=True, min_count=0):
        if numeric_only == False:
            raise ValueError("sum: Currently supported only for numeric columns!")
        return self.agg("sum")

    def count(self, numeric_only=True):
        return self.agg("count")    

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
            if isinstance(aggfuncs, str):
                aggfuncs = [aggfuncs]
            if isinstance(aggfuncs, list):
                params = {}
                for f in aggfuncs:
                    col_as = f + '_' + col
                    params[col_as] = f
                aggfuncs = params
            if isinstance(aggfuncs, dict):
                for new_col, f in aggfuncs.items():
                    if tid == DTYPE.STRING and f != 'count':
                        raise ValueError("Currently Frovedis doesn't support"
                                         " aggregator %s to be applied on"
                                         " string-typed column %s" %(f, col))
                    agg_func.append(f)
                    agg_col.append(col)
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
        return DataFrame().load_dummy(fdata, cols, types)

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
