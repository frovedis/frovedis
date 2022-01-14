package com.nec.frovedis.mllib.recommendation;

import com.nec.frovedis.Jexrpc.{Node,FrovedisServer,JNISupport}
import com.nec.frovedis.Jmllib.IntDoublePair
import com.nec.frovedis.matrix.Utils._
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModel}
import org.apache.spark.rdd.RDD
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.recommendation.Rating

class MatrixFactorizationModel(modelId: Int,
                               modelKind: Short,
                               factor: Int) 
  extends GenericModel(modelId,modelKind) {
  private val rank: Int = factor

  private def parallel_predict(usersProducts: Iterator[(Int,Int)],
                               mptr: Long,
                               t_node: Node) : Iterator[Rating] = {
    val uparr = usersProducts.toArray
    val uids = uparr.map { case (u,p) => (u-1) }
    val pids = uparr.map { case (u,p) => (p-1) }
    val pred = JNISupport.doParallelALSPredict(t_node,mptr,mkind,uids,pids)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return uparr.zip(pred).map { case((u,p),r) => Rating(u,p,r) }.toIterator
  }
  def getRank() : Int = rank
  def predict(uid: Int, pid: Int) : Double = {
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.doSingleALSPredict(fs.master_node,mid,mkind,uid-1,pid-1)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    else return ret;
  }
  def predict(usersProducts: RDD[(Int, Int)]): RDD[Rating] = {
    val fs = FrovedisServer.getServerInstance()
    val each_model = JNISupport.broadcast2AllWorkers(fs.master_node,mid,mkind)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info);
    //println("[scala] Getting worker info for prediction on model[" + mid + "].")
    JNISupport.lockParallel()
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node)
    val info1 = JNISupport.checkServerException()
    if (info1 != "") throw new java.rmi.ServerException(info1)
    val wdata = usersProducts.repartition2(fs.worker_size)
    val ret = wdata.mapPartitionsWithIndex((i,x) => 
                 parallel_predict(x,each_model(i),fw_nodes(i))).cache
    ret.count // to force the transformation to take place
    JNISupport.unlockParallel()
    return ret
  }
  def recommendProducts(user: Int, num: Int): Array[Rating] = {
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.recommendProducts(fs.master_node,mid,mkind,user-1,num)
                     .map(x => Rating(user,x.getKey+1,x.getValue))
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    else return ret;
  }
  def recommendUsers(product: Int, num: Int): Array[Rating] = {
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.recommendUsers(fs.master_node,mid,mkind,product-1,num)
                     .map(x => Rating(x.getKey+1,product,x.getValue))
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    else return ret;
  }

}

object MatrixFactorizationModel {
  def load(sc: SparkContext, path: String): MatrixFactorizationModel = load(path)
  def load(path: String): MatrixFactorizationModel = {
    val mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    // load a MatrixFactorizationModel from the 'path'
    // and register it with 'model_id' at Frovedis server
    val rank = JNISupport.loadFrovedisMFM(fs.master_node,mid,M_KIND.MFM,path)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new MatrixFactorizationModel(mid,M_KIND.MFM,rank)
  } 
}
