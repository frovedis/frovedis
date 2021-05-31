package com.nec.frovedis.mllib.clustering;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import org.apache.spark.rdd.RDD
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModel}
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.matrix.ScalaCRS
import org.apache.spark.SparkContext
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.{Vector, Vectors, DenseMatrix}
import org.apache.spark.mllib.stat.distribution.MultivariateGaussian

class GaussianMixtureModel(modelId: Int)
                           extends GenericModel(modelId,M_KIND.GMM) {

  def k:Int = getWeights().length

  //Model Weights
  def weights:Array[Double] = getWeights()
  private var _weights:Array[Double] = null

  //Model gaussians
  def gaussians:Array[MultivariateGaussian] = getGaussians()
  private var _gaussians:Array[MultivariateGaussian] = null

  //Model Covariances
  private var _covs:Array[Double] = null

  //Model means
  private var _means:Array[Double] = null

  def predict(X: Vector): Int = {
    require(this.mid > 0, "predict() is called before training ")
    var dproxy: Long = -1
    val fs = FrovedisServer.getServerInstance()
    val isDense = X.getClass.toString() matches ".*DenseVector*."
    if (isDense) {
      val nrow: Long = 1
      val ncol: Long = X.size
      val rmjr_arr = X.toArray
      dproxy = JNISupport.loadFrovedisWorkerRmajorData(fs.master_node, nrow,
                                                       ncol, rmjr_arr)
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
    val pred = JNISupport.doSingleGMMPredict(fs.master_node, dproxy,
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

  def predict(X: FrovedisRowmajorMatrix): RDD[Int] = {
    require(this.mid > 0, "predict() is called before training ")
    val fs = FrovedisServer.getServerInstance()
    val isDense = true
    val res = JNISupport.doParallelGMMPredict(fs.master_node, X.get(),
                                              this.mid, isDense)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val context = SparkContext.getOrCreate()
    return context.parallelize(res)
  }

  def predict(X: FrovedisSparseData): RDD[Int] = {
    require(this.mid > 0, "predict() is called before training ")
    val fs = FrovedisServer.getServerInstance()
    val isDense = false
    val res = JNISupport.doParallelGMMPredict(fs.master_node, X.get(),
                                            this.mid, isDense)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val context = SparkContext.getOrCreate()
    return context.parallelize(res)
  }


  private def getMeans(): Array[Double] = {
    if(_means == null) {
      val fs = FrovedisServer.getServerInstance()
      _means = JNISupport.getGMMMeans(fs.master_node,this.mid)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
    }
    _means
  }

  private def getWeights(): Array[Double] = {
    if(_weights == null){
      val fs = FrovedisServer.getServerInstance()
      _weights = JNISupport.getGMMWeights(fs.master_node,this.mid)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
    }
    _weights
  }

  private def getCovs(): Array[Double] = {
    if(_covs == null) {
      val fs = FrovedisServer.getServerInstance()
      _covs = JNISupport.getGMMSigma(fs.master_node,this.mid)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
    }
    _covs
  }

  private def getGaussians(): Array[MultivariateGaussian] = {
    if(_gaussians == null) {
      val ncomponents = getWeights().length
      val means = getMeans()
      val nfeatures = means.length / ncomponents
      val covs = getCovs()

      // Populate the array and return
      val cov_nf = nfeatures * nfeatures //to slice out Cov array for each k
      _gaussians = new Array[MultivariateGaussian](ncomponents)
      for(i <- 0 until ncomponents) {
        val mu_array = means.slice(i*nfeatures, (i+1)*nfeatures)
        val mu = Vectors.dense(mu_array)
        val cov = covs.slice(i*cov_nf, (i+1)*cov_nf)
        val sigma = new DenseMatrix(nfeatures, nfeatures, cov, true) //DenseMatrix is colmajor
        val mvg = new MultivariateGaussian(mu, sigma)
        _gaussians(i) = mvg
      }
    }
    _gaussians
  }
}

object GaussianMixtureModel {
  def load(sc: SparkContext, path: String): GaussianMixtureModel = load(path)
  def load(path: String): GaussianMixtureModel = {
    val mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    // load a GaussianMixtureModel from the 'path'
    // and register it with 'model_id' at Frovedis server
    JNISupport.loadFrovedisModel(fs.master_node,mid,M_KIND.GMM,path)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new GaussianMixtureModel(mid)
  }
}
