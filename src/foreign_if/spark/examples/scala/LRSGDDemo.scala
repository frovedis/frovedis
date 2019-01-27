package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.classification.LogisticRegressionWithSGD
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils
import org.apache.spark.mllib.evaluation.MulticlassMetrics
import org.apache.spark.mllib.linalg.Vectors
import org.apache.spark.mllib.regression.LabeledPoint

object LRSGDDemo {
  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("LRWithSGDExample").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample libSVM file at Spark side--------
    var s_data = MLUtils.loadLibSVMFile(sc, "./input/libSVMFile.txt")

    // -------- train with default training params --------
    val m1 = LogisticRegressionWithSGD.train(s_data) 

    // prediction with single test input
    var tvec = Vectors.dense(Array(1.9, 0.0, 0.0, 2.3, 4.9, 0.0, 0.0))
    println("single-input prediction made on model:")
    m1.debug_print()
    println(m1.toString())
    // will connect to Frovedis master node and get the predicted result back
    println("predicted val: " + m1.predict(tvec)) 

    // prediction with multiple test inputs
    println("multi-input prediction made on model:")
    m1.debug_print()

    // It is [OK] to connect to worker nodes and perform prediction in parallel
    var t_vec2 = s_data.map(_.features)
    m1.predict(t_vec2).collect.foreach(println)

    // -------------------------------- :Note: ----------------------------
    // This kind of lambda expression is really useful in terms of spark's own ML predict().
    // But since it is called with map, all spark worker nodes will take part in this
    // and all of them will invoke singlePrediction version at Frovedis master node.
    // Thus one-by-one crs_vector_local creation at both spark (ScalaCRS) and Frovedis side
    // and making Frovedis master node connection for single input prediction will make this process 
    // extremely slow with current wrapper implementation of LogisticRegressionModel::predict()
    // ---------------------------------------------------------------------
    val predictionAndLabels = s_data.map { case LabeledPoint(label, point) =>
       val pred = m1.predict(point)
       (label, pred)
    }
    // Get evaluation metrics.
    val metrics = new MulticlassMetrics(predictionAndLabels)
    val accuracy = metrics.accuracy
    println(s"Accuracy = $accuracy")

    FrovedisServer.shut_down();
    sc.stop()
  }
}
