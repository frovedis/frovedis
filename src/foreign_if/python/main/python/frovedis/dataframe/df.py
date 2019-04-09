#!/usr/bin/env python

import numpy as np
import pandas as pd
import copy 
from ctypes import *
from array import array
from ..exrpc.rpclib import *
from ..exrpc.server import *
from ..matrix.dvector import *
from ..matrix.dtype import *
from ..matrix.dense import FrovedisRowmajorMatrix
from ..matrix.dense import FrovedisColmajorMatrix
from ..matrix.crs import FrovedisCRSMatrix
from ..mllib.model_util import ModelID
from .info import df_to_sparse_info
from .frovedisColumn import *

class FrovedisDataframe :

  '''A python container for holding Frovedis side created dataframe'''
  def __init__(cls,df=None):
    cls.__fdata = None
    cls.__cols  = None
    cls.__types = None
    if df is not None: cls.load(df)
  def load_dummy(cls,fdata,cols,types):
    cls.__fdata = fdata
    cls.__cols = copy.deepcopy(cols)
    cls.__types = copy.deepcopy(types)
    for i in range(0,len(cols)): 
      cname = cols[i]
      dt = types[i]
      cls.__dict__[cname] = FrovedisColumn(cname, dt)
    return cls

  def show(cls):
   if cls.__fdata is not None:
      (host, port) = FrovedisServer.getServerInstance()
      rpclib.show_frovedis_dataframe(host,port,cls.__fdata)
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"]) 
      print("\n")

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

  #def __del__(cls):
  #  if FrovedisServer.isUP(): cls.release()

  def load(cls,df):
    cls.release()
    cols = df.columns 
    size = len(cols)
    cls.__cols = cols.tolist()
    cls.__types = [0]*size
    tmp = [None]*size
    dvec = [0]*size
    idx = 0

    for cname in cols:
       val = df[cname]  
       vtype = type(val[0]).__name__
       if vtype == 'int32':   
         dt = DTYPE.INT  
         tmp[idx] = FrovedisIntDvector(val)
       elif vtype == 'int64':   
         dt = DTYPE.LONG
         tmp[idx] = FrovedisLongDvector(val)
       elif vtype == 'float32': 
         dt = DTYPE.FLOAT
         tmp[idx] = FrovedisFloatDvector(val)
       elif vtype == 'float64': 
         dt = DTYPE.DOUBLE
         tmp[idx] = FrovedisDoubleDvector(val)
       elif vtype == 'str':     
         dt = DTYPE.STRING
         tmp[idx] = FrovedisStringDvector(val)
       else: 
         raise TypeError("Unsupported column type in creation of frovedis dataframe: ", vtype)
       cls.__types[idx] = dt
       dvec[idx] = tmp[idx].get()
       #For dataframe query purpose 
       cls.__dict__[cname] = FrovedisColumn(cname, dt)
       idx = idx + 1

    cols_a = np.asarray(cls.__cols)
    col_names = (c_char_p * size)()
    col_names[:] = [e.encode('ascii') for e in cols_a.T]
    dvec_arr = np.asarray(dvec,dtype=c_long)
    dptr = dvec_arr.ctypes.data_as(POINTER(c_long))
    type_arr = np.asarray(cls.__types,dtype=c_short)
    tptr = type_arr.ctypes.data_as(POINTER(c_short))

    (host, port) = FrovedisServer.getServerInstance()
    cls.__fdata = rpclib.create_frovedis_dataframe(host,port,tptr,col_names,dptr,size)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 
  
  def __getitem__(cls,target):
    if cls.__fdata is not None:
      if isinstance(target, str):
         return cls.select_frovedis_dataframe([target])
      elif isinstance(target, list):
         return cls.select_frovedis_dataframe(target)
      elif isinstance(target,dfoperator):
         return cls.filter_frovedis_dataframe(target)
      else :
         raise TypeError("Unsupported indexing input type!")
    else: raise ValueError("Operation on invalid frovedis dataframe!")

  def filter_frovedis_dataframe(cls,opt):
    ret = FrovedisDataframe()
    ret.__cols = copy.deepcopy(cls.__cols)
    ret.__types = copy.deepcopy(cls.__types)
    for item in ret.__cols: ret.__dict__[item] = cls.__dict__[item]
    (host, port) = FrovedisServer.getServerInstance()
    ret.__fdata = rpclib.filter_frovedis_dataframe(host,port,cls.get(),opt.get())
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 
    return ret
  
  def select_frovedis_dataframe(cls,targets):
    ret = FrovedisDataframe()
    ret.__cols = copy.deepcopy(targets) #targets is a list 
    ret.__types = [0]*len(targets)
    i = 0

    for item in targets: 
      if not item in cls.__cols:
        raise ValueError("No column named: ", item)
      else:
        ret.__types[i] = cls.__dict__[item].dtype
        ret.__dict__[item] = cls.__dict__[item]
        i = i + 1

    vec = np.asarray(targets)
    vv = vec.T # returns self, since ndim=1
    sz = vec.size
    ptr_arr = (c_char_p * sz)()
    ptr_arr[:] = [e.encode('ascii') for e in vv]

    #print("making exrpc request to select requested columns")
    (host, port) = FrovedisServer.getServerInstance()
    ret.__fdata = rpclib.select_frovedis_dataframe(host,port,cls.get(),ptr_arr,sz)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 
    return ret
    
  # TODO: support list of ascending 
  def sort_values(cls, by, axis=0, ascending=True,
                  inplace=False, kind='quicksort', na_position='last'):
    if cls.__fdata is None: 
      raise ValueError("Operation on invalid frovedis dataframe!")
    if(type(by).__name__ == 'str'):     sort_by = [by]
    elif (type(by).__name__ == 'list'): sort_by = by
    else: raise TypeError("Expected: string|list; Received: ", type(by).__name__)
  
    for item in sort_by:
      if not item in cls.__cols:
        raise ValueError("No column named: ", item)
 
    vec = np.asarray(sort_by)
    vv = vec.T # returns self, since ndim=1
    sz = vec.size
    ptr_arr = (c_char_p * sz)()
    ptr_arr[:] = [e.encode('ascii') for e in vv]

    ret = FrovedisDataframe()
    ret.__cols = copy.deepcopy(cls.__cols)
    ret.__types = copy.deepcopy(cls.__types)
    for item in ret.__cols: ret.__dict__[item] = cls.__dict__[item]
    
    #Making exrpc request for sorting.
    (host, port) = FrovedisServer.getServerInstance()
    ret.__fdata = rpclib.sort_frovedis_dataframe(host,port,cls.get(),
                                                 ptr_arr,sz,ascending)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 
    return ret

  def sort(cls,columns=None,axis=0,ascending=True,
           inplace=False,kind='quicksort',na_position='last',**kwargs):
    if not columns: raise ValueError("Column to be sorted cannot be None!")
    return cls.sort_values(by=columns,axis=axis,ascending=ascending,
                           inplace=inplace,kind=kind,na_position=na_position)

  def groupby(cls, by=None, axis=0, level=None, 
              as_index=True, sort=True, group_keys=True, squeeze=False):
    from frovedis.dataframe import grouped_df
    if cls.__fdata is None: 
      raise ValueError("Operation on invalid frovedis dataframe!")
    if(isinstance(by, str)):    g_by = [by]
    elif(isinstance(by, list)): g_by = by
    else: raise TypeError("Expected: string|list; Received: ", type(by).__name__)

    types = []
    for item in g_by: 
      if not item in cls.__cols:
        raise ValueError("No column named: ", item)
      else:
        types.append(cls.__dict__[item].dtype)

    vec = np.asarray(g_by)
    vv = vec.T
    sz = vec.size
    ptr_arr = (c_char_p * sz)()
    ptr_arr[:] = [e.encode('ascii') for e in vv]

    (host, port) = FrovedisServer.getServerInstance()
    fdata = rpclib.group_frovedis_dataframe(host,port,cls.get(),ptr_arr,sz)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 
    return grouped_df.FrovedisGroupedDataframe().load_dummy(fdata,g_by,types,
                                                 cls.__cols,cls.__types)

  # method to evaluate join keys
  def __evaluate(cls, df, left_on, right_on):
    if(type(left_on).__name__) == 'str' and (type(right_on).__name__) =='str':
      c1 = cls.__dict__[left_on]
      c2 = df.__dict__[right_on]
      dfopt = (c1 == c2)
    elif  (type(left_on).__name__) == 'list' and (type(right_on).__name__) == 'list':
      if(len(left_on) == len(right_on) and len(left_on) >= 1):
        c1 = cls.__dict__[left_on[0]]
        c2 = df.__dict__[right_on[0]]
        dfopt = (c1 == c2)
        for i in range(1, len(left_on)):
          c1 = cls.__dict__[left_on[i]]
          c2 = df.__dict__[right_on[i]]
          dfopt = (dfopt & (c1 == c2))
      else: raise ValueError("Size of left_on and right_on is not matched!")
    else: raise TypeError("Invalid key to join!")
    return dfopt

  def merge(cls, right, on=None, how='inner', left_on=None, right_on=None, 
            left_index=False, right_index=False, sort=False,
            suffixes=['_left','_right'], copy=True , 
            indicator=False,join_type='bcast'):

    if cls.__fdata is None: 
      raise ValueError("Operation on invalid frovedis dataframe!")
    df = FrovedisDataframe.asDF(right)      

    # TODO: to rename joined column on frovedis server
    left = []
    right = []
    for item in cls.__cols:
      if item in df.__cols:
        left.append(item)
        left.append(item + suffixes[0])
        right.append(item)
        right.append(item + suffixes[1])
    sz = len(left)
    vec1 = np.asarray(left)
    vec2 = np.asarray(right)
    ptr_left = (c_char_p * sz)()
    ptr_right = (c_char_p * sz)()
    ptr_left[:] = vec1.T
    ptr_right[:] = vec2.T
       
    if(on): #if key name is same in both dataframes
      if(left_on) or (right_on):
        raise ValueError("Can only pass 'on' OR 'left_on' and 'right_on', not a combination of both!")
      dfopt = cls.__evaluate(df,on,on) # currently exception at frovedis server
    elif (left_on) and (right_on):
      dfopt = cls.__evaluate(df,left_on,right_on)
    else: raise ValueError("Key field cannot be None!")

    ret = FrovedisDataframe()
    # TODO: correct joined names
    ret.__cols = cls.__cols + df.__cols
    ret.__types = cls.__types + df.__types
    for item in cls.__cols: ret.__dict__[item] = cls.__dict__[item]
    for item in df.__cols: 
      if item not in ret.__dict__: ret.__dict__[item] = df.__dict__[item]

    (host, port) = FrovedisServer.getServerInstance()
    ret.__fdata = rpclib.merge_frovedis_dataframe(host,port,cls.get(),df.get(),
                                                  dfopt.get(),how.encode('ascii'),
                                                  join_type.encode('ascii'))
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 
    return ret                                                        

  # exception at frovedis server: same key is not currently supported by frovedis
  def join(cls, right, on, how='inner', 
           lsuffix='_left', rsuffix='_right', sort=False, join_type='bcast'):
    suffix=[]
    suffix.append(lsuffix)
    suffix.append(rsuffix)
    if not on: raise ValueError("Key to join can not be None!")
    return cls.merge(right,on=on,how=how,suffixes=suffix,sort=sort,join_type=join_type) 

  # returns new dataframe with renamed columns
  def rename(cls, columns):
    if cls.__fdata is None: 
      raise ValueError("Operation on invalid frovedis dataframe!")
    if not isinstance(columns,dict): 
      raise TypeError("Expected: dictionery; Received: ", type(columns).__name__)
    names = list(columns.keys())
    new_names = list(columns.values())
    ret = FrovedisDataframe()
    ret.__cols = copy.deepcopy(cls.__cols)
    ret.__types = copy.deepcopy(cls.__types)
    for item in ret.__cols: ret.__dict__[item] = cls.__dict__[item]

    for i in range(0, len(names)):
      item = names[i]
      new_item = new_names[i]
      if not item in ret.__cols:
        raise ValueError("No column named: ", item)
      else:
        idx = ret.__cols.index(item)
        dt = ret.__types[idx]
        ret.__cols[idx] = new_item
        del ret.__dict__[item]
        ret.__dict__[new_item] = FrovedisColumn(new_item, dt)

    sz = len(names)
    vec1 = np.asarray(names)
    vec2 = np.asarray(new_names)
    name_ptr = (c_char_p * sz)()
    new_name_ptr = (c_char_p * sz)()
    name_ptr[:] = [e.encode('ascii') for e in vec1.T]
    new_name_ptr[:] = [e.encode('ascii') for e in vec2.T]

    (host, port) = FrovedisServer.getServerInstance()
    ret.__fdata = rpclib.rename_frovedis_dataframe(host,port,cls.get(),
                                                   name_ptr,new_name_ptr,sz)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 
    return ret                                                        

  def get(cls): return cls.__fdata

  @staticmethod
  def asDF(df):
    if isinstance(df,FrovedisDataframe): return df
    elif isinstance(df,pd.DataFrame): return FrovedisDataframe(df)
    else:  TypeError, "Invalid dataframe type is provided!"

  def min(cls, columns):
    if cls.__fdata is None: 
      raise ValueError("Operation on invalid frovedis dataframe!")
    vtype = type(columns).__name__
    if(vtype == 'str'): by = [columns]
    elif(vtype == 'list'): by = columns 
    else: raise TypeError("Expected: string|list; Received: ", vtype)
    types = cls.__get_column_types(by)
    if DTYPE.STRING in types: 
      raise ValueError("Non-numeric column given in min calculation!") 
    return cls.__get_stat('min',by,types)

  def max(cls, columns):
    if cls.__fdata is None: 
      raise ValueError("Operation on invalid frovedis dataframe!")
    vtype = type(columns).__name__
    if(vtype == 'str'): by = [columns]
    elif(vtype == 'list'): by = columns 
    else: raise TypeError("Expected: string|list; Received: ", vtype)
    types = cls.__get_column_types(by)
    if DTYPE.STRING in types: 
      raise ValueError("Non-numeric column given in max calculation!") 
    return cls.__get_stat('max',by,types)

  def sum(cls, columns):
    if cls.__fdata is None: 
      raise ValueError("Operation on invalid frovedis dataframe!")
    vtype = type(columns).__name__
    if(vtype == 'str'): by = [columns]
    elif(vtype == 'list'): by = columns 
    else: raise TypeError("Expected: string|list; Received: ", vtype)
    types = cls.__get_column_types(by)
    if DTYPE.STRING in types: 
      raise ValueError("Non-numeric column given in sum calculation!") 
    return cls.__get_stat('sum',by,types)

  def std(cls, columns):
    if cls.__fdata is None:
      raise ValueError("Operation on invalid frovedis dataframe!")
    vtype = type(columns).__name__
    if(vtype == 'str'): by = [columns]
    elif(vtype == 'list'): by = columns
    else: raise TypeError("Expected: string|list; Received: ", vtype)
    types = cls.__get_column_types(by)
    if DTYPE.STRING in types:
      raise ValueError("Non-numeric column given in std calculation!")
    return cls.__get_stat('std',by,types)

  def avg(cls, columns):
    if cls.__fdata is None: 
      raise ValueError("Operation on invalid frovedis dataframe!")
    vtype = type(columns).__name__
    if(vtype == 'str'): by = [columns]
    elif(vtype == 'list'): by = columns 
    else: raise TypeError("Expected: string|list; Received: ", vtype)
    types = cls.__get_column_types(by)
    if DTYPE.STRING in types: 
      raise ValueError("Non-numeric column given in avg calculation!") 
    return cls.__get_stat('avg',by)

  def count(cls, columns=None):
    if cls.__fdata is None: 
      raise ValueError("Operation on invalid frovedis dataframe!")
    if not columns: return cls.__get_stat('count',cls.__cols) # count of all columns
    vtype = type(columns).__name__
    if(vtype == 'str'): by = [columns]
    elif(vtype == 'list'): by = columns 
    else: raise TypeError("Expected: None|string|list; Received: ", vtype)
    return cls.__get_stat('count',by)

  def __get_column_types(cls, columns):
    if (type(columns).__name__ != 'list'):
      raise TypeError("Expected: list; Received: ", type(columns).__name__)
    sz = len(columns)
    types = [0]*sz
    for i in range(0, sz):
      item = columns[i]
      if item not in cls.__cols: raise ValueError("No column named: ", item)
      else:
        idx = cls.__cols.index(item)
        types[i] = cls.__types[idx]
    return types

  # returns python list of strings
  def __get_stat(cls, name, columns, types=None):
    if(type(types).__name__ == 'list' and len(columns) != len(types)): 
      raise ValueError("Size of inputs doesn't match!")
    if(type(name).__name__ != 'str'): 
      raise TypeError("Expected: string; Received: ", type(name).__name__)
    (host, port) = FrovedisServer.getServerInstance()
    sz = len(columns)
    cols = np.asarray(columns)
    cols_ptr = (c_char_p * sz)()
    cols_ptr[:] = [e.encode('ascii') for e in cols.T]

    if types: 
      type_arr = np.asarray(types,dtype=c_short)
      tptr = type_arr.ctypes.data_as(POINTER(c_short))
    if (name == 'min'): 
      if not types: raise ValueError("type of target columns is missing for min calculation")
      ret = rpclib.get_min_frovedis_dataframe(host,port,cls.get(),cols_ptr,tptr,sz)
    elif (name == 'max'): 
      if not types: raise ValueError("type of target columns is missing for max calculation")
      ret = rpclib.get_max_frovedis_dataframe(host,port,cls.get(),cols_ptr,tptr,sz)
    elif (name == 'sum'): 
      if not types: raise ValueError("type of target columns is missing for sum calculation")
      ret = rpclib.get_sum_frovedis_dataframe(host,port,cls.get(),cols_ptr,tptr,sz)
    elif (name == 'std'): 
      if not types: raise ValueError("type of target columns is missing for std calculation")
      ret = rpclib.get_std_frovedis_dataframe(host,port,cls.get(),cols_ptr,tptr,sz)
    elif (name == 'avg'): 
      ret = rpclib.get_avg_frovedis_dataframe(host,port,cls.get(),cols_ptr,sz)
    elif (name == 'count'): 
      ret = rpclib.get_cnt_frovedis_dataframe(host,port,cls.get(),cols_ptr,sz)
    else: raise ValueError("Unknown statistics request is encountered!")
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
    return ret

  def __get_numeric_columns(cls):
    cols = []
    for i in range(0, len(cls.__cols)):
      if (cls.__types[i] != DTYPE.STRING): cols.append(cls.__cols[i])
    return cols

  def describe(cls):
    cols = cls.__get_numeric_columns()
    count = cls.count(cols)
    mean = cls.avg(cols)
    std  = cls.std(cols)
    total = cls.sum(cols)
    minimum = cls.min(cols)
    maximum = cls.max(cols)
    index = ['count', 'mean', 'std', 'sum', 'min', 'max']
    sz = len(count) # all length should be same
    data = {}
    for i in range(0,sz):
      cc = cols[i]
      val = []
      val.append(count[i])
      val.append(mean[i])
      val.append(std[i])
      val.append(total[i])
      val.append(minimum[i])
      val.append(maximum[i])
      data[cc] = val
    #print(data)
    return pd.DataFrame(data,index=index)

  def to_panda_dataframe(cls):
    if cls.__fdata is None: 
      raise ValueError("Operation on invalid frovedis dataframe!")
    cols = cls.__cols
    types = cls.__types
    sz = len(cols) # lengths of cols and types are same
    data = {}
    (host, port) = FrovedisServer.getServerInstance()
    for i in range(0,sz):
      col_val = rpclib.get_frovedis_col(host,port,cls.get(),
                                        cols[i].encode('ascii'),types[i]) 
      excpt = rpclib.check_server_exception()
      if excpt["status"]: raise RuntimeError(excpt["info"]) 
      data[cols[i]] = col_val
    #print(data)
    return pd.DataFrame(data)

  def to_frovedis_rowmajor_matrix(cls,t_cols,dtype=np.float32): #default type: float
    if cls.__fdata is None: 
      raise ValueError("Operation on invalid frovedis dataframe!")
    for item in t_cols: # implicit checks for iterable on 't_cols'
      if item not in cls.__cols: raise ValueError("No column named: ", item)
    sz = len(t_cols)
    cols = np.asarray(t_cols)
    cols_ptr = (c_char_p * sz)()
    cols_ptr[:] = [e.encode('ascii') for e in cols.T]
    (host, port) = FrovedisServer.getServerInstance()
    if (dtype == np.float32):
      dmat = rpclib.df_to_rowmajor(host,port,cls.get(),
                                   cols_ptr,sz,DTYPE.FLOAT)
    elif (dtype == np.float64):
      dmat = rpclib.df_to_rowmajor(host,port,cls.get(),
                                   cols_ptr,sz,DTYPE.DOUBLE)
    else: 
       raise TypeError("Supported types: float32/float64; Found: " + dtype.__name__)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 
    return FrovedisRowmajorMatrix(mat=dmat,dtype=dtype)
    
  def to_frovedis_colmajor_matrix(cls,t_cols,dtype=np.float32): #default type: float
    if cls.__fdata is None: 
      raise ValueError("Operation on invalid frovedis dataframe!")
    for item in t_cols: # implicit checks for iterable on 't_cols'
      if item not in cls.__cols: raise ValueError("No column named: ", item)
    sz = len(t_cols)
    cols = np.asarray(t_cols)
    cols_ptr = (c_char_p * sz)()
    cols_ptr[:] = [e.encode('ascii') for e in cols.T]
    (host, port) = FrovedisServer.getServerInstance()
    if (dtype == np.float32):
      dmat = rpclib.df_to_colmajor(host,port,cls.get(),
                                   cols_ptr,sz,DTYPE.FLOAT)
    elif (dtype == np.float64):
      dmat = rpclib.df_to_colmajor(host,port,cls.get(),
                                   cols_ptr,sz,DTYPE.DOUBLE)
    else: 
       raise TypeError("Supported types: float32/float64; Found: " + dtype.__name__)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 
    return FrovedisColmajorMatrix(mat=dmat,dtype=dtype)

  def to_frovedis_crs_matrix(cls,t_cols,cat_cols,
                             dtype=np.float32, #default type: float
                             need_info=False):
    if cls.__fdata is None: 
      raise ValueError("Operation on invalid frovedis dataframe!")
    for item in t_cols: # implicit checks for iterable on 't_cols'
      if item not in cls.__cols: raise ValueError("No column named: ", item)
    for item in cat_cols: # implicit checks for iterable on 'cat_cols'
      if item not in t_cols:
        raise ValueError("target column list doesn't contain categorical column: ", item)
    sz1 = len(t_cols)
    cols = np.asarray(t_cols)
    cols_ptr = (c_char_p * sz1)()
    cols_ptr[:] = [e.encode('ascii') for e in cols.T]
    sz2 = len(cat_cols)
    cat_cols = np.asarray(cat_cols)
    cat_cols_ptr = (c_char_p * sz2)()
    cat_cols_ptr[:] = [e.encode('ascii') for e in cat_cols.T]
    info_id = ModelID.get() # getting unique id for info to be registered at server side
    (host, port) = FrovedisServer.getServerInstance()
    if (dtype == np.float32):
      dmat = rpclib.df_to_crs(host,port,cls.get(),
                              cols_ptr,sz1,
                              cat_cols_ptr,sz2,
                              info_id,DTYPE.FLOAT)
    elif (dtype == np.float64):
      dmat = rpclib.df_to_crs(host,port,cls.get(),
                              cols_ptr,sz1,
                              cat_cols_ptr,sz2,
                              info_id,DTYPE.DOUBLE)
    else: 
      raise TypeError("Supported types: float32/float64; Found: " + dtype.__name__)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
    crs = FrovedisCRSMatrix(mat=dmat,dtype=dtype,itype=np.int64) #default at server: size_t
    info = df_to_sparse_info(info_id)

    if(need_info): 
      return crs,info
    else: 
      info.release()
      return crs

  def to_frovedis_crs_matrix_using_info(cls,info,dtype=np.float32): #default type: float 
    if cls.__fdata is None: 
      raise ValueError("Operation on invalid frovedis dataframe!")
    if info.get() is None: 
      raise ValueError("Operation on invalid frovedis dataframe conversion info!")
    (host, port) = FrovedisServer.getServerInstance()
    if (dtype == np.float32):
      dmat = rpclib.df_to_crs_using_info(host,port,cls.get(),info.get(),DTYPE.FLOAT)
    elif (dtype == np.float64):
      dmat = rpclib.df_to_crs_using_info(host,port,cls.get(),info.get(),DTYPE.DOUBLE)
    else: 
      raise TypeError("Supported types: float32/float64; Found: " + dtype.__name__)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
    return FrovedisCRSMatrix(mat=dmat,dtype=dtype,itype=np.int64) #default at server: size_t 

    
