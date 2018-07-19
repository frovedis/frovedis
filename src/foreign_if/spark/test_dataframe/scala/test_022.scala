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

    val peopleDF = sc.textFile("./input/person.txt")
                     .map(_.split(","))
                     .map(attributes => (attributes(0).trim.toInt, 
                                         attributes(1).trim, attributes(2).trim))
                     .toDF("Id","Name","Subject")

    val countryDF = sc.textFile("./input/person2.txt")
                     .map(_.split(","))
                     .map(attributes => (attributes(0).trim.toInt,attributes(1).trim, attributes(2).trim))
                     .toDF("Pid","Pname","Psubject")

    // creating frovedis dataframe from spark dataframe
    val df1 = new FrovedisDataFrame(peopleDF)
    val df2 = new FrovedisDataFrame(countryDF)

    // join demo
    df1.join(df2, $$"Subject" === $$"Psubject","outer","hash")
       .select("Id","Name","Subject","Pname","Pid").show(); println


     df1.join(df2, $$"Subject" === $$"Psubject","inner","hash")
       .select("Id","Name","Subject","Pname","Pid").show(); println
    









    // releasing the dataframe objects from frovedis side
    df1.release()
    df2.release()

    FrovedisServer.shut_down()
    sc.stop()
  }
}

