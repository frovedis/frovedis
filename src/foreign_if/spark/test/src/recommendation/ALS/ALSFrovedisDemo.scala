package test.recommendation.ALS;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.recommendation.ALS
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.recommendation.Rating

object ALSFrovedisDemo {
  def main(args: Array[String]): Unit = {

    var command: String = ""
    var rank: Int = -1
    var numiter: Int = 100
    var lambda: Double = 0.01
    var alpha: Double = 0.01
    var seed: Long = 0
    var input: String = "../../../input/rating.txt"

    args.sliding(2, 2).toList.collect {
      case Array("--cmd", cmd: String)           => command = cmd
      case Array("--num_iter", iter: String)     => numiter = iter.toInt
      case Array("--alpha", al: String)          => alpha = al.toDouble
      case Array("--lambda", lm: String)         => lambda = lm.toDouble
      case Array("--rank", r: String)            => rank = r.toInt
      case Array("--seed", s: String)            => seed = s.toLong
      case Array("--input", data: String)        => input = data
    }

    if(rank == -1) throw new IllegalArgumentException("Rank value is not provided");

    // -------- configurations --------
    val conf = new SparkConf().setAppName("ALSFrovedis") 
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(command != "") FrovedisServer.initialize(command)

    // -------- data loading from sample rating (COO) file at Spark side--------
    val data = sc.textFile(input)
    val ratings = data.map(_.split(',') match { case Array(user, item, rate) =>
                        Rating(user.toInt, item.toInt, rate.toDouble)
                  })

    // Build the recommendation model using ALS
    val model = ALS.trainImplicit(ratings,rank,numiter,lambda,alpha,seed)

    // Evaluate the model on rating data
    val usersProducts = ratings.map { case Rating(user, product, rate) =>
                        (user, product)
    }

    val predictions = model.predict(usersProducts).map { case Rating(user, product, rate) =>
                      ((user, product), rate)
    }

    val ratesAndPreds = ratings.map { case Rating(user, product, rate) =>
                      ((user, product), rate)
    }.join(predictions)

    val MSE = ratesAndPreds.map { case ((user, product), (r1, r2)) =>
                 val err = (r1 - r2)
                 err * err
    }.mean()

    println("MSE:" + MSE)
    //model.recommendProducts(2,3).foreach(println)
    //model.recommendUsers(2,3).foreach(println)

    FrovedisServer.shut_down();
    sc.stop()
  }
}
