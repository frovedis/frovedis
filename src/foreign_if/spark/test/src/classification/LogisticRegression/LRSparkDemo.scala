package test.classification.LogisticRegression;

import org.apache.spark.mllib.classification.LogisticRegressionWithSGD
import org.apache.spark.mllib.classification.LogisticRegressionWithLBFGS
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils

object LRSparkDemo {
  def main(args: Array[String]): Unit = {

    var command: String = ""
    var numiter: Int = 1000
    var hs: Int = 10
    var stepsize: Double = 0.01
    var mb: Double = 1.0
    var opt: String = "sgd"
    var input: String = "../../../input/libSVMFile.txt"

    args.sliding(2, 2).toList.collect {
      case Array("--cmd", cmd: String)           => command = cmd
      case Array("--num_iter", iter: String)     => numiter = iter.toInt
      case Array("--step_size", step: String)    => stepsize = step.toDouble
      case Array("--minibatch_fr", mbfr: String) => mb = mbfr.toDouble
      case Array("--hist_size", hist: String)    => hs = hist.toInt
      case Array("--opt", optimizer: String)     => opt = optimizer
      case Array("--input", data: String)        => input = data
    }

    // -------- configurations --------
    val conf = new SparkConf().setAppName("LRSpark") 
    val sc = new SparkContext(conf)

    // -------- data loading from sample libSVM file at Spark side--------
    var data = MLUtils.loadLibSVMFile(sc, input)
    val splits = data.randomSplit(Array(0.6, 0.4), seed = 11L)
    val training = splits(0).cache()
    val test = splits(1)
    val tvec = test.map(_.features)

    if (opt == "sgd") {
      val m1 = LogisticRegressionWithSGD.train(training,numiter,stepsize,mb)
      m1.predict(tvec).collect.foreach(println)
    }
    else if (opt == "lbfgs") {
      val m1 = new LogisticRegressionWithLBFGS().setNumClasses(2).run(training)
      m1.predict(tvec).collect.foreach(println)
    }

    sc.stop()
  }
}
