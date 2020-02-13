package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.clustering.SpectralClustering
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.linalg.Vectors
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import org.apache.log4j.{Level, Logger}

object SCADemo {
  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)

    // -------- configurations --------
    val conf = new SparkConf().setAppName("SPAExample").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample file at Spark side--------
    val s_data = sc.textFile("./input/kmeans_data.txt")
                   .map(s => Vectors.dense(s.split(' ').map(_.toDouble)))

    //------- Converting into Frovedis data
    val fdata = new FrovedisRowmajorMatrix(s_data)
    val algo = new SpectralClustering().setNumIteration(100)
                                       .setNumCluster(2)
    val m1 = algo.run(fdata)
    //m1.debug_print()
    print("labels: ")
    for ( x <- m1.labels ) print(x + " ")
    println

    m1.save("./out/Spectralmodel")
 
    println("affinity matrix: ")
    val aff = m1.get_affinity_matrix()
    //aff.debug_print()
    
    // training with precomputed affinity
    val m2 = algo.setPrecomputed(true).run(aff)
    //m2.debug_print()
    print("labels with precomputed affinity: ")
    for ( x <- m2.labels ) print(x + " ")
    println

    fdata.release()
    aff.release() 
    m1.release()
    m2.release()
    FrovedisServer.shut_down()
    sc.stop()
  }
}
