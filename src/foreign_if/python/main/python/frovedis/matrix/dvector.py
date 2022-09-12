"""
dvector.py: contains the implementation of frovedis distributed vector
"""
import sys
import numpy as np
from ctypes import c_char_p, c_int, c_long, c_ulong, c_float, c_double, \
                   c_byte, POINTER

from ..utils import str_type
from ..config import global_config
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer, set_association, \
                           check_association, do_if_active_association
from .dtype import TypeUtil, DTYPE, get_string_array_pointer, str_encode

class FrovedisDvector:
    """
    A python container for holding Frovedis server side
    dvector<T> pointers. Supported 'T' types are; int, long, ulong,
    float, double, string
    """

    def __init__(self, vec=None, dtype=None):  # constructor
        self.__dtype = None if dtype is None else TypeUtil.to_id_dtype(dtype)
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
            self.__dtype = TypeUtil.to_id_dtype(dtype)
        if self.__dtype is not None:
            vec = np.asarray(inp, dtype=self.get_numpy_dtype())
        else:
            vec = np.asarray(inp)
        return self.load_numpy_array(vec, dtype=self.get_numpy_dtype())

    def load_numpy_array(self, vec, dtype=None):
        self.release()
        if vec.ndim > 1:
            raise ValueError(\
            "Input dimension is more than 1 (Expected: Array, Got: Matrix)")
        if dtype is None: 
            dtype = self.get_numpy_dtype()
        else: 
            self.__dtype = TypeUtil.to_id_dtype(dtype)

        if self.__dtype is None:
            self.__dtype = TypeUtil.to_id_dtype(vec.dtype)
        else:
            vec = np.asarray(vec, dtype=self.get_numpy_dtype())

        data_vector = vec.T  # returns self, since ndim=1
        data_size = vec.size
        (host, port) = FrovedisServer.getServerInstance()

        data_type = self.get_dtype()
        # StringDvector -> read config to decide whether to create 
        # dvector<std::string> or node_local<words>
        if data_type == DTYPE.STRING:
            data_type = str_type()

        rawsend = global_config.get("rawsend_enabled")
        #print("sending with rawsend_enabled: {}".format(rawsend))
        if data_type == DTYPE.INT:
            dvec = rpclib.create_frovedis_int_dvector(host, port, data_vector,
                                                      data_size, rawsend)
        elif data_type == DTYPE.LONG:
            dvec = rpclib.create_frovedis_long_dvector(host, port, data_vector,
                                                       data_size, rawsend)
        elif data_type == DTYPE.ULONG:
            dvec = rpclib.create_frovedis_ulong_dvector(host, port, data_vector,
                                                        data_size, rawsend)
        elif data_type == DTYPE.FLOAT:
            dvec = rpclib.create_frovedis_float_dvector(host, port,
                                                        data_vector, 
                                                        data_size, rawsend)
        elif data_type == DTYPE.DOUBLE:
            dvec = rpclib.create_frovedis_double_dvector(host, port,
                                                         data_vector,
                                                         data_size, rawsend)
        elif data_type == DTYPE.STRING:
            ptr_arr = get_string_array_pointer(data_vector)
            dvec = rpclib.create_frovedis_string_dvector(host, port, ptr_arr,
                                                         data_size)
        elif data_type == DTYPE.WORDS:
            byte_arr = np.asarray(bytearray(data_vector))
            #print(data_vector)
            #print(byte_arr)
            is_utf32_le = sys.version_info[0] >= 3
            bptr = byte_arr.ctypes.data_as(POINTER(c_byte))
            dvec = rpclib.create_frovedis_words_node_local(host, port, \
                            bptr, data_vector.nbytes, data_vector.itemsize, \
                            is_utf32_le, FrovedisServer.getSize(), rawsend)
        else:
            raise TypeError(
                "Unsupported dtype is specified for dvector creation!")
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self.load_dummy(dvec)

    @set_association
    def load_dummy(self, dvec):
        self.release()
        try:
            self.__fdata = dvec['dptr']
            self.__size  = dvec['size']
            self.__dtype = dvec['vtype']  # Must be added from c++ side
        except KeyError:
            raise TypeError("[INTERNAL ERROR] Invalid input encountered.")
        return self

    def reset(self):
        """
        to reset the instance members. It expects user program will explicitly
        free dvector memory from server heap
        """
        self.__fdata = None
        self.__dtype = None
        self.__size = 0

    def release(self):
        """
        resets after-fit populated attributes to None
        """
        self.__release_server_heap()
        self.reset()

    @do_if_active_association
    def __release_server_heap(self):
        """
        to release model pointer from server heap
        """
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.release_frovedis_dvector(host, port, self.get(),
                                        self.get_dtype())
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

    @check_association
    def debug_print(self, limit=5):
        dtype = self.get_dtype()
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.show_frovedis_dvector(host, port, self.get(), dtype, limit)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        if dtype == DTYPE.STRING or dtype == DTYPE.WORDS:
            print("dtype: string")
        else:
            print("dtype: %s" % TypeUtil.to_numpy_dtype(dtype))

    def get(self):
        return self.__fdata

    def size(self):
        return self.__size

    def get_dtype(self):
        return self.__dtype

    def get_numpy_dtype(self):
        return None if self.__dtype is None \
                    else TypeUtil.to_numpy_dtype(self.__dtype)

    def encode(self, src=None, target=None, need_logic=False):
        if src is None and target is None:
            return self.__encode_zero_based(need_logic)
        else:
            return self.__encode_as_needed(src, target, need_logic)

    @check_association
    def __encode_zero_based(self, need_logic=False):
        (host, port) = FrovedisServer.getServerInstance()
        proxy = rpclib.encode_frovedis_dvector_zero_based(host, port,
                                                          self.get(),
                                                          self.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        dummy = {'dptr': proxy, 'size': self.size(), 'vtype': self.get_dtype()}
        ret = FrovedisDvector().load(dummy)
        if need_logic:
            dtype = self.get_numpy_dtype()
            src = np.asarray(self.get_unique_elements(), dtype=dtype)
            target = np.arange(src.size, dtype=dtype)
            logic = dict(zip(target, src))
            return (ret, logic)
        else:
            return ret

    @check_association
    def __encode_as_needed(self, src, target, need_logic=False):
        if self.__fdata:
            dtype = self.get_numpy_dtype()
            src = np.asarray(src, dtype=dtype)
            target = np.asarray(target, dtype=dtype)
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
            elif dt == DTYPE.ULONG:
                sptr = src.ctypes.data_as(POINTER(c_ulong))
                tptr = target.ctypes.data_as(POINTER(c_ulong))
                proxy = rpclib.encode_frovedis_ulong_dvector(host, port,
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
            dummy = {'dptr': proxy, 'size': self.size(), \
                     'vtype': self.get_dtype()}
            ret = FrovedisDvector().load(dummy)
            if need_logic:
                logic = dict(zip(target, src))
                return (ret, logic)
            else:
                return ret

    @check_association
    def replace(self, src, target, inplace=False):
        """ replace 'src' by 'target' """
        (host, port) = FrovedisServer.getServerInstance()
        proxy = rpclib.dvector_replace(host, port, self.get(), \
                                       self.get_dtype(), \
                                       str_encode(str(src)), \
                                       str_encode(str(target)), inplace)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        if inplace:
            ret = self
        else:
            dummy = {'dptr': proxy, 'size': self.size(), \
                     'vtype': self.get_dtype()}
            ret = FrovedisDvector().load(dummy)
        return ret

    @check_association
    def get_unique_elements(self):
        (host, port) = FrovedisServer.getServerInstance()
        ret = rpclib.get_distinct_elements(host, \
                port, self.get(), self.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return ret

    @check_association
    def to_numpy_array(self):
        sz = self.size()
        dt = self.get_dtype()
        if dt == DTYPE.INT or dt == DTYPE.BOOL:
            ret = np.empty(sz, dtype=np.int32)
        elif dt == DTYPE.LONG or dt == DTYPE.DATETIME or dt == DTYPE.TIMEDELTA:
            ret = np.empty(sz, dtype=np.int64)
        elif dt == DTYPE.ULONG:
            ret = np.empty(sz, dtype=np.uint)
        elif dt == DTYPE.FLOAT:
            ret = np.empty(sz, dtype=np.float32)
        elif dt == DTYPE.DOUBLE:
            ret = np.empty(sz, dtype=np.float64)
        elif dt == DTYPE.STRING or dt == DTYPE.WORDS:
            pass # handles later
        else:
            raise TypeError(\
            "Report Bug: Unsupported dtype for dvector " \
            "to numpy array conversion!")

        (host, port) = FrovedisServer.getServerInstance()
        if dt == DTYPE.STRING:
            # TODO: improve list to ndarray conversion (as it is slower)
            ret = np.asarray(rpclib.string_dvector_to_numpy_array( \
                             host, port, self.get(), sz))
        elif dt == DTYPE.WORDS:
            tmp = self.to_string_dvector()
            ret = np.asarray(rpclib.string_dvector_to_numpy_array( \
                             host, port, tmp.get(), sz))
        else:
            retptr = ret.__array_interface__['data'][0]
            rpclib.dvector_to_numpy_array(host, port, \
                      self.get(), retptr, dt, sz)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return ret

    def to_string_dvector(self):
        """ convert self to StringDvector """
        (host, port) = FrovedisServer.getServerInstance()
        dvec = rpclib.convert_to_string_dvector(host, port, \
                 self.get(), self.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return FrovedisDvector().load_dummy(dvec)

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

    @staticmethod
    def as_dvec(vec):
        """as_dvec"""
        if isinstance(vec, FrovedisLongDvector):
            return vec
        else:
            return FrovedisLongDvector(vec)

class FrovedisULongDvector(FrovedisDvector):
    """ A python Container handles unsigned long (size_t) type dvector"""

    def __init__(self, vec=None):
        FrovedisDvector.__init__(self, vec=vec, dtype=np.uint)

    @staticmethod
    def as_dvec(vec):
        """as_dvec"""
        if isinstance(vec, FrovedisULongDvector):
            return vec
        else:
            return FrovedisULongDvector(vec)

class FrovedisFloatDvector(FrovedisDvector):
    """A Python container contains float type dvector"""

    def __init__(self, vec=None):
        FrovedisDvector.__init__(self, vec=vec, dtype=np.float32)

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

    @staticmethod
    def as_dvec(vec):
        """as_dvec"""
        if isinstance(vec, FrovedisStringDvector):
            return vec
        else:
            return FrovedisStringDvector(vec)
