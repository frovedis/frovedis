package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.clustering.GaussianMixture
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.linalg.Vectors

//OBJECTIVE: To test GaussianMixture for means dimensions

object GenericTest {

  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("GenericTest").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- create Frovedis row major matrix--------
    val s_data = sc.textFile("./input/gmm/gmm_data.txt")
                   .map(s => Vectors.dense(s.split(' ').map(_.toDouble)))

    //------- Converting into Frovedis data
    val fdata = new FrovedisRowmajorMatrix(s_data)    

    // -------- training --------
    val ncomponents = 2
    val gmm = new GaussianMixture().setK(ncomponents)
                                   .setMaxIterations(100)
    val gmm_model = gmm.run(fdata)
    gmm_model.predict(s_data).collect().foreach(println)
  
    if(gmm_model.gaussians(0).mu.size == fdata.numCols()) println("Status: Passed")
    else println("Status: Failed")

    // -------- clean-up --------
    gmm_model.release() 
 
    FrovedisServer.shut_down()
    sc.stop()
  }
}
