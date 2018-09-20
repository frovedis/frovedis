package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.tree.{DecisionTree, DecisionTreeModel}
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils
import java.io.FileReader
import java.io.FileNotFoundException
import java.io.IOException
import sys.process._
import org.apache.spark.mllib.regression.LabeledPoint

// OBJECTIVE: To test Frovedis scala wrapper of DecisionTree.trainClassifier for save the model 

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
    
    // deleting the previous model file, if any
    var os_stat = "rm -rf ./out/DecisionTreeModel" .! 
    m1.save(sc, "./out/DecisionTreeModel")
    var isFileMissing = false
    try {
      val f = new FileReader("./out/DecisionTreeModel/prediction")
    } 
    catch {
      case ex: FileNotFoundException =>{
         isFileMissing = true
       }
    }

    if(isFileMissing) println("Status :  failed   and   model not saved successfully")
    else println("Status : Passed") 

  
    // -------- clean-up --------
    os_stat = "rm -rf ./out/DecisionTreeModel" .! 
    m1.release() 
    FrovedisServer.shut_down()
    sc.stop()
  }
}



