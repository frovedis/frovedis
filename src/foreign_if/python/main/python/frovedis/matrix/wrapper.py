#!/usr/bin/env python

from scipy.sparse import csr_matrix
from ..exrpc.rpclib import *
from ..exrpc.server import *
from dense import FrovedisBlockcyclicMatrix
from sparse import FrovedisCRSMatrix
from results import GesvdResult, GetrfResult

# Input matrix/vector can be either (user created) FrovedisBlockcyclicMatrix
# or any array-like object (FrovedisBlockcyclicMatrix auto creation and deletion)
class PBLAS:
  "A python wrapper to call pblas routines at Frovedis server"

  @staticmethod
  def swap(v1, v2):
    (host, port) = FrovedisServer.getServerInstance()
    rpclib.pswap(host,port,v1.get(),v2.get()) 

  @staticmethod
  def copy(v1, v2):
    (host, port) = FrovedisServer.getServerInstance()
    rpclib.pcopy(host,port,v1.get(),v2.get()) 

  @staticmethod
  def scal(v, al):
    (host, port) = FrovedisServer.getServerInstance()
    rpclib.pscal(host,port,v.get(),al) 

  @staticmethod
  def axpy(v1, v2, al=1.0):
    (host, port) = FrovedisServer.getServerInstance()
    rpclib.paxpy(host,port,v1.get(),v2.get(),al) 

  @staticmethod
  def dot(v1, v2):
    vv1 = FrovedisBlockcyclicMatrix.asBCM(v1)
    vv2 = FrovedisBlockcyclicMatrix.asBCM(v2)
    (host, port) = FrovedisServer.getServerInstance()
    return rpclib.pdot(host,port,vv1.get(),vv2.get()) 

  @staticmethod
  def nrm2(v):
    vv = FrovedisBlockcyclicMatrix.asBCM(v)
    (host, port) = FrovedisServer.getServerInstance()
    return rpclib.pnrm2(host,port,vv.get()) 

  @staticmethod
  def gemv(mat,vec,trans=False,al=1.0,be=0.0):
    mm = FrovedisBlockcyclicMatrix.asBCM(mat)
    vv = FrovedisBlockcyclicMatrix.asBCM(vec)
    (host, port) = FrovedisServer.getServerInstance()
    dmat = rpclib.pgemv(host,port,mm.get(),vv.get(),trans,al,be)
    return FrovedisBlockcyclicMatrix(dmat)

  @staticmethod
  def ger(v1,v2,al=1.0):
    vv1 = FrovedisBlockcyclicMatrix.asBCM(v1)
    vv2 = FrovedisBlockcyclicMatrix.asBCM(v2)
    (host, port) = FrovedisServer.getServerInstance()
    dmat = rpclib.pger(host,port,vv1.get(),vv2.get(),al)
    return FrovedisBlockcyclicMatrix(dmat)

  @staticmethod
  def gemm(m1,m2,transM1=False,transM2=False,al=1.0,be=0.0):
    mm1 = FrovedisBlockcyclicMatrix.asBCM(m1)
    mm2 = FrovedisBlockcyclicMatrix.asBCM(m2)
    (host, port) = FrovedisServer.getServerInstance()
    dmat = rpclib.pgemm(host,port,mm1.get(),mm2.get(),transM1,transM2,al,be)
    return FrovedisBlockcyclicMatrix(dmat)

  # performs m2 = m1 + m2, m2 has to be blockcyclic matrix
  @staticmethod
  def geadd(m1,m2,trans=False,al=1.0,be=1.0):
    mm1 = FrovedisBlockcyclicMatrix.asBCM(m1)
    (host, port) = FrovedisServer.getServerInstance()
    rpclib.pgeadd(host,port,mm1.get(),m2.get(),trans,al,be)


# Input matrix can be either (user created) FrovedisBlockcyclicMatrix
# or any array-like object (FrovedisBlockcyclicMatrix auto creation and deletion)
class SCALAPACK:
  "A python wrapper to call scalapack routines at Frovedis server"

  @staticmethod
  def getrf(mat):
    mm = FrovedisBlockcyclicMatrix.asBCM(mat)
    (host,port) = FrovedisServer.getServerInstance()
    res = rpclib.pgetrf(host,port,mm.get())
    return GetrfResult(res)

  @staticmethod
  def getri(mat,ipiv_ptr):
    mm = FrovedisBlockcyclicMatrix.asBCM(mat)
    (host,port) = FrovedisServer.getServerInstance()
    return rpclib.pgetri(host,port,mm.get(),ipiv_ptr)

  @staticmethod
  def getrs(matA,matB,ipiv_ptr,isTrans=False):
    mmA = FrovedisBlockcyclicMatrix.asBCM(matA)
    mmB = FrovedisBlockcyclicMatrix.asBCM(matB)
    (host,port) = FrovedisServer.getServerInstance()
    return rpclib.pgetrs(host,port,mmA.get(),mmB.get(),ipiv_ptr,isTrans)

  @staticmethod
  def gesv(matA,matB):
    mmA = FrovedisBlockcyclicMatrix.asBCM(matA)
    mmB = FrovedisBlockcyclicMatrix.asBCM(matB)
    (host,port) = FrovedisServer.getServerInstance()
    return rpclib.pgesv(host,port,mmA.get(),mmB.get())

  @staticmethod
  def gels(matA,matB,isTrans=False):
    mmA = FrovedisBlockcyclicMatrix.asBCM(matA)
    mmB = FrovedisBlockcyclicMatrix.asBCM(matB)
    (host,port) = FrovedisServer.getServerInstance()
    return rpclib.pgesv(host,port,mmA.get(),mmB.get(),isTrans)

  @staticmethod
  def gesvd(mat,wantU=True,wantV=True):
    mm = FrovedisBlockcyclicMatrix.asBCM(mat)
    (host,port) = FrovedisServer.getServerInstance()
    res = rpclib.pgesvd(host,port,mm.get(),wantU,wantV)
    return GesvdResult(res)


# Input matrix can be either (user created) FrovedisCRSMatrix
# or any array-like object (FrovedisCRSMatrix auto creation and deletion)
class ARPACK:
  "A python wrapper to call arpack routines to compute sparse svd at Frovedis server"

  @staticmethod
  def computeSVD(mat,k):
    mm = FrovedisCRSMatrix.asCRS(mat)
    (host,port) = FrovedisServer.getServerInstance()
    res = rpclib.compute_sparse_svd(host,port,mm.get(),k)
    return GesvdResult(res)

