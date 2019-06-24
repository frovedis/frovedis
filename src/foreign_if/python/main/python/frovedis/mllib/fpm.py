#!/usr/bin/env python

import pandas as pd
from frovedis.exrpc.server import FrovedisServer
from frovedis.dataframe.df import FrovedisDataframe
from .model_util import *
from ..exrpc.rpclib import *
from ..exrpc.server import *
from ..matrix.dtype import DTYPE
import sys

class FPGrowth:
   "A python wrapper of Frovedis GPGrowth"
   
   def __init__(cls, minSupport = 0.3, verbose = 0):
      cls.minSupport = minSupport
      cls.verbose = verbose
      cls.__mid = None
      cls.__mkind = M_KIND.FPM
   
   def __convert_to_df(cls,data):
      tid = []
      item = []
      cur_id = 0
      for trans in data: # for each transaction in data
         for it in trans: # for each item in each transaction
           tid.append(cur_id)
           item.append(it)
         cur_id = cur_id + 1
      tid = np.asarray(tid, dtype=np.int32)
      item = np.asarray(item, dtype=np.int32)
      df = pd.DataFrame({'trans_id': tid,'item': item}, columns=['trans_id','item'])
      return FrovedisDataframe(df)   
	  
   def fit(cls, data):
      if(cls.minSupport < 0): raise ValueError("Negative minsupport factor!")
      cls.release()
      cls.__mid = ModelID.get()
      f_df = cls.__convert_to_df(data)
      #f_df.show()
      (host,port) = FrovedisServer.getServerInstance()
      rpclib.fpgrowth_trainer(host,port,f_df.get(),cls.__mid,cls.minSupport,cls.verbose)
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"]) 
      return cls
   
   def generate_rules(cls, confidence=0.8):
       midr = ModelID.get()
       (host,port) = FrovedisServer.getServerInstance()
       rpclib.fpgrowth_fpr(host,port,cls.__mid, midr,confidence)
       return Fp_rules(midr)                                                                          

   def load(cls,fname):
       cls.release()
       cls.__mid = ModelID.get()
       (host,port) = FrovedisServer.getServerInstance()
       rpclib.load_frovedis_model(host, port, cls.__mid, cls.__mkind, 
                                  DTYPE.DOUBLE, fname.encode('ascii'))
       excpt = rpclib.check_server_exception()
       if excpt["status"]: raise RuntimeError(excpt["info"]) 
       return cls

   def save(cls,fname):
       #print("saving")
       if cls.__mid is not None: GLM.save(cls.__mid,cls.__mkind,
                                          DTYPE.DOUBLE,fname)

   def debug_print(cls):
       if cls.__mid is not None: GLM.debug_print(cls.__mid,cls.__mkind,
                                                 DTYPE.DOUBLE)

   def release(cls):
       if cls.__mid is not None:
         GLM.release(cls.__mid,cls.__mkind,DTYPE.DOUBLE)
         cls.__mid = None

   def __del__(cls): 
       if FrovedisServer.isUP(): cls.release()

class Fp_rules:
    def __init__(cls, mid=None):
       cls.__mkind = M_KIND.FPR
       cls.__mid = mid
     
    def debug_print(cls):
       if cls.__mid is not None: GLM.debug_print(cls.__mid,cls.__mkind,
                                                 DTYPE.DOUBLE)



     
            
