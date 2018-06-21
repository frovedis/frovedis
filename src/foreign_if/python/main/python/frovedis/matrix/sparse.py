#!/usr/bin/env python

import numpy as np
from scipy.sparse import issparse, csr_matrix
from ..exrpc.rpclib import *
from ..exrpc.server import *

# dtype: Currently supports only double (numpy.float64) datatype
# Extension for new Frovedis sparse matrix is very simple.
# Just extend FrovedisSparseMatrix with a unique 'mtype'.
# All the matrices in this source code are distributed in nature.
# To support local version, it would be very easy. 
# Just extend FrovedisSparseMatrix with a unique 'mtype' in small case.
# e.g., FrovedisCRSMatrix => mtype: "R"
#       FrovedisCRSMatrixLocal => mtype: "r" (not yet supported)

class FrovedisSparseMatrix:
   "A python container for Frovedis server side sparse matrices of double type"

   def __init__(cls,mtype,mat=None): # constructor
      cls.__mtype = mtype
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
      elif isinstance(inp,str): #fname/dname
         return cls.load_text(inp)
      else: #any array-like object
         if type(inp).__name__ == 'instance': 
            raise TypeError, "Unsupported input encountered: " + str(type(inp))
         mat = np.asmatrix(inp,dtype=np.float64) #supports double type only
         mat = csr_matrix(mat)
         return cls.load_scipy_matrix(mat)

   def load_scipy_matrix(cls,mat):
      cls.release()
      (nrow, ncol) = mat.shape
      (vv, ii, oo) = (mat.data, mat.indices, mat.indptr)
      nelem = mat.data.size
      (host, port) = FrovedisServer.getServerInstance()
      dmat = rpclib.create_frovedis_sparse_matrix(host,port,nrow,ncol,
                                                vv,ii,oo,nelem,cls.__mtype)
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

   def load_text(cls,fname):
      cls.release()
      (host, port) = FrovedisServer.getServerInstance()
      dmat = rpclib.load_frovedis_sparse_matrix(host,port,fname,False,cls.__mtype)
      return cls.load_dummy(dmat)
     
   def load_binary(cls,fname):
      cls.release()
      (host, port) = FrovedisServer.getServerInstance()
      dmat = rpclib.load_frovedis_sparse_matrix(host,port,fname,True,cls.__mtype)
      return cls.load_dummy(dmat)

   def save(cls,fname):
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.save_frovedis_sparse_matrix(host,port,cls.get(),
                                         fname,False,cls.__mtype)

   def save_binary(cls,fname):
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.save_frovedis_sparse_matrix(host,port,cls.get(),
                                         fname,True,cls.__mtype)

   def release(cls):
      if cls.__fdata is not None:
         #print ("Releasing Frovedis sparse matrix of type: " + cls.__mtype)
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.release_frovedis_sparse_matrix(host,port,cls.get(),cls.__mtype)
         cls.__fdata = None
         cls.__num_row = 0
         cls.__num_col = 0

   def __del__(cls): # destructor
      if FrovedisServer.isUP(): cls.release()

   def debug_print(cls):
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.show_frovedis_sparse_matrix(host,port,cls.get(),cls.__mtype)

   def get(cls):
      return cls.__fdata

   def numRows(cls): 
      return cls.__num_row
         
   def numCols(cls): 
      return cls.__num_col


class FrovedisCRSMatrix(FrovedisSparseMatrix):
   "A python container for Frovedis server side crs_matrix<double,int,int>"

   def __init__(cls,mat=None):
      FrovedisSparseMatrix.__init__(cls,'R',mat) # "R" from "cRs(compressed Row sparse)"

   @staticmethod
   def asCRS(mat):
      if isinstance(mat, FrovedisCRSMatrix): return mat 
      else: return FrovedisCRSMatrix(mat)

