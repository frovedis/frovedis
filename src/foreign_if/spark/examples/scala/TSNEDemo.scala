package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.manifold.TSNE
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.linalg.Vectors
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import org.apache.log4j.{Level, Logger}

object TSNEDemo {
  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)

    // -------- configurations --------
    val conf = new SparkConf().setAppName("TSNEExample").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample file at Spark side--------
    val s_data = sc.textFile("./input/tsne_data.txt")
                   .map(s => Vectors.dense(s.split(' ').map(_.toDouble)))

    val fdata = new FrovedisRowmajorMatrix(s_data)
    val algo = new TSNE().setNumComponent(2)

    val m1 = algo.run(fdata)
    m1.debug_print()

    fdata.release()
    m1.release()
    
    FrovedisServer.shut_down()
    sc.stop()
  }
}
