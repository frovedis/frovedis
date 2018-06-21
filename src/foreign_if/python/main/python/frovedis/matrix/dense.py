#!/usr/bin/env python

import numpy as np
from ..exrpc.rpclib import *
from ..exrpc.server import *

# dtype: Currently supports only double (numpy.float64) datatype
# Extension for new Frovedis dense matrix is very simple.
# Just extend FrovedisDenseMatrix with a unique 'mtype'.
# All the matrices in this source code are distributed in nature.
# To support local version, it would be very easy. 
# Just extend FrovedisDenseMatrix with a unique 'mtype' in small case.
# e.g., FrovedisRowmajorMatrix => mtype: "R"
#       FrovedisRowmajorMatrixLocal => mtype: "r" (not yet supported)

class FrovedisDenseMatrix:
   "A python container for Frovedis server side dense matrices of double type"

   def __init__(cls, mtype, mat=None): # constructor
      cls.__mtype = mtype
      cls.__fdata = None
      cls.__num_row = 0 
      cls.__num_col = 0 
      if mat is not None: cls.load(mat)
    
   def load(cls,inp):
      if isinstance(inp,dict): #dummy_mat
         return cls.load_dummy(inp)
      elif isinstance(inp,str): #fname/dname
         return cls.load_text(inp)
      elif isinstance(inp,FrovedisDenseMatrix): #copy cons
         return cls.copy(inp) 
      else: #any array-like object
         if type(inp).__name__ == 'instance': 
           raise TypeError, "Unsupported input encountered: " + str(type(inp))
         #asmatrix doesn't copy if the input is already a matrix or an ndarray
         mat = np.asmatrix(inp,dtype=np.float64) #supports double type only
         return cls.load_numpy_matrix(mat)

   # if input is a vector, it is loaded as a column-vector 
   # to support PBLAS operations and load balancing
   def load_numpy_matrix(cls,mat):
      cls.release()
      vv = mat.A1
      (nrow, ncol) = mat.shape
      # converting row-vector to column-vector
      if nrow == 1: (nrow, ncol) = (ncol,nrow)
      (host, port) = FrovedisServer.getServerInstance()
      dmat = rpclib.create_frovedis_dense_matrix(host,port,
                                               nrow,ncol,vv,cls.__mtype)
      return cls.load_dummy(dmat)

   def load_dummy(cls,dmat):
      cls.release()
      try:
         cls.__fdata = dmat['dptr']
         cls.__num_row = dmat['nrow']
         cls.__num_col = dmat['ncol']
      except KeyError:
         raise TypeError, "[INTERNAL ERROR] Invalid input encountered."
      return cls

   def copy(cls,mat): #cls = mat
      cls.release()
      if mat.__mtype != cls.__mtype: raise TypeError, "Incompatible types"
      if mat.__fdata is not None:
        (host, port) = FrovedisServer.getServerInstance()
        dmat = rpclib.copy_frovedis_dense_matrix(host,port,mat.get(),mat.__mtype)
        return cls.load_dummy(dmat)

   def load_text(cls,fname):
      cls.release()
      (host, port) = FrovedisServer.getServerInstance()
      dmat = rpclib.load_frovedis_dense_matrix(host,port,fname,False,cls.__mtype)
      return cls.load_dummy(dmat)
      
   def load_binary(cls,fname):
      cls.release()
      (host, port) = FrovedisServer.getServerInstance()
      dmat = rpclib.load_frovedis_dense_matrix(host,port,fname,True,cls.__mtype)
      return cls.load_dummy(dmat)

   def save(cls,fname):
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.save_frovedis_dense_matrix(host,port,cls.get(),
                                         fname,False,cls.__mtype)

   def save_binary(cls,fname):
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.save_frovedis_dense_matrix(host,port,cls.get(),
                                         fname,True,cls.__mtype)

   def release(cls):
      if cls.__fdata is not None:
         #print ("Releasing Frovedis dense matrix of type: " + cls.__mtype)
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.release_frovedis_dense_matrix(host,port,cls.get(),cls.__mtype)
         cls.__fdata = None
         cls.__num_row = 0
         cls.__num_col = 0

   def __del__(cls): # destructor
      if FrovedisServer.isUP(): cls.release()

   def debug_print(cls):
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.show_frovedis_dense_matrix(host,port,cls.get(),cls.__mtype)

   def to_numpy_matrix(cls):
      if cls.__fdata is not None:
         sz = cls.numRows() * cls.numCols()
         arr = np.zeros(sz,dtype=np.float64)
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.get_rowmajor_array(host,port,cls.get(),cls.__mtype,arr,sz)
         mat = np.asmatrix(arr)
         mat = mat.reshape(cls.numRows(),cls.numCols())
         return mat
      else: raise ValueError, "Empty input matrix."

   def to_frovedis_rowmatrix(cls):
      if cls.__mtype == 'R': return cls
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         dmat = rpclib.get_frovedis_rowmatrix(host,port,cls.get(),
                                            cls.numRows(), cls.numCols(),
                                            cls.__mtype)
         return FrovedisDenseMatrix('R',dmat)
      else: raise ValueError, "Empty input matrix."

   def get_rowmajor_view(cls):
      if cls.__fdata is not None: cls.to_frovedis_rowmatrix().debug_print()

   def transpose(cls):
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         dmat = rpclib.transpose_frovedis_dense_matrix(host,port,
                                                     cls.get(),cls.__mtype)
         return FrovedisDenseMatrix(cls.__mtype,dmat)
      else: raise ValueError, "Empty input matrix."

   def get(cls):
      return cls.__fdata

   def numRows(cls): 
      return cls.__num_row
         
   def numCols(cls): 
      return cls.__num_col


class FrovedisRowmajorMatrix(FrovedisDenseMatrix):
   "A python container for Frovedis server side rowmajor_matrix<double>"

   def __init__(cls, mat=None):
      FrovedisDenseMatrix.__init__(cls,'R',mat)

   # 'mat' can be either FrovedisRowmajorMatrix or any array-like object
   # FrovedisDenseMatrix inter-conversion is not yet supported (TODO)
   @staticmethod
   def asRMM(mat):
      if isinstance(mat,FrovedisRowmajorMatrix): return mat
      else: return FrovedisRowmajorMatrix(mat)


class FrovedisColmajorMatrix(FrovedisDenseMatrix):
   "A python container for Frovedis server side colmajor_matrix<double>"

   def __init__(cls, mat=None):
      FrovedisDenseMatrix.__init__(cls,'C',mat)

   # 'mat' can be either FrovedisColmajorMatrix or any array-like object
   # FrovedisDenseMatrix inter-conversion is not yet supported (TODO)
   @staticmethod
   def asCMM(mat):
      if isinstance(mat,FrovedisColmajorMatrix): return mat
      else: return FrovedisColmajorMatrix(mat)


from results import GetrfResult
class FrovedisBlockcyclicMatrix(FrovedisDenseMatrix):
   "A python container for Frovedis server side blockcyclic_matrix<double>"

   def __init__(cls, mat=None):
      FrovedisDenseMatrix.__init__(cls,'B',mat)

   def __add__(cls,mat): # returns( cls + mat)
      (host, port) = FrovedisServer.getServerInstance()
      # tmp = cls + tmp(=mat)
      tmp = FrovedisBlockcyclicMatrix(mat) #copy 
      rpclib.pgeadd(host,port,cls.get(),tmp.get(),False,1.0,1.0)
      return tmp 

   def __sub__(cls,mat): # returns (cls - mat)
      (host, port) = FrovedisServer.getServerInstance()
      # tmp = cls - tmp(=mat)
      tmp = FrovedisBlockcyclicMatrix(mat) #copy
      rpclib.pgeadd(host,port,cls.get(),tmp.get(),False,1.0,-1.0)
      return tmp 

   def __mul__(cls,mat): # returns (cls * mat)
      mat = FrovedisBlockcyclicMatrix.asBCM(mat) 
      (host, port) = FrovedisServer.getServerInstance()
      dmat = rpclib.pgemm(host,port,cls.get(),mat.get(),False,False,1.0,0.0)
      return FrovedisBlockcyclicMatrix(dmat)

   def __invert__(cls): # returns transpose (~cls)
      return cls.transpose()

   def inv(cls): # returns inverse of self
      ret = FrovedisBlockcyclicMatrix(cls) # ret = cls
      (host,port) = FrovedisServer.getServerInstance()
      rf = GetrfResult(rpclib.pgetrf(host,port,ret.get()))
      rpclib.pgetri(host,port,ret.get(),rf.ipiv()) # ret = inv(ret)
      rf.release()
      return ret

   # 'mat' can be either FrovedisBlockcyclicMatrix or any array-like object
   # FrovedisDenseMatrix inter-conversion is not yet supported (TODO)
   @staticmethod
   def asBCM(mat):
      if isinstance(mat,FrovedisBlockcyclicMatrix): return mat
      else: return FrovedisBlockcyclicMatrix(mat)


