"""
dfoperator.py
"""

#!/usr/bin/env python

import numpy as np
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from ..matrix.dvector import FrovedisDvector

class dfoperator(object):
    """
    dfoperator
    """
    def __init__(self, proxy, df=None):
        self.__proxy = proxy
        self.df = df

  #def __del__(self):
  #  if FrovedisServer.isUP(): self.release()

    def release(self):
        """releasing stuff"""
        if self.__proxy is not None:
            (host, port) = FrovedisServer.getServerInstance()
            rpclib.release_dfoperator(host, port, self.get())
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            self.__proxy = None

    def __and__(self, opt):
        """logical AND operator"""
        if self.__proxy is not None:
            (host, port) = FrovedisServer.getServerInstance()
            proxy = rpclib.get_dfANDoperator(host, port, self.get(), opt.get())
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df)

    def __or__(self, opt):
        """logical OR operator"""
        if self.__proxy is not None:
            (host, port) = FrovedisServer.getServerInstance()
            proxy = rpclib.get_dfORoperator(host, port, self.get(), opt.get())
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df)

    def __invert__(self):
        """Unary NOT operator"""
        if self.__proxy is not None:
            (host, port) = FrovedisServer.getServerInstance()
            proxy = rpclib.get_dfNOToperator(host, port, self.get())
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy, self.df)


    def to_mask_array(self):
        """return boolean mask"""
        (host, port) = FrovedisServer.getServerInstance()
        dummy_dvec = rpclib.get_bool_mask(host, port, self.get(), self.df.get())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        res = FrovedisDvector(dummy_dvec).to_numpy_array()
        int_to_bool = {1: True, 0: False}
        res = np.array([ int_to_bool[e] for e in res ])
        return res

    def get(self):
        """get object"""
        return self.__proxy

