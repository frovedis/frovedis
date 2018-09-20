package com.nec.frovedis.mllib.classification;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.Jmllib.DummyGLM
import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import com.nec.frovedis.mllib.{M_KIND,ModelID}
import com.nec.frovedis.mllib.regression.GeneralizedLinearModel
import org.apache.spark.SparkContext
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.regression.LabeledPoint

class LogisticRegressionModel(modelId: Int,
                              modelKind: Short,
                              nftr: Long,
                              ncls: Int,
                              icpt: Double,
                              thr: Double) 
  extends GeneralizedLinearModel(modelId,modelKind,nftr,ncls,icpt,thr) {
  def this(m: DummyGLM) = {
    this(m.mid, m.mkind, m.numFeatures, m.numClasses, m.intercept, m.threshold)
  }
}

object LogisticRegressionModel {  // companion object (for static members)
  def load(sc: SparkContext, path: String) : LogisticRegressionModel = load(path)
  def load(path: String) : LogisticRegressionModel = {
    val model_id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    // load a LogisticRegressionModel from the 'path' 
    // and register it with 'model_id' at Frovedis server
    val ret = JNISupport.loadFrovedisGLM(fs.master_node,model_id,M_KIND.LRM,path)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info); 
    return new LogisticRegressionModel(ret) 
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
//   regParam = 0.01           (Spark: 0.01)
// --------------------------------------------------------------------------

object LogisticRegressionWithSGD {
  // Kind of shortcut Spark-like interface for Spark user.
  // Spark data will get converted to Frovedis data and will be freed
  // from Frovedis side memory implicitly, once the training is completed.
  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            miniBatchFraction: Double,
            regParam: Double) : LogisticRegressionModel = {
     val fdata = new FrovedisLabeledPoint(data) // Spark Data => Frovedis Data 
     return train(fdata, numIter, stepSize, miniBatchFraction, regParam, true)
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            miniBatchFraction: Double) : LogisticRegressionModel = {
     return train(data, numIter, stepSize, miniBatchFraction, 0.01)
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double) : LogisticRegressionModel = {
     return train(data, numIter, stepSize, 1.0, 0.01)
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int) : LogisticRegressionModel = {
     return train(data, numIter, 0.01, 1.0, 0.01)
  }

  def train(data: RDD[LabeledPoint]) : LogisticRegressionModel = {
     return train(data, 1000, 0.01, 1.0, 0.01)
  }

  // User needs to convert the Spark data into Frovedis Data by himself before 
  // calling this interface. In this case, user has to explicitly free the 
  // Frovedis Data when it will no longer be needed to avoid memory leak at Frovedis server.
  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            miniBatchFraction: Double,
            regParam: Double,
            isMovableInput: Boolean) : LogisticRegressionModel = {
     val mid = ModelID.get()
     val fs = FrovedisServer.getServerInstance()
     JNISupport.callFrovedisLRSGD(fs.master_node,data.get(),numIter,stepSize,
                                miniBatchFraction,regParam,mid,isMovableInput,
                                data.is_dense())
     val info = JNISupport.checkServerException();
     if (info != "") throw new java.rmi.ServerException(info);
     val numFeatures = data.numCols()
     val intercept = 0.0 // assumed (To-Do: Support isIntercept, as in Frovedis)
     val numClasses = 2  // Currently Frovedis supports binary classification only
     val threshold = 0.5 // default
     return new LogisticRegressionModel(mid,M_KIND.LRM,numFeatures,numClasses,intercept,threshold)
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            miniBatchFraction: Double,
            regParam: Double) : LogisticRegressionModel = {
     return train(data, numIter, stepSize, miniBatchFraction, regParam, false)
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            miniBatchFraction: Double) : LogisticRegressionModel = {
     return train(data, numIter, stepSize, miniBatchFraction, 0.01, false)
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double) : LogisticRegressionModel = {
     return train(data, numIter, stepSize, 1.0, 0.01, false)
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int) : LogisticRegressionModel = {
     return train(data, numIter, 0.01, 1.0, 0.01, false)
  }

  def train(data: FrovedisLabeledPoint) : LogisticRegressionModel = {
     return train(data, 1000, 0.01, 1.0, 0.01, false)
  }
}

object LogisticRegressionWithLBFGS {
  // Kind of shortcut Spark-like interface for Spark user.
  // Spark data will get converted to Frovedis data and will be freed
  // from Frovedis side memory implicitly, once the training is completed.
  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            histSize: Int,
            regParam: Double) : LogisticRegressionModel = {
     val fdata = new FrovedisLabeledPoint(data) // Spark Data => Frovedis Data 
     return train(fdata, numIter, stepSize, histSize, regParam, true)
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            histSize: Int) : LogisticRegressionModel = {
     return train(data, numIter, stepSize, histSize, 0.01)
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double) : LogisticRegressionModel = {
     return train(data, numIter, stepSize, 10, 0.01)
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int) : LogisticRegressionModel = {
     return train(data, numIter, 0.01, 10, 0.01)
  }

  def train(data: RDD[LabeledPoint]) : LogisticRegressionModel = {
     return train(data, 1000, 0.01, 10, 0.01)
  }

  // User needs to convert the Spark data into Frovedis Data by himself before 
  // calling this interface. In this case, user has to explicitly free the 
  // Frovedis Data when it will no longer be needed to avoid memory leak at Frovedis server.
  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            histSize: Int,
            regParam: Double,
            isMovableInput: Boolean) : LogisticRegressionModel = {
     val mid = ModelID.get()
     val fs = FrovedisServer.getServerInstance()
     JNISupport.callFrovedisLRLBFGS(fs.master_node,data.get(),numIter,stepSize,
                                  histSize,regParam,mid,isMovableInput,
                                  data.is_dense())
     val info = JNISupport.checkServerException();
     if (info != "") throw new java.rmi.ServerException(info);
     val numFeatures = data.numCols()
     val intercept = 0.0 // assumed (To-Do: Support isIntercept, as in Frovedis)
     val numClasses = 2  // Currently Frovedis supports binary classification only
     val threshold = 0.5 // default
     return new LogisticRegressionModel(mid,M_KIND.LRM,numFeatures,numClasses,intercept,threshold)
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            histSize: Int,
            regParam: Double) : LogisticRegressionModel = {
     return train(data, numIter, stepSize, histSize, regParam, false)
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            histSize: Int) : LogisticRegressionModel = {
     return train(data, numIter, stepSize, histSize, 0.01, false)
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double) : LogisticRegressionModel = {
     return train(data, numIter, stepSize, 10, 0.01, false)
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int) : LogisticRegressionModel = {
     return train(data, numIter, 0.01, 10, 0.01, false)
  }

  def train(data: FrovedisLabeledPoint) : LogisticRegressionModel = {
     return train(data, 1000, 0.01, 10, 0.01, false)
  }
}
