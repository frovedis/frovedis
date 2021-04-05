package com.nec.frovedis.mllib.regression;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport,MemPair}
import com.nec.frovedis.Jmllib.DummyGLM
import com.nec.frovedis.io.FrovedisIO
import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import com.nec.frovedis.matrix.MAT_KIND
import com.nec.frovedis.mllib.{M_KIND,ModelID}
import org.apache.spark.SparkContext
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.regression.LabeledPoint
import org.apache.spark.mllib.linalg.Vector

class SVRModel(modelId: Int,
               modelKind: Short,
               nftr: Long)
  extends GeneralizedLinearModel(modelId,modelKind,nftr,-1,0) {
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

object SVRModel {  // companion object (for static members)
  def load(path: String) : SVRModel = {
    val exist = FrovedisIO.checkExists(path)
    require(exist, "No model named " + path + " is found!")
    val model_id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    // load an SVRModel from the 'path' 
    // and register it with 'model_id' at Frovedis server
    val ret = JNISupport.loadFrovedisGLM(fs.master_node,model_id,
                                         M_KIND.SVR, path)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new SVRModel(ret)
  }
}

object SVMRegressionWithSGD {
  // Kind of shortcut Spark-like interface for Spark user.
  // Spark data will get converted to Frovedis data and will be freed
  // from Frovedis side memory implicitly, once the training is completed.
  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            regType: String,
            miniBatchFraction: Double,
            loss: String,
            eps: Double,
            isIntercept: Boolean,
            convergenceTol: Double, 
            sample_weight: Array[Double]) : SVRModel = { 
     val fdata = new FrovedisLabeledPoint(data) // Spark Data => Frovedis Data 
     return train(fdata, numIter, stepSize, regParam, regType, 
                  miniBatchFraction, loss, eps, isIntercept, convergenceTol, true, sample_weight)
  }
  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            regType: String,
            miniBatchFraction: Double,
            loss: String,
            eps: Double,
            isIntercept: Boolean,
            convergenceTol: Double) : SVRModel = {
     return train(data, numIter, stepSize, regParam, regType, 
                  miniBatchFraction, loss, eps, isIntercept, convergenceTol, Array.empty[Double])
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            regType: String,
            miniBatchFraction: Double,
            loss: String,
            eps: Double,
            isIntercept: Boolean) : SVRModel = {
     return train(data, numIter, stepSize, regParam, regType, 
                  miniBatchFraction, loss, eps, isIntercept, 0.001, Array.empty[Double])
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            regType: String,
            miniBatchFraction: Double,
            loss: String,
            eps: Double) : SVRModel = {
     return train(data, numIter, stepSize, regParam, regType, 
                  miniBatchFraction, loss, eps, false, 0.001, Array.empty[Double])
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            regType: String,
            miniBatchFraction: Double,
            loss: String) : SVRModel = {
     return train(data, numIter, stepSize, regParam, regType, 
                  miniBatchFraction, loss, 0.0, false, 0.001, Array.empty[Double])
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            regType: String,
            miniBatchFraction: Double) : SVRModel = {
     return train(data, numIter, stepSize, regParam, regType, 
                  miniBatchFraction, "epsilon_insensitive", 0.0, false, 0.001, Array.empty[Double])
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            regType: String) : SVRModel = {
     return train(data, numIter, stepSize, regParam, regType, 1.0, 
                  "epsilon_insensitive", 0.0, false, 0.001, Array.empty[Double])
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            regParam: Double) : SVRModel = {
     return train(data, numIter, stepSize, regParam, "ZERO", 1.0, 
                  "epsilon_insensitive", 0.0, false, 0.001, Array.empty[Double])
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double) : SVRModel = {
     return train(data, numIter, stepSize, 0.01, "ZERO", 1.0, 
                  "epsilon_insensitive", 0.0, false, 0.001, Array.empty[Double])
  }

  def train(data: RDD[LabeledPoint],
            numIter: Int) : SVRModel = {
     return train(data, numIter, 0.01, 0.01, "ZERO", 1.0, 
                  "epsilon_insensitive", 0.0, false, 0.001, Array.empty[Double])
  }

  def train(data: RDD[LabeledPoint]) : SVRModel = {
     return train(data, 1000, 0.01, 0.01, "ZERO", 1.0, 
                  "epsilon_insensitive", 0.0, false, 0.001, Array.empty[Double])
  }

  // User needs to convert the Spark data into Frovedis Data by himself before 
  // calling this interface. In this case, user has to explicitly free the 
  // Frovedis Data when it will no longer be needed to avoid memory leak at Frovedis server.
  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            regType: String,
            miniBatchFraction: Double,
            loss: String,
            eps: Double,
            isIntercept: Boolean,
            convergenceTol: Double,
            isMovableInput: Boolean,
            sample_weight: Array[Double]) : SVRModel = {
    return new LinearSVR().setNumIter(numIter)
                          .setStepSize(stepSize)
                          .setRegParam(regParam)
                          .setRegType(regType)
                          .setMiniBatchFraction(miniBatchFraction)
                          .setLoss(loss)
                          .setEps(eps)
                          .setIsIntercept(isIntercept)
                          .setConvergenceTol(convergenceTol)
                          .run(data, isMovableInput, sample_weight)
  }
  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            regType: String,
            miniBatchFraction: Double,
            loss: String,
            eps: Double,
            isIntercept: Boolean,
            convergenceTol: Double,
            isMovableInput: Boolean) : SVRModel = {
     return train(data, numIter, stepSize, regParam, regType, miniBatchFraction, 
                  loss, eps, isIntercept, convergenceTol, isMovableInput, Array.empty[Double])
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            regType: String,
            miniBatchFraction: Double,
            loss: String,
            eps: Double,
            isIntercept: Boolean,
            convergenceTol: Double) : SVRModel = {
     return train(data, numIter, stepSize, regParam, regType, miniBatchFraction, 
                  loss, eps, isIntercept, convergenceTol, false, Array.empty[Double])
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            regType: String,
            miniBatchFraction: Double,
            loss: String,
            eps: Double,
            isIntercept: Boolean) : SVRModel = {
     return train(data, numIter, stepSize, regParam, regType, miniBatchFraction, 
                  loss, eps, isIntercept, 0.001, false, Array.empty[Double])
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            regType: String,
            miniBatchFraction: Double,
            loss: String,
            eps: Double) : SVRModel = {
     return train(data, numIter, stepSize, regParam, regType, miniBatchFraction, 
                  loss, eps, false, 0.001, false, Array.empty[Double])
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            regType: String,
            miniBatchFraction: Double,
            loss: String) : SVRModel = {
     return train(data, numIter, stepSize, regParam, regType, miniBatchFraction, 
                  loss, 0.0, false, 0.001, false, Array.empty[Double])
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            regType: String,
            miniBatchFraction: Double) : SVRModel = {
     return train(data, numIter, stepSize, regParam, regType, miniBatchFraction, 
                  "epsilon_insensitive", 0.0, false, 0.001, false, Array.empty[Double])
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            regParam: Double,
            regType: String) : SVRModel = {
     return train(data, numIter, stepSize, regParam, regType, 1.0, 
                  "epsilon_insensitive", 0.0, false, 0.001, false, Array.empty[Double])
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double,
            regParam: Double) : SVRModel = {
     return train(data, numIter, stepSize, regParam, "ZERO", 1.0, 
                  "epsilon_insensitive", 0.0, false, 0.001, false, Array.empty[Double])
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int,
            stepSize: Double) : SVRModel = {
     return train(data, numIter, stepSize, 0.01, "ZERO", 1.0, 
                  "epsilon_insensitive", 0.0, false, 0.001, false, Array.empty[Double])
  }

  def train(data: FrovedisLabeledPoint,
            numIter: Int) : SVRModel = {
     return train(data, numIter, 0.01, 0.01, "ZERO", 1.0, 
                  "epsilon_insensitive", 0.0, false, 0.001, false, Array.empty[Double])
  }

  def train(data: FrovedisLabeledPoint) : SVRModel = {
     return train(data, 1000, 0.01, 0.01, "ZERO", 1.0, 
                  "epsilon_insensitive", 0.0, false, 0.001, false, Array.empty[Double])
  }
}

class LinearSVR(var numIter: Int, 
                var stepSize: Double,
                var regParam: Double, 
                var regType: String,
                var miniBatchFraction: Double, 
                var loss: String,
                var eps: Double, 
                var isIntercept: Boolean,
                var convergenceTol: Double){
  def this() = this(1000, 0.01, 0.01, "ZERO", 1.0, "epsilon_insensitive", 0.0, false, 0.001)

  def setNumIter(numIter: Int): this.type = {
    if(numIter <= 0) throw new IllegalStateException(
      s"numIter must be positive but got ${numIter}")
    this.numIter = numIter
    return this
  }

  def getNumIter() = this.numIter

  def setStepSize(stepSize: Double): this.type = {
    if(stepSize <= 0) throw new IllegalStateException(
      s"stepSize must be zero or positive but got ${stepSize}")
    this.stepSize = stepSize
    return this
  }

  def getStepSize() = this.stepSize

  def setRegParam(regParam: Double): this.type = {
    if(regParam < 0) throw new IllegalStateException(
      s"regParam must be positive but got ${regParam}")
    this.regParam = regParam
    return this
  }

  def getRegParam() = this.regParam

  def setRegType(regType: String): this.type = {
    require(Set("ZERO", "L1", "L2").contains(regType),
          s"Given invalid loss for LinearSVR: $regType." +
          s"  Valid regType: ZERO, L1 and L2 ")
    this.regType = regType
    return this
  }

  def getRegType() = this.regType

  def setMiniBatchFraction(mbf: Double): this.type = {
    if(mbf < 0) throw new IllegalStateException(
      s"miniBatchFraction must be positive but got ${mbf}")
    this.miniBatchFraction = mbf
    return this
  }

  def getMiniBatchFraction() = this.miniBatchFraction

  def setLoss(lossType: String): this.type = {
    require(Set("epsilon_insensitive", "squared_epsilon_insensitive").contains(lossType),
          s"Given invalid loss for LinearSVR: $lossType." +
          s"  Valid lossType: epsilon_insensitive and squared_epsilon_insensitive ")
    this.loss = lossType
    return this
  }

  def getLoss() = this.loss

  def setEps(eps: Double): this.type = {
    if(eps <= 0) throw new IllegalStateException(
      s"eps must be zero or positive but got ${eps}")
    this.eps = eps
    return this
  }

  def getEps() = this.eps

  def setIsIntercept(isIntercept: Boolean): this.type = {
    this.isIntercept = isIntercept
    return this
  }

  def getIsIntercept() = this.isIntercept

  def setConvergenceTol(convergenceTol: Double): this.type = {
    if(convergenceTol < 0) throw new IllegalStateException(
      s"convergenceTol must be positive but got ${convergenceTol}")
    this.convergenceTol = convergenceTol
    return this
  }

  def getConvergenceTol() = this.convergenceTol

  def run(input: RDD[LabeledPoint]): SVRModel= {
    val fdata = new FrovedisLabeledPoint(input)
    return run(fdata, true, Array.empty[Double])
  }

  def run(fdata: FrovedisLabeledPoint): SVRModel= {
    return run(fdata, false, Array.empty[Double])
  }

  def run(input: RDD[LabeledPoint],
          sample_weight: Array[Double]): SVRModel= {
    val fdata = new FrovedisLabeledPoint(input)
    return run(fdata, true, sample_weight)
  }

  def run(fdata: FrovedisLabeledPoint,
          sample_weight: Array[Double]): SVRModel= {
    return run(fdata, false, sample_weight)
  }

  def run(data: FrovedisLabeledPoint, movable: Boolean,
          sample_weight: Array[Double]): SVRModel = {
    if (data.is_dense() && data.matType() != MAT_KIND.CMJR) 
       throw new IllegalArgumentException(
        s"fit: please provide column major "+
        s"points as for dense data to frovedis linear svm regression!\n")
    val mid = ModelID.get()
    val sample_weight_length = sample_weight.length

    val fs = FrovedisServer.getServerInstance()
    JNISupport.callFrovedisSVR(fs.master_node,data.get(),numIter,stepSize,
                               miniBatchFraction,regParam,regType,loss,
                               eps,isIntercept,convergenceTol,
                               mid,movable,data.is_dense(),sample_weight, sample_weight_length)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val numFeatures = data.numCols()
    return new SVRModel(mid,M_KIND.SVR,numFeatures)
  }
}

