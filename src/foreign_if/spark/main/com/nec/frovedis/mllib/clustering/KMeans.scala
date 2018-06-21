package com.nec.frovedis.mllib.clustering;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModel}
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
            epsilon: Double): KMeansModel = {
    val fdata = new FrovedisSparseData(data) // Spark Data => Frovedis Data
    return train(fdata, k, iterations, seed, epsilon, true)
  }

  def train(data: RDD[Vector],
            k: Int,
            iterations: Int,
            seed: Long): KMeansModel = {
    return train(data, k, iterations, seed, 0.01)
  }

  def train(data: RDD[Vector],
            k: Int,
            iterations: Int): KMeansModel = {
    return train(data, k, iterations, 0, 0.01)
  }

  def train(data: RDD[Vector],
            k: Int): KMeansModel = {
    return train(data, k, 100, 0, 0.01)
  }

  // User needs to convert the Spark data into Frovedis Data by himself before
  // calling this interface. In this case, user has to explicitly free the
  // Frovedis Data when it will no longer be needed to avoid memory leak at Frovedis server.
  def train(data: FrovedisSparseData,
            k: Int,
            iterations: Int,
            seed: Long,
            epsilon: Double,
            isMovableInput: Boolean): KMeansModel = {
    val mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    JNISupport.callFrovedisKMeans(fs.master_node,data.get(),k,
                                iterations,seed,epsilon,
                                mid,isMovableInput)
    return new KMeansModel(mid,M_KIND.KMEANS,k)
  }

  def train(data: FrovedisSparseData,
            k: Int,
            iterations: Int,
            seed: Long,
            epsilon: Double): KMeansModel = {
    return train(data, k, iterations, seed, epsilon, false)
  }

  def train(data: FrovedisSparseData,
            k: Int,
            iterations: Int,
            seed: Long): KMeansModel = {
    return train(data, k, iterations, seed, 0.01, false)
  }

  def train(data: FrovedisSparseData,
            k: Int,
            iterations: Int): KMeansModel = {
    return train(data, k, iterations, 0, 0.01, false)
  }

  def train(data: FrovedisSparseData,
            k: Int): KMeansModel = {
    return train(data, k, 100, 0, 0.01, false)
  }
}
