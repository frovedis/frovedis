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

    val peopleDF = sc.textFile("./input/people.txt")
                     .map(_.split(","))
                     .map(attributes => (attributes(0).trim, 
                                         attributes(1).trim.toInt, attributes(2).trim))
                     .toDF("EName","Age","Country")

    val countryDF = sc.textFile("./input/country.txt")
                     .map(_.split(","))
                     .map(attributes => (attributes(0).trim, attributes(1).trim))
                     .toDF("CCode","CName")

    // creating frovedis dataframe from spark dataframe
    val df1 = new FrovedisDataFrame(peopleDF)
    val df2 = new FrovedisDataFrame(countryDF)

    // join demo
    df1.join(df2, df1("Country") === df2("CName"),"outer","hash")
       .select("EName","Age","CCode","CName").show(); println

   


    // combined operation demo
    df1.join(df2, df1.col("Country") === df2.col("CName"),"outer","hash")
       .select("EName","Age","CCode","CName")
       .when($$"Age" > 19)
       .sort($$"CCode", $$"Age").show(); println

    // releasing the dataframe objects from frovedis side
    df1.release()
    df2.release()

    FrovedisServer.shut_down()
    sc.stop()
  }
}

