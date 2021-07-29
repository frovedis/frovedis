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
             var algorithm: String,
             var batchFraction: Double) extends java.io.Serializable {
  private var mid: Int = 0
  def this() = this(0.5, 5, "euclidean", "brute", Double.MaxValue)

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

  def setBatchFraction(batchFraction: Double): this.type = {
    require(batchFraction > 0.0 && batchFraction <= 1.0,
      s"batchFraction must be in between 0 and 1 but got ${batchFraction}")
    this.batchFraction = batchFraction
    this
  }
    
  def run(data: RDD[Vector]): Array[Int] = {
    val isDense = data.first.getClass.toString() matches ".*DenseVector*."
    if (isDense) {
      val fdata = new FrovedisRowmajorMatrix(data)
      return run(fdata, true, Array.empty[Double])
    }
    else {
      val fdata = new FrovedisSparseData(data)
      return run(fdata, true, Array.empty[Double])
    }
  } 

  def run(data: FrovedisSparseData): Array[Int] = {
    return run(data, false, Array.empty[Double])
  }

  def run(data: RDD[Vector], 
          sample_weight: Array[Double]): Array[Int] = {
    val isDense = data.first.getClass.toString() matches ".*DenseVector*."
    if (isDense) {
      val fdata = new FrovedisRowmajorMatrix(data)
      return run(fdata, true, sample_weight)
    }
    else {
      val fdata = new FrovedisSparseData(data)
      return run(fdata, true, sample_weight)
    }
  } 
  def run(data: FrovedisSparseData, 
          sample_weight: Array[Double]): Array[Int] = {
    return run(data, false, sample_weight)
  }

  def run(data: FrovedisSparseData,
          movable: Boolean, 
          sample_weight: Array[Double]): Array[Int] = {
    this.mid = ModelID.get()
    val isDense: Boolean = false
    val sample_weight_length = sample_weight.length
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.callFrovedisDBSCAN(fs.master_node,
                                            data.get(),
                                            eps,batchFraction,
                                            min_samples,
                                            mid,isDense,
                                            sample_weight,
                                            sample_weight_length)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return ret
  }
  def run(data: FrovedisRowmajorMatrix): Array[Int] = {
    return run(data, false, Array.empty[Double])
  }

  def run(data: FrovedisRowmajorMatrix, 
          sample_weight: Array[Double]): Array[Int] = {
    return run(data, false, sample_weight)
  }

  def run(data: FrovedisRowmajorMatrix,
          movable: Boolean,
          sample_weight: Array[Double]): Array[Int] = {
    this.mid = ModelID.get()
    val isDense : Boolean = true
    val sample_weight_length = sample_weight.length
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.callFrovedisDBSCAN(fs.master_node,
                                            data.get(),
                                            eps,batchFraction,
                                            min_samples,
                                            mid,isDense,
                                            sample_weight,
                                            sample_weight_length)
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
            algorithm: String,
            sample_weight: Array[Double]): Array[Int] = {
     return new DBSCAN()
               .setEps(eps)
               .setMinSamples(min_samples)
               .setMetric(metric)
               .setAlgorithm(algorithm)
               .run(data, sample_weight)
  }

  def train(data: RDD[Vector] , 
            eps: Double, 
            min_samples: Int,
            metric: String,
            algorithm: String): Array[Int] = {
    return train(data,eps,min_samples,metric,algorithm,Array.empty[Double])
  }
  def train(data: RDD[Vector] , 
            eps: Double, 
            min_samples: Int,
            metric: String): Array[Int] = {
    return train(data,eps,min_samples,metric,"brute",Array.empty[Double])
  }

  def train(data: RDD[Vector] , 
            eps: Double, 
            min_samples: Int): Array[Int] = {
    return train(data,eps,min_samples,"euclidean","brute",Array.empty[Double])
  }

  def train(data: RDD[Vector] ,
            eps: Double): Array[Int] = {
    return train(data,eps,5,"euclidean","brute",Array.empty[Double])
  }

  // --- SparseData Train ---
  def train(data: FrovedisSparseData,
            eps: Double, 
            min_samples: Int,
            metric: String,
            algorithm: String,
            sample_weight: Array[Double]): Array[Int] = {
    return new DBSCAN()
            .setEps(eps)
            .setMinSamples(min_samples)
            .setMetric(metric)
            .setAlgorithm(algorithm)
            .run(data, sample_weight)
  }

  def train(data: FrovedisSparseData,
            eps: Double,
            min_samples: Int,
            metric: String,
            algorithm: String): Array[Int] = {
    return train(data,eps,min_samples,metric,algorithm,Array.empty[Double])
  }

  def train(data: FrovedisSparseData,
            eps: Double,
            min_samples: Int): Array[Int] = {
    return train(data,eps,min_samples,"euclidean","brute",Array.empty[Double])
  }

  def train(data: FrovedisSparseData,
            eps: Double): Array[Int] = {
    return train(data,eps,5,"euclidean","brute",Array.empty[Double])
  }

  // --- RowmajorData Train --- 
  def train(data: FrovedisRowmajorMatrix,
            eps: Double, 
            min_samples: Int,
            metric: String,
            algorithm: String,
            sample_weight: Array[Double]): Array[Int] = {
    return new DBSCAN()
            .setEps(eps)
            .setMinSamples(min_samples)
            .setMetric(metric)
            .setAlgorithm(algorithm)
            .run(data, sample_weight)
  }
  def train(data: FrovedisRowmajorMatrix,
            eps: Double,
            min_samples: Int,
            metric: String,
            algorithm: String): Array[Int] = {
    return train(data,eps,min_samples,metric,algorithm,Array.empty[Double])
  }
  def train(data: FrovedisRowmajorMatrix,
            eps: Double,
            min_samples: Int,
            metric: String): Array[Int] = {
    return train(data,eps,min_samples,metric,"brute",Array.empty[Double])
  }
  def train(data: FrovedisRowmajorMatrix,
            eps: Double,
            min_samples: Int): Array[Int] = {
    return train(data,eps,min_samples,"euclidean","brute",Array.empty[Double])
  }
  def train(data: FrovedisRowmajorMatrix,
            eps: Double): Array[Int] = {
    return train(data,eps,5,"euclidean","brute",Array.empty[Double])
  }
}

