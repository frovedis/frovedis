package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.classification.{NaiveBayes, NaiveBayesModel}
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils

// OBJECTIVE: To test Frovedis scala wrapper of NaiveBayes 
// for release API

object GenericTest {

  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("GenericTest").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample libSVM file at Spark side--------
    var data = MLUtils.loadLibSVMFile(sc, "./input/nb/bernoulli.txt")

    // -------- training --------
    val m1 = new NaiveBayes().setModelType("bernoulli").run(data)
    
    m1.release()
   
    println("Passed") // manually confirmed

    FrovedisServer.shut_down()
    sc.stop()
  }
}



