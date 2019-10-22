"""
info
"""
#!/usr/bin/env python

from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from ..mllib.model_util import ModelID

class df_to_sparse_info(object):
    """
    A python container for holding information related to dataframe to sparse
    conversion
    """

    def __init__(self, info_id):
        """
        __init__
        """
        self.__uid = info_id

    def load(self, dirname):
        """
        load
        """
        self.release()
        if type(dirname).__name__ != 'str':
            raise TypeError("Expected String, Found: " + type(dirname).__name__)
        info_id = ModelID.get()#getting unique id for conversion info to be registered
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.load_dftable_to_sparse_info(host, port, info_id,
                                           dirname.encode('ascii'))
        self.__uid = info_id
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    def save(self, dirname):
        """
        save
        """
        if self.__uid is None:
            raise ValueError("Operation on invalid frovedis \
                              dftable_to_sparse_info!")
        if type(dirname).__name__ != 'str':
            raise TypeError("Expected String, Found: " + type(dirname).__name__)
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.save_dftable_to_sparse_info(host, port, self.get(),
                                           dirname.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

    def release(self):
        """
        release self
        """
        if self.__uid is None:
            raise ValueError("Operation on invalid frovedis \
                              dftable_to_sparse_info!")
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.release_dftable_to_sparse_info(host, port, self.get())
        self.__uid = None

    def get(self):
        """
        get self
        """
        return self.__uid
