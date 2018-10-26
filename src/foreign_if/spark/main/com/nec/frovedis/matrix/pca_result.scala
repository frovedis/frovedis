package com.nec.frovedis.matrix;

import com.nec.frovedis.Jexrpc.JNISupport
import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.Jmatrix.DummyMatrix
import com.nec.frovedis.Jmatrix.DummyPCAResult
import org.apache.spark.rdd.RDD
import org.apache.spark.SparkContext
import org.apache.spark.mllib.linalg.{Vectors, Vector, Matrix}
import org.apache.spark.mllib.linalg.distributed.RowMatrix

class FrovedisPCAModel extends java.io.Serializable {
  protected var k: Int = -1
  protected var pc: FrovedisDenseMatrix = null // n x k
  protected var var_ptr: Long = -1   // k x k (stores only diagonals)      

  def this(dummy: DummyPCAResult, 
           need_variance: Boolean) = {
    this()
    load_dummy(dummy, need_variance)
  }
  private def load_dummy(dummy: DummyPCAResult,
                         need_variance: Boolean) = {
    k = dummy.k
    val pc_mat = new DummyMatrix(dummy.mptr,dummy.nrows,dummy.ncols,dummy.mtype)
    pc = new FrovedisDenseMatrix(pc_mat)
    if(need_variance) var_ptr = dummy.vptr
  }
  def debug_print() = {
    require(pc != null, "No result to display!")
    println("pc: ")
    pc.get_rowmajor_view()
    if (var_ptr != -1) {
      val fs = FrovedisServer.getServerInstance()
      val explainedVariance = JNISupport.getDoubleArray(fs.master_node,var_ptr)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      println("explainedVariance: ")
      explainedVariance.foreach(println)
    }
  }
  def to_spark_result(): (Matrix,Vector) = {
    require(pc != null, "No result to convert!")
    var spark_pc = pc.to_spark_Matrix()
    var spark_explainedVariance: Vector = null
    if (var_ptr != -1) {
      val fs = FrovedisServer.getServerInstance()
      val explainedVariance = JNISupport.getDoubleArray(fs.master_node,var_ptr)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      spark_explainedVariance = Vectors.dense(explainedVariance)
    }
    return (spark_pc, spark_explainedVariance)    
  }
  def release() : Unit = {
    pc.release()
    pc = null
    if(var_ptr != -1) {
      val fs = FrovedisServer.getServerInstance()
      JNISupport.releaseDoubleArray(fs.master_node,var_ptr)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      var_ptr = -1
    }
  }
}
