"""
df.py
"""
#!/usr/bin/env python

import warnings
import copy
import numbers
from ctypes import *
from array import array
import numpy as np
import pandas as pd
from collections import Iterable

from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from ..matrix.dvector import FrovedisDvector
from ..matrix.dvector import FrovedisIntDvector, FrovedisLongDvector
from ..matrix.dvector import FrovedisULongDvector
from ..matrix.dvector import FrovedisFloatDvector, FrovedisDoubleDvector
from ..matrix.dvector import FrovedisStringDvector
from ..matrix.dtype import DTYPE, TypeUtil, get_string_array_pointer
from ..matrix.dense import FrovedisRowmajorMatrix
from ..matrix.dense import FrovedisColmajorMatrix
from ..matrix.crs import FrovedisCRSMatrix
from ..mllib.model_util import ModelID
from .info import df_to_sparse_info
from .frovedisColumn import FrovedisColumn
from .dfoperator import dfoperator
from .dfutil import union_lists, infer_dtype, add_null_column_and_type_cast, \
                    infer_column_type_from_first_notna, get_string_typename
from ..utils import deprecated
from pandas.core.common import SettingWithCopyWarning

def read_csv(filepath_or_buffer, sep=',', delimiter=None,
             header="infer", names=None, index_col=None,
             usecols=None, squeeze=False, prefix=None,
             mangle_dupe_cols=True, dtype=None, na_values=None,
             verbose=False, comment=None, low_memory=True,
             rows_to_see=1024, separate_mb=1024):
    """
    loads frovedis dataframe from csv file
    """

    #TODO: support index_col parameter (currently gets ignored)
    if comment is not None:
        if isinstance(comment, str):
            if len(comment) != 1:
                raise ValueError("read_csv: comment should be either " +\
                                "a string of unit length or None!")
        # TODO: remove the below error when "comment" support would be added in Frovedis
        raise ValueError("read_csv: Frovedis currently doesn't support parameter comment!")
    else:
        comment=""

    if delimiter is None:
        delimiter = sep

    if len(delimiter) != 1:
        raise ValueError("read_csv: Frovedis currently supports only single "
                         "character delimiters!")

    if not isinstance(verbose, bool):
        raise ValueError("read_csv: Frovedis doesn't support "
                         "verbose={0}!".format(verbose))

    if not isinstance(mangle_dupe_cols, bool):
        raise ValueError("read_csv: Frovedis doesn't support "
                         "mangle_dupe_cols={0}!".format(mangle_dupe_cols))
    if names:
        if len(np.unique(names)) != len(names):
            raise ValueError("read_csv: Duplicate names are not allowed.")

    if na_values is None:
        na_values = "NULL"
    elif not isinstance(na_values, str):
        raise ValueError("read_csv: Expected a string or None value "
                         "as for parameter 'na_values'!")
 
    if header not in ("infer", 0, None):
        raise ValueError("read_csv: Frovedis doesn't support "
                         "header={0}!".format(header))

    import csv
    with open(filepath_or_buffer) as f:
        reader = csv.reader(f, delimiter=delimiter)
        row1 = next(reader)
        ncols = len(row1)

    if ncols == 0:
        raise ValueError("read_csv: Frovedis currently doesn't support "
                         "blank line at beginning!")

    if dtype and isinstance(dtype, dict):
        dtype = {key: np.dtype(value).name for (key,value) in dtype.items()}

    add_index = True
    if usecols is not None:
        if all(isinstance(e, int) for e in usecols):
            usecols = np.asarray(usecols, dtype=np.int32)
            if np.min(usecols) < 0 or np.max(usecols) >= ncols:
                raise ValueError("read_csv: usecols index is out-of-bound!")
        else:
            raise ValueError("read_csv: currently Frovedis supports only ids " +\
                             "for usecols parameter!")
    else:
        usecols = np.empty(0, dtype=np.int32)
 
    if names is None:
        if header is None:
            if prefix is None:
                names = [str(i) for i in range(0, ncols)]
            elif isinstance(prefix, str):
                names = [prefix + str(i) for i in range(0, ncols)]
            else:
                raise ValueError("read_csv: Frovedis doesn't support "
                                 "prefix={0}!".format(prefix))
        else:
            names = [] #to be infered from 0th line in input csv file
    else:
        if len(usecols) > 0:
            if len(names) == ncols:
                pass
            elif len(names) == len(usecols):
                # if names is given, frovedis expects all column names to be
                # provided. thus populating dummy names for those columns
                # which are not in usecols
                tnames = ["_c_" + str(i) for i in range(0, ncols)]
                for i in range(len(usecols)):
                    cid = usecols[i]
                    tnames[cid] = names[i]
                names = tnames
            else:
                raise ValueError("read_csv: Passed header names mismatches usecols")
        else:
            if len(names) == ncols - 1:
                names = ["index"] + names
                add_index = False
            elif len(names) > ncols:
                names = names[:ncols] # TODO: support pandas like NaN cols
            elif len(names) < ncols - 1:
                raise ValueError("read_csv: Frovedis currently doesn't support "
                                 "multi-level index loading!")
    index_col_no = -1
    if index_col is None:
        pass
    elif isinstance(index_col, bool):
        if index_col:
            raise ValueError("read_csv: The value of index_col couldn't be 'True'")
        else:
            # dont use 1st col as index
            if names[0] == "index":
                names = names[1:]
                add_index = True
    elif isinstance(index_col, int):
        if index_col < 0 or index_col > (ncols -1 ):
            raise ValueError("read_csv: The value for 'index_col' is invalid!")
        index_col_no = index_col
        if names == []:
            add_index = False
        elif names[0] == "index":
            names = names[1:]
            add_index = False
            if len(names) == ncols - 1:
                names.insert(index_col, "index") 
                #inserted in names, so this is parsed as well   
    else:
        raise ValueError("read_csv: Frovedis currently supports only bool and int types for index_col !")

    single_dtype = None
    types = []
    partial_type_info = False
    if dtype is None:
        dtype = {}
        types = []
    elif isinstance(dtype, dict):
        if not names:
            partial_type_info = True
        elif not set(names).issubset(set(dtype.keys())):
            partial_type_info = True
        else:
            types = [get_string_typename(dtype[e]) for e in names]
    else:
        # single_dtype
        single_dtype = np.dtype(dtype).name #raise exception if dtype not valid
        if names:
            n = len(names)
        else:
            n = ncols
        types = [get_string_typename(single_dtype)] * n

    bool_cols = []
    if isinstance(dtype, dict) :
        bool_cols = [ k for k in dtype if dtype[k] == 'bool' ]

    name_arr = get_string_array_pointer(names)
    type_arr = get_string_array_pointer(types)
    bool_cols_arr = get_string_array_pointer(bool_cols)

    # for partial_type_info
    dtype_keys = []
    dtype_vals = []
    if partial_type_info:
        dtype_keys = list(dtype.keys())
        dtype_vals = [get_string_typename(e) for e in dtype.values()]
    dtype_keys_arr = get_string_array_pointer(dtype_keys)
    dtype_vals_arr = get_string_array_pointer(dtype_vals)

    (host, port) = FrovedisServer.getServerInstance()
    dummy_df = rpclib.load_dataframe_from_csv(host, port, 
                                            filepath_or_buffer.encode('ascii'),
                                            type_arr, name_arr,
                                            len(type_arr), len(name_arr),
                                            delimiter.encode('ascii'),
                                            na_values.encode("ascii"),
                                            comment.encode("ascii"),
                                            rows_to_see, separate_mb,
                                            partial_type_info,
                                            dtype_keys_arr, dtype_vals_arr,
                                            len(dtype_keys), low_memory, add_index,
                                            usecols, len(usecols),
                                            verbose, mangle_dupe_cols, index_col_no,
                                            bool_cols_arr, len(bool_cols_arr))
    excpt = rpclib.check_server_exception()
    if excpt["status"]:
        raise RuntimeError(excpt["info"])

    names = dummy_df["names"]
    types = dummy_df["types"]

    if len(bool_cols) > 0:
        for i in range( len(names) ):
            if names[i] in bool_cols:
                types[i] = DTYPE.BOOL

    res = FrovedisDataframe().load_dummy(dummy_df["dfptr"], \
                                         names[1:], types[1:])
    #TODO: handle index/num_row setting inside load_dummy()
    res.index = FrovedisColumn(names[0], types[0]) #setting index
    res.num_row = dummy_df["nrow"]

    if single_dtype == 'bool':
        res = res.convert_dicstring_to_bool(res.columns, na_values)

    return res

class DataFrame(object):
    """
    DataFrame
    """

    def __init__(self, df=None, need_materialize=False):
        """
        __init__
        """
        self.__fdata = None
        self.__cols = None
        self.__types = None
        self.__need_materialize = need_materialize
        self.index = None
        if df is not None:
            self.load(df)

    def has_index(self):
        """
        has_index
        """
        return self.index is not None

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
            print("\n")

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
    
    def __del__(self):
        """ destructs a dataframe object from server heap """
        if FrovedisServer.isUP(): 
            self.release()

    def __len__(self):
        """
        to be invoked by len() method
        """
        #TODO: set this parameter always
        if "num_row" not in self.__dict__:
            (host, port) = FrovedisServer.getServerInstance()
            self.num_row = rpclib.get_frovedis_dataframe_length(host, port, \
                                                                self.__fdata)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
        return int(self.num_row)

    @property
    def count(self): # similar to spark dataframe 
        """return num_row in input df"""
        return len(self)

    @count.setter
    def count(self, val):
        """count setter"""
        raise AttributeError(\
            "attribute 'count' of DataFrame object is not writable")

    @property
    def dtypes(self):
        """return data types of input df"""
        dt = {}
        for i in range(len(self.__cols)):
            if self.__types[i] == DTYPE.STRING:
                dt[self.__cols[i]] = "object"
            else:
                dt[self.__cols[i]] = \
                TypeUtil.to_numpy_dtype(self.__types[i]).__name__
        return pd.Series(dt)

    @dtypes.setter
    def dtypes(self, val):
        """dtypes setter"""
        raise AttributeError(\
            "attribute 'dtypes' of DataFrame object is not writable")

    @property
    def shape(self):
        """return shape of input df"""
        return (len(self), len(self.__cols))

    @shape.setter
    def shape(self, val):
        """shape setter"""
        raise AttributeError(\
            "attribute 'shape' of DataFrame object is not writable")

    def load(self, df):
        """
        load
        """
        if len(df) == 0:
            raise ValueError("Cannot load an empty pandas dataframe " +\
                             "(column types could not be deduced)")
        if isinstance(df.index, pd.MultiIndex):
            raise ValueError("Cannot load a pandas dataframe " +\
                             "with multi level index")

        self.release()
        self.num_row = len(df)
        cols = df.columns.tolist()
        self.__cols = cols
        indx_name = df.index.name if df.index.name is not None else "index"
        cols = [indx_name] + cols
        size = len(cols)
        types = [0] * size
        dvec = [None] * size

        # null replacements
        null_replacement = {}
        null_replacement['float64'] = np.finfo(np.float64).max
        null_replacement['float32'] = np.finfo(np.float32).max
        null_replacement['int64'] = np.iinfo(np.int64).max
        null_replacement['int32'] = np.iinfo(np.int32).max
        null_replacement['uint64'] = np.iinfo(np.uint).max
        null_replacement['bool'] = np.iinfo(np.int32).max
        null_replacement['bool_'] = np.iinfo(np.int32).max
        null_replacement['str'] = "NULL"

        for idx in range(0, size):
            cname = cols[idx]
            if idx == 0:
                col_vec = df.index
            else:
                col_vec = df[cname]
            vtype = col_vec.dtype.name
            if vtype == 'object': # type-infering required to detect string
                vtype = infer_column_type_from_first_notna(df, \
                                              col_vec, idx == 0)
            try:
                val = col_vec.fillna(null_replacement[vtype])
            except KeyError, e:
                raise ValueError("load: Unsupported type, '%s' of column "
                                 "is deceted" % (vtype))
            #print(cname + ":" + vtype)
            if vtype == 'int32':
                dt = DTYPE.INT
                dvec[idx] = FrovedisIntDvector(val)
            elif vtype == 'int64':
                dt = DTYPE.LONG
                dvec[idx] = FrovedisLongDvector(val)
            elif vtype == 'uint64':
                dt = DTYPE.ULONG
                dvec[idx] = FrovedisULongDvector(val)
            elif vtype == 'float32':
                dt = DTYPE.FLOAT
                dvec[idx] = FrovedisFloatDvector(val)
            elif vtype == 'float64':
                dt = DTYPE.DOUBLE
                dvec[idx] = FrovedisDoubleDvector(val)
            elif vtype == 'str' or vtype == 'str_':
                dt = DTYPE.STRING
                dvec[idx] = FrovedisStringDvector(val)
            elif vtype == 'bool' or vtype == 'bool_':
                dt = DTYPE.BOOL
                dvec[idx] = FrovedisIntDvector(np.asarray(val, dtype=np.int32))
            else:
                raise TypeError("Unsupported column type '%s' in creation of "\
                                "frovedis dataframe: " % (vtype))
            types[idx] = dt
            if idx == 0:
                self.index = FrovedisColumn(cname, dt)   
            else:
                self.__dict__[cname] = FrovedisColumn(cname, dt) #For query purpose

        col_names = get_string_array_pointer(cols)
        dvec_arr = np.asarray([dv.get() for dv in dvec], dtype=c_long)
        dptr = dvec_arr.ctypes.data_as(POINTER(c_long))
        type_arr = np.asarray(types, dtype=c_short)
        tptr = type_arr.ctypes.data_as(POINTER(c_short))
        self.__types = types[1:]

        (host, port) = FrovedisServer.getServerInstance()
        self.__fdata = rpclib.create_frovedis_dataframe(host, port, tptr,
                                                        col_names, dptr, size)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        # dvectors in 'dptr' are already destructed during dataframe
        # construction. thus resetting the metadata to avoid double
        # free issue from server side during destructor calls of the 
        # dvectors in 'dptr'
        for dv in dvec:
            dv.reset()

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

    @property
    def columns(self):
        """returns column list"""
        return self.__cols

    @columns.setter
    def columns(self, newcols):
        """renames columns"""
        if len(newcols) != len(self.__cols):
            raise ValueError("Length mismatch: Expected axis has %d elements"
                             ", new values have %d elements" \
                             % (len(self.__cols), len(newcols)))
        self.rename(columns=dict(zip(self.__cols, newcols)), inplace=True)

    def filter_frovedis_dataframe(self, opt):
        """
        filter_frovedis_dataframe
        """
        ret = DataFrame(need_materialize = True)
        ret.__cols = copy.deepcopy(self.__cols)
        ret.__types = copy.deepcopy(self.__types)
        ret.index = self.index
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
        ret = DataFrame()
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

        if self.has_index():
            targets = [self.index.name] + targets
            ret.index = self.index

        sz = len(targets) 
        ptr_arr = get_string_array_pointer(targets)
        (host, port) = FrovedisServer.getServerInstance()
        ret.__fdata = rpclib.select_frovedis_dataframe(host, port, self.get(),
                                                       ptr_arr, sz)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return ret

    def sort_index(self, axis=0, ascending=True, inplace=False,
                  kind='quicksort', na_position='last'):
        """
        sort_index
        """
        return self.sort_values(by=self.index.name, axis=axis, ascending=ascending, 
                               inplace=inplace, kind=kind, na_position=na_position)

    def sort_values(self, by, axis=0, ascending=True,
                    inplace=False, kind='quicksort', na_position='last'):
        """
        sort_values
        """
        if na_position != "last":
            if na_position == "first":
                raise ValueError("Frovedis currently doesn't support"
                                " na_position='first' !")
            else:
                raise ValueError("invalid na_position: '{}' ".format(na_position))

        if axis not in (0, "index"):
            if axis in (1, "columns"):
                raise ValueError("Frovedis currently doesn't support sorting"
                                " the axis = 1 !")
            else:
                raise ValueError("No axis named {} for frovedis dataframe !"
                                .format(axis))

        if self.__fdata is None:
            raise ValueError("sort: Operation on invalid frovedis dataframe!")
        if type(by).__name__ == 'str':
            sort_by = [by]
        elif type(by).__name__ == 'list':
            sort_by = by
        else:
            raise TypeError("sort: Expected: string|list; Received: ",
                            type(by).__name__)

        for item in sort_by:
            if item not in self.__cols and item != self.index.name:
                raise ValueError("sort: No column named: ", item)

        sz = len(sort_by) 
        sort_by_arr = get_string_array_pointer(sort_by)
        
        if type(ascending).__name__ == 'bool':
            orderlist = [ascending] * sz
            sort_order = np.asarray(orderlist, dtype=np.int32)
        elif type(ascending).__name__ == 'list':
            if len(ascending) != sz:
                raise ValueError("sort: Length of by and ascending parameters"
                                " are not matching!")
            sort_order = np.asarray(ascending, dtype=np.int32)
        else:
            dgt = str(ascending).isdigit()
            if dgt:
                orderlist = [bool(ascending)] * sz
                sort_order = np.asarray(orderlist, dtype=np.int32)
            else:
                raise TypeError("sort: Expected: digit|list; Received: ",
                                type(ascending).__name__)

        ret = DataFrame(need_materialize = True)
        ret.__cols = copy.deepcopy(self.__cols)
        ret.__types = copy.deepcopy(self.__types)
        for item in ret.__cols:
            ret.__dict__[item] = self.__dict__[item]
        ret.index = self.index

        #Making exrpc request for sorting.
        (host, port) = FrovedisServer.getServerInstance()
        ret.__fdata = rpclib.sort_frovedis_dataframe(host, port, self.get(),
                                                     sort_by_arr, sort_order, sz)
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
            if item not in self.__cols:
                raise ValueError("No column named: ", item)
            else:
                types.append(self.__dict__[item].dtype)

        sz = len(g_by) 
        ptr_arr = get_string_array_pointer(g_by)
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
    def __evaluate_join_condition(self, df_right, left_on, right_on):
        """
        __evaluate_join_condition
        """
        # assumes left_on, right_on as python list
        if len(left_on) != len(right_on):
            raise ValueError("Size of left_on and right_on is" +
                             " not matched!")
        #for renaming
        col_dict = {}
        for i in range(len(left_on)):
            if left_on[i] == right_on[i]:
                tmp = right_on[i] + "_right"
                col_dict[right_on[i]] = tmp
                right_on[i] = tmp
        df_right = df_right.rename(col_dict)
        renamed_key = list(col_dict.values()) # [] if no renaming is performed

        # for dfopt combining
        sz = len(left_on)
        left_on_arr = get_string_array_pointer(left_on)
        right_on_arr = get_string_array_pointer(right_on)

        (host, port) = FrovedisServer.getServerInstance()
        dfopt_proxy = rpclib.get_multi_eq_dfopt(host, port, left_on_arr,
                                                right_on_arr, sz)
        dfopt = dfoperator(dfopt_proxy)
        return (df_right, dfopt, renamed_key, right_on)

    def __get_frovedis_how(self, how):
        pandas_to_frov_how = { "left": "outer",
                               "inner": "inner" }
        if how not in pandas_to_frov_how:
            raise ValueError("Frovedis currently doesn't support how={0}!".format(how))
        return pandas_to_frov_how[how]

    def merge(self, right, on=None, how='inner', left_on=None, right_on=None,
              left_index=False, right_index=False, sort=False,
              suffixes=('_x', '_y'), copy=True,
              indicator=False, join_type='bcast'):
        """
        merge
        """

        if self.__fdata is None:
            raise ValueError("Operation on invalid frovedis dataframe!")
        right = DataFrame.asDF(right)

        # no of nulls
        n_nulls = sum(x is None for x in (on, right_on, left_on))
        if n_nulls == 3:
            if self is right: 
                # Early return: all columns are common => all comlumns are treated as keys
                # returning a copy of self
                ret = self.select_frovedis_dataframe(self.columns)
                ret.reset_index(drop=True, inplace=True)
                return ret
            common_cols = list(set(self.columns) & set(right.columns))
            if len(common_cols) == 0:
                raise ValueError("No common columns to perform merge on.")
            left_on = right_on = common_cols

        if on: #if key name is same in both dataframes
            if(left_on) or (right_on):
                raise ValueError("Can only pass 'on' OR 'left_on' and" +
                                 " 'right_on', not a combination of both!")
            left_on = right_on = on
        elif (left_on and not right_on) or (not left_on and right_on):
            raise ValueError("Both left_on and right_on need to be provided."+
                             " In case of common keys, use 'on' parameter!")

        if not isinstance(left_on, (tuple, list)):
            left_keys = [left_on]
        else:
            left_keys = list(left_on)

        if not isinstance(right_on, (tuple, list)):
            right_keys = [right_on]
        else:
            right_keys = list(right_on)

        # right, right_keys may be modified if keys are same ( in __evaluate_join_condition())
        right, dfopt, renamed_keys, right_keys = \
            self.__evaluate_join_condition(right, left_keys, right_keys)

        left_non_key_cols = set(self.__cols) - set(left_keys)
        right_non_key_cols = set(right.__cols) - set(right_keys)
        common_non_key_cols = left_non_key_cols & right_non_key_cols

        # index rename for right
        if right.has_index(): 
            right = right.rename_index(right.index.name + "_right") 

        # if renaming required add suffixes
        df_left = self
        df_right = right
        if len(common_non_key_cols) > 0:
            renamed_left_cols = {}
            renamed_right_cols = {}
            lsuf, rsuf = suffixes
            if lsuf == '' and rsuf == '':
                raise ValueError("columns overlap but no suffix specified: %s " \
                                % common_non_key_cols)
            for e in common_non_key_cols:
                renamed_left_cols[e] = e + str(lsuf)
                renamed_right_cols[e] = e + str(rsuf)
            if lsuf != '':
                df_left = self.rename(renamed_left_cols)
            if rsuf != '':
                df_right = right.rename(renamed_right_cols)

        ret = DataFrame(need_materialize = True)
        ret.__cols = df_left.__cols + df_right.__cols
        ret.__types = df_left.__types + df_right.__types
        for item in df_left.__cols:
            ret.__dict__[item] = df_left.__dict__[item]
        for item in df_right.__cols:
            ret.__dict__[item] = df_right.__dict__[item]

        frov_how = self.__get_frovedis_how(how)

        (host, port) = FrovedisServer.getServerInstance()
        ret.__fdata = \
                    rpclib.merge_frovedis_dataframe(host, port, df_left.get(),
                                                    df_right.get(), dfopt.get(),
                                                    frov_how.encode('ascii'),
                                                    join_type.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        if renamed_keys:
            targets = list(ret.__cols)
            for key in renamed_keys:
                targets.remove(key)
            res = ret.select_frovedis_dataframe(targets)
        else:
            res = ret.select_frovedis_dataframe(ret.columns)

        return res.add_index("index")

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
    def rename(self, columns, inplace=False):
        """
        rename
        """
        if not columns:
            return self
        if self.__fdata is None:
            raise ValueError("Operation on invalid frovedis dataframe!")
        if not isinstance(columns, dict):
            raise TypeError("Expected: dictionary; Received: ",
                            type(columns).__name__)

        names = list(columns.keys())
        new_names = list(columns.values())
        if not inplace:
            ret = DataFrame()
            ret.__cols = copy.deepcopy(self.__cols)
            ret.__types = copy.deepcopy(self.__types)
            for item in ret.__cols:
                ret.__dict__[item] = self.__dict__[item]
            ret.index = self.index
        else:
            ret = self

        t_names = []
        t_new_names = []
        for i in range(0, len(names)):
            item = names[i]
            new_item = new_names[i]
            if item in ret.__cols: # otherwise skipped as in pandas rename()
                idx = ret.__cols.index(item)
                dt = ret.__types[idx]
                ret.__cols[idx] = new_item
                del ret.__dict__[item]
                ret.__dict__[new_item] = FrovedisColumn(new_item, dt)
                t_names.append(item)
                t_new_names.append(new_item)

        sz = len(t_names)
        name_ptr = get_string_array_pointer(t_names)
        new_name_ptr = get_string_array_pointer(t_new_names)
        (host, port) = FrovedisServer.getServerInstance()
        ret.__fdata = rpclib.rename_frovedis_dataframe(host, port, self.get(),
                                                       name_ptr, new_name_ptr,
                                                       sz, inplace)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return None if inplace else ret 

    def rename_index(self, new_name, inplace=False): 
        """ renames index field of self inplace """
        if not self.has_index():
            raise ValueError("rename_index: no index field for renaming!")
        if not inplace:
            ret = DataFrame()
            ret.__cols = copy.deepcopy(self.__cols)
            ret.__types = copy.deepcopy(self.__types)
            for item in ret.__cols:
                ret.__dict__[item] = self.__dict__[item]
        else:
            ret = self

        sz = 1
        name_ptr = get_string_array_pointer([self.index.name])
        new_name_ptr = get_string_array_pointer([new_name])

        (host, port) = FrovedisServer.getServerInstance()
        ret.__fdata = rpclib.rename_frovedis_dataframe(host, port, \
                              self.get(), name_ptr, new_name_ptr, \
                              sz, inplace)
        ret.index = FrovedisColumn(new_name, self.index.dtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return None if inplace else ret 

    def copy(self, deep=True):
        """
        copy input dataframe to construct a new dataframe
        """
        if not deep:
            raise NotImplementedError("copy: shallow-copy is not yet supported!")
        return self[self.columns]

    def get(self):
        """
        get proxy of dataframe at server side
        """
        return self.__fdata

    @staticmethod
    def asDF(df):
        """
        asDF
        """
        if isinstance(df, DataFrame):
            return df
        elif isinstance(df, pd.DataFrame):
            return DataFrame(df)
        else: 
            raise TypeError("Invalid dataframe type is provided!")

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
        if len(columns) == 0:
            return []
        if isinstance(types, list) and len(columns) != len(types):
            raise ValueError("Size of inputs doesn't match!")
        if not isinstance(name, str):
            raise TypeError("Expected: string; Received: ", type(name).__name__)
        (host, port) = FrovedisServer.getServerInstance()
        sz = len(columns)
        cols_ptr = get_string_array_pointer(columns)

        if types:
            type_arr = np.asarray(types, dtype=c_short)
            tptr = type_arr.ctypes.data_as(POINTER(c_short))
        if name == 'min':
            if not types:
                raise ValueError("type of target columns is missing for"
                                 " min calculation")
            ret = rpclib.get_min_frovedis_dataframe(host, port, self.get(),
                                                    cols_ptr, tptr, sz)
        elif name == 'max':
            if not types:
                raise ValueError("type of target columns is missing for"
                                 " max calculation")
            ret = rpclib.get_max_frovedis_dataframe(host, port, self.get(),
                                                    cols_ptr, tptr, sz)
        elif name == 'sum':
            if not types:
                raise ValueError("type of target columns is missing for"
                                 " sum calculation")
            ret = rpclib.get_sum_frovedis_dataframe(host, port, self.get(),
                                                    cols_ptr, tptr, sz)
        elif name == 'std':
            ret = rpclib.get_std_frovedis_dataframe(host, port, self.get(),
                                                    cols_ptr, sz)
        elif name == 'avg' or name == 'mean':
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

    def __get_stat_wrapper(self, func_name, columns):
        if self.__fdata is None:
            raise ValueError("Operation on invalid frovedis dataframe!")
        if isinstance(columns, str):
            columns = [columns]
        elif isinstance(columns, list):
            pass
        else:
            raise TypeError("Expected: string|list; Received: ", type(columns))
        
        types = self.__get_column_types(columns)
        if func_name == "count":
            return self.__get_stat('count', columns, types)

        numeric_cols = []
        numeric_cols_types = []
        for i in range(len(columns)):
            if types[i] != DTYPE.STRING:
                numeric_cols.append(columns[i])
                numeric_cols_types.append(types[i])

        ret = self.__get_stat(func_name, numeric_cols, numeric_cols_types)
        result_dict = dict(zip(numeric_cols, ret))
        final_res = [result_dict.get(col, np.nan) for col in columns]
        return final_res

    def __get_numeric_columns(self):
        """
        __get_numeric_columns
        """
        cols = []
        non_numeric_types = [DTYPE.STRING, DTYPE.BOOL]
        for i in range(0, len(self.__cols)):
            if self.__types[i] not in non_numeric_types:
                cols.append(self.__cols[i])
        return cols

    def describe(self):
        """
        describe
        """
        cols = self.__get_numeric_columns()
        index = ['count', 'mean', 'std', 'sum', 'min', 'max']
        return self.__agg_impl(cols, index)

    def __agg_impl(self, cols, index):
        """
        agg impl
        """
        func_ret = []
        for ind in index:
            func_ret.append(self.__get_stat_wrapper(ind, cols))
        ret = pd.DataFrame(func_ret, index=index, columns=cols)

        ncols = len(cols)
        types = self.__get_column_types(cols)
        has_max = 'max' in ret.index
        has_min = 'min' in ret.index
        # agg func list has std, avg/mean, then dtype = float64
        if ("mean" in index) or ("avg" in index) or ("std" in index):
            rtypes = [np.float64] * ncols
        else:
            rtypes = [np.int32 if x == DTYPE.BOOL \
                      else TypeUtil.to_numpy_dtype(x) for x in types]
        # here we are type-casting column one-by-one.
        # since some pandas version has issue in casting all
        # target columns with dictionary input for astype()
        # REF: https://github.com/pandas-dev/pandas/issues/21445
        warnings.simplefilter(action="ignore", category=SettingWithCopyWarning)
        for i in range(0, ncols):
            ret[cols[i]] = ret[cols[i]].astype(rtypes[i])
            # special treatement for bool columns
            if types[i] == DTYPE.BOOL:
                if has_max:
                    tmp = ret[cols[i]]['max']
                    ret[cols[i]]['max'] = True if tmp == 1 else False
                if has_min:
                    tmp = ret[cols[i]]['min']
                    ret[cols[i]]['min'] = True if tmp == 1 else False
        warnings.simplefilter(action="default", category=SettingWithCopyWarning)
        return ret 

    def agg(self, func):
        if isinstance(func, str):
            return self.__agg_list([func]).transpose()[func]
        elif isinstance(func, list):
            return self.__agg_list(func)
        elif isinstance(func, dict):
            return self.__agg_dict(func)
        else:
            raise ValueError("Unsupported 'func' type : {0}".format(type(func)))

    def __agg_list(self, func):
        return self.__agg_impl(cols=self.columns, index=func)

    def __agg_dict(self, func_dict):
        from itertools import chain 
        func_dict = dict(func_dict)
        for col in func_dict:
            if isinstance(func_dict[col], str):
                func_dict[col] = [func_dict[col]]

        all_cols = list(func_dict.keys())
        all_funcs = list(set(chain.from_iterable(func_dict.values())))

        df_all_res = self.__agg_impl(cols=all_cols, index=all_funcs)
        data_dict = {col: {} for col in all_cols}

        for col in func_dict:
            for func in func_dict[col]:
                data_dict[col][func] = df_all_res[col][func]

        return pd.DataFrame(data_dict)

    def to_pandas_dataframe(self):
        """
        returns a pandas dataframe object from frovedis dataframe
        """
        if self.__fdata is None:
            raise ValueError("Operation on invalid frovedis dataframe!")
        cols = copy.deepcopy(self.__cols)
        types = copy.deepcopy(self.__types)
        if self.has_index():
            cols.append(self.index.name)
            types.append(self.index.dtype)

        # null replacements
        null_replacement = {}
        null_replacement[DTYPE.DOUBLE] = np.finfo(np.float64).max
        null_replacement[DTYPE.FLOAT] = np.finfo(np.float32).max
        null_replacement[DTYPE.ULONG] = np.iinfo(np.uint).max
        null_replacement[DTYPE.LONG] = np.iinfo(np.int64).max
        null_replacement[DTYPE.INT] = np.iinfo(np.int32).max
        null_replacement[DTYPE.STRING] = "NULL"
        int2bool = {0: False, 1: True, np.iinfo(np.int32).max: np.nan}

        res = pd.DataFrame()
        (host, port) = FrovedisServer.getServerInstance()

        for i in range(0, len(cols)):
            dvec = rpclib.get_frovedis_col(host, port, self.get(),
                                           cols[i].encode('ascii'),
                                           types[i])
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])

            col_val = FrovedisDvector(dvec).to_numpy_array()
            if types[i] == DTYPE.BOOL:
                res[cols[i]] = [int2bool[x] for x in col_val]
            else:
                null_val = null_replacement[types[i]]
                res[cols[i]] = [np.nan if x == null_val else x for x in col_val]

        if self.has_index():
            res.set_index(self.index.name, inplace=True)
        return res

    @deprecated("Use to_pandas_dataframe() instead!\n")
    def to_panda_dataframe(self):
        return self.to_pandas_dataframe()

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
        cols_ptr = get_string_array_pointer(t_cols)
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
        cols_ptr = get_string_array_pointer(t_cols)
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
                raise ValueError("target column list doesn't contain"
                                 " categorical column: ", item)
        sz1 = len(t_cols)
        cols_ptr = get_string_array_pointer(t_cols)
        sz2 = len(cat_cols)
        cat_cols_ptr = get_string_array_pointer(cat_cols)
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
            raise ValueError("Operation on invalid frovedis dataframe"
                             " conversion info!")
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

    def filter(self, items=None, like=None, regex=None, axis=None):
        """
        Subset the dataframe rows or columns according to the specified index labels.
        """
        # no of keyword args
        nkw = sum(x is not None for x in (items, like, regex))
        # only 1 can be not-None
        if nkw > 1:
            raise TypeError(
                "Keyword arguments `items`, `like`, or `regex` "
                "are mutually exclusive"
            )

        if axis not in (None, 0, 1, 'columns', 'index'):
            raise ValueError("filter(): Unsupported axis '%s' is"+
                             " provided!" % str(axis))

        # default axis = 1 , i.e. column names
        if axis is None or axis == 'columns':
            axis = 1
        elif axis == 0 or axis == 'index':
            raise ValueError("filter() on 'index' axis is not supported for"+
                             "frovedis DataFrame!")

        if items is not None:
            targets = list(items)
        elif like is not None:
            def func(x):
                """ used for filtering """
                return like in x
            targets = list(filter(func, self.__cols))
        elif regex is not None:
            import re
            pattern = re.compile(regex)
            def func(x):
                """ used for filtering """
                return pattern.search(x) is not None
            targets = list(filter(func, self.__cols))
        else:
            raise TypeError("Must pass either `items`, `like`, or `regex`")

        return self.select_frovedis_dataframe(targets)

    def apply(self, func, axis=0, raw=False, \
              result_type=None, args=(), **kwds):
        return self.to_pandas_dataframe()\
                   .apply(func, axis, raw, result_type, args)

    def isna(self):
        ncol = len(self.__cols)
        cols_ptr = get_string_array_pointer(self.__cols)
        (host, port) = FrovedisServer.getServerInstance()
        ret = DataFrame()
        ret.__fdata = rpclib.isnull_frovedis_dataframe(host, port, self.get(),
                                                       cols_ptr, ncol)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        if self.has_index():
            ret.__copy_index(self, inplace=True) # sets all metadata as well
        else:
            ret.__cols = copy.deepcopy(self.__cols)

        # modify column type metadata INT -> BOOL 
        # required for to_pandas_dataframe()
        for i in range(0, len(ret.__cols)):
            item = ret.__cols[i]
            ret.__types[i] = DTYPE.BOOL 
            ret.__dict__[item] = FrovedisColumn(item, DTYPE.BOOL)
        return ret

    def isnull(self):
        return self.isna()

    def drop_duplicates(self, subset=None, keep='first', inplace=False, \
                        ignore_index=False):
        """ 
        drops duplicate rows for specified columns (None: all columns).
        """
        if subset is None:
            targets = self.columns # for all columns by default
        else: # must be an iterable
            targets = []
            for col in np.unique(subset): # handling duplicate columns
                if col in self.__cols:
                    targets.append(col)
                else:
                    raise KeyError("Index[" + col + "] is not found!")

        if keep != 'first' and keep != 'last':
            raise ValueError("drop_duplicates: currently supports only " \
                             "'first' or 'last' as for 'keep' parameter!")
        sz = len(targets)
        targets_ptr = get_string_array_pointer(targets)
        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.drop_frovedis_duplicate_rows(host, port, \
                           self.get(), targets_ptr, sz, \
                           keep.encode('ascii'), self.__need_materialize)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret = self if inplace else DataFrame()
        ret.num_row = dummy_df["nrow"]
        if self.has_index():
            ret.index = FrovedisColumn(names[0], types[0]) #setting index
            ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        else:
            ret.load_dummy(dummy_df["dfptr"], names[0:], types[0:])
        if ignore_index:
            ret.reset_index(drop=True, inplace=True)
        return None if inplace else ret
       
    def drop(self, labels=None, axis=0, index=None, columns=None, \
             level=None, inplace=False, errors='raise'):
        """ drops specified labels from rows or columns. """
        if columns is not None:
            ret = self.drop_cols(targets=columns, inplace=inplace)
        elif index is not None:
            ret = self.drop_rows(targets=index, inplace=inplace)
        else:
            if labels is None:
                raise TypeError("drop() takes at least 2 arguments (1 given)")
            if axis == 0:
                ret = self.drop_rows(targets=labels, inplace=inplace)
            elif axis == 1:
                ret = self.drop_cols(targets=labels, inplace=inplace)
            else:
                raise ValueError("drop(): No axis named %d for " \
                      "object type %s" % (axis, self.__class__))
        return ret 
                
    def drop_cols(self, targets, inplace=False):
        """ drops specified columns from input dataframe. """
        if isinstance(targets, str):
            targets = [targets]
        elif isinstance(targets, Iterable):
            targets = np.unique(targets)
        else:
            raise ValueError("drop: given columns must be an iterable!")

        if inplace:
            for item in targets:
                try:
                    idx = self.__cols.index(item)
                    self.__cols.pop(idx)
                    self.__types.pop(idx)
                    del self.__dict__[item]
                except ValueError as ve: #item might not be present in col-list
                    raise ValueError("drop: No column named: ", item)

            sz = len(targets)
            targets_ptr = get_string_array_pointer(targets)
            (host, port) = FrovedisServer.getServerInstance()
            rpclib.drop_frovedis_dataframe_columns(host, port, \
                                       self.get(), targets_ptr, sz)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
        else:
            cols = list(self.__cols)
            for item in targets:
                if item not in cols:
                    raise ValueError("drop: No column named: ", item)
                else:
                    cols.remove(item)
            return self[cols] # just select the required columns

    def drop_rows(self, targets, inplace=False):
        """ drops specified columns from input dataframe. """
        if not self.has_index():
            raise ValueError("drop(): input dataframe doesn't " \
                             "have any index column!")

        if isinstance(targets, str):
            targets = [targets]
        if isinstance(targets, numbers.Number):
            targets = [targets]
        elif isinstance(targets, Iterable):
            targets = np.unique(targets)
        else:
            raise ValueError("drop: given indices must be an iterable!")

        sz = len(targets)
        dtype = self.index.dtype
        index_col = self.index.name.encode('ascii')
        (host, port) = FrovedisServer.getServerInstance()

        if dtype == DTYPE.INT:
            targets = np.asarray(targets, dtype=np.int32)
            targets_ptr = targets.ctypes.data_as(POINTER(c_int))
            dummy_df = rpclib.drop_frovedis_dataframe_rows_int(host, port, \
                                          self.get(), targets_ptr, sz, \
                                          index_col)
        elif dtype == DTYPE.LONG:
            targets = np.asarray(targets, dtype=np.int64)
            targets_ptr = targets.ctypes.data_as(POINTER(c_long))
            dummy_df = rpclib.drop_frovedis_dataframe_rows_long(host, port, \
                                          self.get(), targets_ptr, sz, \
                                          index_col)
        elif dtype == DTYPE.ULONG:
            targets = np.asarray(targets, dtype=np.uint)
            targets_ptr = targets.ctypes.data_as(POINTER(c_ulong))
            dummy_df = rpclib.drop_frovedis_dataframe_rows_ulong(host, port, \
                                          self.get(), targets_ptr, sz, \
                                          index_col)
        elif dtype == DTYPE.FLOAT:
            targets = np.asarray(targets, dtype=np.float32)
            targets_ptr = targets.ctypes.data_as(POINTER(c_float))
            dummy_df = rpclib.drop_frovedis_dataframe_rows_float(host, port, \
                                          self.get(), targets_ptr, sz, \
                                          index_col)
        elif dtype == DTYPE.DOUBLE:
            targets = np.asarray(targets, dtype=np.float64)
            targets_ptr = targets.ctypes.data_as(POINTER(c_double))
            dummy_df = rpclib.drop_frovedis_dataframe_rows_double(host, port, \
                                          self.get(), targets_ptr, sz, \
                                          index_col)
        elif dtype == DTYPE.STRING:
            targets_ptr = get_string_array_pointer(targets)
            dummy_df = rpclib.drop_frovedis_dataframe_rows_str(host, port, \
                                          self.get(), targets_ptr, sz, \
                                          index_col)
        else:
            raise TypeError(\
            "drop(): Unsupported index column dtype is detected!")

        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        names = dummy_df["names"]
        types = dummy_df["types"]
        ret = self if inplace else DataFrame()
        ret.index = FrovedisColumn(names[0], types[0]) #setting index
        ret.num_row = dummy_df["nrow"]
        ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        return None if inplace else ret

    def convert_dicstring_to_bool(self, col_names, nullstr):
        """ converts dicstring columns to bool"""
        curr_types = self.__get_column_types(col_names)
        for t in curr_types:
            if t != DTYPE.STRING:
                raise TypeError("convert_dicstring_to_bool: Conversion to "
                               "boolean is supported for string columns only!")

        name_arr = get_string_array_pointer(col_names)
        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_convert_dicstring_to_bool(host, port, self.get(),
                                                       name_arr, len(name_arr),
                                                       nullstr.encode("ascii"),
                                                       self.__need_materialize)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        names = dummy_df["names"]
        types = dummy_df["types"]

        for i in range( len(names) ):
            if names[i] in col_names:
                types[i] = DTYPE.BOOL

        self.index = FrovedisColumn(names[0], types[0]) #setting index
        self.num_row = dummy_df["nrow"]
        self.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        return self

    def __get_dvec(self, dtype, val):
        """
        get dvector from numpy array
        """
        dvec_constructor_map = { DTYPE.INT: FrovedisIntDvector,
                                 DTYPE.LONG: FrovedisLongDvector,
                                 DTYPE.ULONG: FrovedisULongDvector,
                                 DTYPE.FLOAT: FrovedisFloatDvector,
                                 DTYPE.DOUBLE: FrovedisDoubleDvector,
                                 DTYPE.STRING: FrovedisStringDvector,
                                 DTYPE.BOOL: FrovedisIntDvector
                                }
        if dtype not in dvec_constructor_map:
            raise ValueError("Unsupported dtype for creating dvector: {0}!\n"
                            .format(dtype))
        if dtype == DTYPE.BOOL:
            val = np.array(val).astype(np.int32)
        res = dvec_constructor_map[dtype](val)
        return res

    def __append_column(self, col_name, data_type, data, position=None,
                        drop_old=False):
        if position is None:
            pos = -1
        elif isinstance(position, int):
            if position < 0 or position > (len(self.columns)):
                raise ValueError("Invalid position for appending column : {} !"
                                .format(position))
            if self.has_index():
                pos = position + 1 # 0 is reserved for index column
            else:
                pos = position
        else:
            raise TypeError("Invalid type for position, for appending "
                           "column: {} !".format(position))

        # null replacements
        null_replacement = {}
        null_replacement['float64'] = np.finfo(np.float64).max
        null_replacement['float32'] = np.finfo(np.float32).max
        null_replacement['int64'] = np.iinfo(np.int64).max
        null_replacement['int32'] = np.iinfo(np.int32).max
        null_replacement['uint64'] = np.iinfo(np.uint).max
        null_replacement['bool'] = np.iinfo(np.int32).max
        null_replacement['bool_'] = np.iinfo(np.int32).max
        null_replacement['str'] = "NULL"

        #replacing None with np.nan
        data = np.array(list(map(lambda x: np.nan if x==None else x, data)))
        if data_type.char in ("U", "S"):
            data = np.array(list(map(lambda x: null_replacement["str"] \
                                    if x=="nan" else x, data)))
        else:
            nan_idx = np.isnan(data)
            data[nan_idx] = null_replacement[data_type.name]

        data_type = TypeUtil.to_id_dtype(data_type)
        is_bool = (data_type == DTYPE.BOOL)
        dvec = self.__get_dvec(data_type, data)
        (host, port) = FrovedisServer.getServerInstance()
        df_proxy = -1
        if self.__fdata:
            df_proxy = self.get()
        dummy_df = rpclib.df_append_column(host, port, df_proxy,
                                           col_name.encode("ascii"),
                                           data_type, dvec.get(),
                                           pos, self.__need_materialize,
                                           drop_old)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        # dvector 'dvec' is already destructed during append column.
        # thus resetting the metadata to avoid double free issue from
        # server side during destructor call of the dvector 'dvec'
        dvec.reset()

        names = dummy_df["names"]
        types = dummy_df["types"]

        if is_bool:
            ind = names.index(col_name)
            types[ind] = DTYPE.BOOL

        self.num_row = dummy_df["nrow"]
        if self.has_index() or \
           df_proxy == -1: # empty dataframe case: new index column is added
            self.index = FrovedisColumn(names[0], types[0]) #setting index
            self.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        else:
            self.load_dummy(dummy_df["dfptr"], names[0:], types[0:])
        return self

    def __get_dtype_name(self, arr):
        if arr.dtype.name != 'object':
            if arr.dtype.char in ("U", "S"):
                return "str"
            else:
                return arr.dtype.name

        # if dtype is object
        arr_without_nan = np.array(list(filter(lambda x: x == x, arr)))
        if arr_without_nan.dtype.name != 'object':
            if arr_without_nan.dtype.char in ("U", "S"):
                return "str"
            else:
                return arr_without_nan.dtype.name
        elif all(isinstance(e, str) for e in arr_without_nan):
            return "str"
        else:
            # mixed dtypes
            raise ValueError("Cannot convert the values for the given data! ",
                            arr)

        return res

    def __setitem__(self, key, value):
        """
        Interface for appending column into DataFrame
        """
        if not isinstance(key, str):
            raise ValueError("__setitem__: Column name must be specified as "
                             "string only!")

        if np.isscalar(value):
            if self.__fdata is None:
                raise ValueError("__setitem__: Frovedis currently doesn't "
                                 "support adding column with scalar values "
                                 "on empty dataframe!")
            is_imax = value == np.iinfo(np.int32).max
            value = [value] * len(self)
            if is_imax: # check to avoid long array construction by default
                value = np.asarray(value, dtype=np.int32)
            else:
                value = np.asarray(value) # deduce type
        elif not isinstance(value, Iterable):
            raise ValueError("__setitem__: Given column data type '{}' is "
                             "not supported!".format(type(value)))

        col_data = np.asarray(value)
        col_dtype = self.__get_dtype_name(col_data)

        drop_old = False
        if self.__fdata and key in self.columns:
            drop_old = True

        self = self.__append_column(key, np.dtype(col_dtype), col_data, \
                                    None, drop_old)
        return self

    def insert(self, loc, column, value, allow_duplicates=False):
        """
        Insert column into DataFrame at specified location.
        """
        if allow_duplicates == True:
            raise ValueError("insert: Frovedis does not support duplicate "
                             "column names !")

        if np.isscalar(value):
            if self.__fdata is None:
                raise ValueError("insert: Frovedis currently doesn't "
                                 "support adding column with scalar values "
                                 "on empty dataframe!")

            is_imax = value == np.iinfo(np.int32).max
            value = [value] * len(self)
            if is_imax: # check to avoid long array construction by default
                value = np.asarray(value, dtype=np.int32)
            else:
                value = np.asarray(value) # deduce type
        elif not isinstance(value, Iterable):
            raise ValueError("insert: Given column data type '{}' is "
                             "not supported!".format(type(value)))

        col_data = np.array(value)
        col_dtype = self.__get_dtype_name(col_data)

        #insert in empty df, allowed loc = 0 only
        if self.__fdata is None:
            if loc != 0:
                raise IndexError("insert: index '{}' is out of bounds for the "
                                 "given dataframe!".format(loc))
            self = self.__append_column(column, np.dtype(col_dtype), col_data)
        else:
            if column in self.columns:
                raise ValueError("insert: The given column '{}' already exists !"
                                .format(column))
            self = self.__append_column(column, np.dtype(col_dtype), col_data,
                                       loc)
        return self

    def update_index(self, value, key=None, \
                     verify_integrity=False, inplace=False):
        """
        updates/sets index values: pandas-like df.index = [...]
        """
        if inplace: # copy may still take place, if self needs materialize 
            ret = self
        else:
            ret = self.copy()
     
        if not isinstance(value, Iterable):
            raise TypeError("given index value must be an iterable!")
        if verify_integrity:
            if len(np.unique(value)) != len(value):
                raise ValueError("given index values are not unique!")

        col_data = np.asarray(value)
        col_dtype = self.__get_dtype_name(col_data)

        if ret.has_index():
            if key is not None: #rename required
                if ret.index.name != key: 
                    ret.rename_index(key, inplace=True) 
            drop_old = True   # existing index column will be dropped
            position = None   # adds given column in same position (as in existing index)
        else:
            if key is None:
                key = "index" # default name to index-column
            drop_old = False  # no index column to drop
            position = 0      # adds given column as index in 0th position
        ret = ret.__append_column(key, np.dtype(col_dtype), col_data, \
                                  position, drop_old)
        return None if inplace else ret

    #TODO: support drop=False, multi-level index
    def set_index(self, keys, drop=True, append=False,
                  inplace=False, verify_integrity=False): 
        """ sets the index column of self """
        if not drop:
            raise NotImplementedError("set_index: currently frovedis " \
                                  "doesn't support drop=False!")

        if not isinstance(keys, list):
            keys = [keys]
        
        # multi-level index case
        if append or len(keys) > 1:
            raise NotImplementedError("set_index: currently frovedis " \
                                  "doesn't support multi-level index!")

        if inplace:   # copy may still take place, if self needs materialize
            ret = self
        else:
            ret = self.copy()

        if isinstance(keys[0], str):        # keys = ["colname"]
            new_index_name = keys[0] 
            cur_index_name = self.index.name if self.has_index() else ""
            if not new_index_name in ret.__cols:
                raise KeyError("set_index: '%s' key does not found in " \
                               "existing columns!" % (new_index_name))
            (host, port) = FrovedisServer.getServerInstance()
            dummy_df = rpclib.df_set_index(host, port, ret.get(), \
                                  cur_index_name.encode("ascii"), \
                                  new_index_name.encode("ascii"), \
                                  verify_integrity, \
                                  ret.__need_materialize)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            names = dummy_df["names"]
            types = dummy_df["types"]
            ret.index = FrovedisColumn(names[0], types[0]) # with new index
            ret.num_row = dummy_df["nrow"]
            ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        elif isinstance(keys[0], Iterable): # keys = [[1,2,3,4]]
            k = np.asarray(keys[0])
            if k.ndim != 1:
                raise ValueError("set_index: expected a one-dimensional key!")
            ret.update_index(k, key="index", \
                             verify_integrity=verify_integrity, inplace=True)
        else:
            raise TypeError("set_index: unknown type of 'keys' found!")
        return None if inplace else ret 

    def reset_index(self, drop=False, inplace=False): #TODO: support other params
        """ resets the index column of self """
        if inplace:   # copy may still take place, if self needs materialize
            ret = self
        else:
            ret = self.copy()

        if ret.has_index():
            (host, port) = FrovedisServer.getServerInstance()
            dummy_df = rpclib.df_reset_index(host, port, ret.get(), \
                                  drop, ret.__need_materialize)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            names = dummy_df["names"]
            types = dummy_df["types"]
            ret.index = FrovedisColumn(names[0], types[0]) # with new index
            ret.num_row = dummy_df["nrow"]
            ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        else:
            ret = ret.add_index("index")
        return None if inplace else ret

    # assumes self doesn't have any index column
    def __copy_index(self, from_df, inplace=False): 
        """ copies index column from 'from_df' to self """
        if inplace:   # copy may still take place, if self needs materialize
            ret = self
        else:
            ret = self.copy()

        from_df = DataFrame.asDF(from_df)
        if not from_df.has_index():
            raise ValueError("from_df doesn't have any index column!")
        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_copy_index(host, port, ret.get(), from_df.get(), \
                             from_df.index.name.encode("ascii"), \
                             ret.__need_materialize, from_df.index.dtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret.index = FrovedisColumn(names[0], types[0]) #setting index
        ret.num_row = dummy_df["nrow"]
        ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        return None if inplace else ret

    def add_index(self, name): # behavior: inplace=True
        """ adds index column to self """
        (host, port) = FrovedisServer.getServerInstance()
        proxy = -1 # set for empty df
        if self.__fdata:
            proxy = self.get()

        dummy_df = rpclib.df_add_index(host, port, proxy,
                                    name.encode("ascii"),
                                    self.__need_materialize)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        names = dummy_df["names"]
        types = dummy_df["types"]
        self.index = FrovedisColumn(names[0], types[0]) #setting index
        self.num_row = dummy_df["nrow"]
        self.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        return self

    def astype(self, dtype, copy=True, errors='raise'):
        """ Cast a frovedis DataFrame object to a specified dtype """
        t_cols = []
        t_dtypes = []
        if isinstance (dtype, (str, type)):
            numpy_dtype = np.dtype(dtype)
            t_cols = list(self.columns)
            t_dtypes = [TypeUtil.to_id_dtype(numpy_dtype)] * len(t_cols)
        elif isinstance (dtype, dict):
            for k, v in dtype.items():
                if k in self.columns:
                    t_cols.append(k)
                    t_dtypes.append(TypeUtil.to_id_dtype(np.dtype(v)))
                elif self.has_index() and k == self.index.name:
                    if self.index.dtype != DTYPE.STRING:
                        t_cols.append(k)
                        t_dtypes.append(TypeUtil.to_id_dtype(np.dtype(v)))
                    else:
                        raise ValueError("astype is not supported for "
                                         "string-typed index")
        else:
            raise TypeError("astype: supports only string, numpy.dtype " \
                            "or dict object as for 'dtype' parameter!")

        ret = self.copy() # always returns a new dataframe (after casting)
        (host, port) = FrovedisServer.getServerInstance()
        t_cols_ptr = get_string_array_pointer(t_cols)
        type_arr = np.asarray(t_dtypes, dtype=c_short)
        t_dtypes_ptr = type_arr.ctypes.data_as(POINTER(c_short))
        dummy_df = rpclib.df_astype(host, port, ret.get(), t_cols_ptr, \
                                    t_dtypes_ptr, len(t_cols))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret.num_row = dummy_df["nrow"]
        if self.has_index():
            ret.index = FrovedisColumn(names[0], types[0]) #setting index
            ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        else:
            ret.load_dummy(dummy_df["dfptr"], names[0:], types[0:])
        return ret

    def append2(self, other, ignore_index=False, verify_integrity=False,
                sort=False):

        if isinstance(other, FrovedisDataframe):
            other = [other]
        elif isinstance(other, Iterable):
            other = [DataFrame.asDF(e) for e in other]
        else:
            raise ValueError("append: Unsupported value for 'other'!")
        
        res_names = [self.columns] 
        for e in other:
            res_names.append(e.columns)
        res_names = union_lists(res_names)
        dfs = [self] + other
        res_dtypes = [infer_dtype(dfs, col) for col in res_names]
        astype_input = dict(zip(res_names, res_dtypes))
        #print(astype_input)
        dfs = add_null_column_and_type_cast(dfs, astype_input)

        # preserving column order as in self, if not to be sorted
        res_names = sorted(dfs[0].columns) if sort else dfs[0].columns
        all_cols = [dfs[0].index.name] + res_names # adding index
        proxies = np.asarray([e.get() for e in dfs[1:]]) # default dtype=long
        proxies_ptr = proxies.ctypes.data_as(POINTER(c_long))
        verify_integrity = false if ignore_index else verify_integrity
        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_union2(host, port, dfs[0].get(), \
                                   proxies_ptr, len(proxies),  \
                                   get_string_array_pointer(all_cols), \
                                   len(all_cols), verify_integrity)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        names = dummy_df["names"]
        types = dummy_df["types"]
        astypes = astype_input.values()
        for i in range(0, len(astypes)):
            if astypes[i] == np.bool:
                types[i] = DTYPE.BOOL
        res = DataFrame().load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        res.index = FrovedisColumn(names[0], types[0]) #setting index
        res.num_row = dummy_df["nrow"]
        if ignore_index:
            res.reset_index(inplace=True, drop=True)
        return res
     
    def append(self, other, ignore_index=False, verify_integrity=False,
               sort=False):
        null_replacement = {}
        null_replacement[DTYPE.DOUBLE] = np.finfo(np.float64).max
        null_replacement[DTYPE.FLOAT] = np.finfo(np.float32).max
        null_replacement[DTYPE.ULONG] = np.iinfo(np.int64).max
        null_replacement[DTYPE.LONG] = np.iinfo(np.int64).max
        null_replacement[DTYPE.INT] = np.iinfo(np.int32).max
        null_replacement[DTYPE.STRING] = "NULL"
        null_replacement[DTYPE.BOOL] = np.iinfo(np.int32).max

        (host, port) = FrovedisServer.getServerInstance()
        
        if isinstance(other, FrovedisDataframe):
            other = [other]
        elif isinstance(other, list):
            check = all([ isinstance(e, FrovedisDataframe) for e in other ] )
            if not check:
                raise ValueError("append: Unsupported value for 'other'!")
            other = list(other)
        else:
            raise ValueError("append: Unsupported value for 'other'!")        


        from collections import OrderedDict
        all_cols = OrderedDict(zip( self.__cols, self.__types))
        
        for df in other:
            cols = df.columns
            types = df.get_types()
            col_type_map = OrderedDict(zip(cols, types))
            
            for column in col_type_map:
                if column in all_cols:
                    if col_type_map[column] != all_cols[column]:
                        raise ValueError("append: Conflicting types for column : {0} !\n".format(column))
            all_cols.update(col_type_map)

        all_cols_names = set(all_cols.keys())
        bool_cols = set()
        
        base_df_col_set = set(self.columns)
        if base_df_col_set < all_cols_names:
            base_df = self.copy()
            diff_cols = [ k for k in all_cols.keys() if k not in base_df_col_set ] # keep order

            for e in diff_cols:
                if all_cols[e] == DTYPE.BOOL:
                    base_df.__append_column(e, np.dtype("bool"),
                                        [null_replacement[all_cols[e]]] * len(base_df))
                    bool_cols.add(e)
                else:
                    base_df[e] = null_replacement[all_cols[e]]
        else:
            base_df = self

        for i in range(len(other)):
            col_set = set(other[i].columns)
            if col_set < all_cols_names:
                tmp_df = other[i].copy()
                diff_cols = [ k for k in all_cols.keys() if k not in col_set ]

                for e in diff_cols:
                    if all_cols[e] == DTYPE.BOOL:
                        tmp_df.__append_column(e, np.dtype("bool"),
                                            [null_replacement[all_cols[e]]] * len(tmp_df))
                        bool_cols.add(e)
                    else:
                        tmp_df[e] = null_replacement[all_cols[e]]
                other[i] = tmp_df

        # original index names
        orig_index_names = [base_df.index.name] + [df.index.name for df in other ]
        res_index_name = "index"
        if len(set(orig_index_names)) == 1:
            res_index_name = orig_index_names[0]

        # rename all index, for union
        base_df.rename_index("tmp_index", inplace=True)
        for i in range(len(other)):
            other[i].rename_index("tmp_index", inplace=True)

        proxies = [ e.get() for e in other ]
        proxies_arr = np.asarray(proxies, dtype=c_long)
        proxies_ptr = proxies_arr.ctypes.data_as(POINTER(c_long))
        
        
        dummy_df = rpclib.df_union(host, port, base_df.get(), proxies_ptr,
                                len(proxies), ignore_index, verify_integrity,
                                sort)

        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        
        names = dummy_df["names"]
        types = dummy_df["types"]

        if len(bool_cols) > 0:
            for i in range( len(names) ):
                if names[i] in bool_cols:
                    types[i] = DTYPE.BOOL

        res = DataFrame().load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        res.index = FrovedisColumn(names[0], types[0]) #setting index
        res.num_row = dummy_df["nrow"]
        
        res.rename_index(res_index_name, inplace=True)

        if ignore_index:
            res.reset_index(inplace=True, drop=True)

        if sort:
            col_order = sorted(res.columns)
            if res.has_index():
                col_order = [res.index.name] + col_order
            res.__set_col_order(col_order)


        # rename to original indices
        base_df.rename_index(orig_index_names[0], inplace=True)
        for i in range(len(other)):
            other[i].rename_index(orig_index_names[i+1], inplace=True)
        
        return res

    def __set_col_order(self, new_cols):
        if not isinstance(new_cols, list):
            raise ValueError("__set_col_order: The new column order to be set"
                            " must be provided as list of strings!")
        else:
            check = all([ isinstance(e, str) for e in new_cols ] )
            if not check:
                raise ValueError("__set_col_order: The new column order to be set"
                            " must be provided as list of strings!")
        current_col_order = self.columns
        if self.has_index():
            current_col_order = [self.index.name] + current_col_order

        if len(current_col_order) != len(new_cols):
            raise ValueError("__set_col_order: The new column order to be set"
                            " must have same number of columns as the dataframe!")

        if set(current_col_order) != set(new_cols):
            raise ValueError("__set_col_order: The new column order to be set"
                            " must have same column names as the dataframe!")

        sz = len(new_cols)
        new_cols_ptr = get_string_array_pointer(new_cols)

        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_set_col_order(host, port, self.get(),
                                           new_cols_ptr, sz)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        names = dummy_df["names"]
        types = dummy_df["types"]

        self.num_row = dummy_df["nrow"]
        if self.has_index():
            self.index = FrovedisColumn(names[0], types[0]) #setting index
            self.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        else:
            self.load_dummy(dummy_df["dfptr"], names[0:], types[0:])

        return self

    def get_types(self):
        return self.__types

    def get_dtype(self, colname):
        if self.has_index() and colname == self.index.name:
            ret = TypeUtil.to_numpy_dtype(self.index.dtype)
        elif colname in self.columns:
            ret = TypeUtil.to_numpy_dtype(self.__dict__[colname].dtype)
        else:
            raise ValueError("column not found: '%s'" % (colname))
        return ret

    def __setattr__(self, key, value):
        if key in self.__dict__: # instance attribute
            if self.__cols and key in self.__cols:
                self[key] = value
            else:
                self.__dict__[key] = value
        else:
            attr = getattr(self.__class__, key, None)
            if isinstance(attr, property):
                if attr.fset is None:
                    raise AttributeError("%s: can't set attribute" % (key))
                attr.fset(self, value) # calling setter of property
            else:
                self.__dict__[key] = value

    def __str__(self):
        #TODO: fixme to return df as a string
        self.show()
        return ""

    def __repr__(self):
        return self.__str__()

FrovedisDataframe = DataFrame
