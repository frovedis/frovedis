package com.nec.frovedis.mllib.clustering;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.mllib.{M_KIND,ModelID}
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.Vector

// --------------------------------------------------------------------------
// NOTE: Like Spark, We have used multiple train methods instead of default
//       arguments to support Java programs.
// Default values are taken as in Frovedis (not as in Spark)
//   iterations = 100     (Spark: 100)
//   seed = 0             (Spark: Utils.random.nextLong())  
//   epsilon = 0.01       (Spark: 0.0001)
// --------------------------------------------------------------------------
object KMeans {
  // Kind of shortcut Spark-like interface for Spark user. 
  // Spark data will get converted to Frovedis data and will be freed
  // from Frovedis side memory implicitly, once the training is completed.
  def train(data: RDD[Vector],
            k: Int,
            iterations: Int,
            seed: Long,
            epsilon: Double,
            use_shrink: Boolean): KMeansModel = {
    val movable = true
    val isDense = data.first.getClass.toString() matches ".*DenseVector*."
    if (isDense) {
      val fdata = new FrovedisRowmajorMatrix(data)
      return train(fdata, k, iterations, seed, epsilon, use_shrink, movable)
    }
    else {
      val fdata = new FrovedisSparseData(data)
      return train(fdata, k, iterations, seed, epsilon, use_shrink, movable)
    }
  }
  def train(data: RDD[Vector],
            k: Int,
            iterations: Int,
            seed: Long,
            epsilon: Double): KMeansModel = {
    return train(data, k, iterations, seed, epsilon, false)
  }
  def train(data: RDD[Vector],
            k: Int,
            iterations: Int,
            seed: Long): KMeansModel = {
    return train(data, k, iterations, seed, 0.01, false)
  }
  def train(data: RDD[Vector],
            k: Int,
            iterations: Int): KMeansModel = {
    return train(data, k, iterations, 0, 0.01, false)
  }
  def train(data: RDD[Vector],
            k: Int): KMeansModel = {
    return train(data, k, 100, 0, 0.01, false)
  }

  // User needs to convert the Spark data into Frovedis Data by himself before
  // calling this interface. In this case, user has to explicitly free the
  // Frovedis Data when it will no longer be needed to avoid memory leak at Frovedis server.
  def train(data: FrovedisSparseData,
            k: Int,
            iterations: Int,
            seed: Long,
            epsilon: Double,
            use_shrink: Boolean,
            isMovableInput: Boolean): KMeansModel = {
    val isDense = false
    val mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    JNISupport.callFrovedisKMeans(fs.master_node,data.get(),k,
                                iterations,seed,epsilon,
                                mid,isMovableInput,isDense,use_shrink)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new KMeansModel(mid,M_KIND.KMEANS,k)
  }
  def train(data: FrovedisSparseData,
            k: Int,
            iterations: Int,
            seed: Long,
            epsilon: Double,
            use_shrink: Boolean): KMeansModel = {
    val movable = false
    return train(data, k, iterations, seed, epsilon, use_shrink, movable)
  }
  def train(data: FrovedisSparseData,
            k: Int,
            iterations: Int,
            seed: Long,
            epsilon: Double): KMeansModel = {
    val movable = false
    return train(data, k, iterations, seed, epsilon, false, movable)
  }
  def train(data: FrovedisSparseData,
            k: Int,
            iterations: Int,
            seed: Long): KMeansModel = {
    val movable = false
    return train(data, k, iterations, seed, 0.01, false, movable)
  }
  def train(data: FrovedisSparseData,
            k: Int,
            iterations: Int): KMeansModel = {
    val movable = false
    return train(data, k, iterations, 0, 0.01, false, movable)
  }
  def train(data: FrovedisSparseData,
            k: Int): KMeansModel = {
    val movable = false
    return train(data, k, 100, 0, 0.01, false, movable)
  }
  
  //added for dense matrix support 
  def train(data: FrovedisRowmajorMatrix,
            k: Int,
            iterations: Int,
            seed: Long,
            epsilon: Double,
            use_shrink: Boolean,
            isMovableInput: Boolean): KMeansModel = {
    val isDense = true
    val mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    JNISupport.callFrovedisKMeans(fs.master_node,data.get(),k,
                                iterations,seed,epsilon,
                                mid,isMovableInput,isDense,use_shrink)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new KMeansModel(mid,M_KIND.KMEANS,k)
  } 
  def train(data: FrovedisRowmajorMatrix,
            k: Int,
            iterations: Int,
            seed: Long,
            epsilon: Double,
            use_shrink: Boolean): KMeansModel = {
    val movable = false
    return train(data, k, iterations, seed, epsilon, use_shrink, movable)
  }
  def train(data: FrovedisRowmajorMatrix,
            k: Int,
            iterations: Int,
            seed: Long,
            epsilon: Double): KMeansModel = {
    val movable = false
    return train(data, k, iterations, seed, epsilon, false, movable)
  }
  def train(data: FrovedisRowmajorMatrix,
            k: Int,
            iterations: Int,
            seed: Long): KMeansModel = {
    val movable = false
    return train(data, k, iterations, seed, 0.01, false, movable)
  }
  def train(data: FrovedisRowmajorMatrix,
            k: Int,
            iterations: Int): KMeansModel = {
    val movable = false
    return train(data, k, iterations, 0, 0.01, false, movable)
  }
  def train(data: FrovedisRowmajorMatrix,
            k: Int): KMeansModel = {
    val movable = false
    return train(data, k, 100, 0, 0.01, false, movable)
  }
}
