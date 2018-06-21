package com.nec.frovedis.mllib.regression;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.exrpc.FrovedisSparseGLMData
import com.nec.frovedis.mllib.{M_KIND,ModelID}
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.regression.LabeledPoint

// --------------------------------------------------------------------------
// NOTE: Like Spark, We have used multiple train methods instead of default
//       arguments to support Java programs.
// Default values are taken as in Frovedis (not as in Spark)
//   numIter = 1000            (Spark: 100)
//   stepSize = 0.01           (Spark: 1.0)
//   miniBatchFraction = 1.0   (Spark: 1.0)
//   histSize = 10             (Spark: 10)  (corrections)
//   regParam = 0.01           (Spark: 0.01)
// --------------------------------------------------------------------------
object RidgeRegressionWithSGD {
  // Kind of shortcut Spark-like interface for Spark user.
  // Spark data will get converted to Frovedis data and will be freed
  // from Frovedis side memory implicitly, once the training is completed.
  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            miniBatchFraction: Double) : LinearRegressionModel = { 
     val fdata = new FrovedisSparseGLMData(data) // Spark Data => Frovedis Data 
     return train(fdata, numIter, stepSize, regParam, miniBatchFraction, true)
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            regParam: Double) : LinearRegressionModel = {
     return train(data, numIter, stepSize, regParam, 1.0)
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double) : LinearRegressionModel = {
     return train(data, numIter, stepSize, 0.01, 1.0)
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int) : LinearRegressionModel = {
     return train(data, numIter, 0.01, 0.01, 1.0)
  }

  def train(data: RDD[LabeledPoint]) : LinearRegressionModel = {
     return train(data, 1000, 0.01, 0.01, 1.0)
  }

  // User needs to convert the Spark data into Frovedis Data by himself before 
  // calling this interface. In this case, user has to explicitly free the 
  // Frovedis Data when it will no longer be needed to avoid memory leak at Frovedis server.
  def train(data: FrovedisSparseGLMData,
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            miniBatchFraction: Double,
            isMovableInput: Boolean) : LinearRegressionModel = {
     val mid = ModelID.get()
     val fs = FrovedisServer.getServerInstance()
     JNISupport.callFrovedisRidgeSGD(fs.master_node,data.get(),numIter,stepSize,
                                   miniBatchFraction,regParam,mid,isMovableInput)
     val numFeatures = data.numCols()
     val intercept = 0.0 // assumed (To-Do: Support isIntercept, as in Frovedis)
     return new LinearRegressionModel(mid,M_KIND.LNRM,numFeatures,intercept)
  }

  def train(data: FrovedisSparseGLMData,
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            miniBatchFraction: Double) : LinearRegressionModel = {
     return train(data, numIter, stepSize, regParam, miniBatchFraction, false)
  }

  def train(data: FrovedisSparseGLMData,
            numIter: Int,
            stepSize: Double,
            regParam: Double) : LinearRegressionModel = {
     return train(data, numIter, stepSize, regParam, 1.0, false)
  }

  def train(data: FrovedisSparseGLMData,
            numIter: Int,
            stepSize: Double) : LinearRegressionModel = {
     return train(data, numIter, stepSize, 0.01, 1.0, false)
  }

  def train(data: FrovedisSparseGLMData,
            numIter: Int) : LinearRegressionModel = {
     return train(data, numIter, 0.01, 0.01, 1.0, false)
  }

  def train(data: FrovedisSparseGLMData) : LinearRegressionModel = {
     return train(data, 1000, 0.01, 0.01, 1.0, false)
  }
}

object RidgeRegressionWithLBFGS {
  // Kind of shortcut Spark-like interface for Spark user.
  // Spark data will get converted to Frovedis data and will be freed
  // from Frovedis side memory implicitly, once the training is completed.
  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            histSize: Int) : LinearRegressionModel = {
     val fdata = new FrovedisSparseGLMData(data) // Spark Data => Frovedis Data 
     return train(fdata, numIter, stepSize, regParam, histSize, true)
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            regParam: Double) : LinearRegressionModel = {
     return train(data, numIter, stepSize, regParam, 10)
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double) : LinearRegressionModel = {
     return train(data, numIter, stepSize, 0.01, 10)
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int) : LinearRegressionModel = {
     return train(data, numIter, 0.01, 0.01, 10)
  }

  def train(data: RDD[LabeledPoint]) : LinearRegressionModel = {
     return train(data, 1000, 0.01, 0.01, 10)
  }

  // User needs to convert the Spark data into Frovedis Data by himself before 
  // calling this interface. In this case, user has to explicitly free the 
  // Frovedis Data when it will no longer be needed to avoid memory leak at Frovedis server.
  def train(data: FrovedisSparseGLMData,
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            histSize: Int,
            isMovableInput: Boolean) : LinearRegressionModel = {
     val mid = ModelID.get()
     val fs = FrovedisServer.getServerInstance()
     JNISupport.callFrovedisRidgeLBFGS(fs.master_node,data.get(),numIter,stepSize,
                                     histSize,regParam,mid,isMovableInput)
     val numFeatures = data.numCols()
     val intercept = 0.0 // assumed (To-Do: Support isIntercept, as in Frovedis)
     return new LinearRegressionModel(mid,M_KIND.LNRM,numFeatures,intercept)
  }

  def train(data: FrovedisSparseGLMData,
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            histSize: Int) : LinearRegressionModel = {
     return train(data, numIter, stepSize, regParam, histSize, false)
  }

  def train(data: FrovedisSparseGLMData,
            numIter: Int,
            stepSize: Double,
            regParam: Double) : LinearRegressionModel = {
     return train(data, numIter, stepSize, regParam, 10, false)
  }

  def train(data: FrovedisSparseGLMData,
            numIter: Int,
            stepSize: Double) : LinearRegressionModel = {
     return train(data, numIter, stepSize, 0.01, 10, false)
  }

  def train(data: FrovedisSparseGLMData,
            numIter: Int) : LinearRegressionModel = {
     return train(data, numIter, 0.01, 0.01, 10, false)
  }

  def train(data: FrovedisSparseGLMData) : LinearRegressionModel = {
     return train(data, 1000, 0.01, 0.01, 10, false)
  }
}
