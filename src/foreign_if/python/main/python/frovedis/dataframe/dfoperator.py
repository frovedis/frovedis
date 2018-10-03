#!/usr/bin/env python

from ..exrpc.rpclib import *
from ..exrpc.server import *

class dfoperator:
  def __init__(cls, proxy):
    cls.__proxy = proxy

  #def __del__(cls):
  #  if FrovedisServer.isUP(): cls.release()

  def release(cls):
    if cls.__proxy is not None:
      (host, port) = FrovedisServer.getServerInstance()
      rpclib.release_dfoperator(host,port,cls.get())
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"]) 
      cls.__proxy = None

  def __and__(cls, opt):
    if cls.__proxy is not None:
      (host, port) = FrovedisServer.getServerInstance()
      proxy = rpclib.get_dfANDoperator(host,port,cls.get(),opt.get());
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"]) 
      return dfoperator(proxy)
  
  def __or__(cls, opt):
    if cls.__proxy is not None:
      (host, port) = FrovedisServer.getServerInstance()
      proxy = rpclib.get_dfORoperator(host,port,cls.get(),opt.get());
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"]) 
      return dfoperator(proxy)

  def get(cls):
    return cls.__proxy

