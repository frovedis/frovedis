package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.tree.{DecisionTree, DecisionTreeModel}
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils
import org.apache.spark.mllib.tree.configuration.{Algo, FeatureType}
import org.apache.spark.mllib.tree.configuration.Algo._
import org.apache.spark.mllib.tree.configuration.QuantileStrategy._
import org.apache.spark.mllib.tree.impurity._
import org.apache.spark.mllib.tree.configuration.Strategy
import org.apache.spark.mllib.regression.LabeledPoint

// OBJECTIVE: To test Frovedis scala wrapper of DecisionTree.train with 8 parameters

object GenericTest {

  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("GenericTest").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample libSVM file at Spark side--------
    var data = MLUtils.loadLibSVMFile(sc, "./input/dt/libSVMFile.txt")
    data = data.map(x => LabeledPoint(x.label,x.features.toDense)) // only supports dense at this moment
    val numClasses = 2
    val categoricalFeaturesInfo = Map[Int, Int]()
    val maxDepth = 5
    val maxBins = 32

    // -------- training --------
    val m1 = DecisionTree.train(data, Classification, Gini, maxDepth, 
                                numClasses, maxBins, ApproxHist, categoricalFeaturesInfo)
    val m2 = org.apache.spark.mllib.tree.DecisionTree.train(data, Classification, Gini, maxDepth, 
                                         numClasses, maxBins, ApproxHist, categoricalFeaturesInfo)

    // -------- prediction --------
    var predictionAndLabel = data.map(p => (m1.predict(p.features), p.label))
    val accuracy1 = 1.0 * predictionAndLabel.filter(x => x._1 == x._2).count() / data.count()

    predictionAndLabel = data.map(p => (m2.predict(p.features), p.label))
    val accuracy2 = 1.0 * predictionAndLabel.filter(x => x._1 == x._2).count() / data.count()

    m1.debug_print()
    data.map(p => m1.predict(p.features)).foreach(println)
    println("[Frovedis] Test accuracy: " + accuracy1)

    // to print spark model values
    data.map(p => m2.predict(p.features)).foreach(println)
    println("[Spark] Test accuracy: " + accuracy2)

    if (accuracy1 == accuracy2) println("Status: Passed")
    else println("Status: Failed")
  
    // -------- clean-up --------
    m1.release()
    FrovedisServer.shut_down()
    sc.stop()
  }
}



