"""
frovedis_column.py
"""
#!/usr/bin/env python

import numpy as np
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from ..matrix.dtype import TypeUtil, DTYPE, str_encode
from .dfoperator import dfoperator
from .dfutil import check_stat_error, check_none_or_int, \
                    STR, get_str_methods_right_param
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
        """
        returns a FrovedisStringMethods object, for string related operations 
        """
        if self.dtype != DTYPE.STRING:
            raise AttributeError("Can only use .str accessor with string values")

        ret = FrovedisStringMethods(self.name, self.dtype)
        ret.df = self.df
        return ret

    @property
    def dt(self):
        """
        returns a FrovedisDatetimeProperties object
        for datetime related operations
        """
        if self.__dtype != DTYPE.DATETIME:
            raise AttributeError("Can only use .dt accessor with datetimelike values")

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
                   .to_numpy()[0]

    def min(self, skipna=None, numeric_only=None):
        """
        Min for series
        e.g. df.col.min()
        """
        return self.get_frovedis_series() \
                   .min(skipna=skipna, \
                        numeric_only=numeric_only) \
                   .to_numpy()[0]

    def max(self, skipna=None, numeric_only=None):
        """
        Max for series
        e.g. df.col.max()
        """
        return self.get_frovedis_series() \
                   .max(skipna=skipna, \
                        numeric_only=numeric_only) \
                   .to_numpy()[0]

    def mean(self, skipna=None, numeric_only=None):
        """
        Mean for series
        e.g. df.col.mean()
        """
        return self.get_frovedis_series() \
                   .mean(skipna=skipna, \
                         numeric_only=numeric_only) \
                   .to_numpy()[0]

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
                   .to_numpy()[0]

    def var(self, skipna=None, ddof=1, numeric_only=None):
        """
        Var for series
        e.g. df.col.var()
        """
        return self.get_frovedis_series() \
                   .var(skipna=skipna, \
                        ddof=ddof, \
                        numeric_only=numeric_only) \
                   .to_numpy()[0]

    def sem(self, skipna=None, ddof=1, numeric_only=None):
        """
        Sem for series
        e.g. df.col.sem()
        """
        return self.get_frovedis_series() \
                   .sem(skipna=skipna, \
                        ddof=ddof, \
                        numeric_only=numeric_only) \
                   .to_numpy()[0]

    def std(self, skipna=None, ddof=1, numeric_only=None):
        """
        Std for series
        e.g. df.col.std()
        """
        return self.get_frovedis_series() \
                   .std(skipna=skipna, \
                        ddof=ddof, \
                        numeric_only=numeric_only) \
                   .to_numpy()[0]

    def mad(self, skipna=None, numeric_only=None):
        """
        Mad for series
        e.g. df.col.mad()
        """
        return self.get_frovedis_series() \
                   .mad(skipna=skipna, \
                        numeric_only=numeric_only) \
                   .to_numpy()[0]

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

    def add(self, other, axis=1, fill_value=None):
        """
        returns resultant dataframe(series) after performing self + other
        """
        return self.get_frovedis_series().add(other, axis=axis, fill_value=fill_value)

    def radd(self, other, axis=1, fill_value=None):
        """
        returns resultant dataframe(series) after performing other + self
        """
        return self.get_frovedis_series().radd(other, axis=axis, fill_value=fill_value)

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

    def sub(self, other, axis=1, fill_value=None):
        """
        returns resultant dataframe(series) after performing self - other
        """
        return self.get_frovedis_series().sub(other, axis=axis, fill_value=fill_value)

    def rsub(self, other, axis=1, fill_value=None):
        """
        returns resultant dataframe(series) after performing other - self
        """
        return self.get_frovedis_series().rsub(other, axis=axis, fill_value=fill_value)

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

    def mul(self, other, axis=1, fill_value=None):
        """
        returns resultant dataframe(series) after performing self * other
        """
        return self.get_frovedis_series().mul(other, axis=axis, fill_value=fill_value)

    def rmul(self, other, axis=1, fill_value=None):
        """
        returns resultant dataframe(series) after performing other * self
        """
        return self.get_frovedis_series().rmul(other, axis=axis, fill_value=fill_value)

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

    def div(self, other, axis=1, fill_value=None):
        """
        returns resultant dataframe(series) after performing self / other
        """
        return self.get_frovedis_series().div(other, axis=axis, fill_value=fill_value)

    def rdiv(self, other, axis=1, fill_value=None):
        """
        returns resultant dataframe(series) after performing other / self
        """
        return self.get_frovedis_series().rdiv(other, axis=axis, fill_value=fill_value)

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

    def truediv(self, other, axis=1, fill_value=None):
        """
        returns resultant dataframe(series) after performing self / other
        """
        return self.get_frovedis_series().truediv(other, axis=axis, fill_value=fill_value)

    def rtruediv(self, other, axis=1, fill_value=None):
        """
        returns resultant dataframe(series) after performing other / self
        """
        return self.get_frovedis_series().rtruediv(other, axis=axis, fill_value=fill_value)

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

    def floordiv(self, other, axis=1, fill_value=None):
        """
        returns resultant dataframe(series) after performing self // other
        """
        return self.get_frovedis_series().floordiv(other, axis=axis, fill_value=fill_value)

    def rfloordiv(self, other, axis=1, fill_value=None):
        """
        returns resultant dataframe(series) after performing other // self
        """
        return self.get_frovedis_series().rfloordiv(other, axis=axis, fill_value=fill_value)

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

    def mod(self, other, axis=1, fill_value=None):
        """
        returns resultant dataframe(series) after performing self % other
        """
        return self.get_frovedis_series().mod(other, axis=axis, fill_value=fill_value)

    def rmod(self, other, axis=1, fill_value=None):
        """
        returns resultant dataframe(series) after performing other % self
        """
        return self.get_frovedis_series().rmod(other, axis=axis, fill_value=fill_value)

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

    def pow(self, other, axis=1, fill_value=None):
        """
        returns resultant dataframe(series) after performing self ** other
        """
        return self.get_frovedis_series().pow(other, axis=axis, fill_value=fill_value)

    def rpow(self, other, axis=1, fill_value=None):
        """
        returns resultant dataframe(series) after performing other ** self
        """
        return self.get_frovedis_series().rpow(other, axis=axis, fill_value=fill_value)

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

    def __str_methods_impl(self, op_id, **kwargs):
        (host, port) = FrovedisServer.getServerInstance()
        cname = self.__colName
        param = get_str_methods_right_param(op_id, **kwargs)
      
        dummy_df = rpclib.df_string_methods(host, port, self.df.get(),
                                            str_encode(cname),
                                            str_encode(param),
                                            op_id,
                                            self.df.has_index())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self.df._wrap_result(dummy_df, as_series=True) 

    def upper(self):
        """ Convert strings in the Series/Index to uppercase. """
        return self.__str_methods_impl(STR.UPPER)

    def lower(self):
        """ Convert strings in the Series/Index to lowercase. """
        return self.__str_methods_impl(STR.LOWER)

    def title(self):
        """ Convert strings in the Series/Index to titlecase. """
        return self.__str_methods_impl(STR.INITCAP)

    def capitalize(self):
        """ Convert strings in the Series/Index to be capitalized. """
        return self.__str_methods_impl(STR.CAPITALIZE)

    def len(self):
        """ Compute length of each string in the Series/Index. """
        cname = self.__colName
        return self.__str_methods_impl(STR.LEN) \
                   .astype({cname: "int64"}) # since pandas returns length as int64

    def reverse(self):
        """ Convert strings in the Series/Index to reversed order. """
        return self.__str_methods_impl(STR.REV)

    def ascii(self):
        """ 
        Compute ASCII value of the initial character of 
        each string in the Series/Index. 
        """
        return self.__str_methods_impl(STR.ASCII)

    def strip(self, to_strip=None):
        """ 
        Strip whitespace (including newlines) from each string in the
        Series/Index from left and right sides.
        """
        target = " \n" if to_strip is None else to_strip
        return self.__str_methods_impl(STR.TRIMWS, to_strip=target)
       
    def lstrip(self, to_strip=None):
        """ 
        Strip whitespace (including newlines) from each string in the
        Series/Index from left side.
        """
        target = " \n" if to_strip is None else to_strip
        return self.__str_methods_impl(STR.LTRIMWS, to_strip=target)
       
    def rstrip(self, to_strip=None):
        """ 
        Strip whitespace (including newlines) from each string in the
        Series/Index from right side.
        """
        target = " \n" if to_strip is None else to_strip
        return self.__str_methods_impl(STR.RTRIMWS, to_strip=target)

    def slice(self, start=None, stop=None, step=None):
        """ Slice substrings from each element in the Series/Index. """
        if not check_none_or_int(start) or \
           not check_none_or_int(stop) or \
           not check_none_or_int(step):
            raise TypeError("slice: expected start, stop, step to be " + \
                            "either None or of integer type")
        cname = self.__colName
        if start is None:
            start = 0
        elif start > 0:
            start = start + 1 # frovedis expects position
        stop = self.len()[cname].max() if stop is None else stop
        step = 1 if step is None else step
        (host, port) = FrovedisServer.getServerInstance()
        dummy_df = rpclib.df_slice(host, port, self.df.get(),
                                   str_encode(cname),
                                   start, stop, step,
                                   self.df.has_index())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self.df._wrap_result(dummy_df, as_series=True)

    def __getitem__(self, obj):
        if not isinstance(obj, slice):
            raise TypeError("str[]: expected a slice object")
        return self.slice(obj.start, obj.stop, obj.step)

    def pad(self, width, side='left', fillchar=' '):
        """
        Pad strings in the Series/Index with an additional character
        to specified side.
        """
        if not isinstance(side, str) or \
           not isinstance(fillchar, str):
            raise TypeError("side and fillchar are expected to be string")

        if len(fillchar) > 1:
            raise TypeError("fillchar must be a character, not str")

        (host, port) = FrovedisServer.getServerInstance()
        cname = self.__colName
        dummy_df = rpclib.df_pad(host, port, self.df.get(),
                                 str_encode(cname),
                                 str_encode(side),
                                 str_encode(fillchar),
                                 width, self.df.has_index())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self.df._wrap_result(dummy_df, as_series=True)

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

    @property
    def hour(self):
        """
        hour
        """
        (host, port) = FrovedisServer.getServerInstance()
        left_col = self.__colName
        right_col = ""
        as_name = left_col

        dummy_df = rpclib.df_datetime_operation(host, port, self.df.get(),
                                                str_encode(left_col),
                                                str_encode(right_col),
                                                str_encode(as_name),
                                                OPT.GETHOUR,
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
    def minute(self):
        """
        minute
        """
        (host, port) = FrovedisServer.getServerInstance()
        left_col = self.__colName
        right_col = ""
        as_name = left_col

        dummy_df = rpclib.df_datetime_operation(host, port, self.df.get(),
                                                str_encode(left_col),
                                                str_encode(right_col),
                                                str_encode(as_name),
                                                OPT.GETMINUTE,
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
    def second(self):
        """
        second
        """
        (host, port) = FrovedisServer.getServerInstance()
        left_col = self.__colName
        right_col = ""
        as_name = left_col

        dummy_df = rpclib.df_datetime_operation(host, port, self.df.get(),
                                                str_encode(left_col),
                                                str_encode(right_col),
                                                str_encode(as_name),
                                                OPT.GETSECOND,
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
    def nanosecond(self):
        """
        nanosecond
        """
        (host, port) = FrovedisServer.getServerInstance()
        left_col = self.__colName
        right_col = ""
        as_name = left_col

        dummy_df = rpclib.df_datetime_operation(host, port, self.df.get(),
                                                str_encode(left_col),
                                                str_encode(right_col),
                                                str_encode(as_name),
                                                OPT.GETNANOSECOND,
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
    def year(self):
        """
        year
        """
        (host, port) = FrovedisServer.getServerInstance()
        left_col = self.__colName
        right_col = ""
        as_name = left_col

        dummy_df = rpclib.df_datetime_operation(host, port, self.df.get(),
                                                str_encode(left_col),
                                                str_encode(right_col),
                                                str_encode(as_name),
                                                OPT.GETYEAR,
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
    def month(self):
        """
        month
        """
        (host, port) = FrovedisServer.getServerInstance()
        left_col = self.__colName
        right_col = ""
        as_name = left_col

        dummy_df = rpclib.df_datetime_operation(host, port, self.df.get(),
                                                str_encode(left_col),
                                                str_encode(right_col),
                                                str_encode(as_name),
                                                OPT.GETMONTH,
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
    def quarter(self):
        """
        quarter
        """
        (host, port) = FrovedisServer.getServerInstance()
        left_col = self.__colName
        right_col = ""
        as_name = left_col

        dummy_df = rpclib.df_datetime_operation(host, port, self.df.get(),
                                                str_encode(left_col),
                                                str_encode(right_col),
                                                str_encode(as_name),
                                                OPT.GETQUARTER,
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
    def dayofyear(self):
        """
        dayofyear
        """
        (host, port) = FrovedisServer.getServerInstance()
        left_col = self.__colName
        right_col = ""
        as_name = left_col

        dummy_df = rpclib.df_datetime_operation(host, port, self.df.get(),
                                                str_encode(left_col),
                                                str_encode(right_col),
                                                str_encode(as_name),
                                                OPT.GETDAYOFYEAR,
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
    def weekofyear(self):
        """
        weekofyear
        """
        (host, port) = FrovedisServer.getServerInstance()
        left_col = self.__colName
        right_col = ""
        as_name = left_col

        dummy_df = rpclib.df_datetime_operation(host, port, self.df.get(),
                                                str_encode(left_col),
                                                str_encode(right_col),
                                                str_encode(as_name),
                                                OPT.GETWEEKOFYEAR,
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
