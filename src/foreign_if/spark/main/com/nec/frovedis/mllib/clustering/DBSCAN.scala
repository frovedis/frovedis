package com.nec.frovedis.mllib.clustering;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.mllib.{ModelID, M_KIND}
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.Vector
import org.apache.spark.SparkContext

class DBSCAN(var eps: Double,
             var min_samples: Int,
             var metric: String,
             var algorithm: String) extends java.io.Serializable {
  private var mid: Int = 0
  def this() = this(0.5, 5, "euclidean", "brute")

  def setEps(eps: Double): this.type = {
    require(eps > 0.0 ,
      s"eps must be greater than 0 but got  ${eps}")
    this.eps = eps
    this
  }

  def setMinSamples(min_samples: Int): this.type = {
    require(min_samples > 0 ,
      s"min_samples must be greater than 0 but got  ${min_samples}")
    this.min_samples = min_samples
    this
  }

  def setMetric(metric: String): this.type = {
    require(metric == "euclidean" ,
      s"Currently Frovedis DBSCAN does not support  ${metric} metric!")
    this.metric = metric
    this
  }

  def setAlgorithm(algorithm: String): this.type = {
    require(algorithm == "brute" ,
      s"Currently Frovedis DBSCAN does not support  ${algorithm} algorithm!")
    this.algorithm = algorithm
    this
  }

  def run(data: RDD[Vector]): Array[Int] = {
    val isDense = data.first.getClass.toString() matches ".*DenseVector*."
    if (isDense) {
      val fdata = new FrovedisRowmajorMatrix(data)
      return run(fdata, true)
    }
    else {
      val fdata = new FrovedisSparseData(data)
      return run(fdata, true)
    }
  } 

  def run(data: FrovedisSparseData): Array[Int] = {
    return run(data, false)
  }

  def run(data: FrovedisSparseData,
          movable: Boolean): Array[Int] = {
    this.mid = ModelID.get()
    val isDense: Boolean = false
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.callFrovedisDBSCAN(fs.master_node,
                                            data.get(),
                                            eps,min_samples,
                                            mid,isDense)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return ret
  }

  def run(data: FrovedisRowmajorMatrix): Array[Int] = {
    return run(data, false)
  }

  def run(data: FrovedisRowmajorMatrix,
          movable: Boolean): Array[Int] = {
    this.mid = ModelID.get()
    val isDense : Boolean = true
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.callFrovedisDBSCAN(fs.master_node,
                                            data.get(),
                                            eps,min_samples,
                                            mid,isDense)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return ret
  }

  def release(): Unit = {
    if (mid != 0) {
      val fs = FrovedisServer.getServerInstance()
      JNISupport.releaseFrovedisModel(fs.master_node, mid, M_KIND.DBSCAN)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
    }
  }
}

object DBSCAN{
  //  --- RDD ---
  def train(data: RDD[Vector] , 
            eps: Double, 
            min_samples: Int,
            metric: String,
            algorithm: String): Array[Int] = {
     return new DBSCAN()
               .setEps(eps)
               .setMinSamples(min_samples)
               .setMetric(metric)
               .setAlgorithm(algorithm)
               .run(data)
  }

  def train(data: RDD[Vector] , 
            eps: Double, 
            min_samples: Int,
            metric: String): Array[Int] = {
    return train(data,eps,min_samples,metric,"brute")
  }

  def train(data: RDD[Vector] , 
            eps: Double, 
            min_samples: Int): Array[Int] = {
    return train(data,eps,min_samples,"euclidean","brute")
  }

  def train(data: RDD[Vector] ,
            eps: Double): Array[Int] = {
    return train(data,eps,5,"euclidean","brute")
  }

  // --- SparseData Train ---
  def train(data: FrovedisSparseData,
            eps: Double, 
            min_samples: Int,
            metric: String,
            algorithm: String): Array[Int] = {
    return new DBSCAN()
            .setEps(eps)
            .setMinSamples(min_samples)
            .setMetric(metric)
            .setAlgorithm(algorithm)
            .run(data)
  }

  def train(data: FrovedisSparseData,
            eps: Double,
            min_samples: Int,
            metric: String): Array[Int] = {
    return train(data,eps,min_samples,metric,"brute")
  }

  def train(data: FrovedisSparseData,
            eps: Double,
            min_samples: Int): Array[Int] = {
    return train(data,eps,min_samples,"euclidean","brute")
  }

  def train(data: FrovedisSparseData,
            eps: Double): Array[Int] = {
    return train(data,eps,5,"euclidean","brute")
  }

  // --- RowmajorData Train --- 
  def train(data: FrovedisRowmajorMatrix,
            eps: Double, 
            min_samples: Int,
            metric: String,
            algorithm: String): Array[Int] = {
    return new DBSCAN()
            .setEps(eps)
            .setMinSamples(min_samples)
            .setMetric(metric)
            .setAlgorithm(algorithm)
            .run(data)
  }
  def train(data: FrovedisRowmajorMatrix,
            eps: Double,
            min_samples: Int,
            metric: String): Array[Int] = {
    return train(data,eps,min_samples,metric,"brute")
  }
  def train(data: FrovedisRowmajorMatrix,
            eps: Double,
            min_samples: Int): Array[Int] = {
    return train(data,eps,min_samples,"euclidean","brute")
  }
  def train(data: FrovedisRowmajorMatrix,
            eps: Double): Array[Int] = {
    return train(data,eps,5,"euclidean","brute")
  }
}

