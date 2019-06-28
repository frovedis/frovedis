#!/usr/bin/env python

import numpy as np
from scipy.sparse import issparse, csr_matrix
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from .dtype import TypeUtil, DTYPE


class FrovedisCRSMatrix:
    """A python container for Frovedis server side crs_matrix"""

    def __init__(self, mat=None, dtype=None, itype=None):  # constructor
        self.__dtype = dtype
        self.__itype = itype
        self.__fdata = None
        self.__num_row = 0
        self.__num_col = 0
        if mat is not None:
            self.load(mat, dtype=dtype)

    def load(self, inp, dtype=None):
        if issparse(inp):  # any sparse matrix
            mat = inp.tocsr()
            return self.load_scipy_matrix(mat, dtype=dtype)
        elif isinstance(inp, dict):  # dummy_matrix
            return self.load_dummy(inp)
        elif isinstance(inp, str):  # expects text file name
            return self.load_text(inp, dtype=dtype)
        else:
            return self.load_python_data(inp, dtype=dtype)

    def load_dummy(self, dmat):
        self.release()
        try:
            self.__fdata = dmat['dptr']
            self.__num_row = dmat['nrow']
            self.__num_col = dmat['ncol']
        except KeyError:
            raise TypeError("[INTERNAL ERROR] Invalid input encountered.")
        return self

    def load_text(self, fname, dtype=None):
        self.release()
        if dtype is None: dtype = self.__dtype
        else: self.__dtype = dtype
        (host, port) = FrovedisServer.getServerInstance()
        if self.__dtype is None:
            self.__dtype = np.float32
        # default 'float' type data would be loaded
        self.__set_or_validate_itype(np.int32)
        # default 'int' type index woule be loaded
        dmat = rpclib.load_frovedis_crs_matrix(host, port,
                                               fname.encode('ascii'),
                                               False, self.get_dtype(),
                                               self.get_itype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self.load_dummy(dmat)

    def load_binary(self, fname, dtype=None):
        self.release()
        if dtype is None: dtype = self.__dtype
        else: self.__dtype = dtype
        (host, port) = FrovedisServer.getServerInstance()
        if self.__dtype is None:
            self.__dtype = np.float32
        # default 'float' type data would be loaded
        self.__set_or_validate_itype(np.int32)
        # default 'int' type index woule be loaded
        dmat = rpclib.load_frovedis_crs_matrix(host, port,
                                               fname.encode('ascii'),
                                               True, self.get_dtype(),
                                               self.get_itype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]: raise RuntimeError(excpt["info"])
        return self.load_dummy(dmat)

    def load_python_data(self, inp, dtype=None):
        support = ['matrix', 'list', 'ndarray', 'tuple', 'DataFrame']
        if dtype is None: dtype = self.__dtype
        else: self.__dtype = dtype
        if type(inp).__name__ not in support:
            raise TypeError("Unsupported input encountered: " + str(type(inp)))
        if self.__dtype is None:
            mat = csr_matrix(np.asmatrix(inp))  # loaded as input datatype
        else:
            mat = csr_matrix(np.asmatrix(inp), dtype=self.__dtype)
            # loaded as user-given datatype
        return self.load_scipy_matrix(mat, dtype=dtype)

    def load_scipy_matrix(self, mat, dtype=None):
        self.release()
        if dtype is None: dtype = self.__dtype
        else: self.__dtype = dtype
        nelem = mat.data.size
        (nrow, ncol) = mat.shape
        (m_data, m_indices, m_offset) = (mat.data, mat.indices, mat.indptr)
        if self.__dtype is None:
            self.__dtype = m_data.dtype
        else:
            m_data = np.asarray(m_data, self.__dtype)
        if self.__itype is not None:
            m_indices = np.asarray(m_indices, self.__itype)
        self.__set_or_validate_itype(m_indices.dtype)
        m_offset = np.asarray(m_offset, np.int64)
        # always size_t at frovedis server
        ddt = self.get_dtype()
        idt = self.get_itype()
        (host, port) = FrovedisServer.getServerInstance()
        if ddt == DTYPE.INT and idt == DTYPE.INT:
            dmat = rpclib.create_frovedis_crs_II_matrix(host, port, nrow, ncol,
                                                        m_data, m_indices,
                                                        m_offset, nelem)
        elif ddt == DTYPE.INT and idt == DTYPE.LONG:
            dmat = rpclib.create_frovedis_crs_IL_matrix(host, port, nrow, ncol,
                                                        m_data, m_indices,
                                                        m_offset, nelem)
        elif ddt == DTYPE.LONG and idt == DTYPE.INT:
            dmat = rpclib.create_frovedis_crs_LI_matrix(host, port, nrow, ncol,
                                                        m_data, m_indices,
                                                        m_offset, nelem)
        elif ddt == DTYPE.LONG and idt == DTYPE.LONG:
            dmat = rpclib.create_frovedis_crs_LL_matrix(host, port, nrow, ncol,
                                                        m_data, m_indices,
                                                        m_offset, nelem)
        elif ddt == DTYPE.FLOAT and idt == DTYPE.INT:
            dmat = rpclib.create_frovedis_crs_FI_matrix(host, port, nrow, ncol,
                                                        m_data, m_indices,
                                                        m_offset, nelem)
        elif ddt == DTYPE.FLOAT and idt == DTYPE.LONG:
            dmat = rpclib.create_frovedis_crs_FL_matrix(host, port, nrow, ncol,
                                                        m_data, m_indices,
                                                        m_offset, nelem)
        elif ddt == DTYPE.DOUBLE and idt == DTYPE.INT:
            dmat = rpclib.create_frovedis_crs_DI_matrix(host, port, nrow, ncol,
                                                        m_data, m_indices,
                                                        m_offset, nelem)
        elif ddt == DTYPE.DOUBLE and idt == DTYPE.LONG:
            dmat = rpclib.create_frovedis_crs_DL_matrix(host, port, nrow, ncol,
                                                        m_data, m_indices,
                                                        m_offset, nelem)
        else:
            raise TypeError("Unsupported dtype/itype for crs_matrix creation!")
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self.load_dummy(dmat)

    def save(self, fname):
        if self.__fdata is not None:
            (host, port) = FrovedisServer.getServerInstance()
            rpclib.save_frovedis_crs_matrix(host, port, self.get(),
                                            fname.encode('ascii'),
                                            False, self.get_dtype(),
                                            self.get_itype())
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])

    def save_binary(self, fname):
        if self.__fdata is not None:
            (host, port) = FrovedisServer.getServerInstance()
            rpclib.save_frovedis_crs_matrix(host, port, self.get(),
                                            fname.encode('ascii'),
                                            True, self.get_dtype(),
                                            self.get_itype())
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])

    def release(self):
        if self.__fdata is not None:
            (host, port) = FrovedisServer.getServerInstance()
            rpclib.release_frovedis_crs_matrix(host, port, self.get(),
                                               self.get_dtype(),
                                               self.get_itype())
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            self.__fdata = None
            self.__num_row = 0
            self.__num_col = 0

    def __del__(self):  # destructor
        if FrovedisServer.isUP():
            self.release()

    def debug_print(self):
        if self.__fdata is not None:
            (host, port) = FrovedisServer.getServerInstance()
            rpclib.show_frovedis_crs_matrix(host, port, self.get(),
                                            self.get_dtype(),
                                            self.get_itype())
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])

    def get(self):
        return self.__fdata

    def numRows(self):
        return self.__num_row

    def numCols(self):
        return self.__num_col

    def get_dtype(self):
        return TypeUtil.to_id_dtype(self.__dtype)

    def get_itype(self):
        return TypeUtil.to_id_dtype(self.__itype)

    def __set_or_validate_itype(self, dt):
        if self.__itype is None:
            self.__itype = dt
        elif self.__itype != np.int32 and self.__itype != np.int64:
            raise ValueError("Invalid type for crs indices: ", self.__itype)

    @staticmethod
    def asCRS(mat, dtype=None, retIsConverted=False):
        if isinstance(mat, FrovedisCRSMatrix):
            if retIsConverted: return (mat, False)
            else: return mat
        elif issparse(mat):  # any sparse matrix
            smat = mat.tocsr()
            ret = FrovedisCRSMatrix(dtype=dtype).load_scipy_matrix(smat)
            if retIsConverted: return (ret, True)
            else: return ret
        else:
            ret = FrovedisCRSMatrix(dtype=dtype).load_python_data(mat)
            if retIsConverted: return (ret, True)
            else: return ret
           
