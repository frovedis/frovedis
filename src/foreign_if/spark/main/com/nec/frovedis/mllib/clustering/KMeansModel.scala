package com.nec.frovedis.mllib.clustering;

import com.nec.frovedis.Jexrpc.{Node,FrovedisServer,JNISupport}
import com.nec.frovedis.matrix.ScalaCRS
import com.nec.frovedis.matrix.Utils._
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModel}
import org.apache.spark.SparkContext
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.Vector

class KMeansModel(modelId: Int,
                  modelKind: Short,
                  kk: Int) extends GenericModel(modelId,modelKind) {
  private val k: Int = kk

  def getK() : Int = k
  
  private def parallel_predict(data: Iterator[Vector],
                               mptr: Long,
                               t_node: Node) : Iterator[Int] = {
    val scalaCRS = new ScalaCRS(data.toArray)
    val ret = JNISupport.doParallelKMMPredict(t_node, mptr, mkind,
                                              scalaCRS.nrows, 
                                              scalaCRS.ncols,
                                              scalaCRS.off,
                                              scalaCRS.idx,
                                              scalaCRS.data)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return ret.toIterator
  }
  // prediction on single input
  def predict(data: Vector) : Int = {
    val fs = FrovedisServer.getServerInstance()
    val scalaCRS = new ScalaCRS(Array(data))
    val ret = JNISupport.doSingleKMMPredict(fs.master_node, mid, mkind,
                                            scalaCRS.nrows, 
                                            scalaCRS.ncols,
                                            scalaCRS.off,
                                            scalaCRS.idx,
                                            scalaCRS.data)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return ret;
  }
  // prediction on multiple inputs
  def predict(data: RDD[Vector]) : RDD[Int] = {
    val fs = FrovedisServer.getServerInstance()
    val each_model = JNISupport.broadcast2AllWorkers(fs.master_node,mid,mkind)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    //println("[scala] Getting worker info for prediction on model[" + mid + "].")
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node)
    val info1 = JNISupport.checkServerException();
    if (info1 != "") throw new java.rmi.ServerException(info1);
    val wdata = data.repartition2(fs.worker_size)
    return wdata.mapPartitionsWithIndex((i,x) => 
                parallel_predict(x,each_model(i),fw_nodes(i)))
  }
}

object KMeansModel {
  def load(sc: SparkContext, path: String): KMeansModel = load(path)
  def load(path: String): KMeansModel = {
    val mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    // load a KMeansModel from the 'path'
    // and register it with 'model_id' at Frovedis server
    val k = JNISupport.loadFrovedisKMM(fs.master_node,mid,M_KIND.KMEANS,path)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new KMeansModel(mid,M_KIND.KMEANS,k)
  } 
}
