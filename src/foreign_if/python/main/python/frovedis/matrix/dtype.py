"""dtype.py"""
#!/usr/bin/env python

import numpy as np
from ctypes import c_char_p

def get_string_array_pointer(str_vec):
    str_vec = np.asarray(str_vec)
    str_ptr = (c_char_p * len(str_vec))()
    str_ptr[:] = np.array([e.encode('ascii') for e in str_vec.T])
    return str_ptr

class DTYPE:
    """A python container for data types enumerator"""
    INT = 1
    LONG = 2
    FLOAT = 3
    DOUBLE = 4
    STRING = 5
    BOOL = 6
    ULONG = 7

class TypeUtil:
    @staticmethod
    def to_id_dtype(dtype):
        """to_numpy_dtype"""
        if dtype == np.int8 or dtype == np.int32:
            return DTYPE.INT
        elif dtype == np.uint or dtype == np.uint64:
            return DTYPE.ULONG
        elif dtype == np.int64:
            return DTYPE.LONG
        elif dtype == np.float32:
            return DTYPE.FLOAT
        elif dtype == np.float64:
            return DTYPE.DOUBLE
        elif dtype == np.bool:
            return DTYPE.BOOL
        elif dtype == np.dtype(str) or dtype.char == 'S' or dtype.char == 'U':
            return DTYPE.STRING
        else:
            raise TypeError("Unsupported numpy dtype: %s" % dtype)

    @staticmethod
    def to_numpy_dtype(dtype):
        """to_numpy_dtype"""
        if dtype == DTYPE.INT:
            return np.int32
        elif dtype == DTYPE.LONG:
            return np.int64
        elif dtype == DTYPE.ULONG:
            return np.uint
        elif dtype == DTYPE.FLOAT:
            return np.float32
        elif dtype == DTYPE.DOUBLE:
            return np.float64
        elif dtype == DTYPE.BOOL:
            return np.bool
        elif dtype == DTYPE.STRING:
            return np.dtype(str)
        else:
            raise TypeError("Unknown numpy type for the given TID: %d" % dtype)

def get_result_type(arr_of_dtypes):
    sz = len(arr_of_dtypes)
    if sz == 0: 
        raise ValueError("empty array of dtypes is provided!")
    restype = arr_of_dtypes[0]
    for i in range(1, sz):
        restype = np.result_type(restype, arr_of_dtypes[i])
    return restype
