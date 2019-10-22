"""
df.py
"""
#!/usr/bin/env python

import numpy as np
import pandas as pd
import copy
from dfoperator import dfoperator
from ctypes import *
from array import array
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from ..matrix.dvector import FrovedisIntDvector, FrovedisLongDvector
from ..matrix.dvector import FrovedisFloatDvector, FrovedisDoubleDvector
from ..matrix.dvector import FrovedisStringDvector
from ..matrix.dtype import DTYPE
from ..matrix.dense import FrovedisRowmajorMatrix
from ..matrix.dense import FrovedisColmajorMatrix
from ..matrix.crs import FrovedisCRSMatrix
from ..mllib.model_util import ModelID
from .info import df_to_sparse_info
from .frovedisColumn import FrovedisColumn

class FrovedisDataframe(object):
    """
    FrovedisDataframe
    """

    def __init__(self, df=None):
        """
        __init__
        """
        self.__fdata = None
        self.__cols = None
        self.__types = None
        if df is not None:
            self.load(df)
    def load_dummy(self, fdata, cols, types):
        """
        load_dummy
        """
        self.__fdata = fdata
        self.__cols = copy.deepcopy(cols)
        self.__types = copy.deepcopy(types)
        for i in range(0, len(cols)):
            cname = cols[i]
            dt = types[i]
            self.__dict__[cname] = FrovedisColumn(cname, dt)
        return self

    def show(self):
        """
        show
        """
        if self.__fdata is not None:
            (host, port) = FrovedisServer.getServerInstance()
            rpclib.show_frovedis_dataframe(host, port, self.__fdata)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            print "\n"

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

    #def __del__(self):
    #    if FrovedisServer.isUP(): self.release()

    def load(self, df):
        """
        load
        """
        self.release()
        cols = df.columns
        size = len(cols)
        self.__cols = cols.tolist()
        self.__types = [0]*size
        tmp = [None]*size
        dvec = [0]*size
        idx = 0

        for cname in cols:
            val = df[cname]
            vtype = type(val[0]).__name__
            if vtype == 'int32':
                dt = DTYPE.INT
                tmp[idx] = FrovedisIntDvector(val)
            elif vtype == 'int64':
                dt = DTYPE.LONG
                tmp[idx] = FrovedisLongDvector(val)
            elif vtype == 'float32':
                dt = DTYPE.FLOAT
                tmp[idx] = FrovedisFloatDvector(val)
            elif vtype == 'float64':
                dt = DTYPE.DOUBLE
                tmp[idx] = FrovedisDoubleDvector(val)
            elif vtype == 'str':
                dt = DTYPE.STRING
                tmp[idx] = FrovedisStringDvector(val)
            else:
                raise TypeError("Unsupported column type in creation of \
                                 frovedis dataframe: ", vtype)
            self.__types[idx] = dt
            dvec[idx] = tmp[idx].get()
            #For dataframe query purpose
            self.__dict__[cname] = FrovedisColumn(cname, dt)
            idx = idx + 1

        cols_a = np.asarray(self.__cols)
        col_names = (c_char_p * size)()
        col_names[:] = [e.encode('ascii') for e in cols_a.T]
        dvec_arr = np.asarray(dvec, dtype=c_long)
        dptr = dvec_arr.ctypes.data_as(POINTER(c_long))
        type_arr = np.asarray(self.__types, dtype=c_short)
        tptr = type_arr.ctypes.data_as(POINTER(c_short))

        (host, port) = FrovedisServer.getServerInstance()
        self.__fdata = rpclib.create_frovedis_dataframe(host, port, tptr,
                                                        col_names, dptr, size)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

    def __getitem__(self, target):
        """
        __getitem__
        """
        if self.__fdata is not None:
            if isinstance(target, str):
                return self.select_frovedis_dataframe([target])
            elif isinstance(target, list):
                return self.select_frovedis_dataframe(target)
            elif isinstance(target, dfoperator):
                return self.filter_frovedis_dataframe(target)
            else:
                raise TypeError("Unsupported indexing input type!")
        else:
            raise ValueError("Operation on invalid frovedis dataframe!")

    def filter_frovedis_dataframe(self, opt):
        """
        filter_frovedis_dataframe
        """
        ret = FrovedisDataframe()
        ret.__cols = copy.deepcopy(self.__cols)
        ret.__types = copy.deepcopy(self.__types)
        for item in ret.__cols:
            ret.__dict__[item] = self.__dict__[item]
        (host, port) = FrovedisServer.getServerInstance()
        ret.__fdata = rpclib.filter_frovedis_dataframe(host, port,
                                                       self.get(), opt.get())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return ret

    def select_frovedis_dataframe(self, targets):
        """
        select_frovedis_dataframe
        """
        ret = FrovedisDataframe()
        ret.__cols = copy.deepcopy(targets) #targets is a list
        ret.__types = [0]*len(targets)
        i = 0

        for item in targets:
            if not item in self.__cols:
                raise ValueError("No column named: ", item)
            else:
                ret.__types[i] = self.__dict__[item].dtype
                ret.__dict__[item] = self.__dict__[item]
                i = i + 1

        vec = np.asarray(targets)
        vv = vec.T # returns self, since ndim=1
        sz = vec.size
        ptr_arr = (c_char_p * sz)()
        ptr_arr[:] = [e.encode('ascii') for e in vv]

        #print("making exrpc request to select requested columns")
        (host, port) = FrovedisServer.getServerInstance()
        ret.__fdata = rpclib.select_frovedis_dataframe(host, port, self.get(),
                                                       ptr_arr, sz)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return ret

    def sort_values(self, by, axis=0, ascending=True,
                    inplace=False, kind='quicksort', na_position='last'):
        """
        sort_values
        """
        if self.__fdata is None:
            raise ValueError("Operation on invalid frovedis dataframe!")
        if type(by).__name__ == 'str':
            sort_by = [by]
        elif type(by).__name__ == 'list':
            sort_by = by
        else:
            raise TypeError("Expected: string|list; Received: ",
                            type(by).__name__)

        for item in sort_by:
            if not item in self.__cols:
                raise ValueError("No column named: ", item)

        vec = np.asarray(sort_by)
        vv = vec.T # returns self, since ndim=1
        sz = vec.size
        ptr_arr = (c_char_p * sz)()
        ptr_arr[:] = [e.encode('ascii') for e in vv]

        ret = FrovedisDataframe()
        ret.__cols = copy.deepcopy(self.__cols)
        ret.__types = copy.deepcopy(self.__types)
        for item in ret.__cols:
            ret.__dict__[item] = self.__dict__[item]

        #Making exrpc request for sorting.
        (host, port) = FrovedisServer.getServerInstance()
        ret.__fdata = rpclib.sort_frovedis_dataframe(host, port, self.get(),
                                                     ptr_arr, sz, ascending)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return ret

    def sort(self, columns=None, axis=0, ascending=True,
             inplace=False, kind='quicksort', na_position='last', **kwargs):
        """
        sort_
        """
        if not columns:
            raise ValueError("Column to be sorted cannot be None!")
        return self.sort_values(by=columns, axis=axis,
                                ascending=ascending,
                                inplace=inplace, kind=kind,
                                na_position=na_position)

    def groupby(self, by=None, axis=0, level=None,
                as_index=True, sort=True, group_keys=True, squeeze=False):
        """
        groupby
        """
        from frovedis.dataframe import grouped_df
        if self.__fdata is None:
            raise ValueError("Operation on invalid frovedis dataframe!")
        if isinstance(by, str):
            g_by = [by]
        elif isinstance(by, list):
            g_by = by
        else:
            raise TypeError("Expected: string|list; Received: ",
                            type(by).__name__)

        types = []
        for item in g_by:
            if not item in self.__cols:
                raise ValueError("No column named: ", item)
            else:
                types.append(self.__dict__[item].dtype)

        vec = np.asarray(g_by)
        vv = vec.T
        sz = vec.size
        ptr_arr = (c_char_p * sz)()
        ptr_arr[:] = [e.encode('ascii') for e in vv]

        (host, port) = FrovedisServer.getServerInstance()
        fdata = rpclib.group_frovedis_dataframe(host, port, self.get(),
                                                ptr_arr, sz)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return grouped_df.FrovedisGroupedDataframe().load_dummy(fdata, g_by,
                                                                types,
                                                                self.__cols,
                                                                self.__types)

    # method to evaluate join keys
    def __evaluate(self, df, left_on, right_on):
        """
        __evaluate
        """
        if(type(left_on).__name__) == 'str' and (type(right_on).__name__) == \
           'str':
            c1 = self.__dict__[left_on]
            c2 = df.__dict__[right_on]
            dfopt = (c1 == c2)
        elif  (type(left_on).__name__) == 'list' and (type(right_on).__name__)\
                == 'list':
            if len(left_on) == len(right_on) and len(left_on) >= 1:
                c1 = self.__dict__[left_on[0]]
                c2 = df.__dict__[right_on[0]]
                dfopt = (c1 == c2)
            for i in range(1, len(left_on)):
                c1 = self.__dict__[left_on[i]]
                c2 = df.__dict__[right_on[i]]
                dfopt = (dfopt & (c1 == c2))
            else:
                raise ValueError("Size of left_on and right_on is \
                                  not matched!")
        else:
            raise TypeError("Invalid key to join!")
        return dfopt

    def merge(self, right, on=None, how='inner', left_on=None, right_on=None,
              left_index=False, right_index=False, sort=False,
              suffixes=['_left', '_right'], copy=True,
              indicator=False, join_type='bcast'):
        """
        merge
        """

        if self.__fdata is None:
            raise ValueError("Operation on invalid frovedis dataframe!")
        df = FrovedisDataframe.asDF(right)

        left = []
        right = []
        for item in self.__cols:
            if item in df.__cols:
                left.append(item)
                left.append(item + suffixes[0])
                right.append(item)
                right.append(item + suffixes[1])
        sz = len(left)
        vec1 = np.asarray(left)
        vec2 = np.asarray(right)
        ptr_left = (c_char_p * sz)()
        ptr_right = (c_char_p * sz)()
        ptr_left[:] = vec1.T
        ptr_right[:] = vec2.T

        if on: #if key name is same in both dataframes
            if(left_on) or (right_on):
                raise ValueError("Can only pass 'on' OR 'left_on' and \
                                  'right_on', not a combination of both!")
            # currently exception at frovedis server
            dfopt = self.__evaluate(df, on, on)
        elif (left_on) and (right_on):
            dfopt = self.__evaluate(df, left_on, right_on)
        else:
            raise ValueError("Key field cannot be None!")

        ret = FrovedisDataframe()
        ret.__cols = self.__cols + df.__cols
        ret.__types = self.__types + df.__types
        for item in self.__cols:
            ret.__dict__[item] = self.__dict__[item]
        for item in df.__cols:
            if item not in ret.__dict__:
                ret.__dict__[item] = df.__dict__[item]

        (host, port) = FrovedisServer.getServerInstance()
        ret.__fdata = \
                    rpclib.merge_frovedis_dataframe(host, port, self.get(),
                                                    df.get(), dfopt.get(),
                                                    how.encode('ascii'),
                                                    join_type.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return ret

    # exception at frovedis server: same key is not
    # currently supported by frovedis
    def join(self, right, on, how='inner',
             lsuffix='_left', rsuffix='_right', sort=False, join_type='bcast'):
        """
        join
        """
        suffix = []
        suffix.append(lsuffix)
        suffix.append(rsuffix)
        if not on:
            raise ValueError("Key to join can not be None!")
        return self.merge(right, on=on, how=how, suffixes=suffix, sort=sort,
                          join_type=join_type)

    # returns new dataframe with renamed columns
    def rename(self, columns):
        """
        rename
        """
        if self.__fdata is None:
            raise ValueError("Operation on invalid frovedis dataframe!")
        if not isinstance(columns, dict):
            raise TypeError("Expected: dictionery; Received: ",
                             type(columns).__name__)
        names = list(columns.keys())
        new_names = list(columns.values())
        ret = FrovedisDataframe()
        ret.__cols = copy.deepcopy(self.__cols)
        ret.__types = copy.deepcopy(self.__types)
        for item in ret.__cols:
            ret.__dict__[item] = self.__dict__[item]

        for i in range(0, len(names)):
            item = names[i]
            new_item = new_names[i]
            if not item in ret.__cols:
                raise ValueError("No column named: ", item)
        else:
            idx = ret.__cols.index(item)
            dt = ret.__types[idx]
            ret.__cols[idx] = new_item
            del ret.__dict__[item]
            ret.__dict__[new_item] = FrovedisColumn(new_item, dt)

        sz = len(names)
        vec1 = np.asarray(names)
        vec2 = np.asarray(new_names)
        name_ptr = (c_char_p * sz)()
        new_name_ptr = (c_char_p * sz)()
        name_ptr[:] = [e.encode('ascii') for e in vec1.T]
        new_name_ptr[:] = [e.encode('ascii') for e in vec2.T]

        (host, port) = FrovedisServer.getServerInstance()
        ret.__fdata = rpclib.rename_frovedis_dataframe(host, port, self.get(),
                                                       name_ptr, new_name_ptr,
                                                       sz)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return ret

    def get(self):
        """
        get
        """
        return self.__fdata

    @staticmethod
    def asDF(df):
        """
        asDF
        """
        if isinstance(df, FrovedisDataframe):
            return df
        elif isinstance(df, pd.DataFrame):
            return FrovedisDataframe(df)
        else: TypeError, "Invalid dataframe type is provided!"

    def min(self, columns):
        """
        min
        """
        if self.__fdata is None:
            raise ValueError("Operation on invalid frovedis dataframe!")
        vtype = type(columns).__name__
        if vtype == 'str':
            by = [columns]
        elif vtype == 'list':
            by = columns
        else:
            raise TypeError("Expected: string|list; Received: ", vtype)
        types = self.__get_column_types(by)
        if DTYPE.STRING in types:
            raise ValueError("Non-numeric column given in min calculation!")
        return self.__get_stat('min', by, types)

    def max(self, columns):
        """
        max
        """
        if self.__fdata is None:
            raise ValueError("Operation on invalid frovedis dataframe!")
        vtype = type(columns).__name__
        if vtype == 'str':
            by = [columns]
        elif vtype == 'list':
            by = columns
        else:
            raise TypeError("Expected: string|list; Received: ", vtype)
        types = self.__get_column_types(by)
        if DTYPE.STRING in types:
            raise ValueError("Non-numeric column given in max calculation!")
        return self.__get_stat('max', by, types)

    def sum(self, columns):
        """
        sum
        """
        if self.__fdata is None:
            raise ValueError("Operation on invalid frovedis dataframe!")
        vtype = type(columns).__name__
        if vtype == 'str':
            by = [columns]
        elif vtype == 'list':
            by = columns
        else:
            raise TypeError("Expected: string|list; Received: ", vtype)
        types = self.__get_column_types(by)
        if DTYPE.STRING in types:
            raise ValueError("Non-numeric column given in sum calculation!")
        return self.__get_stat('sum', by, types)

    def std(self, columns):
        """
        std
        """
        if self.__fdata is None:
            raise ValueError("Operation on invalid frovedis dataframe!")
        vtype = type(columns).__name__
        if vtype == 'str':
            by = [columns]
        elif vtype == 'list':
            by = columns
        else: raise TypeError("Expected: string|list; Received: ", vtype)
        types = self.__get_column_types(by)
        if DTYPE.STRING in types:
            raise ValueError("Non-numeric column given in std calculation!")
        return self.__get_stat('std', by, types)

    def avg(self, columns):
        """
        avg
        """
        if self.__fdata is None:
            raise ValueError("Operation on invalid frovedis dataframe!")
        vtype = type(columns).__name__
        if vtype == 'str':
            by = [columns]
        elif vtype == 'list':
            by = columns
        else:
            raise TypeError("Expected: string|list; Received: ", vtype)
        types = self.__get_column_types(by)
        if DTYPE.STRING in types:
            raise ValueError("Non-numeric column given in avg calculation!")
        return self.__get_stat('avg', by)

    def count(self, columns=None):
        """
        count
        """
        if self.__fdata is None:
            raise ValueError("Operation on invalid frovedis dataframe!")
        # count of all columns
        if not columns:
            return self.__get_stat('count', self.__cols)
        vtype = type(columns).__name__
        if vtype == 'str':
            by = [columns]
        elif vtype == 'list':
            by = columns
        else:
            raise TypeError("Expected: None|string|list; Received: ", vtype)
        return self.__get_stat('count', by)

    def __get_column_types(self, columns):
        """
        __get_column_types
        """
        if type(columns).__name__ != 'list':
            raise TypeError("Expected: list; Received: ", \
                             type(columns).__name__)
        sz = len(columns)
        types = [0]*sz
        for i in range(0, sz):
            item = columns[i]
            if item not in self.__cols:
                raise ValueError("No column named: ", item)
            else:
                idx = self.__cols.index(item)
                types[i] = self.__types[idx]
        return types

    # returns python list of strings
    def __get_stat(self, name, columns, types=None):
        """
        __get_stat
        """
        if type(types).__name__ == 'list' and len(columns) != len(types):
            raise ValueError("Size of inputs doesn't match!")
        if type(name).__name__ != 'str':
            raise TypeError("Expected: string; Received: ", type(name).__name__)
        (host, port) = FrovedisServer.getServerInstance()
        sz = len(columns)
        cols = np.asarray(columns)
        cols_ptr = (c_char_p * sz)()
        cols_ptr[:] = [e.encode('ascii') for e in cols.T]

        if types:
            type_arr = np.asarray(types, dtype=c_short)
            tptr = type_arr.ctypes.data_as(POINTER(c_short))
        if name == 'min':
            if not types:
                raise ValueError("type of target columns is missing for \
                                  min calculation")
            ret = rpclib.get_min_frovedis_dataframe(host, port, self.get(),
                                                    cols_ptr, tptr, sz)
        elif name == 'max':
            if not types:
                raise ValueError("type of target columns is missing for \
                                  max calculation")
            ret = rpclib.get_max_frovedis_dataframe(host, port, self.get(),
                                                    cols_ptr, tptr, sz)
        elif name == 'sum':
            if not types:
                raise ValueError("type of target columns is missing for \
                                  sum calculation")
            ret = rpclib.get_sum_frovedis_dataframe(host, port, self.get(),
                                                    cols_ptr, tptr, sz)
        elif name == 'std':
            if not types:
                raise ValueError("type of target columns is missing for \
                                  std calculation")
            ret = rpclib.get_std_frovedis_dataframe(host, port, self.get(),
                                                    cols_ptr, tptr, sz)
        elif name == 'avg':
            ret = rpclib.get_avg_frovedis_dataframe(host, port, self.get(),
                                                    cols_ptr, sz)
        elif name == 'count':
            ret = rpclib.get_cnt_frovedis_dataframe(host, port, self.get(),
                                                    cols_ptr, sz)
        else:
            raise ValueError("Unknown statistics request is encountered!")
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return ret

    def __get_numeric_columns(self):
        """
        __get_numeric_columns
        """
        cols = []
        for i in range(0, len(self.__cols)):
            if self.__types[i] != DTYPE.STRING:
                cols.append(self.__cols[i])
        return cols

    def describe(self):
        """
        describe
        """
        cols = self.__get_numeric_columns()
        count = self.count(cols)
        mean = self.avg(cols)
        std = self.std(cols)
        total = self.sum(cols)
        minimum = self.min(cols)
        maximum = self.max(cols)
        index = ['count', 'mean', 'std', 'sum', 'min', 'max']
        sz = len(count) # all length should be same
        data = {}
        for i in range(0, sz):
            cc = cols[i]
            val = []
            val.append(count[i])
            val.append(mean[i])
            val.append(std[i])
            val.append(total[i])
            val.append(minimum[i])
            val.append(maximum[i])
            data[cc] = val
        #print(data)
        return pd.DataFrame(data, index=index)

    def to_panda_dataframe(self):
        """
        to_panda_dataframe
        """
        if self.__fdata is None:
            raise ValueError("Operation on invalid frovedis dataframe!")
        cols = self.__cols
        types = self.__types
        sz = len(cols) # lengths of cols and types are same
        data = {}
        (host, port) = FrovedisServer.getServerInstance()
        for i in range(0, sz):
            col_val = rpclib.get_frovedis_col(host, port, self.get(),
                                              cols[i].encode('ascii'),
                                              types[i])
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            data[cols[i]] = col_val
        #print(data)
        return pd.DataFrame(data)

    #default type: float
    def to_frovedis_rowmajor_matrix(self, t_cols, dtype=np.float32):
        """
        to_frovedis_rowmajor_matrix
        """
        if self.__fdata is None:
            raise ValueError("Operation on invalid frovedis dataframe!")
        for item in t_cols: # implicit checks for iterable on 't_cols'
            if item not in self.__cols:
                raise ValueError("No column named: ", item)
        sz = len(t_cols)
        cols = np.asarray(t_cols)
        cols_ptr = (c_char_p * sz)()
        cols_ptr[:] = [e.encode('ascii') for e in cols.T]
        (host, port) = FrovedisServer.getServerInstance()
        if dtype == np.float32:
            dmat = rpclib.df_to_rowmajor(host, port, self.get(),
                                         cols_ptr, sz, DTYPE.FLOAT)
        elif dtype == np.float64:
            dmat = rpclib.df_to_rowmajor(host, port, self.get(),
                                         cols_ptr, sz, DTYPE.DOUBLE)
        else:
            raise TypeError("Supported types: float32/float64; Found: " \
                             + dtype.__name__)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return FrovedisRowmajorMatrix(mat=dmat, dtype=dtype)

    def to_frovedis_colmajor_matrix(self, t_cols, dtype=np.float32):
        """
        to_frovedis_colmajor_matrix
        """
        #default type: float
        if self.__fdata is None:
            raise ValueError("Operation on invalid frovedis dataframe!")
        for item in t_cols: # implicit checks for iterable on 't_cols'
            if item not in self.__cols:
                raise ValueError("No column named: ", item)
        sz = len(t_cols)
        cols = np.asarray(t_cols)
        cols_ptr = (c_char_p * sz)()
        cols_ptr[:] = [e.encode('ascii') for e in cols.T]
        (host, port) = FrovedisServer.getServerInstance()
        if dtype == np.float32:
            dmat = rpclib.df_to_colmajor(host, port, self.get(),
                                         cols_ptr, sz, DTYPE.FLOAT)
        elif dtype == np.float64:
            dmat = rpclib.df_to_colmajor(host, port, self.get(),
                                         cols_ptr, sz, DTYPE.DOUBLE)
        else:
            raise TypeError("Supported types: float32/float64; Found: " \
                            + dtype.__name__)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return FrovedisColmajorMatrix(mat=dmat, dtype=dtype)

    def to_frovedis_crs_matrix(self, t_cols, cat_cols,
                               dtype=np.float32, #default type: float
                               need_info=False):
        """
        to_frovedis_crs_matrix
        """
        if self.__fdata is None:
            raise ValueError("Operation on invalid frovedis dataframe!")
        for item in t_cols: # implicit checks for iterable on 't_cols'
            if item not in self.__cols:
                raise ValueError("No column named: ", item)
        for item in cat_cols: # implicit checks for iterable on 'cat_cols'
            if item not in t_cols:
                raise ValueError("target column list doesn't contain \
                                  categorical column: ", item)
        sz1 = len(t_cols)
        cols = np.asarray(t_cols)
        cols_ptr = (c_char_p * sz1)()
        cols_ptr[:] = [e.encode('ascii') for e in cols.T]
        sz2 = len(cat_cols)
        cat_cols = np.asarray(cat_cols)
        cat_cols_ptr = (c_char_p * sz2)()
        cat_cols_ptr[:] = [e.encode('ascii') for e in cat_cols.T]
        # getting unique id for info to be registered at server side
        info_id = ModelID.get()
        (host, port) = FrovedisServer.getServerInstance()
        if dtype == np.float32:
            dmat = rpclib.df_to_crs(host, port, self.get(),
                                    cols_ptr, sz1,
                                    cat_cols_ptr, sz2,
                                    info_id, DTYPE.FLOAT)
        elif dtype == np.float64:
            dmat = rpclib.df_to_crs(host, port, self.get(),
                                    cols_ptr, sz1,
                                    cat_cols_ptr, sz2,
                                    info_id, DTYPE.DOUBLE)
        else:
            raise TypeError("Supported types: float32/float64; Found: " \
                             + dtype.__name__)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        #default at server: size_t
        crs = FrovedisCRSMatrix(mat=dmat, dtype=dtype, itype=np.int64)
        info = df_to_sparse_info(info_id)

        if need_info:
            return crs, info
        else:
            info.release()
            return crs

    #default type: float
    def to_frovedis_crs_matrix_using_info(self, info, dtype=np.float32):
        """
        to_frovedis_crs_matrix_using_info
        """
        if self.__fdata is None:
            raise ValueError("Operation on invalid frovedis dataframe!")
        if info.get() is None:
            raise ValueError("Operation on invalid frovedis dataframe \
                              conversion info!")
        (host, port) = FrovedisServer.getServerInstance()
        if dtype == np.float32:
            dmat = rpclib.df_to_crs_using_info(host, port, self.get(),
                                               info.get(), DTYPE.FLOAT)
        elif dtype == np.float64:
            dmat = rpclib.df_to_crs_using_info(host, port, self.get(),
                                               info.get(), DTYPE.DOUBLE)
        else:
            raise TypeError("Supported types: float32/float64; Found: " \
                             + dtype.__name__)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        #default at server: size_t
        return FrovedisCRSMatrix(mat=dmat, dtype=dtype, itype=np.int64)
