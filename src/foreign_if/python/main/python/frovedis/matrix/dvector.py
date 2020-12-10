#!/usr/bin/env python

from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from .dtype import TypeUtil, DTYPE
from ctypes import c_char_p, c_int, c_long, c_float, c_double, POINTER
import numpy as np


class FrovedisDvector:
    """A python container for holding Frovedis server side
    dvector<double> pointers"""

    def __init__(self, vec=None, dtype=None):  # constructor
        self.__dtype = dtype
        self.__fdata = None
        self.__size = 0
        if vec is not None:
            self.load(vec, dtype=dtype)

    def load(self, inp, dtype=None):
        if isinstance(inp, dict):
            return self.load_dummy(inp)
        if type(inp).__name__ == 'instance' or type(inp).__name__ == 'str':
            raise TypeError(
                "Unsupported input encountered: " + str(type(inp)))

        shape = np.shape(inp)
        if len(shape) == 1:
            inp = np.ravel(inp)
        elif len(shape) == 2 and shape[1] == 1: # column-vector
            import warnings 
            warnings.warn("A column-vector y was passed when a 1d array was"
                          " expected. Please change the shape of y to "
                          "(n_samples, ), for example using ravel().",
                          UserWarning)
            inp = np.ravel(inp)
        else:
            raise ValueError("bad input shape {0}".format(shape))
        
        if dtype is not None: 
            self.__dtype = dtype
        if self.__dtype is not None:
            vec = np.asarray(inp, dtype=self.__dtype)
        else:
            vec = np.asarray(inp)
        return self.load_numpy_array(vec, dtype=self.__dtype)

    def load_numpy_array(self, vec, dtype=None):
        self.release()
        if vec.ndim > 1:
            raise ValueError(
                "Input dimension is more than 1 (Expect: Array, Got: Matrix)")
        if dtype is None: dtype = self.__dtype
        else: self.__dtype = dtype
        if self.__dtype is None:
            self.__dtype = vec.dtype
        else:
            vec = np.asarray(vec, dtype=self.__dtype)
        data_vector = vec.T  # returns self, since ndim=1
        data_size = vec.size
        (host, port) = FrovedisServer.getServerInstance()
        data_type = self.get_dtype()
        if data_type == DTYPE.INT:
            dvec = rpclib.create_frovedis_int_dvector(host, port, data_vector,
                                                      data_size)
        elif data_type == DTYPE.LONG:
            dvec = rpclib.create_frovedis_long_dvector(host, port, data_vector,
                                                       data_size)
        elif data_type == DTYPE.FLOAT:
            dvec = rpclib.create_frovedis_float_dvector(host, port,
                                                        data_vector, data_size)
        elif data_type == DTYPE.DOUBLE:
            dvec = rpclib.create_frovedis_double_dvector(host, port,
                                                         data_vector,
                                                         data_size)
        elif data_type == DTYPE.STRING:
            ptr_arr = (c_char_p * data_size)()
            data_vector = np.array([e.encode('ascii') for e in data_vector])
            ptr_arr[:] = data_vector
            dvec = rpclib.create_frovedis_string_dvector(host, port, ptr_arr,
                                                         data_size)
        else:
            raise TypeError(
                "Unsupported dtype is specified for dvector creation!")
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self.load_dummy(dvec)

    def load_dummy(self, dvec):
        self.release()
        try:
            self.__fdata = (dvec['dptr'])
            self.__size = (dvec['size'])
            self.__dtype = TypeUtil.to_numpy_dtype(
                dvec['vtype'])  # Must be added from c++ side
        except KeyError:
            raise TypeError("[INTERNAL ERROR] Invalid input encountered.")
        return self

    def release(self):
        if self.__fdata is not None:
            (host, port) = FrovedisServer.getServerInstance()
            rpclib.release_frovedis_dvector(host, port, self.get(),
                                            self.get_dtype())
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            self.__fdata = None
            self.__dtype = None
            self.__size = 0

    def debug_print(self):
        if self.__fdata is not None:
            (host, port) = FrovedisServer.getServerInstance()
            rpclib.show_frovedis_dvector(host, port, self.get(),
                                         self.get_dtype())
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])

    def get(self):
        return self.__fdata

    def size(self):
        return self.__size

    def get_dtype(self):
        return TypeUtil.to_id_dtype(self.__dtype)

    # def __del__(cls): # destructor
    #   if FrovedisServer.isUP(): cls.release()

    def encode(self, src=None, target=None, need_logic=False):
        if src is None and target is None:
            return self.__encode_zero_based(need_logic)
        else:
            return self.__encode_as_needed(src, target, need_logic)

    def __encode_zero_based(self, need_logic=False):
        if self.__fdata:
            (host, port) = FrovedisServer.getServerInstance()
            proxy = rpclib.encode_frovedis_dvector_zero_based(host, port,
                                                              self.get(),
                                                              self.get_dtype())
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            ret = FrovedisDvector(dtype=self.__dtype)
            ret.__fdata = proxy
            ret.__size = self.__size
            if need_logic:
                src = np.asarray(self.get_unique_elements(), dtype=self.__dtype)
                target = np.arange(src.size, dtype=self.__dtype)
                logic = dict(zip(target, src))
                return (ret, logic)
            else:
                return ret

    def __encode_as_needed(self, src, target, need_logic=False):
        if self.__fdata:
            src = np.asarray(src, self.__dtype)
            target = np.asarray(target, self.__dtype)
            sz = src.size
            if sz != target.size:
                raise ValueError(\
                    "encode: input src and target have different sizes!")
            (host, port) = FrovedisServer.getServerInstance()
            dt = self.get_dtype()
            if dt == DTYPE.INT:
                sptr = src.ctypes.data_as(POINTER(c_int))
                tptr = target.ctypes.data_as(POINTER(c_int))
                proxy = rpclib.encode_frovedis_int_dvector(host, port,
                                                           self.get(),
                                                           sptr, tptr, sz)
            elif dt == DTYPE.LONG:
                sptr = src.ctypes.data_as(POINTER(c_long))
                tptr = target.ctypes.data_as(POINTER(c_long))
                proxy = rpclib.encode_frovedis_long_dvector(host, port,
                                                            self.get(),
                                                            sptr, tptr, sz)
            elif dt == DTYPE.FLOAT:
                sptr = src.ctypes.data_as(POINTER(c_float))
                tptr = target.ctypes.data_as(POINTER(c_float))
                proxy = rpclib.encode_frovedis_float_dvector(host, port,
                                                             self.get(),
                                                             sptr, tptr, sz)
            elif dt == DTYPE.DOUBLE:
                sptr = src.ctypes.data_as(POINTER(c_double))
                tptr = target.ctypes.data_as(POINTER(c_double))
                proxy = rpclib.encode_frovedis_double_dvector(host, port,
                                                              self.get(),
                                                              sptr, tptr, sz)
            else:
                raise TypeError(\
                "encode: currently supports int/long/float/double type only!")
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            ret = FrovedisDvector(dtype=self.__dtype)
            ret.__fdata = proxy
            ret.__size = self.__size
            if need_logic:
                logic = dict(zip(target, src))
                return (ret, logic)
            else:
                return ret

    def get_unique_elements(self):
        if self.__fdata:
            (host, port) = FrovedisServer.getServerInstance()
            ret = rpclib.get_distinct_elements(host, \
                    port, self.get(), self.get_dtype())
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return ret

    @staticmethod
    def as_dvec(vec, dtype=None, retIsConverted=False):
        if isinstance(vec, FrovedisDvector):
            if retIsConverted: (vec, False)
            else: return vec
        else:
            ret = FrovedisDvector(vec, dtype=dtype)
            if retIsConverted: (ret, True)
            else: return ret

class FrovedisIntDvector(FrovedisDvector):
    """A python Container handles integer type dvector"""

    def __init__(self, vec=None):
        FrovedisDvector.__init__(self, vec=vec, dtype=np.int32)
        if vec is not None:
            self.load(vec)

    @staticmethod
    def as_dvec(vec):
        if isinstance(vec, FrovedisIntDvector):
            return vec
        else:
            return FrovedisIntDvector(vec)


class FrovedisLongDvector(FrovedisDvector):
    """ A python Container handles long integer type dvector"""

    def __init__(self, vec=None):
        FrovedisDvector.__init__(self, vec=vec, dtype=np.int64)
        if vec is not None:
            self.load(vec)

    @staticmethod
    def as_dvec(vec):
        """as_dvec"""
        if isinstance(vec, FrovedisLongDvector):
            return vec
        else:
            return FrovedisLongDvector(vec)


class FrovedisFloatDvector(FrovedisDvector):
    """A Python container contains float type dvector"""

    def __init__(self, vec=None):
        FrovedisDvector.__init__(self, vec=vec, dtype=np.float32)
        if vec is not None:
            self.load(vec)

    @staticmethod
    def as_dvec(vec):
        """as_dvec"""
        if isinstance(vec, FrovedisFloatDvector):
            return vec
        else:
            return FrovedisFloatDvector(vec)


class FrovedisDoubleDvector(FrovedisDvector):
    """A python Container handles double type dvector"""

    def __init__(self, vec=None):
        FrovedisDvector.__init__(self, vec=vec, dtype=np.float64)
        if vec is not None:
            self.load(vec)

    @staticmethod
    def as_dvec(vec):
        """as_dvec"""
        if isinstance(vec, FrovedisDoubleDvector):
            return vec
        else:
            return FrovedisDoubleDvector(vec)


class FrovedisStringDvector(FrovedisDvector):
    """A python Container handles string type dvector"""

    def __init__(self, vec=None):
        FrovedisDvector.__init__(self, vec=vec, dtype=np.dtype(str))
        if vec is not None:
            self.load(vec)

    @staticmethod
    def as_dvec(vec):
        """as_dvec"""
        if isinstance(vec, FrovedisStringDvector):
            return vec
        else:
            return FrovedisStringDvector(vec)
