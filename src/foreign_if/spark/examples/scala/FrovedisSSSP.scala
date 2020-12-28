package test.scala;

import org.apache.spark.SparkConf
import org.apache.spark.SparkContext
import com.nec.frovedis.Jexrpc.FrovedisServer
import org.apache.log4j.{Level, Logger}

object FrovedisSSSP {
  def main(args: Array[String]) {
    Logger.getLogger("org").setLevel(Level.ERROR)
    val conf = new SparkConf().setAppName("SSSP").setMaster("local[1]")
    val sc = new SparkContext(conf)

    // Frovedis Demo
    if(args.length != 0) FrovedisServer.initialize(args(0))

    val f1 = "input/urldata_sssp.dat"
    val source_vertex = 50
    val frov_graph = com.nec.frovedis.graphx.GraphLoader.edgeListFile(sc, f1)
    val res = frov_graph.sssp(source_vertex)

    println("...............Frovedis SSSP QUERY................")
    val q = res.sssp_query(30)
    println("Dist: " + q._1)
    println("Path: " + q._2)

    val arr: Array[Long] = Array(10,20,30,40,50,6)
    val query_res = res.sssp_query(arr)
    arr.zip(query_res).map { case (x, y) => println(x + ":" + y) }

    FrovedisServer.shut_down()
  }
}
