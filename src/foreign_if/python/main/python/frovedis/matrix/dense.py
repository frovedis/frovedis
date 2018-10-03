#!/usr/bin/env python

import numpy as np
from ..exrpc.rpclib import *
from ..exrpc.server import *
from dtype import TypeUtil,DTYPE

# dtype: Currently supports float/double/int/long datatypes.
# Extension for new Frovedis dense matrix is very simple.
# Just extend FrovedisDenseMatrix with a unique 'mtype'.
# All the matrices in this source code are distributed in nature.
# To support local version, it would be very easy. 
# Just extend FrovedisDenseMatrix with a unique 'mtype' in small case.
# e.g., FrovedisRowmajorMatrix => mtype: "R"
#       FrovedisRowmajorMatrixLocal => mtype: "r" (not yet supported)

class FrovedisDenseMatrix:
   "A python container for Frovedis server side dense matrices"

   def __init__(cls,mtype,mat=None,dtype=None): # constructor
      if(mtype == 'B' and (dtype == np.int32 or dtype == np.int64)): 
         raise TypeError("Long/Integer type is not supported for blockcyclic matrix!")
      cls.__mtype = mtype
      cls.__dtype = dtype
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
      else: return cls.load_python_data(inp)  #any array-like object
        
   def load_python_data(cls,inp):
      support = ['matrix', 'list', 'ndarray', 'tuple', 'DataFrame']
      if type(inp).__name__ not in support:
        raise TypeError("Unsupported input encountered: " + str(type(inp)))       
      if cls.__dtype is None: mat = np.asmatrix(inp) 
      else: mat = np.asmatrix(inp,cls.__dtype)  
      return cls.load_numpy_matrix(mat)

   def load_numpy_matrix(cls,mat):
      cls.release()
      if cls.__dtype is None: cls.__dtype = mat.dtype
      else: mat = np.asmatrix(mat,cls.__dtype)
      vv = mat.A1
      (nrow, ncol) = mat.shape
      (host, port) = FrovedisServer.getServerInstance()
      dt = cls.get_dtype()
      if(dt == DTYPE.DOUBLE):
         dmat = rpclib.create_frovedis_double_dense_matrix(host,port,           
                                                nrow,ncol,vv,cls.__mtype)
      elif(dt == DTYPE.FLOAT):
         dmat = rpclib.create_frovedis_float_dense_matrix(host,port,
                                               nrow,ncol,vv,cls.__mtype)
      elif(dt == DTYPE.LONG):
         dmat = rpclib.create_frovedis_long_dense_matrix(host,port,
                                               nrow,ncol,vv,cls.__mtype)
      elif(dt == DTYPE.INT):
         dmat = rpclib.create_frovedis_int_dense_matrix(host,port,
                                               nrow,ncol,vv,cls.__mtype)
      else: raise TypeError("Unsupported input type: " + cls.__dtype)   
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"])
      return cls.load_dummy(dmat)

   def load_dummy(cls,dmat):
      cls.release()
      try:
         cls.__fdata = dmat['dptr']
         cls.__num_row = dmat['nrow']
         cls.__num_col = dmat['ncol']
      except KeyError:
         raise TypeError("[INTERNAL ERROR] Invalid input encountered.")
      return cls

   def copy(cls,mat): #cls = mat 
      cls.release()
      if cls.__dtype is None: cls.__dtype = mat.__dtype
      if (mat.__mtype != cls.__mtype or mat.__dtype != cls.__dtype): 
        raise TypeError("Incompatible types for copy operation")
      if mat.__fdata is not None:
        (host, port) = FrovedisServer.getServerInstance()
        dmat = rpclib.copy_frovedis_dense_matrix(host,port,mat.get(),
                                            mat.__mtype,mat.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]: raise RuntimeError(excpt["info"]) 
        return cls.load_dummy(dmat)

   def load_text(cls,fname):
      cls.release()
      (host, port) = FrovedisServer.getServerInstance()
      if cls.__dtype is None: cls.__dtype = np.float32 # default type: float
      dmat = rpclib.load_frovedis_dense_matrix(host,port,fname,False,
                                               cls.__mtype,cls.get_dtype())
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"]) 
      return cls.load_dummy(dmat)
      
   def load_binary(cls,fname):
      cls.release()
      (host, port) = FrovedisServer.getServerInstance()
      if cls.__dtype is None: cls.__dtype = np.float32 # default type: float
      dmat = rpclib.load_frovedis_dense_matrix(host,port,fname,True,
                                               cls.__mtype,cls.get_dtype())
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"]) 
      return cls.load_dummy(dmat)

   def save(cls,fname):
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.save_frovedis_dense_matrix(host,port,cls.get(),
                                         fname,False,cls.__mtype,cls.get_dtype())
         excpt = rpclib.check_server_exception()
         if excpt["status"]: raise RuntimeError(excpt["info"]) 
   
   def save_binary(cls,fname):
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.save_frovedis_dense_matrix(host,port,cls.get(),
                                         fname,True,cls.__mtype,cls.get_dtype())
         excpt = rpclib.check_server_exception()
         if excpt["status"]: raise RuntimeError(excpt["info"]) 

   def release(cls):
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.release_frovedis_dense_matrix(host,port,cls.get(),
                                              cls.__mtype,cls.get_dtype())
         excpt = rpclib.check_server_exception()
         if excpt["status"]: raise RuntimeError(excpt["info"]) 
         cls.__fdata = None
         cls.__num_row = 0
         cls.__num_col = 0

   def __del__(cls): # destructor
      if FrovedisServer.isUP(): cls.release()

   def debug_print(cls):
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.show_frovedis_dense_matrix(host,port,cls.get(),
                                           cls.__mtype,cls.get_dtype())
         excpt = rpclib.check_server_exception()
         if excpt["status"]: raise RuntimeError(excpt["info"]) 

   def to_numpy_matrix(cls):
      if cls.__fdata is not None:
         sz = cls.numRows() * cls.numCols()
         dt = cls.get_dtype()
         (host, port) = FrovedisServer.getServerInstance()
         arr = np.zeros(sz,dtype=cls.__dtype)
         if(dt == DTYPE.DOUBLE):
            rpclib.get_double_rowmajor_array(host,port,cls.get(),cls.__mtype,arr,sz)
         elif(dt == DTYPE.FLOAT):
            rpclib.get_float_rowmajor_array(host,port,cls.get(),cls.__mtype,arr,sz)
         elif(dt == DTYPE.LONG):
            rpclib.get_long_rowmajor_array(host,port,cls.get(),cls.__mtype,arr,sz)
         elif(dt == DTYPE.INT):
            rpclib.get_int_rowmajor_array(host,port,cls.get(),cls.__mtype,arr,sz)
         else: raise TypeError("Unsupported input type: " + dt)
         excpt = rpclib.check_server_exception()
         if excpt["status"]: raise RuntimeError(excpt["info"])
         mat = np.asmatrix(arr)
         mat = mat.reshape(cls.numRows(),cls.numCols())
         return mat

   def to_frovedis_rowmatrix(cls):
      if cls.__mtype == 'R': return cls
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         dmat = rpclib.get_frovedis_rowmatrix(host,port,cls.get(),
                                              cls.numRows(), cls.numCols(),
                                              cls.__mtype,cls.get_dtype())
         excpt = rpclib.check_server_exception()
         if excpt["status"]: raise RuntimeError(excpt["info"]) 
         return FrovedisDenseMatrix(mtype='R',mat=dmat,dtype=cls.__dtype)
      else: raise ValueError("Empty input matrix.")

   def get_rowmajor_view(cls):
      if cls.__fdata is not None: cls.to_frovedis_rowmatrix().debug_print()

   def transpose(cls):
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         dmat = rpclib.transpose_frovedis_dense_matrix(host,port,
                                         cls.get(),cls.__mtype,cls.get_dtype())
         excpt = rpclib.check_server_exception()
         if excpt["status"]: raise RuntimeError(excpt["info"]) 
         return FrovedisDenseMatrix(mtype=cls.__mtype,mat=dmat,dtype=cls.__dtype)
      else: raise ValueError("Empty input matrix.")

   def get(cls):
      return cls.__fdata

   def numRows(cls): 
      return cls.__num_row
         
   def numCols(cls): 
      return cls.__num_col

   def get_dtype(cls):
     return TypeUtil.to_id_dtype(cls.__dtype)

   def get_mtype(cls): return cls.__mtype

class FrovedisRowmajorMatrix(FrovedisDenseMatrix):
   "A python container for Frovedis server side rowmajor_matrix"

   def __init__(cls, mat=None, dtype=None):
      FrovedisDenseMatrix.__init__(cls,'R',mat,dtype)

   # 'mat' can be either FrovedisRowmajorMatrix or any array-like object
   # FrovedisDenseMatrix inter-conversion is not yet supported (TODO)
   @staticmethod
   def asRMM(mat):
      if isinstance(mat,FrovedisRowmajorMatrix): return mat
      elif (isinstance(mat,FrovedisDenseMatrix) and 
            (mat.get_mtype() == 'R')): return mat
      else: return FrovedisRowmajorMatrix().load_python_data(mat)


class FrovedisColmajorMatrix(FrovedisDenseMatrix):
   "A python container for Frovedis server side colmajor_matrix"

   def __init__(cls, mat=None, dtype=None):
      FrovedisDenseMatrix.__init__(cls,'C',mat,dtype)

   # 'mat' can be either FrovedisColmajorMatrix or any array-like object
   # FrovedisDenseMatrix inter-conversion is not yet supported (TODO)
   @staticmethod
   def asCMM(mat):
      if isinstance(mat,FrovedisColmajorMatrix): return mat
      elif (isinstance(mat,FrovedisDenseMatrix) and 
            (mat.get_mtype() == 'C')): return mat
      else: return FrovedisColmajorMatrix().load_python_data(mat)
    
from results import GetrfResult
class FrovedisBlockcyclicMatrix(FrovedisDenseMatrix):
   "A python container for Frovedis server side blockcyclic_matrix"

   def __init__(cls, mat=None, dtype=None):
      if(dtype == np.int32 or dtype == np.int64):
        raise TypeError("Unsupported dtype for blockcyclic matrix creation!")
      FrovedisDenseMatrix.__init__(cls,'B',mat,dtype)

   def __add__(cls,mat): # returns( cls + mat)
      (host, port) = FrovedisServer.getServerInstance()
      # tmp = cls + tmp(=mat)
      tmp = FrovedisBlockcyclicMatrix(mat=mat) #copy 
      # geadd performs B = al*A + be*B, thus tmp = B and tmp = A + tmp
      rpclib.pgeadd(host,port,cls.get(),tmp.get(),False,1.0,1.0)
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"]) 
      return tmp 

   def __sub__(cls,mat): # returns (cls - mat)
      (host, port) = FrovedisServer.getServerInstance()
      # tmp = cls - tmp(=mat)
      tmp = FrovedisBlockcyclicMatrix(mat=mat) #copy
      # geadd performs B = al*A + be*B, thus tmp = B and tmp = A - tmp
      rpclib.pgeadd(host,port,cls.get(),tmp.get(),False,1.0,-1.0)
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"]) 
      return tmp 

   def __mul__(cls,mat): # returns (cls * mat)
      tmp = FrovedisBlockcyclicMatrix.asBCM(mat) 
      (host, port) = FrovedisServer.getServerInstance()
      dmat = rpclib.pgemm(host,port,cls.get(),tmp.get(),False,False,1.0,0.0)
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"])
      dt = TypeUtil.to_numpy_dtype(cls.get_dtype()) 
      return FrovedisBlockcyclicMatrix(mat=dmat,dtype=dt)

   def __invert__(cls): # returns transpose (~cls)
      return cls.transpose()

   def inv(cls): # returns inverse of self
      ret = FrovedisBlockcyclicMatrix(mat=cls) # ret = cls
      (host,port) = FrovedisServer.getServerInstance()
      rf = GetrfResult(rpclib.pgetrf(host,port,ret.get()))
      rpclib.pgetri(host,port,ret.get(),rf.ipiv()) # ret = inv(ret)
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"]) 
      rf.release()
      return ret

   # 'mat' can be either FrovedisBlockcyclicMatrix or any array-like object
   # FrovedisDenseMatrix inter-conversion is not yet supported (TODO)
   @staticmethod
   def asBCM(mat):
      if isinstance(mat,FrovedisBlockcyclicMatrix): return mat
      elif (isinstance(mat,FrovedisDenseMatrix) and 
            (mat.get_mtype() == 'B')): return mat
      else: return FrovedisBlockcyclicMatrix().load_python_data(mat)




