package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.tree.{DecisionTree, DecisionTreeModel}
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils
import scala.math.sqrt
import org.apache.spark.mllib.regression.LabeledPoint

// OBJECTIVE: To test Frovedis scala wrapper of DecisionTree.trainegressor

object GenericTest {
  def rmse(a: Array[Double], b: Array[Double]): Double = {
    require(a.length == b.length, "Incompatible input size for rmse calculation")
    var mse: Double  = 0.0
    for (i <- 0 to (a.length-1)) {
      var error = a(i) - b(i)
      var sq = error * error
      mse += sq
    }   
    val ret = sqrt(mse/a.length)
    println("RMSE: " + ret)
    return ret
  }

  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("GenericTest").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample libSVM file at Spark side--------
    var data = MLUtils.loadLibSVMFile(sc, "./input/dt/regression.txt")
    data = data.map(x => LabeledPoint(x.label,x.features.toDense)) // only supports dense at this moment
    val numClasses = 2
    val categoricalFeaturesInfo = Map[Int, Int]()
    val impurity = "variance" 
    val maxDepth = 5
    val maxBins = 32
   
    // -------- training --------
    val m1 = DecisionTree.trainRegressor(data, categoricalFeaturesInfo,
                                                                     impurity, maxDepth, maxBins)
    val m2 = org.apache.spark.mllib.tree.DecisionTree.trainRegressor(data, categoricalFeaturesInfo,
                                                                     impurity, maxDepth, maxBins)

    // -------- prediction --------
    val frov_pd = data.map(p => m1.predict(p.features)).collect
    val sprk_pd = data.map(p => m2.predict(p.features)).collect

    println("Frovedis prediction: ")
    frov_pd.foreach(println)
    println("Spark prediction: ")
    sprk_pd.foreach(println)


    if (rmse(frov_pd,sprk_pd) < 0.001) println("Status: Passed")
    else println("Status: Failed")
  
    // -------- clean-up --------
    m1.release() 
    FrovedisServer.shut_down()
    sc.stop()
  }
}



