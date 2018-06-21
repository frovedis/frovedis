package com.nec.frovedis.matrix;

import com.nec.frovedis.Jexrpc.JNISupport
import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.Jmatrix.DummyMatrix
import com.nec.frovedis.Jmatrix.DummyGetrfResult
import com.nec.frovedis.Jmatrix.DummyGesvdResult
import org.apache.spark.rdd.RDD
import org.apache.spark.SparkContext
import org.apache.spark.mllib.linalg.{Vector, Vectors}
import org.apache.spark.mllib.linalg.{Matrix, Matrices}
import org.apache.spark.mllib.linalg.distributed.RowMatrix
import org.apache.spark.mllib.linalg.SingularValueDecomposition

class GetrfResult extends java.io.Serializable {
  protected var ipiv_ptr: Long = -1
  protected var info: Int = -1
  private var mtype: Short = -1

  def this(dummy: DummyGetrfResult) =  {
    this()
    mtype = dummy.mtype
    ipiv_ptr = dummy.ipiv_ptr
    info = dummy.info
  }
  def release(): Unit = {
    if(ipiv_ptr != -1) {
      val fs = FrovedisServer.getServerInstance()
      JNISupport.releaseIPIV(fs.master_node,mtype,ipiv_ptr)
      ipiv_ptr = -1
      info = -1
      mtype = -1
    }
  }
  def ipiv() = ipiv_ptr
  def stat() = info
}

// TODO - Consider quick return case (whether to return null or empty?)
class GesvdResult extends java.io.Serializable {
  protected var umat: FrovedisDenseMatrix = null // m x k 
  protected var vmat: FrovedisDenseMatrix = null // n x k
  protected var svec_ptr: Long = -1            // k x k (stores only diagonals)      
  protected var info: Int = -1

  private def load_dummy(dummy: DummyGesvdResult) = {
    val d_umat = new DummyMatrix(dummy.umat_ptr, dummy.m, dummy.k, dummy.mtype)
    val d_vmat = new DummyMatrix(dummy.vmat_ptr, dummy.n, dummy.k, dummy.mtype)
    if(d_umat.mptr != -1) umat = new FrovedisDenseMatrix(d_umat)
    if(d_vmat.mptr != -1) vmat = new FrovedisDenseMatrix(d_vmat)
    svec_ptr = dummy.svec_ptr
    info = dummy.info
  }
  def this(dummy: DummyGesvdResult) = {
    this()
    load_dummy(dummy)
  }
  def debug_print() = {
    if(svec_ptr != -1) {
      val fs = FrovedisServer.getServerInstance()
      val svec = JNISupport.getDoubleArray(fs.master_node,svec_ptr)
      println("svec: ")
      svec.foreach(println)
      if(umat != null) {
        println("umat: ")
        umat.get_rowmajor_view()
      }
      if(vmat != null) {
        println("vmat: ")
        vmat.get_rowmajor_view()
      }
    }
  }
  def to_spark_result(ctxt: SparkContext): 
    SingularValueDecomposition[RowMatrix,Matrix] = {
    if(svec_ptr == -1) return null // quick return
    val fs = FrovedisServer.getServerInstance()
    val svec = JNISupport.getDoubleArray(fs.master_node,svec_ptr)
    val spark_s = Vectors.dense(svec)
    val spark_u = if(umat != null) umat.to_spark_RowMatrix(ctxt) else null
    val spark_v = if(vmat != null) vmat.to_spark_Matrix() else null
    SingularValueDecomposition(spark_u,spark_s,spark_v)
  }
  // ufl is ignored, if wantU is false
  // vfl is ignored, if wantV is false
  private def save_impl(sfl: String, 
                        ufl: String, vfl: String,
                        wantU: Boolean, wantV: Boolean,
                        isbinary: Boolean) : Unit = {
    if(svec_ptr != -1) {
      val fs = FrovedisServer.getServerInstance()
      JNISupport.saveAsFrovedisDiagMatrixLocal(fs.master_node,
                                             svec_ptr,sfl,isbinary)
      if(wantU && ufl != null) {
        if(isbinary) umat.savebinary(ufl)
        else umat.save(ufl)
      }
      if(wantV && vfl != null) {
        if(isbinary) vmat.savebinary(vfl)
        else vmat.save(vfl)
      }
    }
  }
  def save(sfl: String, ufl: String, vfl: String) : Unit = {
    // TODO - assert(sfl != null)
    val wantU = if(umat == null) false else true
    val wantV = if(vmat == null) false else true
    save_impl(sfl,ufl,vfl,wantU,wantV,false)
  }
  def savebinary(sfl: String, ufl: String, vfl: String) : Unit = {
    // TODO - assert(sfl != null)
    val wantU = if(umat == null) false else true
    val wantV = if(vmat == null) false else true
    save_impl(sfl,ufl,vfl,wantU,wantV,true)
  }
  // ufl is ignored, if wantU is false
  // vfl is ignored, if wantV is false
  private def load_impl(sfl: String,
                        ufl: String, vfl: String, 
                        wantU: Boolean, wantV: Boolean,
                        mtype: Short,
                        isbinary: Boolean): Unit = {
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.getSVDResultFromFiles(fs.master_node,mtype,
                                               sfl,ufl,vfl,wantU,wantV,isbinary)
    release() // releasing old data
    load_dummy(ret) 
  }
  def load_as_blockcyclic(sfl: String, 
                          ufl: String, vfl: String): Unit = {
    // TODO - assert(sfl != null)
    val wantU = if(ufl == null) false else true
    val wantV = if(vfl == null) false else true
    load_impl(sfl,ufl,vfl,wantU,wantV,DMAT_KIND.BCLC,false)
  }
  def load_as_colmajor(sfl: String, 
                       ufl: String, vfl: String): Unit = {
    // TODO - assert(sfl != null)
    val wantU = if(ufl == null) false else true
    val wantV = if(vfl == null) false else true
    load_impl(sfl,ufl,vfl,wantU,wantV,DMAT_KIND.CMJR,false)
  }
  def loadbinary_as_blockcyclic(sfl: String, 
                                ufl: String, vfl: String): Unit = {
    // TODO - assert(sfl != null)
    val wantU = if(ufl == null) false else true
    val wantV = if(vfl == null) false else true
    load_impl(sfl,ufl,vfl,wantU,wantV,DMAT_KIND.BCLC,true)
  }
  def loadbinary_as_colmajor(sfl: String, 
                             ufl: String, vfl: String): Unit = {
    // TODO - assert(sfl != null)
    val wantU = if(ufl == null) false else true
    val wantV = if(vfl == null) false else true
    load_impl(sfl,ufl,vfl,wantU,wantV,DMAT_KIND.CMJR,true)
  }
  def release() : Unit = {
    if(svec_ptr != -1) {
      info = -1
      val fs = FrovedisServer.getServerInstance()
      JNISupport.releaseDoubleArray(fs.master_node,svec_ptr)
      svec_ptr = -1
      if(umat != null) {
        umat.release()
        umat = null
      }
      if(vmat != null) {
        vmat.release()
        vmat = null
      }
    }
  }
  def stat() = info
}
