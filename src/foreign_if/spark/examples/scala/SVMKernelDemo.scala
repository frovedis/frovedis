package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import com.nec.frovedis.mllib.classification.{SVC, SVCModel}
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.regression.LabeledPoint
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.{Vector, Vectors}
import org.apache.log4j.{Level, Logger}
import com.nec.frovedis.exrpc.FrovedisSparseData

object SVMKernelDemo {
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

    val lbl = Vector(10, 10, 10, 20, 10, 20)
    val zip1 = lbl zip data
    var lpv = zip1.map( a => LabeledPoint(a._1, a._2) ) // vector of LabeledPoint
    var d_lp: RDD[LabeledPoint] = sc.parallelize(lpv)  // distributed LabeledPoint
    val f_lp = new FrovedisLabeledPoint(d_lp, true) // frovedis LabeledPoint Row major for non-linear
    //val f_lp = new FrovedisLabeledPoint(d_lp, false) // frovedis LabeledPoint Column major for linear

    val C = 1.0
    val kernelType = "rbf" //poly, sigmoid, rbf, linear
    val degree = 3
    val gamma = 0.1
    val coef0 = 0.0
    val tol = 0.001
    val cacheSize = 128
    val maxIter = 100

    //val model = SVC.train(f_lp, C, kernelType, degree, gamma, coef0, tol, cacheSize, maxIter)
    val model = new SVC().setKernelType(kernelType).run(f_lp)

    println("--- DEBUG PRINT START ---")
    model.debug_print()
    println("TO_STRING: ", model.toString())
    println("--- DEBUG PRINT DONE ---")

    // --- vector ---
    var test_data = sc.parallelize(data)
    println("Prediction with trained model on vector data...start ")
    println("test_data: ", test_data.getClass)  
    val pred = model.predict(test_data)
    println("Prediction with trained model on vector data...done ")
    println("pred: ", pred.getClass)
    pred.collect().foreach(println)

    // --- RMM ---
    var frov_test_data = new FrovedisRowmajorMatrix(test_data)
    println("Prediction with trained model on frovedis data: ")
    var pred2 = model.predict(frov_test_data)
    pred2.collect().foreach(println)

    //// --- Sparse --- //
    //var sp_data = new FrovedisSparseData(test_data)
    //println("Prediction with trained model on frovedis sparse data: ")
    //var pred_sp = model.predict(sp_data)
    //pred_sp.foreach(println)

    // --- save/load ---
    println("Saving model...: ")
    model.save(sc, "out/SVCModel")
    println("...done.")
    println("Loading model...: ")
    val sameModel = SVCModel.load("out/SVCModel")
    println("...done.")

    println("Prediction with loaded model: ")
    val pred3 = sameModel.predict(test_data)
    pred3.collect().foreach(println)

    // -------- clean-up --------
    println("--- CLEAN-UP ---")
    model.release()
    //sameModel.release()
    FrovedisServer.shut_down()
    sc.stop()
  }
}
