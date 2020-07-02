package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import com.nec.frovedis.mllib.classification.{LogisticRegressionWithSGD,LogisticRegressionModel}
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils
import org.apache.spark.mllib.evaluation.MulticlassMetrics
import org.apache.spark.mllib.linalg.Vectors
import org.apache.spark.mllib.regression.LabeledPoint
import org.apache.log4j.{Level, Logger}

object LRModelDemo {
  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)

    // -------- configurations --------
    val conf = new SparkConf().setAppName("LRMExample").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample libSVM file at Spark side--------
    var s_data = MLUtils.loadLibSVMFile(sc, "./input/libSVMFile.txt")

    // -------- train with default training params --------
    // Frovedis side data creation and deletion will implicitly take place
    val m1 = LogisticRegressionWithSGD.train(s_data)

    /*   
    // User will create Frovedis side data by himself and post the training, 
    // he has to release the same to avoid memory leak at Frovedis side.
    val fdata = new FrovedisLabeledPoint(s_data)
    println("Frovedis Data: ")
    fdata.debug_print()
    val m1 = LogisticRegressionWithSGD.train(fdata)
    fdata.release();
    println("Frovedis Data is released")
    */

    println("trained model: ")
    m1.debug_print()
    println(m1.toString)

    var t_vec2 = s_data.map(_.features)
    println("[predict]: multi-input prediction on trained model:")
    m1.predict(t_vec2).collect.foreach(println)

    val thr = m1.getThreshold
    println("current-threshold: " + thr + "........clearing threshold")
    m1.clearThreshold()

    println("[predict-probability]: multi-input prediction:")
    m1.predict(t_vec2).collect.foreach(println)

    println("setting back the older threshold: " + thr)
    m1.setThreshold(thr)
    
    // -------- load/save --------
    println("saving model to ./out/MyLRModel:")
    m1.save("./out/MyLRModel")

    println("loading same model from ./out/MyLRModel:")
    val m2 = LogisticRegressionModel.load(sc,"./out/MyLRModel") 
    println("prediction on loaded model: ")
    m2.predict(t_vec2).collect.foreach(println) // prediction on loaded model

    // -------------------------------- :Note: ----------------------------
    // This kind of lambda expression is really useful in terms of spark's own ML predict().
    // But since it is called with map, all spark worker nodes will take part in this
    // and all of them will invoke singlePrediction version at Frovedis master node.
    // Thus one-by-one crs_vector_local creation at both spark (ScalaCRS) and Frovedis side
    // and making Frovedis master node connection for single input prediction will make this process 
    // extremely slow with current wrapper implementation of LogisticRegressionModel::predict()
    // ---------------------------------------------------------------------

    
    val predictionAndLabels = s_data.map { case LabeledPoint(label, point) =>
       val pred = m2.predict(point)
       (label, pred)
    }
    // Get evaluation metrics.
    val metrics = new MulticlassMetrics(predictionAndLabels)
    val accuracy = metrics.accuracy
    println("prediction made on loaded model:")
    println(s"Test Accuracy = $accuracy") 
    
    // -------- clean-up --------
    m1.release() 
    m2.release() 

    FrovedisServer.shut_down()
    sc.stop()
  }
}
