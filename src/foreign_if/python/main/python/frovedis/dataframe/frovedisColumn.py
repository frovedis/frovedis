"""
frovedis_column.py
"""
#!/usr/bin/env python

from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from ..matrix.dtype import TypeUtil
from .dfoperator import dfoperator
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
            return dfoperator(proxy)
        else:
            #print ('Filtering dataframe where', self.colName, '<', other)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   self.colName.encode('ascii'),
                                                   str(other).encode('ascii'),
                                                   self.dtype, OPT.LT, True)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy)

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
            return dfoperator(proxy)
        else:
            #print ('Filtering dataframe where', self.colName, '>', other)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   self.colName.encode('ascii'),
                                                   str(other).encode('ascii'),
                                                   self.dtype, OPT.GT, True)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy)

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
            return dfoperator(proxy)
        else:
            #print ('Filtering dataframe where', self.colName, '==', other)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   self.colName.encode('ascii'),
                                                   str(other).encode('ascii'),
                                                   self.dtype, OPT.EQ, True)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy)

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
            return dfoperator(proxy)
        else:
            #print ('Filtering dataframe where', self.colName, '!=', other)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   self.colName.encode('ascii'),
                                                   str(other).encode('ascii'),
                                                   self.dtype, OPT.NE, True)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy)

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
            return dfoperator(proxy)
        else:
            #print ('Filtering dataframe where', self.colName, '<=', other)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   self.colName.encode('ascii'),
                                                   str(other).encode('ascii'),
                                                   self.dtype, OPT.LE, True)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy)

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
            return dfoperator(proxy)
        else:
            #print ('Filtering dataframe where', self.colName, '>=', other)
            proxy = rpclib.get_frovedis_dfoperator(host, port,
                                                   self.colName.encode('ascii'),
                                                   str(other).encode('ascii'),
                                                   self.dtype, OPT.GE, True)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy)

    @property
    def str(self):
        """returns a FrovedisStringMethods object, for: \
        startswith/endwith/contains operations
        """
        return FrovedisStringMethods(self.colName, self.dtype)


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
            return dfoperator(proxy)

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


