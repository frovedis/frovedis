package com.nec.frovedis.mllib.classification

import com.nec.frovedis.matrix.Utils._
import com.nec.frovedis.matrix.MAT_KIND
import com.nec.frovedis.Jexrpc.{Node,FrovedisServer,JNISupport}
import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModelWithPredict}
import org.apache.spark.SparkContext
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.regression.LabeledPoint
import org.apache.spark.mllib.linalg.Vector

class NaiveBayes private(var lambda: Double,
                         var modelType: String,
                         var threshold: Double,
                         var fit_prior: Boolean,
                         var class_prior: Array[Double]) {
						 
  def this() = this(1.0, "multinomial", 0.0, true, Array.empty[Double])
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

  def setBinarize(thr: Double): NaiveBayes = {
    this.threshold = thr
    this
  }

  def getBinarize(): Double = this.threshold

  def setFitPrior(fir_prior: Double): NaiveBayes = {
    this.fit_prior = fit_prior
    this
  }

  def setClassPrior(class_prior: Array[Double]): NaiveBayes = {
    this.class_prior = class_prior
    this
  }

  def run(data: RDD[LabeledPoint], sample_weight: Array[Double]): NaiveBayesModel = {
    val need_rowmajor = true
    val fdata = new FrovedisLabeledPoint(data, need_rowmajor)
    return run(fdata, true, sample_weight)
  }

  def run(fdata: FrovedisLabeledPoint, sample_weight: Array[Double]): NaiveBayesModel =  {
     return run(fdata, false, sample_weight)
  }  

  def run(fdata: FrovedisLabeledPoint,
          movable: Boolean, sample_weight: Array[Double]): NaiveBayesModel =  {
    if (fdata.is_dense() && fdata.matType() != MAT_KIND.RMJR) { 
       throw new IllegalArgumentException(
        s"run: please provide row major "+
        s"points as for dense data to frovedis naive bayes!\n")
    }
    val model_id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    val sample_weight_length = sample_weight.length
    val class_prior_length = class_prior.length
    JNISupport.callFrovedisNBM(fs.master_node,fdata.get(),lambda,
                               threshold,
                               fit_prior, class_prior, class_prior_length,
                               sample_weight, sample_weight_length,
                               model_id,modelType,movable,
                               fdata.is_dense())
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new NaiveBayesModel (model_id,modelType)
  }  
}

object NaiveBayes {
  def train(data: RDD[LabeledPoint], 
            lambda: Double, 
            modelType: String,
            sample_weight: Array[Double]): NaiveBayesModel = {
    return new NaiveBayes(lambda).setModelType(modelType).run(data, sample_weight)
  } 
 
  def train(data:RDD[LabeledPoint], 
            lambda: Double,
            modelType: String): NaiveBayesModel =  {
    return train(data, lambda, modelType, Array.empty[Double])
  }
 def train(data:RDD[LabeledPoint], 
            lambda: Double): NaiveBayesModel =  {
    return train(data, lambda, "multinomial", Array.empty[Double])
  }
  def train(data: RDD[LabeledPoint]): NaiveBayesModel =  {
    return train(data, 1.0, "multinomial", Array.empty[Double])
  }
  def train(fdata: FrovedisLabeledPoint,
            lambda: Double,
            modelType: String,
            sample_weight: Array[Double]): NaiveBayesModel =  {
    return new NaiveBayes(lambda).setModelType(modelType).run(fdata, sample_weight)
  }
  def train(fdata: FrovedisLabeledPoint,
            lambda: Double,
            modelType: String): NaiveBayesModel = {
    return train(fdata, lambda, modelType, Array.empty[Double])
  }
  def train(fdata: FrovedisLabeledPoint,
            lambda: Double): NaiveBayesModel = {
    return train(fdata, lambda, "multinomial", Array.empty[Double])
  }
  def train(fdata: FrovedisLabeledPoint): NaiveBayesModel = {
    return train(fdata, 1.0, "multinomial", Array.empty[Double])
  } 
}

class NaiveBayesModel (val model_Id: Int,
                       val modelType: String)
  extends GenericModelWithPredict(model_Id, M_KIND.NBM) { 
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

