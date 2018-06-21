package com.nec.frovedis.matrix;

import com.nec.frovedis.Jexrpc.JNISupport
import com.nec.frovedis.Jexrpc.FrovedisServer

object LAPACK {
  def getrf(mat: FrovedisColmajorMatrix): GetrfResult = {
    val fs = FrovedisServer.getServerInstance()
    val r = JNISupport.getrf(fs.master_node,DMAT_KIND.CMJR,mat.get())
    return new GetrfResult(r)
  }

  def getri(mat: FrovedisColmajorMatrix,
            ipiv_ptr: Long) : Int = {
    val fs = FrovedisServer.getServerInstance()
    return JNISupport.getri(fs.master_node,DMAT_KIND.CMJR,
                            mat.get(),ipiv_ptr)
  }

  def getrs(matA: FrovedisColmajorMatrix,
            matB: FrovedisColmajorMatrix,
            ipiv_ptr: Long) : Int = {
    return getrs(matA,matB,ipiv_ptr,false);
  }

  def getrs(matA: FrovedisColmajorMatrix,
            matB: FrovedisColmajorMatrix,
            ipiv_ptr: Long,
            isTrans: Boolean) : Int = {
    val fs = FrovedisServer.getServerInstance()
    return JNISupport.getrs(fs.master_node,DMAT_KIND.CMJR,
                            matA.get(),matB.get(),
                            ipiv_ptr,isTrans)
  }

  def gesv(matA: FrovedisColmajorMatrix,
           matB: FrovedisColmajorMatrix): Int = {
    val fs = FrovedisServer.getServerInstance()
    return JNISupport.gesv(fs.master_node,DMAT_KIND.CMJR,
                           matA.get(),matB.get())
  }

  def gels(matA: FrovedisColmajorMatrix,
           matB: FrovedisColmajorMatrix) : Int = {
    return gels(matA,matB,false)
  }
  
  def gels(matA: FrovedisColmajorMatrix,
           matB: FrovedisColmajorMatrix,
           isTrans: Boolean): Int = {
    val fs = FrovedisServer.getServerInstance()
    return JNISupport.gels(fs.master_node,DMAT_KIND.CMJR,
                           matA.get(),matB.get(),isTrans)
  }

  // want both U and V
  def gesvd(mat: FrovedisColmajorMatrix) : GesvdResult = { 
    return gesvd(mat,true,true)
  }

  def gesvd(mat: FrovedisColmajorMatrix,
            wantU: Boolean,
            wantV: Boolean) :  GesvdResult = { 
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.gesvd(fs.master_node,DMAT_KIND.CMJR,
                               mat.get(),wantU,wantV)
    return new GesvdResult(ret) // outputs V (not VT as in scalapack)
  }
}
