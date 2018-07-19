package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.sql.FrovedisDataFrame
import com.nec.frovedis.sql.implicits_._
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.sql.SparkSession

object FrovedisDataframeDemo {
  def main(args: Array[String]): Unit = {
    // -------- configurations --------
    val spark = SparkSession.builder()
                            .appName("FrovedisDataframeDemo")
                            .config("spark.master", "local[2]")
                            .getOrCreate()
    val sc = spark.sparkContext
    import spark.implicits._
   
    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    val peopleDF = sc.textFile("./input/person2.txt")
                     .map(_.split(","))
                     .map(attributes => (attributes(0).trim.toInt, 
                                         attributes(1).trim, attributes(2).trim))
                     .toDF("Id","Name","Subject")

    // creating frovedis dataframe from spark dataframe
    val df1 = new FrovedisDataFrame(peopleDF)

    // groupBy demo
       df1.groupBy("Subject").select("Subject").show(); println
       df1.groupBy($$"Subject").select($$"Subject").show(); println
       df1.groupBy($$"Subject",$$"Name").select($$"Subject",$$"Name").show(); println
       df1.groupBy("Subject","Name").select($$"Subject",$$"Name").show(); println

    // releasing the dataframe objects from frovedis side
    df1.release()

    FrovedisServer.shut_down()
    sc.stop()
  }
}

