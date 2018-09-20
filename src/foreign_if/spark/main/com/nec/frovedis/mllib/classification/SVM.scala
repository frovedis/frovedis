package com.nec.frovedis.mllib.classification;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.Jmllib.DummyGLM
import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import com.nec.frovedis.mllib.{M_KIND,ModelID}
import com.nec.frovedis.mllib.regression.GeneralizedLinearModel
import org.apache.spark.SparkContext
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.regression.LabeledPoint

class SVMModel(modelId: Int,
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

object SVMModel {  // companion object (for static members)
  def load(sc: SparkContext, path: String) : SVMModel = load(path)
  def load(path: String) : SVMModel = {
    val model_id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    // load an SVMModel from the 'path' 
    // and register it with 'model_id' at Frovedis server
    val ret = JNISupport.loadFrovedisGLM(fs.master_node,model_id,M_KIND.SVM,path)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new SVMModel(ret)
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

object SVMWithSGD {
  // Kind of shortcut Spark-like interface for Spark user.
  // Spark data will get converted to Frovedis data and will be freed
  // from Frovedis side memory implicitly, once the training is completed.
  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            miniBatchFraction: Double) : SVMModel = { 
     val fdata = new FrovedisLabeledPoint(data) // Spark Data => Frovedis Data 
     return train(fdata, numIter, stepSize, regParam, miniBatchFraction, true)
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            regParam: Double) : SVMModel = {
     return train(data, numIter, stepSize, regParam, 1.0)
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double) : SVMModel = {
     return train(data, numIter, stepSize, 0.01, 1.0)
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int) : SVMModel = {
     return train(data, numIter, 0.01, 0.01, 1.0)
  }

  def train(data: RDD[LabeledPoint]) : SVMModel = {
     return train(data, 1000, 0.01, 0.01, 1.0)
  }

  // User needs to convert the Spark data into Frovedis Data by himself before 
  // calling this interface. In this case, user has to explicitly free the 
  // Frovedis Data when it will no longer be needed to avoid memory leak at Frovedis server.
  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            miniBatchFraction: Double,
            isMovableInput: Boolean) : SVMModel = {
     val mid = ModelID.get()
     val fs = FrovedisServer.getServerInstance()
     JNISupport.callFrovedisSVMSGD(fs.master_node,data.get(),numIter,stepSize,
                                   miniBatchFraction,regParam,mid,isMovableInput,
                                   data.is_dense())
     val info = JNISupport.checkServerException();
     if (info != "") throw new java.rmi.ServerException(info);
     val numFeatures = data.numCols()
     val intercept = 0.0 // assumed (To-Do: Support isIntercept, as in Frovedis)
     val numClasses = 2  // Currently Frovedis supports binary classification only
     val threshold = 0.0 // default
     return new SVMModel(mid,M_KIND.SVM,numFeatures,numClasses,intercept,threshold)
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            miniBatchFraction: Double) : SVMModel = {
     return train(data, numIter, stepSize, regParam, miniBatchFraction, false)
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            regParam: Double) : SVMModel = {
     return train(data, numIter, stepSize, regParam, 1.0, false)
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double) : SVMModel = {
     return train(data, numIter, stepSize, 0.01, 1.0, false)
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int) : SVMModel = {
     return train(data, numIter, 0.01, 0.01, 1.0, false)
  }

  def train(data: FrovedisLabeledPoint) : SVMModel = {
     return train(data, 1000, 0.01, 0.01, 1.0, false)
  }
}

object SVMWithLBFGS {
  // Kind of shortcut Spark-like interface for Spark user.
  // Spark data will get converted to Frovedis data and will be freed
  // from Frovedis side memory implicitly, once the training is completed.
  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            histSize: Int) : SVMModel = {
     val fdata = new FrovedisLabeledPoint(data) // Spark Data => Frovedis Data 
     return train(fdata, numIter, stepSize, regParam, histSize, true)
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            regParam: Double) : SVMModel = {
     return train(data, numIter, stepSize, regParam, 10)
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double) : SVMModel = {
     return train(data, numIter, stepSize, 0.01, 10)
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int) : SVMModel = {
     return train(data, numIter, 0.01, 0.01, 10)
  }

  def train(data: RDD[LabeledPoint]) : SVMModel = {
     return train(data, 1000, 0.01, 0.01, 10)
  }

  // User needs to convert the Spark data into Frovedis Data by himself before 
  // calling this interface. In this case, user has to explicitly free the 
  // Frovedis Data when it will no longer be needed to avoid memory leak at Frovedis server.
  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            histSize: Int,
            isMovableInput: Boolean) : SVMModel = {
     val mid = ModelID.get()
     val fs = FrovedisServer.getServerInstance()
     JNISupport.callFrovedisSVMLBFGS(fs.master_node,data.get(),numIter,stepSize,
                                   histSize,regParam,mid,isMovableInput,
                                   data.is_dense())
     val info = JNISupport.checkServerException();
     if (info != "") throw new java.rmi.ServerException(info);
     val numFeatures = data.numCols()
     val intercept = 0.0 // assumed (To-Do: Support isIntercept, as in Frovedis)
     val numClasses = 2  // Currently Frovedis supports binary classification only
     val threshold = 0.0 // default
     return new SVMModel(mid,M_KIND.SVM,numFeatures,numClasses,intercept,threshold)
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            histSize: Int) : SVMModel = {
     return train(data, numIter, stepSize, regParam, histSize, false)
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            regParam: Double) : SVMModel = {
     return train(data, numIter, stepSize, regParam, 10, false)
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double) : SVMModel = {
     return train(data, numIter, stepSize, 0.01, 10, false)
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int) : SVMModel = {
     return train(data, numIter, 0.01, 0.01, 10, false)
  }

  def train(data: FrovedisLabeledPoint) : SVMModel = {
     return train(data, 1000, 0.01, 0.01, 10, false)
  }
}
