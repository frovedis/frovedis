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
    val m1 = gmm.run(fdata)
    println("GMM converged in " + m1.n_iter + 
            " steps with training score: " + m1.lower_bound)

    val single = Vectors.dense(1.0, 2.0)
    println("Single Predict: " + m1.predict(single))
    println("Parallel predict : ") 
    m1.predict(s_data).collect().foreach(println)

    println("Single Predict Proba: ")
    for(p <- m1.predictSoft(single)) print(p + " ")
    println("")

    println("Parallel predict : ")
    val probs = m1.predictSoft(s_data).collect()
    for(vals <- probs) {
      for(j <- vals) print(j + " ")
      println("")
    }      
      
    m1.save("./out/GMMModelSpark")
    m1.debug_print()

    for (i <- 0 until ncomponents) {
      println(" weight=%f \n mu=%s \n sigma=%s\n" format
        (m1.weights(i), m1.gaussians(i).mu, m1.gaussians(i).sigma))
    }
      
    val m2 = GaussianMixtureModel.load("./out/GMMModelSpark")
    m2.debug_print()

    // --- clean up ---
    sc.stop()
    m1.release()
    m2.release()
    FrovedisServer.shut_down()
  }
}
