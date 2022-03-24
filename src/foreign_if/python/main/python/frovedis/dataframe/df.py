"""
df.py
"""
import warnings
import copy
import sys
import numbers
from ctypes import *
from array import array
import numpy as np
import pandas as pd
from collections import Iterable, OrderedDict

from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer, set_association, \
                           check_association, do_if_active_association
from ..matrix.dvector import FrovedisDvector
from ..matrix.dvector import FrovedisIntDvector, FrovedisLongDvector
from ..matrix.dvector import FrovedisULongDvector
from ..matrix.dvector import FrovedisFloatDvector, FrovedisDoubleDvector
from ..matrix.dvector import FrovedisStringDvector
from ..matrix.dtype import DTYPE, TypeUtil, get_string_array_pointer
from ..matrix.dtype import get_result_type
from ..matrix.dense import FrovedisRowmajorMatrix
from ..matrix.dense import FrovedisColmajorMatrix
from ..matrix.crs import FrovedisCRSMatrix
from ..mllib.model_util import ModelID
from .info import df_to_sparse_info
from .frovedisColumn import FrovedisColumn
from .dfoperator import dfoperator
from .dfutil import union_lists, infer_dtype, add_null_column_and_type_cast, \
                    infer_column_type_from_first_notna, get_string_typename, \
                    get_python_scalar_type, check_string_or_array_like, \
                    check_stat_error
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
        na_values = ['null', 'NULL', 'nan', '-nan', 'NaN', '-NaN', 'NA', 'N/A', 'n/a']
    elif isinstance(na_values, dict):
        raise ValueError("read_csv: Frovedis currently doesn't support "\
                         "na_values as dictionary!")
    else:
        na_values = check_string_or_array_like(na_values, 'read_csv')

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
        if single_dtype == 'bool':
            types = [] # let it be infered at server side
        else:
            types = [get_string_typename(single_dtype)] * n

    is_all_bools = single_dtype == 'bool'
    bool_cols = []
    if isinstance(dtype, dict):
        bool_cols = [k for k in dtype if dtype[k] == 'bool']

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

    na_sz = len(na_values)
    na_ptr = get_string_array_pointer(na_values)

    (host, port) = FrovedisServer.getServerInstance()
    dummy_df = rpclib.load_dataframe_from_csv(host, port,
                                            filepath_or_buffer.encode('ascii'),
                                            type_arr, name_arr,
                                            len(type_arr), len(name_arr),
                                            delimiter.encode('ascii'),
                                            na_ptr, na_sz,
                                            comment.encode("ascii"),
                                            rows_to_see, separate_mb,
                                            partial_type_info,
                                            dtype_keys_arr, dtype_vals_arr,
                                            len(dtype_keys), low_memory, add_index,
                                            usecols, len(usecols),
                                            verbose, mangle_dupe_cols, index_col_no,
                                            bool_cols_arr, len(bool_cols_arr),
                                            is_all_bools)
    excpt = rpclib.check_server_exception()
    if excpt["status"]:
        raise RuntimeError(excpt["info"])

    names = dummy_df["names"]
    types = dummy_df["types"]

    if is_all_bools:
        types = [DTYPE.BOOL] * len(types)
    elif len(bool_cols) > 0:
        for i in range(len(names)):
            if names[i] in bool_cols:
                types[i] = DTYPE.BOOL

    res = FrovedisDataframe().load_dummy(dummy_df["dfptr"], \
                                         names[1:], types[1:])
    #TODO: handle index/num_row setting inside load_dummy()
    res.index = FrovedisColumn(names[0], types[0]) #setting index
    res.num_row = dummy_df["nrow"]
    return res

class DataFrame(object):
    """
    DataFrame
    """

    def __init__(self, df=None, is_series=False):
        """
        __init__
        """
        self.__fdata = None
        self.__cols = None
        self.__types = None
        self.index = None
        self.__multi_cols = None
        self.__multi_index = None
        self.is_series = is_series
        if df is not None:
            self.load(df)

    def has_index(self):
        """
        has_index
        """
        return self.index is not None

    @set_association
    def load_dummy(self, fdata, cols, types):
        """
        load_dummy
        """
        self.__fdata = fdata
        self.__cols = cols
        self.__types = types
        for i in range(0, len(cols)):
            cname = cols[i]
            dt = types[i]
            self.__dict__[cname] = FrovedisColumn(cname, dt)

        for column in self.columns:
            self.__dict__[column].df = self

        if self.has_index():
            self.index.df = self

        return self

    def show(self):
        """
        show
        """
        '''
        if self.__fdata is not None:
            (host, port) = FrovedisServer.getServerInstance()
            rpclib.show_frovedis_dataframe(host, port, self.__fdata)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            print("\n")
        '''
        print(str(self))

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
        self.index = None
        self.is_series = None

    @do_if_active_association
    def __release_server_heap(self):
        """ releases the dataframe pointer from server heap """
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.release_frovedis_dataframe(host, port, self.__fdata)
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
    def ndim(self):
        """return dimension in input df"""
        return 1 if self.is_series else 2

    @ndim.setter
    def ndim(self, val):
        """ndim setter"""
        raise AttributeError(\
            "attribute 'ndim' of DataFrame object is not writable")

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
    @check_association
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
    @check_association
    def shape(self):
        """return shape of input df"""
        return (len(self), len(self.__cols))

    @shape.setter
    def shape(self, val):
        """shape setter"""
        raise AttributeError(\
            "attribute 'shape' of DataFrame object is not writable")

    @set_association
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
        if isinstance(df, pd.Series):
            if len(df) == 1 and len(df[0]) > 1: # contains array-like
                raise ValueError("Cannot load a pandas series " +\
                                 "containing non-atomic elements!")
            df = df.to_frame()
            self.is_series = True

        self.num_row = len(df)
        cols = df.columns.tolist()
        self.__cols = [str(x) for x in cols]
        indx_name = df.index.name if df.index.name is not None else "index"
        cols = [indx_name] + cols
        size = len(cols)
        types = [0] * size
        dvec = [None] * size

        for idx in range(0, size):
            cname = cols[idx]
            s_cname = str(cname) # cname might be int etc.
            if idx == 0:
                val = df.index
            else:
                val = df[cname]
            vtype = val.dtype.name
            if vtype == 'object': # type-infering required to detect string
                vtype = infer_column_type_from_first_notna(df, val, idx == 0)
            #print(cname + ":" + vtype)
            if vtype == 'int32' or vtype == 'int':
                dt = DTYPE.INT
                dvec[idx] = FrovedisIntDvector(val)
            elif vtype == 'int64' or vtype == 'long':
                dt = DTYPE.LONG
                dvec[idx] = FrovedisLongDvector(val)
            elif vtype == 'uint64':
                dt = DTYPE.ULONG
                dvec[idx] = FrovedisULongDvector(val)
            elif vtype == 'float32' or vtype == 'float':
                dt = DTYPE.FLOAT
                dvec[idx] = FrovedisFloatDvector(val)
            elif vtype == 'float64' or vtype == 'double':
                dt = DTYPE.DOUBLE
                dvec[idx] = FrovedisDoubleDvector(val)
            elif vtype == 'str' or vtype == 'str_' or vtype == 'string':
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
                self.index = FrovedisColumn(s_cname, dt)
            else:
                self.__dict__[s_cname] = FrovedisColumn(s_cname, dt) #For query purpose

        strcols = [str(cols[0])] + self.__cols # to ensure always a string-vec
        col_names = get_string_array_pointer(strcols)
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

        for col in self.columns:
            self.__dict__[col].df = self

        if self.has_index():
            self.index.df = self

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
                if all(isinstance(e, bool) for e in target):
                    return self.__filter_using_mask(target)
                return self.select_frovedis_dataframe(target)
            elif isinstance(target, dfoperator):
                return self.filter_frovedis_dataframe(target)
            elif isinstance(target, slice):
                return self.__filter_slice_range(target)
            elif isinstance(target, np.ndarray):
                if target.dtype == "bool":
                    return self.__filter_using_mask(target)
                return self.select_frovedis_dataframe(target.tolist())
            else:
                raise TypeError("Unsupported indexing input type!")
        else:
            raise ValueError("Operation on invalid frovedis dataframe!")

    @property
    @check_association
    def columns(self):
        """returns column list"""
        return self.__cols

    @columns.setter
    @check_association
    def columns(self, newcols):
        """renames columns"""
        if newcols is None or np.isscalar(newcols):
            raise TypeError("must be called with a collection of some "
                            "kind, '%s' was passed" % (newcols))
        if len(newcols) != len(self.__cols):
            raise ValueError("Length mismatch: Expected axis has %d elements"
                             ", new values have %d elements" \
                             % (len(self.__cols), len(newcols)))
        self.rename(columns=dict(zip(self.__cols, newcols)), inplace=True)

    @check_association
    def filter_frovedis_dataframe(self, opt):
        """
        filters rows on the basis of given 'opt' condition
        from the input dataframe
        """
        if (opt.df.get() != self.get()):
            ret = self.__filter_using_mask(opt.mask)
        else:
            ret = DataFrame(is_series=self.is_series)
            ret.index = self.index.copy()
            (host, port) = FrovedisServer.getServerInstance()
            proxy = rpclib.filter_frovedis_dataframe(host, port, \
                                                     self.get(), opt.get())
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            ret.load_dummy(proxy, list(self.__cols), list(self.__types))
        return ret

    @check_association
    def __filter_using_mask(self, mask):
        """
        filters rows on the basis of boolean-mask
        """
        if not isinstance(mask, FrovedisIntDvector):
            mask = FrovedisIntDvector(np.asarray(mask, dtype=np.int32)) #checks errors
        ret = DataFrame(is_series=self.is_series)
        ret.index = self.index.copy()
        (host, port) = FrovedisServer.getServerInstance()
        proxy = rpclib.filter_df_using_mask(host, port, \
                                            self.get(), mask.get())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        ret.load_dummy(proxy, list(self.__cols), list(self.__types))
        return ret

    @check_association
    def select_frovedis_dataframe(self, targets):
        """ selects given columns from the input dataframe """
        targets = list(check_string_or_array_like(targets, "select"))
        is_ser = len(targets) == 1
        ret = DataFrame(is_series=is_ser)
        ret_types = self.__get_column_types(targets)
        ret_cols = list(targets) #targets is a list
        ret.num_row = len(self)
        ret.index = self.index.copy()

        if self.has_index():
            targets = [self.index.name] + targets

        sz = len(targets)
        ptr_arr = get_string_array_pointer(targets)
        (host, port) = FrovedisServer.getServerInstance()
        proxy = rpclib.select_frovedis_dataframe(host, port, self.get(), \
                                                 ptr_arr, sz)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        ret.load_dummy(proxy, ret_cols, ret_types)
        return ret

    @check_association
    def nsort(self, n, columns, keep='first', is_desc=False):
        """
        returns top n sorted rows (in ascending or descending order)
        """
        func = "nlargest" if is_desc else "nsmallest"
        if not isinstance(n, int):
            raise TypeError(\
            func + ": expected a positive integer for 'n' parameter!\n")
        elif n < 0:
            raise ValueError(\
            func + ": expected a positive integer for 'n' parameter!\n")

        if not isinstance(keep, str):
            raise TypeError(\
            func + ": expected a string for 'keep' parameter!\n")
        elif keep != "all" and keep != "first" and keep != "last":
            raise ValueError(func + ": supported 'keep' values are 'first', "\
                             "'last' and 'all' only! receieved %s.\n" % (keep))

        sort_by = check_string_or_array_like(columns, func)
        for item in sort_by:
            # TODO: confirm possibility of including index column
            #if item not in self.columns and item != self.index.name:
            if item not in self.columns:
                raise ValueError(func + ": No column named: " + str(item))

        sz = len(sort_by)
        ptr_arr = get_string_array_pointer(sort_by)
        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_ksort(host, port, self.get(),
                                   n, ptr_arr, sz, keep.encode('ascii'),
                                   is_desc)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret = DataFrame(is_series=self.is_series)
        ret.num_row = dummy_df["nrow"]
        if self.has_index():
            ret.index = FrovedisColumn(names[0], types[0]) #setting index
            ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        else:
            ret.load_dummy(dummy_df["dfptr"], names, types)
        return ret

    def nlargest(self, n, columns, keep='first'):
        """
        returns top n sorted rows in descending order
        """
        return self.nsort(n, columns, keep, is_desc=True)

    def nsmallest(self, n, columns, keep='first'):
        """
        returns top n sorted rows in ascending order
        """
        return self.nsort(n, columns, keep, is_desc=False)

    def sort_index(self, axis=0, ascending=True, inplace=False,
                  kind='quicksort', na_position='last'):
        """
        sort_index
        """
        return self.sort_values(by=self.index.name, axis=axis, ascending=ascending,
                               inplace=inplace, kind=kind, na_position=na_position)

    @check_association
    def sort_values(self, by, axis=0, ascending=True,
                    inplace=False, kind='radixsort', na_position='last'):
        """
        sort_values
        """
        if na_position != "last":
            if na_position == "first":
                raise ValueError("Frovedis currently doesn't support " \
                                 "na_position='first' !")
            else:
                raise ValueError("invalid na_position: '{}' ".format(na_position))

        if axis not in (0, "index"):
            if axis in (1, "columns"):
                raise ValueError("Frovedis currently doesn't support sorting " \
                                 "the axis = 1 !")
            else:
                raise ValueError("No axis named {} for frovedis dataframe !"
                                .format(axis))

        if kind not in ["radixsort", "stable"]:
            warnings.warn("Frovedis currently supports radixsort (stable) " \
                     "internally, other 'kind' parameters will be ignored!")

        if inplace:
            raise ValueError("Frovedis currently doesn't support inplace " \
                             "sorting of dataframes ! ")

        sort_by = check_string_or_array_like(by, "sort")
        for item in sort_by:
            if item not in self.columns and item != self.index.name:
                raise ValueError("sort: No column named: %s" % str(item))

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

        ret = DataFrame(is_series=self.is_series)
        ret.index = self.index.copy()
        ret.num_row = len(self)
        (host, port) = FrovedisServer.getServerInstance()
        proxy = rpclib.sort_frovedis_dataframe(host, port, self.get(),\
                                               sort_by_arr, sort_order, sz)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        ret.load_dummy(proxy, list(self.__cols), list(self.__types))
        return ret

    def sort(self, columns=None, axis=0, ascending=True,
             inplace=False, kind='radixsort', na_position='last', **kwargs):
        """
        sort_
        """
        if not columns:
            raise ValueError("Column to be sorted cannot be None!")
        return self.sort_values(by=columns, axis=axis,
                                ascending=ascending,
                                inplace=inplace, kind=kind,
                                na_position=na_position)

    @check_association
    def groupby(self, by=None, axis=0, level=None,
                as_index=True, sort=True, group_keys=True, squeeze=False,
                observed=False, dropna=True):
        """
        groupby
        """
        from frovedis.dataframe import grouped_df
        if axis != 0:
            raise NotImplementedError(\
            "groupby: axis = '%d' is currently not supported!" % (axis))

        if level is not None:
            raise NotImplementedError( \
            "groupby: level is currently not supported!")

        g_by = check_string_or_array_like(by, "groupby")
        types = self.__get_column_types(g_by)
        if dropna:
            g_df = self.dropna(subset=g_by)
        else:
            g_df = self

        sz = len(g_by)
        ptr_arr = get_string_array_pointer(g_by)
        (host, port) = FrovedisServer.getServerInstance()
        fdata = rpclib.group_frovedis_dataframe(host, port, g_df.get(),
                                                ptr_arr, sz)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return grouped_df.FrovedisGroupedDataframe()\
                         .load_dummy(fdata, list(g_by), list(types), \
                                     list(self.__cols), list(self.__types))
    def __merge_rename_helper(self, rename_dict):
        """
        helper function for renaming columns/index for dataframe
        """
        ret = self.rename(rename_dict)
        index_name = self.index.name

        if index_name is not None and index_name in rename_dict:
            ret = self.rename_index(rename_dict[index_name])

        return ret

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
        df_right = df_right.__merge_rename_helper(col_dict)
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

    @check_association
    def merge(self, right, on=None, how='inner', left_on=None, right_on=None,
              left_index=False, right_index=False, sort=False,
              suffixes=('_x', '_y'), copy=True,
              indicator=False, join_type='bcast'):
        """
        merge
        """

        right = DataFrame.asDF(right)
        on_index = None

        if not isinstance(left_index, bool):
            raise ValueError(
                "left_index parameter must be of type bool, not ", type(left_index)
            )
        if not isinstance(right_index, bool):
            raise ValueError(
                "right_index parameter must be of type bool, not", type(right_index)
            )

        if left_on and left_index:
            raise ValueError("Can only pass 'left_on' OR 'left_index'" +
                            " not a combination of both!")
        elif right_on and right_index:
            raise ValueError("Can only pass 'right_on' OR 'right_index'" +
                            " not a combination of both!")
        elif on and (left_index or right_index):
            raise ValueError("Can only pass 'on' OR 'left_index' and" +
                            " 'right_index', not a combination of both!")

        # table must have index, if merge-target is index
        if left_index and not self.has_index():
            raise ValueError("left table doesn't have index!\n")
        if right_index and not right.has_index():
            raise ValueError("right table doesn't have index!\n")

        # index rename for right, if same with left
        if self.has_index() and right.has_index():
            if self.index.name == right.index.name:
                right = right.rename_index(right.index.name + "_right")
                reset_index_name = False #rename not required, if both are same
            else:
                # if both table have different names for index,
                # then if merge takes place on both indices i.e.,
                # (left_index=right_index=True), resultant table
                # would have generic name for index column
                reset_index_name = True

        if left_index and right_index:
            on_index = self.index
            if self.index.name == "index":
                #rename not required, if index-name is already generic
                reset_index_name = False
        elif left_index and not right_index:
            on_index = right.index
            reset_index_name = False
        elif right_index and not left_index:
            on_index = self.index
            reset_index_name = False

        if on: #if key name is same in both dataframes
            if(left_on) or (right_on):
                raise ValueError("Can only pass 'on' OR 'left_on' and" +
                                 " 'right_on', not a combination of both!")
            left_on = right_on = on

        # no of nulls
        n_nulls = sum(x is None for x in (on, right_on, left_on))
        if n_nulls == 3:
            if self is right:
                # Early return: all columns are common => all comlumns are treated as keys
                # returning a copy of self
                ret = self.select_frovedis_dataframe(self.columns)
                ret.reset_index(drop=True, inplace=True)
                return ret

            if left_index and right_index:
                left_on, right_on = self.index.name, right.index.name
            elif left_index:
                raise ValueError("Must pass right_on or right_index=True")
            elif right_index:
                raise ValueError("Must pass left_on or left_index=True")
            else:
                common_cols = list(set(self.columns) & set(right.columns))
                if len(common_cols) == 0:
                    raise ValueError("No common columns to perform merge on.")
                left_on = right_on = common_cols
        elif left_on and right_index:
            right_on = right.index.name
        elif left_index and right_on:
            left_on = self.index.name

        if (left_on and not right_on) or (not left_on and right_on):
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

        left_non_key_column_in_right_key = left_non_key_cols & set(right_keys)
        right_non_key_column_in_left_key = right_non_key_cols & set(left_keys)

        # if renaming required add suffixes
        df_left = self
        df_right = right
        if len(common_non_key_cols) > 0 \
            or len(left_non_key_column_in_right_key) > 0 \
            or len(right_non_key_column_in_left_key) > 0:
            renamed_left_cols = {}
            renamed_right_cols = {}
            lsuf, rsuf = suffixes
            if lsuf == '' and rsuf == '':
                raise ValueError("columns overlap but no suffix specified: %s " \
                                % common_non_key_cols)
            for e in common_non_key_cols:
                renamed_left_cols[e] = e + str(lsuf)
                renamed_right_cols[e] = e + str(rsuf)

            for e in left_non_key_column_in_right_key:
                renamed_left_cols[e] = e + str(lsuf)

            for e in right_non_key_column_in_left_key:
                renamed_right_cols[e] = e + str(rsuf)

            if lsuf != '':
                df_left = self.__merge_rename_helper(renamed_left_cols)
            if rsuf != '':
                df_right = right.__merge_rename_helper(renamed_right_cols)

        ret = DataFrame()
        ret_cols = df_left.__cols + df_right.__cols
        ret_types = df_left.__types + df_right.__types
        frov_how = self.__get_frovedis_how(how)
        (host, port) = FrovedisServer.getServerInstance()
        proxy = rpclib.merge_frovedis_dataframe(host, port, df_left.get(), \
                                     df_right.get(), dfopt.get(), \
                                     frov_how.encode('ascii'), \
                                     join_type.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        ret.load_dummy(proxy, ret_cols, ret_types)

        targets = ret.columns
        if renamed_keys:
            for key in renamed_keys:
                if key in targets:
                    targets.remove(key)

        if on_index:
            index_name = on_index.name
            ret.index = FrovedisColumn(index_name, dtype=on_index.dtype)
            ret = ret.select_frovedis_dataframe(targets)
            if index_name.endswith("_right"):
                index_name = index_name[:-len("_right")]
                ret.rename_index(index_name, inplace=True)
            elif reset_index_name:
                ret.rename_index("index", inplace=True)
        else:
            ret = ret.select_frovedis_dataframe(targets)
            ret = ret.add_index("index")

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

    @check_association
    def rename(self, columns, inplace=False):
        """ returns new dataframe with renamed columns """
        if not columns:
            return self
        if not isinstance(columns, dict):
            raise TypeError("Expected: dictionary; Received: ",
                            type(columns).__name__)

        names = list(columns.keys())
        new_names = list(columns.values())
        if inplace:
            ret = self
        else:
            ret = DataFrame(is_series=self.is_series)
            ret.index = self.index.copy()
            ret.num_row = len(self)
        ret_cols = list(self.__cols)
        ret_types = list(self.__types)

        t_names = []
        t_new_names = []
        for i in range(0, len(names)):
            item = names[i]
            new_item = new_names[i]
            if item in ret_cols: # otherwise skipped as in pandas rename()
                if inplace:
                    del self.__dict__[item]
                idx = ret_cols.index(item)
                ret_cols[idx] = new_item
                t_names.append(item)
                t_new_names.append(new_item)

        sz = len(t_names)
        name_ptr = get_string_array_pointer(t_names)
        new_name_ptr = get_string_array_pointer(t_new_names)
        (host, port) = FrovedisServer.getServerInstance()
        proxy = rpclib.rename_frovedis_dataframe(host, port, self.get(), \
                                                 name_ptr, new_name_ptr, \
                                                 sz, inplace)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        ret.load_dummy(proxy, ret_cols, ret_types)
        return None if inplace else ret

    @check_association
    def rename_index(self, new_name, inplace=False):
        """ renames index field of self inplace """
        if not self.has_index():
            raise ValueError("rename_index: no index field for renaming!")
        if inplace:
            ret = self
        else:
            ret = DataFrame(is_series=self.is_series)
            ret.num_row = len(self)
        ret.index = FrovedisColumn(new_name, self.index.dtype)
        ret_cols = list(self.__cols)
        ret_types = list(self.__types)

        sz = 1
        name_ptr = get_string_array_pointer([self.index.name])
        new_name_ptr = get_string_array_pointer([new_name])

        (host, port) = FrovedisServer.getServerInstance()
        proxy = rpclib.rename_frovedis_dataframe(host, port, \
                              self.get(), name_ptr, new_name_ptr, \
                              sz, inplace)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        ret.load_dummy(proxy, ret_cols, ret_types)
        return None if inplace else ret

    def copy(self, deep=True):
        """
        copy input dataframe to construct a new dataframe
        """
        if not deep:
            raise NotImplementedError("copy: shallow-copy is not yet supported!")
        if self.get() is None: # empty dataframe
            return DataFrame()
        else:
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
        elif isinstance(df, (pd.DataFrame, pd.Series)):
            return DataFrame(df)
        else:
            raise TypeError("asDF: invalid dataframe type '%s' "
                            "is provided!" % (type(df).__name__))

    def __get_column_types(self, columns):
        """
        __get_column_types
        """
        columns = check_string_or_array_like(columns, "__get_column_types")
        sz = len(columns)
        types = [0]*sz
        for i in range(0, sz):
            item = columns[i]
            if item not in self.columns:
                raise ValueError("No column named: %s" % str(item))
            else:
                types[i] = self.__dict__[item].dtype
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
        elif name == 'sem':
            ret = rpclib.get_sem_frovedis_dataframe(host, port, self.get(),
                                                    cols_ptr, sz)
        elif name == 'avg' or name == 'mean':
            ret = rpclib.get_avg_frovedis_dataframe(host, port, self.get(),
                                                    cols_ptr, sz)
        elif name == 'count':
            ret = rpclib.get_cnt_frovedis_dataframe(host, port, self.get(),
                                                    cols_ptr, sz)
        elif name == 'var':
            ret = rpclib.get_var_frovedis_dataframe(host, port, self.get(),
                                                    cols_ptr, sz)
        elif name == 'mad':
            ret = rpclib.get_mad_frovedis_dataframe(host, port, self.get(),
                                                    cols_ptr, sz)
        elif name == 'median':
            ret = rpclib.get_median_frovedis_dataframe(host, port, self.get(),
                                                       cols_ptr, tptr, sz)
        else:
            raise ValueError("Unknown statistics request is encountered!")
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return ret

    def __get_stat_wrapper(self, func_name, columns):
        if self.__fdata is None:
            raise ValueError("Operation on invalid frovedis dataframe!")
        columns = check_string_or_array_like(columns, func_name)

        if func_name == "count": # uses all columns
            types = self.__get_column_types(columns)
            return self.__get_stat('count', columns, types)
        else:
            numeric_cols, numeric_cols_types = \
              self.__get_numeric_columns(cols=columns)
            ret = self.__get_stat(func_name, numeric_cols, numeric_cols_types)
            result_dict = dict(zip(numeric_cols, ret))
            return [result_dict.get(col, np.nan) for col in columns]

    def __get_numeric_columns(self, cols=None, include_bool=True):
        """
        __get_numeric_columns
        """
        if cols is not None:
            cols = check_string_or_array_like(cols, "__get_numeric_columns")
            types = self.__get_column_types(cols)
        else:
            cols = self.__cols
            types = self.__types

        if include_bool:
            non_numeric_types = [DTYPE.STRING]
        else:
            non_numeric_types = [DTYPE.STRING, DTYPE.BOOL]

        numeric_cols = []
        numeric_col_types = []
        for i in range(0, len(cols)):
            if types[i] not in non_numeric_types:
                numeric_cols.append(cols[i])
                numeric_col_types.append(types[i])
        return numeric_cols, numeric_col_types

    #TODO: support pandas-like input parameters and
    #      improve result compatibility
    @check_association
    def describe(self):
        """
        describe
        """
        cols, types = self.__get_numeric_columns(include_bool=False)
        index = ['count', 'mean', 'median', 'var', 'mad', 'std', 'sem', \
                 'sum', 'min', 'max']
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
        if (len(set(index).intersection(set(['mean', 'avg', 'std', 'mad', \
                                            'var', 'median', 'sem']))) > 0):
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

    @check_association
    def agg(self, func):
        if isinstance(func, str):
            if "cov" == func:
                return self.cov(1, 1.0, True)
            return self.__agg_list([func]).transpose()[func]
        elif isinstance(func, list):
            if len(func) == 1 and "cov" in func:
                return self.cov(1, 1.0, True)
            if len(func) > 1 and "cov" in func:
                func.remove("cov")
            return self.__agg_list(func)
        elif isinstance(func, dict):
            covs = [k for k in func if "cov" in func[k]]
            if len(covs) > 0:
                if len(func) == 1 and len(func[covs[0]]) == 1 and "cov" in func[covs[0]]:
                    return self[covs[0]].cov(1, 1.0, True)
                if len(func) >= 1:
                    for k in covs:
                      func[k].remove("cov")
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

    @check_association
    def to_dict(self, orient="dict", into=dict):
        """
        returns dataframe columns as dictionary
        """
        if orient not in ["dict", "list"]:
            raise ValueError("to_dict: supported only 'dict' and 'list' " \
                             "as for 'orient' parameter!\n")
        if into != dict:
        #if not isinstance(into, dict):
            raise ValueError(\
            "to_dict: supported only dictionary as per return type!\n")
        return self.__to_dict_impl(orient=orient, into=into)

    def __get_index_column(self):
        """ returns index column if present """
        if not self.has_index():
            raise ValueError("input dataframe doesn't have index column!\n")

        (host, port) = FrovedisServer.getServerInstance()
        indx_dvec = rpclib.get_frovedis_col(host, port, self.get(),
                                       self.index.name.encode('ascii'),
                                       self.index.dtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return FrovedisDvector(indx_dvec).to_numpy_array()

    def __to_dict_impl(self, orient="dict", into=dict, include_index=True):
        """
        helper for to_dict
        """

        out_data = OrderedDict()
        if orient == "dict": # presence of index is must
            if self.has_index():
                indx_arr = self.__get_index_column()
            else:
                indx_arr = np.arange(len(self), dtype=np.int64)
        elif orient == "list":
            if include_index and self.has_index(): # extract index, if presents
                out_data[self.index.name] = self.__get_index_column()

        else:
            raise ValueError("to_dict: supported 'orient' values are " \
                             "'dict' and 'list' only!\n")

        (host, port) = FrovedisServer.getServerInstance()
        for i in range(0, len(self.columns)):
            cc = self.__cols[i]
            tt = self.__types[i]
            dvec = rpclib.get_frovedis_col(host, port, self.get(),
                                           cc.encode('ascii'), tt)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            col_arr = FrovedisDvector(dvec).to_numpy_array()
            #TODO: fix NULL conversion in case of string-vector;
            #TODO: fix numeric to boolean conversion in case of boolean-vector
            if orient == "dict":
                out_data[cc] = dict(zip(indx_arr, col_arr)) # this is very slow...
            else: # list
                out_data[cc] = col_arr

        return out_data

    @check_association
    def to_pandas(self):
        """
        returns a pandas dataframe object from frovedis dataframe
        """
        res = pd.DataFrame(self.__to_dict_impl(orient="list", \
                           include_index=True))

        if self.has_index():
            res.set_index(self.index.name, inplace=True)
            if (self.index.dtype == DTYPE.BOOL):
                res.index = res.index.to_series().replace({0: False, 1: True})
            elif (self.index.dtype == DTYPE.STRING):
                res.index = res.index.to_series().replace({"NULL": None})

        for col in self.columns:
            # BOOL treatment
            if (self.__dict__[col].dtype == DTYPE.BOOL):
                res[col].replace(to_replace={0: False, 1: True}, inplace=True)
            # NULL treatment for string columns
            elif (self.__dict__[col].dtype == DTYPE.STRING):
                res[col].replace(to_replace={"NULL": None}, inplace=True)

        if self.is_series:
            res = res[self.columns[0]]

        if self.__multi_index is not None:
            res.set_index(self.__multi_index, drop=True, inplace=True)

        if self.__multi_cols is not None:
            res.columns = self.__multi_cols

        return res

    @deprecated("Use to_pandas() instead!\n")
    def to_panda_dataframe(self):
        return self.to_pandas()

    @deprecated("Use to_pandas() instead!\n")
    def to_pandas_dataframe(self):
        return self.to_pandas()

    @check_association
    def to_numpy(self, dtype=None, copy=False, na_value=None):
        """
        frovedis dataframe to numpy array conversion
        """
        out_data = self.__to_dict_impl(orient="list", include_index=False)
        return np.array(list(out_data.values()), dtype=dtype).T

    @property
    def values(self):
        """
        frovedis dataframe to numpy array conversion
        """
        return self.to_numpy()

    @values.setter
    def values(self, val):
        """values setter"""
        raise AttributeError(\
            "attribute 'values' of DataFrame object is not writable")

    @check_association
    def to_frovedis_rowmajor_matrix(self, t_cols, dtype=np.float32):
        """
        to_frovedis_rowmajor_matrix
        """
        for item in t_cols: # implicit checks for iterable on 't_cols'
            if item not in self.__cols:
                raise ValueError("No column named: %s" % str(item))
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

    @check_association
    def to_frovedis_colmajor_matrix(self, t_cols, dtype=np.float32):
        """
        to_frovedis_colmajor_matrix
        """
        for item in t_cols: # implicit checks for iterable on 't_cols'
            if item not in self.__cols:
                raise ValueError("No column named: %s" % str(item))
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

    @check_association
    def to_frovedis_crs_matrix(self, t_cols, cat_cols,
                               dtype=np.float32, #default type: float
                               need_info=False):
        """
        to_frovedis_crs_matrix
        """
        for item in t_cols: # implicit checks for iterable on 't_cols'
            if item not in self.__cols:
                raise ValueError("No column named: %s" % str(item))
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

    @check_association
    def to_frovedis_crs_matrix_using_info(self, info, dtype=np.float32):
        """
        to_frovedis_crs_matrix_using_info
        """
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
        """apply"""
        return self.to_pandas_dataframe()\
                   .apply(func, axis, raw, result_type, args)

    @check_association
    def isna(self):
        """isna"""
        ncol = len(self.__cols)
        cols_ptr = get_string_array_pointer(self.__cols)
        (host, port) = FrovedisServer.getServerInstance()
        proxy = rpclib.isnull_frovedis_dataframe(host, port, self.get(), \
                                        cols_ptr, ncol, self.has_index())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        ret = DataFrame()
        ret.num_row = len(self)
        ret.index = self.index.copy()
        ret_cols = list(self.__cols)
        ret_types = [DTYPE.BOOL]*len(self)
        ret.load_dummy(proxy, ret_cols, ret_types)
        if not self.has_index():
          ret.add_index("index")
        return ret

    def isnull(self):
        """isnull"""
        return self.isna()

    @check_association
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
                           keep.encode('ascii'))
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

    @check_association
    def drop_cols(self, targets, inplace=False):
        """ drops specified columns from input dataframe. """
        targets = check_string_or_array_like(targets, "drop")

        if inplace:
            for item in targets:
                try:
                    idx = self.__cols.index(item)
                    self.__cols.pop(idx)
                    self.__types.pop(idx)
                    del self.__dict__[item]
                except ValueError as ve: #item might not be present in col-list
                    raise ValueError("drop: No column named: %s" % str(item))

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
                    raise ValueError("drop: No column named: %s" % str(item))
                else:
                    cols.remove(item)
            return self[cols] # just select the required columns

    @check_association
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
        """append_column"""
        if col_name is None:
            raise ValueError("cannot label index with a null key")
        elif not isinstance(col_name, str):
            raise TypeError("expected a string parameter for 'col_name'! "
                            "received: '%s'" % (type(col_name).__name__))

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

        data_type = TypeUtil.to_id_dtype(data_type)
        dvec = self.__get_dvec(data_type, data)
        return self.__append_column_impl(col_name, data_type, dvec, pos, drop_old)

    def __append_column_impl(self, col_name, data_type, dvec, pos, drop_old):
        df_proxy = -1
        if self.__fdata:
            df_proxy = self.get()
        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_append_column(host, port, df_proxy,
                                           col_name.encode("ascii"),
                                           data_type, dvec.get(),
                                           pos, drop_old)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        # dvector 'dvec' is already destructed during append column.
        # thus resetting the metadata to avoid double free issue from
        # server side during destructor call of the dvector 'dvec'
        dvec.reset()

        names = dummy_df["names"]
        types = dummy_df["types"]

        if data_type == DTYPE.BOOL:
            ind = names.index(col_name)
            types[ind] = DTYPE.BOOL

        bool_cols = set([self.__cols[i] for i in range(len(self.__types)) \
                        if self.__types[i] == DTYPE.BOOL])
        if len(bool_cols) > 0:
            for i in range(len(names)):
                if names[i] in bool_cols:
                    types[i] = DTYPE.BOOL

        self.num_row = dummy_df["nrow"]
        if self.has_index() or \
           df_proxy == -1: # empty dataframe case: new index column is added
            self.index = FrovedisColumn(names[0], types[0]) #setting index
            self.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        else:
            self.load_dummy(dummy_df["dfptr"], names[0:], types[0:])
        return self

    def __get_dtype_name(self, arr): #TODO: improve
        """get_dtype_name"""
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
        if key is None:
            raise ValueError("cannot label index with a null key")
        elif isinstance(key, str):
            key = [key]
        elif not isinstance(key, list):
            raise TypeError("__setitem__: supported types for 'key' parameter "
                            "are either string or list! received: '%s'"
                            % (type(key).__name__))

        if isinstance(value, (DataFrame, pd.DataFrame)):
            self.copy_column(value, names_as=key, inplace=True)
        elif isinstance(value, dfoperator):
            pos = -1 # insert at last
            drop_old = False
            self = self.__append_column_impl(key[0], DTYPE.INT, value.mask, pos, drop_old)
        else:
            if np.isscalar(value):
                if self.__fdata is None:
                    raise ValueError("__setitem__: Frovedis currently doesn't "
                                     "support adding column with scalar "
                                     "values on empty dataframe!")
                is_imax = (value == np.iinfo(np.int32).max)
                value = [value] * len(self)
                if is_imax: # check to avoid long array construction by default
                    value = np.asarray(value, dtype=np.int32)
                else:
                    value = np.asarray(value) # deduce type
            elif isinstance(value, (list, pd.Series, np.ndarray)):
                value = np.asarray(value)
                value = np.array(list(map(lambda x: np.nan if x is None else x, value)))
                if value.ndim != 1:
                    raise ValueError(\
                    "array is not broadcastable to correct shape")
                if self.__fdata is not None and len(value) != len(self):
                    raise ValueError("Length of values (%d) does not match " \
                    "with length of index (%d)" % (len(value), len(self)))
            else:
                raise ValueError("__setitem__: Given column data type '{}' "
                                 "is not supported!".format(type(value)))

            col_dtype = np.dtype(self.__get_dtype_name(value))
            for k in key:
                drop_old = False
                if self.__fdata and k in self.columns:
                    drop_old = True
                self = self.__append_column(k, col_dtype, value, None, drop_old)
        return self

    def insert(self, loc, column, value, allow_duplicates=False):
        """
        Insert column into DataFrame at specified location.
        """
        if allow_duplicates == True: #TODO: allow it
            raise ValueError("insert: Frovedis does not support duplicate "
                             "column names !")

        if np.isscalar(value):
            if self.__fdata is None:
                raise ValueError("insert: Frovedis currently doesn't "
                                 "support adding column with scalar values "
                                 "on empty dataframe!")
            is_imax = (value == np.iinfo(np.int32).max)
            value = [value] * len(self)
            if is_imax: # check to avoid long array construction by default
                value = np.asarray(value, dtype=np.int32)
            else:
                value = np.asarray(value) # deduce type
        elif isinstance(value, (list, pd.Series, np.ndarray)):
            value = np.asarray(value)
            value = np.array(list(map(lambda x: np.nan if x is None else x, value)))
            if value.ndim != 1:
                raise ValueError("array is not broadcastable to correct shape")
            if self.__fdata is not None and len(value) != len(self):
                raise ValueError(\
                "Length of values does not match length of index")
        else:
            raise ValueError("insert: Given column data type '{}' is "
                             "not supported!".format(type(value)))

        col_dtype = np.dtype(self.__get_dtype_name(value))
        if self.__fdata is None:
            if loc != 0: #insert in empty df, allowed loc = 0 only
                raise IndexError("insert: index '{}' is out of bounds for the "
                                 "given dataframe!".format(loc))
            self = self.__append_column(column, col_dtype, value)
        else:
            if column in self.columns:
                raise ValueError("insert: The given column '{}' already exists !"
                                .format(column))
            self = self.__append_column(column, col_dtype, value, loc)
        return self

    @check_association
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
    @check_association
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
                                  verify_integrity)
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

    @check_association
    def reset_index(self, drop=False, inplace=False): #TODO: support other params
        """ resets the index column of self """
        if inplace:   # copy may still take place, if self needs materialize
            ret = self
        else:
            ret = self.copy()

        if ret.has_index():
            (host, port) = FrovedisServer.getServerInstance()
            dummy_df = rpclib.df_reset_index(host, port, ret.get(), \
                                             drop)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            names = dummy_df["names"]
            types = dummy_df["types"]

            bool_cols = set([self.__cols[i] for i in range(len(self.__types)) \
                            if self.__types[i] == DTYPE.BOOL])
            if len(bool_cols) > 0:
                for i in range(len(names)):
                    if names[i] in bool_cols:
                        types[i] = DTYPE.BOOL

            ret.index = FrovedisColumn(names[0], types[0]) # with new index
            ret.num_row = dummy_df["nrow"]
            ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        else:
            ret = ret.add_index("index")
        return None if inplace else ret

    @check_association
    def copy_index(self, from_df, inplace=False, overwrite=True):
        """ copies index column from 'from_df' to self """
        from_df = DataFrame.asDF(from_df)
        if not from_df.has_index():
            raise ValueError("from_df doesn't have any index column!")

        if self.has_index():
            if not overwrite and from_df.index.name == self.index.name:
                raise ValueError("input dataframe already has an index "\
                                 "column with same name!")

        if inplace:   # copy may still take place, if self needs materialize
            ret = self
        else:
            ret = self.copy()

        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_copy_index(host, port, ret.get(), from_df.get(), \
                             from_df.index.name.encode("ascii"), \
                             from_df.index.dtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret.index = FrovedisColumn(names[0], types[0]) #setting index
        ret.num_row = dummy_df["nrow"]
        ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        return None if inplace else ret

    def copy_column(self, from_df, names_as=None, inplace=False):
        """
        copies all columns from 'from_df' to self. if 'names_as' is given,
        copied columns would be renamed based on 'names_as'
        """
        names = list(from_df.columns)
        if names_as is None:
            names_as = names
        elif isinstance(names_as, str):
            names_as = [names_as]
        elif not isinstance(names_as, list):
            raise TypeError("copy: supported types for 'names_as' parameter "
                            "is either string or list! Received '%s' "
                            % (type(names_as).__name__))

        if len(names) != len(names_as):
            raise ValueError("Columns must be same length as key")

        from_df = DataFrame.asDF(from_df)
        types = np.asarray(from_df.get_types(), dtype=c_short)
        sz = len(types)
        names_arr = get_string_array_pointer(names)
        names_as_arr = get_string_array_pointer(names_as)
        types_arr = types.ctypes.data_as(POINTER(c_short))

        if inplace:   # copy may still take place, if self needs materialize
            ret = self
        else:
            ret = self.copy()

        if ret.__fdata is None: # self is empty DataFrame
            ret.__fdata = -1

        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_copy_column(host, port, ret.get(), \
                                         from_df.get(), names_arr, \
                                         names_as_arr, types_arr, sz)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret.index = FrovedisColumn(names[0], types[0]) #setting index
        ret.num_row = dummy_df["nrow"]
        ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        return None if inplace else ret

    @check_association
    def add_index(self, name): # behavior: inplace=True
        """ adds index column to self """
        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_add_index(host, port, self.get(),
                                    name.encode("ascii"))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        names = dummy_df["names"]
        types = dummy_df["types"]

        bool_cols = set([self.__cols[i] for i in range(len(self.__types)) \
                        if self.__types[i] == DTYPE.BOOL])
        if len(bool_cols) > 0:
            for i in range(len(names)):
                if names[i] in bool_cols:
                    types[i] = DTYPE.BOOL

        self.index = FrovedisColumn(names[0], types[0]) #setting index
        self.num_row = dummy_df["nrow"]
        self.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        return self

    @check_association
    def astype(self, dtype, copy=True, errors='raise',
               check_bool_like_string=False):
        """
        Casts a frovedis DataFrame object to a specified dtype

        check_bool_like_string: added parameter; if True, can cast string column
        having bool like case-insensitive strings (True, False, yes, No,
        On, Off, Y, N, T, F) to boolean columns
        """
        t_cols = []
        t_dtypes = []
        if isinstance (dtype, (str, type)):
            numpy_dtype = np.dtype(dtype)
            t_cols = list(self.columns)
            t_dtypes = [TypeUtil.to_id_dtype(numpy_dtype)] * len(t_cols)
        elif isinstance (dtype, dict): # might include index as well
            for k, v in dtype.items():
                if k in self.columns or \
                    (self.has_index() and k == self.index.name):
                    t_cols.append(k)
                    t_dtypes.append(TypeUtil.to_id_dtype(np.dtype(v)))
        else:
            raise TypeError("astype: supports only string, numpy.dtype " \
                            "or dict object as for 'dtype' parameter!")

        ret = self.copy() # always returns a new dataframe (after casting)
        (host, port) = FrovedisServer.getServerInstance()
        t_cols_ptr = get_string_array_pointer(t_cols)
        type_arr = np.asarray(t_dtypes, dtype=c_short)
        t_dtypes_ptr = type_arr.ctypes.data_as(POINTER(c_short))
        dummy_df = rpclib.df_astype(host, port, ret.get(), t_cols_ptr, \
                                    t_dtypes_ptr, len(t_cols), \
                                    check_bool_like_string)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        names = dummy_df["names"]
        types = dummy_df["types"]
        ret.num_row = dummy_df["nrow"]
        for i in range(0, len(t_dtypes)):
            if t_dtypes[i] == DTYPE.BOOL:
                col = t_cols[i]
                ind = names.index(col)
                types[ind] = DTYPE.BOOL
        if self.has_index():
            ret.index = FrovedisColumn(names[0], types[0]) #setting index
            ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        else:
            ret.load_dummy(dummy_df["dfptr"], names[0:], types[0:])
        return ret

    @check_association
    def append(self, other, ignore_index=False, verify_integrity=False,
               sort=False):
        """append"""
        if isinstance(other, DataFrame):
            other = [other]
            is_frov_df = [True]
        elif isinstance(other, pd.DataFrame):
            other = [DataFrame.asDF(other)]
            is_frov_df = [False]
        elif isinstance(other, list):
            is_frov_df = [isinstance(e, DataFrame) for e in other]
            other = [DataFrame.asDF(e) for e in other]
        else:
            raise ValueError("append: unsupported type '%s' for "
                             "'other'!" % (type(other).__name__))

        dfs = [self] + other
        is_frov_df = [True] + is_frov_df # prepending [True]; self is DataFrame
        res_names = [self.columns]
        for e in other:
            res_names.append(e.columns)
        res_names = union_lists(res_names)
        res_dtypes = [infer_dtype(dfs, col) for col in res_names]
        astype_input = dict(zip(res_names, res_dtypes))
        #print(astype_input)
        dfs = add_null_column_and_type_cast(dfs, is_frov_df, astype_input)

        # preserving column order as in self, if not to be sorted
        res_names = sorted(dfs[0].columns) if sort else dfs[0].columns
        all_cols = [dfs[0].index.name] + res_names # adding index
        proxies = np.asarray([e.get() for e in dfs[1:]]) # default dtype=long
        proxies_ptr = proxies.ctypes.data_as(POINTER(c_long))
        verify_integrity = False if ignore_index else verify_integrity
        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_union(host, port, dfs[0].get(), \
                                   proxies_ptr, len(proxies),  \
                                   get_string_array_pointer(all_cols), \
                                   len(all_cols), verify_integrity)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        names = dummy_df["names"]
        types = dummy_df["types"]
        for c, t in astype_input.items():
            if t == np.bool:
                # searching index, since dictionary items might not be ordered
                ind = names.index(c)
                types[ind] = DTYPE.BOOL
        res = DataFrame().load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        res.index = FrovedisColumn(names[0], types[0]) #setting index
        res.num_row = dummy_df["nrow"]
        if ignore_index:
            res.reset_index(inplace=True, drop=True)
        return res

    @check_association
    def __set_col_order(self, new_cols):
        """set_col_order"""
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
        """get_types"""
        return self.__types

    def get_dtype(self, colname):
        """
        returns numpy_dtype of given column,
        in case presents in (self.index.name or self.columns).
        otherwise, raises ValueError
        """
        if self.has_index() and colname == self.index.name:
            ret = TypeUtil.to_numpy_dtype(self.index.dtype)
        elif colname in self.columns:
            ret = TypeUtil.to_numpy_dtype(self.__dict__[colname].dtype)
        else:
            raise ValueError("column not found: '%s'" % (colname))
        return ret

    @check_association
    def fillna(self, value=None, method=None, axis=None,
               inplace=False, limit=None, downcast=None):
        """
        replaces nulls in each column with given value
        """
        if method is not None:
            raise NotImplementedError("fillna: currently doesn't support "
                                      "method: '%s'" % method)

        if limit is not None:
            raise NotImplementedError("fillna: currently doesn't support "
                                      "limit: '%d'" % limit)

        if downcast is not None:
            raise NotImplementedError("fillna: currently doesn't support "
                                      "downcasting!")

        if axis is None:
            axis = 1 # columns
        elif axis not in (1, 'columns'):
            raise NotImplementedError("fillna: can only be performed "\
                                      "on axis=1 (columns)")

        if value is None:
            raise ValueError("fillna: must specify a value")
        elif value is np.nan:
            return None if inplace else self.copy() # no need for any replacement
        elif not np.isscalar(value):
            raise NotImplementedError("fillna: curreently supports only "\
                                      "scalar values for 'value' parameter, "\
                                      "received: '%s'" % type(value).__name__)
        value = str(value)
        if inplace:
            ret = self
        else:
            ret = DataFrame(is_series=self.is_series)

        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_fillna(host, port, \
                       self.get(), value.encode('ascii'), \
                       self.has_index())
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret.num_row = dummy_df["nrow"]
        if self.has_index():
            ret.index = FrovedisColumn(names[0], types[0]) #setting index
            ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        else:
            ret.load_dummy(dummy_df["dfptr"], names, types)
        return None if inplace else ret

    # optimized implementation for query like "self.isna().sum(axis=0)"
    @check_association
    def countna(self, axis=0):
        """ counts number of missing values in the given axis """
        if axis not in [0, 1, "index", "columns"]:
            raise ValueError("No axis named '%s' for DataFrame object" % str(axis))
        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_countna(host, port, self.get(), \
                                     axis, self.has_index())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        # returns a series
        ret = DataFrame(is_series=True)
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret.num_row = dummy_df["nrow"]
        ret.index = FrovedisColumn(names[0], types[0]) #setting index
        ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        return ret

    @check_association
    def dropna(self, axis=0, how='any', thresh=None, subset=None, inplace=False):
        """ drops rows/columns having null values"""
        if inplace:
            ret = self
        else:
            ret = DataFrame(is_series=self.is_series)

        if thresh is None:
            thresh = np.iinfo(np.uint).max
        elif not isinstance(thresh, int):
            raise ValueError(\
            "dropna: expected an integer value for 'thresh' parameter!\n")

        (host, port) = FrovedisServer.getServerInstance()
        if axis == 0:   # dropna by rows (extraction subset: column names)
            if subset is None:
                subset = self.columns
            sz = len(subset)
            targets_ptr = get_string_array_pointer(subset)
            dummy_df = rpclib.df_dropna_by_rows(host, port, \
                           self.get(), targets_ptr, sz, \
                           how.encode('ascii'), thresh)
        elif axis == 1: # dropna by cols (extraction subset: index values)
            if not self.has_index():
                raise ValueError(
                "dropna: input dataframe doesn't have an index column!")
            index_nm = self.index.name.encode('ascii')
            if subset is None:
                subset = np.asarray([], dtype=np.int32) # int32: dummy type
                sz = len(subset)
                targets_ptr = subset.__array_interface__['data'][0]
                dummy_df = rpclib.df_dropna_by_cols_with_numeric_icol( \
                               host, port, \
                               self.get(), index_nm, targets_ptr, sz, \
                               how.encode('ascii'), thresh, DTYPE.INT)
            else:
                tt = self.index.dtype
                subset = np.asarray(subset, dtype=TypeUtil.to_numpy_dtype(tt))
                sz = len(subset)
                if tt != DTYPE.STRING:
                    targets_ptr = subset.__array_interface__['data'][0]
                    dummy_df = rpclib.df_dropna_by_cols_with_numeric_icol( \
                                   host, port, \
                                   self.get(), index_nm, targets_ptr, sz, \
                                   how.encode('ascii'), thresh, tt)
                else:
                    targets_ptr = get_string_array_pointer(subset)
                    dummy_df = rpclib.df_dropna_by_cols_with_string_icol( \
                                   host, port, \
                                   self.get(), index_nm, targets_ptr, sz, \
                                   how.encode('ascii'), thresh)
        else:
            raise ValueError(\
            "dropna: Unsupported axis parameter: '%d'" % (axis))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        names = dummy_df["names"]
        types = dummy_df["types"]
        if self.has_index():
            ret.index = FrovedisColumn(names[0], types[0]) #setting index
            ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        else:
            ret.load_dummy(dummy_df["dfptr"], names[0:], types[0:])
        ret.num_row = dummy_df["nrow"]
        return None if inplace else ret

    @check_association
    def __binary_operator_impl(self, other, op_type, \
                               axis='columns', level=None,
                               fill_value=None, is_rev=False):
        """
        implements binary opeartions (+, -, *, /, //, %) of
        two dataframes invoking server side operations
        """
        if not self.has_index():
            raise TypeError(op_type + \
            ": input dataframe must have index column!")

        if self.get() is None:
            raise ValueError(op_type + ": invoked on empty dataframe!")

        if DTYPE.STRING in self.get_types():
            raise NotImplementedError(op_type + \
            ": table contains string column. Arithmetic operations "\
            "on string column is not supported currently!")

        if axis not in (1, 'columns'):
            raise NotImplementedError(op_type + \
            ": can only be performed on axis=1 (columns)")

        if fill_value is None or fill_value is np.nan:
            fillv = "NULL"
            fillv_dt = "none"
        elif np.isscalar(fill_value):
            fillv = str(fill_value)
            fillv_dt = get_python_scalar_type(fill_value)
        else:
            raise NotImplementedError(op_type + \
            ": supports either None or scalar as for 'fill_value', "\
            "received: %s" % (fill_value))

        if np.isscalar(other):
            if not isinstance(other, (numbers.Number, np.integer, \
                              np.float32, np.float64)):
                raise TypeError(op_type + \
                ": supported type of 'other' parameter is either DataFrame "\
                "or numbers. Received: '%s'" % type(other).__name__)
            immed = True
            immed_val = str(other)
            immed_dt = get_python_scalar_type(other)
            is_series = self.is_series
        elif isinstance(other, (list, np.ndarray)):
            other = np.asarray(other)
            is_numeric = all([isinstance(e, numbers.Number) for e in other])
            if not is_numeric:
                raise NotImplementedError(op_type + \
                ": array-like other contains non-numeric values!")
            if other.ndim != 1:
                raise ValueError(op_type + \
                ": multi-dimensional array-like 'other' is not supported!")
            # below implementation is according to axis = 1
            nrow = len(self)
            ncol = len(self.columns)
            if ncol == 1:
                if len(other) != nrow:
                    raise ValueError(op_type + ": Unable to coerce to Series," \
                                     " length must be %d: given %d"
                                     % (nrow, len(other)))
                other = DataFrame(pd.DataFrame(other, columns=self.columns))
            else:
                if len(other) != ncol:
                    raise ValueError(op_type + ": Wrong number of items " \
                                     "passed %d, placement implies %d" \
                                     % (len(other), ncol))
                tmp_df = pd.DataFrame()
                for i in range(0, ncol):
                    tmp_df[self.columns[i]] = [other[i]] * nrow
                other = DataFrame(tmp_df)
            immed = False
            is_series = False
            other.copy_index(self, inplace=True, overwrite=True)
        else:
            other = DataFrame.asDF(other)
            if other.get() is None:
                raise ValueError(op_type + ": right table is empty!")
            if DTYPE.STRING in other.get_types():
                raise NotImplementedError(op_type + \
                ": right table contains string column. Arithmetic operations "\
                "on string column is not supported currently!")
            immed = False
            is_series = self.is_series and other.is_series

        (host, port) = FrovedisServer.getServerInstance()
        # nan values can be generated during pow, div, mod operations
        # and pandas treats nan values as null
        if op_type in ("pow", "fdiv", "idiv", "mod"):
            nan_is_null = True
        else:
            nan_is_null = False

        if immed:
            # any operation with immediate value as nan would produce nan
            if other is np.nan:
                nan_is_null = True
            dummy_df = rpclib.df_immed_binary_operation(host, port, \
                       self.get(), immed_val.encode('ascii'), \
                       immed_dt.encode('ascii'), \
                       op_type.encode('ascii'), is_rev, nan_is_null)
        else:
            if is_rev:
                left, right = other, self
            else:
                left, right = self, other
            dummy_df = rpclib.df_binary_operation(host, port, left.get(), \
                       right.get(), is_series, fillv.encode('ascii'), \
                       fillv_dt.encode('ascii'), \
                       op_type.encode('ascii'), nan_is_null)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        ret = DataFrame(is_series=is_series)
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret.index = FrovedisColumn(names[0], types[0]) #setting index
        ret.num_row = dummy_df["nrow"]
        ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        #sorting columns as per pandas result
        ret.__set_col_order([names[0]] + sorted(names[1:]))
        return ret

    @check_association
    def abs(self):
        """
        returns resultant dataframe after performing
        absolute value of each column
        """
        if not self.has_index():
            raise TypeError("abs: input dataframe must have index column!")

        if DTYPE.STRING in self.get_types():
            raise TypeError("bad operand type for abs(): 'str'")

        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_abs(host, port, self.get())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        ret = DataFrame(is_series=self.is_series)
        names = dummy_df["names"]
        types = dummy_df["types"]
        # treatment of bool columns
        for i in range(0, len(names)):
            if self.__dict__[names[i]].dtype == DTYPE.BOOL:
                types[i] = DTYPE.BOOL
        ret.index = FrovedisColumn(names[0], types[0]) #setting index
        ret.num_row = dummy_df["nrow"]
        ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        return ret

    def add(self, other, axis='columns', level=None, fill_value=None):
        """ returns resultant dataframe after performing self + other """
        return self.__binary_operator_impl(other, "add", axis, level, \
                                           fill_value, is_rev=False)

    def radd(self, other, axis='columns', level=None, fill_value=None):
        """ returns resultant dataframe after performing other + self"""
        return self.__binary_operator_impl(other, "add", axis, level, \
                                           fill_value, is_rev=True)

    def __add__(self, other):
        """
        overloaded binary operator+ (wrapper of self.add())
        returns resultant dataframe after adding self + other
        """
        return self.add(other)

    def __IADD__(self, other):
        """
        overloaded binary operator+= to perform self += other
        """
        self = self.add(other)

    def sub(self, other, axis='columns', level=None, fill_value=None):
        """ returns resultant dataframe after performing self - other """
        return self.__binary_operator_impl(other, "sub", axis, level, \
                                           fill_value, is_rev=False)

    def rsub(self, other, axis='columns', level=None, fill_value=None):
        """ returns resultant dataframe after performing other - self"""
        return self.__binary_operator_impl(other, "sub", axis, level, \
                                           fill_value, is_rev=True)

    def __sub__(self, other):
        """
        overloaded binary operator- (wrapper of self.sub())
        returns resultant dataframe after adding self - other
        """
        return self.sub(other)

    def __ISUB__(self, other):
        """
        overloaded binary operator-= to perform self -= other
        """
        self = self.sub(other)

    def mul(self, other, axis='columns', level=None, fill_value=None):
        """ returns resultant dataframe after performing self * other """
        return self.__binary_operator_impl(other, "mul", axis, level, \
                                           fill_value, is_rev=False)

    def rmul(self, other, axis='columns', level=None, fill_value=None):
        """ returns resultant dataframe after performing other * self"""
        return self.__binary_operator_impl(other, "mul", axis, level, \
                                           fill_value, is_rev=True)

    def __mul__(self, other):
        """
        overloaded binary operator* (wrapper of self.mul())
        returns resultant dataframe after adding self * other
        """
        return self.mul(other)

    def __IMUL__(self, other):
        """
        overloaded binary operator*= to perform self *= other
        """
        self = self.mul(other)

    def truediv(self, other, axis='columns', level=None, fill_value=None):
        """ returns resultant dataframe after performing self / other """
        return self.__binary_operator_impl(other, "fdiv", axis, level, \
                                           fill_value, is_rev=False)

    def rtruediv(self, other, axis='columns', level=None, fill_value=None):
        """ returns resultant dataframe after performing other / self"""
        return self.__binary_operator_impl(other, "fdiv", axis, level, \
                                           fill_value, is_rev=True)

    def div(self, other, axis='columns', level=None, fill_value=None):
        """ returns resultant dataframe after performing self / other """
        return self.truediv(other, axis, level, fill_value)

    def rdiv(self, other, axis='columns', level=None, fill_value=None):
        """ returns resultant dataframe after performing self / other """
        return self.rtruediv(other, axis, level, fill_value)

    def __div__(self, other):
        """
        overloaded binary operator/ for python 2.x (wrapper of self.truediv())
        returns resultant dataframe after adding self / other
        """
        return self.truediv(other)

    def __truediv__(self, other):
        """
        overloaded binary operator/ (wrapper of self.truediv())
        returns resultant dataframe after adding self / other
        """
        return self.truediv(other)

    def __IDIV__(self, other):
        """
        overloaded binary operator/= to perform self /= other
        """
        self = self.truediv(other)

    def floordiv(self, other, axis='columns', level=None, fill_value=None):
        """ returns resultant dataframe after performing self // other """
        return self.__binary_operator_impl(other, "idiv", axis, level, \
                                           fill_value, is_rev=False)

    def rfloordiv(self, other, axis='columns', level=None, fill_value=None):
        """ returns resultant dataframe after performing other // self"""
        return self.__binary_operator_impl(other, "idiv", axis, level, \
                                           fill_value, is_rev=True)

    def __floordiv__(self, other):
        """
        overloaded binary operator// (wrapper of self.floordiv())
        returns resultant dataframe after adding self // other
        """
        return self.floordiv(other)

    def __IFLOORDIV__(self, other):
        """
        overloaded binary operator//= to perform self //= other
        """
        self = self.floordiv(other)

    def mod(self, other, axis='columns', level=None, fill_value=None):
        """ returns resultant dataframe after performing self % other """
        return self.__binary_operator_impl(other, "mod", axis, level, \
                                           fill_value, is_rev=False)

    def rmod(self, other, axis='columns', level=None, fill_value=None):
        """ returns resultant dataframe after performing other % self"""
        return self.__binary_operator_impl(other, "mod", axis, level, \
                                           fill_value, is_rev=True)

    def __mod__(self, other):
        """
        overloaded binary operator% (wrapper of self.mod())
        returns resultant dataframe after adding self % other
        """
        return self.mod(other)

    def __IMOD__(self, other):
        """
        overloaded binary operator%= to perform self %= other
        """
        self = self.mod(other)

    def pow(self, other, axis='columns', level=None, fill_value=None):
        """ returns resultant dataframe after performing self ** other """
        return self.__binary_operator_impl(other, "pow", axis, level, \
                                           fill_value, is_rev=False)

    def rpow(self, other, axis='columns', level=None, fill_value=None):
        """ returns resultant dataframe after performing other ** self"""
        return self.__binary_operator_impl(other, "pow", axis, level, \
                                           fill_value, is_rev=True)

    def __pow__(self, other):
        """
        overloaded binary operator** (wrapper of self.pow())
        returns resultant dataframe after adding self ** other
        """
        return self.pow(other)

    def __IPOW__(self, other):
        """
        overloaded binary operator**= to perform self **= other
        """
        self = self.pow(other)

    @check_association
    def head(self, n=5):
        """
        return first n rows of the dataframe
        """
        if not isinstance(n, int):
            raise TypeError("head: Invalid type '%s' for n"
                            "is provided!" % (type(n).__name__))
        if n < 0:
            nrows = len(self)
            n = max(0, nrows + n)

        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_head(host, port, self.get(), n)

        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        ret = DataFrame()
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret.num_row = dummy_df["nrow"]
        if self.has_index():
            ret.index = FrovedisColumn(names[0], types[0]) #setting index
            ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        else:
            ret.load_dummy(dummy_df["dfptr"], names, types)
        return ret

    @check_association
    def tail(self, n=5):
        """
        return last n rows of the dataframe
        """
        if not isinstance(n, int):
            raise TypeError("tail: Invalid type '%s' for n"
                            "is provided!" % (type(n).__name__))
        if n < 0:
            nrows = len(self)
            n = max(0, nrows + n)

        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_tail(host, port, self.get(), n)

        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        ret = DataFrame()
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret.num_row = dummy_df["nrow"]
        if self.has_index():
            ret.index = FrovedisColumn(names[0], types[0]) #setting index
            ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        else:
            ret.load_dummy(dummy_df["dfptr"], names, types)
        return ret

    @check_association
    def __filter_slice_range(self, target):
        """
        return filtered dataframe in given slice range
        """
        a, b, c = target.start, target.stop, target.step

        if c is None:
            c = 1
        elif not isinstance(c, int):
            raise TypeError("filter_slice_range: slice step must be integer or None!")

        if c == 0:
            raise ValueError("filter_slice_range: slice step cannot be zero!")
        elif c < 0:
            raise ValueError("filter_slice_range: slice step cannot be negative!")

        nrows = len(self)
        a_int = -1
        b_int = -1

        ## check for bool moved to first
        ## since isinstance(True, int) also returns True

        if  isinstance(a, bool) or isinstance(b, bool) \
            or not (isinstance(a, int) or a is None) \
            or not (isinstance(b, int) or b is None):
            # non - integer slice
            if a is None:
                a_int = 0
            else:
                aloc = self.__get_index_loc_impl(a)
                if len(aloc) != 1:
                    raise ValueError("Cannot get slice bound for " \
                                     "non-unique label '%s'" % (a))
                a_int = int(aloc[0])

            if b is None:
                b_int = nrows
            else:
                bloc = self.__get_index_loc_impl(b)
                if len(bloc) != 1:
                    raise ValueError("Cannot get slice bound for " \
                                     "non-unique label '%s'" % (b))
                b_int = int(bloc[0])
        else:
            # integer slice
            a_int = int(a) if a is not None else 0
            b_int = int(b) if b is not None else nrows

            if a_int < 0:
                a_int = max(0, nrows + a_int)
            if b_int < 0:
                b_int = max(0, nrows + b_int)

        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_slice_range(host, port, self.get(), a_int, b_int, c)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        ret = DataFrame()
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret.num_row = dummy_df["nrow"]
        if self.has_index():
            ret.index = FrovedisColumn(names[0], types[0]) #setting index
            ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        else:
            ret.load_dummy(dummy_df["dfptr"], names, types)
        return ret

    @check_association
    def __get_index_loc_impl(self, value):
        """
        helper for get_index_loc
        """
        if not self.has_index():
            raise ValueError("get_index_loc: no index column in input df!")

        index_name = self.index.name
        dtype = self.index.dtype

        (host, port) = FrovedisServer.getServerInstance()
        ret = rpclib.df_get_index_loc(host, port, self.get(), \
                                      index_name.encode('ascii'), \
                                      str(value).encode('ascii'), \
                                      dtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return ret

    # similar to pandas pd.Index.get_loc(value)
    def get_index_loc(self, value):
        """
        returns row-ids (int, slice or maask) for given value in index column
        """
        idx = self.__get_index_loc_impl(value)
        sz = len(idx)
        if sz == 1:          # int
            ret = int(idx[0])
        else:
            is_monotonic = np.all(np.diff(idx) == 1)
            if is_monotonic: # slice
                ret = slice(idx[0], idx[sz - 1] + 1, None)
            else:            # masked-array
                mask = np.asarray([False] * len(self))
                mask[idx] = True
                ret = mask
        return ret

    def between(self, left, right, inclusive="both"):
        """
        filtering rows according to the specified bounds
        """
        if not self.is_series:
            raise AttributeError("'DataFrame' object has no attribute 'between'\n")
        col_name = self.columns[0]
        dfopt = self.__dict__[col_name].between(left, right, inclusive)
        return dfopt

    @check_association
    def sum(self, axis=None, skipna=None, level=None,
            numeric_only=None, min_count=0, **kwargs):
        """
        returns the sum of the values over the requested axis.
        """
        param = check_stat_error("sum", DTYPE.STRING in self.__types, \
                                 axis_ = axis, skipna_ = skipna, \
                                 level_ = level, \
                                 numeric_only_= numeric_only, \
                                 min_count_ = min_count)
        cols, types = self.__get_numeric_columns()
        dtypes = [self.get_dtype(c) for c in cols]
        res_type = TypeUtil.to_id_dtype(get_result_type(dtypes))

        ncol = len(cols)
        cols_arr = get_string_array_pointer(cols)
        type_arr = np.asarray(types, dtype=c_short)
        tptr = type_arr.ctypes.data_as(POINTER(c_short))
        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_sum(host, port, self.get(), \
                                 cols_arr, tptr, ncol, \
                                 param.axis_, res_type, \
                                 param.skipna_, param.min_count_, \
                                 self.has_index())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        # returns a series
        ret = DataFrame(is_series=True)
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret.num_row = dummy_df["nrow"]
        ret.index = FrovedisColumn(names[0], types[0]) #setting index
        ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        return ret

    @check_association
    def first_element(self, col_name, skipna=None):
        """
        Returns the first element/row from column/dataframe
        """
        param = check_stat_error("first", DTYPE.STRING in self.__types, \
                                 skipna_=skipna)
        ret_df = self[col_name]
        if param.skipna_:
            ret_df = ret_df.dropna()
        if len(ret_df):
            (host, port) = FrovedisServer.getServerInstance()
            dummy_df = rpclib.df_first(host, port, \
                                       ret_df.get(), \
                                       col_name.encode('ascii'))
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            ret = DataFrame()
            names = dummy_df["names"]
            types = dummy_df["types"]
            ret.num_row = dummy_df["nrow"]
            ret.index = FrovedisColumn(names[0], types[0])
            ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
            return ret.to_numpy()[0][0]
        return np.nan

    @check_association
    def last_element(self, col_name, skipna=None):
        """
        Returns the last element from column
        """
        param = check_stat_error("last", DTYPE.STRING in self.__types, \
                                 skipna_=skipna)
        ret_df = self[col_name]
        if param.skipna_:
            ret_df = ret_df.dropna()
        if len(ret_df):
            (host, port) = FrovedisServer.getServerInstance()
            dummy_df = rpclib.df_last(host, port, \
                                      ret_df.get(), \
                                      col_name.encode('ascii'))
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            ret = DataFrame()
            names = dummy_df["names"]
            types = dummy_df["types"]
            ret.num_row = dummy_df["nrow"]
            ret.index = FrovedisColumn(names[0], types[0])
            ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
            return ret.to_numpy()[0][0]
        return np.nan

    @check_association
    def min(self, axis=None, skipna=None, level=None,
            numeric_only=None, **kwargs):
        """
        returns the min of the values over the requested axis.
        """
        param = check_stat_error("min", DTYPE.STRING in self.__types, \
                                 axis_ = axis, skipna_ = skipna, \
                                 level_ = level, \
                                 numeric_only_= numeric_only)
        cols, types = self.__get_numeric_columns()
        dtypes = [self.get_dtype(c) for c in cols]
        res_type = TypeUtil.to_id_dtype(get_result_type(dtypes))

        ncol = len(cols)
        cols_arr = get_string_array_pointer(cols)
        type_arr = np.asarray(types, dtype=c_short)
        tptr = type_arr.ctypes.data_as(POINTER(c_short))
        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_min(host, port, self.get(), \
                                 cols_arr, tptr, ncol, \
                                 param.axis_, res_type, \
                                 param.skipna_, \
                                 self.has_index())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        # returns a series
        ret = DataFrame(is_series=True)
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret.num_row = dummy_df["nrow"]
        ret.index = FrovedisColumn(names[0], types[0]) #setting index
        ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        return ret


    @check_association
    def max(self, axis=None, skipna=None, level=None,
            numeric_only=None, **kwargs):
        """
        returns the max of the values over the requested axis.
        """
        param = check_stat_error("max", DTYPE.STRING in self.__types, \
                                 axis_ = axis, skipna_ = skipna, \
                                 level_ = level, \
                                 numeric_only_= numeric_only)
        cols, types = self.__get_numeric_columns()
        dtypes = [self.get_dtype(c) for c in cols]
        res_type = TypeUtil.to_id_dtype(get_result_type(dtypes))

        ncol = len(cols)
        cols_arr = get_string_array_pointer(cols)
        type_arr = np.asarray(types, dtype=c_short)
        tptr = type_arr.ctypes.data_as(POINTER(c_short))
        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_max(host, port, self.get(), \
                                 cols_arr, tptr, ncol, \
                                 param.axis_, res_type, \
                                 param.skipna_, \
                                 self.has_index())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        # returns a series
        ret = DataFrame(is_series=True)
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret.num_row = dummy_df["nrow"]
        ret.index = FrovedisColumn(names[0], types[0]) #setting index
        ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        return ret


    @check_association
    def mean(self, axis=None, skipna=None, level=None,
             numeric_only=None, **kwargs):
        """
        returns the mean of the values over the requested axis.
        """
        param = check_stat_error("mean", DTYPE.STRING in self.__types, \
                                 numeric_only_= numeric_only, \
                                 axis_=axis, skipna_=skipna, level_=level)
        cols, types = self.__get_numeric_columns()

        ncol = len(cols)
        cols_arr = get_string_array_pointer(cols)
        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_mean(host, port, self.get(), \
                                  cols_arr, ncol, \
                                  param.axis_, param.skipna_, \
                                  self.has_index())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        # returns a series
        ret = DataFrame(is_series=True)
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret.num_row = dummy_df["nrow"]
        ret.index = FrovedisColumn(names[0], types[0]) #setting index
        ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        return ret

    @check_association
    def var(self, axis=None, skipna=None, level=None, ddof=1,
            numeric_only=None, **kwargs):
        """
        returns the variance of the values over the requested axis.
        """
        param = check_stat_error("var", DTYPE.STRING in self.__types, \
                                 axis_=axis, skipna_=skipna, \
                                 numeric_only_= numeric_only, \
                                 level_=level, ddof_=ddof)
        cols, types = self.__get_numeric_columns()

        ncol = len(cols)
        cols_arr = get_string_array_pointer(cols)
        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_var(host, port, self.get(), \
                                 cols_arr, ncol, \
                                 param.axis_, param.skipna_, \
                                 param.ddof_, self.has_index())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        # returns a series
        ret = DataFrame(is_series=True)
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret.num_row = dummy_df["nrow"]
        ret.index = FrovedisColumn(names[0], types[0]) #setting index
        ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        return ret

    @check_association
    def mad(self, axis=None, skipna=None, level=None,
            numeric_only=None, **kwargs):
        """
        returns the mean absolute deviation of the values over the
        requested axis.
        """
        param = check_stat_error("mad", DTYPE.STRING in self.__types, \
                                 numeric_only_= numeric_only, \
                                 axis_=axis, skipna_=skipna, level_=level)
        cols, types = self.__get_numeric_columns()

        ncol = len(cols)
        cols_arr = get_string_array_pointer(cols)
        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_mad(host, port, self.get(), \
                                 cols_arr, ncol, \
                                 param.axis_, param.skipna_, \
                                 self.has_index())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        # returns a series
        ret = DataFrame(is_series=True)
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret.num_row = dummy_df["nrow"]
        ret.index = FrovedisColumn(names[0], types[0]) #setting index
        ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        return ret

    @check_association
    def std(self, axis=None, skipna=None, level=None, ddof=1,
            numeric_only=None, **kwargs):
        """
        returns the standard deviatioon of the values over the requested axis.
        """
        param = check_stat_error("std", DTYPE.STRING in self.__types, \
                                 numeric_only_= numeric_only, \
                                 axis_=axis, skipna_=skipna, \
                                 level_=level, ddof_=ddof)
        cols, types = self.__get_numeric_columns()

        ncol = len(cols)
        cols_arr = get_string_array_pointer(cols)
        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_std(host, port, self.get(), \
                                  cols_arr, ncol, \
                                  param.axis_, param.skipna_, \
                                  param.ddof_, self.has_index())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        # returns a series
        ret = DataFrame(is_series=True)
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret.num_row = dummy_df["nrow"]
        ret.index = FrovedisColumn(names[0], types[0]) #setting index
        ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        return ret

    @check_association
    def mode(self, axis=0, numeric_only=False, dropna=True):
        """
        Calculate the mode of elements along the specified axis
        """
        param = check_stat_error("mode", DTYPE.STRING in self.__types, \
                                 numeric_only_= numeric_only, \
                                 axis_=axis, skipna_=dropna)
        if param.numeric_only_:
            cols = self.__get_numeric_columns()[0]
            if param.axis_ == 1:
                is_string = False
        else:
            cols = self.columns
            if param.axis_ == 1:
                types = self.__get_column_types(cols)
                non_numeric_count = 0
                for i in range(len(types)):
                    non_numeric_count += types[i] == DTYPE.STRING
                is_string = non_numeric_count != 0

                if is_string and non_numeric_count != len(types):
                    raise ValueError("mode: Cannot calculate mode for both numeric"
                                     " and string columns, with axis=1!")
        ncols = len(cols)
        cols_arr = get_string_array_pointer(cols)
        (host, port) = FrovedisServer.getServerInstance()

        if param.axis_ == 0:
            dummy_df = rpclib.df_mode_cols(host, port, self.get(),
                                           cols_arr, ncols, param.skipna_)
        else:
            dummy_df = rpclib.df_mode_rows(host, port, self.get(),
                                           cols_arr, ncols, is_string, param.skipna_)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        ret = DataFrame()
        names = dummy_df["names"]
        types = dummy_df["types"]

        bool_cols = set([self.__cols[i] for i in range(len(self.__types)) \
                        if self.__types[i] == DTYPE.BOOL])
        if len(bool_cols) > 0:
            for i in range(len(names)):
                if names[i] in bool_cols:
                    types[i] = DTYPE.BOOL

        ret.num_row = dummy_df["nrow"]
        ret.index = FrovedisColumn(names[0], types[0]) #setting index
        ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        return ret

    @check_association
    def sem(self, axis=None, skipna=None, level=None, ddof=1,
            numeric_only=None, **kwargs):
        """
        returns the standard error of mean of the values over the requested axis.
        """
        param = check_stat_error("sem", DTYPE.STRING in self.__types, \
                                 numeric_only_= numeric_only, \
                                 axis_=axis, skipna_=skipna, \
                                 level_=level, ddof_=ddof)
        cols, types = self.__get_numeric_columns()

        ncol = len(cols)
        cols_arr = get_string_array_pointer(cols)
        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_sem(host, port, self.get(), \
                                 cols_arr, ncol, \
                                 param.axis_, param.skipna_, \
                                 param.ddof_, self.has_index())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        # returns a series
        ret = DataFrame(is_series=True)
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret.num_row = dummy_df["nrow"]
        ret.index = FrovedisColumn(names[0], types[0]) #setting index
        ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        return ret

    @check_association
    def median(self, axis=None, skipna=None, level=None,
               numeric_only=None, **kwargs):
        """
        returns the median of the values over the requested axis.
        """
        param = check_stat_error("median", DTYPE.STRING in self.__types, \
                                 numeric_only_= numeric_only, \
                                 axis_=axis, skipna_=skipna, level_=level)
        cols, types = self.__get_numeric_columns()

        ncol = len(cols)
        cols_arr = get_string_array_pointer(cols)
        type_arr = np.asarray(types, dtype=c_short)
        tptr = type_arr.ctypes.data_as(POINTER(c_short))
        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_median(host, port, self.get(), \
                                    cols_arr, tptr, ncol, \
                                    param.axis_, param.skipna_, \
                                    self.has_index())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        # returns a series
        ret = DataFrame(is_series=True)
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret.num_row = dummy_df["nrow"]
        ret.index = FrovedisColumn(names[0], types[0]) #setting index
        ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        return ret

    @check_association
    def cov(self, min_periods=None, ddof=1.0, low_memory=True, other=None):
        """
        returns the covariance matrix for the given dataframe.
        """
        if (other != None):
            other = DataFrame.asDF(other) # checks error internally
            if not (self.is_series and other.is_series):
                raise TypeError("cov(other): input is expected to be series.")
            s1 = self.__dict__[self.columns[0]]   # FrovedisColumn
            s2 = other.__dict__[other.columns[0]] # FrovedisColumn
            return s1.cov(s2, min_periods, ddof)

        param = check_stat_error("cov", DTYPE.STRING in self.__types, \
                                 min_periods_=min_periods, \
                                 ddof_=ddof, low_memory_=low_memory)
        cols, types = self.__get_numeric_columns()

        ncol = len(cols)
        cols_arr = get_string_array_pointer(cols)
        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_covariance(host, port, self.get(), \
                                    cols_arr, ncol, \
                                    param.min_periods_, param.ddof_, \
                                    param.low_memory_, \
                                    self.has_index())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        # returns a series
        ret = DataFrame(is_series=True)
        names = dummy_df["names"]
        types = dummy_df["types"]
        ret.num_row = dummy_df["nrow"]
        ret.index = FrovedisColumn(names[0], types[0]) #setting index
        ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        return ret

    def __setattr__(self, key, value):
        """ sets the specified attribute """
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

    @check_association
    def __str__(self):
        """
        returns string representation of the dataframe
        """
        (host, port) = FrovedisServer.getServerInstance()
        df_str = rpclib.df_to_string(host, port, self.__fdata, self.has_index())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return df_str

    def __repr__(self):
        """
        returns string representation of the dataframe
        """
        return self.__str__()

    def set_multi_level_column(self, multi):
        """
        sets column name to be MultiIndex, used in to_pandas()
        """
        if self.__multi_index is None:
            tsize = len(self.__cols)
        else:
            # multi-index is included in self.__cols
            tsize = len(self.__cols) - len(self.__multi_index)

        if (multi.size != tsize):
            raise ValueError("set_multi_level_column: size mismatch")
        if (not isinstance(multi, (pd.Index, pd.MultiIndex))):
            raise TypeError(\
            "set_multi_level_column: input is not an instance of Index")
        self.__multi_cols = multi
        return self

    def set_multi_index_targets(self, cols):
        """
        sets target columns to be set as MultiIndex index in to_pandas()
        """
        self.__multi_index = cols
        return self

FrovedisDataframe = DataFrame
