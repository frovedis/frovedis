package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.clustering.SpectralEmbedding
import com.nec.frovedis.mllib.clustering.KMeans
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.linalg.Vectors
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.mllib.neighbors.NearestNeighbors
import org.apache.log4j.{Level, Logger}

object KNNDemo {
  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)

    // -------- configurations --------
    val conf = new SparkConf().setAppName("KNNExample").setMaster("local[2]")
    val sc = new SparkContext(conf)
    sc.setLogLevel("ERROR")
    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample file at Spark side--------
    val s_data = sc.textFile("./input/knn_data.txt")
                   .map(s => Vectors.dense(s.split(' ').map(_.toDouble)))
    val fdata = new FrovedisRowmajorMatrix(s_data)
   
    val knn = new NearestNeighbors().setNNeighbors(3)
                                    .setRadius(2.0F)
                                    .setAlgorithm("brute")
                                    .setMetric("euclidean")
    println("Using frovedis data ")
    knn.run(fdata)
    var (dist, ind) = knn.kneighbors(fdata, 3, true);
    println("distance matrix")
    dist.debug_print()
    println("indices matrix")
    ind.debug_print()

    var graph = knn.kneighbors_graph(fdata, 3, "connectivity")
    println("knn graph :")
    graph.debug_print()

    var radius_neighbors = knn.radius_neighbors(fdata, 2.0F, true) // crs matrix
    println("radius_neighbors: ")
    radius_neighbors.debug_print()

    var radius_neighbors_graph = knn.radius_neighbors_graph(fdata, 2.0F, "connectivity") // crs matrix
    println("radius_neighbors_graph: ")
    radius_neighbors_graph.debug_print()

    println("Using spark data ")
    knn.run(s_data)
    var (dist2, ind2) = knn.kneighbors(s_data);

    println("Distance Row matrix: ")
    dist2.rows.collect.foreach(println)
    println("Indices Row matrix:")
    ind2.rows.collect.foreach(println)
    
    var graph2 = knn.kneighbors_graph(s_data)
    println("knn graph :")
    graph2.foreach(println)

    var radius_neighbors2 = knn.radius_neighbors(s_data, 2.0F, true) // crs matrix
    println("radius_neighbors: ")
    radius_neighbors2.debug_print()

    var radius_neighbors_graph2 = knn.radius_neighbors_graph(s_data, 2.0F, "connectivity") // crs matrix
    println("radius_neighbors_graph: ")
    radius_neighbors_graph2.foreach(println)

    fdata.release()
    knn.release()
        
    FrovedisServer.shut_down()
    sc.stop()
  }
}
