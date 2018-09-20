package com.nec.frovedis.matrix;

import com.nec.frovedis.Jexrpc.JNISupport
import com.nec.frovedis.Jexrpc.FrovedisServer
import org.apache.spark.rdd.RDD
import org.apache.spark.SparkContext
import org.apache.spark.mllib.linalg.Vector
import org.apache.spark.mllib.linalg.distributed.RowMatrix

object BLAS {
  def swap(vec1: FrovedisColmajorMatrix,
           vec2: FrovedisColmajorMatrix): Unit = {
    val fs = FrovedisServer.getServerInstance()
    JNISupport.swap(fs.master_node,DMAT_KIND.CMJR,vec1.get(),vec2.get())
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
  }

  def copy(vec1: FrovedisColmajorMatrix,
           vec2: FrovedisColmajorMatrix): Unit = {
    val fs = FrovedisServer.getServerInstance()
    JNISupport.copy(fs.master_node,DMAT_KIND.CMJR,vec1.get(),vec2.get())
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
  }

  def scal(vec: FrovedisColmajorMatrix,
           alpha: Double): Unit = {
    val fs = FrovedisServer.getServerInstance()
    JNISupport.scal(fs.master_node,DMAT_KIND.CMJR,vec.get(),alpha)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
  }

  def axpy(invec: FrovedisColmajorMatrix,
           outvec: FrovedisColmajorMatrix): Unit = {
    axpy(invec,outvec,1.0)
  }

  def axpy(invec: FrovedisColmajorMatrix,
           outvec: FrovedisColmajorMatrix,
           alpha: Double): Unit = {
    val fs = FrovedisServer.getServerInstance()
    JNISupport.axpy(fs.master_node,DMAT_KIND.CMJR,
                    invec.get(),outvec.get(),alpha)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
  }

  def dot(vec1: FrovedisColmajorMatrix,
          vec2: FrovedisColmajorMatrix): Double = {
    val fs = FrovedisServer.getServerInstance()
    val ret =  JNISupport.dot(fs.master_node,DMAT_KIND.CMJR,vec1.get(),vec2.get())
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    else return ret;
  }

  def nrm2(vec: FrovedisColmajorMatrix): Double = {
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.nrm2(fs.master_node,DMAT_KIND.CMJR,vec.get())
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    else return ret;
  }

  def gemv(inmat: FrovedisColmajorMatrix,
           invec: FrovedisColmajorMatrix,
           isTrans: Boolean,
           alpha: Double) : FrovedisColmajorMatrix = {
    return gemv(inmat,invec,isTrans,alpha,0.0)
  }

  def gemv(inmat: FrovedisColmajorMatrix,
           invec: FrovedisColmajorMatrix,
           isTrans: Boolean) : FrovedisColmajorMatrix = {
    return gemv(inmat,invec,isTrans,1.0,0.0)
  }

  def gemv(inmat: FrovedisColmajorMatrix,
           invec: FrovedisColmajorMatrix) : FrovedisColmajorMatrix = {
    return gemv(inmat,invec,false,1.0,0.0)
  }

  def gemv(inmat: FrovedisColmajorMatrix,
           invec: FrovedisColmajorMatrix,
           isTrans: Boolean,
           alpha: Double,
           beta: Double) : FrovedisColmajorMatrix = {
    val fs = FrovedisServer.getServerInstance()
    val dmat = JNISupport.gemv(fs.master_node,DMAT_KIND.CMJR,
                               inmat.get(),invec.get(),
                               isTrans,alpha,beta)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new FrovedisColmajorMatrix(dmat)    
  }  

  def ger(vec1: FrovedisColmajorMatrix,
          vec2: FrovedisColmajorMatrix): FrovedisColmajorMatrix = {
    return ger(vec1,vec2,1.0)
  }

  def ger(vec1: FrovedisColmajorMatrix,
          vec2: FrovedisColmajorMatrix,
          alpha: Double) : FrovedisColmajorMatrix = {
    val fs = FrovedisServer.getServerInstance()
    val dmat = JNISupport.ger(fs.master_node,DMAT_KIND.CMJR,
                              vec1.get(),vec2.get(),alpha)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new FrovedisColmajorMatrix(dmat)
  }

  def gemm(mat1: FrovedisColmajorMatrix,
           mat2: FrovedisColmajorMatrix) : FrovedisColmajorMatrix = {
    return gemm(mat1,mat2,false,false,1.0,0.0)
  }

  def gemm(mat1: FrovedisColmajorMatrix,
           mat2: FrovedisColmajorMatrix,
           isTransM1: Boolean) : FrovedisColmajorMatrix = {
    return gemm(mat1,mat2,isTransM1,false,1.0,0.0)
  }

  def gemm(mat1: FrovedisColmajorMatrix,
           mat2: FrovedisColmajorMatrix,
           isTransM1: Boolean,
           isTransM2: Boolean) : FrovedisColmajorMatrix = {
    return gemm(mat1,mat2,isTransM1,isTransM2,1.0,0.0)
  }

  def gemm(mat1: FrovedisColmajorMatrix,
           mat2: FrovedisColmajorMatrix,
           isTransM1: Boolean,
           isTransM2: Boolean,
           alpha: Double) : FrovedisColmajorMatrix = {
    return gemm(mat1,mat2,isTransM1,isTransM2,alpha,0.0)
  }

  def gemm(mat1: FrovedisColmajorMatrix,
           mat2: FrovedisColmajorMatrix,
           isTransM1: Boolean,
           isTransM2: Boolean,
           alpha: Double,
           beta: Double) : FrovedisColmajorMatrix = {
    val fs = FrovedisServer.getServerInstance()
    val dmat = JNISupport.gemm(fs.master_node,DMAT_KIND.CMJR,
                               mat1.get(),mat2.get(),
                               isTransM1,isTransM2,alpha,beta)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new FrovedisColmajorMatrix(dmat)
  }
}
