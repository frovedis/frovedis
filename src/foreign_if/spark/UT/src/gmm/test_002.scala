package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.clustering.GaussianMixture
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.linalg.Vectors

//OBJECTIVE: To test  GaussianMixture for weights dimension

object GenericTest {

  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("GenericTest").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- create dense data--------
    val vec1 = Vectors.dense(1.0,1.0,1.0,10.0,10.0,10.0)
    val vec2 = Vectors.dense(2.0,4.0,0.0,2.0,4.0,0.0)
    val dense_data = sc.parallelize(Array(vec1, vec2))

    // -------- training --------
    val ncomponents = 2
    val gmm = new GaussianMixture().setK(ncomponents)
                                   .setMaxIterations(100)
    val gmm_model = gmm.run(dense_data)
    gmm_model.predict(dense_data).collect().foreach(println)
    
    if(gmm_model.weights.length == ncomponents) println("Status: Passed")
    else println("Status: Failed")  
    
    // -------- clean-up --------
    gmm_model.release()
 
    FrovedisServer.shut_down()
    sc.stop()
  }
}
