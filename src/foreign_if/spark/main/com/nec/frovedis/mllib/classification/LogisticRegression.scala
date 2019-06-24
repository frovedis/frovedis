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
                              thr: Double) 
  extends GeneralizedLinearModel(modelId,modelKind,nftr,ncls,thr) {
  def this(m: DummyGLM) = {
    this(m.mid, m.mkind, m.numFeatures, m.numClasses, m.threshold)
  }
}

object LogisticRegressionModel {  // companion object (for static members)
  def load(sc: SparkContext,
           path: String,
           isMultinomial: Boolean = false) : LogisticRegressionModel = {
    val model_id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    // load a LogisticRegressionModel from the 'path'
    // and register it with 'model_id' at Frovedis server
    var kind = M_KIND.LRM
    if(isMultinomial) kind = M_KIND.MLR
    val ret = JNISupport.loadFrovedisGLM(fs.master_node,model_id,kind,path)
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

class LogisticRegression {
  private var regParam: Double = 0.01
  private var regType: String = "zero"
  private var elasticNetParam: Double = 0.01
  private var threshold: Double = 0.5
  private var maxIter: Int = 1000
  private var tol: Double = 1E-6
  private var fitIntercept: Boolean = false
  private var family: String = "auto"
  private var stepSize: Double = 0.01
  private var solver: String = "sgd"
  private var miniBatchFraction: Double = 1.0
  private var histSize: Int = 10
  
  def setRegParam(rprm: Double) = { 
    this.regParam = rprm
    this
  } 
  def setRegType(regT: String) ={
    val allowed = Array("l1", "l2", "zero")
    val check = allowed.contains(regT)
    require(check == true, 
      s"regType should be l1/l2/zero but got ${regT}")
    this.regType = regT
    this
  } 
  def setElasticNetParam(elprm: Double) = {
    require(elprm >= 0.0 && elprm <= 1.0,
       s"elasticNetParam must be between 0.0 and 1.0 but got ${elprm}")
    this.elasticNetParam = elprm
    this
  } 
  def setThreshold(thr: Double) = { 
    require(thr >= 0.0 && thr <= 1.0,
        s"threshold must be between 0.0 and 1.0 but got ${thr}")
    this.threshold = thr
    this
  } 
  def setMaxIter(iter: Int) = {
    require(iter > 0,
      s"maximum iteration must be positive but got ${iter}")
    this.maxIter = iter
    this
  } 
  def setHistSize(hs: Int) = {
    require(hs > 0,
      s"history size must be positive but got ${hs}")
    this.histSize = hs
    this
  } 
  def setTol(tol: Double) = { 
    require(tol > 0, 
       s"tolerance must have positive value but got ${tol}")
    this.tol = tol
    this
  } 
  def setFitIntercept(icpt: Boolean) = {
    this.fitIntercept = icpt
    this
  } 
  def setFamily(family: String) = {
    val allowed = Array("auto", "binomial", "multinomial")
    val check = allowed.contains(family)
    require(check == true, 
      s"family should be auto/binomial/multinomial but got ${family}")
    this.family = family
    this
  } 
  def setSolver(sol: String) = {
    val allowed = Array("sgd", "lbfgs")
    val check = allowed.contains(sol)
    require(check == true, 
      s"solver should be sgd/lbfgs but got ${sol}")
    this.solver = sol
    this
  } 
  def setMiniBatchFraction(fraction: Double) = { 
    require(fraction > 0 && fraction <= 1.0,
      s"Fraction for mini-batch SGD must be in range (0, 1] but got ${fraction}")
    this.miniBatchFraction = fraction
    this
  }
  def setStepSize(step: Double) = { 
    require(step > 0,
      s"Initial step size must be positive but got ${step}")
    this.stepSize = step
    this
  }

  def fit(data: RDD[LabeledPoint]): LogisticRegressionModel = {
    val fdata = new FrovedisLabeledPoint(data) // Spark Data => Frovedis Data
    return fit(fdata, true)
  }
  def fit(data: FrovedisLabeledPoint): LogisticRegressionModel = {
    return fit(data, false)
  }
  def fit(data: FrovedisLabeledPoint,
          inputMovable: Boolean): LogisticRegressionModel = {
    val ncls = data.get_distinct_label_count().intValue
    if (this.family == "auto") {
      if (ncls > 2) this.family = "multinomial"
      else          this.family = "binomial"
    }
    var isMult = false
    var mkind = M_KIND.LRM
    if (this.family == "multinomial") { 
      isMult = true
      mkind = M_KIND.MLR
    }
    var regT: Int = 0
    if(regType == "l1") regT = 1
    else if(regType == "l2") regT = 2
    val mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    if (solver == "sgd")
      JNISupport.callFrovedisLRSGD(fs.master_node,data.get(),maxIter,stepSize,
                                   miniBatchFraction,regT,regParam,isMult,
                                   fitIntercept,tol,
                                   mid,inputMovable,data.is_dense())
    else if(solver == "lbfgs")
      JNISupport.callFrovedisLRLBFGS(fs.master_node,data.get(),maxIter,stepSize,
                                     histSize,regT,regParam,isMult,
                                     fitIntercept,tol,
                                     mid,inputMovable,data.is_dense())
    else throw new IllegalArgumentException("solver should be sgd/lbfgs")
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    val numFeatures = data.numCols()
    return new LogisticRegressionModel(mid,mkind,numFeatures,ncls,threshold)
  }
}

object LogisticRegressionWithSGD {
  // Kind of shortcut Spark-like interface for Spark user.
  // Spark data will get converted to Frovedis data and will be freed
  // from Frovedis side memory implicitly, once the training is completed.
  def train(data: RDD[LabeledPoint],
            numIter: Int,
            stepSize: Double,
            miniBatchFraction: Double,
            regParam: Double) : LogisticRegressionModel = {
     return new LogisticRegression().setMaxIter(numIter)
                                    .setStepSize(stepSize)
                                    .setMiniBatchFraction(miniBatchFraction)
                                    .setRegParam(regParam)
                                    .setSolver("sgd")
                                    .fit(data)
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
     return new LogisticRegression().setMaxIter(numIter)
                                    .setStepSize(stepSize)
                                    .setMiniBatchFraction(miniBatchFraction)
                                    .setRegParam(regParam)
                                    .setSolver("sgd")
                                    .fit(data,isMovableInput)
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
     return new LogisticRegression().setMaxIter(numIter)
                                    .setStepSize(stepSize)
                                    .setHistSize(histSize)
                                    .setRegParam(regParam)
                                    .setSolver("lbfgs")
                                    .fit(data)
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
     return new LogisticRegression().setMaxIter(numIter)
                                    .setStepSize(stepSize)
                                    .setHistSize(histSize)
                                    .setRegParam(regParam)
                                    .setSolver("lbfgs")
                                    .fit(data,isMovableInput)
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
