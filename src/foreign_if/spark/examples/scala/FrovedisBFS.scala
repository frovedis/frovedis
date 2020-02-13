package test.scala;

import org.apache.spark.SparkConf
import org.apache.spark.SparkContext
import com.nec.frovedis.Jexrpc.FrovedisServer
import org.apache.log4j.{Level, Logger}

object FrovedisBFS {
  def main(args: Array[String]) {
    Logger.getLogger("org").setLevel(Level.ERROR)

    val conf = new SparkConf().setAppName("SSSP").setMaster("local[1]")
    val sc = new SparkContext(conf)

    // Frovedis Demo
    if(args.length != 0) FrovedisServer.initialize(args(0))

    val f1 = "input/urldata_bfs.dat"
    val frov_graph = com.nec.frovedis.graphx.GraphLoader.edgeListFile(sc, f1)
    val res = frov_graph.bfs()
    println("...............Frovedis BFS SUMMARY................")
    res.print_summary()

    FrovedisServer.shut_down()
  }
}
