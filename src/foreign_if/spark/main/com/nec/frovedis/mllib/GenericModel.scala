package com.nec.frovedis.mllib;

import com.nec.frovedis.Jexrpc.{Node,FrovedisServer,JNISupport}
import com.nec.frovedis.matrix.Utils._
import com.nec.frovedis.matrix.ScalaCRS
import org.apache.spark.SparkContext
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.Vector

object M_KIND {
  val GLM:    Short = 0
  val LRM:    Short = 1
  val SVM:    Short = 2
  val LNRM:   Short = 3
  val MFM:    Short = 4
  val KMEANS: Short = 5
  val DTM:    Short = 6
  val NBM:    Short = 7
  val FMM:    Short = 8
  val FPM:    Short = 9
  val FPR:    Short = 10
  val ACM:    Short = 11
  val SCM:    Short = 12
  val SEM:    Short = 13
  val SPARSE_CONV_INFO: Short = 14
  val MLR: Short = 15
  val W2V: Short = 16
  val DBSCAN: Short = 17
  val KNN: Short = 18
  val KNC: Short = 19
  val KNR: Short = 20
  val LDA: Short = 21
  val LDASP: Short = 22
}

object ModelID {
  private var mid: Int = 0
  // A threshold value, assuming it is safe to re-iterate
  // ModelID after reaching this value without affecting the
  // registered models at Frovedis server side.
  private val ID_MAX: Int = 1 << 15
  def get() : Int = {
    mid = (mid + 1) % ID_MAX
    return mid
  }
}  

class GenericModel(modelId: Int,
                   modelKind: Short) extends java.io.Serializable {
  protected val mid: Int = modelId
  protected val mkind: Short = modelKind

  def debug_print() : Unit = {
    val fs = FrovedisServer.getServerInstance() 
    JNISupport.showFrovedisModel(fs.master_node,mid,mkind)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
  }
  def save(sc: SparkContext, path: String) : Unit = save(path) 
  def save(path: String) : Unit = {
    val fs = FrovedisServer.getServerInstance()
    JNISupport.saveFrovedisModel(fs.master_node,mid,mkind,path) 
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
  }
  def release() : Unit = {
    val fs = FrovedisServer.getServerInstance() 
    // It is serializable class. Thus an if check must be performed before the below JNI call.
    // To-Do: if(iam == rank0), only then call below JNI method 
    // [What is the Spark way of getting selfid()?]
    // Currently, the same is taken care using a deleted-model tracker at Frovedis side.
    JNISupport.releaseFrovedisModel(fs.master_node,mid,mkind)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    //println("[scala] model[" + mid + "] is finalized.")
  }
  //override def finalize() = release()
}

class GenericModelWithPredict(modelId: Int,
                              modelKind: Short,
                              logic: Map[Double, Double]) 
  extends GenericModel(modelId,modelKind) {
  protected val enc_logic: Map[Double, Double] = logic

  private[mllib] def parallel_predict(data: Iterator[Vector],
                                      mptr: Long,
                                      t_node: Node) : Iterator[Double] = {
    val darr = data.map(x => x.toSparse).toArray
    val scalaCRS = new ScalaCRS(darr)
    var ret = JNISupport.doParallelGLMPredict(t_node, mptr, mkind,
                                              scalaCRS.nrows, 
                                              scalaCRS.ncols,
                                              scalaCRS.off.toArray,
                                              scalaCRS.idx.toArray,
                                              scalaCRS.data.toArray)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val needed_decoding = Array(M_KIND.LRM, M_KIND.SVM, M_KIND.DTM)
    if ((needed_decoding contains mkind) && (logic != null))
      ret = ret.map(x => logic(x)) 
    return ret.toIterator
  }
  // prediction on single input
  def predict(data: Vector) : Double = {
    val fs = FrovedisServer.getServerInstance()
    val darr = Array(data.toSparse) // an array of one SparseVector
    val scalaCRS = new ScalaCRS(darr)
    var ret = JNISupport.doSingleGLMPredict(fs.master_node, mid, mkind,
                                            scalaCRS.nrows, 
                                            scalaCRS.ncols,
                                            scalaCRS.off.toArray,
                                            scalaCRS.idx.toArray,
                                            scalaCRS.data.toArray);
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val needed_decoding = Array(M_KIND.LRM, M_KIND.SVM, M_KIND.DTM)
    if ((needed_decoding contains mkind) && (logic != null)) ret = logic(ret) 
    return ret
  }
  // prediction on multiple inputs
  def predict(data: RDD[Vector]) : RDD[Double] = {
    val fs = FrovedisServer.getServerInstance()
    val each_model = JNISupport.broadcast2AllWorkers(fs.master_node,mid,mkind)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    //println("[scala] Getting worker info for prediction on model[" + mid + "].")
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node)
    val info1 = JNISupport.checkServerException()
    if (info1 != "") throw new java.rmi.ServerException(info1)
    val wdata = data.repartition2(fs.worker_size)
    return wdata.mapPartitionsWithIndex((i,x) => 
                parallel_predict(x,each_model(i),fw_nodes(i))).cache()
  }
}
