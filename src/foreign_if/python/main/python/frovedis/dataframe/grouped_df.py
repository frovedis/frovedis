"""
grouped_df
"""
#!/usr/bin/env python

import copy
from ctypes import c_char_p
import numpy as np
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer, set_association, \
                           check_association, do_if_active_association
from ..matrix.dtype import DTYPE, get_string_array_pointer
from .frovedisColumn import FrovedisColumn
from .df import DataFrame
from .dfutil import check_string_or_array_like 

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

    @set_association
    def load_dummy(self, fdata, cols, types, p_cols, p_types):
        """
        load_dummy
        """
        self.__fdata = fdata
        self.__cols = cols
        self.__types = types
        self.__p_cols = p_cols
        self.__p_types = p_types
        for i in range(0, len(p_cols)):
            cname = p_cols[i]
            dt = p_types[i]
            self.__dict__[cname] = FrovedisColumn(cname, dt)
        return self

    def release(self):
        """
        to release dataframe pointer from server heap and
        resets after-fit populated attributes to None
        """
        self.__release_server_heap()
        if self.__cols is not None:
            for cname in self.__cols:
                del self.__dict__[cname]
        self.__fdata = None
        self.__cols = None
        self.__types = None
        self.__p_cols = None
        self.__p_types = None

    @do_if_active_association
    def __release_server_heap(self):
        """ releases the dataframe pointer from server heap """
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.release_frovedis_grouped_dataframe(host, port, self.__fdata)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

    def __del__(self):
        """ destructs a python dataframe object """
        self.release()

    def is_fitted(self):
        """ function to confirm if the dataframe is already constructed """
        return self.__fdata is not None

    @check_association
    def __getitem__(self, target):
        """  __getitem__  """
        target = check_string_or_array_like(target, "select")
        return self.__select_gdf(target)

    def __select_gdf(self, cols):
        """ To select grouped columns """
        sz = len(cols)
        types = [0] * sz
        i = 0
        for item in cols:
            if item not in self.__dict__:
                raise ValueError("No column named: %s" % (item))
            types[i] = self.__dict__[item].dtype
            i = i + 1
        tcols_arr = get_string_array_pointer(cols)
        (host, port) = FrovedisServer.getServerInstance()
        fdata = rpclib.select_grouped_dataframe(host, port, self.get(),
                                                tcols_arr, sz)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        ret = DataFrame().load_dummy(fdata, cols, types)
        if (len(self.__cols) > 1): #TODO: support multi-level index
            ret.add_index("index") # not similar to pandas behavior though...
        else:
            ret.set_index(keys=self.__cols, drop=True, inplace=True)
        return ret

    def agg(self, func, *args, **kwargs):
        """
        agg
        """
        return self.aggregate(func, args, kwargs)

    @check_association
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

    def size(self, numeric_only=True):
        return self.agg("size")

    def __agg_with_list(self, func):
        """
        __agg_with_list
        """
        num_cols = self.__get_numeric_columns()
        args = {}
        funcs = []
        if len(func) == 1 and "size" in func:
            if len(self.__cols) > 0:
                args[self.__cols[0]] = ["size"]
            else:
                raise TypeError("Column not provided to GroupBy.")
        else:
            for col in num_cols:
                args[col] = func
            if 'count' in func:
                funcs.append("count")
            if 'size' in func:
                funcs.append("size")
            if len(funcs) > 0:
                n_num_cols = self.__get_non_numeric_columns()
                for col in n_num_cols:
                    args[col] = funcs
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
                raise ValueError("agg: No column named: ", col)
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
                    if tid == DTYPE.STRING and f != 'count' and f != 'size':
                        raise ValueError("Currently Frovedis doesn't support"
                                         " aggregator %s to be applied on"
                                         " string-typed column %s" %(f, col))
                    agg_func.append(f)
                    agg_col.append(col)
                    agg_col_as.append(new_col)
                    if f == 'count' or f == 'size':
                        col_as_tid = DTYPE.ULONG
                    elif f == 'mean':
                        col_as_tid = DTYPE.DOUBLE
                    else:
                        col_as_tid = tid
                    agg_col_as_types.append(col_as_tid)
        #print(agg_func)
        #print(agg_col)
        #print(agg_col_as)
        #print(agg_col_as_types)
        sz1 = len(self.__cols) 
        sz2 = len(agg_func) 
        g_cols_arr = get_string_array_pointer(self.__cols)
        a_func_arr = get_string_array_pointer(agg_func)
        a_col_arr = get_string_array_pointer(agg_col)
        a_col_as_arr = get_string_array_pointer(agg_col_as)

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
        ret = DataFrame().load_dummy(fdata, cols, types)
        if (len(self.__cols) > 1): #TODO: support multi-level index
            ret.add_index("index")
        else:
            ret.set_index(keys=self.__cols, drop=True, inplace=True)
        return ret

    def __get_numeric_columns(self):
        """
        __get_numeric_columns
        """
        non_numeric_types = [DTYPE.STRING]
        cols = []
        non_grouped_cols = [x for x in self.__p_cols if x not in self.__cols]
        #for i in range(0, len(self.__p_cols)):
        #    if self.__p_types[i] not in non_numeric_types:
        #        cols.append(self.__p_cols[i])
        for col in non_grouped_cols:
            if self.__dict__[col].dtype not in non_numeric_types:
                cols.append(col)
        return cols

    def __get_non_numeric_columns(self):
        """
        __get_non_numeric_columns
        """
        non_numeric_types = [DTYPE.STRING]
        cols = []
        non_grouped_cols = [x for x in self.__p_cols if x not in self.__cols]
        #for i in range(0, len(self.__p_cols)):
        #    if self.__p_types[i] not in non_numeric_types:
        #        cols.append(self.__p_cols[i])
        for col in non_grouped_cols:
            if self.__dict__[col].dtype in non_numeric_types:
                cols.append(col)
        return cols

    def get(self):
        """
        get
        """
        return self.__fdata
