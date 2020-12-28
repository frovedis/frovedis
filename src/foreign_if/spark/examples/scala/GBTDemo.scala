package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import com.nec.frovedis.mllib.tree.{GradientBoostedTrees, GradientBoostedTreesModel}
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import org.apache.spark.mllib.regression.LabeledPoint
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.{Vector, Vectors}
import org.apache.spark.mllib.tree.configuration.BoostingStrategy
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.log4j.{Level, Logger}

object GBTDemo {

  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)

    // -------- configurations --------
    val conf = new SparkConf().setAppName("GBTexample").setMaster("local[2]")
    val sc = new SparkContext(conf)

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
    val lp_vec  = (lbl zip data).map(a => LabeledPoint(a._1, a._2)) 
    val d_lp: RDD[LabeledPoint] = sc.parallelize(lp_vec) // spark RDD[LabeledPoint]
    val f_lp = new FrovedisLabeledPoint(d_lp, true)   // frovedis LabeledPoint
    
    val boostingStrategy = BoostingStrategy.defaultParams("Classification")
    // val boostingStrategy = BoostingStrategy.defaultParams("Regression")
    boostingStrategy.numIterations = 3 
    boostingStrategy.treeStrategy.maxDepth = 5
    boostingStrategy.treeStrategy.categoricalFeaturesInfo = Map[Int, Int]()

    println("Fitting data on spark data")
    val model = GradientBoostedTrees.train(d_lp, boostingStrategy)
    
    
    val single_tdata = Vectors.dense(-1,1)
    //println("single_test_data: " + single_tdata)
    println("Predicting on single test data (spark data) ")
    println(model.predict(single_tdata))

    val test_data = sc.parallelize(data)
    println("Predicting on multiple test data (spark data)")
    val pred = model.predict(test_data)
    pred.collect().foreach(println)

    val frov_test_data = new FrovedisRowmajorMatrix(test_data)
    println("Predicting on multiple test data (frovedis data)")
    val pred2 = model.predict(frov_test_data)
    pred2.collect().foreach(println)

    model.save("./out/gbt")
    model.debug_print()
    
    val model2 = GradientBoostedTreesModel.load("./out/gbt")
    println("Loaded model: ")
    model2.debug_print()

    println("Predicting on loaded model")
    val pred3 = model2.predict(test_data)
    pred3.collect().foreach(println)
    
    model.release()
    model2.release()
    FrovedisServer.shut_down()
    sc.stop()
  }
}
