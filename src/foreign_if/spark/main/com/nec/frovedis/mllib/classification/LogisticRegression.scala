package com.nec.frovedis.mllib.classification;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport,MemPair}
import com.nec.frovedis.Jmllib.DummyGLM
import com.nec.frovedis.io.FrovedisIO
import com.nec.frovedis.matrix.{ENUM, MAT_KIND}
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import com.nec.frovedis.mllib.{M_KIND,ModelID}
import com.nec.frovedis.mllib.regression.GeneralizedLinearModel
import org.apache.spark.SparkContext
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.regression.LabeledPoint
import org.apache.spark.mllib.linalg.Vector
import scala.collection.immutable.Map

class LogisticRegressionModel(modelId: Int,
                              modelKind: Short,
                              nftr: Long,
                              ncls: Int,
                              thr: Double,
                              logic: Map[Double,Double]) 
  extends GeneralizedLinearModel(modelId,modelKind,nftr,ncls,thr) {
  protected val enc_logic: Map[Double,Double] = logic
  def this(m: DummyGLM,
           logic: Map[Double,Double]) = {
    this(m.mid, m.mkind, m.numFeatures, m.numClasses, m.threshold, logic)
  }
  override def predict(data: Vector) : Double = {
     val ret = super.predict(data)
     return if (threshold == ENUM.NONE) ret else enc_logic(ret)
  }
  override def predict(data: FrovedisRowmajorMatrix) : RDD[Double] = {
    val ret = super.predict(data)
    return if (threshold == ENUM.NONE) ret else ret.map(x => enc_logic(x))
  }
  override def predict(data: FrovedisSparseData) : RDD[Double] = {
    val ret = super.predict(data)
    return if (threshold == ENUM.NONE) ret else ret.map(x => enc_logic(x))
  }
  override def save(path: String) : Unit = {
    val context = SparkContext.getOrCreate()
    save(context, path)
  }
  override def save(sc: SparkContext, path: String) : Unit = {
    val success = FrovedisIO.createDir(path) 
    require(success, "Another model named " + path + " already exists!")
    super.save(path + "/model")
    sc.parallelize(logic.toSeq, 2).saveAsObjectFile(path + "/label_map_spk")
  }
}

object LogisticRegressionModel {  // companion object (for static members)
  def load(sc: SparkContext,
           path: String,
           isMultinomial: Boolean = false) : LogisticRegressionModel = {
    val exist = FrovedisIO.checkExists(path)
    require(exist, "No model named " + path + " is found!")
    val model_id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    // load a LogisticRegressionModel from the 'path'
    // and register it with 'model_id' at Frovedis server
    var kind = M_KIND.LRM
    if(isMultinomial) kind = M_KIND.MLR
    val ret = JNISupport.loadFrovedisGLM(fs.master_node,
                                         model_id, kind, path + "/model")
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val context = SparkContext.getOrCreate()
    val logic = context.objectFile[(Double, Double)](path + "/label_map_spk")
                       .collectAsMap // returns generic scala.collection.Map
                       .toMap        // to make it immutable Map
    return new LogisticRegressionModel(ret, logic)
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

class LogisticRegression(sol: String = "sgd") {
  protected var solver: String = sol
  protected var regParam: Double = 0.01
  protected var regType: String = "zero"
  protected var elasticNetParam: Double = 0.01
  protected var threshold: Double = 0.5
  protected var maxIter: Int = 1000
  protected var tol: Double = 1E-6
  protected var fitIntercept: Boolean = false
  protected var family: String = "auto"
  protected var stepSize: Double = 0.01
  protected var miniBatchFraction: Double = 1.0
  protected var histSize: Int = 10
  protected var use_shrink: Boolean = false
  
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
  def setUseShrink(shrink: Boolean) = {
    this.use_shrink = shrink
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
    require(this.solver == "sgd", "mini batch fraction can be set for SGD solver only!")
    require(fraction > 0 && fraction <= 1.0,
      s"Fraction for mini-batch SGD must be in range (0, 1] but got ${fraction}")
    this.miniBatchFraction = fraction
    this
  }
  def setHistSize(hs: Int) = {
    require(this.solver == "lbfgs", "history size can be set for LBFGS solver only!")
    require(hs > 0,
      s"history size must be positive but got ${hs}")
    this.histSize = hs
    this
  } 
  def setStepSize(step: Double) = { 
    require(step > 0,
      s"Initial step size must be positive but got ${step}")
    this.stepSize = step
    this
  }
  override def toString() : String = {
    var str = " solver: " + this.solver + 
              "\n family: " + this.family +
              "\n step size: " + this.stepSize +
              "\n max iter: " + this.maxIter +
              "\n regularization type: " + this.regType +
              "\n regularization parameter: " + this.regParam +
              "\n threshold: " + this.threshold +
              "\n convergence tolerance: " + this.tol +
              "\n fit intercept: " + this.fitIntercept +
              "\n use shrink: " + this.use_shrink +
              "\n elastic net parameter: " + this.elasticNetParam
    if (solver == "sgd") str += "\n mini-batch fraction: " + this.miniBatchFraction
    else if (solver == "lbfgs") str += "\n history size: " + this.histSize
    else throw new IllegalArgumentException("Currently supported solvers are: sgd/lbfgs\n")
    return str + "\n"
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
    if (data.is_dense() && use_shrink)
      throw new IllegalArgumentException(
      s"fit: use_shrink is supported only for sparse data!\n")
    if (data.is_dense() && data.matType() != MAT_KIND.CMJR) 
       throw new IllegalArgumentException(
        s"fit: please provide column major "+
        s"points as for dense data to frovedis logictic regression!\n")
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

    var enc_ret: (MemPair,  Map[Double, Double]) = null
    if (isMult) enc_ret = data.encode_labels()
    else        enc_ret = data.encode_labels(Array(-1.0, 1.0))
    val encoded_data = enc_ret._1
    val logic = enc_ret._2

    var regT: Int = 0
    if(regType == "l1") regT = 1
    else if(regType == "l2") regT = 2
    val mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    if (solver == "sgd")
      JNISupport.callFrovedisLRSGD(fs.master_node,encoded_data,maxIter,stepSize,
                                   miniBatchFraction,regT,regParam,isMult,
                                   fitIntercept,tol,
                                   mid,inputMovable,data.is_dense(),use_shrink)
    else if(solver == "lbfgs")
      JNISupport.callFrovedisLRLBFGS(fs.master_node,encoded_data,maxIter,stepSize,
                                     histSize,regT,regParam,isMult,
                                     fitIntercept,tol,
                                     mid,inputMovable,data.is_dense())
    else throw new IllegalArgumentException("Currently supported solvers are: sgd/lbfgs\n")
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val numFeatures = data.numCols()
    data.release_encoded_labels() // deleting encoded labels from server
    return new LogisticRegressionModel(mid,mkind,numFeatures,ncls,threshold,logic)
  }
}

class LogisticRegressionWithSGD extends LogisticRegression("sgd") {
  override def setStepSize(step: Double) = {
    super.setStepSize(step)
    this
  }
  override def setRegParam(rprm: Double) = {
    super.setRegParam(rprm)
    this
  }
  override def setRegType(regT: String) = {
    super.setRegType(regT)
    this
  }
  override def setElasticNetParam(elprm: Double) = {
    super.setElasticNetParam(elprm)
    this
  }
  override def setThreshold(thr: Double) = {
    super.setThreshold(thr)
    this
  }
  override def setMaxIter(iter: Int) = {
    super.setMaxIter(iter)
    this
  }
  override def setTol(tol: Double) = {
    super.setTol(tol)
    this
  }
  override def setFitIntercept(icpt: Boolean) = {
    super.setFitIntercept(icpt)
    this
  }
  override def setUseShrink(shrink: Boolean) = {
    super.setUseShrink(shrink)
    this
  }
  override def setFamily(family: String) = {
    super.setFamily(family)
    this
  }
  override def setSolver(sol: String) = {
    throw new NotImplementedError("Solver: Not applicable to be set for LogisticRegressionWithSGD")
  }
  override def setMiniBatchFraction(fraction: Double) = {
    super.setMiniBatchFraction(fraction)
    this
  }
  override def setHistSize(hs: Int) = {
    throw new NotImplementedError("HistSize: Not applicable to be set for LogisticRegressionWithSGD")
  }
}

class LogisticRegressionWithLBFGS extends LogisticRegression("lbfgs") {
  override def setStepSize(step: Double) = {
    super.setStepSize(step)
    this
  }
  override def setRegParam(rprm: Double) = {
    super.setRegParam(rprm)
    this
  }
  override def setRegType(regT: String) = {
    super.setRegType(regT)
    this
  }
  override def setElasticNetParam(elprm: Double) = {
    super.setElasticNetParam(elprm)
    this
  }
  override def setThreshold(thr: Double) = {
    super.setThreshold(thr)
    this
  }
  override def setMaxIter(iter: Int) = {
    super.setMaxIter(iter)
    this
  }
  override def setTol(tol: Double) = {
    super.setTol(tol)
    this
  }
  override def setFitIntercept(icpt: Boolean) = {
    super.setFitIntercept(icpt)
    this
  }
  override def setUseShrink(shrink: Boolean) = {
    super.setUseShrink(shrink)
    this
  }
  override def setFamily(family: String) = {
    super.setFamily(family)
    this
  }
  override def setSolver(sol: String) = {
    throw new NotImplementedError("Solver: Not applicable to be set for LogisticRegressionWithLBFGS")
  }
  override def setMiniBatchFraction(fraction: Double) = {
    throw new NotImplementedError("MiniBatchFraction: Not applicable to be set for LogisticRegressionWithLBFGS")
  }
  override def setHistSize(hs: Int) = {
    super.setHistSize(hs)
    this
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
