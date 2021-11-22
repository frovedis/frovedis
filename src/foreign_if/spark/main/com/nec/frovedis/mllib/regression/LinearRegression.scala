package com.nec.frovedis.mllib.regression;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.Jmllib.DummyGLM
import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import com.nec.frovedis.matrix.MAT_KIND
import com.nec.frovedis.mllib.{M_KIND,ModelID}
import org.apache.spark.SparkContext
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.regression.LabeledPoint

class LinearRegressionModel(modelId: Int,
                            modelKind: Short,
                            nftr: Long) 
  extends GeneralizedLinearModel(modelId,modelKind,nftr,0,1.0) {
  def this(m: DummyGLM) = this(m.mid, m.mkind, m.numFeatures)
  override def toString() : String = {
    val str = s"numFeatures: " + numFeatures  
    return str
  }
  // accessing threshold related routines of this model would cause runtime exception
  override private[this] def getThreshold(): Double = 1.0
  override private[this] def setThreshold(thr: Double) : Unit = { }
  override private[this] def clearThreshold() : Unit = { }
}

object LinearRegressionModel {  // companion object (for static members)
  def load(sc: SparkContext, path: String) : LinearRegressionModel = load(path)
  def load(path: String) : LinearRegressionModel = {
    val model_id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    // load a LinearRegressionModel from the 'path' 
    // and register it with 'model_id' at Frovedis server
    val ret = JNISupport.loadFrovedisGLM(fs.master_node,model_id,M_KIND.LNRM,path)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new LinearRegressionModel(ret)
  }
}

// --------------------------------------------------------------------------
// NOTE: Like Spark, We have used multiple train methods instead of default
//       arguments to support Java programs.
// Default values are taken as in Frovedis (not as in Spark)
//   numIter = 1000            (Spark: 100)
//   stepSize = 0.01           (Spark: 1.0)
//   miniBatchFraction = 1.0   (Spark: 1.0)
//   histSize = 10             (Spark: 10)  (corrections)
// --------------------------------------------------------------------------

object LinearRegressionWithSGD {
  // Kind of shortcut Spark-like interface for Spark user.
  // Spark data will get converted to Frovedis data and will be freed
  // from Frovedis side memory implicitly, once the training is completed.
  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            miniBatchFraction: Double, 
            sample_weight: Array[Double]) : LinearRegressionModel = { 
     val fdata = new FrovedisLabeledPoint(data) // Spark Data => Frovedis Data 
     return train(fdata, numIter, stepSize, miniBatchFraction, true, sample_weight)
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            miniBatchFraction: Double) : LinearRegressionModel = {
     return train(data, numIter, stepSize, miniBatchFraction, Array.empty[Double])
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double) : LinearRegressionModel = {
     return train(data, numIter, stepSize, 1.0, Array.empty[Double])
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int) : LinearRegressionModel = {
     return train(data, numIter, 0.01, 1.0, Array.empty[Double])
  }

  def train(data: RDD[LabeledPoint]) : LinearRegressionModel = {
     return train(data, 1000, 0.01, 1.0, Array.empty[Double])
  }

  // User needs to convert the Spark data into Frovedis Data by himself before 
  // calling this interface. In this case, user has to explicitly free the 
  // Frovedis Data when it will no longer be needed to avoid memory leak at Frovedis server.
  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            miniBatchFraction: Double,
            isMovableInput: Boolean,
            sample_weight: Array[Double]) : LinearRegressionModel = {
     if (data.is_dense() && data.matType() != MAT_KIND.CMJR) 
       throw new IllegalArgumentException(
        s"fit: please provide column major "+
        s"points as for dense data to frovedis linear regression!\n")
     val mid = ModelID.get()
     val sample_weight_length = sample_weight.length
     val fs = FrovedisServer.getServerInstance()
     JNISupport.callFrovedisLNR(fs.master_node,data.get(),numIter,stepSize,
                                10,miniBatchFraction,mid,isMovableInput,
                                data.is_dense(), sample_weight, 
                                sample_weight_length, "sgd", false)
     val info = JNISupport.checkServerException()
     if (info != "") throw new java.rmi.ServerException(info)
     val numFeatures = data.numCols()
     return new LinearRegressionModel(mid,M_KIND.LNRM,numFeatures)
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            miniBatchFraction: Double,
            isMovableInput: Boolean) : LinearRegressionModel = {
     return train(data, numIter, stepSize, miniBatchFraction, 
                  isMovableInput, Array.empty[Double])
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            miniBatchFraction: Double) : LinearRegressionModel = {
     return train(data, numIter, stepSize, miniBatchFraction, false, Array.empty[Double])
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double) : LinearRegressionModel = {
     return train(data, numIter, stepSize, 1.0, false, Array.empty[Double])
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int) : LinearRegressionModel = {
     return train(data, numIter, 0.01, 1.0, false, Array.empty[Double])
  }

  def train(data: FrovedisLabeledPoint) : LinearRegressionModel = {
     return train(data, 1000, 0.01, 1.0, false, Array.empty[Double])
  }
}

object LinearRegressionWithLBFGS {
  // Kind of shortcut Spark-like interface for Spark user.
  // Spark data will get converted to Frovedis data and will be freed
  // from Frovedis side memory implicitly, once the training is completed.

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            histSize: Int,
            sample_weight: Array[Double]) : LinearRegressionModel = {
     val fdata = new FrovedisLabeledPoint(data) // Spark Data => Frovedis Data 
     return train(fdata, numIter, stepSize, histSize, true, sample_weight)
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            histSize: Int) : LinearRegressionModel = {
     return train(data, numIter, stepSize, histSize, Array.empty[Double])
  }


  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double) : LinearRegressionModel = {
     return train(data, numIter, stepSize, 10, Array.empty[Double])
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int) : LinearRegressionModel = {
     return train(data, numIter, 0.01, 10, Array.empty[Double])
  }

  def train(data: RDD[LabeledPoint]) : LinearRegressionModel = {
     return train(data, 1000, 0.01, 10, Array.empty[Double])
  }

  // User needs to convert the Spark data into Frovedis Data by himself before 
  // calling this interface. In this case, user has to explicitly free the 
  // Frovedis Data when it will no longer be needed to avoid memory leak at Frovedis server.
  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            histSize: Int,
            isMovableInput: Boolean,
            sample_weight: Array[Double]) : LinearRegressionModel = {
     if (data.is_dense() && data.matType() != MAT_KIND.CMJR) 
       throw new IllegalArgumentException(
        s"fit: please provide column major "+
        s"points as for dense data to frovedis linear regression!\n")
     val mid = ModelID.get()
     val sample_weight_length = sample_weight.length
     val fs = FrovedisServer.getServerInstance()
     JNISupport.callFrovedisLNR(fs.master_node,data.get(),numIter,
                                stepSize,histSize,0.1,
                                mid,isMovableInput,
                                data.is_dense(), sample_weight,
                                sample_weight_length, "lbfgs", false)
     val info = JNISupport.checkServerException()
     if (info != "") throw new java.rmi.ServerException(info)
     val numFeatures = data.numCols()
     return new LinearRegressionModel(mid,M_KIND.LNRM,numFeatures)
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            histSize: Int,
            isMovableInput: Boolean) : LinearRegressionModel = {
     return train(data, numIter, stepSize, histSize, 
                  isMovableInput, Array.empty[Double])
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            histSize: Int) : LinearRegressionModel = {
     return train(data, numIter, stepSize, histSize, false, Array.empty[Double])
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double) : LinearRegressionModel = {
     return train(data, numIter, stepSize, 10, false, Array.empty[Double])
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int) : LinearRegressionModel = {
     return train(data, numIter, 0.01, 10, false, Array.empty[Double])
  }

  def train(data: FrovedisLabeledPoint) : LinearRegressionModel = {
     return train(data, 1000, 0.01, 10, false, Array.empty[Double])
  }
}
