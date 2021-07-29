package com.nec.frovedis.mllib.neighbors;

import com.nec.frovedis.Jexrpc.{FrovedisServer, JNISupport}
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.mllib.{M_KIND, ModelID}
import com.nec.frovedis.Jmllib.DummyKNNResult
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.Vector
import org.apache.spark.SparkContext
import org.apache.spark.mllib.linalg.distributed.RowMatrix

class NearestNeighbors(var nNeighbors: Int, 
                       var radius: Float, 
                       var algorithm: String,
                       var metric: String,
                       var chunkSize: Float,
                       var batchFraction: Double) extends java.io.Serializable {
  private var mid: Int = 0 
  private var mdense: Boolean = false

  def this() = this(5, 1.0F, "brute", "euclidean", 1.0F, Double.MaxValue)

  def setNNeighbors(nNeighbors: Int): this.type = {
    require(nNeighbors > 0 ,
      s"nNeighbors must be greater than 0 but got  ${nNeighbors}")
    this.nNeighbors = nNeighbors
    this
  }
  
  def setRadius(radius: Float): this.type = {
    require(radius > 0 ,
      s"radius must be greater than 0 but got  ${radius}")
    this.radius = radius
    this
  }

  def setAlgorithm(algorithm: String): this.type = {
    val supported_algorithms: List[String] = List("brute")
    require(supported_algorithms.contains(algorithm) == true ,
      s"Given algorithm: ${algorithm} is not currently supported."+
      " \n Currently supported algorithms: ${supported_algorithms}")
    this.algorithm = algorithm
    this
  }

  def setMetric(metric: String): this.type = {
    val supported_metrics: List[String] = List("euclidean", "seuclidean")
    require(supported_metrics.contains(metric) == true ,
      s"Given metric: ${metric} is not currently supported. \n "+
      "Currently supported metrics: ${supported_metrics}")
    this.metric = metric
    this
  }

  def setChunkSize(chunkSize: Float): this.type = {
    require(chunkSize > 0 ,
      s"chunkSize must be greater than 0 but got  ${chunkSize}")
    this.chunkSize = chunkSize
    this
  }

  def setBatchFraction(batchFraction: Double): this.type = {
    require(batchFraction > 0.0 && batchFraction <= 1.0,
      s"batchFraction must be greater than 0 but got ${batchFraction}")
    this.batchFraction = batchFraction
    this
  }     
    
  def run(data: RDD[Vector]): this.type = {
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

  def run(data: FrovedisRowmajorMatrix): this.type = {
    return run(data, false)
  }

  def run(data: FrovedisSparseData): this.type = {
    return run(data, false)
  }

  def run(data: FrovedisRowmajorMatrix,
          movable: Boolean): this.type = {
    release() // releasing old model (if any)
    this.mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    mdense = true
    JNISupport.callFrovedisKnnFit(fs.master_node,
                               data.get(), nNeighbors,
                               radius, algorithm, metric,
                               chunkSize, batchFraction, 
                               mid, mdense)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return this
  }

  def run(data: FrovedisSparseData,
          movable: Boolean): this.type = {
    release() // releasing old model (if any)
    this.mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    JNISupport.callFrovedisKnnFit(fs.master_node,
                               data.get(), nNeighbors,
                               radius, algorithm, metric,
                               chunkSize, batchFraction, 
                               mid, mdense)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return this
  }

  def kneighbors(X: RDD[Vector], 
                 nNeighbors: Int = this.nNeighbors,
                 returnDistance: Boolean = true): (RowMatrix, RowMatrix)  = {
    require(mid > 0, "kneighbors() is called before fitting data using run()")
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
    val knn_res = JNISupport.knnKneighbors(fs.master_node, X.get(), nNeighbors,
                                           mid, returnDistance, true, mdense)
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
    val knn_res = JNISupport.knnKneighbors(fs.master_node, X.get(), nNeighbors,
                                           mid, returnDistance, false, mdense)
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
    val graph = JNISupport.knnKneighborsGraph(fs.master_node, X.get(), 
                              nNeighbors, mid, mode, true, mdense) //dummy mat: crs 
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
    val graph = JNISupport.knnKneighborsGraph(fs.master_node, X.get(), 
                              nNeighbors, mid, mode, false, mdense) //dummy mat: crs 
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = new FrovedisSparseData(graph)
    return ret
  }

  def radius_neighbors(X: RDD[Vector], 
                       radius: Float = this.radius):
    FrovedisSparseData = {
    require(mid > 0, "radius_neighbors() is called before fitting data using run()")
    val isDense = X.first.getClass.toString() matches ".*DenseVector*."
    if (isDense) {
      val fdata = new FrovedisRowmajorMatrix(X)
      val neighbors = radius_neighbors(fdata, radius)
      fdata.release() // release intermediate matrix
      return neighbors
    }
    else {
      val fdata = new FrovedisSparseData(X)
      val neighbors = radius_neighbors(fdata, radius)
      fdata.release() // release intermediate matrix
      return neighbors
    }
  }

  // dense radius_neighbors
  def radius_neighbors(X: FrovedisRowmajorMatrix, 
                       radius: Float): FrovedisSparseData = {
    require(mid > 0, "radius_neighbors() is called before fitting data using run()")
    val fs = FrovedisServer.getServerInstance()
    val dmat = JNISupport.knnRadiusNeighbors(fs.master_node, X.get(), 
                          radius, mid, true, mdense) //dummy mat: crs 
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = new FrovedisSparseData(dmat)
    return ret 
  }

  // sparse radius_neighbors
  def radius_neighbors(X: FrovedisSparseData, 
                       radius: Float): FrovedisSparseData = {
    require(mid > 0, "radius_neighbors() is called before fitting data using run()")
    val fs = FrovedisServer.getServerInstance()
    val dmat = JNISupport.knnRadiusNeighbors(fs.master_node, X.get(), 
                          radius, mid, false, mdense) //dummy mat: crs 
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = new FrovedisSparseData(dmat)
    return ret 
  }

  def radius_neighbors_graph(X: RDD[Vector], 
                             radius: Float = this.radius,
                             mode: String = "connectivity"): RDD[Vector] = {
    require(mid > 0, "radius_neighbors_graph() is called before fitting data "+
                     "using run()")
    val isDense = X.first.getClass.toString() matches ".*DenseVector*."
    if (isDense) {
      val fdata = new FrovedisRowmajorMatrix(X)
      val graph = radius_neighbors_graph(fdata, radius, mode)
      fdata.release() // release intermediate matrix
      return graph.to_spark_sparse_matrix()
    }
    else {
      val fdata = new FrovedisSparseData(X)
      val graph = radius_neighbors_graph(fdata, radius, mode)
      fdata.release() // release intermediate matrix
      return graph.to_spark_sparse_matrix()
    }
  }

  // dense radius_neighbors_graph
  def radius_neighbors_graph(X: FrovedisRowmajorMatrix, 
                             radius: Float,
                             mode: String): FrovedisSparseData = {
    require(mid > 0, "radius_neighbors_graph() is called before fitting data "+
                     "using run()")
    val fs = FrovedisServer.getServerInstance()
    val graph = JNISupport.knnRadiusNeighborsGraph(fs.master_node, X.get(), 
                                       radius, mid, mode, true, mdense) //dummy mat: crs 
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = new FrovedisSparseData(graph)
    return ret
  }

  // sparse radius_neighbors_graph
  def radius_neighbors_graph(X: FrovedisSparseData, 
                             radius: Float,
                             mode: String): FrovedisSparseData = {
    require(mid > 0, "radius_neighbors_graph() is called before fitting data "+
                     "using run()")
    val fs = FrovedisServer.getServerInstance()
    val graph = JNISupport.knnRadiusNeighborsGraph(fs.master_node, X.get(), 
                                       radius, mid, mode, false, mdense) //dummy mat: crs 
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = new FrovedisSparseData(graph)
    return ret
  }

  def release(): Unit = {
    if (mid != 0) {
      val fs = FrovedisServer.getServerInstance()
      JNISupport.releaseFrovedisModelKNN(fs.master_node, mid, M_KIND.KNN, mdense)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
    }
  }
}
