package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.recommendation.ALS
import com.nec.frovedis.mllib.recommendation.MatrixFactorizationModel
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.recommendation.Rating

object ALSDemo {
  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("ALSExample").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample libSVM file at Spark side--------
    var rating = sc.textFile("./input/rating.txt")
    val s_data = rating.map(_.split(',') match { case Array(user, item, rate) =>
                             Rating(user.toInt, item.toInt, rate.toDouble)
                  })

    // -------- train with default training params --------
    val rank = 4
    val m1 = ALS.trainImplicit(s_data,rank) 

    // prediction with single test input
    println("single-input prediction made on model:")
    m1.debug_print()
    // will connect to Frovedis master node and get result back
    println("predicted val: " + m1.predict(2,2)) // second user, second product

    // prediction with multiple test inputs
    println("multi-input prediction made on model:")
    m1.debug_print()
    val d = sc.parallelize(Array((1,2), (3,4), (4,1)))
    m1.predict(d).collect.foreach(println)

    // saving model
    m1.save("./out/MyMFModel")
    println("model is saved in: ./out/MyMFModel")

    // loading same model
    val m2 = MatrixFactorizationModel.load("./out/MyMFModel")
    println("loaded model: ./out/MyMFModel:")
    m2.debug_print()

    // recommending users for given product
    println("recommending top 2 users for 2nd product:")
    m2.recommendUsers(2,2).foreach(println) 

    // recommending products for given user
    println("recommending top 2 products for 2nd user:")
    m2.recommendProducts(2,2).foreach(println) 

    FrovedisServer.shut_down()
    sc.stop()
  }
}
