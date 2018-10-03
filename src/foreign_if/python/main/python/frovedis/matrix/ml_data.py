#!/usr/bin/env python

import numpy as np
from scipy.sparse import issparse
from dvector import FrovedisDvector
from crs import FrovedisCRSMatrix
from dense import FrovedisColmajorMatrix
from dense import FrovedisRowmajorMatrix
from dtype import TypeUtil, DTYPE
from ..exrpc.server import *

class FrovedisLabeledPoint:
  "A python container for frovedis side data for supervised ML algorithms"

  def __init__(cls,mat,lbl):
    # decision making whether the converted data would be movable upon destruction
    if (isinstance(mat,FrovedisCRSMatrix) or 
        isinstance(mat,FrovedisColmajorMatrix)): cls.__mat_movable = False
    else: cls.__mat_movable = True

    if isinstance(lbl,FrovedisDvector): cls.__lbl_movable = False
    else: cls.__lbl_movable = True

    if(issparse(mat) or isinstance(mat,FrovedisCRSMatrix)):
      cls.__isDense = False
      cls.X = FrovedisCRSMatrix.asCRS(mat)
      cls.__dtype = cls.X.get_dtype()
      cls.__itype = cls.X.get_itype()
    else:
      cls.__isDense = True
      cls.X = FrovedisColmajorMatrix.asCMM(mat)
      cls.__dtype = cls.X.get_dtype()
      cls.__itype = 0 # not meaningful for colmajor matrix 
    cls.__num_row = cls.X.numRows()
    cls.__num_col = cls.X.numCols()
    cls.y = FrovedisDvector.asDvec(lbl)

    if (cls.__dtype != DTYPE.FLOAT and cls.__dtype != DTYPE.DOUBLE):
      raise TypeError("Expected training data either of float or double type!")
    if (not cls.__isDense and (cls.__itype != DTYPE.INT and cls.__itype != DTYPE.LONG)):
      raise TypeError("Expected training sparse data itype to be either int or long!")
    if (cls.__dtype != cls.y.get_dtype()):
      t1 = TypeUtil.to_numpy_dtype(cls.__dtype)
      t2 = TypeUtil.to_numpy_dtype(cls.y.get_dtype())
      msg = "Type(point): " + str(t1) + "; Type(label): " + str(t2) 
      raise ValueError("Incompatible types for input labels and points: ", msg)
    if(cls.__num_row != cls.y.size()): 
      msg = "Size(point): " + str(cls.__num_row) + "; Size(label): " + str(cls.y.size())
      raise ValueError("Incompatible sizes of input labels and points: ", msg)

  def release(cls): 
   if cls.__lbl_movable: cls.y.release()
   if cls.__mat_movable: cls.X.release()

  def debug_print(cls):
    cls.y.debug_print()
    cls.X.debug_print()

  def get(cls): return (cls.X, cls.y)
  def get_dtype(cls): return cls.__dtype
  def get_itype(cls): return cls.__itype
  def numRows(cls): return cls.__num_row
  def numCols(cls): return cls.__num_col
  def is_dense(cls): return cls.__isDense

  def __del__(cls):
    if FrovedisServer.isUP(): cls.release() 

class FrovedisFeatureData:
  "A python container for frovedis side data for unsupervised ML algorithms"

  def __init__(cls,mat,isDense=None,dense_kind=None):
    # decision making whether the converted data would be movable upon destruction
    if (isinstance(mat,FrovedisCRSMatrix) or 
        isinstance(mat,FrovedisRowmajorMatrix) or
        isinstance(mat,FrovedisColmajorMatrix)): cls.__mat_movable = False
    else: cls.__mat_movable = True

    tt = type(isDense).__name__
    if(tt != 'NoneType' and tt != 'bool'): 
      raise TypeError("Expected None or boolean for isDense, received: ", tt)
    if (tt == 'NoneType'): #decide from input data
      if(issparse(mat) or isinstance(mat,FrovedisCRSMatrix)): cls.__isDense = False
      else: cls.__isDense = True
    else: cls.__isDense = isDense

    if(cls.__isDense):
      tt = type(dense_kind).__name__
      if(tt != 'NoneType' and tt != 'str'): 
        raise TypeError("Expected None or String for dense_kind, received: ", tt)
      if (tt == 'NoneType'): dense_kind = 'rowmajor' #default dense type
      if(dense_kind == 'colmajor'):
        cls.X = FrovedisColmajorMatrix.asCMM(mat)
      elif(dense_kind == 'rowmajor'):
        cls.X = FrovedisRowmajorMatrix.asRMM(mat)
      else:
        raise ValueError("Supported dense kinds are either rowmajor or colmajor")
      cls.__dtype = cls.X.get_dtype()
      cls.__itype = 0 # not meaningful for dense matrix 
    else:
      cls.X = FrovedisCRSMatrix.asCRS(mat)
      cls.__dtype = cls.X.get_dtype()
      cls.__itype = cls.X.get_itype()

    cls.__num_row = cls.X.numRows()
    cls.__num_col = cls.X.numCols()

    if (cls.__dtype != DTYPE.FLOAT and cls.__dtype != DTYPE.DOUBLE):
      raise TypeError("Expected training data either of float or double type!")
    if (not cls.__isDense and (cls.__itype != DTYPE.INT and cls.__itype != DTYPE.LONG)):
      raise TypeError("Expected training sparse data itype to be either int or long!")

  def release(cls): 
   if cls.__mat_movable: cls.X.release()

  def debug_print(cls): cls.X.debug_print()

  def get(cls): return cls.X
  def get_dtype(cls): return cls.__dtype
  def get_itype(cls): return cls.__itype
  def numRows(cls): return cls.__num_row
  def numCols(cls): return cls.__num_col
  def is_dense(cls): return cls.__isDense

  def __del__(cls):
    if FrovedisServer.isUP(): cls.release() 
