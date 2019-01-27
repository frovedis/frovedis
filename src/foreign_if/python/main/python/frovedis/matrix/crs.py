#!/usr/bin/env python

import numpy as np
from scipy.sparse import issparse, csr_matrix
from ..exrpc.rpclib import *
from ..exrpc.server import *
from .dtype import TypeUtil, DTYPE

class FrovedisCRSMatrix:
   "A python container for Frovedis server side crs_matrix"

   def __init__(cls,mat=None,dtype=None,itype=None): # constructor
      cls.__dtype = dtype
      cls.__itype = itype
      cls.__fdata = None
      cls.__num_row = 0 
      cls.__num_col = 0 
      if mat is not None: cls.load(mat)

   def load(cls,inp):
      if issparse(inp): #any sparse matrix
         mat = inp.tocsr()
         return cls.load_scipy_matrix(mat)
      elif isinstance(inp,dict): #dummy_matrix
         return cls.load_dummy(inp)
      elif isinstance(inp,str):  #expects text file name
         return cls.load_text(inp)
      else: return cls.load_python_data(inp)

   def load_dummy(cls,dmat):
      cls.release()
      try:
         cls.__fdata = dmat['dptr']
         cls.__num_row = dmat['nrow']
         cls.__num_col = dmat['ncol']
      except KeyError:
         raise TypeError("[INTERNAL ERROR] Invalid input encountered.")
      return cls

   def load_text(cls,fname):
      cls.release()
      (host, port) = FrovedisServer.getServerInstance()
      if cls.__dtype is None: cls.__dtype = np.float32 # default 'float' type data would be loaded
      cls.__set_or_validate_itype(np.int32) # default 'int' type index woule be loaded
      dmat = rpclib.load_frovedis_crs_matrix(host,port,fname.encode('ascii'),
                                             False,cls.get_dtype(),
                                             cls.get_itype())
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"]) 
      return cls.load_dummy(dmat)
     
   def load_binary(cls,fname):
      cls.release()
      (host, port) = FrovedisServer.getServerInstance()
      if cls.__dtype is None: cls.__dtype = np.float32 # default 'float' type data would be loaded
      cls.__set_or_validate_itype(np.int32) # default 'int' type index woule be loaded
      dmat = rpclib.load_frovedis_crs_matrix(host,port,fname.encode('ascii'),
                                             True,cls.get_dtype(),
                                             cls.get_itype())
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"]) 
      return cls.load_dummy(dmat)

   def load_python_data(cls,inp):
      support = ['matrix', 'list', 'ndarray', 'tuple', 'DataFrame']
      if type(inp).__name__ not in support:
        raise TypeError("Unsupported input encountered: " + str(type(inp)))
      if cls.__dtype is None: mat = csr_matrix(np.asmatrix(inp)) # loaded as input datatype
      else: mat = csr_matrix(np.asmatrix(inp),dtype=cls.__dtype) # loaded as user-given datatype
      return cls.load_scipy_matrix(mat)

   def load_scipy_matrix(cls,mat):
      cls.release()
      nelem = mat.data.size
      (nrow, ncol) = mat.shape
      (vv, ii, oo) = (mat.data, mat.indices, mat.indptr)
      if cls.__dtype is None: cls.__dtype = vv.dtype 
      else: vv = np.asarray(vv,cls.__dtype)
      if cls.__itype is not None: ii = np.asarray(ii,cls.__itype)
      cls.__set_or_validate_itype(ii.dtype)
      oo = np.asarray(oo,np.int64) # always size_t at frovedis server 
      ddt = cls.get_dtype()
      idt = cls.get_itype()
      (host, port) = FrovedisServer.getServerInstance()
      if(ddt == DTYPE.INT and idt == DTYPE.INT):  
        dmat = rpclib.create_frovedis_crs_II_matrix(host,port,nrow,ncol,
                                                    vv,ii,oo,nelem)
      elif(ddt == DTYPE.INT and idt == DTYPE.LONG):  
        dmat = rpclib.create_frovedis_crs_IL_matrix(host,port,nrow,ncol,
                                                    vv,ii,oo,nelem)
      elif(ddt == DTYPE.LONG and idt == DTYPE.INT):  
        dmat = rpclib.create_frovedis_crs_LI_matrix(host,port,nrow,ncol,
                                                    vv,ii,oo,nelem)
      elif(ddt == DTYPE.LONG and idt == DTYPE.LONG):  
        dmat = rpclib.create_frovedis_crs_LL_matrix(host,port,nrow,ncol,
                                                    vv,ii,oo,nelem)
      elif(ddt == DTYPE.FLOAT and idt == DTYPE.INT):  
        dmat = rpclib.create_frovedis_crs_FI_matrix(host,port,nrow,ncol,
                                                    vv,ii,oo,nelem)
      elif(ddt == DTYPE.FLOAT and idt == DTYPE.LONG):  
        dmat = rpclib.create_frovedis_crs_FL_matrix(host,port,nrow,ncol,
                                                    vv,ii,oo,nelem)
      elif(ddt == DTYPE.DOUBLE and idt == DTYPE.INT):  
        dmat = rpclib.create_frovedis_crs_DI_matrix(host,port,nrow,ncol,
                                                    vv,ii,oo,nelem)
      elif(ddt == DTYPE.DOUBLE and idt == DTYPE.LONG):  
        dmat = rpclib.create_frovedis_crs_DL_matrix(host,port,nrow,ncol,
                                                    vv,ii,oo,nelem)
      else: raise TypeError("Unsupported dtype/itype for crs_matrix creation!")
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"])
      return cls.load_dummy(dmat)

   def save(cls,fname):
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.save_frovedis_crs_matrix(host,port,cls.get(),
                                         fname.encode('ascii'),
                                         False,cls.get_dtype(),cls.get_itype())
         excpt = rpclib.check_server_exception()
         if excpt["status"]: raise RuntimeError(excpt["info"]) 

   def save_binary(cls,fname):
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.save_frovedis_crs_matrix(host,port,cls.get(),
                                         fname.encode('ascii'),
                                         True,cls.get_dtype(),cls.get_itype())
         excpt = rpclib.check_server_exception()
         if excpt["status"]: raise RuntimeError(excpt["info"]) 

   def release(cls):
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.release_frovedis_crs_matrix(host,port,cls.get(),
                                            cls.get_dtype(),cls.get_itype())
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
         rpclib.show_frovedis_crs_matrix(host,port,cls.get(),
                                         cls.get_dtype(),cls.get_itype())
         excpt = rpclib.check_server_exception()
         if excpt["status"]: raise RuntimeError(excpt["info"]) 

   def get(cls):
      return cls.__fdata

   def numRows(cls): 
      return cls.__num_row
         
   def numCols(cls): 
      return cls.__num_col

   def get_dtype(cls):
     return TypeUtil.to_id_dtype(cls.__dtype)

   def get_itype(cls):
     return TypeUtil.to_id_dtype(cls.__itype)

   def __set_or_validate_itype(cls,dt):
      if cls.__itype is None: cls.__itype = dt   
      elif (cls.__itype != np.int32 and cls.__itype != np.int64):
        raise ValueError("Invalid type for crs indices: ", cls.__itype)

   @staticmethod
   def asCRS(mat):
      if isinstance(mat, FrovedisCRSMatrix): return mat 
      elif issparse(mat): #any sparse matrix
         smat = mat.tocsr()
         return FrovedisCRSMatrix().load_scipy_matrix(smat)
      else: return FrovedisCRSMatrix().load_python_data(mat)

