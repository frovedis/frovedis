package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.clustering.SpectralEmbedding
import com.nec.frovedis.mllib.clustering.KMeans
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.linalg.Vectors
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.mllib.neighbors.NearestNeighbors

object KNNDemo {
  def main(args: Array[String]): Unit = {
    // -------- configurations --------
    val conf = new SparkConf().setAppName("KNNExample").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample file at Spark side--------
    val s_data = sc.textFile("./input/knn_data.txt")
                   .map(s => Vectors.dense(s.split(' ').map(_.toDouble)))

    val fdata = new FrovedisRowmajorMatrix(s_data)
   
    val algo = new NearestNeighbors().setNNeighbors(3)
                                      .setRadius(2.0F)
                                      .setAlgorithm("brute")
                                      .setMetric("euclidean")
    println("\n\n for frovedis data \n\n")
    val m1 = algo.run(fdata)
    println("fit ok")
    var (dist, ind) = m1.kneighbors(fdata, 3, true);
    println("distance matrix")
    dist.debug_print()
    println("indices matrix")
    ind.debug_print()

    var graph = m1.kneighbors_graph(fdata, 3, "connectivity")
    println("knn graph :")
    graph.debug_print()

    var radius_neighbors = m1.radius_neighbors(fdata, 2.0F, true) // crs matrix
    println("radius_neighbors: ")
    radius_neighbors.debug_print()

    var radius_neighbors_graph = m1.radius_neighbors_graph(fdata, 2.0F, "connectivity") // crs matrix
    println("radius_neighbors_graph: ")
    radius_neighbors_graph.debug_print()

    println("\n\n for spark data\n\n")
    val m2 = algo.run(s_data)
    var (dist2, ind2) = m2.kneighbors(s_data);

    println("Distance Row matrix: ")
    println(dist2)
    dist2.rows.collect.foreach(println)
    println("Indices Row matrix:")
    println(ind2)
    ind2.rows.collect.foreach(println)
    
    var graph2 = m2.kneighbors_graph(s_data)
    graph2.debug_print()

    var radius_neighbors2 = m2.radius_neighbors(s_data, 2.0F, true) // crs matrix
    println("radius_neighbors: ")
    radius_neighbors2.debug_print()

    var radius_neighbors_graph2 = m2.radius_neighbors_graph(s_data, 2.0F, "connectivity") // crs matrix
    println("radius_neighbors_graph: ")
    radius_neighbors_graph2.debug_print()

    fdata.release()
    m1.release()
    m2.release()
        
    FrovedisServer.shut_down()
    sc.stop()
  }
}
