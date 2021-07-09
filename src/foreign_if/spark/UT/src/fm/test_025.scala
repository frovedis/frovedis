package test.scala;

import org.apache.spark.mllib.evaluation.RegressionMetrics
import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.fm.{FactorizationMachine,FactorizationMachineModel}
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils
import scala.math.sqrt

// Objective : test a regression problem and match result with Frovedis c++ output
// this test should be executed with frovedis process size 1-4

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
    val conf = new SparkConf().setAppName("FactorizationMachineDemo").setMaster("local[1]")
    val sc = new SparkContext(conf)
    
    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample libSVM file at Spark side--------
    var data = MLUtils.loadLibSVMFile(sc, "./input/fm/regression.txt")

    // -------- training --------
    val model = FactorizationMachine.train(data,initStdev = 0.1,iter = 100,
                                           stepSize = 0.01, optimizer="SGD",
                                           isRegression = true,
                                           fitIntercept = true,
                                           fitLinear = true,
                                           factorSize = 8,
                                           regParam = (0.0, 0.0, 0.1),
                                           miniBatchFraction = 1,
                                           seed = 1)

    var t_vec2 = data.map(_.features)
    var prediction =  model.predict(t_vec2).collect

    // expected output from an x86_64 system with ubuntu distribution, openmpi 1.10.1
    val e_mpi1 = Array(2.9972, 3.99248, 3.02071, 3.00474, 3.99754, 1.04026, 4.97436, 2.02321, 4.98711, 4.9904)
    val e_mpi2 = Array(2.98144, 3.95072, 2.96803, 3.03138, 3.96073, 0.17135, 1.11821, 4.83429, 2.04545, 4.84875)
    val e_mpi3 = Array(3.04798, 3.97503, 3.04907, 3.01073, 3.94718, 1.19058, 4.89789, 2.1192, 4.87063, 4.91434)
    val e_mpi4 = Array(2.9879, 3.95895, 3.01012, 3.01411, 3.92989, 0.14836, 1.11304, 4.89781, 4.90791, 4.89681)
    val expected_out = Array(e_mpi1, e_mpi2, e_mpi3, e_mpi4)
    val wsize = FrovedisServer.getServerInstance().worker_size 

    println("Obtained Prediction: ")
    prediction.foreach(println)
    println("Expected Prediction: ")
    expected_out(wsize-1).foreach(println)

    if (rmse(prediction,expected_out(wsize-1)) < 3)  println("Passed")
    else println("Failed")
  
    model.release() 
    FrovedisServer.shut_down()
    sc.stop()
 }
}



