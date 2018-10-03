#!/usr/bin/env python

import node, rpclib

class FrovedisServer(object):
   "A singleton implementation to store Frovedis server information"

   __instance = None
   __cmd = "mpirun -np 2 ../../server/frovedis_server" #default command
 
   def __new__(cls):
      if FrovedisServer.__instance is None:
         FrovedisServer.__instance = object.__new__(cls)
         n = rpclib.initialize_server(FrovedisServer.__cmd)
         excpt = rpclib.check_server_exception()
         if excpt["status"]: raise RuntimeError(excpt["info"])
         host = n['hostname']
         port = n['rpcport']
         FrovedisServer.__instance.mnode = node.exrpc_node(host,port)
         FrovedisServer.__instance.wsize = rpclib.get_worker_size(host,port)
         excpt = rpclib.check_server_exception()
         if excpt["status"]: raise RuntimeError(excpt["info"]) 
      return FrovedisServer.__instance

   @classmethod
   def getServerInstance(cls):
      inst = cls()
      return (inst.mnode.get_host(),inst.mnode.get_port())

   @classmethod
   def initialize(cls,command):
      if FrovedisServer.__instance is None:
         FrovedisServer.__cmd = command
      #else:
      #   print("Frovedis server is already initialized!!")
      cls.getServerInstance()

   @classmethod
   def shut_down(cls):
      if FrovedisServer.__instance is not None:
         (host,port) = cls.getServerInstance()
         rpclib.clean_server(host,port)
         excpt = rpclib.check_server_exception()
         if excpt["status"]: raise RuntimeError(excpt["info"]) 
         rpclib.finalize_server(host,port)
         excpt = rpclib.check_server_exception()
         if excpt["status"]: raise RuntimeError(excpt["info"]) 
         FrovedisServer.__instance = None
      #else:
      #   print("No server to finalize!")

   def display(cls):
      cls.mnode.display()
      print ("Frovedis master has " + str(cls.wsize) + " workers.")

   @classmethod
   def isUP(cls):
      if FrovedisServer.__instance is None: return False
      else: return True

# ensuring Frovedis Server will definitely shut-down on termination of 
# a python program which will import this module.
import atexit
atexit.register(FrovedisServer.shut_down)
