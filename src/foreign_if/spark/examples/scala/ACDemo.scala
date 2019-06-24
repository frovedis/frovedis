package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.clustering.AgglomerativeClustering
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.linalg.Vectors
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix

object ACDemo {
  def main(args: Array[String]): Unit = {
    // -------- configurations --------
    val conf = new SparkConf().setAppName("ACExample").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample file at Spark side--------
    val s_data = sc.textFile("./input/kmeans_data.txt")
                   .map(s => Vectors.dense(s.split(' ').map(_.toDouble)))

    // data conversion (spark to frovedis)
    val fdata = new FrovedisRowmajorMatrix(s_data) 
    // training
    val m1 = new AgglomerativeClustering().setLinkage("average").run(fdata)

    // data conversion + training
    //val m1 = new AgglomerativeClustering().setLinkage("average").run(s_data)

    // showing model information (computed dendogram)
    m1.debug_print()

    // prediction
    print("labels: ")
    for (x <- m1.predict(2)) print(x + " ")
    println
    
    // save  model
    m1.save("./out/Agglomerativemodel")

    m1.release()
    FrovedisServer.shut_down()
    sc.stop()
  }
}
