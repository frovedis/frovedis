#!/usr/bin/env python

import numpy as np
import pandas as pd
import copy 
from ctypes import *
from ..exrpc.rpclib import *
from ..exrpc.server import *
from ..matrix.dtype import *
from .frovedisColumn import *
from . import df

class FrovedisGroupedDataframe :

  '''A python container for holding Frovedis side created grouped dataframe'''
  def __init__(cls,df=None):
    cls.__fdata = None
    cls.__cols  = None
    cls.__types = None
    cls.__p_cols  = None
    cls.__p_types = None
 
  def load_dummy(cls,fdata,cols,types,p_cols,p_types):
    cls.__fdata = fdata
    cls.__cols = copy.deepcopy(cols)
    cls.__types = copy.deepcopy(types)
    cls.__p_cols = copy.deepcopy(p_cols)
    cls.__p_types = copy.deepcopy(p_types)
    for i in range(0,len(p_cols)): 
      cname = p_cols[i]
      dt = p_types[i]
      cls.__dict__[cname] = FrovedisColumn(cname, dt)
    return cls

  def release(cls):
    if cls.__fdata is not None:
       (host, port) = FrovedisServer.getServerInstance()
       rpclib.release_frovedis_dataframe(host,port,cls.__fdata)
       excpt = rpclib.check_server_exception()
       if excpt["status"]: raise RuntimeError(excpt["info"]) 
       for cname in cls.__cols: del cls.__dict__[cname]
       cls.__fdata = None 
       cls.__cols  = None
       cls.__types = None
       cls.__p_cols  = None
       cls.__p_types = None

  #def __del__(cls):
  #  if FrovedisServer.isUP(): cls.release()

  def agg(cls, func, *args, **kwargs):
    return cls.aggregate(func, args, kwargs)

  def aggregate(cls, func, *args, **kwargs):
    if cls.__fdata is not None:
      if isinstance(func, str):
         return cls.__agg_with_list([func])
      elif isinstance(func, list):
         return cls.__agg_with_list(func)
      elif isinstance(func, dict):
         return cls.__agg_with_dict(func)
      else: raise TypeError("Unsupported input type for aggregation")
    else: raise ValueError("Operation on invalid frovedis grouped dataframe!")

  def __agg_with_list(cls,func):
    num_cols = cls.__get_numeric_columns()
    args = {}
    for col in num_cols:
      args[col] = func 
    if 'count' in func:
      n_num_cols = cls.__get_non_numeric_columns()
      for col in n_num_cols:
        args[col] = ['count']
    return cls.__agg_with_dict(args)   
  
  def __agg_with_dict(cls,func):
    agg_func = []
    agg_col = []
    agg_col_as = []
    agg_col_as_types = []
    for col,aggfuncs in func.items():
      if col not in cls.__dict__:
        raise ValueError("No column named: ", col)
      else: tid = cls.__dict__[col].dtype
      for f in aggfuncs: 
        if (tid == DTYPE.STRING and f != 'count'): 
          raise ValueError("Currently Frovedis doesn't support aggregator %s \
          to be applied on string-typed column %s" %(f,col))
        else: 
          agg_func.append(f)
          agg_col.append(col)
          new_col = f + '(' + col + ')'
          agg_col_as.append(new_col)
          if(f == 'count'):  col_as_tid = DTYPE.LONG
          elif(f == 'mean'): col_as_tid = DTYPE.DOUBLE
          else: col_as_tid = tid
          agg_col_as_types.append(col_as_tid)
    #print(agg_func)
    #print(agg_col)
    #print(agg_col_as)
    #print(agg_col_as_types)
    g_cols = np.asarray(cls.__cols)
    sz1 = g_cols.size
    g_cols_arr = (c_char_p * sz1)()
    g_cols_arr[:] = np.array([e.encode('ascii') for e in g_cols.T])

    a_func = np.asarray(agg_func)
    a_col = np.asarray(agg_col)
    a_col_as = np.asarray(agg_col_as)
    sz2 = a_func.size
    a_func_arr = (c_char_p * sz2)()
    a_col_arr = (c_char_p * sz2)()
    a_col_as_arr = (c_char_p * sz2)()
    a_func_arr[:] = np.array([e.encode('ascii') for e in a_func.T])
    a_col_arr[:] = np.array([e.encode('ascii') for e in a_col.T])
    a_col_as_arr[:] = np.array([e.encode('ascii') for e in a_col_as.T]) 

    (host, port) = FrovedisServer.getServerInstance()
    fdata = rpclib.agg_grouped_dataframe(host,port,cls.__fdata,
                                         g_cols_arr,sz1, 
                                         a_func_arr,a_col_arr,
                                         a_col_as_arr,sz2)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
 
    cols = cls.__cols + agg_col_as
    types = cls.__types + agg_col_as_types
    return df.FrovedisDataframe().load_dummy(fdata,cols,types)

  def __get_numeric_columns(cls):
    cols = []
    for i in range(0, len(cls.__p_cols)):
      if (cls.__p_types[i] != DTYPE.STRING): cols.append(cls.__p_cols[i])
    return cols

  def __get_non_numeric_columns(cls):
    cols = []
    for i in range(0, len(cls.__p_cols)):
      if (cls.__p_types[i] == DTYPE.STRING): cols.append(cls.__p_cols[i])
    return cols

  def get(cls): return cls.__fdata
