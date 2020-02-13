package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.classification.LogisticRegression
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils
import org.apache.spark.mllib.linalg.Vectors
import org.apache.spark.mllib.regression.LabeledPoint
import org.apache.log4j.{Level, Logger}

object LRSGDDemo {
  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)

    // -------- configurations --------
    val conf = new SparkConf().setAppName("LRWithSGDExample").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample libSVM file at Spark side--------
    var binary_data = MLUtils.loadLibSVMFile(sc, "./input/libSVMFile.txt")
    var multi_data = MLUtils.loadLibSVMFile(sc, "./input/iris")

    // -------- train for binomial logistic regression with default training params --------
    val m1 = new LogisticRegression().setFamily("binomial")
                                     .setSolver("sgd")
                                     .fit(binary_data) // data conversion + training
    m1.debug_print()

    // prediction with single test input
    var tvec = Vectors.dense(Array(1.9, 0.0, 0.0, 2.3, 4.9, 0.0, 0.0))
    println("single-input prediction made on model:")
    println("predicted val: " + m1.predict(tvec)) //single value prediction 

    // prediction with multiple test inputs
    var tvec2 = binary_data.map(_.features)
    println("multi-input prediction made on binary model:")
    m1.predict(tvec2).collect.foreach(println)
    
    // save  model
    m1.save("./out/BinaryLogisticRegressionModel")

    // -------- train for multinomial logistic regression with default training params --------
    val m2 = new LogisticRegression().setFamily("multinomial")
                                     .setSolver("sgd")
                                     .fit(multi_data) // data conversion + training
    m2.debug_print()
    println("multi-input prediction made on multinomial model:")
    tvec2 = multi_data.map(_.features)
    m2.predict(tvec2).collect.foreach(println)

    // save  model
    m2.save("./out/MultinomialLogisticRegressionModel")

    m1.release()
    m2.release()
    FrovedisServer.shut_down();
    sc.stop()
  }
}
