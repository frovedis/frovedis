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
                     .map(attributes => (attributes(0).trim.toLong, 
                                         attributes(1).trim, attributes(2).trim))
                     .toDF("Id","Name","Subject")

    // creating frovedis dataframe from spark dataframe
    val df1 = new FrovedisDataFrame(peopleDF)

    // filter demo
    df1.filter($$"Id" > 2).show(); println
    df1.filter(df1.col("Id") > 10).show(); println
     df1.filter(df1("Id") > 10).show(); println
    

    df1.filter($$"Id" === 3).show(); println
    df1.filter(df1.col("Id") === 3).show(); println
    df1.filter(df1("Id") === 3).show(); println

    df1.filter($$"Id" !== 2).show(); println
    df1.filter(df1.col("Id") !== 2).show(); println
    df1.filter(df1("Id") !== 2).show(); println

     
    df1.filter($$"Subject" === "sub1").show(); println
    df1.filter(df1.col("Subject") === "sub1").show(); println
   
    df1.filter(($$"Subject" === "sub2") && ($$"Id" > 1)).show(); println
    df1.filter((df1.col("Subject") === "sub1") && (df1.col("Id") > 1)).show(); println

    // releasing the dataframe objects from frovedis side
    df1.release()

    FrovedisServer.shut_down()
    sc.stop()
  }
}

