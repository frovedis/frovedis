package com.nec.frovedis.mllib.tree;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport,MemPair}
import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import com.nec.frovedis.mllib.{M_KIND,ModelID}
import com.nec.frovedis.mllib.tree.{Losses=> FrovLosses}
import org.apache.spark.mllib.regression.LabeledPoint
import org.apache.spark.mllib.tree.configuration.Strategy
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.tree.configuration.Algo._
import org.apache.spark.mllib.tree.configuration.BoostingStrategy
import org.apache.spark.mllib.tree.loss._


class GradientBoostedTrees private(val boostingStrategy: BoostingStrategy,
                                  var seed: Int,
                                  var impurity: String = "default"){

  private var featureSubsetStrategy: String = "auto"
  
  def this(boostingStrategy: BoostingStrategy) = this(boostingStrategy,
                                                      seed = 0,
                                                      "default")
  def assertValid(): Unit = {
    val loss = FrovLosses.toString(boostingStrategy.getLoss)
    val algo = getAlgo()
    val learningRate = getLearningRate()
    val numIterations = getNumIterations()
    val maxDepth = getMaxDepth()
    val maxBins = getMaxBins()
    val subsamplingRate = getSubsamplingRate()

    algo match {
      case Classification =>
        require(boostingStrategy.treeStrategy.numClasses == 2,
          "Only binary classification is supported for boosting.")
        require(Set("logloss").contains(loss),
          "Unspported loss type for GBT classifier: $loss")
      case Regression =>
        require(Set("leastsquareserror", "leastabsoluteerror").contains(loss),
          "Unspported loss type for GBT regressor: $loss")
      case _ =>
        throw new IllegalArgumentException(
          s"BoostingStrategy given invalid algo parameter:"+
          s" ${algo}." +
          s"  Valid settings are: Classification, Regression.")
    }
    
    require(learningRate > 0 && learningRate <= 1,
      "Learning rate should be in range (0, 1]. Provided learning rate is: " +
      s"$learningRate.")

    require(numIterations > 0,
      "Number of iterations should be > 0. Provided numIterations is: "+
      s"$numIterations.")

    require(maxDepth >= 0,
      s"Strategy given invalid maxDepth parameter: "+
      s"$maxDepth. Valid values are integers >= 0.")

    require(maxBins >= 2,
      s"Strategy given invalid maxBins parameter:"+
      s" $maxBins. Valid values are integers >= 2.")

    require(subsamplingRate > 0 && subsamplingRate <= 1,
      s"Strategy requires subsamplingRate <=1 and >0, but was given: " +
      s"$subsamplingRate")
  } 
	
  def setSeed(seedInput: Int): this.type = {
    if(seedInput < 0) throw new IllegalStateException("Illegal Seed Value")
    this.seed = seedInput
    return this
  }

  def getSeed() = this.seed

  // all impurities applicable for classification and regression cases
  def setImpurity(impt: String): this.type = {
    require(Set("friedman_mse", "mae", "mse", "default").contains(impt),
          s"Given invalid impurity for GBT: $impt." +
          s"Valid settings: friedman_mse, mae, mse, default.")
    this.impurity = impt
    return this
  }

  def getImpurity() = this.impurity

  def setLoss(loss: Loss): this.type = {
    this.boostingStrategy.setLoss(loss)
    this
  }

  def getLoss() =  this.boostingStrategy.getLoss

  def setLearningRate(lr: Double): this.type = {
    this.boostingStrategy.setLearningRate(lr)
    this
  }

  def getLearningRate() = this.boostingStrategy.getLearningRate

  def setMaxDepth(md: Int): this.type = {
    this.boostingStrategy.getTreeStrategy.setMaxDepth(md)
    this
  }

  def getMaxDepth() = this.boostingStrategy.getTreeStrategy.getMaxDepth

  def setMinInfoGain(gain: Double): this.type = {
    this.boostingStrategy.getTreeStrategy.setMinInfoGain(gain)
    this
  }

  def getMinInfoGain() = this.boostingStrategy.getTreeStrategy.getMinInfoGain

  def setValidationTol(tol: Double): this.type = {
    this.boostingStrategy.setValidationTol(tol)
    this
  }

  def getValidationTol() = this.boostingStrategy.getValidationTol

  def setMaxBins(bins: Int): this.type = {
    this.boostingStrategy.getTreeStrategy.setMaxBins(bins)
    this
  }

  def getMaxBins() = this.boostingStrategy.getTreeStrategy.getMaxBins

  def setSubsamplingRate(rate: Double): this.type = {
    this.boostingStrategy.getTreeStrategy.setSubsamplingRate(rate)
    this
  }

  def getSubsamplingRate() = this.boostingStrategy.getTreeStrategy
                                 .getSubsamplingRate

  def setFeatureSubsetStrategy(strategy: String): this.type ={
    require(Set("auto", "all", "sqrt", "log2", "onethird").contains(strategy),
          s"Given invalid featureSubsetStrategy for "+
          s" GradientBoostedTrees: $strategy" +
          s" Valid settings: auto, all, sqrt, log2, onethird ")
    this.featureSubsetStrategy = strategy
    this
  }

  def getFeatureSubsetStrategy() = this.featureSubsetStrategy

  def setNumIterations(n: Int): this.type = {
    this.boostingStrategy.setNumIterations(n)
    this
  }

  def getNumIterations() = this.boostingStrategy.getNumIterations

  def setNumClasses(ncls: Int): this.type = {
    this.boostingStrategy.getTreeStrategy.setNumClasses(ncls)
    this
  }

  def getNumClasses() = this.boostingStrategy.getTreeStrategy
                            .getNumClasses

  def setAlgo(algo: String): this.type = {
    algo match {
      case "Classification" =>
        boostingStrategy.treeStrategy.algo = Classification
      case "Regression" =>
        boostingStrategy.treeStrategy.algo = Regression
      case _ =>
        throw new IllegalArgumentException(
          s"Given invalid algo parameter: ${algo}.")
    }
    this
  }

  def getAlgo() = this.boostingStrategy.getTreeStrategy.getAlgo


  def run(input: RDD[LabeledPoint]): GradientBoostedTreesModel= {
    val fdata = new FrovedisLabeledPoint(input)
    return run(fdata,true)	
  }

  def run(fdata: FrovedisLabeledPoint): GradientBoostedTreesModel= {
    return run(fdata,false)
  }

  def run(fdata: FrovedisLabeledPoint, movable: Boolean): GradientBoostedTreesModel = {
    val model_id = ModelID.get()
    val keys = boostingStrategy.getTreeStrategy.getCategoricalFeaturesInfo()
                              .keys.toArray
    val values = boostingStrategy.getTreeStrategy.getCategoricalFeaturesInfo()
                                .values.toArray
    val size = keys.size
    val algo = boostingStrategy.getTreeStrategy.getAlgo().toString()
    var encoded_data: MemPair = fdata.get()
    var logic: Map[Double, Double] = null
    if (algo == "Classification") {
      val ncls = fdata.get_distinct_label_count().intValue
      setNumClasses(ncls)
      var enc_ret: (MemPair,  Map[Double, Double]) = null
      if (ncls > 2) enc_ret = fdata.encode_labels()
      else enc_ret = fdata.encode_labels(Array(-1.0, 1.0))
      encoded_data = enc_ret._1
      logic = enc_ret._2
    }
    assertValid()
    val loss = FrovLosses.toString(boostingStrategy.getLoss)
    val fs = FrovedisServer.getServerInstance()     
    JNISupport.callFrovedisGbtFit(fs.master_node,
                                  encoded_data,
                                  algo,
                                  loss,
                                  getImpurity(),
                                  getLearningRate(),
                                  getMaxDepth(),
                                  getMinInfoGain(),
                                  getSeed(),
                                  getValidationTol(),
                                  getMaxBins(),
                                  getSubsamplingRate(),
                                  featureSubsetStrategy,
                                  getNumIterations(),
                                  getNumClasses(),
                                  keys,
                                  values,
                                  size,
                                  model_id,
                                  movable,
                                  fdata.is_dense())
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    fdata.release_encoded_labels()
    return new GradientBoostedTreesModel(model_id, logic)
  }
}

object GradientBoostedTrees {
  def train(
      input: RDD[LabeledPoint],
      boostingStrategy: BoostingStrategy,
      impurity: String = "default"): GradientBoostedTreesModel = {
    new GradientBoostedTrees(boostingStrategy, seed = 0, impurity).run(input)
  }

  def train(
      input: FrovedisLabeledPoint,
      boostingStrategy: BoostingStrategy,
      impurity: String): GradientBoostedTreesModel = {
    new GradientBoostedTrees(boostingStrategy, seed = 0, impurity).run(input)
  }

  def train(
      input: FrovedisLabeledPoint,
      boostingStrategy: BoostingStrategy): GradientBoostedTreesModel = {
    new GradientBoostedTrees(boostingStrategy, seed = 0, "default").run(input)
  }
}
