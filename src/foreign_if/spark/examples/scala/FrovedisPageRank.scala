package test.scala;

import org.apache.spark.SparkConf
import org.apache.spark.SparkContext
import com.nec.frovedis.Jexrpc.FrovedisServer

object FrovedisPageRank {
  def main(args: Array[String]) {
    val conf = new SparkConf().setAppName("PageRank").setMaster("local[1]")
    val sc = new SparkContext(conf)
    val f1 = "input/urldata.dat"

    // Spark Demo
    import org.apache.spark.graphx.GraphLoader
    val sp_graph = GraphLoader.edgeListFile(sc, f1)
    val sp_res = sp_graph.pageRank(0.0001, 0.05) 
    println("...............Spark Page Rank................")
    sp_res.vertices.collect().foreach(println)

    // Frovedis Demo
    if(args.length != 0) FrovedisServer.initialize(args(0))

    //import com.nec.frovedis.graphx.GraphLoader
    val frov_graph = com.nec.frovedis.graphx.GraphLoader.edgeListFile(sc, f1)
    val frov_res = frov_graph.pageRank(0.0001, 0.05)
    println("...............Frovedis Page Rank................")
    frov_res.vertices.collect().foreach(println)
 
    println("---FROVEDIS input GRAPH---")
    frov_graph.debug_print()

    println("---FROVEDIS page rank output GRAPH---")
    frov_res.debug_print()

    println("---FROVEDIS GRAPH TO SPARK GRAPH CONVERSION---")
    val sp_grf = frov_res.to_spark_graph()
    println("vertices: ")
    sp_grf.vertices.collect().foreach(println)
    println("edges: ")
    sp_grf.edges.collect().foreach(println)

    // save -> release from server -> load from saved data -> print for verification
    frov_res.save("./out/frovedis_graph")
    frov_res.release()
    frov_res.load_text("./out/frovedis_graph")
    frov_res.debug_print()

    FrovedisServer.shut_down()
  }
}
