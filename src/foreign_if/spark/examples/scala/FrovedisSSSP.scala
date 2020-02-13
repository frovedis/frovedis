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
    val source_vertex = 4
    val frov_graph = com.nec.frovedis.graphx.GraphLoader.edgeListFile(sc, f1)
    val res = frov_graph.sssp(source_vertex)

    println("...............Frovedis SSSP QUERY................")
    val q = res.sssp_query(4)
    println("Dist: " + q._1)
    println("Path: " + q._2)

    val arr: Array[Int] = Array(-1,0,1,2,3,4,5,6,50,100,101,102,100000,100001)
    val qs: Array[(Int, String)] = res.sssp_query(arr)
    qs.foreach{ println }

    FrovedisServer.shut_down()
  }
}
