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
from ..matrix.dtype import DTYPE, get_string_array_pointer, str_encode
from .frovedisColumn import FrovedisColumn
from .df import DataFrame
from .dfutil import check_string_or_array_like, check_stat_error, \
                    non_numeric_supporter, double_typed_aggregator, \
                    ulong_typed_aggregator
import numbers

class FrovedisGroupedDataframe(object):
    """
    A python container for holding Frovedis side created grouped dataframe
    """
    def __init__(self, df=None, as_index=True):
        """
        __init__
        """
        self.__fdata = None
        self.__cols = None
        self.__types = None
        self.__p_cols = None
        self.__p_types = None
        self.as_index = as_index

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
        if not self.as_index or len(self.__cols) > 1: #TODO: support multi-level index
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
                                                 str_encode(func),
                                                 a_col_arr,
                                                 a_col_as_arr, sz2,
                                                 param.min_count_) 
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        names = dummy_df["names"]
        types = dummy_df["types"]
        ret = DataFrame().load_dummy(dummy_df["dfptr"], names, types)
        if not self.as_index:
            ret.add_index("index")
            single = pd.Index(self.__cols + agg_col)
        else:
            single = pd.Index(agg_col)
            if len(self.__cols) > 1:
                ret.add_index("index")
                ret.set_multi_index_targets(self.__cols)
            else:
                ret.set_index(keys=self.__cols, drop=True, inplace=True)

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
        if axis == 1:
            raise ValueError("mad: Currently supported only for axis = 0!")
        if skipna == False:
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
                                            str_encode(func),
                                            a_col_arr, a_col_as_arr, sz2,
                                            ddof_)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        names = dummy_df["names"]
        types = dummy_df["types"]
        ret = DataFrame().load_dummy(dummy_df["dfptr"], names, types)

        if not self.as_index:
            ret.add_index("index")
            single = pd.Index(self.__cols + agg_col)
        else:
            single = pd.Index(agg_col)
            if len(self.__cols) > 1:
                ret.add_index("index")
                ret.set_multi_index_targets(self.__cols)
            else:
                ret.set_index(keys=self.__cols, drop=True, inplace=True)

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

    def agg(self, func=None, *args, **kwargs):
        """
        agg
        """
        return self.aggregate(func, *args, **kwargs)

    @check_association
    def aggregate(self, func=None, *args, **kwargs):
        """
        aggregate
        """
        if self.__fdata is None:
            raise ValueError(\
            "Operation on invalid frovedis grouped dataframe!")

        if func is None:
            if len(kwargs) == 0:
                raise TypeError(\
                "Must provide 'func' or tuples of '(column, aggfunc)'")
            fdict = {}
            asname = {}
            for k, v in kwargs.items():
                if (isinstance(v, tuple) and len(v) == 2):
                    cname = v[0]
                    fname = v[1]
                    if cname in fdict:
                        fdict[cname].append(fname)
                        asname[cname].append(k)
                    else:
                        fdict[cname] = [fname]
                        asname[cname] = [k]
                else:
                    raise TypeError(\
                    "Must provide 'func' or tuples of '(column, aggfunc)'")
            order = list(kwargs.keys())
            return self.__agg_with_dict(func=fdict, asname=asname, order=order)
        elif isinstance(func, (str, list, tuple)):
            lfunc = check_string_or_array_like(func, "agg")
            return self.__agg_with_list(lfunc)
        elif isinstance(func, dict):
            return self.__agg_with_dict(func)
        else: 
            raise TypeError("agg: Unsupported 'func' of type " +
                            "'{}' is provided.".format(type(func).__name__))

    def __agg_with_list(self, func):
        """
        __agg_with_list
        """
        num_cols = self.__get_numeric_columns()
        args = {col: func for col in num_cols}
        funcs = list(filter(lambda x: x in non_numeric_supporter, func))
        if len(funcs) > 0:
            n_num_cols = self.__get_non_numeric_columns()
            for col in n_num_cols:
                args[col] = funcs
        return self.__agg_with_dict(args)

    # when asname is provided, it must be in-sync with func
    def __agg_with_dict(self, func, asname=None, order=None):
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

            params = {}
            if isinstance(aggfuncs, str):
                if asname:
                    col_as = asname[col] 
                else:
                    col_as = aggfuncs + '_' + col
                params[col_as] = aggfuncs
            elif isinstance(aggfuncs, list):
                for i in range(len(aggfuncs)):
                    f = aggfuncs[i]
                    if asname:
                        col_as = asname[col][i]
                    else:
                        col_as = f + '_' + col
                    params[col_as] = f
            else: 
                raise ValueError("agg: invalid dictionary input is provided.")

            tid = self.__dict__[col].dtype
            for new_col, f in params.items():
                if tid == DTYPE.STRING and f not in non_numeric_supporter:
                    raise ValueError("Currently Frovedis doesn't support"
                                     " aggregator %s to be applied on"
                                     " string-typed column %s" %(f, col))
                agg_func.append(f)
                agg_col.append(col)
                agg_col_as.append(new_col)
                if f in ulong_typed_aggregator:
                    col_as_tid = DTYPE.ULONG
                elif f in double_typed_aggregator:
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
        if not self.as_index:
            with_added_index = True
            ret.add_index("index")
        else:
            if len(self.__cols) > 1:
                with_added_index = True
                ret.add_index("index")
                ret.set_multi_index_targets(self.__cols)
            else:
                with_added_index = False
                ret.set_index(keys=self.__cols, drop=True, inplace=True)
        
        if len(agg_col) > 0:
            if self.as_index:
                tcol = agg_col
                tfunc = agg_func
            else:
                tcol = self.__cols + agg_col
                tfunc = [''] * len(self.__cols) + agg_func
            multi = pd.MultiIndex.from_tuples(list(zip(tcol, tfunc)))
            ret.set_multi_level_column(multi)

        if order:
            if with_added_index:
                ordered_cols = ["index"] + self.__cols + order
            else:
                ordered_cols = self.__cols + order
            ret.set_col_order(ordered_cols)

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
