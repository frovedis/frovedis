package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.clustering.KMeans
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils
import org.apache.spark.mllib.linalg.Vectors

//OBJECTIVE: To test Frovedis scala wrapper of kmeans with sgd optimizer for sparse and dense data

object GenericTest {

  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("GenericTest").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- create dense and sparse data--------
    val vec1 = Vectors.dense(1.0,0.0,0.0,1.0,1.0,0.0)
    val vec2 = Vectors.dense(1.0,0.0,1.0,0.0,1.0,0.0)
    val dense_data = sc.parallelize(Array(vec1, vec2)) 
   
    val svec1 = Vectors.sparse(6, Array(0, 3, 4), Array(1.0, 1.0, 1.0))
    val svec2 = Vectors.sparse(6, Array(0, 2, 4), Array(1.0, 1.0, 1.0))
    val sparse_data = sc.parallelize(Array(svec1, svec2)) 

    // -------- training --------
    val kclass = 2
    val m1 = KMeans.train(dense_data, kclass)
    val m2 = KMeans.train(sparse_data, kclass)
  
    // -------- prediction --------
    val pd1 = m1.predict(dense_data).collect
    val pd2 = m2.predict(dense_data).collect

    if (pd1.deep == pd2.deep) println("Status: Passed")
    else println("Status: Failed") 

    // -------- clean-up --------
    m1.release() 
    m2.release() 

    FrovedisServer.shut_down()
    sc.stop()
  }
}
