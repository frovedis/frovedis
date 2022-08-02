package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.tsa.arima.{ARIMA,ARIMAModel}
import org.apache.spark.sql.SparkSession
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.rdd.RDD
import org.apache.log4j.{Level, Logger}

//TEST CASE DESCRIPTION:
//Testing ARIMA object when non-compatible solver is provided.

object GenericTest {
  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)

    // -------- configurations --------
    val conf = new SparkConf().setAppName("SVRexample").setMaster("local[2]")
    val sc = new SparkContext(conf)
    sc.setLogLevel("ERROR")
    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data load: RDD--------
    var s_data = sc.textFile("./input/arima.txt")
    val endog:RDD[Float] = s_data.map(x => x.toFloat)

    // -------- test --------
    var testStatus = "Passed"
    try {
        val model = new ARIMA(order = List(1,1,1),solver = "spack").fit(endog)
        model.release()
    }
    catch {
      case e: Exception => testStatus = "Failed"
    }
    println (testStatus)

    // -------- clean-up --------
    FrovedisServer.shut_down()
    sc.stop()
  }
}

