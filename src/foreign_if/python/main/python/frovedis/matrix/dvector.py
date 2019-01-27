#!/usr/bin/env python

from ..exrpc.rpclib import *
from ..exrpc.server import *
from .dtype import TypeUtil,DTYPE

class FrovedisDvector:
   
   "A python container for holding Frovedis server side dvector<double> pointers"
   def __init__(cls,vec=None,dtype=None): # constructor
      cls.__dtype = dtype
      cls.__fdata = None
      cls.__size = 0
      if vec is not None: cls.load(vec)
  
   def load(cls,inp):
      if isinstance(inp,dict):
         return cls.load_dummy(inp)
      else:
         if type(inp).__name__ == 'instance' or type(inp).__name__ == 'str':
            raise TypeError("Unsupported input encountered: " + str(type(inp)))
         if cls.__dtype is not None: vec = np.asarray(inp,dtype=cls.__dtype)
         else: vec = np.asarray(inp)
         return cls.load_numpy_array(vec)

   def load_numpy_array(cls,vec):
      cls.release()
      if(vec.ndim > 1):
         raise ValueError("Input dimension is more than 1 (Expect: Array, Got: Matrix)")
      if cls.__dtype is None: cls.__dtype = vec.dtype
      else: vec = np.asarray(vec,dtype=cls.__dtype)
      vv = vec.T # returns self, since ndim=1
      sz = vec.size
      (host, port) = FrovedisServer.getServerInstance()
      dt = cls.get_dtype()
      if (dt == DTYPE.INT): 
        dvec = rpclib.create_frovedis_int_dvector(host,port,vv,sz)
      elif (dt == DTYPE.LONG): 
        dvec = rpclib.create_frovedis_long_dvector(host,port,vv,sz)
      elif (dt == DTYPE.FLOAT): 
        dvec = rpclib.create_frovedis_float_dvector(host,port,vv,sz)
      elif (dt == DTYPE.DOUBLE): 
        dvec = rpclib.create_frovedis_double_dvector(host,port,vv,sz)
      elif (dt == DTYPE.STRING): 
        ptr_arr = (c_char_p * sz)()
        vv = np.array([e.encode('ascii')  for e in vv])
        ptr_arr[:] = vv
        dvec = rpclib.create_frovedis_string_dvector(host,port,ptr_arr,sz)
      else:
        raise TypeError("Unsupported dtype is specified for dvector creation!")
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"]) 
      return cls.load_dummy(dvec) 

   def load_dummy(cls,dvec):
      cls.release()
      try:
         cls.__fdata = (dvec['dptr'])
         cls.__size = (dvec['size'])
         cls.__dtype = TypeUtil.to_numpy_dtype(dvec['vtype']) #Must be added from c++ side
      except KeyError:
         raise TypeError("[INTERNAL ERROR] Invalid input encountered.")
      return cls
   
   def release(cls):
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.release_frovedis_dvector(host,port,cls.get(),cls.get_dtype())
         excpt = rpclib.check_server_exception()
         if excpt["status"]: raise RuntimeError(excpt["info"]) 
         cls.__fdata = None
         cls.__dtype = None
         cls.__size = 0
      
   def debug_print(cls):
      if cls.__fdata is not None:
         (host, port) = FrovedisServer.getServerInstance()
         rpclib.show_frovedis_dvector(host,port,cls.get(),cls.get_dtype())
         excpt = rpclib.check_server_exception()
         if excpt["status"]: raise RuntimeError(excpt["info"]) 
   
   def get(cls):
      return cls.__fdata
  
   def size(cls):
      return cls.__size

   def get_dtype(cls):
      return TypeUtil.to_id_dtype(cls.__dtype)

   #def __del__(cls): # destructor
   #   if FrovedisServer.isUP(): cls.release()

   @staticmethod
   def asDvec(vec):
      if isinstance(vec,FrovedisDvector): return vec
      else: return FrovedisDvector(vec)


class FrovedisIntDvector(FrovedisDvector):

  '''A python Container handles integer type dvector'''
  def __init__(cls,vec=None):
    FrovedisDvector.__init__(cls,vec=vec,dtype=np.int32)
    if vec is not None : cls.load(vec)
  
  @staticmethod
  def asDvec(vec):
    if isinstance(vec,FrovedisIntDvector): return vec
    else: return FrovedisIntDvector(vec)
  
class FrovedisLongDvector(FrovedisDvector):

  ''' A python Container handles long integer type dvector''' 
  def __init__(cls,vec=None):
    FrovedisDvector.__init__(cls,vec=vec,dtype=np.int64)
    if vec is not None : cls.load(vec)

  @staticmethod
  def asDvec(vec):
    if isinstance(vec,FrovedisLongDvector): return vec
    else: return FrovedisLongDvector(vec)
  
class FrovedisFloatDvector(FrovedisDvector):

  '''A Python container contains float type dvector'''
  def __init__(cls,vec=None):
    FrovedisDvector.__init__(cls,vec=vec,dtype=np.float32)
    if vec is not None : cls.load(vec)

  @staticmethod
  def asDvec(vec):
    if isinstance(vec,FrovedisFloatDvector): return vec
    else: return FrovedisFloatDvector(vec)

class FrovedisDoubleDvector(FrovedisDvector):

  '''A python Container handles double type dvector''' 
  def __init__(cls,vec=None):
    FrovedisDvector.__init__(cls,vec=vec,dtype=np.float64)
    if vec is not None : cls.load(vec)

  @staticmethod
  def asDvec(vec):
    if isinstance(vec,FrovedisDoubleDvector): return vec
    else: return FrovedisDoubleDvector(vec)

class FrovedisStringDvector(FrovedisDvector):

  '''A python Container handles string type dvector'''
  def __init__(cls,vec=None):
    FrovedisDvector.__init__(cls,vec=vec,dtype=np.dtype(str))
    if vec is not None : cls.load(vec)

  @staticmethod
  def asDvec(vec):
    if isinstance(vec,FrovedisStringDvector): return vec
    else: return FrovedisStringDvector(vec)
