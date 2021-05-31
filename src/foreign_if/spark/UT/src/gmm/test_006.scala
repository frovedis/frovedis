package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.clustering.GaussianMixture
import com.nec.frovedis.mllib.clustering.GaussianMixtureModel
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.linalg.Vectors

//OBJECTIVE: To test GaussianMixture for covariance dimensions

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
                                   .setCovarianceType("full")  
                                   .setMaxIterations(100)
                                   .setConvergenceTol(0.01)
                                   .setInitParams("kmeans")
                                   .setSeed(1)
    val gmm_model = gmm.run(fdata)
    gmm_model.save("./out/gmm_model")
    
    val new_model = GaussianMixtureModel.load("./out/gmm_model")
     
    if(new_model.k == ncomponents) println("Status: Passed")
    else println("Status: Failed")    
    
    // -------- clean-up --------
    gmm_model.release() 
 
    FrovedisServer.shut_down()
    sc.stop()
  }
}
