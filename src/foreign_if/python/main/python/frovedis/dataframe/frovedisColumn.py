"""
frovedis_column.py
"""
#!/usr/bin/env python

import numpy as np
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from ..matrix.dtype import TypeUtil
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

    def __str__(self):
        """ to-string """
        return "name: " + self.__colName + "; dtype: " + \
               str(TypeUtil.to_numpy_dtype(self.__dtype))

    def __repr__(self):
        """ REPR """
        return str(self)

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
        raise AttributeError("attribute 'name' of FrovedisColumn object"
                            " is not writable!")

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
        raise AttributeError("attribute 'colName' of FrovedisColumn object"
                            " is not writable!")

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
        raise AttributeError("attribute 'dtype' of FrovedisColumn object"
                            " is not writable!")

    def __lt__(self, other):
        """
        dtype
        """
        (host, port) = FrovedisServer.getServerInstance()
        if isinstance(other, FrovedisColumn):
            #print ('Filtering dataframe where', self.colName, '<', other.colName)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   self.colName.encode('ascii'),
                                                   other.colName.encode('ascii'),
                                                   self.dtype, OPT.LT, False)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df) 
        else:
            #print ('Filtering dataframe where', self.colName, '<', other)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   self.colName.encode('ascii'),
                                                   str(other).encode('ascii'),
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
                                                   self.colName.encode('ascii'),
                                                   other.colName.encode('ascii'),
                                                   self.dtype, OPT.GT, False)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df) 
        else:
            #print ('Filtering dataframe where', self.colName, '>', other)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   self.colName.encode('ascii'),
                                                   str(other).encode('ascii'),
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
                                                   self.colName.encode('ascii'),
                                                   other.colName.encode('ascii'),
                                                   self.dtype, OPT.EQ, False)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df) 
        else:
            #print ('Filtering dataframe where', self.colName, '==', other)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   self.colName.encode('ascii'),
                                                   str(other).encode('ascii'),
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
                                                   self.colName.encode('ascii'),
                                                   other.colName.encode('ascii'),
                                                   self.dtype, OPT.NE, False)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df) 
        else:
            #print ('Filtering dataframe where', self.colName, '!=', other)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   self.colName.encode('ascii'),
                                                   str(other).encode('ascii'),
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
                                                   self.colName.encode('ascii'),
                                                   other.colName.encode('ascii'),
                                                   self.dtype, OPT.LE, False)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df) 
        else:
            #print ('Filtering dataframe where', self.colName, '<=', other)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   self.colName.encode('ascii'),
                                                   str(other).encode('ascii'),
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
                                                   self.colName.encode('ascii'),
                                                   other.colName.encode('ascii'),
                                                   self.dtype, OPT.GE, False)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df) 
        else:
            #print ('Filtering dataframe where', self.colName, '>=', other)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   self.colName.encode('ascii'),
                                                   str(other).encode('ascii'),
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
        ret = FrovedisStringMethods(self.colName, self.dtype)
        ret.df = self.df
        return ret

    def cov(self, other, min_periods=None, ddof=1):
        """
        Covariance for series
        call example: df.col1.cov(df.col2)
        """
        if not isinstance(other, FrovedisColumn):
            raise TypeError("other: input is expected to be a FrovedisColumn.")
            
        if len(self.df) != len(other.df):
            raise ValueError("cov: input series with unequal no. of elements.")

        param = check_stat_error(min_periods_=min_periods, ddof_=ddof)
        c1 = self.df.columns[0]
        c2 = other.df.columns[0]
        (host, port) = FrovedisServer.getServerInstance()
        ret = rpclib.series_covariance(host, port, \
                                       self.df.get(), c1.encode('ascii'), \
                                       other.df.get(), c2.encode('ascii'), \
                                       param.min_periods_, param.ddof_)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        # returns a double
        return np.nan if (ret == np.finfo(np.float64).max) else ret

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
                                                   self.colName.encode('ascii'),
                                                   str(other).encode('ascii'),
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


