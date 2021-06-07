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
  def load(sc: SparkContext, path: String): FactorizationMachineModel = load(path)
  def load(path: String): FactorizationMachineModel = {
    val modelId = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    JNISupport.loadFrovedisModel(fs.master_node,modelId,M_KIND.FMM,path)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FactorizationMachineModel(modelId)
  }
}

class FactorizationMachine private (val fm_config: FMConfig) {
  
  private var labelCol:String = "label"
    
  private var featuresCol:String = "features"
  
  def setLabelCol(value: String):this.type = {
    this.labelCol = value
    this  
  }
    
  def setFeaturesCol(value: String):this.type = {
    this.featuresCol = value
    this  
  }
    
  def getLabelCol:String = labelCol
    
  def getFeaturesCol:String = featuresCol  
    
  def extractLabeledPoints(data: Dataset[_], numClasses: Int): RDD[LabeledPoint] = {
    data.select(col(labelCol), col(featuresCol)).rdd.map {
      case Row(label: Double, features: Vector) =>
        require(label % 1 == 0 && label >= 0 && label < numClasses, s"Classifier was given" +
          s" dataset with invalid label $label.  Labels must be integers in range" +
          s" [0, $numClasses).")
        LabeledPoint(label, Vectors.fromML(features))
    }    
  }
    
  def run(input: Dataset[_]):  FactorizationMachineModel = {
    //Convert to RDD[LabeledPoint]   
    val rdd_data = extractLabeledPoints(input, 2)
    return run(rdd_data)
  }  
  def run(input: RDD[LabeledPoint]): FactorizationMachineModel = {
    val fdata = new FrovedisLabeledPoint(input)
    return run(fdata,true)
  }
  def run(fdata: FrovedisLabeledPoint): FactorizationMachineModel = {
    run(fdata,false)
  }
  def run(fdata: FrovedisLabeledPoint, 
          movable: Boolean): FactorizationMachineModel =  {
    if (fdata.is_dense()) throw new IllegalArgumentException(
        s"fit: Currently frovedis factorization machine " +
        s"supports only sparse data!")
    fm_config.assertValid()
    val model_Id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    val res = JNISupport.callFrovedisFM(fs.master_node, fdata.get(),
                         fm_config.getInitStdev(),    // Double
                         fm_config.getLearnRate(),    // Double
                         fm_config.getIterations(),   // Int
                         fm_config.getOptimizer(),    // String
                         fm_config.getIsRegression(), // Boolean
                         fm_config.getBatchSize(),    // Int
                         fm_config.dim._1,            // Boolean
                         fm_config.dim._2,	      // Boolean
                         fm_config.dim._3,            // Int
                         fm_config.regParam._1,	      // Double
                         fm_config.regParam._2,       // Double	
                         fm_config.regParam._3,       // Double
                         model_Id,                    // Int
			 movable)                     // Boolean
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
           learnRate: Double,
           optimizer: String,
           isRegression: Boolean, 
           dim: (Boolean, Boolean, Int),
           regParam: (Double, Double, Double),
           batchsize: Int): FactorizationMachineModel =  {
  
    val fm_config = new FMConfig(initStdev, iter,
                                 learnRate, optimizer, isRegression, 
                                 dim, regParam, batchsize)
    return new FactorizationMachine(fm_config).run(fdata)
  }

  def train(fdata: FrovedisLabeledPoint,
            initStdev: Double,
            iter: Int, 
            learnRate: Double, 
            optimizer: String,
            isRegression: Boolean, 
            dim: (Boolean, Boolean, Int),
            regParam: (Double, Double,Double)): FactorizationMachineModel = {
    return train(fdata,initStdev,iter,learnRate,optimizer,isRegression,dim,regParam,100)
  }
  
  def train(fdata: FrovedisLabeledPoint,
            initStdev: Double,
            iter: Int, 
            learnRate: Double, 
            optimizer: String,
            isRegression: Boolean,
            dim: (Boolean, Boolean, Int)): FactorizationMachineModel = {
    val regParam = (0.0, 1e-3, 1e-4) 
    return train(fdata,initStdev,iter,learnRate,optimizer,isRegression,dim,regParam,100)
  }

  def train(fdata: FrovedisLabeledPoint,
            initStdev: Double,
            iter: Int, 
            learnRate: Double, 
            optimizer: String,
            isRegression: Boolean): FactorizationMachineModel =  {
    val dim = (true,true,8) 
    val regParam = (0.0, 1e-3, 1e-4) 
    return train(fdata,initStdev,iter,learnRate,optimizer,isRegression,dim,regParam,100)
  }

  def train(fdata: FrovedisLabeledPoint, 
            initStdev: Double,
            iter: Int, 
            learnRate: Double, 
            optimizer: String): FactorizationMachineModel =  {
    val dim = (true,true,8) 
    val regParam = (0.0, 1e-3, 1e-4) 
    return train(fdata,initStdev,iter,learnRate,optimizer,true,dim,regParam,100)
  }

  def train(fdata: FrovedisLabeledPoint, 
            initStdev: Double,
            iter: Int, 
            learnRate: Double): FactorizationMachineModel =  { 
    val dim = (true,true,8) 
    val regParam = (0.0, 1e-3, 1e-4) 
    return train(fdata,initStdev,iter,learnRate,"SGD",true,dim,regParam,100)
  }

  def train(fdata: FrovedisLabeledPoint,
            initStdev: Double,
            iter: Int): FactorizationMachineModel =  { 
    val dim = (true,true,8) 
    val regParam = (0.0, 1e-3, 1e-4) 
    return train(fdata,initStdev,iter,0.01,"SGD",true,dim,regParam,100)
  }  
 
  def train(fdata: FrovedisLabeledPoint,
            initStdev: Double): FactorizationMachineModel =  {
    val dim = (true,true,8) 
    val regParam = (0.0, 1e-3, 1e-4) 
    return train(fdata,initStdev,100,0.01,"SGD",true,dim,regParam,100)
  }  
 
  def train(fdata: FrovedisLabeledPoint): FactorizationMachineModel =  {
    val dim = (true,true,8) 
    val regParam = (0.0, 1e-3, 1e-4)
    return train(fdata,0.1,100,0.01,"SGD",true,dim,regParam,100)
  }  

  // train with spark data, along with diffrent hyper-parameters
  def train(data: RDD[LabeledPoint], 
            initStdev: Double,
            iter: Int, 
            learnRate: Double, 
            optimizer: String,
            isRegression: Boolean, 
            dim: (Boolean, Boolean, Int),
            regParam: (Double, Double,Double),
            batchsize: Int): FactorizationMachineModel =  {
    val fm_config = new FMConfig(initStdev,iter,
                                 learnRate,optimizer,isRegression,
                                 dim,regParam,batchsize)
    return new FactorizationMachine(fm_config).run(data)
  }
  def train(data: RDD[LabeledPoint], 
            initStdev: Double,
            iter: Int, 
            learnRate: Double, 
            optimizer: String,
            isRegression: Boolean,
            dim: (Boolean, Boolean, Int),
            regParam: (Double, Double,Double)): FactorizationMachineModel =  {
    return train(data,initStdev,iter,learnRate,optimizer,isRegression,dim,regParam,100)
  }
  def train(data: RDD[LabeledPoint], 
            initStdev: Double,
            iter: Int, 
            learnRate: Double, 
            optimizer: String,
            isRegression: Boolean,
            dim: (Boolean,Boolean,Int)): FactorizationMachineModel =  {
    val regParam = (0.0, 1e-3, 1e-4)
    return train(data,initStdev,iter,learnRate,optimizer,isRegression,dim,regParam,100)
  }
  def train(data: RDD[LabeledPoint], 
            initStdev: Double,
            iter: Int, 
            learnRate: Double, 
            optimizer: String,
            isRegression: Boolean): FactorizationMachineModel =  {
    val dim = (true,true,8) 
    val regParam = (0.0, 1e-3, 1e-4)
    return train(data,initStdev,iter,learnRate,optimizer,isRegression,dim,regParam,100)
  }
  def train(data: RDD[LabeledPoint],
            initStdev: Double,
            iter: Int, 
            learnRate: Double, 
            optimizer: String): FactorizationMachineModel =  {
    val dim = (true,true,8) 
    val regParam = (0.0, 1e-3, 1e-4)
    return train(data,initStdev,iter,learnRate,optimizer,true,dim,regParam,100)
  }
  def train(data: RDD[LabeledPoint],
            initStdev: Double,
            iter: Int, 
            learnRate: Double): FactorizationMachineModel =  { 
    val dim = (true,true,8) 
    val regParam = (0.0, 1e-3, 1e-4)
    return train(data,initStdev,iter,learnRate,"SGD",true,dim,regParam,100)
  }
  def train(data: RDD[LabeledPoint],
            initStdev: Double,
            iter: Int): FactorizationMachineModel =  { 
    val dim = (true,true,8) 
    val regParam = (0.0, 1e-3, 1e-4)
    return train(data,initStdev,iter,0.01,"SGD",true,dim,regParam,100)
  }
  def train(data: RDD[LabeledPoint],
            initStdev: Double): FactorizationMachineModel = {
    val dim = (true,true,8) 
    val regParam = (0.0, 1e-3, 1e-4)
    return train(data,initStdev,100,0.01,"SGD",true,dim,regParam,100)
  }
  def train(data: RDD[LabeledPoint]): FactorizationMachineModel =  {
    val dim = (true,true,8) 
    val regParam = (0.0, 1e-3, 1e-4)
    return train(data,0.1,100,0.01,"SGD",true,dim,regParam,100)
  }
  
  // train with spark Dataset, along with diffrent hyper-parameters
  def train(data: Dataset[_], 
            initStdev: Double,
            iter: Int, 
            learnRate: Double, 
            optimizer: String,
            isRegression: Boolean, 
            dim: (Boolean, Boolean, Int),
            regParam: (Double, Double,Double),
            batchsize: Int): FactorizationMachineModel =  {
    val fm_config = new FMConfig(initStdev,iter,
                                 learnRate,optimizer,isRegression,
                                 dim,regParam,batchsize)
    return new FactorizationMachine(fm_config).run(data)
  }

  def train(data: Dataset[_], 
            initStdev: Double,
            iter: Int, 
            learnRate: Double, 
            optimizer: String,
            isRegression: Boolean,
            dim: (Boolean, Boolean, Int),
            regParam: (Double, Double,Double)): FactorizationMachineModel =  {
    return train(data,initStdev,iter,learnRate,optimizer,isRegression,dim,regParam,100)
  }
  def train(data: Dataset[_], 
            initStdev: Double,
            iter: Int, 
            learnRate: Double, 
            optimizer: String,
            isRegression: Boolean,
            dim: (Boolean,Boolean,Int)): FactorizationMachineModel =  {
    val regParam = (0.0, 1e-3, 1e-4)
    return train(data,initStdev,iter,learnRate,optimizer,isRegression,dim,regParam,100)
  }
  def train(data: Dataset[_], 
            initStdev: Double,
            iter: Int, 
            learnRate: Double, 
            optimizer: String,
            isRegression: Boolean): FactorizationMachineModel =  {
    val dim = (true,true,8) 
    val regParam = (0.0, 1e-3, 1e-4)
    return train(data,initStdev,iter,learnRate,optimizer,isRegression,dim,regParam,100)
  }
  def train(data: Dataset[_],
            initStdev: Double,
            iter: Int, 
            learnRate: Double, 
            optimizer: String): FactorizationMachineModel =  {
    val dim = (true,true,8) 
    val regParam = (0.0, 1e-3, 1e-4)
    return train(data,initStdev,iter,learnRate,optimizer,true,dim,regParam,100)
  }
  def train(data: Dataset[_],
            initStdev: Double,
            iter: Int, 
            learnRate: Double): FactorizationMachineModel =  { 
    val dim = (true,true,8) 
    val regParam = (0.0, 1e-3, 1e-4)
    return train(data,initStdev,iter,learnRate,"SGD",true,dim,regParam,100)
  }
  def train(data: Dataset[_],
            initStdev: Double,
            iter: Int): FactorizationMachineModel =  { 
    val dim = (true,true,8) 
    val regParam = (0.0, 1e-3, 1e-4)
    return train(data,initStdev,iter,0.01,"SGD",true,dim,regParam,100)
  }
  def train(data: Dataset[_],
            initStdev: Double): FactorizationMachineModel = {
    val dim = (true,true,8) 
    val regParam = (0.0, 1e-3, 1e-4)
    return train(data,initStdev,100,0.01,"SGD",true,dim,regParam,100)
  }
  def train(data: Dataset[_]): FactorizationMachineModel =  {
    val dim = (true,true,8) 
    val regParam = (0.0, 1e-3, 1e-4)
    return train(data,0.1,100,0.01,"SGD",true,dim,regParam,100)
  }    
}

