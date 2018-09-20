package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.classification.{NaiveBayes, NaiveBayesModel}
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils
import java.io.FileReader
import java.io.FileNotFoundException
import java.io.IOException
import sys.process._

// OBJECTIVE: To test Frovedis scala wrapper of Naivebayes for 
// save API  

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
    val m1 = new NaiveBayes(0.8).setModelType("multinomial").run(data)
 
    var os_stat = "rm -rf ./out/NaiveBayesModel" .! 
    m1.save(sc, "./out/NaiveBayesModel")
    var isFileMissing = false 
    try{
      val f = new FileReader("./out/NaiveBayesModel/label")
    } 
    catch {
      case ex: FileNotFoundException => {
                 isFileMissing = true
      }
    }

    if(isFileMissing) println("Failed")
    else println("Passed")

    // -------- clean-up --------
    os_stat = "rm -rf ./out/NaiveBayesModel" .! 
    m1.release() 
    FrovedisServer.shut_down()
    sc.stop()
  }
}

