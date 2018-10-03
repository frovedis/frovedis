#!/usr/bin/env python

from scipy.sparse import csr_matrix
from ..exrpc.rpclib import *
from ..exrpc.server import *
from dense import FrovedisBlockcyclicMatrix
from crs import FrovedisCRSMatrix
from results import GesvdResult, GetrfResult
from dtype import DTYPE, TypeUtil

class WrapperUtil:
  "A python class to check dtype equality of wrapper argumentsr"

  @staticmethod
  def type_check(arg1, arg2, name):
    dt1 = arg1.get_dtype()
    dt2 = arg2.get_dtype()
    if(dt1 != dt2): raise TypeError(name, ": input argument dtypes are not matched!")
    # temporary check ...
    if(dt1 != DTYPE.DOUBLE): raise TypeError(name, ": only double type data is supported!")
    return dt1 #returns the type

# Input matrix/vector can be either (user created) FrovedisBlockcyclicMatrix
# or any array-like object (FrovedisBlockcyclicMatrix auto creation and deletion)
class PBLAS:
  "A python wrapper to call pblas routines at Frovedis server"

  @staticmethod
  def swap(v1, v2):
    (host, port) = FrovedisServer.getServerInstance()
    dt = WrapperUtil.type_check(v1,v2,"swap")
    rpclib.pswap(host,port,v1.get(),v2.get()) 
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 

  @staticmethod
  def copy(v1, v2):
    (host, port) = FrovedisServer.getServerInstance()
    dt = WrapperUtil.type_check(v1,v2,"copy")
    rpclib.pcopy(host,port,v1.get(),v2.get()) 
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 

  @staticmethod
  def scal(v, al):
    (host, port) = FrovedisServer.getServerInstance()
    rpclib.pscal(host,port,v.get(),al) 
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 

  @staticmethod
  def axpy(v1, v2, al=1.0):
    (host, port) = FrovedisServer.getServerInstance()
    dt = WrapperUtil.type_check(v1,v2,"axpy")
    rpclib.paxpy(host,port,v1.get(),v2.get(),al) 
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 

  @staticmethod
  def dot(v1, v2):
    vv1 = FrovedisBlockcyclicMatrix.asBCM(v1)
    vv2 = FrovedisBlockcyclicMatrix.asBCM(v2)
    (host, port) = FrovedisServer.getServerInstance()
    dt = WrapperUtil.type_check(vv1,vv2,"dot")
    ret = rpclib.pdot(host,port,vv1.get(),vv2.get()) 
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
    return ret

  @staticmethod
  def nrm2(v):
    vv = FrovedisBlockcyclicMatrix.asBCM(v)
    (host, port) = FrovedisServer.getServerInstance()
    ret = rpclib.pnrm2(host,port,vv.get()) 
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
    return ret

  @staticmethod
  def gemv(mat,vec,trans=False,al=1.0,be=0.0):
    mm = FrovedisBlockcyclicMatrix.asBCM(mat)
    vv = FrovedisBlockcyclicMatrix.asBCM(vec)
    dt = WrapperUtil.type_check(mm,vv,"gemv")
    (host, port) = FrovedisServer.getServerInstance()
    dmat = rpclib.pgemv(host,port,mm.get(),vv.get(),trans,al,be)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
    dt = TypeUtil.to_numpy_dtype(mm.get_dtype()) 
    return FrovedisBlockcyclicMatrix(mat=dmat,dtype=dt)

  @staticmethod
  def ger(v1,v2,al=1.0):
    vv1 = FrovedisBlockcyclicMatrix.asBCM(v1)
    vv2 = FrovedisBlockcyclicMatrix.asBCM(v2)
    (host, port) = FrovedisServer.getServerInstance()
    dt = WrapperUtil.type_check(vv1,vv2,"ger")
    dmat = rpclib.pger(host,port,vv1.get(),vv2.get(),al)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
    dt = TypeUtil.to_numpy_dtype(vv1.get_dtype()) 
    return FrovedisBlockcyclicMatrix(mat=dmat,dtype=dt)

  @staticmethod
  def gemm(m1,m2,transM1=False,transM2=False,al=1.0,be=0.0):
    mm1 = FrovedisBlockcyclicMatrix.asBCM(m1)
    mm2 = FrovedisBlockcyclicMatrix.asBCM(m2)
    (host, port) = FrovedisServer.getServerInstance()
    dt = WrapperUtil.type_check(mm1,mm2,"gemm")
    dmat = rpclib.pgemm(host,port,mm1.get(),mm2.get(),transM1,transM2,al,be)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
    dt = TypeUtil.to_numpy_dtype(mm1.get_dtype()) 
    return FrovedisBlockcyclicMatrix(mat=dmat,dtype=dt)

  # performs m2 = m1 + m2, m2 has to be blockcyclic matrix
  @staticmethod
  def geadd(m1,m2,trans=False,al=1.0,be=1.0):
    mm1 = FrovedisBlockcyclicMatrix.asBCM(m1)
    (host, port) = FrovedisServer.getServerInstance()
    dt = WrapperUtil.type_check(mm1,m2,"geadd")
    rpclib.pgeadd(host,port,mm1.get(),m2.get(),trans,al,be)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])


# Input matrix can be either (user created) FrovedisBlockcyclicMatrix
# or any array-like object (FrovedisBlockcyclicMatrix auto creation and deletion)
class SCALAPACK:
  "A python wrapper to call scalapack routines at Frovedis server"

  @staticmethod
  def getrf(mat):
    mm = FrovedisBlockcyclicMatrix.asBCM(mat)
    (host,port) = FrovedisServer.getServerInstance()
    res = rpclib.pgetrf(host,port,mm.get())
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 
    return GetrfResult(res)

  @staticmethod
  def getri(mat,ipiv_ptr):
    mm = FrovedisBlockcyclicMatrix.asBCM(mat)
    (host,port) = FrovedisServer.getServerInstance()
    ret = rpclib.pgetri(host,port,mm.get(),ipiv_ptr)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"]) 
    return ret

  @staticmethod
  def getrs(matA,matB,ipiv_ptr,isTrans=False):
    mmA = FrovedisBlockcyclicMatrix.asBCM(matA)
    mmB = FrovedisBlockcyclicMatrix.asBCM(matB)
    (host,port) = FrovedisServer.getServerInstance()
    dt = WrapperUtil.type_check(mmA,mmB,"getrs")
    ret = rpclib.pgetrs(host,port,mmA.get(),mmB.get(),ipiv_ptr,isTrans)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
    return ret

  @staticmethod
  def gesv(matA,matB):
    mmA = FrovedisBlockcyclicMatrix.asBCM(matA)
    mmB = FrovedisBlockcyclicMatrix.asBCM(matB)
    (host,port) = FrovedisServer.getServerInstance()
    dt = WrapperUtil.type_check(mmA,mmB,"gesv")
    ret = rpclib.pgesv(host,port,mmA.get(),mmB.get())
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
    return ret

  @staticmethod
  def gels(matA,matB,isTrans=False):
    mmA = FrovedisBlockcyclicMatrix.asBCM(matA)
    mmB = FrovedisBlockcyclicMatrix.asBCM(matB)
    (host,port) = FrovedisServer.getServerInstance()
    dt = WrapperUtil.type_check(mmA,mmB,"gels")
    ret = rpclib.pgesv(host,port,mmA.get(),mmB.get(),isTrans)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
    return ret

  @staticmethod
  def gesvd(mat,wantU=True,wantV=True):
    mm = FrovedisBlockcyclicMatrix.asBCM(mat)
    (host,port) = FrovedisServer.getServerInstance()
    res = rpclib.pgesvd(host,port,mm.get(),wantU,wantV)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
    return GesvdResult(res,TypeUtil.to_numpy_dtype(mm.get_dtype()))


# Input matrix can be either (user created) FrovedisCRSMatrix
# or any array-like object (FrovedisCRSMatrix auto creation and deletion)
class ARPACK:
  "A python wrapper to call arpack routines to compute sparse svd at Frovedis server"

  @staticmethod
  def computeSVD(mat,k):
    mm = FrovedisCRSMatrix.asCRS(mat)
    dtype = mm.get_dtype()
    itype = mm.get_itype()
    (host,port) = FrovedisServer.getServerInstance()
    res = rpclib.compute_sparse_svd(host,port,mm.get(),k,dtype,itype)
    excpt = rpclib.check_server_exception()
    if excpt["status"]: raise RuntimeError(excpt["info"])
    return GesvdResult(res,TypeUtil.to_numpy_dtype(mm.get_dtype()))

