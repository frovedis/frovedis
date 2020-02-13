package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.sql.FrovedisDataFrame
import com.nec.frovedis.sql.implicits_._
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.sql.SparkSession
import org.apache.log4j.{Level, Logger}

object FrovedisDataframeDemo {
  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)

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

    // filter demo
    df1.filter($$"Age" > 25).show()
    df1.filter($$"Age" < 25).show()
    df1.filter($$"Age" === 19).show()
    df1.filter($$"Age" !== 19).show()
    df1.filter($$"Age" >= 19).show()
    df1.filter($$"Age" <= 19).show()
    df1.filter($$"Country" === "Japan").show()
    df1.filter($$"Country" !== "USA").show()
    df1.filter(($$"Country" !== "Japan") && ($$"Age" > 19)).show()

    // sort demo
    df1.sort($$"Age").show()

    // join demo
    df1.join(df2, df1("Country") === df2("CName"),"outer","hash")
       .select("EName","Age","CCode","CName").show()

    // combined operation demo
    df1.join(df2, df1("Country") === df2("CName"),"outer","hash")
       .select("EName","Age","CCode","CName")
       .when($$"Age" > 19)
       .sort($$"CCode", $$"Age").show()

    val countryDF2 = sc.textFile("./input/country.txt")
                      .map(_.split(","))
                      .map(attributes => (attributes(0).trim, attributes(1).trim))
                      .toDF("CCode","Country")
    val df3 = new FrovedisDataFrame(countryDF2)

    // exception at frovedis server: joining table have same key name
    //df1.join(df3, df1("Country") === df3("Country"),"outer","hash")
    //   .select("EName","Age","CCode","CName").show()

    // for above case, just perform a renaming of the target key in right table (df3)
    val df4 = df3.withColumnRenamed("Country", "CName")
    df1.join(df4, df1("Country") === df4("CName"),"outer","hash")
       .select("EName","Age","CCode","CName").show()

    // groupBy demo
    df1.groupBy("Country").select("Country").show()

    // miscellaneous
    df1.withColumnRenamed("Country", "Cname").show()
    df1.show()
    peopleDF.describe().show() // spark implementation
    df1.describe().show()      // frovedis implementation
    println("Total rows: " + df1.count())
    println("Total countries: " + df1.groupBy("Country").count())
    df1.count("Age").foreach(println)
    df1.min("Age").foreach(println)
    df1.max("Age").foreach(println)
    df1.sum("Age").foreach(println)
    df1.avg("Age").foreach(println)
    df1.std("Age").foreach(println)

    val sampleDF = sc.textFile("./input/sample.txt")
                     .map(_.split(","))
                     .map(attributes => (attributes(0).trim.toInt, attributes(1).trim.toInt))
                     .toDF("A","B")
    val df5 = new FrovedisDataFrame(sampleDF)
    df5.show()

    // converting into the matrices from specific dataframe columns
    val rmat = df5.toFrovedisRowmajorMatrix(Array("A", "B"))
    val cmat = df5.toFrovedisColmajorMatrix(Array("A", "B"))
    val (crsmat1,info) = df5.toFrovedisSparseData(Array("A", "B"), Array("A"), true)
    val crsmat2 = df5.toFrovedisSparseData(info)

    // displaying the converted matrices
    rmat.debug_print()
    cmat.debug_print()
    crsmat1.debug_print()
    crsmat2.debug_print()

    // releasing the dataframe objects from frovedis side
    df1.release()
    df2.release()
    df3.release()
    df4.release()
    df5.release()
    rmat.release()
    cmat.release()
    crsmat1.release()
    crsmat2.release()
    info.release()

    FrovedisServer.shut_down()
    sc.stop()
  }
}

