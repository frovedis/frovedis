"""ml_data.py"""

import numpy as np
import pandas as pd
from scipy.sparse import issparse, csr_matrix
from ..exrpc.server import FrovedisServer
from ..exrpc.rpclib import distinct_count, check_server_exception
from .dvector import FrovedisDvector
from .crs import FrovedisCRSMatrix
from .dense import FrovedisColmajorMatrix
from .dense import FrovedisRowmajorMatrix
from .dtype import TypeUtil, DTYPE

def encode_array_using_map(lbl, src=None, target= None, need_logic = False):
    if src is None and target is None:
        key = np.unique(lbl) # returns sorted keys
        val = np.arange(len(key))
    else:
        key, val = src, target
    label_map = dict(zip(key, val))
    enc_label = np.array([label_map[i] for i in lbl])
    if need_logic:
        return enc_label, dict(zip(val, key)) # returns decoding logic
    else:
        return enc_label

def encode_array_using_df(lbl, src=None, target= None, need_logic = False):
    left = pd.DataFrame({'label': np.asarray(lbl)})
    if src is None and target is None:
        key = left.label.unique() # keys are not sorted
        key.sort()
        val = np.arange(len(key))
    else:
        key, val = src, target
    right = pd.DataFrame({'label': key, 'enc_label': val})
    # default inner-join seems not to preserve order of left keys
    #tmp = left.merge(right, on='label') 
    tmp = left.merge(right, on='label', how='left')
    if need_logic:
        return tmp.enc_label.values, dict(zip(val, key)) # returns decoding logic
    else:
        return tmp.enc_label.values

# encoding also supports string-type array data
# lbl: any array-like data (can accept column-vector as numpy matrix)
def encode_numpy_array(lbl, src=None, target= None, need_logic = False):
    shape = np.shape(lbl)
    if len(shape) == 1:
        lbl = np.ravel(lbl)
    elif len(shape) == 2 and shape[1] == 1: # column-vector
        lbl = np.ravel(lbl)
    else:
        raise ValueError("bad input shape {0}".format(shape))

    if len(lbl) < 100000: # evaluated heuristic
        return encode_array_using_map(lbl, src, target, need_logic)
    else:
        return encode_array_using_df(lbl, src, target, need_logic)

class FrovedisLabeledPoint:
    """A python container for frovedis side data for supervised
    ML algorithms"""

    def __init__(self, mat, lbl, caller="", encode_label=False, 
                 binary_encoder=[-1, 1],
                 dense_kind='colmajor',
                 densify=False):
        # decision making whether the converted data would be movable upon
        # destruction
        if isinstance(mat, (FrovedisCRSMatrix, FrovedisRowmajorMatrix,
                            FrovedisColmajorMatrix)):
            self.__mat_movable = False
        else:
            self.__mat_movable = True

        (host, port) = FrovedisServer.getServerInstance()
        if isinstance(lbl, FrovedisDvector):
            self.__lbl_movable = False
            self.unique_elements = np.asarray(lbl.get_unique_elements())
            if encode_label:
                #TODO: check for non-fraction labels
                ncls = len(self.unique_elements)
                if ncls == 2:
                    enc_y, enc_logic = lbl.encode(self.unique_elements, \
                                       binary_encoder, need_logic = True)
                elif ncls > 2:
                    enc_y, enc_logic = lbl.encode(need_logic = True)
                else:
                    raise ValueError(caller + "number of unique " + \
                             + "labels in y are less than two!")
        else:
            self.__lbl_movable = True
            self.unique_elements = np.unique(lbl)
            if encode_label:
                #TODO: check for non-fraction labels
                ncls = len(self.unique_elements)
                if ncls == 2:
                    enc_y, enc_logic = encode_numpy_array(lbl, \
                                       self.unique_elements, binary_encoder, \
                                       need_logic = True)
                elif ncls > 2:
                    enc_y, enc_logic = encode_numpy_array(lbl, \
                                       need_logic = True)
                else:
                    raise ValueError(caller + "number of unique " + \
                             + "labels in y are less than two!")

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
                if dense_kind == 'colmajor':
                    self.X = FrovedisColmajorMatrix.asCMM(mat, \
                             dtype=target_dtype)
                elif dense_kind == 'rowmajor':
                    self.X = FrovedisRowmajorMatrix.asRMM(mat, \
                             dtype=target_dtype)
                else:
                    raise ValueError(caller + "supported dense_kind is" + \
                                     " either rowmajor or colmajor!")
            else:
                if dense_kind == 'rowmajor' and mat.get_mtype() == 'C' or \
                   dense_kind == 'colmajor' and mat.get_mtype() == 'R':
                    raise TypeError(caller + "currently supports only " + \
                    dense_kind + " matrix as for dense data!")
                self.X = mat # already created colmajor/rowmajor matrix
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
            # for algorithms which don't support sparse data, 
            # please set densify = true
            if densify:
                if dense_kind == 'rowmajor':
                    self.X = self.X.to_frovedis_rowmajor_matrix()
                elif dense_kind == 'colmajor':
                    self.X = self.X.to_frovedis_colmajor_matrix()
                else:
                    raise ValueError(caller + "supported dense_kind is" + \
                                     " either rowmajor or colmajor!")
                self.__isDense = True
                self.__dtype = self.X.get_dtype()
                self.__itype = 0  # not meaningful for dense matrix
            else:
                self.__dtype = self.X.get_dtype()
                self.__itype = self.X.get_itype()

        self.__num_row = self.X.numRows()
        self.__num_col = self.X.numCols()

        # if lbl is ndarary or any python tuple/list etc.,
        # it would be converted as xdtype data, while creating frovedis dvector
        # to support sklearn style integer input for labels
        # (mainly in case of classification problems)
        xdtype = TypeUtil.to_numpy_dtype(self.__dtype)
        if encode_label:
            self.y = FrovedisDvector.as_dvec(enc_y, dtype=xdtype)
            self.enc_logic = enc_logic
        else:
            self.y = FrovedisDvector.as_dvec(lbl, dtype=xdtype)
            self.enc_logic = None

        if self.__dtype != DTYPE.FLOAT and self.__dtype != DTYPE.DOUBLE:
            raise TypeError(caller + "Expected training data" + \
                            " either of float or double type!")
        if not self.__isDense and (
                self.__itype != DTYPE.INT and self.__itype != DTYPE.LONG):
            raise TypeError(caller + "Expected training sparse" + \
                            " data itype to be either int or long!")
        if self.__dtype != self.y.get_dtype():
            type1 = TypeUtil.to_numpy_dtype(self.__dtype)
            type2 = TypeUtil.to_numpy_dtype(self.y.get_dtype())
            msg = "Type(point): " + str(type1) + "; Type(label): " + str(type2)
            raise ValueError(caller + "Incompatible types for" + \
                             " input labels and points: ", msg)
        if self.__num_row != self.y.size():
            msg = "Size(point): " + str(
                self.__num_row) + "; Size(label): " + str(self.y.size())
            raise ValueError(caller + "Incompatible sizes of input" + \
                             " labels and points: ", msg)

    def release(self):
        """release"""
        if self.enc_logic is None:
            if self.__lbl_movable:
                self.y.release()
        else:
            self.y.release() # y is internally encoded labels for this case
        if self.__mat_movable:
            self.X.release()
        self.X = self.y = self.enc_logic = None
        self.unique_elements = None
        self.__dtype = self.__itype = None
        self.__lbl_movable = self.__mat_movable = None
        self.__num_row = self.__num_col = None
        self.__isDense = None

    def is_movable(self):
        """is_movable"""
        return (self.__lbl_movable, self.__mat_movable)

    def debug_print(self):
        """debug_print"""
        if self.X and self.y: # if not released
            if self.enc_logic is None:
                print("label: ")
                self.y.debug_print()
            else:
                print("encoded label: ")
                self.y.debug_print()
                print("encoding logic: ")
                print(self.enc_logic)
            print("point: ")
            self.X.debug_print()
            print("dtype: ", TypeUtil.to_numpy_dtype(self.get_dtype()))
            if not self.is_dense():
                print("itype: ", TypeUtil.to_numpy_dtype(self.get_itype()))

    def get(self):
        """get"""
        if self.enc_logic is None:
            return (self.X, self.y)
        else:
            return (self.X, self.y, self.enc_logic)

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

    @property
    def shape(self):
        return (self.numRows(), self.numCols())

    def is_dense(self):
        """is_dense"""
        return self.__isDense

    def get_distinct_labels(self):
        """get_distinct_labels"""
        return self.unique_elements

    def get_distinct_label_count(self):
        """get_distinct_label_count"""
        return self.unique_elements.size

class FrovedisFeatureData:
    """A python container for frovedis side data for unsupervised
    ML algorithms"""

    def __init__(self, mat, caller="", dtype=None, itype=None, 
                 allow_int_dtype=False, 
                 dense_kind='rowmajor', densify=False):
        # decision making whether the converted data would be movable
        # upon destruction
        if isinstance(mat, (FrovedisCRSMatrix, FrovedisRowmajorMatrix,
                            FrovedisColmajorMatrix)):
            self.__mat_movable = False
        else:
            self.__mat_movable = True

        if issparse(mat) or isinstance(mat, FrovedisCRSMatrix):
            self.__isDense = False
        else:
            self.__isDense = True

        if self.__isDense:
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
                    raise ValueError(caller + "supported dense_kind is" + \
                                     " either rowmajor or colmajor!")
            else:
                if dense_kind == 'rowmajor' and mat.get_mtype() == 'C' or \
                   dense_kind == 'colmajor' and mat.get_mtype() == 'R':
                    raise TypeError(caller + "currently supports only " + \
                    dense_kind + " matrix as for dense data!")
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
            # for algorithms which don't support sparse data,
            # please set densify = true
            if densify:
                if dense_kind == 'rowmajor':
                    self.X = self.X.to_frovedis_rowmajor_matrix()
                elif dense_kind == 'colmajor':
                    self.X = self.X.to_frovedis_colmajor_matrix()
                else:
                    raise ValueError(caller + "supported dense_kind is" + \
                                     " either rowmajor or colmajor!")
                self.__isDense = True
                self.__dtype = self.X.get_dtype()
                self.__itype = 0  # not meaningful for dense matrix
            else:
                self.__dtype = self.X.get_dtype()
                self.__itype = self.X.get_itype()

        self.__num_row = self.X.numRows()
        self.__num_col = self.X.numCols()

        if self.__dtype != DTYPE.FLOAT and self.__dtype != DTYPE.DOUBLE \
                                       and allow_int_dtype != True:
            raise TypeError(caller + "Expected training data" + \
                            " either of float or double type!")
        elif self.__dtype != DTYPE.INT and self.__dtype != DTYPE.LONG \
                                       and allow_int_dtype == True:
            raise TypeError(caller + "Expected training data" + \
                            " either of int or long type!")
        if not self.__isDense and (
                self.__itype != DTYPE.INT and self.__itype != DTYPE.LONG):
            raise TypeError(caller + "Expected training sparse" + \
                            " data itype to be either int or long!")

    def release(self):
        """release"""
        if self.__mat_movable:
            self.X.release()
        self.X = None
        self.__dtype = self.__itype = None
        self.__mat_movable = None
        self.__num_row = self.__num_col = None
        self.__isDense = None

    def is_movable(self):
        """is_movable"""
        return self.__mat_movable

    def debug_print(self):
        """debug_print"""
        if self.X is not None:
            print("point: ")
            self.X.debug_print()
            print("dtype: ", TypeUtil.to_numpy_dtype(self.get_dtype()))
            if not self.is_dense():
                print("itype: ", TypeUtil.to_numpy_dtype(self.get_itype()))

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

    @property
    def shape(self):
        return (self.numRows(), self.numCols())

    def is_dense(self):
        """is_dense"""
        return self.__isDense

