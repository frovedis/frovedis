package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.rdd.RDD
import com.nec.frovedis.mllib.feature.StandardScaler
import com.nec.frovedis.matrix._
import org.apache.spark.mllib.linalg.Vectors


// Objective : Testing StandardScaler inverse_transform(), with fit On RDD[vect]

object GenericTest {

  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("StandardScalerDemo").setMaster("local[1]")
    val sc = new SparkContext(conf)
 
    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample file at Spark side--------
    val rdd_vec = sc.textFile("././input/kmeans_data.txt")
                    .map(x => Vectors.dense(x.split(' ').map(_.toDouble)))

    val model = new StandardScaler().setWithMean(true).setWithStd(true).setSamStd(true).fit(rdd_vec)

    // -------- training --------
    var isException = false
    try {
      model.inverse_transform(rdd_vec)
    }
    catch {
      case e: Exception => isException = true
    }

   if(isException) println("Fail")
   else println("Passed")

   model.release()
   FrovedisServer.shut_down()
   sc.stop()
 }
}
