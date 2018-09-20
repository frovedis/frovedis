package com.nec.frovedis.matrix;

import com.nec.frovedis.Jexrpc.JNISupport
import com.nec.frovedis.Jexrpc.FrovedisServer

object ScaLAPACK {
  def getrf(mat: FrovedisBlockcyclicMatrix): GetrfResult = {
    val fs = FrovedisServer.getServerInstance()
    val r = JNISupport.getrf(fs.master_node,DMAT_KIND.BCLC,mat.get())
    return new GetrfResult(r)
  }

  def getri(mat: FrovedisBlockcyclicMatrix,
            ipiv_ptr: Long) : Int = {
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.getri(fs.master_node,DMAT_KIND.BCLC,
                            mat.get(),ipiv_ptr)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    else return ret;
  }

  def getrs(matA: FrovedisBlockcyclicMatrix,
            matB: FrovedisBlockcyclicMatrix,
            ipiv_ptr: Long) : Int = {
    return getrs(matA,matB,ipiv_ptr,false);
  }

  def getrs(matA: FrovedisBlockcyclicMatrix,
            matB: FrovedisBlockcyclicMatrix,
            ipiv_ptr: Long,
            isTrans: Boolean) : Int = {
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.getrs(fs.master_node,DMAT_KIND.BCLC,
                            matA.get(),matB.get(),
                            ipiv_ptr,isTrans)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    else return ret;
  }

  def gesv(matA: FrovedisBlockcyclicMatrix,
           matB: FrovedisBlockcyclicMatrix): Int = {
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.gesv(fs.master_node,DMAT_KIND.BCLC,
                           matA.get(),matB.get())
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    else return ret;
  }

  def gels(matA: FrovedisBlockcyclicMatrix,
           matB: FrovedisBlockcyclicMatrix) : Int = {
    return gels(matA,matB,false)
  }
  
  def gels(matA: FrovedisBlockcyclicMatrix,
           matB: FrovedisBlockcyclicMatrix,
           isTrans: Boolean): Int = {
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.gels(fs.master_node,DMAT_KIND.BCLC,
                           matA.get(),matB.get(),isTrans)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    else return ret;
  }

  // want both U and V
  def gesvd(mat: FrovedisBlockcyclicMatrix) : GesvdResult = { 
    return gesvd(mat,true,true)
  }

  def gesvd(mat: FrovedisBlockcyclicMatrix,
            wantU: Boolean,
            wantV: Boolean) : GesvdResult = { 
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.gesvd(fs.master_node,DMAT_KIND.BCLC,
                               mat.get(),wantU,wantV)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new GesvdResult(ret) // outputs V (not VT as in scalapack)
  }
}
