"""ml_data.py"""
#!/usr/bin/env python

import numpy as np
from scipy.sparse import issparse, csr_matrix
from ..exrpc.server import FrovedisServer
from ..exrpc.rpclib import distinct_count, check_server_exception
from .dvector import FrovedisDvector
from .crs import FrovedisCRSMatrix
from .dense import FrovedisColmajorMatrix
from .dense import FrovedisRowmajorMatrix
from .dtype import TypeUtil, DTYPE


class FrovedisLabeledPoint:
    """A python container for frovedis side data for supervised
    ML algorithms"""

    def __init__(self, mat, lbl):
        # decision making whether the converted data would be movable upon
        # destruction
        if (isinstance(mat, FrovedisCRSMatrix) or\
            isinstance(mat, FrovedisColmajorMatrix)):
            self.__mat_movable = False
        else:
            self.__mat_movable = True

        (host, port) = FrovedisServer.getServerInstance()
        if isinstance(lbl, FrovedisDvector):
            self.__lbl_movable = False
            #self.unique_label_count = distinct_count(host, port, lbl.get(),
            #                                         lbl.get_dtype())
            self.unique_elements = np.asarray(lbl.get_unique_elements())
            excpt = check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
        else:
            self.__lbl_movable = True
            #self.unique_label_count = len(np.unique(lbl))
            self.unique_elements = np.unique(lbl)

        if issparse(mat) or isinstance(mat, FrovedisCRSMatrix):
            self.__isDense = False
        else:
            self.__isDense = True

        if self.__isDense:
            if self.__mat_movable:
                mat = np.asmatrix(mat)
                if mat.dtype != np.float32 and mat.dtype != np.float64:
                    # default double type (in case input matrix is integer type)
                    target_dtype = np.float64
                else:
                    target_dtype = mat.dtype
                self.X = FrovedisColmajorMatrix.asCMM(mat, dtype=target_dtype)
            else:
                self.X = mat # already created colmajor matrix
            self.__dtype = self.X.get_dtype()
            self.__itype = 0  # not meaningful for dense matrix
        else: # sparse case
            if self.__mat_movable:
                mat = mat.tocsr()
                if mat.dtype != np.float32 and mat.dtype != np.float64:
                    # default double type (in case input matrix is integer type)
                    target_dtype = np.float64
                else:
                    target_dtype = mat.dtype
                self.X = FrovedisCRSMatrix.asCRS(mat, dtype=target_dtype)
            else:
                self.X = mat # already created crs matrix
            self.__dtype = self.X.get_dtype()
            self.__itype = self.X.get_itype()

        self.__num_row = self.X.numRows()
        self.__num_col = self.X.numCols()

        # if lbl is ndarary or any python tuple/list etc.,
        # it would be converted as xdtype data, while creating frovedis dvector
        # to support sklearn style integer input for labels
        # (mainly in case of classification problems)
        xdtype = TypeUtil.to_numpy_dtype(self.__dtype)
        self.y = FrovedisDvector.as_dvec(lbl, dtype=xdtype)

        if self.__dtype != DTYPE.FLOAT and self.__dtype != DTYPE.DOUBLE:
            raise TypeError(
                "Expected training data either of float or double type!")
        if not self.__isDense and (
                self.__itype != DTYPE.INT and self.__itype != DTYPE.LONG):
            raise TypeError(
                "Expected training sparse data itype to be"
                " either int or long!")
        if self.__dtype != self.y.get_dtype():
            type1 = TypeUtil.to_numpy_dtype(self.__dtype)
            type2 = TypeUtil.to_numpy_dtype(self.y.get_dtype())
            msg = "Type(point): " + str(type1) + "; Type(label): " + str(type2)
            raise ValueError(
                "Incompatible types for input labels and points: ", msg)
        if self.__num_row != self.y.size():
            msg = "Size(point): " + str(
                self.__num_row) + "; Size(label): " + str(self.y.size())
            raise ValueError("Incompatible sizes of input labels and "
                             "points: ", msg)

    def release(self):
        """release"""
        if self.__lbl_movable:
            self.y.release()
        if self.__mat_movable:
            self.X.release()

    def is_movable(self):
        """is_movable"""
        return (self.__lbl_movable, self.__mat_movable)

    def debug_print(self):
        """debug_print"""
        print("label: ")
        self.y.debug_print()
        print("point: ")
        self.X.debug_print()
        print("dtype: ", TypeUtil.to_numpy_dtype(self.get_dtype()))

    def get(self):
        """get"""
        return self.X, self.y

    def get_dtype(self):
        """get_dtype"""
        return self.__dtype

    def get_itype(self):
        """get_itype"""
        return self.__itype

    def numRows(self):
        """numRows"""
        return self.__num_row

    def numCols(self):
        """numCols"""
        return self.__num_col

    def is_dense(self):
        """is_dense"""
        return self.__isDense

    def get_distinct_labels(self):
        """get_distinct_labels"""
        return self.unique_elements

    def get_distinct_label_count(self):
        """get_distinct_label_count"""
        return self.unique_elements.size

    def __del__(self):
        if FrovedisServer.isUP():
            self.release()


class FrovedisFeatureData:
    """A python container for frovedis side data for unsupervised
    ML algorithms"""

    def __init__(self, mat, is_dense=None, dense_kind=None, dtype=None, \
                 itype=None, allow_int_dtype=False):
        # decision making whether the converted data would be movable
        # upon destruction
        if isinstance(mat, (FrovedisCRSMatrix, FrovedisRowmajorMatrix,
                            FrovedisColmajorMatrix)):
            self.__mat_movable = False
        else:
            self.__mat_movable = True

        data_type = type(is_dense).__name__
        if data_type != 'NoneType':
            if data_type != 'bool':
                raise TypeError(
                    "Expected None or boolean for is_dense, received: ",
                    data_type)
        if data_type == 'NoneType':  # decide from input data
            if issparse(mat) or isinstance(mat, FrovedisCRSMatrix):
                self.__isDense = False
            else:
                self.__isDense = True
        else:
            self.__isDense = is_dense

        if self.__isDense:
            data_type = type(dense_kind).__name__
            # decide dense_type (rowmajor or colmajor)
            if data_type != 'NoneType':
                if data_type != 'str':
                    raise TypeError(
                        "Expected None or String for dense_kind, received: ",
                        data_type)
            if data_type == 'NoneType':
                dense_kind = 'rowmajor'  # default dense type

            # load dense data
            if self.__mat_movable:
                if dtype is None:
                    mat = np.asmatrix(mat)
                else:
                    mat = np.asmatrix(mat, dtype=dtype) # user given dtype
                                                        #can not be int kind
                if allow_int_dtype:
                    if mat.dtype != np.int32 and mat.dtype != np.int64:
                        target_dtype = np.int64 #(default long)
                    else:
                        target_dtype = mat.dtype
                else:
                    if mat.dtype != np.float32 and mat.dtype != np.float64: 
                        target_dtype = np.float64 #(default double)
                    else:
                        target_dtype = mat.dtype
                if dense_kind == 'colmajor':
                    self.X = FrovedisColmajorMatrix.asCMM(mat, \
                        dtype=target_dtype)
                elif dense_kind == 'rowmajor':
                    self.X = FrovedisRowmajorMatrix.asRMM(mat, \
                        dtype=target_dtype)
                else:
                    raise ValueError(\
                        "Supported dense kinds are either rowmajor or colmajor")
            else:
                self.X = mat # already frovedis supported matrix
            self.__dtype = self.X.get_dtype()
            self.__itype = 0  # not meaningful for dense matrix
        else:
            if self.__mat_movable:
                if dtype is None:
                    mat = csr_matrix(mat) #ok for dense and sparse matrices
                else:
                    mat = csr_matrix(mat, dtype=dtype)
                if allow_int_dtype:
                    if mat.dtype != np.int32 and mat.dtype != np.int64:
                        target_dtype = np.int64 #(default long)
                    else:
                        target_dtype = mat.dtype
                else:
                    if mat.dtype != np.float32 and mat.dtype != np.float64: 
                        target_dtype = np.float64 #(default double)
                    else:
                        target_dtype = mat.dtype
                self.X = FrovedisCRSMatrix.asCRS(mat, dtype=target_dtype, \
                                                 itype=itype)
            else:
                self.X = mat # already frovedis crs matrix
            self.__dtype = self.X.get_dtype()
            self.__itype = self.X.get_itype()

        self.__num_row = self.X.numRows()
        self.__num_col = self.X.numCols()

        if self.__dtype != DTYPE.FLOAT and self.__dtype != DTYPE.DOUBLE \
                                       and allow_int_dtype != True:
            raise TypeError(
                "Expected training data either of float or double type!")
        elif self.__dtype != DTYPE.INT and self.__dtype != DTYPE.LONG \
                                       and allow_int_dtype == True:
            raise TypeError(
                "Expected training data either of int or long type!")
        if not self.__isDense and (
                self.__itype != DTYPE.INT and self.__itype != DTYPE.LONG):
            raise TypeError(
                "Expected training sparse data itype to be"
                " either int or long!")

    def release(self):
        """release"""
        if self.__mat_movable:
            self.X.release()

    def is_movable(self):
        """is_movable"""
        return self.__mat_movable

    def debug_print(self):
        """debug_print"""
        self.X.debug_print()

    def get(self):
        """get"""
        return self.X

    def get_dtype(self):
        """get_dtype"""
        return self.__dtype

    def get_itype(self):
        """get_itype"""
        return self.__itype

    def numRows(self):
        """numRows"""
        return self.__num_row

    def numCols(self):
        """numCols"""
        return self.__num_col

    def is_dense(self):
        """is_dense"""
        return self.__isDense

    def __del__(self):
        if FrovedisServer.isUP():
            self.release()
