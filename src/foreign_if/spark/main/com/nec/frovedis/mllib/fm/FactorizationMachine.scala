package com.nec.frovedis.mllib.fm;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModelWithPredict}
import org.apache.spark.SparkContext
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.regression.LabeledPoint
import org.apache.spark.sql.{Dataset, Row}
import org.apache.spark.sql.functions.col
import org.apache.spark.ml.linalg.Vector
import org.apache.spark.mllib.linalg.Vectors

class FactorizationMachineModel (val model_Id: Int)
  extends GenericModelWithPredict(model_Id, M_KIND.FMM) {
}

object FactorizationMachineModel{
  def load(sc: SparkContext, path: String):  FactorizationMachineModel = load(path)
  def load(path: String):  FactorizationMachineModel = {
    val modelId = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    JNISupport.loadFrovedisModel(fs.master_node,modelId,M_KIND.FMM,path)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FactorizationMachineModel(modelId)
  }
}

class FactorizationMachine(var initStd: Double,
                           var maxIter: Int,
                           var stepSize: Double,
                           var optimizer: String,
                           var isRegression: Boolean,
                           var fitIntercept: Boolean,
                           var fitLinear: Boolean,
                           var factorSize: Int,
                           var regParam: (Double,Double,Double),
                           var miniBatchFraction: Double,
                           var seed: Int) {
    
  def this() = this(0.1, 100, 0.01, "SGD", true, true, true, 8, (0, 1e-3, 1e-4), 1.0, 1)

  def clear():  Unit = {
    setInitStd(0.1)
    setMaxIter(100)
    setStepSize(0.01)
    setOptimizer("SGD")
    setIsRegression(true)
    setFitIntercept(true)
    setFitLinear(true)
    setFactorSize(8)
    setRegParam((0.0, 1e-3, 1e-4))
    setMiniBatchFraction(1.0)
    setSeed(1)
  }
    
  private var labelCol:String = "label"

  private var featuresCol:String = "features"

  private var predictionCol:String = "predictions"

  private var weightCol:String = "weight"

  private var probabilityCol:String = "probability"

  def setLabelCol(value: String):  this.type = {
    this.labelCol = value
    this
  }

  def getLabelCol:String = labelCol
    
  def setFeaturesCol(value: String):  this.type = {
    this.featuresCol = value
    this
  }

  def getFeaturesCol:String = featuresCol

  def setPredictionCol(value: String):  this.type = {
    this.predictionCol = value
    this
  }

  def getPredictionCol:String = predictionCol  
    
  def setWeightCol(value: String):  this.type = {
    this.weightCol = value
    this
  }

  def getWeightCol:String = weightCol
    
  def setProbabilityCol(value: String):  this.type = {
    this.probabilityCol = value
    this
  }

  def getProbabilityCol:String = probabilityCol

  def setInitStd(initStd: Double):  this.type = {
    if(initStd < 0)
      throw new IllegalStateException("Invalid initDev: " + initStd)
    this.initStd = initStd
    this
  }

  def getInitStd():  Double = this.initStd

  def setStepSize(stepSize: Double):  this.type = {
    if(stepSize < 0.0001 || stepSize > 1.0) {
      throw new IllegalStateException("Invalid step size (learning-rate):  " + stepSize)
    }
    this.stepSize = stepSize
    this
  }

  def getStepSize():  Double = this.stepSize

  def setMaxIter(maxIter: Int): this.type = {
    if(maxIter < 0)
      throw new IllegalStateException("Invalid iterations: " + maxIter)
    this.maxIter = maxIter
    this
  }
    
  def getMaxIter():  Int = this.maxIter

  def setOptimizer(optimizer: String):  this.type = {
    val supported_opt = Array("SGD", "SGDA", "ALS", "MCMC")
    if (!(supported_opt contains optimizer))
      throw new IllegalStateException("Invalid optimizer: " + optimizer)
    this.optimizer = optimizer
    this
  }

  def getOptimizer():  String = this.optimizer

  def setIsRegression(reg: Boolean): this.type = {
    this.isRegression = reg
    this
  }

  def getIsRegression():  Boolean = this.isRegression

  /*
  In previous version, this was batchSize with default value 100.
  
  Provided fraction value is used to calculate the batch size from
  the data. batchSize = fraction * no. of samples
  */
  def setMiniBatchFraction(miniBatchFraction: Double): this.type = {
    if(miniBatchFraction < 0.0 || miniBatchFraction > 1.0)
      throw new IllegalStateException(s"miniBatchFraction should be between 0.0 and 1.0,"+ 
                                      s" provided: " + miniBatchFraction)
    this.miniBatchFraction = miniBatchFraction
    this
  }

  def getMiniBatchFraction():  Double = this.miniBatchFraction

  def setSeed(seed: Int): this.type = {
    if(seed < 0)
      throw new IllegalStateException("Seed should be a positive value: " + seed)
    this.seed = seed
    this
  }

  def getSeed(): Int = this.seed
    
  def setFitIntercept(fitIntercept: Boolean):  this.type = {
    this.fitIntercept = fitIntercept
    this
  }

  def getFitLinear():  Boolean = this.fitLinear

  def getFactorSize():  Int = this.factorSize

  def getfitIntercept():  Boolean  = this.fitIntercept

  def setFitLinear(fitLinear: Boolean):  this.type = {
    this.fitLinear = fitLinear
    this
  }

  def setFactorSize(factorSize: Int):  this.type = {
    if(factorSize < 0)
      throw new IllegalStateException("Invalid factor size: " +  factorSize)
    this.factorSize = factorSize
    this
  }

  def setRegParam(regParams: (Double, Double, Double)): this.type = {
    require(regParams._1 >= 0 && regParams._2 >= 0 && regParams._3 >= 0)
    this.regParam.copy(_1 = regParams._1)
    this.regParam.copy(_2 = regParams._2)
    this.regParam.copy(_3 = regParams._3)
    this
  }

  def setRegParam(regIntercept: Double = 0,
                  reg1Way: Double = 0,
                  reg2Way: Double = 0): this.type = {
    setRegParam((regIntercept, reg1Way, reg2Way))
  }    

  def extractLabeledPoints(data: Dataset[_]):  RDD[LabeledPoint] = {
    data.select(col(labelCol), col(featuresCol)).rdd.map {
      case Row(label: Double, features: Vector) =>
        LabeledPoint(label, Vectors.fromML(features))
    }
  }
    
  def fit(data: Dataset[_]):  FactorizationMachineModel = {
    return run(data)
  }

  def run(input: Dataset[_]):   FactorizationMachineModel = {
    //Convert to RDD[LabeledPoint]
    val rdd_data = extractLabeledPoints(input)
    return run(rdd_data)
  }
    
  def run(input: RDD[LabeledPoint]):  FactorizationMachineModel = {
    val fdata = new FrovedisLabeledPoint(input)
    return run(fdata,true)
  }
    
  def run(fdata: FrovedisLabeledPoint):  FactorizationMachineModel = {
    run(fdata,false)
  }
    
  def run(fdata: FrovedisLabeledPoint,
          movable: Boolean):  FactorizationMachineModel =  {
    if (fdata.is_dense()) throw new IllegalArgumentException(
        s"fit: Currently frovedis factorization machine " +
        s"supports only sparse data!")
    val batchSize = (miniBatchFraction * fdata.numRows()).asInstanceOf[Int]
    val model_Id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    val res = JNISupport.callFrovedisFM(fs.master_node, fdata.get(),
                                        initStd,      // Double
                                        stepSize,     // Double
                                        maxIter,      // Int
                                        optimizer,    // String
                                        isRegression, // Boolean
                                        batchSize,    // Int
                                        fitIntercept, // Boolean
                                        fitLinear,    // Boolean
                                        factorSize,   // Int
                                        regParam._1,  // Double
                                        regParam._2,  // Double
                                        regParam._3,  // Double
                                        seed,         //Int
                                        model_Id,     // Int
                                        movable)      // Boolean
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FactorizationMachineModel(model_Id)
  }
}

object FactorizationMachine{
  // train with Frovedis data, along with diffrent hyper-parameters  
  def train(fdata: FrovedisLabeledPoint,
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String,
            isRegression: Boolean,
            fitIntercept : Boolean,
            fitLinear : Boolean,
            factorSize : Int,
            regParam: (Double, Double,Double),
            miniBatchFraction: Double,
            seed: Int):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(isRegression)
                                     .setFitIntercept(fitIntercept)
                                     .setFitLinear(fitLinear)
                                     .setFactorSize(factorSize)
                                     .setRegParam(regParam)
                                     .setMiniBatchFraction(miniBatchFraction)
                                     .setSeed(seed)
                                     .run(fdata)

  }
  
  def train(fdata: FrovedisLabeledPoint,
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String,
            isRegression: Boolean,
            fitIntercept : Boolean,
            fitLinear : Boolean,
            factorSize : Int,
            regParam: (Double, Double,Double),
            miniBatchFraction: Double):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(isRegression)
                                     .setFitIntercept(fitIntercept)
                                     .setFitLinear(fitLinear)
                                     .setFactorSize(factorSize)
                                     .setRegParam(regParam)
                                     .setMiniBatchFraction(miniBatchFraction)
                                     .setSeed(1)
                                     .run(fdata)
  } 
   
  def train(fdata: FrovedisLabeledPoint,
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String,
            isRegression: Boolean,
            fitIntercept : Boolean,
            fitLinear : Boolean,
            factorSize : Int,
            regParam: (Double, Double,Double)):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(isRegression)
                                     .setFitIntercept(fitIntercept)
                                     .setFitLinear(fitLinear)
                                     .setFactorSize(factorSize)
                                     .setRegParam(regParam)
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(fdata)
  }
  
  def train(fdata: FrovedisLabeledPoint,
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String,
            isRegression: Boolean,
            fitIntercept : Boolean,
            fitLinear : Boolean,
            factorSize : Int):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(isRegression)
                                     .setFitIntercept(fitIntercept)
                                     .setFitLinear(fitLinear)
                                     .setFactorSize(factorSize)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(fdata)
  } 
 
  def train(fdata: FrovedisLabeledPoint,
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String,
            isRegression: Boolean,
            fitIntercept : Boolean,
            fitLinear : Boolean):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(isRegression)
                                     .setFitIntercept(fitIntercept)
                                     .setFitLinear(fitLinear)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(fdata)
  }
  def train(fdata: FrovedisLabeledPoint,
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String,
            isRegression: Boolean,
            fitIntercept : Boolean):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(isRegression)
                                     .setFitIntercept(fitIntercept)
                                     .setFitLinear(true)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(fdata)
  }
   
  def train(fdata: FrovedisLabeledPoint,
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String,
            isRegression: Boolean):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(isRegression)
                                     .setFitIntercept(true)
                                     .setFitLinear(true)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(fdata)
  }

  def train(fdata: FrovedisLabeledPoint,
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(true)
                                     .setFitIntercept(true)
                                     .setFitLinear(true)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(fdata)
  }

  def train(fdata: FrovedisLabeledPoint,
            initStdev: Double,
            iter: Int,
            stepSize: Double ):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer("SGD")
                                     .setIsRegression(true)
                                     .setFitIntercept(true)
                                     .setFitLinear(true)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(fdata)
  }

  def train(fdata: FrovedisLabeledPoint,
            initStdev: Double,
            iter: Int ):  FactorizationMachineModel = {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize( 0.01)
                                     .setOptimizer("SGD")
                                     .setIsRegression(true)
                                     .setFitIntercept(true)
                                     .setFitLinear(true)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(fdata)
  }

  def train(fdata: FrovedisLabeledPoint,
            initStdev: Double ):  FactorizationMachineModel = {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(100)
                                     .setStepSize( 0.01)
                                     .setOptimizer("SGD")
                                     .setIsRegression(true)
                                     .setFitIntercept(true)
                                     .setFitLinear(true)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(fdata)
  }

  def train(fdata: FrovedisLabeledPoint):  FactorizationMachineModel = {
    return new FactorizationMachine().setInitStd(0.1)
                                     .setMaxIter(100)
                                     .setStepSize( 0.01)
                                     .setOptimizer("SGD")
                                     .setIsRegression(true)
                                     .setFitIntercept(true)
                                     .setFitLinear(true)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(fdata)
  }
   
  // train with spark data, along with diffrent hyper-parameters 
  def train(data: RDD[LabeledPoint],
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String,
            isRegression: Boolean,
            fitIntercept : Boolean,
            fitLinear : Boolean,
            factorSize : Int,
            regParam: (Double, Double,Double),
            miniBatchFraction: Double,
            seed: Int):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(isRegression)
                                     .setFitIntercept(fitIntercept)
                                     .setFitLinear(fitLinear)
                                     .setFactorSize(factorSize)
                                     .setRegParam(regParam)
                                     .setMiniBatchFraction(miniBatchFraction)
                                     .setSeed(seed)
                                     .run(data)
  }
  
  def train(data: RDD[LabeledPoint],
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String,
            isRegression: Boolean,
            fitIntercept : Boolean,
            fitLinear : Boolean,
            factorSize : Int,
            regParam: (Double, Double,Double),
            miniBatchFraction: Double):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(isRegression)
                                     .setFitIntercept(fitIntercept)
                                     .setFitLinear(fitLinear)
                                     .setFactorSize(factorSize)
                                     .setRegParam(regParam)
                                     .setMiniBatchFraction(miniBatchFraction)
                                     .setSeed(1)
                                     .run(data)
  }

  def train(data: RDD[LabeledPoint],
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String,
            isRegression: Boolean,
            fitIntercept : Boolean,
            fitLinear : Boolean,
            factorSize : Int,
            regParam: (Double, Double,Double)):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(isRegression)
                                     .setFitIntercept(fitIntercept)
                                     .setFitLinear(fitLinear)
                                     .setFactorSize(factorSize)
                                     .setRegParam(regParam)
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(data)
  }

  def train(data: RDD[LabeledPoint],
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String,
            isRegression: Boolean,
            fitIntercept : Boolean,
            fitLinear : Boolean,
            factorSize : Int):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(isRegression)
                                     .setFitIntercept(fitIntercept)
                                     .setFitLinear(fitLinear)
                                     .setFactorSize(factorSize)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(data)
  }

  def train(data: RDD[LabeledPoint],
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String,
            isRegression: Boolean,
            fitIntercept : Boolean,
            fitLinear : Boolean):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(isRegression)
                                     .setFitIntercept(fitIntercept)
                                     .setFitLinear(fitLinear)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(data)
  }

  def train(data: RDD[LabeledPoint],
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String,
            isRegression: Boolean,
            fitIntercept : Boolean):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(isRegression)
                                     .setFitIntercept(fitIntercept)
                                     .setFitLinear(true)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(data)
  }

  def train(data: RDD[LabeledPoint],
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String,
            isRegression: Boolean):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(isRegression)
                                     .setFitIntercept(true)
                                     .setFitLinear(true)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(data)
  }

  def train(data: RDD[LabeledPoint],
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(true)
                                     .setFitIntercept(true)
                                     .setFitLinear(true)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(data)
  }
  
  def train(data: RDD[LabeledPoint],
            initStdev: Double,
            iter: Int,
            stepSize: Double ):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer("SGD")
                                     .setIsRegression(true)
                                     .setFitIntercept(true)
                                     .setFitLinear(true)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(data)
  }
  
  def train(data: RDD[LabeledPoint],
            initStdev: Double,
            iter: Int ):  FactorizationMachineModel = {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize( 0.01)
                                     .setOptimizer("SGD")
                                     .setIsRegression(true)
                                     .setFitIntercept(true)
                                     .setFitLinear(true)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(data)
  }

  def train(data: RDD[LabeledPoint],
            initStdev: Double ):  FactorizationMachineModel = {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(100)
                                     .setStepSize( 0.01)
                                     .setOptimizer("SGD")
                                     .setIsRegression(true)
                                     .setFitIntercept(true)
                                     .setFitLinear(true)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(data)
  }
  
  def train(data: RDD[LabeledPoint]):  FactorizationMachineModel =  {

    return new FactorizationMachine().setInitStd(0.1)
                                     .setMaxIter(100)
                                     .setStepSize( 0.01)
                                     .setOptimizer("SGD")
                                     .setIsRegression(true)
                                     .setFitIntercept(true)
                                     .setFitLinear(true)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(data)
  }
  
  // train with spark Dataset, along with diffrent hyper-parameters
  def train(data: Dataset[_],
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String,
            isRegression: Boolean,
            fitIntercept : Boolean,
            fitLinear : Boolean,
            factorSize : Int,
            regParam: (Double, Double,Double),
            miniBatchFraction: Double,
            seed: Int):  FactorizationMachineModel = {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(isRegression)
                                     .setFitIntercept(fitIntercept)
                                     .setFitLinear(fitLinear)
                                     .setFactorSize(factorSize)
                                     .setRegParam(regParam)
                                     .setMiniBatchFraction(miniBatchFraction)
                                     .setSeed(seed)
                                     .run(data)
    
  }

  def train(data: Dataset[_],
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String,
            isRegression: Boolean,
            fitIntercept : Boolean,
            fitLinear : Boolean,
            factorSize : Int,
            regParam: (Double, Double,Double),
            miniBatchFraction: Double):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(isRegression)
                                     .setFitIntercept(fitIntercept)
                                     .setFitLinear(fitLinear)
                                     .setFactorSize(factorSize)
                                     .setRegParam(regParam)
                                     .setMiniBatchFraction(miniBatchFraction)
                                     .setSeed(1)
                                     .run(data)
  }

  def train(data: Dataset[_],
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String,
            isRegression: Boolean,
            fitIntercept : Boolean,
            fitLinear : Boolean,
            factorSize : Int,
            regParam: (Double, Double,Double)):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(isRegression)
                                     .setFitIntercept(fitIntercept)
                                     .setFitLinear(fitLinear)
                                     .setFactorSize(factorSize)
                                     .setRegParam(regParam)
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(data)
  }

  def train(data: Dataset[_],
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String,
            isRegression: Boolean,
            fitIntercept : Boolean,
            fitLinear : Boolean,
            factorSize : Int):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(isRegression)
                                     .setFitIntercept(fitIntercept)
                                     .setFitLinear(fitLinear)
                                     .setFactorSize(factorSize)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(data)
  }

   def train(data: Dataset[_],
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String,
            isRegression: Boolean,
            fitIntercept : Boolean,
            fitLinear : Boolean):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(isRegression)
                                     .setFitIntercept(fitIntercept)
                                     .setFitLinear(fitLinear)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(data)
  }

   def train(data: Dataset[_],
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String,
            isRegression: Boolean,
            fitIntercept : Boolean):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(isRegression)
                                     .setFitIntercept(fitIntercept)
                                     .setFitLinear(true)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(data)
  }
  
  def train(data: Dataset[_],
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String,
            isRegression: Boolean):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(isRegression)
                                     .setFitIntercept(true)
                                     .setFitLinear(true)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(data)
  }

  def train(data: Dataset[_],
            initStdev: Double,
            iter: Int,
            stepSize: Double,
            optimizer: String):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer(optimizer)
                                     .setIsRegression(true)
                                     .setFitIntercept(true)
                                     .setFitLinear(true)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(data)
  }
  
  def train(data: Dataset[_],
            initStdev: Double,
            iter: Int,
            stepSize: Double ):  FactorizationMachineModel =  {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize(stepSize)
                                     .setOptimizer("SGD")
                                     .setIsRegression(true)
                                     .setFitIntercept(true)
                                     .setFitLinear(true)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(data)
  }

  def train(data: Dataset[_],
            initStdev: Double,
            iter: Int ):  FactorizationMachineModel = {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(iter)
                                     .setStepSize( 0.01)
                                     .setOptimizer("SGD")
                                     .setIsRegression(true)
                                     .setFitIntercept(true)
                                     .setFitLinear(true)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(data)
  }
  
  def train(data: Dataset[_],
            initStdev: Double ):  FactorizationMachineModel = {
    return new FactorizationMachine().setInitStd(initStdev)
                                     .setMaxIter(100)
                                     .setStepSize( 0.01)
                                     .setOptimizer("SGD")
                                     .setIsRegression(true)
                                     .setFitIntercept(true)
                                     .setFitLinear(true)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(data)
  }

  def train(data: Dataset[_]):  FactorizationMachineModel = {

    return new FactorizationMachine().setInitStd(0.1)
                                     .setMaxIter(100)
                                     .setStepSize(0.01)
                                     .setOptimizer("SGD")
                                     .setIsRegression(true)
                                     .setFitIntercept(true)
                                     .setFitLinear(true)
                                     .setFactorSize(8)
                                     .setRegParam((0.0, 1e-3, 1e-4))
                                     .setMiniBatchFraction(1.0)
                                     .setSeed(1)
                                     .run(data)
  }  
}
