package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.clustering.SpectralEmbedding
import com.nec.frovedis.mllib.clustering.KMeans
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.linalg.{Vector, Vectors}
import org.apache.spark.mllib.regression.LabeledPoint
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.mllib.neighbors.KNeighborsRegressor
import org.apache.spark.rdd.RDD
import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import com.nec.frovedis.matrix.DoubleDvector
import com.nec.frovedis.exrpc.FrovedisSparseData
import org.apache.log4j.{Level, Logger}


object KNRDemo {
  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)

    // -------- configurations --------
    val conf = new SparkConf().setAppName("KNRDemo").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    //  -------- data loading from sample file at Spark side--------
    val s_data = sc.textFile("./input/knn_data.txt")
                   .map(s => Vectors.dense(s.split(' ').map(_.toDouble)))
    val fdata = new FrovedisRowmajorMatrix(s_data)

    val lbl = sc.parallelize(Array(1.2, 0.3, 1.1, 1.9, 1.7, 0.5))
    val lbv = (lbl zip s_data).map(a => LabeledPoint(a._1, a._2))
    val f_lp = new FrovedisLabeledPoint(lbv, true) // frovedis LabeledPoint

    val knr = new KNeighborsRegressor().setNNeighbors(3)
                                       .setAlgorithm("brute")
                                       .setMetric("euclidean")

    println("Using Frovedis data ")
    knr.run(f_lp)
    
    var (dist, ind) = knr.kneighbors(fdata, 3, true);
    println("distance matrix")
    dist.debug_print()
    println("indices matrix")
    ind.debug_print()
    
    var graph = knr.kneighbors_graph(fdata, 3, "connectivity")
    println("knn graph:")
    graph.debug_print()

    var pred: Array[Double] = knr.predict(fdata)
    println("predicted output: ")
    for(e <- pred) print(e + " ")
    
    var score = knr.score(fdata, DoubleDvector.get(lbl))
    println("score: " + score)

    println("Using Spark data")
    knr.run(lbv)

    var (dist2, ind2) = knr.kneighbors(s_data);
    println("Distance Row matrix: ")
    dist2.rows.collect().foreach(println)
    println("Indices Row matrix:")
    ind2.rows.collect().foreach(println)

    var pred2: Array[Double] = knr.predict(s_data)
    println("predicted output: ")
    for(e <- pred) print(e + " ")

    var score2 = knr.score(lbv)
    println("score: " + score)

    knr.release()
    FrovedisServer.shut_down()
    sc.stop()
  }
}
