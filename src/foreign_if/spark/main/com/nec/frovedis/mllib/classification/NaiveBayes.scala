package com.nec.frovedis.mllib.classification

import com.nec.frovedis.matrix.Utils._
import com.nec.frovedis.matrix.ScalaCRS
import com.nec.frovedis.Jexrpc.{Node,FrovedisServer,JNISupport}
import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModelWithPredict}
import org.apache.spark.SparkContext
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.regression.LabeledPoint
import org.apache.spark.mllib.linalg.Vector

class NaiveBayes private(var lambda: Double,
                         var modelType: String) {
						 
  def this() = this(1.0, "multinomial")
  def this(lambda: Double) = {
    this()
    setLambda(lambda)
  }
  
  def setLambda(lambda: Double): NaiveBayes = {
    require(lambda >= 0,
      s"Smoothing parameter must be nonnegative but got $lambda")
    this.lambda = lambda
    this
  }
  def getLambda(): Double = this.lambda
    
  def setModelType(modelType: String): NaiveBayes = {
    val allowed_types = Array("multinomial", "bernoulli")
    if (!(allowed_types contains modelType)) 
      throw new IllegalArgumentException("Unknown model type: " + modelType) 
    this.modelType = modelType
    this
  }
   
  def getModelType(): String = this.modelType

  def run(data: RDD[LabeledPoint]): NaiveBayesModel = {
    val fdata = new FrovedisLabeledPoint(data)
    return run(fdata,true)
  }

  def run(fdata: FrovedisLabeledPoint): NaiveBayesModel =  {
     return run(fdata,false)
  }  

  def run(fdata: FrovedisLabeledPoint,
          movable: Boolean): NaiveBayesModel =  {
    val model_id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    JNISupport.callFrovedisNBM(fs.master_node,fdata.get(),lambda,model_id,modelType,movable,
                               fdata.is_dense())
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new NaiveBayesModel (model_id,modelType)
  }  
}

object NaiveBayes {
  def train(data: RDD[LabeledPoint], 
            lambda: Double, 
            modelType: String): NaiveBayesModel = {
    return new NaiveBayes(lambda).setModelType(modelType).run(data)
  } 
  def train(data:RDD[LabeledPoint], 
            lambda: Double): NaiveBayesModel =  {
    return train(data, lambda, "multinomial")
  }
  def train(data: RDD[LabeledPoint]): NaiveBayesModel =  {
    return train(data, 1.0, "multinomial")
  }
  def train(fdata: FrovedisLabeledPoint,
            lambda: Double,
            modelType: String): NaiveBayesModel =  {
    return new NaiveBayes(lambda).setModelType(modelType).run(fdata)
  }
  def train(fdata: FrovedisLabeledPoint,
            lambda: Double): NaiveBayesModel = {
    return train(fdata, lambda, "multinomial")
  }
  def train(fdata: FrovedisLabeledPoint): NaiveBayesModel = {
    return train(fdata, 1.0, "multinomial")
  } 
}

class NaiveBayesModel (val model_Id: Int,
                       val modelType: String) // Not used [?]
   extends GenericModelWithPredict(model_Id, M_KIND.NBM, 
                                   null) { // no encoding required for naive bayes
} 

object NaiveBayesModel{
  def load(sc: SparkContext, path: String) : NaiveBayesModel = load(path)
  def load(path: String) : NaiveBayesModel = {
    val model_id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.loadFrovedisNBM(fs.master_node,model_id,M_KIND.NBM,path)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new NaiveBayesModel(model_id, ret)
  }
}

