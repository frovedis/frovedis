package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.classification.{NaiveBayes, NaiveBayesModel}
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils

// OBJECTIVE: To test Frovedis scala wrapper of NaiveBayes.train with single input/nb

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
    val m1 = NaiveBayes.train(data)
    val m2 = org.apache.spark.mllib.classification.NaiveBayes.train(data)
  
    // -------- prediction --------
    var predictionAndLabel = data.map(p => (m1.predict(p.features), p.label))
    val accuracy1 = 1.0 * predictionAndLabel.filter(x => x._1 == x._2).count() / data.count()

    predictionAndLabel = data.map(p => (m2.predict(p.features), p.label))
    val accuracy2 = 1.0 * predictionAndLabel.filter(x => x._1 == x._2).count() / data.count()

    if (accuracy1 == accuracy2) println("Status: Passed")
    else println("Status: Failed")
  
    // -------- clean-up --------
    m1.release() 

    FrovedisServer.shut_down()
    sc.stop()
  }
}



