package com.nec.frovedis.mllib.clustering;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModel}
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.matrix.ScalaCRS
import com.nec.frovedis.io.FrovedisIO
import org.apache.spark.SparkContext
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.{Vector, Vectors, DenseMatrix}
import org.apache.spark.mllib.stat.distribution.MultivariateGaussian

class GaussianMixtureModel(modelId: Int, niter: Int, lb: Double)
                           extends GenericModel(modelId, M_KIND.GMM) {
  private var _niter: Int = niter
  private var _lb: Double = lb
  private var _weights: Array[Double] = null
  private var _gaussians: Array[MultivariateGaussian] = null

  def n_iter: Int = _niter
  def lower_bound: Double = _lb
  def k: Int = getWeights().length          // model dimension
  def weights: Array[Double] = getWeights() // model weights
  def gaussians: Array[MultivariateGaussian] = getGaussians() // model gaussians

  override def save(path: String): Unit = {
    val context = SparkContext.getOrCreate()
    save(context, path)
  }
  override def save(sc: SparkContext, path: String): Unit = {
    val success = FrovedisIO.createDir(path)
    require(success, "Another model named " + path + " already exists!")
    super.save(path + "/model")
    val metadata: Map[String, Double] = Map("niter" -> n_iter.toDouble, 
                                            "lb" -> lower_bound) 
    FrovedisIO.saveDictionary(sc, metadata, path + "/metadata")
  }

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
      val scalaCRS = new ScalaCRS(Array(X), X.size)
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

  def predictSoft(X: Vector): Array[Double] = {
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
      val scalaCRS = new ScalaCRS(Array(X), X.size)
      dproxy = JNISupport.loadFrovedisWorkerData(fs.master_node,
                                                 scalaCRS.nrows,
                                                 scalaCRS.ncols,
                                                 scalaCRS.off,
                                                 scalaCRS.idx,
                                                 scalaCRS.data)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
    }
    val pred = JNISupport.doSingleGMMPredictProba(fs.master_node, dproxy,
                                                  this.mid, isDense)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return pred
  }

  def predictSoft(X: RDD[Vector]): RDD[Array[Double]] = {
    require(this.mid > 0, "predict() is called before training ")
    val isDense = X.first.getClass.toString() matches ".*DenseVector*."
    if (isDense) {
      val fdata = new FrovedisRowmajorMatrix(X)
      val res = predictSoft(fdata)
      fdata.release()
      return res
    }
    else {
      val fdata = new FrovedisSparseData(X)
      val res = predictSoft(fdata)
      fdata.release()
      return res
    }
  }

  def predictSoft(X: FrovedisRowmajorMatrix): RDD[Array[Double]] = {
    require(this.mid > 0, "predict() is called before training ")
    val fs = FrovedisServer.getServerInstance()
    val isDense = true
    val res = JNISupport.doParallelGMMPredictProba(fs.master_node, X.get(),
                                                   this.mid, isDense)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    //NOTE: scala for loops doesn't support Long types
    val sz = X.numRows().asInstanceOf[Int]  
    val probs = new Array[Array[Double]](sz)
    for(i <- 0 until sz) {
      val prob = res.slice(i*k, (i+1)*k)
      probs(i) = prob  
    }  
    val context = SparkContext.getOrCreate()
    return context.parallelize(probs)
  }

  def predictSoft(X: FrovedisSparseData): RDD[Array[Double]] = {
    require(this.mid > 0, "predict() is called before training ")
    val fs = FrovedisServer.getServerInstance()
    val isDense = false
    val res = JNISupport.doParallelGMMPredictProba(fs.master_node, X.get(),
                                            this.mid, isDense)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    //NOTE: scala for loops doesn't support Long types
    val sz = X.numRows().asInstanceOf[Int]  
    val probs = new Array[Array[Double]](sz)
    for(i <- 0 until sz) {
      val prob = res.slice(i*k, (i+1)*k)
      probs(i) = prob  
    }  
    val context = SparkContext.getOrCreate()
    return context.parallelize(probs)
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

  private def getMeans(): Array[Double] = {
    val fs = FrovedisServer.getServerInstance()
    val _means = JNISupport.getGMMMeans(fs.master_node,this.mid)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return _means
  }

  private def getCovs(): Array[Double] = {
    val fs = FrovedisServer.getServerInstance()
    val _covs = JNISupport.getGMMSigma(fs.master_node,this.mid)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return _covs
  }

  private def getGaussians(): Array[MultivariateGaussian] = {
    if(_gaussians == null) {
      val ncomponents = getWeights().length
      _gaussians = new Array[MultivariateGaussian](ncomponents)
      val means = getMeans()
      val nfeatures = means.length / ncomponents
      val covs = getCovs()
      val cov_nf = nfeatures * nfeatures //to slice out Cov array for each k
      for(i <- 0 until ncomponents) {
        val mu_array = means.slice(i * nfeatures, (i + 1) * nfeatures)
        val mu = Vectors.dense(mu_array)
        val cov = covs.slice(i * cov_nf, (i + 1) * cov_nf)
        val sigma = new DenseMatrix(nfeatures, nfeatures, cov, true) //DenseMatrix is colmajor
        val mvg = new MultivariateGaussian(mu, sigma)
        _gaussians(i) = mvg
      }
    }
    return _gaussians
  }
}

object GaussianMixtureModel {
  def load(path: String): GaussianMixtureModel = {
    val context = SparkContext.getOrCreate()
    load(context, path)
  }
  def load(sc: SparkContext, 
           path: String): GaussianMixtureModel = {
    val mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    // load a GaussianMixtureModel from the 'path'
    // and register it with 'model_id' at Frovedis server
    JNISupport.loadFrovedisModel(fs.master_node, mid, M_KIND.GMM, path + "/model")
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    val metadata = FrovedisIO.loadDictionary[String, Double](sc, path + "/metadata")
    return new GaussianMixtureModel(mid, metadata("niter").toInt, metadata("lb"))
  }
}
