package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.sql.FrovedisDataFrame
import com.nec.frovedis.sql.functions._
import com.nec.frovedis.sql.implicits_._
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.sql.SparkSession
import org.apache.log4j.{Level, Logger}
import org.apache.spark.sql.{ functions=>spark_functions}

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
                     .toDF("EName","Age","Country").persist

    val countryDF = sc.textFile("./input/country.txt")
                     .map(_.split(","))
                     .map(attributes => (attributes(0).trim.toInt, attributes(1).trim))
                     .toDF("CCode","CName").persist

    // creating frovedis dataframe from spark dataframe
    val df1 = new FrovedisDataFrame(peopleDF)
    val df2 = new FrovedisDataFrame(countryDF)
    // filter demo
    df1.filter($$"Age" > 25).show()
    df1.filter($$"Age" < 25).show()
    df1.filter($$"Age" === 19).show()
    df1.filter($$"Age" !== 19).show()
    df1.filter($$"Age" >= lit(19)).show() // same as df1.filter($$"Age" >= 19).show()
    df1.filter($$"Age" <= lit(19)).show() // same as df1.filter($$"Age" <= 19).show()
    df1.filter($$"Country" === "Japan").show()
    df1.filter($$"Country" =!= lit("USA")).show()
    df1.filter($$"Country" >= "Japan").show()
    df1.filter(($$"Country" =!= "Japan") && ($$"Age" > 19)).show()
    df1.filter($$"EName".startsWith("Ra")).show()
    
    // not operator
    df1.filter(!($$"Country" === "France")).show()
    df1.filter(not($$"Country" === "France")).show()

    // select demo
    df1.select(lit(2).as("const_two"),
               lit(null),
               lit(null).isNull, 
               lit(false).as("false_col"),
               $$"Age".cast("double"), 
               $$"Country" === "Japan",
               $$"Country".substr(1, 4),
               lit(2) * $$"Age", // reversed operation
               $$"Age" > 19).to_spark_DF().show()

    // sort demo
    df1.sort(col("Country").asc, $$"Age".desc).show()

    // join demo
    df1.join(df2, df2("CName") === df1("Country"))
       .select("EName","Age","CCode","CName").show()

    // combined operation demo
    df1.join(df2, $$"Country" === $$"CName")
       .select($$"EName".as("x"), ($$"Age" + 1).as("y"), $$"Country".as("z"))
       .where($$"y" > 20)
       .sort($$"x").show()

    // dwithColumn, distinct, dropDuplicates, limit demo
    val ddf = df1.withColumn("AgePlusOne", $$"Age" + 1)
       .withColumn("is_adult", $$"Age" >= 20)
       .withColumn("lit(2)", lit(2))
       .withColumn("null_col", lit(null))
       .withColumn("true_col", lit(true))
    ddf.to_spark_DF().show()
    ddf.dropDuplicates("is_adult").to_spark_DF().show()
    ddf.select("is_adult").distinct().to_spark_DF().show()
    ddf.select("is_adult").limit(3).to_spark_DF().show()

    val countryDF2 = sc.textFile("./input/country.txt")
                      .map(_.split(","))
                      .map(attributes => (attributes(0).trim, attributes(1).trim))
                      .toDF("CCode","Country").persist
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
    val countryDF3 = country3.toDF(countryColumns3:_*).persist
    val country_frov1 = new FrovedisDataFrame(countryDF3)

    val country4 = Seq(
      (1, "A", 120),
      (2, "B", 240),
      (3, "C", 100),
      (4, "F", 240)
    )
    val countryColumns4 = Seq("CCode", "City", "ElectricalRating")
    val countryDF4 = country4.toDF(countryColumns4:_*).persist
    val country_frov2 = new FrovedisDataFrame(countryDF4)

    var df_tmp = country_frov1.join(country_frov2,
                                   $$"CCode" === $$"CCode"
                                   && $$"City" === $$"City")
    df_tmp.show()
    

    //non-equi
    country_frov2.join(country_frov1, $$"CCode" < $$"CCode").show()    

    // self-join
    country_frov2.join(country_frov2, $$"ElectricalRating" < $$"ElectricalRating").show()

    // groupBy demo along with to_spark_DF()
    df1.groupBy("Country").select("Country").show()
    df1.groupBy("Country").min(pow($$"Age", 2)).show()
    df1.groupBy("Country").agg(count($$"Age" >= 20).as("no_of_adults"),
                               min("Age").as("min_age"),
                               sum($$"Age").as("sum_age"),
                               sumDistinct($$"Age"),
                               mean($$"Age").as("avg_age"),
                               first($$"Age"),
                               last($$"Age"),
                               variance($$"Age"),
                               stddev($$"Age"),
                               sem($$"Age"),
                               count($$"Age"),
                               countDistinct($$"Age")).to_spark_DF().show() // aggregate using only aggregator
    df1.groupBy("Age").agg($$"Age" * 2, $$"Age", $$"Age" + 1).to_spark_DF().show() // aggregate using only columns, no agregator
    df1.groupBy("Age").agg($$"Age" * 2, min($$"Age"), countDistinct("Country")).to_spark_DF().show() // aggregate using columns + aggregator
    df1.groupBy("Age").max().to_spark_DF().show() // group-wise (by Age) max of all numeric columns

    // miscellaneous
    df1.withColumnRenamed("Country", "Cname").show()
    peopleDF.describe().show() // spark implementation
    df1.describe().show()      // frovedis implementation
    println("Total rows: " + df1.count())
    df1.agg(min($$"Age" + lit(1)),
            max("Age").as("max_age"),
            sum($$"Age").as("sum_age"),
            avg($$"Age").as("avg_age"),
            stddev($$"Age"),
            sum(lit(2)),
            sumDistinct($$"Age"),
            countDistinct($$"Age") // ignores null, like in spark-sql
          ).show() 

    val dataWithNull = Seq(
      ("Smith","NY","M"),
      ("James","NY","F"),
      ("Julia",null,null)
    )
    val columns = Seq("name","state","gender")
    val dfWithNull = dataWithNull.toDF(columns:_*).persist
    val frov_dfWithNull = new FrovedisDataFrame(dfWithNull)
    frov_dfWithNull.show()
    frov_dfWithNull.filter($$"state".isNotNull).show()
    frov_dfWithNull.filter($$"state".isNull).show()
    frov_dfWithNull.groupBy("state").agg(count("state"), count("*")).show()
    frov_dfWithNull.release()

    val cc_df = Seq("China", "Canada", "Italy", "Tralfamadore").toDF("word").persist
    val f_cc_df = new FrovedisDataFrame(cc_df)
    f_cc_df.withColumn("continent",
        when($$"word" === lit("China"), lit("Asia"))
          .when($$"word" === lit("Canada"), lit("North Amerika"))
          .when($$"word" === lit("Italy"), lit("Europe"))
          .otherwise(lit("Not sure"))
    ).show()

    val sampleDF = sc.textFile("./input/sample.txt")
                     .map(_.split(","))
                     .map(attributes => (attributes(0).trim.toInt, attributes(1).trim.toInt))
                     .toDF("A","B").persist
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

    // datetime
    var dateDate = Seq( ("A", "02/07/2016"),
                        ("B", "08/01/2018"),
                        ("C", "18/12/2021"),
                        ("D", "03/06/2022") )
    val sp_df6 = dateDate.toDF("c1", "c2").select( spark_functions.col("c1"),
                         spark_functions.to_date( spark_functions.col("c2"), "dd/MM/yyyy").as("date_c2"),
                         spark_functions.to_timestamp( spark_functions.col("c2"), "dd/MM/yyyy").as("ts_c2")
                          )
    sp_df6.show()
    val df6 = new FrovedisDataFrame(sp_df6)
    df6.show()
    df6.to_spark_DF().show()

    // releasing the dataframe objects from frovedis side
    df1.release()
    df2.release()
    df3.release()
    df5.release()
    rmat.release()
    cmat.release()
    crsmat1.release()
    crsmat2.release()
    info.release()
    country_frov1.release()
    country_frov2.release()
    df_tmp.release()

    FrovedisServer.shut_down()
    sc.stop()
  }
}

