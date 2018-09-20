package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.tree.{DecisionTree, DecisionTreeModel}
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils
import org.apache.spark.mllib.regression.LabeledPoint

// OBJECTIVE: To test Frovedis scala wrapper of DecisionTree.trainClassifier for release api

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
    val impurity = "gini"
    val maxDepth = 5
    val maxBins = 32

    // -------- training --------
    val m1 = DecisionTree.trainClassifier(data, numClasses, categoricalFeaturesInfo,impurity, maxDepth, maxBins)
    println("Model before release() : ")
    m1.debug_print()

    m1.release()
    println("Model After release() : ")
    var isServerException = false
    try {
      m1.debug_print() // exception at server
    }
    catch {
      case ex: java.rmi.ServerException =>{
         isServerException = true
       }
    }
    if(isServerException)
      println("Status : Passed") 
    else
      println("Status : Failed")

   FrovedisServer.shut_down()
   sc.stop()
  }
}



