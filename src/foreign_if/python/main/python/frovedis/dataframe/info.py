#!/usr/bin/env python

from ..exrpc.rpclib import *
from ..exrpc.server import *
from ..mllib.model_util import ModelID

class df_to_sparse_info:

  '''A python container for holding information related to dataframe to sparse conversion'''

  def __init__(cls,info_id):
    cls.__uid = info_id

  def load(cls,dirname):
    cls.release()
    if (type(dirname).__name__ != 'str'):
      raise TypeError("Expected String, Found: " + type(dirname).__name__)
    info_id = ModelID.get() #getting unique id for conversion info to be registered
    (host, port) = FrovedisServer.getServerInstance()
    rpclib.load_dftable_to_sparse_info(host,port,info_id,dirname.encode('ascii'))
    cls.__uid = info_id
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
    return cls

  def save(cls,dirname):
    if cls.__uid is None:
      raise ValueError("Operation on invalid frovedis dftable_to_sparse_info!")
    if (type(dirname).__name__ != 'str'):
      raise TypeError("Expected String, Found: " + type(dirname).__name__)
    (host, port) = FrovedisServer.getServerInstance()
    rpclib.save_dftable_to_sparse_info(host,port,cls.get(),dirname.encode('ascii'))
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])

  def release(cls):
    if cls.__uid is None:
      raise ValueError("Operation on invalid frovedis dftable_to_sparse_info!")
    (host, port) = FrovedisServer.getServerInstance()
    rpclib.release_dftable_to_sparse_info(host,port,cls.get())
    cls.__uid = None

  def get(cls): return cls.__uid
    
