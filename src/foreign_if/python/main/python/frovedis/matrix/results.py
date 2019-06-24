#!/usr/bin/env python

import numpy as np
from ..exrpc.rpclib import *
from ..exrpc.server import *
from .dense import FrovedisDenseMatrix

class GetrfResult:
   "A python container for holding pointers of Frovedis server side getrf results"

   def __init__(cls, dummy=None): # constructor
      cls.__mtype = None 
      cls.__ipiv_ptr = None
      cls.__info = None
      if dummy is not None: cls.load_dummy(dummy)

   def load_dummy(cls,dummy):
      cls.release()
      try:
         cls.__mtype = dummy['mtype']
         cls.__ipiv_ptr = dummy['dptr']
         cls.__info = dummy['info']
      except KeyError:
         raise TypeError("[INTERNAL ERROR] Invalid input encountered.")

   def release(cls):
      if cls.__mtype is not None:
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.release_ipiv(host,port,cls.mtype().encode('ascii'),cls.ipiv())
         excpt = rpclib.check_server_exception()
         if excpt["status"]: raise RuntimeError(excpt["info"]) 
         cls.__mtype = None 
         cls.__ipiv_ptr = None
         cls.__info = None

   def __del__(cls): # destructor
      if FrovedisServer.isUP(): cls.release()

   def mtype(cls):
      return cls.__mtype

   def ipiv(cls):
      return cls.__ipiv_ptr

   def stat(cls):
      return cls.__info

# A generic class for storing SVD results 
# of the type colmajor_matrix<float/double> or blockcyclic_matrix<float/double>
class svdResult:
   "A python container for holding pointers of Frovedis server side gesvd results"

   def __init__(cls, dummy=None, dtype=None): # constructor
      cls.__umat = None
      cls.__vmat = None
      cls.__svec = None
      cls.__k = None
      cls.__info = None
      if dtype is None: raise ValueError("Nonetype matrix can't be handled!")
      else: cls.__dtype = dtype
      if dummy is not None: cls.load_dummy(dummy)

   def load_dummy(cls, dummy):
      cls.release()
      try:
         mtype = dummy['mtype']
         uptr = dummy['uptr']
         vptr = dummy['vptr']
         sptr = dummy['sptr']
         info = dummy['info']
         m = dummy['m']
         n = dummy['n']
         k = dummy['k']
         if uptr != 0:
           dmat = {'dptr': uptr, 'nrow': m, 'ncol': k}
           cls.__umat = FrovedisDenseMatrix(mtype,dmat,dtype=cls.__dtype)
         if vptr != 0:
           dmat = {'dptr': vptr, 'nrow': n, 'ncol': k}
           cls.__vmat = FrovedisDenseMatrix(mtype,dmat,dtype=cls.__dtype)
         cls.__k = k 
         cls.__svec = sptr
         cls.__info = info
      except KeyError:
         raise TypeError("[INTERNAL ERROR] Invalid input encountered.")

   def debug_print(cls):
      if cls.__svec is not None:
        (host,port) = FrovedisServer.getServerInstance()
        if(cls.__dtype == np.float64):
          svec = np.zeros(cls.getK(),dtype=np.float64)
          rpclib.get_double_array(host, port, cls.__svec, svec)
        elif(cls.__dtype == np.float32):
          svec = np.zeros(cls.getK(),dtype=np.float32)
          rpclib.get_float_array(host, port, cls.__svec, svec)
        else: raise TypeError("Invalid dtype encountered. Expects float/double.")
        excpt = rpclib.check_server_exception()
        if excpt["status"]: raise RuntimeError(excpt["info"]) 
        print ("svec: ", svec)
        if cls.__umat is not None: 
          print ("umat: ")
          cls.__umat.get_rowmajor_view()
        if cls.__vmat is not None: 
          print ("vmat: ")
          cls.__vmat.get_rowmajor_view()

   def to_numpy_results(cls):
      if cls.__svec is not None:
        umat = None
        vmat = None
        (host,port) = FrovedisServer.getServerInstance()
        if(cls.__dtype == np.float64):
          svec = np.zeros(cls.getK(),dtype=np.float64)
          rpclib.get_double_array(host, port, cls.__svec, svec)
        elif(cls.__dtype == np.float32):
          svec = np.zeros(cls.getK(),dtype=np.float32)
          rpclib.get_float_array(host, port, cls.__svec, svec)
        else: raise TypeError("Invalid dtype encountered. Expects float/double.")
        excpt = rpclib.check_server_exception()
        if excpt["status"]: raise RuntimeError(excpt["info"]) 
        if cls.__umat is not None: 
          umat = cls.__umat.to_numpy_matrix() 
        if cls.__vmat is not None: 
          vmat = cls.__vmat.to_numpy_matrix().T 
        return (umat,svec,vmat)
      else: raise ValueError("Empty input matrix.")

   def save(cls,sfl,ufl=None,vfl=None):
      if cls.__svec is not None:
        if sfl is None: raise ValueError("s_filename can't be None")
        (host,port) = FrovedisServer.getServerInstance()
        if(cls.__dtype == np.float32): dtype = 'F'
        elif(cls.__dtype == np.float64): dtype = 'D'
        else: raise TypeError("Invalid dtype, expected double/float.")
        rpclib.save_as_diag_matrix(host, port, cls.__svec,
                                   sfl.encode('ascii'), False,
                                   dtype.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]: raise RuntimeError(excpt["info"]) 
        wantU = cls.__umat is not None and ufl is not None
        wantV = cls.__vmat is not None and vfl is not None
        if wantU == True: cls.__umat.save(ufl)
        if wantV == True: cls.__vmat.save(vfl)
         
   def save_binary(cls,sfl,ufl=None,vfl=None):
      if cls.__svec is not None:
        if sfl is None: raise ValueError("s_filename can't be None")
        (host,port) = FrovedisServer.getServerInstance()
        if(cls.__dtype == np.float32): dtype = 'F'
        elif(cls.__dtype == np.float64): dtype = 'D'
        else: raise TypeError("Invalid dtype, expected double/float.")
        rpclib.save_as_diag_matrix(host, port, cls.__svec,
                                   sfl.encode('ascii'), True,
                                   dtype.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]: raise RuntimeError(excpt["info"]) 
        wantU = cls.__umat is not None and ufl is not None
        wantV = cls.__vmat is not None and vfl is not None
        if wantU == True: cls.__umat.save_binary(ufl)
        if wantV == True: cls.__vmat.save_binary(vfl)
         
   def load(cls,sfl,ufl=None,vfl=None,mtype='B'):
      cls.__load_impl(sfl,ufl,vfl,mtype,False)

   def load_binary(cls,sfl,ufl=None,vfl=None,mtype='B'):
      cls.__load_impl(sfl,ufl,vfl,mtype,True)

   def __load_impl(cls,sfl,ufl,vfl,mtype,bin):
      if sfl is None: raise ValueError("s_filename can't be None")
      if mtype != 'B' and mtype != 'C':
        raise ValueError("Unknown matrix type. Expected: (B or C), Got: ", mtype)
      if(cls.__dtype == np.float32): dtype = 'F'
      elif(cls.__dtype == np.float64): dtype = 'D'
      else: raise TypeError("Invalid dtype, expected double/float.")
      wantU = ufl is not None   
      wantV = vfl is not None   
      if wantU == False: ufl = ''
      if wantV == False: vfl = ''
      (host,port) = FrovedisServer.getServerInstance()
      dummy = rpclib.get_svd_results_from_file(host, port,
                                               sfl.encode('ascii'),
                                               ufl.encode('ascii'),
                                               vfl.encode('ascii'),
                                               bin, wantU, wantV,
                                               mtype.encode('ascii'),
                                               dtype.encode('ascii'))
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"]) 
      cls.load_dummy(dummy)

   def release(cls):
      if cls.__svec is not None:
         (host,port) = FrovedisServer.getServerInstance()
         if(cls.__dtype == np.float64):
           rpclib.release_double_array(host,port,cls.__svec)
         elif(cls.__dtype == np.float32):
           rpclib.release_float_array(host,port,cls.__svec)
         else: raise TypeError("Invalid dtype, Expected double/float.")
         excpt = rpclib.check_server_exception()
         if excpt["status"]: raise RuntimeError(excpt["info"]) 
         if cls.__umat is not None: cls.__umat.release()
         if cls.__vmat is not None: cls.__vmat.release()
         cls.__umat = None
         cls.__vmat = None
         cls.__svec = None
         cls.__k = None
         cls.__info = None
      
   def __del__(cls): # destructor
      if FrovedisServer.isUP(): cls.release()

   def stat(cls):
      return cls.__info
   
   def getK(cls):
      return cls.__k

