package com.nec.frovedis.mllib.clustering;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModel}
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.Vector
import org.apache.spark.SparkContext

class SpectralClustering(var nCluster: Int,
                         var nIteration: Int,
                         var nComponent: Int,
                         var nInit: Int,
                         var seed: Int, 
                         var eps: Double, 
                         var gamma: Double, 
                         var affinity: String,
                         var nNeighbors: Int,
                         var normLaplacian: Boolean,
                         var dropFirst: Boolean,
                         var mode: Int) extends java.io.Serializable {
  def this() = this(2, 100, 2, 1, 0, 0.0001, 1.0, "rbf", 10, true, true, 3)
 
  def setNumIteration(nIteration: Int ): this.type = {
    require(nIteration > 0 ,
      s"nIteration must be greater than 0 but got  ${nIteration}")
    this.nIteration = nIteration
    this
  }

  def setNumInit(nInit: Int ): this.type = {
    require(nInit > 0 ,
      s"nInit must be greater than 0 but got  ${nInit}")
    this.nInit = nInit
    this
  }

  def setNumCluster(nCluster: Int): this.type = {
    require(nCluster > 0 ,
      s"nCluster must be greater than 0 but got  ${nCluster}")
    this.nCluster = nCluster
    this
  }

  def setNumComponent(nComponent: Int): this.type = {
    require(nComponent > 0 ,
      s"nComponent must be greater than 0 but got  ${nComponent}")
    this.nComponent = nComponent
    this
  }

  def setEps(eps: Double): this.type = {
    require(eps > 0.0 && eps <= 1.0,
      s"eps must be greater than 0 but got  ${eps}")
    this.eps = eps
    this
  }

  def setSeed(seed: Int ): this.type = {
    this.seed = seed
    this
  }

  def setGamma(gamma: Double): this.type = {
    this.gamma = gamma
    this
  }

  def setAffinity(aff: String): this.type = {
    require(aff == "rbf" || aff == "precomputed" || aff == "nearest_neighbors",
      s"affinity must be either rbf, precomputed or nearest_neighbors, but got ${aff}")
    this.affinity = aff
    this
  }

  def setNumNeighbors(n_nb: Int): this.type = {
    require(n_nb > 0,
      s"n_nb must be greater than 0 but got  ${n_nb}")
    this.nNeighbors = n_nb
    this
  }

  def setNormLaplacian(normLaplacian: Boolean): this.type = {
    this.normLaplacian = normLaplacian
    this
  }

  def setMode(mode: Int): this.type = {
    require(mode == 1 || mode == 3 ,
      s"mode must be either 1 or 3, but got  ${mode}")
    this.mode = mode
    this
  }

  def setDropFirst(dropFirst: Boolean): this.type = {
    this.dropFirst = dropFirst
    this
  }

  def run(data:RDD[Vector]): SpectralClusteringModel = {
    val isDense = data.first.getClass.toString() matches ".*DenseVector*."
    if (isDense) {
      val fdata = new FrovedisRowmajorMatrix(data)
      return run(fdata,true)
    }
    else {
      val fdata = new FrovedisSparseData(data)
      return run(fdata,true)
    }
  }
  def run(data: FrovedisSparseData): SpectralClusteringModel = {
    return run(data, false)
  }
  def run(data: FrovedisSparseData,
          movable: Boolean): SpectralClusteringModel = {
    val mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    val ret =  JNISupport.callFrovedisSCA(fs.master_node,
                                  data.get(), nCluster, nIteration,
                                  nComponent, eps, nInit, seed,
                                  gamma, affinity, nNeighbors,
                                  normLaplacian, dropFirst, mode, 
                                  mid, movable, false)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new SpectralClusteringModel(mid).setLabels(ret)
  }
  def run(data: FrovedisRowmajorMatrix): SpectralClusteringModel = {
    return run(data, false)
  }
  def run(data: FrovedisRowmajorMatrix,
          movable: Boolean): SpectralClusteringModel = {
    val mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    val ret =  JNISupport.callFrovedisSCA(fs.master_node,
                                  data.get(), nCluster, nIteration,
                                  nComponent, eps, nInit, seed,
                                  gamma, affinity, nNeighbors,
                                  normLaplacian, dropFirst, mode, 
                                  mid, movable, true)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new SpectralClusteringModel(mid).setLabels(ret)
  }
}

class SpectralClusteringModel(model_Id: Int)
      extends GenericModel(model_Id, M_KIND.SCM) {
  var labels: Array[Int] = null

  def setLabels(labels: Array[Int]): this.type = {
    this.labels = labels
    this
  }
  def get_affinity_matrix(): FrovedisRowmajorMatrix = {
    val fs = FrovedisServer.getServerInstance()
    val dmat = JNISupport.getSCMAffinityMatrix(fs.master_node,model_Id)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisRowmajorMatrix(dmat)
  }
}

object SpectralClusteringModel {
  def load(sc: SparkContext, path: String): SpectralClusteringModel = load(path)
  def load(path: String): SpectralClusteringModel = {
    val model_id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.loadFrovedisSCM(fs.master_node,model_id,path)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new SpectralClusteringModel(model_id).setLabels(ret)
  }
}

