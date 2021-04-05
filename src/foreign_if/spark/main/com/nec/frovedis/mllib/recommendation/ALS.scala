package com.nec.frovedis.mllib.recommendation;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModel}
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.Vector
import org.apache.spark.mllib.recommendation.Rating

// --------------------------------------------------------------------------
// NOTE: Like Spark, We have used multiple trainImplicit methods instead of default
//       arguments to support Java programs.
// Default values are taken as in Frovedis (not as in Spark)
//   iterations = 100     (Spark: 10)
//   lambda = 0.01        (Spark: 0.01)
//   alpha = 0.01         (Spark: 1.0)
//   seed = 0             (Spark: System.nanoTime())  
// --------------------------------------------------------------------------
object ALS {
  // Kind of shortcut Spark-like interface for Spark user. 
  // Input spark Rating data will get converted to Frovedis crs data and will be freed
  // from Frovedis side memory implicitly, once the training is completed.
  def trainImplicit(ratings: RDD[Rating],
            rank: Int,
            iterations: Int,
            lambda: Double,
            alpha: Double,
            sim_factor: Double,                    
            seed: Long): MatrixFactorizationModel = {
    val fdata = new FrovedisSparseData()
    fdata.loadcoo(ratings) // Spark Rating Data (coo) => Frovedis crs Data
    return trainImplicit(fdata, rank, iterations, lambda, alpha, sim_factor, seed, true);
  }

  def trainImplicit(ratings: RDD[Rating],
            rank: Int,
            iterations: Int,
            lambda: Double,        
            alpha: Double,
            sim_factor: Double): MatrixFactorizationModel = {
    return trainImplicit(ratings, rank, iterations, lambda, alpha, sim_factor, 0);
  }

  def trainImplicit(ratings: RDD[Rating],
            rank: Int,
            iterations: Int,
            lambda: Double,
            alpha: Double): MatrixFactorizationModel = {
    return trainImplicit(ratings, rank, iterations, lambda, alpha, 0.1, 0);
  }

  def trainImplicit(ratings: RDD[Rating],
            rank: Int,
            iterations: Int,
            lambda: Double): MatrixFactorizationModel = {
    return trainImplicit(ratings, rank, iterations, lambda, 0.01, 0.1, 0);
  }

  def trainImplicit(ratings: RDD[Rating],
            rank: Int,
            iterations: Int): MatrixFactorizationModel = {
    return trainImplicit(ratings, rank, iterations, 0.01, 0.01, 0.1, 0);
  }

  def trainImplicit(ratings: RDD[Rating],
            rank: Int): MatrixFactorizationModel = {
    return trainImplicit(ratings, rank, 100, 0.01, 0.01, 0.1, 0);
  }    
    
/*
  // Kind of shortcut Spark-like interface for Spark user. Instead of "Rating" (coo),
  // it accepts "Vector" (crs) type input data (actually SparseVector).
  // Spark data will get converted to Frovedis data and will be freed
  // from Frovedis side memory implicitly, once the trainImpliciting is completed.
  def trainImplicit(ratings: RDD[Vector],
            rank: Int,
            iterations: Int,
            lambda: Double,
            alpha: Double,
            seed: Long): MatrixFactorizationModel = {
    val fdata = new FrovedisSparseData(ratings) // Spark Data => Frovedis Data
    return trainImplicit(fdata, rank, iterations, lambda, alpha, seed, true);
  }

  def trainImplicit(ratings: RDD[Vector],
            rank: Int,
            iterations: Int,
            lambda: Double,
            alpha: Double): MatrixFactorizationModel = {
    return trainImplicit(ratings, rank, iterations, lambda, alpha, 0);
  }

  def trainImplicit(ratings: RDD[Vector],
            rank: Int,
            iterations: Int,
            lambda: Double): MatrixFactorizationModel = {
    return trainImplicit(ratings, rank, iterations, lambda, 0.01, 0);
  }

  def trainImplicit(ratings: RDD[Vector],
            rank: Int,
            iterations: Int): MatrixFactorizationModel = {
    return trainImplicit(ratings, rank, iterations, 0.01, 0.01, 0);
  }

  def trainImplicit(ratings: RDD[Vector],
            rank: Int): MatrixFactorizationModel = {
    return trainImplicit(ratings, rank, 100, 0.01, 0.01, 0);
  }
*/

  // User needs to convert the Spark data into Frovedis Data by himself before
  // calling this interface. In this case, user has to explicitly free the
  // Frovedis Data when it will no longer be needed to avoid memory leak at Frovedis server.
  def trainImplicit(data: FrovedisSparseData,
            rank: Int,
            iterations: Int,
            lambda: Double,
            alpha: Double,
            sim_factor: Double,        
            seed: Long,
            isMovableInput: Boolean): MatrixFactorizationModel = {
    val mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    JNISupport.callFrovedisMFUsingALS(fs.master_node,data.get(),rank,
                                    iterations,alpha,sim_factor,lambda,seed,
                                    mid,isMovableInput)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new MatrixFactorizationModel(mid,M_KIND.MFM,rank)
  }

  def trainImplicit(data: FrovedisSparseData,
            rank: Int,
            iterations: Int,
            lambda: Double,
            alpha: Double,
            sim_factor: Double,        
            seed: Long): MatrixFactorizationModel = {
    return trainImplicit(data, rank, iterations, lambda, alpha, sim_factor, seed, false);
  }
 
  def trainImplicit(data: FrovedisSparseData,
            rank: Int,
            iterations: Int,
            lambda: Double,
            alpha: Double,
            sim_factor: Double): MatrixFactorizationModel = {
    return trainImplicit(data, rank, iterations, lambda, alpha, sim_factor, 0, false);
  } 

  def trainImplicit(data: FrovedisSparseData,
            rank: Int,
            iterations: Int,
            lambda: Double,
            alpha: Double): MatrixFactorizationModel = {
    return trainImplicit(data, rank, iterations, lambda, alpha, 0.1, 0, false);
  } 

  def trainImplicit(data: FrovedisSparseData,
            rank: Int,
            iterations: Int,
            lambda: Double): MatrixFactorizationModel = {
    return trainImplicit(data, rank, iterations, lambda, 0.01, 0.1, 0, false);
  } 

  def trainImplicit(data: FrovedisSparseData,
            rank: Int,
            iterations: Int): MatrixFactorizationModel = {
    return trainImplicit(data, rank, iterations, 0.01, 0.01, 0.1, 0, false);
  }
    
  def trainImplicit(data: FrovedisSparseData,
            rank: Int): MatrixFactorizationModel = {
    return trainImplicit(data, rank, 100, 0.01, 0.01, 0.1, 0, false);
  }    
}
