package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.classification.{NaiveBayes, NaiveBayesModel}
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils

// OBJECTIVE: To test Frovedis scala wrapper of NaiveBayes.train for abnormal modelType data

object GenericTest {

  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("GenericTest").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample libSVM file at Spark side--------
    var data = MLUtils.loadLibSVMFile(sc, "./input/nb/libSVMFile.txt")

    // -------- training --------
    var isException = false
    try {
      val m1 = NaiveBayes.train(data, 1.0, modelType="Gaussian") // unsupported modelType
    }
    catch {
      case e: Exception => isException = true
    }
  
    if (isException) println("Status: Passed")
    else println("Status: Failed")
  
    // -------- clean-up --------
    FrovedisServer.shut_down()
    sc.stop()
  }
}



