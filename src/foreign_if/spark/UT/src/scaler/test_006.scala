package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.log4j.{Level, Logger}
import org.apache.spark.sql.SparkSession
import com.nec.frovedis.mllib.feature.StandardScaler
import com.nec.frovedis.matrix._
import org.apache.spark.sql.{Dataset, Row}
import org.apache.spark.sql.functions.col

// Objective : Testing for accessing 'stddev_' attribute after calling fit() on dataset- Sparse data

object GenericTest {

  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("StandardScalerExample").setMaster("local[1]")
    val sc = new SparkContext(conf)
    val spark = SparkSession.builder.appName("StandardScalerExample").getOrCreate()

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample file at Spark side--------
    val dataset = spark.read.format("libsvm").load("././input/libSVMFile.txt") //DataFrame

    val model = new StandardScaler().setWithMean(false).setWithStd(true).setSamStd(true).fit(dataset)

    var isException = false
    try {
      model.stddev
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
   
