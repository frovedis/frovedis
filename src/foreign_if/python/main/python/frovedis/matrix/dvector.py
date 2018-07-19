#!/usr/bin/env python

from ..exrpc.rpclib import *
from ..exrpc.server import *
from dtype import *

class FrovedisDvector:
   
   "A python container for holding Frovedis server side dvector<double> pointers"
   def __init__(cls,vtype): # constructor
      cls.__fdata = None
      cls.__vtype = vtype
      cls.__size = 0
  
   def load_dummy(cls,dvec):
      cls.release()
      try:
         cls.__fdata = (dvec['dptr'])
         cls.__size = (dvec['size'])
         cls.__vtype = (dvec['vtype']) #Must be added from c++ side
      except KeyError:
         raise TypeError, "[INTERNAL ERROR] Invalid input encountered."
      return cls
   
   def release(cls):
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         #rpclib.release_frovedis_dvector(host,port,cls.get(),cls.__vtype)
         cls.__fdata = None
         cls.__size = 0
      
   def debug_print(cls):
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.show_frovedis_dvector(host,port,cls.get(),cls.__vtype) #added vtype as argument 
   
   def get(cls):
      return cls.__fdata
  
   def size(cls):
      return cls.__size
   def __del__(cls): # destructor
      if FrovedisServer.isUP(): cls.release()

class FrovedisIntDvector(FrovedisDvector):

  '''A python Container handles integer type dvector'''
  def __init__(cls,vec=None):
    FrovedisDvector.__init__(cls,DTYPE.INT)
    if vec is not None : cls.load(vec)
  
  def load(cls,inp):
      if isinstance(inp,dict):
         return cls.load_dummy(inp)
      else:
         if type(inp).__name__ == 'instance' or type(inp).__name__ == 'str':
            raise TypeError, "Unsupported input encountered: " + str(type(inp))
         vec = np.asarray(inp,dtype=np.int32)
         return cls.load_numpy_array(vec)

  def load_numpy_array(cls,vec):
      cls.release()
      if(vec.ndim > 1):
         raise ValueError, "Input dimension is more than 1 (Expect: Array, Got: Matrix)"
      vv = vec.T # returns self, since ndim=1
      sz = vec.size
      (host, port) = FrovedisServer.getServerInstance()
      dvec = rpclib.create_frovedis_int_dvector(host,port,vv,sz)
      return cls.load_dummy(dvec) 

  @staticmethod
  def asDvec(vec):
    if isinstance(vec,FrovedisIntDvector): return vec
    else: return FrovedisIntDvector(vec)
  
class FrovedisLongDvector(FrovedisDvector):

  ''' A python Container handles long integer type dvector''' 
  def __init__(cls,vec=None):
    FrovedisDvector.__init__(cls,DTYPE.LONG)
    if vec is not None : cls.load(vec)

  def load(cls,inp):
      if isinstance(inp,dict):
         return cls.load_dummy(inp)
      else:
         if type(inp).__name__ == 'instance' or type(inp).__name__ == 'str':
            raise TypeError, "Unsupported input encountered: " + str(type(inp))
         vec = np.asarray(inp,dtype=np.int64)
         return cls.load_numpy_array(vec)

  def load_numpy_array(cls,vec):
      cls.release()
      if(vec.ndim > 1):
         raise ValueError, "Input dimension is more than 1 (Expect: Array, Got: Matrix)"
      vv = vec.T # returns self, since ndim=1
      sz = vec.size
      (host, port) = FrovedisServer.getServerInstance()
      dvec = rpclib.create_frovedis_long_dvector(host,port,vv,sz)
      return cls.load_dummy(dvec)

  @staticmethod
  def asDvec(vec):
    if isinstance(vec,FrovedisLongDvector): return vec
    else: return FrovedisLongDvector(vec)
  
class FrovedisFloatDvector(FrovedisDvector):

  '''A Python container contains float type dvector'''
  def __init__(cls,vec=None):
    FrovedisDvector.__init__(cls,DTYPE.FLOAT)
    if vec is not None : cls.load(vec)

  def load(cls,inp):
      if isinstance(inp,dict):
         return cls.load_dummy(inp)
      else:
         if type(inp).__name__ == 'instance' or type(inp).__name__ == 'str':
            raise TypeError, "Unsupported input encountered: " + str(type(inp))
         vec = np.asarray(inp,dtype=np.float32)
         return cls.load_numpy_array(vec)

  def load_numpy_array(cls,vec):
      cls.release()
      if(vec.ndim > 1):
         raise ValueError, "Input dimension is more than 1 (Expect: Array, Got: Matrix)"
      vv = vec.T # returns self, since ndim=1
      sz = vec.size
      (host, port) = FrovedisServer.getServerInstance()
      dvec = rpclib.create_frovedis_float_dvector(host,port,vv,sz)
      return cls.load_dummy(dvec)

  @staticmethod
  def asDvec(vec):
    if isinstance(vec,FrovedisFloatDvector): return vec
    else: return FrovedisFloatDvector(vec)

class FrovedisDoubleDvector(FrovedisDvector):

  '''A python Container handles double type dvector''' 
  def __init__(cls,vec=None):
    FrovedisDvector.__init__(cls,DTYPE.DOUBLE)
    if vec is not None : cls.load(vec)

  def load(cls,inp):
      if isinstance(inp,dict):
         return cls.load_dummy(inp)
      else:
         if type(inp).__name__ == 'instance' or type(inp).__name__ == 'str':
            raise TypeError, "Unsupported input encountered: " + str(type(inp))
         vec = np.asarray(inp,dtype=np.float64)
         return cls.load_numpy_array(vec)

  def load_numpy_array(cls,vec):
      cls.release()
      if(vec.ndim > 1):
         raise ValueError, "Input dimension is more than 1 (Expect: Array, Got: Matrix)"
      vv = vec.T # returns self, since ndim=1
      sz = vec.size
      (host, port) = FrovedisServer.getServerInstance()
      dvec = rpclib.create_frovedis_double_dvector(host,port,vv,sz)
      return cls.load_dummy(dvec)

  @staticmethod
  def asDvec(vec):
    if isinstance(vec,FrovedisDoubleDvector): return vec
    else: return FrovedisDoubleDvector(vec)

class FrovedisStringDvector(FrovedisDvector):

  '''A python Container handles string type dvector'''
  def __init__(cls,vec=None):
    FrovedisDvector.__init__(cls,DTYPE.STRING)
    if vec is not None : cls.load(vec)

  def load(cls,inp):
      if isinstance(inp,dict):
         return cls.load_dummy(inp)
      else:
         if type(inp).__name__ == 'instance' or type(inp).__name__ == 'str':
            raise TypeError, "Unsupported input encountered: " + str(type(inp))
         vec = np.asarray(inp) #dtype=np.str
         return cls.load_numpy_array(vec)

  def load_numpy_array(cls,vec):
      cls.release()
      if(vec.ndim > 1):
         print "This is more than one dimension"
         raise ValueError, "Input dimension is more than 1 (Expect: Array, Got: Matrix)"
      vv = vec.T # returns self, since ndim=1
      sz = vec.size
      (host, port) = FrovedisServer.getServerInstance()
      ptr_arr = (c_char_p * sz)()
      ptr_arr[:] = vv
      dvec = rpclib.create_frovedis_string_dvector(host,port,ptr_arr,sz)
      return cls.load_dummy(dvec)

  @staticmethod
  def asDvec(vec):
    if isinstance(vec,FrovedisStringDvector): return vec
    else: return FrovedisStringDvector(vec)
