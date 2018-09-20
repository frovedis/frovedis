package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.classification.{NaiveBayes, NaiveBayesModel}
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils

object NaiveBayesDemo {

  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("NaiveBayesDemo").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample libSVM file at Spark side--------
    var data = MLUtils.loadLibSVMFile(sc, "./input/libSVMFile.txt")

    // -------- training --------
    val model = NaiveBayes.train(data, 0.8)
    model.debug_print()
  
    // -------- prediction --------
    val predictionAndLabel = data.map(p => (model.predict(p.features), p.label))
    val accuracy = 1.0 * predictionAndLabel.filter(x => x._1 == x._2).count() / data.count()
    println("Test accuracy: " + accuracy)
  
    // -------- load/save --------
    model.save(sc, "./out/NaiveBayesModel")
    val model2 = NaiveBayesModel.load(sc, "./out/NaiveBayesModel")
    model2.debug_print()

    // -------- clean-up --------
    model.release() 
    model2.release() 

    FrovedisServer.shut_down()
    sc.stop()
  }
}



