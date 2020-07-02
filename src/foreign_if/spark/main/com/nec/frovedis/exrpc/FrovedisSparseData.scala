package com.nec.frovedis.exrpc;

import com.nec.frovedis.Jexrpc._
import com.nec.frovedis.Jmatrix.DummyMatrix
import com.nec.frovedis.matrix.Utils._
import com.nec.frovedis.matrix.ScalaCRS
import com.nec.frovedis.matrix.MAT_KIND
import org.apache.spark.rdd.RDD
import org.apache.spark.SparkContext
import org.apache.spark.mllib.linalg.{Matrices, Vector}
import org.apache.spark.mllib.recommendation.Rating

class FrovedisSparseData extends java.io.Serializable {
  protected var fdata : Long = -1
  protected var num_row: Long = 0
  protected var num_col: Long = 0

  def this (data: RDD[Vector]) = {
    this()
    load(data)
  }
  def this(matInfo: DummyMatrix) = {
    this()
    fdata = matInfo.mptr
    num_row = matInfo.nrow
    num_col = matInfo.ncol
  }
  private def setEachPartitionsData(index: Int, t_node: Node,
                                    t_dptr: Long, nrow: Int, 
                                    nnz: Int): Iterator[Vector] = {
    val data = new Array[Double](nnz)
    val index = new Array[Int](nnz)
    val offset = new Array[Int](nrow+1)
    JNISupport.getLocalCRSMatrixComponents(t_node, t_dptr, data, index, 
                                           offset, nrow, nnz) 
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val crs = new ScalaCRS()
    crs.nrows = nrow
    crs.ncols = this.num_col 
    crs.off = offset
    crs.idx = index
    crs.data = data
    return crs.to_vector_array().toIterator
  }
  def to_spark_sparse_matrix() : RDD[Vector] = {
    if(fdata == -1) return null    
    val ctxt: SparkContext = SparkContext.getOrCreate()
    return to_spark_sparse_matrix(ctxt)
  }
  def to_spark_sparse_matrix(ctxt: SparkContext) : RDD[Vector] = {
    if(fdata == -1) return null    
    val fs = FrovedisServer.getServerInstance()
    /* get array of local crs_matrix pointers from all workers */
    val eps = JNISupport.getAllSparseMatrixLocalPointers(fs.master_node,
                                                         fdata, MAT_KIND.SCRS)
    var info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val l_rows = JNISupport.getAllSparseMatrixLocalRows(fs.master_node, fdata,
                                                        MAT_KIND.SCRS)
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val l_nnz = JNISupport.getAllSparseMatrixLocalNNZ(fs.master_node, fdata,
                                                      MAT_KIND.SCRS)
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node)
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info) 
    val dummy = new Array[Boolean](eps.size)
    val dist_dummy =  ctxt.parallelize(dummy,eps.size)
    val rows = dist_dummy.mapPartitionsWithIndex((i,x) =>
                          setEachPartitionsData(i,fw_nodes(i),eps(i),
                                                l_rows(i),l_nnz(i))).cache()
    return rows
  }
  private def convert_and_send_local_data(data: Iterator[Vector],
                                          t_node: Node) : Iterator[Long] = {
    val scalaCRS = new ScalaCRS(data.toArray)
    val ret = JNISupport.loadFrovedisWorkerData(t_node, 
                                              scalaCRS.nrows,
                                              scalaCRS.ncols,
                                              scalaCRS.off,
                                              scalaCRS.idx,
                                              scalaCRS.data)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    //mapPartitionsWithIndex needs to return an Iterator object
    return Array(ret).toIterator
  }
  def load(data: RDD[Vector]) : Unit  = {
    /** releasing the old data (if any) */
    release()

    /** Getting the global nrows and ncols information from the RDD */
    num_row = data.count
    num_col = data.first.size

    /** This will intantiate the frovedis server instance (if not already instantiated) */
    val fs = FrovedisServer.getServerInstance()
    val fs_size = fs.worker_size   /** number of worker nodes at frovedis side */

    /** converting spark worker data to frovedis worker data */
    val work_data = data.repartition2(fs_size)
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    val ep_all = work_data.mapPartitionsWithIndex(
                 (i,x) => convert_and_send_local_data(x,fw_nodes(i))).collect

    /** getting frovedis distributed data from frovedis local data */ 
    fdata = JNISupport.createFrovedisSparseData(fs.master_node, 
                                              ep_all, num_row, num_col)
    val info1 = JNISupport.checkServerException();
    if (info1 != "") throw new java.rmi.ServerException(info1);
  }
  private def convert_and_send_local_data_as_string_vector(data: Iterator[Rating],
                                          t_node: Node) : Iterator[Long] = {
    val darr = data.map(p => p.user + " " + p.product + " " + p.rating).toArray
    val size = darr.length
    val ret = JNISupport.loadFrovedisWorkerVectorStringData(t_node,darr,size)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    //mapPartitionsWithIndex needs to return an Iterator object
    return Array(ret).toIterator
  }
  def loadcoo(data: RDD[Rating]) : Unit  = {
    /** releasing the old data (if any) */
    release()

    /** This will intantiate the frovedis server instance (if not already instantiated) */
    val fs = FrovedisServer.getServerInstance()
    val fs_size = fs.worker_size   /** number of worker nodes at frovedis side */

    /** converting spark worker data to frovedis worker data */
    val work_data = data.repartition2(fs_size)
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    val ep_all = work_data.mapPartitionsWithIndex(
                 (i,x) => convert_and_send_local_data_as_string_vector(x,fw_nodes(i))).collect

    /** getting frovedis distributed sparse data from frovedis local coo vector strings */ 
    val dm = JNISupport.createFrovedisSparseMatrix(fs.master_node, ep_all, MAT_KIND.SCRS);
    val info1 = JNISupport.checkServerException();
    if (info1 != "") throw new java.rmi.ServerException(info1);
    fdata = dm.mptr
    num_row = dm.nrow
    num_col = dm.ncol
  }
  def release() : Unit = {
    if (fdata != -1) {
      val fs = FrovedisServer.getServerInstance() 
      JNISupport.releaseFrovedisSparseData(fs.master_node,fdata)
      val info = JNISupport.checkServerException();
      if (info != "") throw new java.rmi.ServerException(info);
      fdata = -1
      num_row = 0 
      num_col = 0
    }
  }
  def debug_print() : Unit = {
    if (fdata != -1) {
      val fs = FrovedisServer.getServerInstance() 
      JNISupport.showFrovedisSparseData(fs.master_node,fdata)
      val info = JNISupport.checkServerException();
      if (info != "") throw new java.rmi.ServerException(info);
    }
  }
  def get() = fdata
  def matType() = MAT_KIND.SCRS 
  def numRows() = num_row
  def numCols() = num_col
}
