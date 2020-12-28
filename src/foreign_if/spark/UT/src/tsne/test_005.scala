package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.manifold.TSNE
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.linalg.Vectors
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.Vector

// OBJECTIVE: To test Frovedis scala wrapper of TSNE.run with RDD Dense data

object GenericTest {
  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("GenericTest").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- create dense data --------
    val vec1 = Vectors.dense(0.0,0.0,0.0,0.0)
    val vec2 = Vectors.dense(0.0,1.0,1.0,1.0)
    val vec3 = Vectors.dense(1.0,0.0,1.0,0.0)
    val vec4 = Vectors.dense(1.0,1.0,1.0,0.0)
    val vec5 = Vectors.dense(1.0,1.0,1.0,1.0)
    val dense_data = sc.parallelize(Array(vec1, vec2, vec3, vec4, vec5))
    
    // ------- training --------
    var isRDD = true
    var isException = false
    try{
      val algo = new TSNE().setNumComponent(2)
      val m1 = algo.run(dense_data)
      isRDD = m1.isInstanceOf[RDD[Vector]]
    }
    catch {
      case e: Exception => isException = true
    }
    if(isRDD) println("Passed")
    else println("Failed")

    FrovedisServer.shut_down()
    sc.stop()
  }
}
