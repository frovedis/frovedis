package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import com.nec.frovedis.mllib.regression.{SVRModel, LinearSVR, SVMRegressionWithSGD}
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.regression.LabeledPoint
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.{Vector, Vectors}
import org.apache.log4j.{Level, Logger}

object SVRDemo {
  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)

    // -------- configurations --------
    val conf = new SparkConf().setAppName("SVRexample").setMaster("local[2]")
    val sc = new SparkContext(conf)
    sc.setLogLevel("ERROR")
    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))
    val data = Vector(
       Vectors.dense(-1,1),
       Vectors.dense(-2, -1),
       Vectors.dense(-3, -2),
       Vectors.dense(1,1),
       Vectors.dense(2, 1),
       Vectors.dense(3, 2)
    )

    val lbl = Vector(10.0, 10.0, 10.0, 20.0, 10.0, 20.0)
    val zip1 = lbl zip data
    var lpv = zip1.map( a => LabeledPoint(a._1, a._2) ) // vector of LabeledPoint
    var d_lp: RDD[LabeledPoint] = sc.parallelize(lpv)  // distributed LabeledPoint
    val f_lp = new FrovedisLabeledPoint(d_lp, true) // frovedis LabeledPoint

    val numIter = 1000
    val stepSize = 0.01
    val regParam = 0.01
    val regType = "ZERO"
    val miniBatchFraction = 1.0
    val loss = "epsilon_insensitive"
    val eps = 0.1
    val isIntercept = false
    val convergenceTol = 0.001

    println("numIter: ", numIter)
    println("stepSize: ", stepSize)
    println("regParam: ", regParam)
    println("regType: ", regType)

    //val model = SVMRegressionWithSGD.train(f_lp, numIter, stepSize, regParam, regType, miniBatchFraction, loss, eps, isIntercept, convergenceTol)
    val model = new LinearSVR().run(f_lp, false)

    println("numIter: ", numIter)
    println("stepSize: ", stepSize)
    println("regParam: ", regParam)
    println("regType: ", regType)

    var test_data = sc.parallelize(data)
    println("Prediction with trained model: ")
    val pred = model.predict(test_data)
    pred.foreach(println)

    var frov_test_data = new FrovedisRowmajorMatrix(test_data)
    println("Prediction with trained model on frovedis data: ")
    var pred2 = model.predict(frov_test_data)
    pred2.foreach(println)

    // --- save/load ---
    model.save(sc, "out/mySVMRegressionModel")
    val sameModel = SVRModel.load("out/mySVMRegressionModel")

    println("Prediction with loaded model: ")
    val pred3 = sameModel.predict(test_data)
    pred3.foreach(println)

    // -------- clean-up --------
    model.release()
    sameModel.release()
    FrovedisServer.shut_down()
    sc.stop()
  }
}

