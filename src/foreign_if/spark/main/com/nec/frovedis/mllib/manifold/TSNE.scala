package com.nec.frovedis.mllib.manifold;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.Jmllib.DummyTSNEResult
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.Vector
import org.apache.spark.SparkContext

class TSNE(var nComponent: Int, 
           var perplexity: Double, 
           var earlyExaggeration: Double,
           var learningRate: Double,
           var nIter: Int,
           var nIterWithoutProgress: Int,
           var minGradNorm: Double,
           var metric: String,
           var init: String,
           var verbose: Boolean,
           var method: String) extends java.io.Serializable {

  def this() = this(2,30.0,12.0,200.0,1000,300,1e-7,"euclidean","random",true,"exact")

  def setNumComponent(nComponent: Int): this.type = {
    require(nComponent == 2 ,
      s"Currently Frovedis TSNE supports n_components = 2 but got ${nComponent}")
    this.nComponent = nComponent
    this
  }
  def getNumComponent() = this.nComponent

  def setPerplexity(perplexity: Double): this.type = {
    require(perplexity >= 0 ,
      s"perplexity should be positive but got ${perplexity}")
    this.perplexity = perplexity
    this
  }
  def getPerplexity() = this.perplexity

  def setEarlyExaggeration(earlyExaggeration: Double): this.type = {
    require(earlyExaggeration >= 1.0 ,
      s"earlyExaggeration must be atleast 1 but got ${earlyExaggeration}")
    this.earlyExaggeration = earlyExaggeration
    this
  }
  def getEarlyExaggeration() = this.earlyExaggeration

  def setLearningRate(learningRate: Double): this.type = {
    require(learningRate >= 0 ,
      s"learningRate must be positive but got ${learningRate}")
    this.learningRate = learningRate
    this
  }
  def getLearningRate() = this.learningRate

  def setNumIter(nIter: Int): this.type = {
    require(nIter >= 250 ,
      s"nIter should be atleast 250 but got ${nIter}")
    this.nIter = nIter
    this
  }
  def getNumIter() = this.nIter

  def setNumIterWithoutProgress(nIterWithoutProgress: Int): this.type = {
    this.nIterWithoutProgress = nIterWithoutProgress
    this
  }
  def getNumIterWithoutProgress() = this.nIterWithoutProgress

  def setMinGradNorm(minGradNorm: Double): this.type = {
    this.minGradNorm = minGradNorm
    this
  }
  def getMinGradNorm() = this.minGradNorm

  def setMetric(metric: String): this.type = {
    val supported_metrics: List[String] = List("euclidean", "precomputed")
    require(supported_metrics.contains(metric) == true ,
      s"Given metric: ${metric} is not currently supported. \n "+
      "Currently supported metrics: ${supported_metrics}")
    this.metric = metric
    this
  }
  def getMetric() = this.metric

  def setInit(init: String): this.type = {
    this.init = init
    this
  }
  def getInit() = this.init

  def setVerbose(verbose: Boolean): this.type = {
    this.verbose = verbose
    this
  }
  def getVerbose() = this.verbose

  def setMethod(method: String): this.type = {
    this.method = method
    this
  }
  def getMethod() = this.method

  def run(data: RDD[Vector]): RDD[Vector] = {
    val isDense = data.first.getClass.toString() matches ".*DenseVector*."
    var emb: FrovedisRowmajorMatrix = null
    if (isDense) {
      val fdata = new FrovedisRowmajorMatrix(data)
      emb = run(fdata)
    }
    else {
      val fdata = new FrovedisSparseData(data)
      emb = run(fdata)
    }
    return emb.to_spark_RowMatrix(data.context).rows
  }
  def run(data: FrovedisSparseData): FrovedisRowmajorMatrix = {
    // convert sparse data to row major mat
    return run(data.to_frovedis_rowmajor_matrix()) // Need to define this function
  }
  def run(data: FrovedisRowmajorMatrix): FrovedisRowmajorMatrix = {
    val fs = FrovedisServer.getServerInstance()
    val tsne_res = JNISupport.computeTSNE(fs.master_node,
                                          data.get(),
                                          perplexity,
                                          earlyExaggeration,
                                          minGradNorm,
                                          learningRate,
                                          nComponent,
                                          nIter,
                                          nIterWithoutProgress,
                                          metric,
                                          method,
                                          init,
                                          verbose)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val embedding = new FrovedisRowmajorMatrix(tsne_res.embedding_ptr,
                                                tsne_res.nrow_emb,
                                                tsne_res.ncol_emb)
    println("tSNE converges in " + tsne_res.n_iter_ + " iterations with error : " + tsne_res.kl_divergence_);
    return embedding
  }
}
