"""
df.py
"""
#!/usr/bin/env python

import copy
from ctypes import *
from array import array
import numpy as np
import pandas as pd

from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from ..matrix.dvector import FrovedisIntDvector, FrovedisLongDvector
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
from ..utils import deprecated

import warnings
from pandas.core.common import SettingWithCopyWarning

def get_string_typename(numpy_type):
    """
    return frovedis types from numpy types
    """
    numpy_to_string_type = { "int32": "int",
                             "int64": "long",
                             "float32": "float",
                             "float64": "double",
                             "str": "dic_string",
                             "string": "dic_string",
                             "bool": "dic_string",
                             "uint64": "unsigned long"}
    if numpy_type not in numpy_to_string_type:
        raise TypeError("data type '{0}'' not understood\n"
                        .format(numpy_type))
    return numpy_to_string_type[numpy_type]


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
    
    #def __del__(self):
    #    if FrovedisServer.isUP(): self.release()

    #def __del__(self):
    #    if FrovedisServer.isUP(): self.release()

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
    def count(self):
        """return num_row in input df"""
        return len(self)

    @count.setter
    def count(self, val):
        """count setter"""
        raise AttributeError(\
            "attribute 'count' of DataFrame object is not writable")

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
        tmp = [None] * size
        dvec = [0] * size

        # null replacements
        null_replacement = {}
        null_replacement['float64'] = np.finfo(np.float64).max
        null_replacement['float32'] = np.finfo(np.float32).max
        null_replacement['int64'] = np.iinfo(np.int64).max
        null_replacement['int32'] = np.iinfo(np.int32).max
        null_replacement['bool'] = np.iinfo(np.int32).max
        null_replacement['bool_'] = np.iinfo(np.int32).max
        null_replacement['str'] = "NULL"

        for idx in range(0, size):
            cname = cols[idx]
            if idx == 0:
                col_vec = df.index
                valid_idx = df.index[0] # assuming df.index[0] is not null
            else:
                col_vec = df[cname]
                # first value can be NaN, so can't deduce type
                valid_idx = col_vec.first_valid_index()

            if valid_idx is None:  # all NaN in col_vec
                vtype = 'int32'
                val = np.full(len(df), np.iinfo(np.int32).max, dtype=np.int32)
            else:
                valid_idx = df.index.get_loc(valid_idx)
                vtype = type(col_vec.values[valid_idx]).__name__
                val = col_vec.fillna(null_replacement[vtype])
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
            elif vtype == 'str' or vtype == 'str_':
                dt = DTYPE.STRING
                tmp[idx] = FrovedisStringDvector(val)
            elif vtype == 'bool' or vtype == 'bool_':
                dt = DTYPE.BOOL
                tmp[idx] = FrovedisIntDvector([int(i) for i in val])
            else:
                raise TypeError("Unsupported column type in creation of \
                                 frovedis dataframe: ", vtype)
            types[idx] = dt
            dvec[idx] = tmp[idx].get()
            if idx == 0:
                self.index = FrovedisColumn(cname, dt)   
            else:
                self.__dict__[cname] = FrovedisColumn(cname, dt) #For query purpose

        col_names = get_string_array_pointer(cols)
        dvec_arr = np.asarray(dvec, dtype=c_long)
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

    def filter_frovedis_dataframe(self, opt):
        """
        filter_frovedis_dataframe
        """
        ret = DataFrame(need_materialize = True)
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
            if item not in self.__cols:
                raise ValueError("sort: No column named: ", item)

        vec = np.asarray(sort_by)
        vv = vec.T # returns self, since ndim=1
        sz = vec.size
        sort_by_arr = (c_char_p * sz)()
        sort_by_arr[:] = [e.encode('ascii') for e in vv]
        
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
        left_on_ = np.asarray(left_on)
        right_on_ = np.asarray(right_on)
        sz = left_on_.size

        left_on_arr = (c_char_p * sz)()
        right_on_arr = (c_char_p * sz)()

        left_on_arr[:] = np.array([e.encode('ascii') for e in left_on_.T])
        right_on_arr[:] = np.array([e.encode('ascii') for e in right_on_.T])

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

        # index rename for right
        if right.index is not None:
            right = right.rename({right.index.name: right.index.name + "_right"})

        # no of nulls
        n_nulls = sum(x is None for x in (on, right_on, left_on))
        if n_nulls == 3:
            if self == right:
                # Early return , all columns are common => all comlumns are treated as keys
                # returning a copy of self
                return self.select_frovedis_dataframe(self.columns)
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

        return res.__add_index("index")

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
            raise TypeError("Expected: dictionary; Received: ",
                            type(columns).__name__)
        if not columns:
            return self
        names = list(columns.keys())
        new_names = list(columns.values())
        ret = DataFrame()
        ret.__cols = copy.deepcopy(self.__cols)
        ret.__types = copy.deepcopy(self.__types)
        for item in ret.__cols:
            ret.__dict__[item] = self.__dict__[item]

        # add index
        if self.has_index():
            ret.__dict__["index"] = self.__dict__["index"]

        for i in range(0, len(names)):
            item = names[i]
            new_item = new_names[i]

            # adding case for index
            if self.has_index() and item == self.index.name:
                del ret.__dict__[item]
                ret.__dict__["index"] = FrovedisColumn(new_item, self.index.dtype)
            elif item not in ret.__cols:
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
        if isinstance(df, DataFrame):
            return df
        elif isinstance(df, pd.DataFrame):
            return DataFrame(df)
        else: TypeError, "Invalid dataframe type is provided!"

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
        cols = np.asarray(columns)
        cols_ptr = (c_char_p * sz)()
        cols_ptr[:] = [e.encode('ascii') for e in cols.T]

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
            if not types:
                raise ValueError("type of target columns is missing for "
                                 " std calculation")
            ret = rpclib.get_std_frovedis_dataframe(host, port, self.get(),
                                                    cols_ptr, tptr, sz)
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
        null_replacement[DTYPE.ULONG] = np.iinfo(np.int64).max
        null_replacement[DTYPE.LONG] = np.iinfo(np.int64).max
        null_replacement[DTYPE.INT] = np.iinfo(np.int32).max
        null_replacement[DTYPE.STRING] = "NULL"
        int2bool = {0: False, 1: True, np.iinfo(np.int32).max: np.nan}

        res = pd.DataFrame()
        (host, port) = FrovedisServer.getServerInstance()

        for i in range(0, len(cols)):
            col_val = rpclib.get_frovedis_col(host, port, self.get(),
                                              cols[i].encode('ascii'),
                                              types[i])
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])

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
                raise ValueError("target column list doesn't contain"
                                 " categorical column: ", item)
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

        self.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        self.index = FrovedisColumn(names[0], types[0]) #setting index
        self.num_row = dummy_df["nrow"]
        
        return self

    def __get_dvec(self, dtype, val):
        """
        get dvector from numpy array
        """
        dvec_constructor_map = { DTYPE.INT: FrovedisIntDvector,
                                 DTYPE.LONG: FrovedisLongDvector,
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

        names = dummy_df["names"]
        types = dummy_df["types"]

        self.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        self.index = FrovedisColumn(names[0], types[0]) #setting index
        self.num_row = dummy_df["nrow"]

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
            n = len(self)
            value = [value] * n

        if not isinstance(value, (list, tuple, range, pd.Series, np.ndarray)):
            raise ValueError("__setitem__: Given column data type '{}' is "
                            "not supported!".format(type(value)))


        col_data = np.array(value)
        col_dtype = self.__get_dtype_name(col_data)

        drop_old = False
        if self.__fdata and key in self.columns:
            drop_old = True

        self = self.__append_column(key, np.dtype(col_dtype), col_data, None,
                                drop_old)

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
            n = len(self)
            value = [value] * n

        if not isinstance(value, (list, tuple, range, pd.Series, np.ndarray)):
            raise ValueError("insert: Given column data type '{}' is not "
                            "supported!".format(type(value)))
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

    def __add_index(self, name):
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

        self.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        self.index = FrovedisColumn(names[0], types[0]) #setting index
        self.num_row = dummy_df["nrow"]

        return self

    def __setattr__(self, key, value):
        if key in self.__dict__:
            if self.__cols and key in self.__cols:
                self[key] = value
            else:
                self.__dict__[key] = value
        else:
            self.__dict__[key] = value

    def __str__(self):
        #TODO: fixme to return df as a string
        self.show()
        return ""

    def __repr__(self):
        return self.__str__()

FrovedisDataframe = DataFrame
