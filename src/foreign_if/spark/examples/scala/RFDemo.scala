package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import com.nec.frovedis.mllib.tree.RandomForest
import com.nec.frovedis.mllib.tree.RandomForestModel
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.regression.LabeledPoint
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.{Vector, Vectors}
import org.apache.log4j.{Level, Logger}

object RFDemo {
  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)

    // -------- configurations --------
    val conf = new SparkConf().setAppName("RFexample").setMaster("local[2]")
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

    val numClasses = 2
    val categoricalFeaturesInfo = Map[Int, Int]()
    val numTrees = 3
    val featureSubsetStrategy = "auto" // Let the algorithm choose.
    val maxDepth = 4
    val maxBins = 32
    val seed = 0
    val model = RandomForest.trainClassifier(f_lp, numClasses, categoricalFeaturesInfo,
                                            numTrees, featureSubsetStrategy, "gini",
                                            maxDepth, maxBins, seed)
    // val model = RandomForest.trainRegressor(f_lp, numClasses, categoricalFeaturesInfo,
    //                                         numTrees, featureSubsetStrategy, "variance",
    //                                         maxDepth, maxBins, seed)

    var test_data = sc.parallelize(data)
    println("Prediction with trained model: ")
    val pred = model.predict(test_data)
    pred.foreach(println)

    var frov_test_data = new FrovedisRowmajorMatrix(test_data)
    println("Prediction with trained model on frovedis data: ")
    var pred2 = model.predict(frov_test_data)
    pred2.foreach(println)

    // --- save/load ---
    model.save(sc, "out/myRandomForestRegressionModel")
    val sameModel = RandomForestModel.load(sc, "out/myRandomForestRegressionModel")

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

