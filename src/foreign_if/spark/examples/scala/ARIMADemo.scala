package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.tsa.arima.{ARIMA,ARIMAModel}
import org.apache.spark.sql.SparkSession
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.rdd.RDD
import org.apache.log4j.{Level, Logger}

object ARIMADemo {
  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)
    
    // -------- configurations --------
    val conf = new SparkConf().setAppName("FPGrowthDemo").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line 
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data load: RDD--------
    var s_data = sc.textFile("./input/arima.txt")
    //var s_data = sc.textFile("../../python/examples/input/shampoo")
    val endog:RDD[Double] = s_data.map(x => x.toDouble)
    //val endog:RDD[Float] = s_data.map(x => x.toFloat)

    // -------- fit --------
    println("--fitting--")
    val model = new ARIMA().setOrder(List(2,1,2))
                           .setSeasonal(1)
                           .setSolver("lapack")
                           .setAutoArima(true)
                           .fit(endog)
    
    // -------- fittedvalues --------
    val fitted_values = model.fittedvalues()
    println("--Fitted values--")
    fitted_values.foreach(println)

    // -------- predict --------
    val predicted_values = model.predict(start = 10, end = 11)
    println("--Predicted values--")
    predicted_values.foreach(println)

    // -------- forecast --------
    val forecasted_values = model.forecast(steps = 2)
    println("--forecasted values--")
    forecasted_values.foreach(println)

    // -------- clean-up --------
    model.release()
    FrovedisServer.shut_down()
    sc.stop()
 } 
}
  
