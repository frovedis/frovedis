#!/usr/bin/env python

import numpy as np
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer, set_association, \
                           check_association, do_if_active_association
from .dtype import TypeUtil, DTYPE
from ctypes import POINTER, c_int, c_long, c_float, c_double, c_char_p

class FrovedisVector:
    """ A python container for holding Frovedis server side
    std::vector """

    def __init__(self, vec=None, dtype=None):  # constructor
        self.__dtype = dtype
        self.__fdata = None
        self.__size = None
        if vec is not None:
            self.load(vec=vec, dtype=dtype)

    def load(self, vec, dtype=None):
        """load"""
        if isinstance(vec, dict):
            return self.load_dummy(vec)
        elif isinstance(vec, str):
            return self.load_text(path=vec)
        else:
            return self.load_numpy_array(np.asarray(vec), dtype=dtype)

    def load_numpy_array(self, vec, dtype=None):
        """load_numpy_array"""
        self.release()
        if dtype is None:
            dtype = self.__dtype
        else: self.__dtype = dtype
        if self.__dtype is None:
            self.__dtype = vec.dtype
        else:
            vec = np.asarray(vec, self.__dtype)
        (host, port) = FrovedisServer.getServerInstance()
        dt = self.get_dtype()
        sz = vec.size
        if dt == DTYPE.INT:
            vptr = vec.ctypes.data_as(POINTER(c_int))
            dmat = rpclib.create_frovedis_int_vector(host, port, vptr, sz, dt)
        elif dt == DTYPE.LONG:
            vptr = vec.ctypes.data_as(POINTER(c_long))
            dmat = rpclib.create_frovedis_long_vector(host, port, vptr, sz, dt)
        elif dt == DTYPE.FLOAT:
            vptr = vec.ctypes.data_as(POINTER(c_float))
            dmat = rpclib.create_frovedis_float_vector(host, port, vptr, sz, dt)
        elif dt == DTYPE.DOUBLE:
            vptr = vec.ctypes.data_as(POINTER(c_double))
            dmat = \
            rpclib.create_frovedis_double_vector(host, port, vptr, sz, dt)
        elif dt == DTYPE.STRING:
            vptr = (c_char_p * sz)()
            vptr[:] = [e.encode('ascii') for e in vec.T]
            dmat = \
            rpclib.create_frovedis_string_vector(host, port, vptr, sz, dt)
        else:
            raise ValueError(\
            "load_numpy_array: Unsupported dtype is encountered!")
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self.load_dummy(dmat)

    @set_association
    def load_dummy(self, vec, dtype=None):
        """load_dummy"""
        self.release()
        try:
            self.__fdata = (vec['dptr'])
            self.__size = (vec['size'])
            #self.__dtype = TypeUtil.to_numpy_dtype(vec['vtype'])
        except KeyError:
            raise TypeError("[INTERNAL ERROR] Invalid input encountered.")
        return self

    @check_association
    def to_numpy_array(self):
        """to_numpy_array"""
        (host, port) = FrovedisServer.getServerInstance()
        dt = TypeUtil.to_id_dtype(self.__dtype)
        arr = rpclib.get_frovedis_array(host, port, self.__fdata, dt)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return np.asarray(arr, dtype=self.__dtype)

    @check_association
    def debug_print(self):
        """debug_print"""
        print("vector: ")
        print(self.to_numpy_array())

    def release(self):
        """
        resets after-fit populated attributes to None
        """
        self.__release_server_heap()
        self.__fdata = None
        self.__size = None

    @do_if_active_association
    def __release_server_heap(self):
        """
        to release model pointer from server heap
        """
        (host, port) = FrovedisServer.getServerInstance()
        data_type = TypeUtil.to_id_dtype(self.__dtype)
        rpclib.release_frovedis_array(host, port, self.__fdata, data_type)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

    def __del__(self):
        """
        NAME: __del__
        """
        self.release()

    def is_fitted(self):
        """ function to confirm if the model is already fitted """
        return self.__fdata is not None

    def get(self):
        """get"""
        return self.__fdata

    def size(self):
        """size"""
        return self.__size

    def get_dtype(self):
        """get_dtype"""
        return TypeUtil.to_id_dtype(self.__dtype)

    @check_association
    def save(self, path):
        """save"""
        is_binary = False
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.save_frovedis_vector_client(\
            host, port, self.get(), path.encode('ascii'),\
            is_binary, self.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

    @check_association
    def save_binary(self, path):
        """save_binary"""
        is_binary = True
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.save_frovedis_vector_client(\
            host, port, self.get(), path.encode('ascii'),\
            is_binary, self.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

    def load_text(self, path, dtype=None):
        """load_text"""
        self.release()
        if dtype is not None:
            self.__dtype = dtype
        if self.__dtype is None:
            self.__dtype = np.float32 #default type
        is_binary = False
        (host, port) = FrovedisServer.getServerInstance()
        dummy_vec = \
            rpclib.load_frovedis_vector_client(\
            host, port, path.encode('ascii'),\
            is_binary, self.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self.load_dummy(dummy_vec)

    def load_binary(self, path, dtype=None):
        """load_binary"""
        self.release()
        if dtype is not None:
            self.__dtype = dtype
        if self.__dtype is None:
            self.__dtype = np.float32 #default type
        is_binary = True
        (host, port) = FrovedisServer.getServerInstance()
        dummy_vec = rpclib.load_frovedis_vector_client(\
            host, port, path.encode('ascii'),\
            is_binary, self.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self.load_dummy(dummy_vec)

