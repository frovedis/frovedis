package com.nec.frovedis.mllib.clustering;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.mllib.{M_KIND,ModelID}
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.Vector
//import org.apache.spark.util.Utils
import java.util.Random


class GaussianMixture(var k: Int,
                      var covarianceType: String,
                      var convergenceTol: Double,
                      var maxIterations: Int,
                      var initParams: String,
                      var seed: Long) extends java.io.Serializable {
  def this() = this(2, "full", 0.01, 100, "kmeans", new Random().nextLong().abs)

  def setK(k: Int): this.type = {
    require(k > 0 ,
      s"Number of components must be greater than 0 but got  ${k}")
    this.k = k
    this
  }

  def getK: Int = k

  def setCovarianceType(covarianceType: String): this.type = {
    require(covarianceType == "full" ,
      s"Spark GMM only supports 'full' covariance type but got ${covarianceType}")
    this.covarianceType = covarianceType
    this
  }

  def getCovarianceType: String = covarianceType

  def setConvergenceTol(convergenceTol: Double): this.type = {
    require(convergenceTol >= 0.0,
      s"Convergence tolerance must be nonnegative but got ${convergenceTol}")
    this.convergenceTol = convergenceTol
    this
  }

  def getConvergenceTol: Double = convergenceTol

  def setMaxIterations(maxIterations: Int): this.type = {
    require(maxIterations >= 0,
      s"Maximum of iterations must be nonnegative but got ${maxIterations}")
    this.maxIterations = maxIterations
    this
  }

  def getMaxIterations: Int = maxIterations

  def setInitParams(initParams: String): this.type = {
    require((initParams == "kmeans") || (initParams == "random"),
      s"Spark GMM only supports 'kmeans', 'random' init type but got ${initParams}")
    this.initParams = initParams
    this
  }

  def getInitParams: String = initParams

  def setSeed(seed: Long): this.type = {
    this.seed = seed.abs
    this
  }

  def getSeed: Long = seed

  def run(data: RDD[Vector]): GaussianMixtureModel = {
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

  def run(data: FrovedisSparseData): GaussianMixtureModel = {
    return run(data, false)
  }

  def run(data: FrovedisSparseData,
          movable: Boolean): GaussianMixtureModel = {
    val isDense = false
    val mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    //NOTE: This returns no. of iters, lower_bound used for convergence
    val ret =  JNISupport.callFrovedisGMM(fs.master_node,
                                          data.get(), k,
                                          covarianceType,
                                          convergenceTol,
                                          maxIterations,
                                          initParams, seed, mid,
                                          isDense, movable)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)  
    val niters = ret.getKey() // number if iterations in convergence
    val lb = ret.getValue() // lower_bound (log_likelihood)
    return new GaussianMixtureModel(mid).setIters(niters)
                                        .setLowerBound(lb)
  }

  def run(data: FrovedisRowmajorMatrix): GaussianMixtureModel = {
    return run(data, false)
  }

  def run(data: FrovedisRowmajorMatrix,
          movable: Boolean): GaussianMixtureModel = {
    val isDense = true
    val mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    //NOTE: This returns no. of iters, lower_bound used for convergence
    val ret =  JNISupport.callFrovedisGMM(fs.master_node,
                                          data.get(), k,
                                          covarianceType,
                                          convergenceTol,
                                          maxIterations,
                                          initParams, seed, mid,
                                          isDense, movable)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val niters = ret.getKey() // number if iterations in convergence
    val lb = ret.getValue() // lower_bound (log_likelihood)
    return new GaussianMixtureModel(mid).setIters(niters)
                                        .setLowerBound(lb)
  }
}
