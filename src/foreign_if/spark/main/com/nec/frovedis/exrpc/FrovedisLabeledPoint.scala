package com.nec.frovedis.exrpc;

import com.nec.frovedis.Jexrpc._
import com.nec.frovedis.matrix.Utils._
import com.nec.frovedis.matrix.ScalaCRS
import org.apache.spark.rdd.RDD
import org.apache.spark.SparkContext
import org.apache.spark.mllib.regression.LabeledPoint

class FrovedisLabeledPoint extends java.io.Serializable {
  protected var fdata : MemPair = null
  protected var num_row: Long = 0
  protected var num_col: Long = 0

  def this (data: RDD[LabeledPoint]) = {
    this()
    load(data)
  }
  private def convert_and_send_local_data(data: Iterator[LabeledPoint],
                                          t_node: Node) : Iterator[MemPair] = {
    val darr = data.toArray
    // Note: For Frovedis training data, output labels are either 1.0 or -1.0
    val lbls = darr.map(_.label).map(l => if (l == 0.0) -1.0 else l)
    val points = darr.map(_.features).map(p => p.toSparse)
    val scalaCRS = new ScalaCRS(points)
    val ret = JNISupport.loadFrovedisWorkerGLMData(t_node, 
                                                 scalaCRS.nrows,
                                                 scalaCRS.ncols, 
                                                 lbls,
                                                 scalaCRS.off.toArray,
                                                 scalaCRS.idx.toArray,
                                                 scalaCRS.data.toArray)
    //mapPartitionsWithIndex needs to return an Iterator object
    return Array(ret).toIterator
  }
  def load(data: RDD[LabeledPoint]) : Unit  = {
    /** releasing the old data (if any) */
    release()

    /** Getting the global nrows and ncols information from the RDD */
    num_row = data.count
    num_col = data.map(_.features.size).first()

    /** This will intantiate the frovedis server instance (if not already instantiated) */
    val fs = FrovedisServer.getServerInstance() 
    val fs_size = fs.worker_size   /** number of worker nodes at frovedis side */

    /** converting spark worker data to frovedis worker data */
    val work_data = data.repartition2(fs_size)
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    val ep_all = work_data.mapPartitionsWithIndex(
                 (i,x) => convert_and_send_local_data(x,fw_nodes(i))).collect

    /** getting frovedis distributed data from frovedis local data */ 
    fdata = JNISupport.createFrovedisLabeledPoint(fs.master_node, 
                                                 ep_all, num_row, num_col)
  }
  def release() : Unit = {
    if (fdata != null) {
      val fs = FrovedisServer.getServerInstance() 
      JNISupport.releaseFrovedisLabeledPoint(fs.master_node,fdata)
      fdata = null
      num_row = 0 
      num_col = 0
    }
  }
  def debug_print() : Unit = {
    if (fdata != null) {
      val fs = FrovedisServer.getServerInstance() 
      JNISupport.showFrovedisLabeledPoint(fs.master_node,fdata)
    }
  }
  def get() = fdata
  def numRows() = num_row
  def numCols() = num_col
}
