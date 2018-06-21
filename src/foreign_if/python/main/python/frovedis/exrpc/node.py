#!/usr/bin/env python

class exrpc_node:
   """ Python side node structure to contain Frovedis side node information """

   def __init__(cls,hostname,rpcport):
      cls.__hostname = hostname
      cls.__rpcport = rpcport

   def display(cls):
      print("Hostname: " + cls.__hostname + ", Port: " + str(cls.__rpcport))

   def get_host(cls):
      return cls.__hostname
   
   def get_port(cls):
      return cls.__rpcport
