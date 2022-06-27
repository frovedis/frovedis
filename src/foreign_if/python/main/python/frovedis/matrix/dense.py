#!/usr/bin/env python

import numpy as np
from frovedis.exrpc import rpclib
from frovedis.exrpc.server import FrovedisServer, set_association, \
                           check_association, do_if_active_association
from ..config import global_config
from .dtype import TypeUtil, DTYPE

# dtype: Currently supports float/double/int/long datatypes.
# Extension for new Frovedis dense matrix is very simple.
# Just extend FrovedisDenseMatrix with a unique 'mtype'.
# All the matrices in this source code are distributed in nature.
# To support local version, it would be very easy.
# Just extend FrovedisDenseMatrix with a unique 'mtype' in small case.
# e.g., FrovedisRowmajorMatrix => mtype: "R"
#       FrovedisRowmajorMatrixLocal => mtype: "r" (not yet supported)

class FrovedisDenseMatrix(object):
    """A python container for Frovedis server side dense matrices"""

    def __init__(self, mtype, mat=None, dtype=None):  # constructor
        if mtype == 'B' and (dtype == np.int32 or dtype == np.int64):
            raise TypeError(
                "Long/Integer type is not supported for blockcyclic matrix!")
        self.__mtype = mtype
        self.__dtype = dtype
        self.__fdata = None
        self.__num_row = 0
        self.__num_col = 0
        if mat is not None:
            self.load(mat, dtype=dtype)

    def load(self, inp, dtype=None):
        """load"""
        if isinstance(inp, dict):  # dummy_mat
            return self.load_dummy(inp)
        elif isinstance(inp, str):  # fname/dname
            return self.load_text(inp, dtype=dtype)
        elif isinstance(inp, FrovedisDenseMatrix):  # copy cons
            return self.copy(inp)
        else:
            return self.load_python_data(inp, dtype=dtype)#any array-like object

    def load_python_data(self, inp, dtype=None):
        """load_python_data"""
        support = ['matrix', 'list', 'ndarray', 'tuple', 'DataFrame']
        if type(inp).__name__ not in support:
            raise TypeError("Unsupported input encountered: " + str(type(inp)))
        if dtype is None: dtype = self.__dtype
        else: self.__dtype = dtype
        if self.__dtype is None:
            mat = np.asmatrix(inp)
        else:
            mat = np.asmatrix(inp, self.__dtype)
        return self.load_numpy_matrix(mat, dtype=dtype)

    def load_numpy_matrix(self, mat, dtype=None):
        """load_numpy_matrix"""
        self.release()
        if dtype is None: dtype = self.__dtype
        else: self.__dtype = dtype
        if self.__dtype is None:
            self.__dtype = mat.dtype
        else:
            mat = np.asmatrix(mat, self.__dtype)
        m_data = mat.A1
        (nrow, ncol) = mat.shape
        (host, port) = FrovedisServer.getServerInstance()
        data_type = self.get_dtype()
        rawsend = global_config.get("rawsend_enabled")
        #print(rawsend)
        if data_type == DTYPE.DOUBLE:
            dmat = rpclib.create_frovedis_double_dense_matrix(host, port,
                                                              nrow, ncol,
                                                              m_data,
                                                              self.__mtype.
                                                              encode('ascii'),
                                                              rawsend)
        elif data_type == DTYPE.FLOAT:
            dmat = rpclib.create_frovedis_float_dense_matrix(host, port,
                                                             nrow, ncol,
                                                             m_data,
                                                             self.__mtype.
                                                             encode('ascii'),
                                                             rawsend)
        elif data_type == DTYPE.LONG:
            dmat = rpclib.create_frovedis_long_dense_matrix(host, port,
                                                            nrow, ncol, m_data,
                                                            self.__mtype.
                                                            encode('ascii'),
                                                            rawsend)
        elif data_type == DTYPE.INT:
            dmat = rpclib.create_frovedis_int_dense_matrix(host, port,
                                                           nrow, ncol, m_data,
                                                           self.__mtype.
                                                           encode('ascii'),
                                                           rawsend)
        else:
            raise TypeError("Unsupported input type: " + self.__dtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self.load_dummy(dmat)

    @set_association
    def load_dummy(self, dmat):
        """load_dummy"""
        self.release()
        try:
            self.__fdata = dmat['dptr']
            self.__num_row = dmat['nrow']
            self.__num_col = dmat['ncol']
        except KeyError:
            raise TypeError("[INTERNAL ERROR] Invalid input encountered.")
        return self

    def copy(self, mat):  # cls = mat
        """copy"""
        self.release()
        if self.__dtype is None:
            self.__dtype = mat.__dtype
        if mat.__mtype != self.__mtype or mat.__dtype != self.__dtype:
            raise TypeError("Incompatible types for copy operation")
        if mat.__fdata is not None:
            (host, port) = FrovedisServer.getServerInstance()
            dmat = rpclib.copy_frovedis_dense_matrix(host, port, mat.get(),
                                                     mat.__mtype.encode(
                                                         'ascii'),
                                                     mat.get_dtype())
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return self.load_dummy(dmat)

    def load_text(self, fname, dtype=None):
        """load_text"""
        self.release()
        if dtype is None: dtype = self.__dtype
        else: self.__dtype = dtype
        (host, port) = FrovedisServer.getServerInstance()
        if self.__dtype is None:
            self.__dtype = np.float32  # default type: float
        dmat = rpclib.load_frovedis_dense_matrix(host, port,
                                                 fname.encode('ascii'),
                                                 False,
                                                 self.__mtype.encode('ascii'),
                                                 self.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self.load_dummy(dmat)

    def load_binary(self, fname, dtype=None):
        """load_binary"""
        self.release()
        if dtype is None: dtype = self.__dtype
        else: self.__dtype = dtype
        (host, port) = FrovedisServer.getServerInstance()
        if self.__dtype is None:
            self.__dtype = np.float32  # default type: float
        dmat = rpclib.load_frovedis_dense_matrix(host, port,
                                                 fname.encode("ascii"),
                                                 True,
                                                 self.__mtype.encode('ascii'),
                                                 self.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self.load_dummy(dmat)

    @check_association
    def save(self, fname):
        """save"""
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.save_frovedis_dense_matrix(host, port, self.get(),
                                          fname.encode('ascii'), False,
                                          self.__mtype.encode('ascii'),
                                          self.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

    @check_association
    def save_binary(self, fname):
        """save_binary"""
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.save_frovedis_dense_matrix(host, port, self.get(),
                                          fname.encode('ascii'), True,
                                          self.__mtype.encode('ascii'),
                                          self.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

    # developer's API
    def set_none(self):
        """ this api sets the metadata to None. it does not
            release the actual matrix
            in server side. this API should
            call when actual matrix is moved to
            another matrix, e.g., a FrovedisDenseMatrix is moved as
            FrovedisBlockclicMatrix and in order to avoid memory release of
            source FrovedisDenseMatrix, its metadata can be set to None """
        self.__fdata = None
        self.__num_row = None
        self.__num_col = None

    def release(self):
        """
        resets after-fit populated attributes to None
        """
        self.__release_server_heap()
        self.set_none()

    @do_if_active_association
    def __release_server_heap(self):
        """
        to release model pointer from server heap
        """
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.release_frovedis_dense_matrix(host, port, self.get(),
                                             self.__mtype.encode('ascii'),
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
    def debug_print(self):
        """debug_print"""
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.show_frovedis_dense_matrix(host, port, self.get(),
                                          self.__mtype.encode('ascii'),
                                          self.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

    def __get_empty_array(self, dtype=None):
        """__get_empty_array"""
        my_shape = [self.numRows(), self.numCols()]
        if dtype is None:
            arr = np.empty(my_shape, dtype=self.__dtype)
        else:
            arr = np.empty(my_shape, dtype=dtype) # user requested dtype
        return arr

    def to_numpy_matrix(self, dtype=None):
        """to_numpy_matrix"""
        arr = self.__get_empty_array(dtype=dtype)
        mat = np.asmatrix(arr) # no-copy
        self.to_numpy_matrix_inplace(mat)
        return mat

    def to_numpy_array(self, dtype=None):
        """to_numpy_array"""
        arr = self.__get_empty_array(dtype=dtype)
        self.to_numpy_array_inplace(arr)
        return arr

    @check_association
    def to_numpy_matrix_inplace(self, mat):
        """ non-retuning function to overwrite input numpy matrix with
            converted matrix. self.size needs to be matched with mat.size
            self.__dtype needs to be matched with mat.dtype """
        if type(mat).__name__ != 'matrix':
            raise TypeError(\
            "to_numpy_matrix_inplace: input is not a numpy matrix!")
        self.__to_numpy_data_inplace(mat, is_ndarray=False)

    @check_association
    def to_numpy_array_inplace(self, arr):
        """ non-retuning function to overwrite input numpy array with
            converted matrix. self.size needs to be matched with mat.size
            self.__dtype needs to be matched with mat.dtype """
        if type(arr).__name__ != 'ndarray':
            raise TypeError(
            "to_numpy_array_inplace: input is not a numpy ndarray!")
        self.__to_numpy_data_inplace(arr, is_ndarray=True)

    def __to_numpy_data_inplace(self, data, is_ndarray=True):
        """ non-retuning function to overwrite input numpy matrix/ndarray with
            converted matrix. self.size needs to be matched with data.size
            self.__dtype needs to be matched with data.dtype """
        if self.__fdata is not None:
            data = np.asmatrix(data) # doesn't copy. it is needed
                                     #to get flattened array A1
            if data.size != self.size:
                raise ValueError(\
                "input matrix/ndarray size is different than self size!")
            (host, port) = FrovedisServer.getServerInstance()
            arr = data.A1  #getting the flatten array from numpy matrix
            data_size = self.size
            inp_type = self.get_dtype()
            out_type = TypeUtil.to_id_dtype(data.dtype)
            # rpc functions overwrite the arr data in C-level
            if inp_type == DTYPE.DOUBLE and out_type == DTYPE.INT:
                rpclib.get_double_rowmajor_array_as_int_array(
                                                 host, port, self.get(),
                                                 self.__mtype.encode('ascii'),
                                                 arr, data_size)
            elif inp_type == DTYPE.DOUBLE and out_type == DTYPE.LONG:
                rpclib.get_double_rowmajor_array_as_long_array(
                                                 host, port, self.get(),
                                                 self.__mtype.encode('ascii'),
                                                 arr, data_size)
            elif inp_type == DTYPE.DOUBLE and out_type == DTYPE.FLOAT:
                rpclib.get_double_rowmajor_array_as_float_array(
                                                 host, port, self.get(),
                                                 self.__mtype.encode('ascii'),
                                                 arr, data_size)
            elif inp_type == DTYPE.DOUBLE and out_type == DTYPE.DOUBLE:
                rpclib.get_double_rowmajor_array_as_double_array(
                                                 host, port, self.get(),
                                                 self.__mtype.encode('ascii'),
                                                 arr, data_size)
            elif inp_type == DTYPE.FLOAT and out_type == DTYPE.INT:
                rpclib.get_float_rowmajor_array_as_int_array(
                                                host, port, self.get(),
                                                self.__mtype.encode('ascii'),
                                                arr, data_size)
            elif inp_type == DTYPE.FLOAT and out_type == DTYPE.LONG:
                rpclib.get_float_rowmajor_array_as_long_array(
                                                host, port, self.get(),
                                                self.__mtype.encode('ascii'),
                                                arr, data_size)
            elif inp_type == DTYPE.FLOAT and out_type == DTYPE.FLOAT:
                rpclib.get_float_rowmajor_array_as_float_array(
                                                host, port, self.get(),
                                                self.__mtype.encode('ascii'),
                                                arr, data_size)
            elif inp_type == DTYPE.FLOAT and out_type == DTYPE.DOUBLE:
                rpclib.get_float_rowmajor_array_as_double_array(
                                                host, port, self.get(),
                                                self.__mtype.encode('ascii'),
                                                arr, data_size)
            elif inp_type == DTYPE.LONG and out_type == DTYPE.INT:
                rpclib.get_long_rowmajor_array_as_int_array(
                                               host, port, self.get(),
                                               self.__mtype.encode('ascii'),
                                               arr, data_size)
            elif inp_type == DTYPE.LONG and out_type == DTYPE.LONG:
                rpclib.get_long_rowmajor_array_as_long_array(
                                               host, port, self.get(),
                                               self.__mtype.encode('ascii'),
                                               arr, data_size)
            elif inp_type == DTYPE.LONG and out_type == DTYPE.FLOAT:
                rpclib.get_long_rowmajor_array_as_float_array(
                                               host, port, self.get(),
                                               self.__mtype.encode('ascii'),
                                               arr, data_size)
            elif inp_type == DTYPE.LONG and out_type == DTYPE.DOUBLE:
                rpclib.get_long_rowmajor_array_as_double_array(
                                               host, port, self.get(),
                                               self.__mtype.encode('ascii'),
                                               arr, data_size)
            elif inp_type == DTYPE.INT and out_type == DTYPE.INT:
                rpclib.get_int_rowmajor_array_as_int_array(
                                              host, port, self.get(),
                                              self.__mtype.encode('ascii'),
                                              arr, data_size)
            elif inp_type == DTYPE.INT and out_type == DTYPE.LONG:
                rpclib.get_int_rowmajor_array_as_long_array(
                                              host, port, self.get(),
                                              self.__mtype.encode('ascii'),
                                              arr, data_size)
            elif inp_type == DTYPE.INT and out_type == DTYPE.FLOAT:
                rpclib.get_int_rowmajor_array_as_float_array(
                                              host, port, self.get(),
                                              self.__mtype.encode('ascii'),
                                              arr, data_size)
            elif inp_type == DTYPE.INT and out_type == DTYPE.DOUBLE:
                rpclib.get_int_rowmajor_array_as_double_array(
                                              host, port, self.get(),
                                              self.__mtype.encode('ascii'),
                                              arr, data_size)
            else:
                raise TypeError("to_numpy_matrix/array: \
                Supported dtypes are int/long/float/double only!")
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            if is_ndarray: data = np.asarray(data)

    @check_association
    def to_frovedis_rowmatrix(self):
        """to_frovedis_rowmatrix"""
        if self.__mtype == 'R':
            return self
        (host, port) = FrovedisServer.getServerInstance()
        dmat = rpclib.get_frovedis_rowmatrix(host, port, self.get(),
                                             self.numRows(),
                                             self.numCols(),
                                             self.__mtype.encode('ascii'),
                                             self.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return FrovedisDenseMatrix(mtype='R', mat=dmat, dtype=self.__dtype)

    @check_association
    def get_rowmajor_view(self):
        """get_rowmajor_view"""
        self.to_frovedis_rowmatrix().debug_print()

    @check_association
    def transpose(self):
        """transpose"""
        (host, port) = FrovedisServer.getServerInstance()
        dmat = rpclib.transpose_frovedis_dense_matrix(host, port,
                                                      self.get(),
                                                      self.__mtype.encode(
                                                          'ascii'),
                                                      self.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return FrovedisDenseMatrix(mtype=self.__mtype, mat=dmat,
                                   dtype=self.__dtype)

    def get(self):
        """get"""
        return self.__fdata

    def numRows(self):
        """numRows"""
        return self.__num_row

    def numCols(self):
        return self.__num_col

    def get_dtype(self):
        """get_dtype"""
        return TypeUtil.to_id_dtype(self.__dtype)

    def get_mtype(self):
        """get_mtype"""
        return self.__mtype

    @property
    def shape(self):
        """shape getter"""
        return (self.numRows(), self.numCols())

    @shape.setter
    def shape(self, s):
        """shape setter"""
        raise AttributeError(\
        "attribute 'shape' of FrovedisDenseMatrix objects is not writable")

    @property
    def size(self):
        """size getter"""
        return self.numRows() * self.numCols()

    @size.setter
    def size(self, s):
        """size setter"""
        raise AttributeError(\
        "attribute 'size' of FrovedisDenseMatrix objects is not writable")

class FrovedisRowmajorMatrix(FrovedisDenseMatrix):
    """A python container for Frovedis server side rowmajor_matrix"""

    def __init__(self, mat=None, dtype=None):
        FrovedisDenseMatrix.__init__(self, 'R', mat, dtype)

    # 'mat' can be either FrovedisRowmajorMatrix or any array-like object
    # FrovedisDenseMatrix inter-conversion is not yet supported (TODO)
    @staticmethod
    def asRMM(mat, dtype=None, retIsConverted=False):
        """asRMM"""
        if isinstance(mat, FrovedisRowmajorMatrix):
            if retIsConverted: return (mat, False)
            else: return mat
        elif (isinstance(mat, FrovedisDenseMatrix) and
              (mat.get_mtype() == 'R')):
            dummy = {'dptr': mat.get(), 'nrow': mat.numRows(), \
                     'ncol': mat.numCols()}
            dt = TypeUtil.to_numpy_dtype(mat.get_dtype())
            rm_mat = FrovedisRowmajorMatrix(mat=dummy, dtype=dt)
            if retIsConverted: return (rm_mat, False)
            else: return rm_mat
        else:
            ret = FrovedisRowmajorMatrix(dtype=dtype).load_python_data(mat)
            if retIsConverted: return (ret, True)
            else: return ret


class FrovedisColmajorMatrix(FrovedisDenseMatrix):
    """A python container for Frovedis server side colmajor_matrix"""

    def __init__(self, mat=None, dtype=None):
        FrovedisDenseMatrix.__init__(self, 'C', mat, dtype)

    # 'mat' can be either FrovedisColmajorMatrix or any array-like object
    # FrovedisDenseMatrix inter-conversion is not yet supported (TODO)
    @staticmethod
    def asCMM(mat, dtype=None, retIsConverted=False):
        """asCMM"""
        if isinstance(mat, FrovedisColmajorMatrix):
            if retIsConverted: return (mat, False)
            else: return mat
        elif (isinstance(mat, FrovedisDenseMatrix) and
              (mat.get_mtype() == 'C')):
            dummy = {'dptr': mat.get(), 'nrow': mat.numRows(), \
                     'ncol': mat.numCols()}
            dt = TypeUtil.to_numpy_dtype(mat.get_dtype())
            cm_mat = FrovedisColmajorMatrix(mat=dummy, dtype=dt)
            if retIsConverted: return (cm_mat, False)
            else: return cm_mat
        else:
            ret = FrovedisColmajorMatrix(dtype=dtype).load_python_data(mat)
            if retIsConverted: return (ret, True)
            else: return ret


from .results import GetrfResult

class FrovedisBlockcyclicMatrix(FrovedisDenseMatrix):
    """A python container for Frovedis server side blockcyclic_matrix"""

    def __init__(self, mat=None, dtype=None):
        if dtype == np.int32 or dtype == np.int64:
            raise TypeError(
                "Unsupported dtype for blockcyclic matrix creation!")
        FrovedisDenseMatrix.__init__(self, 'B', mat, dtype)

    @check_association
    def __add__(self, mat):  # returns(self + mat)
        # tmp = self + tmp(=mat)
        tmp = FrovedisBlockcyclicMatrix(mat=mat)  # copy
        if(self.get_dtype() != tmp.get_dtype()):
            raise TypeError("add: input matrix types are not same!")
        # geadd performs B = al*A + be*B, thus tmp = B and tmp = A + tmp
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.pgeadd(host, port, self.get(), tmp.get(),
                      False, 1.0, 1.0, self.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return tmp

    @check_association
    def __sub__(self, mat):  # returns (cls - mat)
        # tmp = cls - tmp(=mat)
        tmp = FrovedisBlockcyclicMatrix(mat=mat)  # copy
        if(self.get_dtype() != tmp.get_dtype()):
            raise TypeError("sub: input matrix types are not same!")
        # geadd performs B = al*A + be*B, thus tmp = B and tmp = A - tmp
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.pgeadd(host, port, self.get(), tmp.get(),
                      False, 1.0, -1.0, self.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return tmp

    @check_association
    def __mul__(self, mat):  # returns (cls * mat)
        tmp = FrovedisBlockcyclicMatrix.asBCM(mat)
        if(self.get_dtype() != tmp.get_dtype()):
            raise TypeError("mul: input matrix types are not same!")
        (host, port) = FrovedisServer.getServerInstance()
        dmat = rpclib.pgemm(host, port, self.get(), tmp.get(), False, False,
                            1.0, 0.0, self.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        data_type = TypeUtil.to_numpy_dtype(self.get_dtype())
        return FrovedisBlockcyclicMatrix(mat=dmat, dtype=data_type)

    def __invert__(self):  # returns transpose (~cls)
        return self.transpose()

    @check_association
    def inv(self):  # returns inverse of self
        """inv"""
        ret = FrovedisBlockcyclicMatrix(mat=self)  # ret = cls
        (host, port) = FrovedisServer.getServerInstance()
        mat_rf = GetrfResult(rpclib.pgetrf(host, port, ret.get(),
                                           ret.get_dtype()))
        rpclib.pgetri(host, port, ret.get(), mat_rf.ipiv(),
                      ret.get_dtype())  # ret = inv(ret)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return ret

    # 'mat' can be either FrovedisBlockcyclicMatrix or any array-like object
    # FrovedisDenseMatrix inter-conversion is not yet supported (TODO)
    @staticmethod
    def asBCM(mat, dtype=None, retIsConverted=False):
        """asBCM"""
        if isinstance(mat, FrovedisBlockcyclicMatrix):
            if retIsConverted: return (mat, False)
            else: return mat
        elif (isinstance(mat, FrovedisDenseMatrix) and
              (mat.get_mtype() == 'B')):
            dummy = {'dptr': mat.get(), 'nrow': mat.numRows(), \
                     'ncol': mat.numCols()}
            dt = TypeUtil.to_numpy_dtype(mat.get_dtype())
            bc_mat = FrovedisBlockcyclicMatrix(mat=dummy, dtype=dt)
            if retIsConverted: return (bc_mat, False)
            else: return bc_mat
        else:
            ret = FrovedisBlockcyclicMatrix(dtype=dtype).load_python_data(mat)
            if retIsConverted: return (ret, True)
            else: return ret

