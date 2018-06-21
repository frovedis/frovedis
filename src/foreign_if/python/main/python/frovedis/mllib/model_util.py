#!/usr/bin/env python

import numpy as np
from ..exrpc.server import *
from ..exrpc.rpclib import *
from ..matrix.sparse import FrovedisCRSMatrix

class M_KIND:
  "A python enumerator for wrapping model kinds"

  GLM = 0
  LRM = 1
  SVM = 2
  LNRM = 3
  MFM = 4
  KMEANS = 5

class ModelID:
  "A python container for generating model IDs for ML"

  # initial model ID for python's tracking [spark generates 1 to (1 << 15)]
  __initial = (1 << 15)
  __mid = __initial

  # A threshold value, assuming it is safe to re-iterate
  # ModelID after reaching this value without affecting the
  # registered models at Frovedis server side.
  __max_id = (1 << 31) 

  @staticmethod
  def get():
    ModelID.__mid = (ModelID.__mid + 1) % ModelID.__max_id
    if ModelID.__mid == 0: ModelID.__mid = ModelID.__initial + 1
    return ModelID.__mid

# Defines very generic functionaties of a Frovedis model
class GLM:
   "A python util for common operations on Generic Model"

   @staticmethod
   def predict(X,mid,mkind,prob):
      X = FrovedisCRSMatrix.asCRS(X)
      (host,port) = FrovedisServer.getServerInstance()
      len = X.numRows()
      ret = np.zeros(len,dtype=np.float64)
      rpclib.parallel_glm_predict(host,port,mid,mkind,X.get(),ret,len,prob)
      return ret

   @staticmethod
   def release(mid,mkind):
      (host,port) = FrovedisServer.getServerInstance()
      rpclib.release_frovedis_model(host,port,mid,mkind)

   @staticmethod
   def load(mid,mkind,fname):
      (host,port) = FrovedisServer.getServerInstance()
      rpclib.load_frovedis_model(host,port,mid,mkind,fname)

   @staticmethod
   def save(mid,mkind,fname):
      (host,port) = FrovedisServer.getServerInstance()
      rpclib.save_frovedis_model(host,port,mid,mkind,fname)

   @staticmethod
   def debug_print(mid,mkind):
      #print("model: " + str(mid) + " information is being displayed: ")
      (host,port) = FrovedisServer.getServerInstance()
      rpclib.show_frovedis_model(host,port,mid,mkind)

