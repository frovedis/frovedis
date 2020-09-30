package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.sql.FrovedisDataFrame
import com.nec.frovedis.sql.functions._
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
    
    // not operator
    df1.filter(!($$"Country" === "France")).show()
    df1.filter(not($$"Country" === "France")).show()

    // sort demo
    df1.sort(col("Country").asc, $$"Age".desc).show()

    // join demo
    df1.join(df2, df1("Country") === df2("CName"))
       .select("EName","Age","CCode","CName").show()

    // combined operation demo
    df1.join(df2, df1("Country") === df2("CName"))
       .select("EName","Age","CCode","CName")
       .when($$"Age" > 19)
       .sort($$"CCode", $$"Age").show()

    val countryDF2 = sc.textFile("./input/country.txt")
                      .map(_.split(","))
                      .map(attributes => (attributes(0).trim, attributes(1).trim))
                      .toDF("CCode","Country")
    val df3 = new FrovedisDataFrame(countryDF2)

    df1.join(df3, Seq("Country")) //SAME AS : df1("Country") === df3("Country"))
       .select("EName","Age","CCode","Country").show()

    // multi-eq
  	val country3 = Seq(
      (1, "USA", "A"),
      (2, "England", "B"),
      (3, "Japan", "C"),
      (4, "France", "F")
    )
    val countryColumns3 = Seq("CCode","CName", "City")
    val countryDF3 = country3.toDF(countryColumns3:_*)
    val country_frov1 = new FrovedisDataFrame(countryDF3)

    val country4 = Seq(
      (1, "A", 120),
      (2, "B", 240),
      (3, "C", 100),
      (4, "F", 240)
    )
    val countryColumns4 = Seq("CCode", "City", "ElectricalRating")
    val countryDF4 = country4.toDF(countryColumns4:_*)
    val country_frov2 = new FrovedisDataFrame(countryDF4)

    var df_tmp = country_frov1.join(country_frov2,
                                   $$"CCode" === $$"CCode"
                                   && $$"City" === $$"City")
    df_tmp.show()

    //non-equi
    val df_tmp2 = df_tmp.withColumnRenamed(Array("CCode", "City", "CName"),
                                          Array("CCode2", "City2", "CName2"))
    country_frov2.join(df_tmp2, $$"ElectricalRating" < $$"ElectricalRating")
                 .select("City", "City2").show()    


    // groupBy demo
    df1.groupBy("Country").count().show()
    val gdf = df1.groupBy("Country").agg(max("Age").as("max_age"),
                                         min("Age").as("min_age"),
                                         avg($$"Age").as("avg_age"),
                                         mean("Age").as("mean_age"),
                                         sum($$"Age").as("sum_age"))
    gdf.show()

    // miscellaneous
    df1.withColumnRenamed("Country", "Cname").show()
    df1.show()
    peopleDF.describe().show() // spark implementation
    df1.describe().show()      // frovedis implementation
    println("Total rows: " + df1.count())
    df1.agg(min("Age").as("min_age"),
            max("Age").as("max_age"),
            sum($$"Age").as("sum_age"),
            avg($$"Age").as("avg_age"),
            std("Age").as("stddev(age)")).show()

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
    df5.release()
    gdf.release()
    rmat.release()
    cmat.release()
    crsmat1.release()
    crsmat2.release()
    info.release()
    country_frov1.release()
    country_frov2.release()
    df_tmp.release()
    df_tmp2.release()

    FrovedisServer.shut_down()
    sc.stop()
  }
}

