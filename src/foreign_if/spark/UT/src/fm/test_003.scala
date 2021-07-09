package test.scala;

import org.apache.spark.mllib.evaluation.RegressionMetrics
import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.fm.{FactorizationMachine,FactorizationMachineModel}
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils


import org.apache.log4j.{Level, Logger}
import org.apache.spark.sql.SparkSession
// Objective : test algo when stepSize not correct

object GenericTest {

  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("FactorizationMachineDemo").setMaster("local[1]")
    val sc = new SparkContext(conf)
    
    val spark = SparkSession.builder.appName("FactorizationMachineDemo").getOrCreate()
    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample libSVM file at Spark side--------
    var data = MLUtils.loadLibSVMFile(sc, "./input/fm/libSVMFile.txt")
   
    val dataset = spark.read.format("libsvm").load("././input/fm/libSVMFile.txt") //DataFrame 
    // -------- training --------
    var isException = false
    try {
      val model = FactorizationMachine.train(data,initStdev = 0.1, iter = 100,
                                             stepSize = 10.1, optimizer="SGD",
                                             isRegression = true,
                                             fitIntercept = true ,
                                             fitLinear = true,
                                             factorSize = 2,
                                             regParam = (0.0, 0.0, 0.0),
                                             miniBatchFraction = 0.07,
                                             seed = 1)

    }
    catch {
      case e: Exception => isException = true
    }
    if(isException) println("Passed")
    else println("Failed")


   FrovedisServer.shut_down()
   sc.stop()
 }
}



