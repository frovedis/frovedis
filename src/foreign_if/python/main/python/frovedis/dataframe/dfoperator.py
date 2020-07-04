"""
dfoperator.py
"""

#!/usr/bin/env python

from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer

class dfoperator(object):
    """
    dfoperator
    """
    def __init__(self, proxy):
        self.__proxy = proxy

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
            return dfoperator(proxy)

    def __or__(self, opt):
        """logical OR operator"""
        if self.__proxy is not None:
            (host, port) = FrovedisServer.getServerInstance()
            proxy = rpclib.get_dfORoperator(host, port, self.get(), opt.get())
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy)

    def __invert__(self):
        """Unary NOT operator"""
        if self.__proxy is not None:
            (host, port) = FrovedisServer.getServerInstance()
            proxy = rpclib.get_dfNOToperator(host, port, self.get())
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return dfoperator(proxy)

    def get(self):
        """get object"""
        return self.__proxy

