"""
grouped_df
"""
#!/usr/bin/env python

import copy
from ctypes import c_char_p
import numpy as np
import pandas as pd
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer, set_association, \
                           check_association, do_if_active_association
from ..matrix.dtype import DTYPE, get_string_array_pointer
from .frovedisColumn import FrovedisColumn
from .df import DataFrame
from .dfutil import check_string_or_array_like, check_stat_error
import numbers

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
        if len(self.__cols) > 1: #TODO: support multi-level index
            ret.add_index("index") # not similar to pandas behavior though...
        else:
            ret.set_index(keys=self.__cols, drop=True, inplace=True)
        return ret

    @check_association
    def __agg_func_with_mincount(self, func, numeric_only=None, min_count=-1):
        cols, types = self.__get_non_group_targets()
        param = check_stat_error(func, DTYPE.STRING in types, \
                                 numeric_only_= numeric_only, \
                                 min_count_ = min_count)

        agg_col = self.__get_numeric_columns() if param.numeric_only_ else cols
        agg_col_as = [func + "_" + e for e in agg_col]
        sz1 = len(self.__cols)
        sz2 = len(agg_col)
        g_cols_arr = get_string_array_pointer(self.__cols)
        a_col_arr = get_string_array_pointer(agg_col)
        a_col_as_arr = get_string_array_pointer(agg_col_as)
        
        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.gdf_aggr_with_mincount(host, port, self.__fdata,
                                                 g_cols_arr, sz1,
                                                 func.encode("ascii"),
                                                 a_col_arr,
                                                 a_col_as_arr, sz2,
                                                 param.min_count_) 
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        names = dummy_df["names"]
        types = dummy_df["types"]
        ret = DataFrame().load_dummy(dummy_df["dfptr"], names, types)
        if len(self.__cols) > 1:
            ret.add_index("index")
            ret.set_multi_index_targets(self.__cols)
        else:
            ret.set_index(keys=self.__cols, drop=True, inplace=True)

        single = pd.Index(agg_col)
        ret.set_multi_level_column(single)
        return ret

    def min(self, numeric_only=None, min_count=-1):
        """ min """
        return self.__agg_func_with_mincount("min",
               numeric_only = numeric_only, min_count = min_count)

    def max(self, numeric_only=None, min_count=-1):
        """ max """
        return self.__agg_func_with_mincount("max",
               numeric_only = numeric_only, min_count = min_count)

    def mean(self, numeric_only=None, min_count=-1):
        """ mean """
        return self.__agg_func_with_mincount("mean",
               numeric_only = numeric_only, min_count = min_count)

    def sum(self, numeric_only=None, min_count=0):
        """ sum """
        return self.__agg_func_with_mincount("sum",
               numeric_only = numeric_only, min_count = min_count)

    def count(self, numeric_only=None, min_count=-1):
        """ count """
        return self.__agg_func_with_mincount("count",
               numeric_only = numeric_only, min_count = min_count)

    def size(self, numeric_only=None, min_count=-1):
        """ size """
        return self.__agg_func_with_mincount("size",
               numeric_only = numeric_only, min_count = min_count)

    def mad(self, axis=0, skipna=True, level=None, min_count=-1):
        """ mad """
        if axis is 1:
            raise ValueError("mad: Currently supported only for axis = 0!")
        if skipna is False:
            raise ValueError("mad: Currently supported only for skipna = True!")
        if level is not None:
            raise ValueError("mad: Currently supported only for level = None!")
        return self.__agg_func_with_mincount("mad", min_count = min_count)

    def first(self, numeric_only=False, min_count=-1):
        """ first """
        return self.__agg_func_with_mincount("first", 
               numeric_only = numeric_only, min_count = min_count)

    def last(self, numeric_only=False, min_count=-1):
        """ last """
        return self.__agg_func_with_mincount("last",
               numeric_only = numeric_only, min_count = min_count)

    @check_association
    def __agg_func_with_ddof(self, func, ddof):
        """ aggregator functions supporting ddof: var, std, sem etc. """
        if not isinstance(ddof, numbers.Number):
            raise ValueError("var: parameter 'ddof' must be a number!")
        func_with_int_ddof = ["std", "sem"]  #TODO: confirm behavior with latest pandas
        ddof_ = int(ddof) if func in func_with_int_ddof else ddof
        agg_col = self.__get_numeric_columns()
        agg_col_as = [func + "_" + e for e in agg_col]
        sz1 = len(self.__cols)
        sz2 = len(agg_col)
        g_cols_arr = get_string_array_pointer(self.__cols)
        a_col_arr = get_string_array_pointer(agg_col)
        a_col_as_arr = get_string_array_pointer(agg_col_as)

        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.gdf_aggr_with_ddof(host, port, self.__fdata,
                                            g_cols_arr, sz1,
                                            func.encode("ascii"),
                                            a_col_arr, a_col_as_arr, sz2,
                                            ddof_)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        names = dummy_df["names"]
        types = dummy_df["types"]
        ret = DataFrame().load_dummy(dummy_df["dfptr"], names, types)
        if len(self.__cols) > 1:
            ret.add_index("index")
            ret.set_multi_index_targets(self.__cols)
        else:
            ret.set_index(keys=self.__cols, drop=True, inplace=True)

        single = pd.Index(agg_col)
        ret.set_multi_level_column(single)
        return ret

    def var(self, ddof=1.0):
        """ var """
        return self.__agg_func_with_ddof("var", ddof)

    def sem(self, ddof=1.0):
        """ sem """
        return self.__agg_func_with_ddof("sem", ddof)

    def std(self, ddof=1.0):
        """ std """
        return self.__agg_func_with_ddof("std", ddof)

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
            if isinstance(func, list):
                return self.__agg_with_list(func)
            if isinstance(func, dict):
                return self.__agg_with_dict(func)
            raise TypeError("Unsupported input type for aggregation")
        raise ValueError("Operation on invalid frovedis grouped dataframe!")

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
                    elif f in ('mean', 'var', 'sem'):
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
        if len(self.__cols) > 1: #TODO: support multi-level index
            ret.add_index("index")
            ret.set_multi_index_targets(self.__cols)
        else:
            ret.set_index(keys=self.__cols, drop=True, inplace=True)

        multi = pd.MultiIndex.from_tuples(list(zip(agg_col, agg_func)))
        no_of_func = multi.levels[1].size
        if no_of_func == 1:
            single = pd.Index(agg_col)
            ret.set_multi_level_column(single)
        else:
            ret.set_multi_level_column(multi)
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

    def __get_non_group_targets(self):
        """
        __get_non_group_targets
        """
        cols  = [x for x in self.__p_cols if x not in self.__cols]
        types = [self.__dict__[x].dtype for x in cols]
        return cols, types

    def get(self):
        """
        get
        """
        return self.__fdata
