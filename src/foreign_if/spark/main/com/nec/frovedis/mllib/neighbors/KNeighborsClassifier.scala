package com.nec.frovedis.mllib.neighbors;

import com.nec.frovedis.Jexrpc.{FrovedisServer, JNISupport}
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.matrix.MAT_KIND
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
    if (data.is_dense() && data.matType() != MAT_KIND.RMJR) 
       throw new IllegalArgumentException(
        s"fit: please provide row-major "+
        s"points as for dense data to frovedis kneighbor classifier!\n")
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
    else {
      val fdata = new FrovedisSparseData(X)
      val (dist, ind) = kneighbors(fdata, nNeighbors, returnDistance) 
      fdata.release() // releasing intermediate matrix
      val context = X.context
      if (returnDistance) 
        return (dist.to_spark_RowMatrix(context),
                ind.to_spark_RowMatrix(context))
      else 
        return (null, ind.to_spark_RowMatrix(context)) 
    }
  }

  // dense kneighbors()
  def kneighbors(X: FrovedisRowmajorMatrix, 
                 nNeighbors: Int,
                 returnDistance: Boolean): 
    (FrovedisRowmajorMatrix, FrovedisRowmajorMatrix) = {
    require(mid > 0, "kneighbors() is called before fitting data using run()")
    val fs = FrovedisServer.getServerInstance()
    val knn_res = JNISupport.kncKneighbors(fs.master_node, X.get(), nNeighbors,
                                           mid, returnDistance, true) 
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val distances = new FrovedisRowmajorMatrix(knn_res.distances_ptr,
                                               knn_res.nrow_dist,
                                               knn_res.ncol_dist)
    val indices = new FrovedisRowmajorMatrix(knn_res.indices_ptr,
                                             knn_res.nrow_ind,
                                             knn_res.ncol_ind)
    if (returnDistance) 
      return (distances, indices)
    else 
      return (null, indices)
  }

  // sparse kneighbors()
  def kneighbors(X: FrovedisSparseData, 
                 nNeighbors: Int,
                 returnDistance: Boolean): 
    (FrovedisRowmajorMatrix, FrovedisRowmajorMatrix) = {
    require(mid > 0, "kneighbors() is called before fitting data using run()")
    val fs = FrovedisServer.getServerInstance()
    val knn_res = JNISupport.kncKneighbors(fs.master_node, X.get(), nNeighbors,
                                           mid, returnDistance, false) 
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val distances = new FrovedisRowmajorMatrix(knn_res.distances_ptr,
                                               knn_res.nrow_dist,
                                               knn_res.ncol_dist)
    val indices = new FrovedisRowmajorMatrix(knn_res.indices_ptr,
                                             knn_res.nrow_ind,
                                             knn_res.ncol_ind)
    if (returnDistance) 
      return (distances, indices)
    else 
      return (null, indices)
  }

  def kneighbors_graph(X: RDD[Vector], 
                       nNeighbors: Int = this.nNeighbors,
                       mode: String = "connectivity"): RDD[Vector] = {
    require(mid > 0, "kneighbors_graph() is called before fitting data using run()")
    val isDense = X.first.getClass.toString() matches ".*DenseVector*."
    if (isDense) {
      val fdata = new FrovedisRowmajorMatrix(X)
      val graph = kneighbors_graph(fdata, nNeighbors, mode)
      fdata.release() // release intermediate matrix
      return graph.to_spark_sparse_matrix()
    }
    else {
      val fdata = new FrovedisSparseData(X)
      val graph = kneighbors_graph(fdata, nNeighbors, mode)
      fdata.release() // release intermediate matrix
      return graph.to_spark_sparse_matrix()
    }
  }

  // dense kneighbors_graph()
  def kneighbors_graph(X: FrovedisRowmajorMatrix, 
                       nNeighbors: Int, 
                       mode: String): FrovedisSparseData = {
    require(mid > 0, "kneighbors_graph() is called before fitting data using run()")
    val fs = FrovedisServer.getServerInstance()
    var graph = JNISupport.kncKneighborsGraph(fs.master_node, X.get(),
                                              nNeighbors, mid, mode, true) //dummy mat: crs 
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = new FrovedisSparseData(graph)
    return ret
  }

  // sparse kneighbors_graph()
  def kneighbors_graph(X: FrovedisSparseData, 
                       nNeighbors: Int, 
                       mode: String): FrovedisSparseData = {
    require(mid > 0, "kneighbors_graph() is called before fitting data using run()")
    val fs = FrovedisServer.getServerInstance()
    val graph = JNISupport.kncKneighborsGraph(fs.master_node, X.get(),
                                              nNeighbors, mid, mode, false) //dummy mat: crs 
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = new FrovedisSparseData(graph)
    return ret
  }

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
    else {
      val fdata = new FrovedisSparseData(X)
      val res = predict(fdata, saveProba)
      fdata.release()
      return res
    }
  }
  
  // dense predict()
  def predict(X: FrovedisRowmajorMatrix, 
              saveProba: Boolean): Array[Double] = {
    require(mid > 0, "predict() is called before fitting data using run()")
    val fs = FrovedisServer.getServerInstance()
    val res: Array[Double] = JNISupport.kncDoublePredict(fs.master_node, X.get(),
                              mid, saveProba, true) // double array
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return res
  }
  def predict(X: FrovedisRowmajorMatrix): Array[Double] = {
    return predict(X, false)
  } 

  // sparse predict()
  def predict(X: FrovedisSparseData, 
              saveProba: Boolean): Array[Double] = {
    require(mid > 0, "predict() is called before fitting data using run()")
    val fs = FrovedisServer.getServerInstance()
    val res: Array[Double] = JNISupport.kncDoublePredict(fs.master_node, X.get(),
                              mid, saveProba, false) // double array
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return res
  }
  def predict(X: FrovedisSparseData): Array[Double] = {
    return predict(X, false)
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
    else {
      val fdata = new FrovedisSparseData(X)
      val res = predict_proba(fdata)
      fdata.release()
      val context = X.context
      return res.to_spark_RowMatrix(context)
    }
  }

  // dense predict_proba()
  def predict_proba(X: FrovedisRowmajorMatrix): FrovedisRowmajorMatrix = {
    require(mid > 0, "predict_proba() is called before fitting data using run()")
    val fs = FrovedisServer.getServerInstance()
    val dmat = JNISupport.kncPredictProba(fs.master_node, X.get(), mid, true)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = new FrovedisRowmajorMatrix(dmat)
    return ret
  }
  
  // sparse predict_proba()
  def predict_proba(X: FrovedisSparseData): FrovedisRowmajorMatrix = {
    require(mid > 0, "predict_proba() is called before fitting data using run()")
    val fs = FrovedisServer.getServerInstance()
    val dmat = JNISupport.kncPredictProba(fs.master_node, X.get(), mid, false)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = new FrovedisRowmajorMatrix(dmat)
    return ret
  }

  def score(data: RDD[LabeledPoint]): Float = {
    require(mid > 0, "score() is called before fitting data using run()")
    val y = data.map(_.label)
    val x = data.map(_.features)
    val yptr = DoubleDvector.get(y)
    val isDense = x.first.getClass.toString() matches ".*DenseVector*."
    var res : Float = scala.Float.NegativeInfinity
    if (isDense){
      val x_ = new FrovedisRowmajorMatrix(x)
      res = score(x_, yptr)
      x_.release()  
    }
    else{
      val x_ = new FrovedisSparseData(x)
      val res = score(x_, yptr)
      x_.release()
    }
    return res
  }

  // dense score()
  def score(data: FrovedisRowmajorMatrix, 
            labelPtr: Long): Float = { 
    require(mid > 0, "score() is called before fitting data using run()")
    val fs = FrovedisServer.getServerInstance()
    val res = JNISupport.kncModelScore(fs.master_node,
                                       data.get(), labelPtr, mid, true)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return res
  }
  
  // sparse score()
  def score(data: FrovedisSparseData, 
            labelPtr: Long): Float = { 
    require(mid > 0, "score() is called before fitting data using run()")
    val fs = FrovedisServer.getServerInstance()
    val res = JNISupport.kncModelScore(fs.master_node,
                                       data.get(), labelPtr, mid, false)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
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

