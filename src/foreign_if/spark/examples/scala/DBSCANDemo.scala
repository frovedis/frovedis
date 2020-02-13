package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.clustering.DBSCAN
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.linalg.Vectors
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import org.apache.log4j.{Level, Logger}

object DBSCANDemo {
  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)

    // -------- configurations --------
    val conf = new SparkConf().setAppName("DBSCANDemo").setMaster("local[1]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample file at Spark side--------
    val s_data = sc.textFile("./input/kmeans_data.txt")
                   .map(s => Vectors.dense(s.split(' ').map(_.toDouble)))

    //------- Converting into Frovedis data
    val fdata = new FrovedisRowmajorMatrix(s_data)
    val algo = new DBSCAN().setEps(2)
                           .setMinSamples(3)
                           .setAlgorithm("brute")
                           .setMetric("euclidean")
    val m1 = algo.run(fdata)
    print("labels: ")
    for ( x <- m1 ) print(x + " ")
    println

    fdata.release()
    algo.release()
    FrovedisServer.shut_down()
    sc.stop()
  }
}
