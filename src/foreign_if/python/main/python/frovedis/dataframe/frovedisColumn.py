#!/usr/bin/env python

from ..exrpc.rpclib import *
from ..exrpc.server import *
from dfoperator import *
from optype import *

class FrovedisColumn:
  def __init__(cls,colName,dtype):
    cls.__colName = colName
    cls.__dtype   = dtype

  @property
  def colName(cls):
    return cls.__colName

  @colName.setter
  def colName(cls, value):
    cls.__colName = value

  @property
  def dtype(cls):
    return cls.__dtype

  @dtype.setter
  def dtype(cls, value):
    cls.__dtype = value
 
  def __lt__(cls,other):
    (host, port) = FrovedisServer.getServerInstance()
    if isinstance(other,FrovedisColumn):
       #print 'Filtering dataframe where', cls.colName, '<', other.colName
       proxy = rpclib.get_frovedis_dfoperator(host,port,cls.colName,other.colName,cls.dtype,OPT.LT,False)
       return dfoperator(proxy)
    else:    
       #print 'Filtering dataframe where', cls.colName, '<', other 
       proxy = rpclib.get_frovedis_dfoperator(host,port,cls.colName,str(other),cls.dtype,OPT.LT,True)
       return dfoperator(proxy)

  def __gt__(cls,other):
    (host, port) = FrovedisServer.getServerInstance()
    if isinstance(other,FrovedisColumn):
       #print 'Filtering dataframe where', cls.colName, '>', other.colName
       proxy = rpclib.get_frovedis_dfoperator(host,port,cls.colName,other.colName,cls.dtype,OPT.GT,False)
       return dfoperator(proxy)
    else:
       #print 'Filtering dataframe where', cls.colName, '>', other
       proxy = rpclib.get_frovedis_dfoperator(host,port,cls.colName,str(other),cls.dtype,OPT.GT,True)
       return dfoperator(proxy)
    
  def __eq__(cls,other):
    (host, port) = FrovedisServer.getServerInstance()
    if isinstance(other,FrovedisColumn):
       #print 'Filtering dataframe where', cls.colName, '==', other.colName
       proxy = rpclib.get_frovedis_dfoperator(host,port,cls.colName,other.colName,cls.dtype,OPT.EQ,False)
       return dfoperator(proxy)
    else:
       #print 'Filtering dataframe where', cls.colName, '==', other
       proxy = rpclib.get_frovedis_dfoperator(host,port,cls.colName,str(other),cls.dtype,OPT.EQ,True)
       return dfoperator(proxy)
       
  def __ne__(cls,other):
    (host, port) = FrovedisServer.getServerInstance()
    if isinstance(other,FrovedisColumn):
       #print 'Filtering dataframe where', cls.colName, '!=', other.colName
       proxy = rpclib.get_frovedis_dfoperator(host,port,cls.colName,other.colName,cls.dtype,OPT.NE,False)
       return dfoperator(proxy)
    else:
       #print 'Filtering dataframe where', cls.colName, '!=', other
       proxy = rpclib.get_frovedis_dfoperator(host,port,cls.colName,str(other),cls.dtype,OPT.NE,True)
       return dfoperator(proxy)
 
  def __le__(cls,other):
    (host, port) = FrovedisServer.getServerInstance()
    if isinstance(other,FrovedisColumn):
       #print 'Filtering dataframe where', cls.colName, '<=', other.colName
       proxy = rpclib.get_frovedis_dfoperator(host,port,cls.colName,other.colName,cls.dtype,OPT.LE,False)
       return dfoperator(proxy)
    else:
       #print 'Filtering dataframe where', cls.colName, '<=', other
       proxy = rpclib.get_frovedis_dfoperator(host,port,cls.colName,str(other),cls.dtype,OPT.LE,True)
       return dfoperator(proxy)

  def __ge__(cls,other):
    (host, port) = FrovedisServer.getServerInstance()
    if isinstance(other,FrovedisColumn):
       #print 'Filtering dataframe where', cls.colName, '>=', other.colName
       proxy = rpclib.get_frovedis_dfoperator(host,port,cls.colName,other.colName,cls.dtype,OPT.GE,False)
       return dfoperator(proxy)
    else:
       #print 'Filtering dataframe where', cls.colName, '>=', other
       proxy = rpclib.get_frovedis_dfoperator(host,port,cls.colName,str(other),cls.dtype,OPT.GE,True)
       return dfoperator(proxy)

