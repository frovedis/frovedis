package com.nec.frovedis.mllib.clustering;

import com.nec.frovedis.Jexrpc.{Node,FrovedisServer,JNISupport}
import com.nec.frovedis.matrix.ScalaCRS
import com.nec.frovedis.matrix.Utils._
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModel}
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import org.apache.spark.SparkContext
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.Vector

class KMeansModel(modelId: Int,
                  modelKind: Short,
                  kk: Int) extends GenericModel(modelId,modelKind) {
  private val k: Int = kk
  def getK() : Int = k

  def predict(X: Vector): Int = {
    require(this.mid > 0, "predict() is called before training ")
    var dproxy: Long = -1
    val fs = FrovedisServer.getServerInstance()
    val isDense = X.getClass.toString() matches ".*DenseVector*."
    if (isDense) {
      val nrow: Long = 1
      val ncol: Long = X.size
      val rmjr_arr = X.toArray
      dproxy = JNISupport.loadFrovedisWorkerRmajorData(fs.master_node,nrow,ncol,rmjr_arr)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
    }
    else {
      val scalaCRS = new ScalaCRS(Array(X))
      dproxy = JNISupport.loadFrovedisWorkerData(fs.master_node,
                                                 scalaCRS.nrows,
                                                 scalaCRS.ncols,
                                                 scalaCRS.off,
                                                 scalaCRS.idx,
                                                 scalaCRS.data)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
    }
    val pred = JNISupport.doSingleKMMPredict(fs.master_node, dproxy,
                                             this.mid, isDense)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return pred
  }

  def predict(X: RDD[Vector]): RDD[Int] = {
    require(this.mid > 0, "predict() is called before training ")
    val isDense = X.first.getClass.toString() matches ".*DenseVector*."
    if (isDense) {
      val fdata = new FrovedisRowmajorMatrix(X)
      val res = predict(fdata)
      fdata.release()
      return res
    }
    else {
      val fdata = new FrovedisSparseData(X)
      val res = predict(fdata)
      fdata.release()
      return res
    }
  }

  // dense predict()
  def predict(X: FrovedisRowmajorMatrix): RDD[Int] = {
    require(this.mid > 0, "predict() is called before training ")
    val fs = FrovedisServer.getServerInstance()
    val isDense = true
    val res = JNISupport.doParallelKMMPredict(fs.master_node, X.get(),
                                              this.mid, isDense)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val context = SparkContext.getOrCreate()
    return context.parallelize(res)
  }

  // sparse predict()
  def predict(X: FrovedisSparseData): RDD[Int] = {
    require(this.mid > 0, "predict() is called before training ")
    val fs = FrovedisServer.getServerInstance()
    val isDense = false
    val res = JNISupport.doParallelKMMPredict(fs.master_node, X.get(),
                                              this.mid, isDense)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val context = SparkContext.getOrCreate()
    return context.parallelize(res)
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
