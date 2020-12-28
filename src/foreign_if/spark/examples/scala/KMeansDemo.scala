package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.clustering.KMeans
import com.nec.frovedis.mllib.clustering.KMeansModel
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.linalg.Vectors
import org.apache.log4j.{Level, Logger}

object KMeansDemo {
  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)

    // -------- configurations --------
    val conf = new SparkConf().setAppName("KmeansExample").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample file at Spark side--------
    val s_data = sc.textFile("./input/kmeans_data.txt")
                   .map(s => Vectors.dense(s.split(' ').map(_.toDouble)))

    // -------- train with default training params --------
    val k = 2
    val m1 = KMeans.train(s_data,k) 

    // prediction with single test input
    println("single-input prediction made on model:")
    m1.debug_print()
    var tvec = Vectors.dense(Array(1.9, 0.0, 2.3))
    println("predicted val: " + m1.predict(tvec))

    // prediction with multiple test inputs
    println("multi-input prediction made on model:")
    m1.debug_print()
    val test_data = sc.textFile("./input/kmeans_tdata.txt")
                      .map(s => Vectors.dense(s.split(' ').map(_.toDouble)))
    m1.predict(test_data).collect().foreach(println)

    // saving model
    m1.save("./out/MyKMeansModel")
    println("model is saved in: ./out/MyKMeansModel")

    // loading same model
    val m2 = KMeansModel.load("./out/MyKMeansModel")
    println("loaded model: ./out/MyKMeansModel:")
    m2.debug_print()

    FrovedisServer.shut_down()
    sc.stop()
  }
}
