package com.nec.frovedis.mllib.tree;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport,MemPair}
import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import com.nec.frovedis.mllib.{M_KIND,ModelID}
import org.apache.spark.mllib.tree.configuration.{Algo, FeatureType}
import org.apache.spark.mllib.regression.LabeledPoint
import org.apache.spark.mllib.tree.configuration.QuantileStrategy._
import org.apache.spark.mllib.tree.configuration.Strategy
import org.apache.spark.mllib.tree.configuration.Algo._
import org.apache.spark.mllib.tree.impurity._
import org.apache.spark.rdd.RDD

class RandomForest private(val strategy: Strategy, var numTrees: Int,
                          var featureSubsetStrategy: String = "auto",
                          var seed: Int = 0){
  
  def this(strategy: Strategy) = this(strategy, numTrees = 5, "auto", seed = 0)
	
  def setAlgo(algorithm: String): this.type = {
    require(Set("Classification", "Regression").contains(algorithm),
          s"Given invalid Algorithm for RandomForest: $algorithm." +
          s"  Valid settings: Classification, Regression ")
    this.strategy.setAlgo(algorithm)
    return this
  }

  def getAlgo() = this.strategy.getAlgo

  // gini, entropy - classifier, variance-> regressor
  def setImpurity(impurityType: Impurity): this.type = {
    this.strategy.setImpurity(impurityType)
    return this
  }

  def getImpurity() = this.strategy.getImpurity

  def setMaxDepth(depthInput: Int): this.type = {
    if(depthInput < 0) throw new IllegalStateException("Illegal maxDepth Value")
    this.strategy.setMaxDepth(depthInput)
    return this
  }
  
  def getMaxDepth() = this.strategy.getMaxDepth

  def setNumClasses(classInput: Int): this.type = {
    if(classInput < 0) throw new IllegalStateException("Illegal numClasses Value")
    this.strategy.setNumClasses(classInput)
    return this
  }

  def getNumClasses() = this.strategy.getNumClasses

  def setMaxBins(binInput: Int): this.type = {
    if(binInput < 0) throw new IllegalStateException("Illegal maxBins Value")
    this.strategy.setMaxBins(binInput)
    return this
  }

  def getMaxBins() = this.strategy.getMaxBins

  def setNumTrees(treeInput: Int): this.type = {
    if(treeInput < 0) throw new IllegalStateException("Illegal numTrees Value")
    this.numTrees = treeInput
    return this
  }

  def getNumTrees() = this.numTrees

  def setFeatureSubsetStrategy(featureSubset: String): this.type = {
    require(Set("auto", "all", "sqrt", "log2", "onethird").contains(featureSubset),
          s"Given invalid featureSubsetStrategy for RandomForest: $featureSubset." +
          s"  Valid settings: auto, all, sqrt, log2, onethird ")
    this.featureSubsetStrategy = featureSubset
    return this
  }

  def getFeatureSubsetStrategy() = this.featureSubsetStrategy

  def setSeed(seedInput: Int): this.type = {
    if(seedInput<0) throw new IllegalStateException("Illegal Seed Value")
    this.seed = seedInput
    return this
  }

  def getSeed() = this.seed

   def setSubsamplingRate(rate: Double): this.type = {
    this.strategy.setSubsamplingRate(rate)
    this
  }

  def getSubsamplingRate() = this.strategy.getSubsamplingRate

  def setMinInfoGain(gain: Double): this.type = {
    this.strategy.setMinInfoGain(gain)
    this
  }

  def getMinInfoGain() = this.strategy.getMinInfoGain

  private[frovedis] def assertValid(): Unit = {
    val algo = getAlgo()
    val impt = Impurities.toString(getImpurity)
    algo match {
      case Classification =>
        val numClasses = getNumClasses()
        require(numClasses >= 2,
          s"RandomForest Strategy for Classification must have numClasses >= 2," +
          s" but numClasses = $numClasses.")
        require(Set("gini", "entropy").contains(impt),
          s"RandomForest Strategy given invalid impurity for Classification: $impt." +
          s"  Valid settings: gini, entropy")
      case Regression =>
        require(Set("variance").contains(impt),
          s"RandomForest Strategy given invalid impurity for Regression: $impt." +
          s"  Valid settings: variance")
      case _ =>
        throw new IllegalArgumentException(
          s"RandomForest Strategy given invalid algo parameter: $algo." +
          s"  Valid settings are: Classification, Regression.")
    }
    val maxDepth = getMaxDepth()
    require(maxDepth >= 0, s"RandomForest Strategy given invalid maxDepth parameter: $maxDepth." +
                           s"  Valid values are integers >= 0.")
    val maxBins = getMaxBins()
    require(maxBins >= 2, s"RandomForest Strategy given invalid maxBins parameter: $maxBins." +
                          s"  Valid values are integers >= 2.")
  }

  def run(input: RDD[LabeledPoint]): RandomForestModel= {
    val fdata = new FrovedisLabeledPoint(input)
    return run(fdata,true)	
  }

  def run(fdata: FrovedisLabeledPoint): RandomForestModel= {
    return run(fdata,false)
  }

  def run(fdata: FrovedisLabeledPoint, movable: Boolean): RandomForestModel = {
    val model_id = ModelID.get()
    val keys = strategy.getCategoricalFeaturesInfo().keys.toArray
    val values = strategy.getCategoricalFeaturesInfo().values.toArray
    val size = keys.size
    val algo = getAlgo().toString()
    var encoded_data: MemPair = fdata.get()
    var logic: Map[Double, Double] = null
    if (algo == "Classification") {
      val ncls = fdata.get_distinct_label_count().intValue
      setNumClasses(ncls)
      val enc_ret = fdata.encode_labels()
      encoded_data = enc_ret._1
      logic = enc_ret._2
    }
    assertValid()
    val impt = Impurities.toString(getImpurity())
    val fs = FrovedisServer.getServerInstance()
    JNISupport.callFrovedisRF(fs.master_node,encoded_data,
                             algo,  
                             getMaxDepth(),
                             getMinInfoGain(),
		                         getNumClasses(), 
			                       getMaxBins(),
                             getSubsamplingRate(),
                             impt, getNumTrees(),  
                             getFeatureSubsetStrategy(),
                             getSeed(),
                             keys, values, size,        
                             model_id, movable,
                             fdata.is_dense())       
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    fdata.release_encoded_labels()
    return new RandomForestModel (model_id, logic)
  }
}

object RandomForest {
  def trainClassifier(
      input: RDD[LabeledPoint],
      strategy: Strategy,
      numTrees: Int,
      featureSubsetStrategy: String,
      seed: Int): RandomForestModel = {
    require(strategy.algo == Classification,
      s"RandomForest.trainClassifier given Strategy with invalid algo: ${strategy.algo}")
    val rf = new RandomForest(strategy, numTrees, featureSubsetStrategy, seed)
    rf.run(input)
  }

  def trainClassifier(
      input: RDD[LabeledPoint],
      numClasses: Int,
      categoricalFeaturesInfo: Map[Int, Int],
      numTrees: Int,
      featureSubsetStrategy: String,
      impurity: String,
      maxDepth: Int,
      maxBins: Int,
      seed: Int): RandomForestModel = {
    val impurityType = Impurities.fromString(impurity)
    val strategy = new Strategy(Classification, impurityType, maxDepth,
      numClasses, maxBins, Sort, categoricalFeaturesInfo)
    trainClassifier(input, strategy, numTrees, featureSubsetStrategy, seed)
  }

  def trainClassifier(
      input: FrovedisLabeledPoint,
      strategy: Strategy,
      numTrees: Int,
      featureSubsetStrategy: String,
      seed: Int): RandomForestModel = {
    require(strategy.algo == Classification,
      s"RandomForest.trainClassifier given Strategy with invalid algo: ${strategy.algo}")
    val rf = new RandomForest(strategy, numTrees, featureSubsetStrategy, seed)
    rf.run(input)
  }

  def trainClassifier(
      input: FrovedisLabeledPoint,
      numClasses: Int,
      categoricalFeaturesInfo: Map[Int, Int],
      numTrees: Int,
      featureSubsetStrategy: String,
      impurity: String,
      maxDepth: Int,
      maxBins: Int,
      seed: Int): RandomForestModel = {
    val impurityType = Impurities.fromString(impurity)
    val strategy = new Strategy(Classification, impurityType, maxDepth,
      numClasses, maxBins, Sort, categoricalFeaturesInfo)
    trainClassifier(input, strategy, numTrees, featureSubsetStrategy, seed)
  }

  def trainRegressor(
      input: RDD[LabeledPoint],
      strategy: Strategy,
      numTrees: Int,
      featureSubsetStrategy: String,
      seed: Int): RandomForestModel = {
    require(strategy.algo == Regression,
      s"RandomForest.trainRegressor given Strategy with invalid algo: ${strategy.algo}")
    val rf = new RandomForest(strategy, numTrees, featureSubsetStrategy, seed)
    rf.run(input)
  }

  def trainRegressor(
      input: RDD[LabeledPoint],
      categoricalFeaturesInfo: Map[Int, Int],
      numTrees: Int,
      featureSubsetStrategy: String,
      impurity: String,
      maxDepth: Int,
      maxBins: Int,
      seed: Int): RandomForestModel = {
    val impurityType = Impurities.fromString(impurity)
    val strategy = new Strategy(Regression, impurityType, maxDepth,
      0, maxBins, Sort, categoricalFeaturesInfo)
    trainRegressor(input, strategy, numTrees, featureSubsetStrategy, seed)
  }

  def trainRegressor(
      input: FrovedisLabeledPoint,
      strategy: Strategy,
      numTrees: Int,
      featureSubsetStrategy: String,
      seed: Int): RandomForestModel = {
    require(strategy.algo == Regression,
      s"RandomForest.trainRegressor given Strategy with invalid algo: ${strategy.algo}")
    val rf = new RandomForest(strategy, numTrees, featureSubsetStrategy, seed)
    rf.run(input)
  }

  def trainRegressor(
      input: FrovedisLabeledPoint,
      categoricalFeaturesInfo: Map[Int, Int],
      numTrees: Int,
      featureSubsetStrategy: String,
      impurity: String,
      maxDepth: Int,
      maxBins: Int,
      seed: Int): RandomForestModel = {
    val impurityType = Impurities.fromString(impurity)
    val strategy = new Strategy(Regression, impurityType, maxDepth,
      0, maxBins, Sort, categoricalFeaturesInfo)
    trainRegressor(input, strategy, numTrees, featureSubsetStrategy, seed)
  }
}
