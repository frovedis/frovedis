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

    // preparing sample train data
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

    // training hyper-parameters
    val numIter = 1000
    val stepSize = 0.01
    val loss = "epsilon_insensitive"
    val eps = 0.1
    val tol = 0.001
    val svr = new LinearSVR().setNumIter(numIter)
                             .setStepSize(stepSize)
                             .setLoss(loss)
                             .setEps(eps)
                             .setConvergenceTol(tol)
    val model = svr.run(d_lp)

    // to train with alreday constructed frovedis server side data
    //val f_lp = new FrovedisLabeledPoint(d_lp) // frovedis LabeledPoint
    //val model = svr.run(f_lp)

    var test_data = sc.parallelize(data)
    println("prediction with trained model: ")
    val pred = model.predict(test_data)
    pred.collect().foreach(println)

    // --- save/load ---
    model.save(sc, "out/mySVMRegressionModel")
    val sameModel = SVRModel.load("out/mySVMRegressionModel")

    println("prediction with loaded model: ")
    val pred2 = sameModel.predict(test_data)
    pred2.collect().foreach(println)

    // -------- clean-up --------
    model.release()
    sameModel.release()
    FrovedisServer.shut_down()
    sc.stop()
  }
}

