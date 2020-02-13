package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.clustering.SpectralEmbedding
import com.nec.frovedis.mllib.clustering.KMeans
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.linalg.Vectors
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import org.apache.log4j.{Level, Logger}

object SEADemo {
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

    val fdata = new FrovedisRowmajorMatrix(s_data)
    val algo = new SpectralEmbedding().setNumComponent(2)
                                      .setDropFirst(true)
    val m1 = algo.run(fdata)
    m1.save("./out/SEMModel")
    //m1.debug_print()

    println("affinity matrix: ")
    val aff = m1.get_affinity_matrix()
    //aff.debug_print()

    // training with precomputed affinity matrix
    val m2 = algo.setPrecomputed(true).run(aff)
    //m2.debug_print()

    //---embedding matrix---
    println("embedding matrix: ")
    val emat = m1.get_embedding_matrix()
    //emat.debug_print()
    
    // kmeans predict can be called on RDD[Vector]
    val spark_emat = emat.to_spark_RowMatrix(sc).rows
    val label = KMeans.train(emat,2).predict(spark_emat) // kind of fit-predict
    print("labels using kmeans on embed: ")
    for ( x <- label ) print(x + " ")
    println

    fdata.release()
    emat.release()
    aff.release()
    m1.release()
    m2.release()
    
    FrovedisServer.shut_down()
    sc.stop()
  }
}
