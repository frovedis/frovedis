package com.nec.frovedis.matrix;

import java.util.Arrays
import com.nec.frovedis.Jexrpc._
import com.nec.frovedis.Jmatrix.DummyMatrix
import com.nec.frovedis.matrix.Utils._
import org.apache.spark.rdd.RDD
import org.apache.spark.SparkContext
import org.apache.spark.mllib.linalg.{Vector,Vectors}
import org.apache.spark.mllib.linalg.{Matrix,Matrices}
import org.apache.spark.mllib.linalg.distributed.RowMatrix

// TODO - consider what to return? null or empty object, when fdata = -1
class FrovedisDenseMatrix(mt: Short) extends java.io.Serializable {
  private val mtype: Short = mt
  protected var fdata: Long = -1
  protected var num_row: Long = 0
  protected var num_col: Long = 0

  private def copy_local_matrix(index: Int, data: Iterator[Vector],
                                t_node: Node, ncol: Long) : Iterator[Long] = {
    val darr = data.toArray
    val nrow: Long = darr.size
    //println("index: " + index + ", nrow: " + nrow + ", ncol: " + ncol)
    val rmjr_mat = darr.map(_.toArray) //.flatten
    val ret = JNISupport.loadFrovedisWorkerRmajorMatData(t_node,nrow,ncol,rmjr_mat)
    //val ret = JNISupport.loadFrovedisWorkerRmajorData(t_node,nrow,ncol,rmjr_arr)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    //mapPartitionsWithIndex needs to return an Iterator object
    return Array(ret).toIterator
  }

  // TODO - RowMatrix seems to have unordered distribution of the row vectors
  // def load(data: RowMatrix) : Unit  = { }

  // creating new FrovedisDenseMatrix from DummyMatrix
  def this(matInfo: DummyMatrix) = {
    this(matInfo.mtype)
    fdata = matInfo.mptr
    num_row = matInfo.nrow
    num_col = matInfo.ncol
  }
  // loading existing FrovedisDenseMatrix from DummyMatrix
  private def load_dummy(matInfo: DummyMatrix) : Unit = {
    // TODO: add assert(mtype == matInfo.mtype)
    release() // releasing old data (if any)
    fdata = matInfo.mptr
    num_row = matInfo.nrow
    num_col = matInfo.ncol
  }
  def load(path: String) : Unit = { // loads from text file data
    val fs = FrovedisServer.getServerInstance()
    val dmat = JNISupport.loadFrovedisDenseData(fs.master_node,mtype,path,false)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);  
    load_dummy(dmat)
  }
  def loadbinary(path: String) : Unit = {  // loads from binary file data
    val fs = FrovedisServer.getServerInstance()
    val dmat = JNISupport.loadFrovedisDenseData(fs.master_node,mtype,path,true)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    load_dummy(dmat)
  }
  def load(data: RDD[Vector]) : Unit  = { // loads from spark worker data
    /** releasing the old data (if any) */
    release()

    /** Getting the global nrows and ncols information from the RDD */
    num_row = data.count
    num_col = data.first.size

    /** This will instantiate the frovedis server instance (if not already instantiated) */
    val fs = FrovedisServer.getServerInstance()
    val fs_size = fs.worker_size   /** number of worker nodes at frovedis side */

    /** converting spark worker data to frovedis worker data */
    val work_data = data.repartition2(fs_size)
    JNISupport.lockParallel()
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ep_all = work_data.mapPartitionsWithIndex(
                 (i,x) => copy_local_matrix(i,x,fw_nodes(i),num_col)).collect
    JNISupport.unlockParallel()

    /** getting frovedis distributed data from frovedis local data */ 
    fdata = JNISupport.createFrovedisDenseData(fs.master_node, ep_all,
                                             num_row, num_col, mtype)
    val info1 = JNISupport.checkServerException();
    if (info1 != "") throw new java.rmi.ServerException(info1);
  }
  def release(): Unit = {
    if (fdata != -1) {
      val fs = FrovedisServer.getServerInstance() 
      JNISupport.releaseFrovedisDenseData(fs.master_node,fdata,mtype)
      val info = JNISupport.checkServerException();
      if (info != "") throw new java.rmi.ServerException(info);
      fdata = -1
      num_row = 0 
      num_col = 0
    }
  }
  // if mtype = RMJR, returns *this 
  // else converts to rowmajor_matrix<double> and then returns converted matrix
  // Thus if you perform release() on the returned matrix, original matrix will
  // be released when mtype=RMJR (since in that case *this is returned)
  def to_frovedis_RowMatrix(): FrovedisDenseMatrix = {
    if(fdata == -1) return null // quick return
    val fs = FrovedisServer.getServerInstance() 
    var rmat = new DummyMatrix(fdata,num_row,num_col,mtype) // make ME a dummy
    if(mtype != MAT_KIND.RMJR) // converts to rowmajor matrix (new'ed)
      rmat = JNISupport.getFrovedisRowmajorMatrix(fs.master_node,fdata,mtype)
      val info = JNISupport.checkServerException();
      if (info != "") throw new java.rmi.ServerException(info);
    // TODO: mtype:RMJR => self destruction risk on having destructor
    return new FrovedisDenseMatrix(rmat) // outputs FrovedisRowmajorMatrix actually
  }
  def transpose(): FrovedisDenseMatrix = {
    if(fdata == -1) return null // quick return
    val fs = FrovedisServer.getServerInstance()
    val tmat = JNISupport.transposeFrovedisDenseData(fs.master_node,fdata,mtype)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new FrovedisDenseMatrix(tmat) // outputs this "mtype" matrix actually
  }
  private def setEachPartitionsData(index: Int, t_node: Node,
                                    t_dptr: Long): Iterator[Vector] = {
    //println("[setEachPartitionsData] index: " + index)
    val loc_arr = JNISupport.getLocalArray(t_node,t_dptr,MAT_KIND.RMJR_L)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    val l_num_col = num_col.intValue
    val l_num_row = loc_arr.size/l_num_col
    //println("nrow: " + l_num_row + ", ncol: " + l_num_col)
    //loc_arr.foreach(println)
    var ret = new Array[Vector](l_num_row)
    for(i <- 0 to (l_num_row-1)) {
      var st = i * l_num_col
      var end = st + l_num_col
      ret(i) = Vectors.dense(Arrays.copyOfRange(loc_arr, st, end))
    }
    return ret.toIterator
  }
  def to_spark_RowMatrix(ctxt: SparkContext) : RowMatrix = {
    if(fdata == -1) return null // quick return
    val rmat = to_frovedis_RowMatrix()
    val fs = FrovedisServer.getServerInstance()
    val eps = JNISupport.getAllLocalPointers(fs.master_node,
                                             rmat.fdata,rmat.mtype)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    JNISupport.lockParallel()
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node)
    val info1 = JNISupport.checkServerException();
    if (info1 != "") throw new java.rmi.ServerException(info1);
    val dummy = new Array[Boolean](eps.size)
    val dist_dummy =  ctxt.parallelize(dummy,eps.size)
    val rows = dist_dummy.mapPartitionsWithIndex((i,x) => 
                          setEachPartitionsData(i,fw_nodes(i),eps(i))).collect 
    // why collect? the below should work... TODO: test and confirm
    //val rows = dist_dummy.mapPartitionsWithIndex((i,x) => 
    //                      setEachPartitionsData(i,fw_nodes(i),eps(i))).cache
    //rows.count // to force the transformation to take place
    JNISupport.unlockParallel()
    // releasing intermediate rowmajor matrix from server, if new'ed
    if(mtype != MAT_KIND.RMJR) rmat.release()
    return new RowMatrix(ctxt.parallelize(rows,eps.size))
  }
  def to_spark_Matrix(): Matrix = { 
    if(fdata == -1) return null // quick return
    val fs = FrovedisServer.getServerInstance()
    val arr = JNISupport.getColmajorArray(fs.master_node,fdata,mtype)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return Matrices.dense(num_row.intValue,num_col.intValue,arr) 
  }
  def to_spark_Vector(): Vector = {
    if(fdata == -1) return null // quick return
    val fs = FrovedisServer.getServerInstance()
    val arr = JNISupport.getRowmajorArray(fs.master_node,fdata,mtype)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return Vectors.dense(arr)
  }
  def save(path: String) : Unit = { // saves to text file
    if(fdata != -1) {
      val fs = FrovedisServer.getServerInstance()
      JNISupport.saveFrovedisDenseData(fs.master_node,fdata,mtype,path,false)
      val info = JNISupport.checkServerException();
      if (info != "") throw new java.rmi.ServerException(info);
    }
  }
  def savebinary(path: String) : Unit = { // saves to binary file
    if(fdata != -1) {
      val fs = FrovedisServer.getServerInstance()
      JNISupport.saveFrovedisDenseData(fs.master_node,fdata,mtype,path,true)
      val info = JNISupport.checkServerException();
      if (info != "") throw new java.rmi.ServerException(info);  
  }
  }
  def debug_print() : Unit = {
    if (fdata != -1) {
      val fs = FrovedisServer.getServerInstance() 
      JNISupport.showFrovedisDenseData(fs.master_node,fdata,mtype)
      val info = JNISupport.checkServerException();
      if (info != "") throw new java.rmi.ServerException(info); 
    }
   }
  def get_rowmajor_view() : Unit = {
    if (fdata != -1) {
      val rmat = to_frovedis_RowMatrix()
      rmat.debug_print()
      // releasing intermediate rowmajor matrix from server, if new'ed
      if(mtype != MAT_KIND.RMJR) rmat.release()
    }
  }
  def get() = fdata
  def matType() = mtype
  def numRows() = num_row
  def numCols() = num_col
}

class FrovedisRowmajorMatrix extends FrovedisDenseMatrix(MAT_KIND.RMJR) {
  // TODO: def this (data: RowMatrix) = {}
  def this (data: RDD[Vector]) = {
    this()
    load(data)
  }
  def this(ptr: Long, nr: Long, nc: Long) = {
    this()
    fdata = ptr
    num_row = nr
    num_col = nc
  }
  def this (matInfo: DummyMatrix) = {
    this()
    // TODO: add assert(mtype == matInfo.mtype)
    fdata = matInfo.mptr
    num_row = matInfo.nrow
    num_col = matInfo.ncol
  }
}

class FrovedisColmajorMatrix extends FrovedisDenseMatrix(MAT_KIND.CMJR) {
  // TODO: def this (data: RowMatrix) = {}
  def this (data: RDD[Vector]) = {
    this()
    load(data)
  }
  def this(ptr: Long, nr: Long, nc: Long) = {
    this()
    fdata = ptr
    num_row = nr
    num_col = nc
  }
  def this (matInfo: DummyMatrix) = {
    this()
    // TODO: add assert(mtype == matInfo.mtype)
    fdata = matInfo.mptr
    num_row = matInfo.nrow
    num_col = matInfo.ncol
  }
}

class FrovedisBlockcyclicMatrix extends FrovedisDenseMatrix(MAT_KIND.BCLC) {
  // TODO: def this (data: RowMatrix) = {}
  def this (data: RDD[Vector]) = {
    this()
    load(data)
  }
  def this(ptr: Long, nr: Long, nc: Long) = {
    this()
    fdata = ptr
    num_row = nr
    num_col = nc
  }
  def this (matInfo: DummyMatrix) = {
    this()
    // TODO: add assert(mtype == matInfo.mtype)
    fdata = matInfo.mptr
    num_row = matInfo.nrow
    num_col = matInfo.ncol
  }
}
