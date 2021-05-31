package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.clustering.GaussianMixture
import com.nec.frovedis.mllib.clustering.GaussianMixtureModel
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.linalg.Vectors
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import org.apache.log4j.{Level, Logger}
import org.apache.spark.mllib.stat.distribution.MultivariateGaussian //REMOVE
import org.apache.spark.mllib.linalg.DenseMatrix
import org.apache.spark.mllib.linalg.Vectors


object GMMDemo {
  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)

    // -------- configurations --------
    val conf = new SparkConf().setAppName("GMMExample").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample file at Spark side--------
    val s_data = sc.textFile("./input/gmm_data.txt")
                   .map(s => Vectors.dense(s.split(' ').map(_.toDouble)))

    //------- Converting into Frovedis data
    val fdata = new FrovedisRowmajorMatrix(s_data)

    //Create GMM object
    val ncomponents = 2
    val gmm = new GaussianMixture().setK(ncomponents)
                                   .setMaxIterations(100)
    val gmm_model1 = gmm.run(fdata)
    val single = Vectors.dense(1.0, 2.0)
    println("Single Predict : ")
    println(gmm_model1.predict(single))
    println("Parallel predict : ")
    gmm_model1.predict(s_data).collect().foreach(println)

    println("Single Predict Proba: ")
    gmm_model1.predictSoft(single).foreach(println)
    println("Parallel predict : ")
    val probs = gmm_model1.predictSoft(s_data).collect()
    //Print values
    for(vals <- probs) {
      for(j <- vals) {
        print(j + " ")
      }
      println("")
    }      
      
    gmm_model1.save("./out/GMMModelSpark")
      
    //gmm_model1.debug_print()

    for (i <- 0 until ncomponents) {
      println("weight=%f\nmu=%s\nsigma=\n%s\n" format
        (gmm_model1.weights(i), gmm_model1.gaussians(i).mu,
                                gmm_model1.gaussians(i).sigma))
    }
      
    println("No. of iters : " + gmm_model1.n_iter)
    println("Log likelihood : " + gmm_model1.lower_bound)  

    val gmm_model2 = GaussianMixtureModel.load("./out/GMMModelSpark")
    println("")
    gmm_model2.debug_print()
  }
}
