package com.nec.frovedis.matrix;

import com.nec.frovedis.Jexrpc.JNISupport
import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.Jmatrix.DummyMatrix
import com.nec.frovedis.Jmatrix.DummyEvdResult
import org.apache.spark.rdd.RDD
import org.apache.spark.SparkContext
import org.apache.spark.mllib.linalg.{Vector, Vectors}
import org.apache.spark.mllib.linalg.{Matrix, Matrices}
import org.apache.spark.mllib.linalg.distributed.RowMatrix
import org.apache.spark.mllib.linalg.SingularValueDecomposition

class EvdResult extends java.io.Serializable {
  protected var vmat: FrovedisDenseMatrix = null // for storing eigenvectors
  protected var svec_ptr: Long = -1            // for storing eigenvalues    

  private def load_dummy(dummy: DummyEvdResult) = {
    val d_vmat = new DummyMatrix(dummy.vmat_ptr, dummy.n, dummy.k, MAT_KIND.CMJR)
    vmat = new FrovedisDenseMatrix(d_vmat)
    svec_ptr = dummy.svec_ptr
  }
  def this(dummy: DummyEvdResult) = {
    this()
    load_dummy(dummy)
  }
  def debug_print() = {
    if(svec_ptr != -1) {
      val fs = FrovedisServer.getServerInstance()
      val svec = JNISupport.getDoubleArray(fs.master_node,svec_ptr)
      val info = JNISupport.checkServerException();
      if (info != "") throw new java.rmi.ServerException(info);
      println("svec: ")
      svec.foreach(println)
      if(vmat != null) {
        println("vmat: ")
        vmat.get_rowmajor_view()
      }
    }
  }
  //to display result using SingularValueDecomposition in spark
  def to_spark_result(ctxt: SparkContext): 
    SingularValueDecomposition[RowMatrix,Matrix] = {
    if(svec_ptr == -1) return null // quick return
    val fs = FrovedisServer.getServerInstance()
    val svec = JNISupport.getDoubleArray(fs.master_node,svec_ptr)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    val spark_s = Vectors.dense(svec)
    val spark_v = if(vmat != null) vmat.to_spark_Matrix() else null
    SingularValueDecomposition(null,spark_s,spark_v)
  }
  //release the frovedis result from server
  def release() : Unit = {
    if(svec_ptr != -1) {
      val fs = FrovedisServer.getServerInstance()
      JNISupport.releaseDoubleArray(fs.master_node,svec_ptr)
      val info1 = JNISupport.checkServerException();
      if (info1 != "") throw new java.rmi.ServerException(info1);
      svec_ptr = -1
      if(vmat != null) {
        vmat.release()
        vmat = null
      }
    }
  }
}
