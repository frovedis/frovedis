package com.nec.frovedis.exrpc;

import com.nec.frovedis.Jexrpc._
import com.nec.frovedis.matrix.FrovedisColmajorMatrix
import com.nec.frovedis.matrix.DoubleDvector
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.regression.LabeledPoint

class FrovedisLabeledPoint extends java.io.Serializable {

  protected var fdata : MemPair = null
  protected var num_row: Long = 0
  protected var num_col: Long = 0
  protected var isDense: Boolean = false

  def this (data: RDD[LabeledPoint]) = {
    this()
    load(data)
  }

  def load(data: RDD[LabeledPoint]) : Unit  = {
    /** releasing the old data (if any) */
    release()

    // Getting the global nrows and ncols information from the RDD 
    val nrow = data.count
    val ncol = data.first.features.size

    // extracting label and points
    val y = data.map(_.label)
    val x = data.map(_.features)

    // judging type of Vector
    this.isDense = x.first.getClass.toString() matches ".*DenseVector*."

    val yptr = DoubleDvector.get(y) // getting dvector pointer

    // getting matrix pointer along with num_row, num_col info
    var xptr: Long = -1
    if (this.isDense) {
       val mat = new FrovedisColmajorMatrix(x)
       xptr = mat.get()
       this.num_row = mat.numRows()
       this.num_col = mat.numCols()
    }
    else {
       val mat = new FrovedisSparseData(x)
       xptr = mat.get()
       this.num_row = mat.numRows()
       this.num_col = mat.numCols()
    }
    require(nrow == this.num_row && ncol == this.num_col, 
      "Internal error occured in FrovedisLabeledPoint creation - report bug!")
    this.fdata = new MemPair(xptr, yptr)
  }

  def release() : Unit = {
    if (fdata != null) {
      val fs = FrovedisServer.getServerInstance()
      JNISupport.releaseFrovedisLabeledPoint(fs.master_node,fdata,isDense) // isDense is added
      val info = JNISupport.checkServerException();
      if (info != "") throw new java.rmi.ServerException(info);
      fdata = null
      num_row = 0
      num_col = 0
      isDense = false
    }
  }

  def debug_print() : Unit = {
    if (fdata != null) {
      val fs = FrovedisServer.getServerInstance()
      JNISupport.showFrovedisLabeledPoint(fs.master_node,fdata,isDense) // isDense is added
      val info = JNISupport.checkServerException();
      if (info != "") throw new java.rmi.ServerException(info);
    }
  }

  def get() = fdata
  def numRows() = num_row
  def numCols() = num_col
  def is_dense() = isDense
}
