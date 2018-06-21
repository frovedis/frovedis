package test.regression.RidgeRegression;

import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.linalg.Vectors
import org.apache.spark.mllib.regression.LabeledPoint
import org.apache.spark.mllib.regression.RidgeRegressionWithSGD

object RidgeSparkDemo {
  def main(args: Array[String]): Unit = {

    var command: String = ""
    var numiter: Int = 1000
    var hs: Int = 10
    var stepsize: Double = 0.01
    var regp: Double = 0.01
    var mb: Double = 1.0
    var opt: String = "sgd"
    var input: String = "../../../input/regression.txt"

    args.sliding(2, 2).toList.collect {
      case Array("--cmd", cmd: String)           => command = cmd
      case Array("--num_iter", iter: String)     => numiter = iter.toInt
      case Array("--step_size", step: String)    => stepsize = step.toDouble
      case Array("--reg_param", rg: String)      => regp = rg.toDouble
      case Array("--minibatch_fr", mbfr: String) => mb = mbfr.toDouble
      case Array("--hist_size", hist: String)    => hs = hist.toInt
      case Array("--opt", optimizer: String)     => opt = optimizer
      case Array("--input", data: String)        => input = data
    }

    // -------- configurations --------
    val conf = new SparkConf().setAppName("RidgeSpark") 
    val sc = new SparkContext(conf)

    // -------- data loading from sample regression data file at Spark side--------
    val data = sc.textFile(input)
    val parsedData = data.map { line =>
        val parts = line.split(',')
        LabeledPoint(parts(0).toDouble, Vectors.dense(parts(1).split(' ').map(_.toDouble)))
    }
    val splits = parsedData.randomSplit(Array(0.6, 0.4), seed = 11L)
    val training = splits(0).cache()
    val test = splits(1)

    if (opt == "sgd") {
      val m1 = RidgeRegressionWithSGD.train(training,numiter,stepsize,regp,mb)
      //val tvec = test.map(_.features)
      //m1.predict(tvec).collect.foreach(println)
      //test.map(_.label).collect.foreach(println)
      val valuesAndPreds = test.map { point =>
         val prediction = m1.predict(point.features)
         (point.label, prediction)
      }
      val MSE = valuesAndPreds.map{ case(v, p) => math.pow((v - p), 2) }.mean()
      println("MSE: " + MSE)
    }
    else if (opt == "lbfgs") {
      throw new IllegalArgumentException("Spark doesn't support RidgeRegressionWithLBFGS.");
    }

    sc.stop()
  }
}
