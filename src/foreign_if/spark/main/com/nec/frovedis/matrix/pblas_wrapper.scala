package com.nec.frovedis.matrix;

import com.nec.frovedis.Jexrpc.JNISupport
import com.nec.frovedis.Jexrpc.FrovedisServer
import org.apache.spark.rdd.RDD
import org.apache.spark.SparkContext
import org.apache.spark.mllib.linalg.Vector
import org.apache.spark.mllib.linalg.distributed.RowMatrix

object PBLAS {
  def swap(vec1: FrovedisBlockcyclicMatrix,
           vec2: FrovedisBlockcyclicMatrix): Unit = {
    val fs = FrovedisServer.getServerInstance()
    JNISupport.swap(fs.master_node,DMAT_KIND.BCLC,vec1.get(),vec2.get())
  }

  def copy(vec1: FrovedisBlockcyclicMatrix,
           vec2: FrovedisBlockcyclicMatrix): Unit = {
    val fs = FrovedisServer.getServerInstance()
    JNISupport.copy(fs.master_node,DMAT_KIND.BCLC,vec1.get(),vec2.get())
  }

  def scal(vec: FrovedisBlockcyclicMatrix,
           alpha: Double): Unit = {
    val fs = FrovedisServer.getServerInstance()
    JNISupport.scal(fs.master_node,DMAT_KIND.BCLC,vec.get(),alpha)
  }

  def axpy(invec: FrovedisBlockcyclicMatrix,
           outvec: FrovedisBlockcyclicMatrix): Unit = {
    axpy(invec,outvec,1.0)
  }

  def axpy(invec: FrovedisBlockcyclicMatrix,
           outvec: FrovedisBlockcyclicMatrix,
           alpha: Double): Unit = {
    val fs = FrovedisServer.getServerInstance()
    JNISupport.axpy(fs.master_node,DMAT_KIND.BCLC,
                    invec.get(),outvec.get(),alpha)
  }

  def dot(vec1: FrovedisBlockcyclicMatrix,
          vec2: FrovedisBlockcyclicMatrix): Double = {
    val fs = FrovedisServer.getServerInstance()
    return JNISupport.dot(fs.master_node,DMAT_KIND.BCLC,vec1.get(),vec2.get())
  }

  def nrm2(vec: FrovedisBlockcyclicMatrix): Double = {
    val fs = FrovedisServer.getServerInstance()
    return JNISupport.nrm2(fs.master_node,DMAT_KIND.BCLC,vec.get())
  }

  def gemv(inmat: FrovedisBlockcyclicMatrix,
           invec: FrovedisBlockcyclicMatrix,
           isTrans: Boolean,
           alpha: Double) : FrovedisBlockcyclicMatrix = {
    return gemv(inmat,invec,isTrans,alpha,0.0)
  }

  def gemv(inmat: FrovedisBlockcyclicMatrix,
           invec: FrovedisBlockcyclicMatrix,
           isTrans: Boolean) : FrovedisBlockcyclicMatrix = {
    return gemv(inmat,invec,isTrans,1.0,0.0)
  }

  def gemv(inmat: FrovedisBlockcyclicMatrix,
           invec: FrovedisBlockcyclicMatrix) : FrovedisBlockcyclicMatrix = {
    return gemv(inmat,invec,false,1.0,0.0)
  }

  def gemv(inmat: FrovedisBlockcyclicMatrix,
           invec: FrovedisBlockcyclicMatrix,
           isTrans: Boolean,
           alpha: Double,
           beta: Double) : FrovedisBlockcyclicMatrix = {
    val fs = FrovedisServer.getServerInstance()
    val dmat = JNISupport.gemv(fs.master_node,DMAT_KIND.BCLC,
                               inmat.get(),invec.get(),
                               isTrans,alpha,beta)
    return new FrovedisBlockcyclicMatrix(dmat)    
  }  

  def ger(vec1: FrovedisBlockcyclicMatrix,
          vec2: FrovedisBlockcyclicMatrix): FrovedisBlockcyclicMatrix = {
    return ger(vec1,vec2,1.0)
  }

  def ger(vec1: FrovedisBlockcyclicMatrix,
          vec2: FrovedisBlockcyclicMatrix,
          alpha: Double) : FrovedisBlockcyclicMatrix = {
    val fs = FrovedisServer.getServerInstance()
    val dmat = JNISupport.ger(fs.master_node,DMAT_KIND.BCLC,
                              vec1.get(),vec2.get(),alpha)
    return new FrovedisBlockcyclicMatrix(dmat)
  }

  def gemm(mat1: FrovedisBlockcyclicMatrix,
           mat2: FrovedisBlockcyclicMatrix) : FrovedisBlockcyclicMatrix = {
    return gemm(mat1,mat2,false,false,1.0,0.0)
  }

  def gemm(mat1: FrovedisBlockcyclicMatrix,
           mat2: FrovedisBlockcyclicMatrix,
           isTransM1: Boolean) : FrovedisBlockcyclicMatrix = {
    return gemm(mat1,mat2,isTransM1,false,1.0,0.0)
  }

  def gemm(mat1: FrovedisBlockcyclicMatrix,
           mat2: FrovedisBlockcyclicMatrix,
           isTransM1: Boolean,
           isTransM2: Boolean) : FrovedisBlockcyclicMatrix = {
    return gemm(mat1,mat2,isTransM1,isTransM2,1.0,0.0)
  }

  def gemm(mat1: FrovedisBlockcyclicMatrix,
           mat2: FrovedisBlockcyclicMatrix,
           isTransM1: Boolean,
           isTransM2: Boolean,
           alpha: Double) : FrovedisBlockcyclicMatrix = {
    return gemm(mat1,mat2,isTransM1,isTransM2,alpha,0.0)
  }

  def gemm(mat1: FrovedisBlockcyclicMatrix,
           mat2: FrovedisBlockcyclicMatrix,
           isTransM1: Boolean,
           isTransM2: Boolean,
           alpha: Double,
           beta: Double) : FrovedisBlockcyclicMatrix = {
    val fs = FrovedisServer.getServerInstance()
    val dmat = JNISupport.gemm(fs.master_node,DMAT_KIND.BCLC,
                               mat1.get(),mat2.get(),
                               isTransM1,isTransM2,alpha,beta)
    return new FrovedisBlockcyclicMatrix(dmat)
  }

  def geadd(mat1: FrovedisBlockcyclicMatrix,
            mat2: FrovedisBlockcyclicMatrix): Unit = {
    geadd(mat1,mat2,false,1.0,1.0)
  }

  def geadd(mat1: FrovedisBlockcyclicMatrix,
            mat2: FrovedisBlockcyclicMatrix,
            isTrans: Boolean): Unit = {
    geadd(mat1,mat2,isTrans,1.0,1.0)
  }

  def geadd(mat1: FrovedisBlockcyclicMatrix,
            mat2: FrovedisBlockcyclicMatrix,
            isTrans: Boolean,
            alpha: Double): Unit = {
    geadd(mat1,mat2,isTrans,alpha,1.0)
  }

  def geadd(mat1: FrovedisBlockcyclicMatrix,
            mat2: FrovedisBlockcyclicMatrix,
            isTrans: Boolean,
            alpha: Double,
            beta: Double) : Unit = {
    val fs = FrovedisServer.getServerInstance()
    JNISupport.geadd(fs.master_node,DMAT_KIND.BCLC,
                     mat1.get(),mat2.get(),
                     isTrans,alpha,beta)
  }
}
