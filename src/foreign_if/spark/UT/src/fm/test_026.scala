package test.scala;

import org.apache.spark.mllib.evaluation.RegressionMetrics
import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.fm.{FactorizationMachine,FactorizationMachineModel}
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils
import scala.math.sqrt

// Objective : test a classification problem and match result with Frovedis c++ output

object GenericTest {

  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("FactorizationMachineDemo").setMaster("local[1]")
    val sc = new SparkContext(conf)
    
    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample libSVM file at Spark side--------
    var data = MLUtils.loadLibSVMFile(sc, "./input/fm/libSVMFile.txt")
    println("data loading")
    // -------- training --------
    val model = FactorizationMachine.train(data,initStdev = 0.1,iter = 100,
                                           stepSize = 0.1, optimizer="SGD",
                                           isRegression = false,
                                           fitIntercept = true,
                                           fitLinear = true,
                                           factorSize = 8,
                                           regParam = (0.0, 0.0, 0.1),
                                           miniBatchFraction = 0.2,
                                           seed = 1)

    var t_data = data.map(_.features)
    var t_lbl = data.map(_.label).collect
    var prediction =  model.predict(t_data).collect
     
    println("Obtained: ")
    prediction.foreach(println)
    println("Expected: ")
    t_lbl.foreach(println)

    if (prediction.deep == t_lbl.deep)  println("Passed")
    else println("Failed")
  
    model.release() 
    FrovedisServer.shut_down()
    sc.stop()
 }
}



