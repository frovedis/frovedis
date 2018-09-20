package com.nec.frovedis.mllib.tree;

import collection.JavaConversions._
import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import com.nec.frovedis.mllib.{M_KIND,ModelID}
import org.apache.spark.mllib.tree.configuration.{Algo, FeatureType}
import org.apache.spark.mllib.regression.LabeledPoint
import org.apache.spark.mllib.tree.configuration.QuantileStrategy._
import org.apache.spark.mllib.tree.configuration.Strategy
import org.apache.spark.mllib.tree.impurity._
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.tree.configuration.Algo._

class DecisionTree private(val strategy: Strategy, var seed: Int){
  
  def this(strategy: Strategy) = this(strategy, seed = 0) 
	
  def setSeed(seedInput:Int): DecisionTree = {
    if(seedInput<0) throw new IllegalStateException("Illegal Seed Value")
    this.seed = seedInput
    return this
  }

  def getSeed() = this.seed

  private[frovedis] def assertValid(strategy: Strategy): Unit = {
    val algo = strategy.getAlgo()
    val impt = Impurities.toString(strategy.getImpurity)
    algo match {
      case Classification =>
        val numClasses = strategy.getNumClasses()
        require(numClasses >= 2,
          s"DecisionTree Strategy for Classification must have numClasses >= 2," +
          s" but numClasses = $numClasses.")
        require(Set("gini", "entropy").contains(impt),
          s"DecisionTree Strategy given invalid impurity for Classification: $impt." +
          s"  Valid settings: Gini, Entropy")
      case Regression =>
        require(impt == "variance",
          s"DecisionTree Strategy given invalid impurity for Regression: $impt." +
          s"  Valid settings: Variance")
      case _ =>
        throw new IllegalArgumentException(
          s"DecisionTree Strategy given invalid algo parameter: $algo." +
          s"  Valid settings are: Classification, Regression.")
    }
    val maxDepth = strategy.getMaxDepth()
    require(maxDepth >= 0, s"DecisionTree Strategy given invalid maxDepth parameter: $maxDepth." +
                           s"  Valid values are integers >= 0.")
    val maxBins = strategy.getMaxBins()
    require(maxBins >= 2, s"DecisionTree Strategy given invalid maxBins parameter: $maxBins." +
                          s"  Valid values are integers >= 2.")
    val minInstancesPerNode = strategy.getMinInstancesPerNode()
    require(minInstancesPerNode >= 1,
      s"DecisionTree Strategy requires minInstancesPerNode >= 1 but was given $minInstancesPerNode")
  }

  def run(input: RDD[LabeledPoint]): DecisionTreeModel= {
    val fdata = new FrovedisLabeledPoint(input)
    return run(fdata,true)	
  }

  def run(fdata: FrovedisLabeledPoint): DecisionTreeModel= {
    return run(fdata,false)
  }

  def run(fdata: FrovedisLabeledPoint, movable:Boolean): DecisionTreeModel = {
    assertValid(strategy)
    val model_id = ModelID.get()
    val keys = strategy.getCategoricalFeaturesInfo().keys.toArray
    val values = strategy.getCategoricalFeaturesInfo().values.toArray
    val size = keys.size
    val algo = strategy.getAlgo().toString()
    val impt = Impurities.toString(strategy.getImpurity)
    val fs = FrovedisServer.getServerInstance()
    JNISupport.callFrovedisDT(fs.master_node,fdata.get(),
                             algo,  
                             strategy.getMaxDepth(),   
		             strategy.getNumClasses(), 
			     strategy.getMaxBins(),    
			     strategy.getQuantileCalculationStrategy().toString(), 
			     strategy.getMinInfoGain(),         
			     strategy.getMinInstancesPerNode(), 
                             impt, keys, values, size,        
                             model_id, movable,
                             fdata.is_dense())       
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new DecisionTreeModel (model_id)
  }
}

object DecisionTree {
  def train(input: RDD[LabeledPoint], algo: Algo, impurity: Impurity,
	    maxDepth: Int, numClasses: Int, maxBins: Int,
  	    quantileCalculationStrategy: QuantileStrategy,
       	    categoricalFeaturesInfo: Map[Int, Int]): DecisionTreeModel = {
    
    val strategy = new Strategy(algo, impurity,
			      maxDepth, 
			      numClasses, maxBins,
       			      quantileCalculationStrategy,
			      categoricalFeaturesInfo)
    return new DecisionTree(strategy).run(input)
  }
  def train(input: RDD[LabeledPoint], algo: Algo, impurity: Impurity,
            maxDepth: Int, numClasses: Int, maxBins: Int,
            quantileCalculationStrategy: QuantileStrategy): DecisionTreeModel = {
 
    val categorical_feature = Map[Int,Int]()
    return train(input, algo,impurity,
                 maxDepth, numClasses, maxBins,
                 quantileCalculationStrategy, categorical_feature)
  }
  def train(input: RDD[LabeledPoint], algo: Algo, impurity: Impurity,
            maxDepth: Int, numClasses: Int, maxBins: Int): DecisionTreeModel = {
    
    val categorical_feature=Map[Int,Int]()
    return train(input, algo, impurity, 
                 maxDepth, numClasses, maxBins,
                 ApproxHist, categorical_feature)
  }
  def train(input: RDD[LabeledPoint], algo: Algo, impurity: Impurity,
            maxDepth: Int, numClasses: Int): DecisionTreeModel = {
    
    val categorical_feature=Map[Int,Int]()
    return train(input, algo, impurity, 
                 maxDepth, numClasses, 32, 
                 ApproxHist, categorical_feature)
  }
  def train(input: RDD[LabeledPoint],
           algo: Algo,
           impurity: Impurity,
           maxDepth: Int): DecisionTreeModel = {
 
    val categorical_feature=Map[Int,Int]()
    return train(input, algo, impurity,
                 maxDepth, 2 ,32, 
                 ApproxHist, categorical_feature)
  }
  def train(input: RDD[LabeledPoint], algo: Algo,
            impurity: Impurity): DecisionTreeModel = {
    
    val categorical_feature=Map[Int,Int]()
    return train(input, algo, impurity, 
                 5, 2, 32, 
                 ApproxHist, categorical_feature)
  }
  def train(input: RDD[LabeledPoint],
            algo: Algo): DecisionTreeModel = {
    
    val categorical_feature=Map[Int,Int]()
    return train(input, algo, Gini, 
                 5, 2, 32, 
                 ApproxHist, categorical_feature)
  }
  def train(input: RDD[LabeledPoint]): DecisionTreeModel = {
    val categorical_feature=Map[Int,Int]()
    return train(input, Classification, Gini,
                 5, 2, 32,
                 ApproxHist,categorical_feature)
  }

  // Below trains accepts input types of Frovedis data, along with other hyper-paramters
  def train(fdata:FrovedisLabeledPoint,
           algo: Algo,
           impurity: Impurity,
           maxDepth: Int,
           numClasses: Int,
           maxBins: Int,
           quantileCalculationStrategy: QuantileStrategy,
           categoricalFeaturesInfo: Map[Int, Int]): DecisionTreeModel = {
    val strategy = new Strategy(algo, impurity, maxDepth, 
                                numClasses, maxBins, 
                                quantileCalculationStrategy,categoricalFeaturesInfo)
    return new DecisionTree(strategy).run(fdata)
  }

  def train(fdata:FrovedisLabeledPoint, algo: Algo,impurity: Impurity,
            maxDepth: Int, numClasses: Int, maxBins: Int,
            quantileCalculationStrategy: QuantileStrategy): DecisionTreeModel = {
    val categorical_feature = Map[Int,Int]()
    return train(fdata, algo,  impurity,
                 maxDepth, numClasses, maxBins,
                 quantileCalculationStrategy,categorical_feature)
  }
  def train(fdata:FrovedisLabeledPoint,
           algo: Algo,
           impurity: Impurity,
           maxDepth: Int,
           numClasses: Int,
           maxBins: Int): DecisionTreeModel = {
    val categorical_feature=Map[Int,Int]()
    return train(fdata,algo,impurity,maxDepth,numClasses,maxBins,ApproxHist,categorical_feature)
  }
  def train(fdata:FrovedisLabeledPoint,
           algo: Algo,
           impurity: Impurity,
           maxDepth: Int,
           numClasses: Int): DecisionTreeModel = {
    val categorical_feature=Map[Int,Int]()
    return train(fdata,algo,impurity,maxDepth,numClasses,32,ApproxHist,categorical_feature)
  }
  def train(fdata:FrovedisLabeledPoint,
           algo: Algo,
           impurity: Impurity,
           maxDepth: Int): DecisionTreeModel = {
    val categorical_feature=Map[Int,Int]()
    return train(fdata,algo,impurity,maxDepth,2,32,ApproxHist,categorical_feature)
  }
  def train(fdata:FrovedisLabeledPoint,
           algo: Algo,
           impurity: Impurity): DecisionTreeModel = {
    val categorical_feature=Map[Int,Int]()
    return train(fdata,algo,impurity,5,2,32,ApproxHist,categorical_feature)
  }
  def train(fdata:FrovedisLabeledPoint,
           algo: Algo): DecisionTreeModel = {
    val categorical_feature=Map[Int,Int]()
    return train(fdata,algo,Gini,5,2,32,ApproxHist,categorical_feature)
  }
  def train(fdata:FrovedisLabeledPoint): DecisionTreeModel = {
    val categorical_feature=Map[Int,Int]()
    return train(fdata,Classification,Gini,5,2,32,ApproxHist,categorical_feature)



  }
  //trainRegressor with input of type RDD type
  def trainRegressor(input: RDD[LabeledPoint],
                     categoricalFeaturesInfo: Map[Int, Int],
                     impurity: String,
                     maxDepth: Int,
                     maxBins: Int): DecisionTreeModel = { 
    val impuritiType=Impurities.fromString(impurity)
    return  train(input, Regression,impuritiType, 
		  maxDepth,0, maxBins, 
                  ApproxHist, categoricalFeaturesInfo)
  }


  //trainRegressor with frovedis data
  def trainRegressor(fdata: FrovedisLabeledPoint,
                      categoricalFeaturesInfo: Map[Int, Int],
                      impurity: String,
                      maxDepth: Int,
                      maxBins: Int) : DecisionTreeModel = {
    val impuritiType=Impurities.fromString(impurity)
    return  train(fdata, Regression, impuritiType,
                  maxDepth, 0, maxBins,
                  ApproxHist, categoricalFeaturesInfo)
  }

  
  //trainClassifier with input of RDD type
  def trainClassifier(input: RDD[LabeledPoint], 
                      numClasses: Int, 
                      categoricalFeaturesInfo: Map[Int, Int],
                      impurity: String,
                      maxDepth: Int, 
                      maxBins: Int) : DecisionTreeModel = {

    val impuritiType=Impurities.fromString(impurity)
    return train(input, Classification, impuritiType,
                 maxDepth, numClasses, maxBins,
                 ApproxHist, categoricalFeaturesInfo)
  }


  //trainClassifier with Frovedis Data
  def trainClassifier(fdata: FrovedisLabeledPoint, 
                      numClasses: Int,
                      categoricalFeaturesInfo: Map[Int, Int],
                      impurity: String,
                      maxDepth: Int, 
                      maxBins: Int): DecisionTreeModel = {
    val impuritiType = Impurities.fromString(impurity)
    return  train(fdata, Classification, impuritiType, maxDepth, numClasses,
        	maxBins, ApproxHist, categoricalFeaturesInfo)
  }
}
