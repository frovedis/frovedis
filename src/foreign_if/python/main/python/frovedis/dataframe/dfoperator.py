"""
dfoperator.py
"""

#!/usr/bin/env python

import numpy as np
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer, set_association, \
                           check_association, do_if_active_association
from ..matrix.dvector import FrovedisIntDvector

class dfoperator(object):
    """
    dfoperator
    """
    @set_association
    def __init__(self, proxy, df=None):
        self.df = df
        self.__proxy = proxy
        self.__mask = None

    @check_association
    def __and__(self, opt):
        """logical AND operator"""
        (host, port) = FrovedisServer.getServerInstance()
        proxy = rpclib.get_dfANDoperator(host, port, self.get(), opt.get())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return dfoperator(proxy, self.df)

    @check_association
    def __or__(self, opt):
        """logical OR operator"""
        (host, port) = FrovedisServer.getServerInstance()
        proxy = rpclib.get_dfORoperator(host, port, self.get(), opt.get())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return dfoperator(proxy, self.df)

    @check_association
    def __invert__(self):
        """Unary NOT operator"""
        (host, port) = FrovedisServer.getServerInstance()
        proxy = rpclib.get_dfNOToperator(host, port, self.get())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return dfoperator(proxy, self.df)

    @property
    @check_association
    def mask(self):
        """return boolean mask as IntDvector"""
        # Note-1: Once mask dvector is consumed at server side while 
        # appending column etc., it is explicitly moved and deleted.
        # Hence, even if self.__mask is not None (computed once), the actual
        # dvector might be moved (and deleted), so its status checking is
        # required to ensure whether it needs to be re-generated
        if self.__mask is None or \
            (not self.__mask.is_fitted()): # Note-1
            ignore_nulls = True # pandas ignores nulls and treats as False, by default
            (host, port) = FrovedisServer.getServerInstance()
            dummy_dvec = rpclib.get_bool_mask(host, port, self.get(), 
                                              self.df.get(), ignore_nulls)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            self.__mask = FrovedisIntDvector(dummy_dvec)
        return self.__mask

    @mask.setter
    def mask(self, val):
        """mask setter"""
        raise AttributeError(\
            "attribute 'mask' of dfoperator object is not writable!")

    def to_mask_array(self):
        """return boolean mask as numpy array"""
        res = self.mask.to_numpy_array()
        return np.array([e == 1 for e in res])

    @check_association
    def get(self):
        """get object"""
        return self.__proxy

    @do_if_active_association
    def release(self):
        """releasing stuff"""
        if self.is_fitted():
            (host, port) = FrovedisServer.getServerInstance()
            rpclib.release_dfoperator(host, port, self.get())
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
        self.__proxy = None
        self.__mask = None
        self.df = None

    def __del__(self):
        self.release()

    def is_fitted(self):
        """ function to confirm if the dfoperator proxy is already set """
        return self.__proxy is not None
