package com.nec.frovedis.mllib.neighbors;

import com.nec.frovedis.Jexrpc.{FrovedisServer, JNISupport}
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.mllib.{M_KIND, ModelID}
import com.nec.frovedis.Jmllib.DummyKNNResult
import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import com.nec.frovedis.matrix.DoubleDvector
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.Vector
import org.apache.spark.SparkContext
import org.apache.spark.mllib.regression.LabeledPoint
import org.apache.spark.mllib.linalg.distributed.RowMatrix

class KNeighborsClassifier(var nNeighbors: Int, 
                           var algorithm: String,
                           var metric: String,
                           var chunkSize: Float) extends java.io.Serializable {
  private var mid: Int = 0
  def this() = this(5, "brute", "euclidean", 1.0F)

  def setNNeighbors(nNeighbors: Int): this.type = {
    require(nNeighbors > 0 ,
      s"nNeighbors must be greater than 0 but got  ${nNeighbors}")
    this.nNeighbors = nNeighbors
    this
  }

  def setAlgorithm(algorithm: String): this.type = {
    val supported_algorithms: List[String] = List("brute")
    require(supported_algorithms.contains(algorithm) == true ,
      s"Given algorithm: ${algorithm} is not currently supported. \n "+
      "Currently supported algorithms: ${supported_algorithms}")
    this.algorithm = algorithm
    this
  }

  def setMetric(metric: String): this.type = {
    val supported_metrics: List[String] = List("euclidean", "seuclidean")
    require(supported_metrics.contains(metric) == true ,
      s"Given metric: ${metric} is not currently supported. "+
      "\n Currently supported metrics: ${supported_metrics}")
    this.metric = metric
    this
  }

  def setChunkSize(chunkSize: Float): this.type = {
    require(chunkSize > 0 ,
      s"chunkSize must be greater than 0 but got  ${chunkSize}")
    this.chunkSize = chunkSize
    this
  }
  
  def run(data: RDD[LabeledPoint]): this.type = {
    val fdata = new FrovedisLabeledPoint(data, true)
    return run(fdata, true)
  }

  def run(data: FrovedisLabeledPoint): this.type = {
    return run(data, false)
  }

  def run(data: FrovedisLabeledPoint,
          movable: Boolean): this.type = {
    release() // releasing previous algorithm object, if any
    this.mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    JNISupport.callFrovedisKncFit(fs.master_node,
                                  data.get(), nNeighbors,
                                  algorithm, metric,
                                  chunkSize, mid, data.is_dense())
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return this
  }

  def kneighbors(X: RDD[Vector], 
                 nNeighbors: Int = this.nNeighbors,
                 returnDistance: Boolean = true): 
    (RowMatrix, RowMatrix)  = {
    require(mid > 0, "kneighbors is called before fitting data using run()")
    val isDense = X.first.getClass.toString() matches ".*DenseVector*."
    if (isDense) {
      val fdata = new FrovedisRowmajorMatrix(X)
      val (dist, ind) = kneighbors(fdata, nNeighbors, returnDistance) 
      fdata.release() // releasing intermediate matrix
      val context = X.context
      if (returnDistance) 
        return (dist.to_spark_RowMatrix(context),
                ind.to_spark_RowMatrix(context))
      else 
        return (null, ind.to_spark_RowMatrix(context)) 
    }
    else { // TODO: add call for FrovedisSparseData, instead of raising exception here
      throw new IllegalArgumentException("Sparse data for KNC"+
                                        " is currently not supported.")
    }
  }

  def kneighbors_graph(X: RDD[Vector], 
                       nNeighbors: Int = this.nNeighbors,
                       mode: String = "connectivity"): FrovedisSparseData = {
    require(mid > 0, "kneighbors_graph() is called before fitting data using run()")
    val isDense = X.first.getClass.toString() matches ".*DenseVector*."
    if (isDense) {
      val fdata = new FrovedisRowmajorMatrix(X)
      val graph = kneighbors_graph(fdata, nNeighbors, mode)
      fdata.release() // release intermediate matrix
      return graph
    }
    else { // TODO: add call for FrovedisSparseData, instead of raising exception here
      throw new IllegalArgumentException("Sparse data for KNC"+
                                        " is currently not supported.")
    }
  }

  def kneighbors(X: FrovedisRowmajorMatrix, 
                 nNeighbors: Int,
                 returnDistance: Boolean): 
    (FrovedisRowmajorMatrix, FrovedisRowmajorMatrix) = {
    require(mid > 0, "kneighbors() is called before fitting data using run()")
    val fs = FrovedisServer.getServerInstance()
    var knn_res = JNISupport.kncKneighbors(fs.master_node, X.get(), nNeighbors,
                                           mid, returnDistance) 
    var distances = new FrovedisRowmajorMatrix(knn_res.distances_ptr,
                                               knn_res.nrow_dist,
                                               knn_res.ncol_dist)
    var indices = new FrovedisRowmajorMatrix(knn_res.indices_ptr,
                                             knn_res.nrow_ind,
                                             knn_res.ncol_ind)
    if (returnDistance) 
      return (distances, indices)
    else 
      return (null, indices)
  }

  def kneighbors_graph(X: FrovedisRowmajorMatrix, 
                       nNeighbors: Int, 
                       mode: String): FrovedisSparseData = {
    require(mid > 0, "kneighbors_graph() is called before fitting data using run()")
    val fs = FrovedisServer.getServerInstance()
    var graph = JNISupport.kncKneighborsGraph(fs.master_node, X.get(),
                                              nNeighbors, mid, mode) //dummy mat: crs 
    var ret = new FrovedisSparseData(graph)
    return ret
  }

  // TODO: Add kneighbors and kneighbors_graph with FrovedisSparseData input as well...

  def predict(X: RDD[Vector], 
              saveProba: Boolean = false): Array[Double] = {
    require(mid > 0, "predict() is called before fitting data using run()")
    val isDense = X.first.getClass.toString() matches ".*DenseVector*."
    if (isDense) {
      val fdata = new FrovedisRowmajorMatrix(X)
      val res = predict(fdata, saveProba)
      fdata.release()
      return res
    }
    else { // TODO: add call for FrovedisSparseData, instead of raising exception here
      throw new IllegalArgumentException("Sparse data for KNC "+
                                         "is currently not supported.")
    }
  }
  
  def predict_proba(X: RDD[Vector]): RowMatrix = {
    require(mid > 0, "predict_proba() is called before fitting data using run()")
    val isDense = X.first.getClass.toString() matches ".*DenseVector*."
    if (isDense) {
      val fdata = new FrovedisRowmajorMatrix(X)
      val res = predict_proba(fdata)
      fdata.release()
      val context = X.context
      return res.to_spark_RowMatrix(context)
    }
    else { // TODO: add call for FrovedisSparseData, instead of raising exception here
      throw new IllegalArgumentException("Sparse data for KNC "+
                                         "is currently not supported.")
    }
  }

  def predict(X: FrovedisRowmajorMatrix, 
              saveProba: Boolean): Array[Double] = {
    require(mid > 0, "predict() is called before fitting data using run()")
    val fs = FrovedisServer.getServerInstance()
    var res: Array[Double] = JNISupport.kncDoublePredict(fs.master_node, X.get(),
                              mid, saveProba) // double array
    return res
  }

  def predict_proba(X: FrovedisRowmajorMatrix): FrovedisRowmajorMatrix = {
    require(mid > 0, "predict_proba() is called before fitting data using run()")
    val fs = FrovedisServer.getServerInstance()
    var dmat = JNISupport.kncPredictProba(fs.master_node, X.get(), mid)
    var ret = new FrovedisRowmajorMatrix(dmat)
    return ret
  }

  // TODO: Add predict and predict_proba with FrovedisSparseData input as well...

  def score(data: RDD[LabeledPoint]): Float = {
    require(mid > 0, "score() is called before fitting data using run()")
    val y = data.map(_.label)
    val x = data.map(_.features)
    val yptr = DoubleDvector.get(y)
    // TODO: check if x is DenseVector, then create RowmajorMatrix, else create Fr`ovedisSparseData
    val x_ = new FrovedisRowmajorMatrix(x)
    val res = score(x_, yptr)
    x_.release()
    return res
  }

  def score(data: FrovedisRowmajorMatrix, 
            labelPtr: Long): Float = { 
    require(mid > 0, "score() is called before fitting data using run()")
    val fs = FrovedisServer.getServerInstance()
    var res = JNISupport.kncModelScore(fs.master_node,
                                       data.get(), labelPtr, mid)
    return res
  }

  def release(): Unit = {
    if (mid != 0) {
      val fs = FrovedisServer.getServerInstance()
      JNISupport.releaseFrovedisModel(fs.master_node, mid, M_KIND.KNC)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
    }
  }
}

