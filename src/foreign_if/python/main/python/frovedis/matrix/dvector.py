#!/usr/bin/env python

import numpy as np
from ..exrpc.rpclib import *
from ..exrpc.server import *

# dtype: Currently supports only double (numpy.float64) datatype
class FrovedisDvector:
   "A python container for holding Frovedis server side dvector<double> pointers"

   def __init__(cls, vec=None): # constructor
      cls.__fdata = None
      cls.__size = 0
      if vec is not None: cls.load(vec)

   def load(cls,inp):
      if isinstance(inp,dict): #dummy_mat
         return cls.load_dummy(inp)
      else: #any array-like object
         if type(inp).__name__ == 'instance' or type(inp).__name__ == 'str':
           raise TypeError, "Unsupported input encountered: " + str(type(inp))
         vec = np.asarray(inp,dtype=np.float64) #supports double type only
         return cls.load_numpy_array(vec)

   def load_numpy_array(cls,vec):
      cls.release()
      if(vec.ndim > 1): 
         raise ValueError, "Input dimension is more than 1 (Expect: Array, Got: Matrix)"
      vv = vec.T # returns self, since ndim=1
      sz = vec.size
      (host, port) = FrovedisServer.getServerInstance()
      dvec = rpclib.create_frovedis_dvector(host,port,vv,sz)
      return cls.load_dummy(dvec)

   def load_dummy(cls,dvec):
      cls.release()
      try:
         cls.__fdata = dvec['dptr']
         cls.__size = dvec['size']
      except KeyError:
         raise TypeError, "[INTERNAL ERROR] Invalid input encountered."
      return cls

   def release(cls):
      if cls.__fdata is not None:
         #print ("Releasing Frovedis dvector")
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.release_frovedis_dvector(host,port,cls.get())
         cls.__fdata = None
         cls.__size = 0

   def __del__(cls): # destructor
      if FrovedisServer.isUP(): cls.release()

   def debug_print(cls):
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.show_frovedis_dvector(host,port,cls.get())

   def get(cls):
      return cls.__fdata

   def size(cls):
      return cls.__size

   @staticmethod
   def asDvec(vec):
      if isinstance(vec,FrovedisDvector): return vec
      else: return FrovedisDvector(vec)
     

