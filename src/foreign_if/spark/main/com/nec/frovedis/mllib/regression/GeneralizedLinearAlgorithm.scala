package com.nec.frovedis.mllib.regression;

import com.nec.frovedis.Jexrpc.{Node,FrovedisServer,JNISupport}
import com.nec.frovedis.matrix.Utils._
import com.nec.frovedis.matrix.ScalaCRS
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModel}
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.Vector

object FROVEDIS {
  val NONE = 0xDEAD
}

class GeneralizedLinearModel(modelId: Int,
                             modelKind: Short,
                             nftr: Long,
                             ncls: Int,
                             icpt: Double,
                             thr: Double) 
  extends GenericModel(modelId,modelKind) {
  protected val numFeatures: Long = nftr
  protected val numClasses: Int = ncls
  protected var intercept: Double = icpt
  protected var threshold: Double = thr

  override def toString() : String = {
    val str = s"intercept: " + intercept + ", numFeatures: " + numFeatures + 
              s", numClasses: " + numClasses + ", threshold: " + threshold
    return str
  }
  def getThreshold(): Double = threshold 
  def setThreshold(thr: Double) : Unit = {
    threshold = thr
    val fs = FrovedisServer.getServerInstance() 
    JNISupport.setFrovedisGLMThreshold(fs.master_node,mid,mkind,thr)
  }
  def clearThreshold() : Unit = setThreshold(FROVEDIS.NONE); // predict-probability

  private def parallel_predict(data: Iterator[Vector],
                               mptr: Long,
                               t_node: Node) : Iterator[Double] = {
    val darr = data.map(x => x.toSparse).toArray
    val scalaCRS = new ScalaCRS(darr)
    val ret = JNISupport.doParallelGLMPredict(t_node, mptr, mkind,
                                              scalaCRS.nrows, 
                                              scalaCRS.ncols,
                                              scalaCRS.off.toArray,
                                              scalaCRS.idx.toArray,
                                              scalaCRS.data.toArray)
    return ret.toIterator
  }
  // prediction on single input
  def predict(data: Vector) : Double = {
    val fs = FrovedisServer.getServerInstance()
    val darr = Array(data.toSparse) // an array of one SparseVector
    val scalaCRS = new ScalaCRS(darr)
    val ret = JNISupport.doSingleGLMPredict(fs.master_node, mid, mkind,
                                            scalaCRS.nrows, 
                                            scalaCRS.ncols,
                                            scalaCRS.off.toArray,
                                            scalaCRS.idx.toArray,
                                            scalaCRS.data.toArray);
    return ret;
  }
  // prediction on multiple inputs
  def predict(data: RDD[Vector]) : RDD[Double] = {
    val fs = FrovedisServer.getServerInstance()
    val each_model = JNISupport.broadcast2AllWorkers(fs.master_node,mid,mkind)
    //println("[scala] Getting worker info for prediction on model[" + mid + "].")
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node)
    val wdata = data.repartition2(fs.worker_size)
    return wdata.mapPartitionsWithIndex((i,x) => 
                parallel_predict(x,each_model(i),fw_nodes(i)))
  }
}

