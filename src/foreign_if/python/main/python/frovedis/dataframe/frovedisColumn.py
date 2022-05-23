"""
frovedis_column.py
"""
#!/usr/bin/env python

import numpy as np
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from ..matrix.dtype import TypeUtil, DTYPE, str_encode
from .dfoperator import dfoperator
from .dfutil import check_stat_error
from .optype import *

class FrovedisColumn(object):
    """
    FrovedisColumn
    """
    def __init__(self, colName, dtype):
        """
        __init__
        """
        self.__colName = colName
        self.__dtype = dtype
        self.df = None

    def get_frovedis_series(self):
        """ helper method to create frovedis dataframe with is_series = True """
        if self.df is None:
            raise ValueError("column->series: column is not associated with a DataFrame!")
        ret = self.df[[self.name]]
        ret.is_series = True
        return ret

    def copy(self):
        ret = FrovedisColumn(self.name, self.dtype)
        ret.df = self.df
        return ret

    def __str__(self):
        """ to-string """
        if self.df is not None:
            return str(self.df[[self.name]])
        else:
            return "name: " + self.name + "; dtype: " + \
               str(TypeUtil.to_numpy_dtype(self.dtype))

    def __repr__(self):
        """ REPR """
        return str(self)

    def show(self):
        """ to print """
        if self.df is None:
            raise ValueError("show() is called on a series with unassociated DataFrame!")
        print(str(self))
        
    def to_pandas(self):
        """ to pandas series """
        return self.get_frovedis_series().to_pandas()

    def to_numpy(self, dtype=None, copy=False, na_value=None):
        """
        frovedis column to numpy array conversion
        """
        ret = self.get_frovedis_series().to_numpy(dtype=dtype, \
                                         copy=copy, na_value=na_value)
        return np.ravel(ret)

    @property
    def values(self):
        """
        frovedis column to numpy array conversion
        """
        return self.to_numpy()

    @values.setter
    def values(self, val):
        """values setter"""
        raise AttributeError(\
            "attribute 'values' of FrovedisColumn object is not writable")

    @property
    def name(self):
        """
        name
        """
        return self.__colName

    @name.setter
    def name(self, value):
        """
        name
        """
        raise AttributeError("attribute 'name' of FrovedisColumn object " +
                             "is not writable!")

    @property
    def colName(self):
        """
        colName
        """
        return self.__colName

    @colName.setter
    def colName(self, value):
        """
        colName
        """
        raise AttributeError("attribute 'colName' of FrovedisColumn object " +
                             "is not writable!")

    @property
    def dtype(self):
        """
        dtype
        """
        return self.__dtype

    @dtype.setter
    def dtype(self, value):
        """
        dtype
        """
        raise AttributeError("attribute 'dtype' of FrovedisColumn object " +
                             "is not writable!")

    def __lt__(self, other):
        """
        dtype
        """
        (host, port) = FrovedisServer.getServerInstance()
        if isinstance(other, FrovedisColumn):
            #print ('Filtering dataframe where', self.colName, '<', other.colName)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   str_encode(self.colName),
                                                   str_encode(other.colName),
                                                   self.dtype, OPT.LT, False)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df)
        else:
            #print ('Filtering dataframe where', self.colName, '<', other)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   str_encode(self.colName),
                                                   str_encode(str(other)),
                                                   self.dtype, OPT.LT, True)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df)

    def __gt__(self, other):
        """
        dtype
        """
        (host, port) = FrovedisServer.getServerInstance()
        if isinstance(other, FrovedisColumn):
            #print ('Filtering dataframe where', self.colName, '>', other.colName)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   str_encode(self.colName),
                                                   str_encode(other.colName),
                                                   self.dtype, OPT.GT, False)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df)
        else:
            #print ('Filtering dataframe where', self.colName, '>', other)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   str_encode(self.colName),
                                                   str_encode(str(other)),
                                                   self.dtype, OPT.GT, True)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df)

    def __eq__(self, other):
        """
        dtype
        """
        (host, port) = FrovedisServer.getServerInstance()
        if isinstance(other, FrovedisColumn):
            #print ('Filtering dataframe where', self.colName, '==', other.colName)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   str_encode(self.colName),
                                                   str_encode(other.colName),
                                                   self.dtype, OPT.EQ, False)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df)
        else:
            #print ('Filtering dataframe where', self.colName, '==', other)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   str_encode(self.colName),
                                                   str_encode(str(other)),
                                                   self.dtype, OPT.EQ, True)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df)

    def __ne__(self, other):
        """
        dtype
        """
        (host, port) = FrovedisServer.getServerInstance()
        if isinstance(other, FrovedisColumn):
            #print ('Filtering dataframe where', self.colName, '!=', other.colName)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   str_encode(self.colName),
                                                   str_encode(other.colName),
                                                   self.dtype, OPT.NE, False)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df)
        else:
            #print ('Filtering dataframe where', self.colName, '!=', other)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   str_encode(self.colName),
                                                   str_encode(str(other)),
                                                   self.dtype, OPT.NE, True)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df)

    def __le__(self, other):
        """
        dtype
        """
        (host, port) = FrovedisServer.getServerInstance()
        if isinstance(other, FrovedisColumn):
            #print ('Filtering dataframe where', self.colName, '<=', other.colName)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   str_encode(self.colName),
                                                   str_encode(other.colName),
                                                   self.dtype, OPT.LE, False)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df)
        else:
            #print ('Filtering dataframe where', self.colName, '<=', other)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   str_encode(self.colName),
                                                   str_encode(str(other)),
                                                   self.dtype, OPT.LE, True)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df)

    def __ge__(self, other):
        """
        dtype
        """
        (host, port) = FrovedisServer.getServerInstance()
        if isinstance(other, FrovedisColumn):
            #print ('Filtering dataframe where', self.colName, '>=', other.colName)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   str_encode(self.colName),
                                                   str_encode(other.colName),
                                                   self.dtype, OPT.GE, False)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df)
        else:
            #print ('Filtering dataframe where', self.colName, '>=', other)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   str_encode(self.colName),
                                                   str_encode(str(other)),
                                                   self.dtype, OPT.GE, True)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df)

    def between(self, left, right, inclusive="both"):
        """
        filtering rows according to the specified bounds
        """
        dfopt = None
        if inclusive == "both":
            dfopt = (self >= left) & (self <= right)
        elif inclusive == "left":
            dfopt = (self >= left) & (self < right)
        elif inclusive == "right":
            dfopt = (self > left) & (self <= right)
        elif inclusive == "neither":
            dfopt = (self > left) & (self < right)
        else:
            raise ValueError(
                "Inclusive has to be either string of 'both',"
                "'left', 'right', or 'neither'."
            )
        return dfopt

    @property
    def str(self):
        """returns a FrovedisStringMethods object, for: \
        startswith/endwith/contains operations
        """
        ret = FrovedisStringMethods(self.name, self.dtype)
        ret.df = self.df
        return ret

    @property
    def dt(self):
        """returns a FrovedisDatetimeProperties object, for: \
        datetime related operations
        """
        ret = FrovedisDatetimeProperties(self.name, self.dtype)
        ret.df = self.df
        return ret

    def cov(self, other, min_periods=None, ddof=1):
        """
        Covariance for series
        call example: df.col1.cov(df.col2)
        """
        if not isinstance(other, FrovedisColumn): #TODO: support pandas series
            raise TypeError("other: input is expected to be a FrovedisColumn.")

        if len(self.df) != len(other.df):
            raise ValueError("cov: input series with unequal no. of elements.")

        c1 = self.name
        c2 = other.name
        c1_dt = self.dtype
        c2_dt = other.dtype
        if c1_dt == DTYPE.STRING or c2_dt == DTYPE.STRING:
            raise TypeError("cov: is not supported for 'string' type")
        param = check_stat_error("cov", False, \
                                 min_periods_=min_periods, ddof_=ddof)

        (host, port) = FrovedisServer.getServerInstance()
        ret = rpclib.series_covariance(host, port, \
                                       self.df.get(), str_encode(c1), \
                                       other.df.get(), str_encode(c2), \
                                       param.min_periods_, param.ddof_)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        # returns a double
        return np.nan if (ret == np.finfo(np.float64).max) else ret

    def sum(self, skipna=None, numeric_only=None, min_count=0):
        """
        Sum for series
        e.g. df.col.sum()
        """
        return self.get_frovedis_series() \
                   .sum(skipna=skipna, \
                        numeric_only=numeric_only, \
                        min_count=min_count) \
                   .to_numpy()[0][0]

    def min(self, skipna=None, numeric_only=None):
        """
        Min for series
        e.g. df.col.min()
        """
        return self.get_frovedis_series() \
                   .min(skipna=skipna, \
                        numeric_only=numeric_only) \
                   .to_numpy()[0][0]

    def max(self, skipna=None, numeric_only=None):
        """
        Max for series
        e.g. df.col.max()
        """
        return self.get_frovedis_series() \
                   .max(skipna=skipna, \
                        numeric_only=numeric_only) \
                   .to_numpy()[0][0]

    def mean(self, skipna=None, numeric_only=None):
        """
        Mean for series
        e.g. df.col.mean()
        """
        return self.get_frovedis_series() \
                   .mean(skipna=skipna, \
                         numeric_only=numeric_only) \
                   .to_numpy()[0][0]

    def mode(self, numeric_only=None, dropna=True):
        """
        Mode for series
        e.g. df.col.mode()
        """
        return self.get_frovedis_series() \
                   .mode(numeric_only=numeric_only, \
                         dropna=dropna)

    def median(self, skipna=None, numeric_only=None):
        """
        Median for series
        e.g. df.col.median()
        """
        return self.get_frovedis_series() \
                   .median(skipna=skipna, \
                           numeric_only=numeric_only) \
                   .to_numpy()[0][0]

    def var(self, skipna=None, ddof=1, numeric_only=None):
        """
        Var for series
        e.g. df.col.var()
        """
        return self.get_frovedis_series() \
                   .var(skipna=skipna, \
                        ddof=ddof, \
                        numeric_only=numeric_only) \
                   .to_numpy()[0][0]

    def sem(self, skipna=None, ddof=1, numeric_only=None):
        """
        Sem for series
        e.g. df.col.sem()
        """
        return self.get_frovedis_series() \
                   .sem(skipna=skipna, \
                        ddof=ddof, \
                        numeric_only=numeric_only) \
                   .to_numpy()[0][0]

    def std(self, skipna=None, ddof=1, numeric_only=None):
        """
        Std for series
        e.g. df.col.std()
        """
        return self.get_frovedis_series() \
                   .std(skipna=skipna, \
                        ddof=ddof, \
                        numeric_only=numeric_only) \
                   .to_numpy()[0][0]

    def mad(self, skipna=None, numeric_only=None):
        """
        Mad for series
        e.g. df.col.mad()
        """
        return self.get_frovedis_series() \
                   .mad(skipna=skipna, \
                        numeric_only=numeric_only) \
                   .to_numpy()[0][0]

    def first_element(self, skipna=None):
        """
        Returns the first element for column
        e.g. df.col.first_element()
        """
        return self.get_frovedis_series() \
                   .first_element(self.name, skipna=skipna)

    def last_element(self, skipna=None):
        """
        Returns the last element for column
        e.g. df.col.last_element()
        """
        return self.get_frovedis_series() \
                   .last_element(self.name, skipna=skipna)

    def __binop_impl(self, other, op):
        """
        returns resultant dataframe(series) after performing self (op) other
        """

        left = self.get_frovedis_series()

        if isinstance(other, FrovedisColumn):
            right_series = True
            right = other.get_frovedis_series()
        else:
            right_series = False
            right = other

        if (op == "add"):
            ret = left + right
        elif (op == "radd"):
            ret = right + left
        elif (op == "sub"):
            ret = left - right
        elif (op == "rsub"):
            ret = right - left
        elif (op == "mul"):
            ret = left * right
        elif (op == "rmul"):
            ret = right * left
        elif (op == "truediv"):
            ret = left / right
        elif (op == "rtruediv"):
            ret = right / left
        elif (op == "floordiv"):
            ret = left // right
        elif (op == "rfloordiv"):
            ret = right // left
        elif (op == "mod"):
            ret = left % right
        elif (op == "rmod"):
            ret = right % left
        elif (op == "pow"):
            ret = left ** right
        elif (op == "rpow"):
            ret = right ** left
        else:
            right_type = "series" if right_series else type(other).__name__
            raise ValueError("Unsupported operation: 'series' (" + op + \
                             ") '" + right_type +"'")
        return ret

    def __len__(self):
        if self.df is None:
            raise ValueError("len: column is not associated with a DataFrame!")
        return len(self.df)

    def __add__(self, other):
        """
        returns resultant dataframe(series) after performing self + other
        """
        return self.__binop_impl(other, "add")

    def __radd__(self, other):
        """
        returns resultant dataframe(series) after performing other + self
        """
        return self.__binop_impl(other, "radd")

    def __sub__(self, other):
        """
        returns resultant dataframe(series) after performing self - other
        """
        return self.__binop_impl(other, "sub")

    def __rsub__(self, other):
        """
        returns resultant dataframe(series) after performing other - self
        """
        return self.__binop_impl(other, "rsub")

    def __mul__(self, other):
        """
        returns resultant dataframe(series) after performing self * other
        """
        return self.__binop_impl(other, "mul")

    def __rmul__(self, other):
        """
        returns resultant dataframe(series) after performing other * self
        """
        return self.__binop_impl(other, "rmul")

    def __div__(self, other): # python 2.x
        """
        returns resultant dataframe(series) after performing self / other
        """
        return self.__binop_impl(other, "truediv")

    def __rdiv__(self, other): # python 2.x
        """
        returns resultant dataframe(series) after performing other / self
        """
        return self.__binop_impl(other, "rtruediv")

    def __truediv__(self, other): # python 3.x
        """
        returns resultant dataframe(series) after performing self / other
        """
        return self.__binop_impl(other, "truediv")

    def __rtruediv__(self, other): # python 3.x
        """
        returns resultant dataframe(series) after performing other / self
        """
        return self.__binop_impl(other, "rtruediv")

    def __floordiv__(self, other):
        """
        returns resultant dataframe(series) after performing self // other
        """
        return self.__binop_impl(other, "floordiv")

    def __rfloordiv__(self, other):
        """
        returns resultant dataframe(series) after performing other // self
        """
        return self.__binop_impl(other, "rfloordiv")

    def __mod__(self, other):
        """
        returns resultant dataframe(series) after performing self % other
        """
        return self.__binop_impl(other, "mod")

    def __rmod__(self, other):
        """
        returns resultant dataframe(series) after performing other % self
        """
        return self.__binop_impl(other, "rmod")

    def __pow__(self, other):
        """
        returns resultant dataframe(series) after performing self ** other
        """
        return self.__binop_impl(other, "pow")

    def __rpow__(self, other):
        """
        returns resultant dataframe(series) after performing other ** self
        """
        return self.__binop_impl(other, "rpow")

class FrovedisStringMethods(object):
    """
    FrovedisStringMethods
    """
    def __init__(self, colName, dtype):
        """
        __init__
        """
        self.__colName = colName
        self.__dtype = dtype
        self.df = None

    @property
    def colName(self):
        """
        colName
        """
        return self.__colName

    @colName.setter
    def colName(self, value):
        """
        colName
        """
        self.__colName = value

    @property
    def dtype(self):
        """
        dtype
        """
        return self.__dtype

    @dtype.setter
    def dtype(self, value):
        """
        dtype
        """
        self.__dtype = value

    def like(self, other):
        """
        like function
        """
        (host, port) = FrovedisServer.getServerInstance()
        if isinstance(other, FrovedisColumn):
            raise RuntimeError("like operator can be applied on pattern only!")
        else:
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   str_encode(self.colName),
                                                   str_encode(str(other)),
                                                   self.dtype, OPT.LIKE, True)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df)

    def startswith(self, pat, na=False):
        """
        startswith => ( "pattern% )
        """
        if na:
            raise ValueError("startswith: replacing missing values with True"+
                             " is currently unsupported!")
        return self.like(pat + "%")

    def contains(self, pat, case=True, flags=0, na=False, regex=False):
        """
        contains => ( "%pattern%" )
        """
        import warnings
        if na:
            raise ValueError("contains: replacing missing values with True is"+
                             " currently unsupported!")
        if not case:
            raise ValueError("contains: case insensitive matching is currently"+
                             " unsupported!")
        if regex:
            warnings.warn("contains: 'regex' is not supported! 'pat' will be"+
                          " treated as literal string!")
        return self.like("%" + pat + "%")

    def endswith(self, pat, na=False):
        """
        endswith => ( "%pattern" )
        """
        if na:
            raise ValueError("endswith: replacing missing values with True is"+
                             " currently unsupported!")
        return self.like("%" + pat)


class FrovedisDatetimeProperties(object):
    """
    FrovedisDatetimeProperties
    """
    def __init__(self, colName, dtype):
        """
        __init__
        """
        self.__colName = colName
        self.__dtype = dtype
        self.df = None
    
    @property
    def day(self):
        """
        day
        """
        (host, port) = FrovedisServer.getServerInstance()
        left_col = self.__colName
        right_col = ""
        as_name = left_col

        dummy_df = rpclib.df_datetime_operation(host, port, self.df.get(),
                                                str_encode(left_col),
                                                str_encode(right_col),
                                                str_encode(as_name),
                                                OPT.GETDAYOFMONTH,
                                                self.df.has_index())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        names = dummy_df["names"]
        types = dummy_df["types"]

        from .df import DataFrame
        ret = DataFrame(is_series=self.df.is_series)
        ret.num_row = dummy_df["nrow"]
        if self.df.has_index():
            ret.index = FrovedisColumn(names[0], types[0]) #setting index
            ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        else:
            ret.load_dummy(dummy_df["dfptr"], names, types)

        return ret

    @property
    def dayofweek(self):
        """
        dayofweek
        """
        (host, port) = FrovedisServer.getServerInstance()
        left_col = self.__colName
        right_col = ""
        as_name = left_col
        dummy_df = rpclib.df_datetime_operation(host, port, self.df.get(),
                                                str_encode(left_col),
                                                str_encode(right_col),
                                                str_encode(as_name),
                                                OPT.GETDAYOFWEEK,
                                                self.df.has_index())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        names = dummy_df["names"]
        types = dummy_df["types"]

        from .df import DataFrame
        ret = DataFrame(is_series=self.df.is_series)
        ret.num_row = dummy_df["nrow"]
        if self.df.has_index():
            ret.index = FrovedisColumn(names[0], types[0]) #setting index
            ret.load_dummy(dummy_df["dfptr"], names[1:], types[1:])
        else:
            ret.load_dummy(dummy_df["dfptr"], names, types)

        ret[ret.columns[0]] = (ret[ret.columns[0]] + 5) % 7
        return ret