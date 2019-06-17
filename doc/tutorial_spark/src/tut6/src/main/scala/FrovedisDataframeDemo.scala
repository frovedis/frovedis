import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.sql.FrovedisDataFrame
import com.nec.frovedis.sql.implicits_._
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.sql.SparkSession

object FrovedisDataframeDemo extends Serializable {
  def main(args: Array[String]): Unit = {
    val spark = SparkSession.builder()
                            .appName("FrovedisDataframeDemo")
                            .config("spark.master", "local[2]")
                            .getOrCreate()
    val sc = spark.sparkContext
    import spark.implicits._

    FrovedisServer.initialize("mpirun -np 2 " + sys.env("FROVEDIS_SERVER"))
    val input = "file://" + sys.env("INSTALLPATH") + "/x86/doc/tutorial_spark/src/tut6/"
   
    val peopleDF = sc.textFile(input + "/people.txt")
                     .map(_.split(","))
                     .map(attributes => (attributes(0).trim, 
                                         attributes(1).trim.toInt,
                                         attributes(2).trim))
                     .toDF("EName","Age","Country")

    val countryDF = sc.textFile(input + "/country.txt")
                     .map(_.split(","))
                     .map(attributes => (attributes(0).trim, 
                                         attributes(1).trim))
                     .toDF("CCode","CName")

    // creating frovedis dataframe from spark dataframe
    val df1 = new FrovedisDataFrame(peopleDF)
    val df2 = new FrovedisDataFrame(countryDF)

    df1.show()
    df2.show()

    // filter demo
    df1.filter($$"Age" > 25).show()
    /*
    df1.filter($$"Age" < 25).show()
    df1.filter($$"Age" === 19).show()
    df1.filter($$"Age" !== 19).show()
    df1.filter($$"Age" >= 19).show()
    df1.filter($$"Age" <= 19).show()
    df1.filter($$"Country" === "Japan").show()
    df1.filter($$"Country" !== "USA").show()
    df1.filter(($$"Country" !== "Japan") && ($$"Age" > 19)).show()
    */

    // sort demo
    df1.sort($$"Age").show()

    // join demo
    df1.join(df2, df1("Country") === df2("CName"),"outer","hash")
       .select("EName","Age","CCode","CName").show()

    // change column name
    // df1.withColumnRenamed("Country", "Cname").show()

    // combined operation demo
    df1.join(df2, df1("Country") === df2("CName"),"outer","hash")
       .select("EName","Age","CCode","CName")
       .when($$"Age" > 19)
       .sort($$"CCode", $$"Age").show()

    // groupBy demo
    df1.groupBy("Country").select("Country").show()

    // miscellaneous
    println("Total rows: " + df1.count())
    df1.count("Age").foreach(println)
    df1.min("Age").foreach(println)
    df1.max("Age").foreach(println)
    df1.sum("Age").foreach(println)
    df1.avg("Age").foreach(println)
    df1.std("Age").foreach(println)
    //peopleDF.describe().show() // spark implementation
    df1.describe().show()      // frovedis implementation

    val sampleDF = sc.textFile(input + "/sample.txt")
                     .map(_.split(","))
                     .map(attributes => (attributes(0).trim.toInt, 
                                         attributes(1).trim.toInt))
                     .toDF("A","B")
    val df3 = new FrovedisDataFrame(sampleDF)
    df3.show()

    // converting into the matrices from specific dataframe columns
    val rmat = df3.toFrovedisRowmajorMatrix(Array("A", "B"))
    rmat.debug_print()

    //val cmat = df3.toFrovedisColmajorMatrix(Array("A", "B"))
    //cmat.debug_print()

    val (crsmat1,info) = df3.toFrovedisSparseData(Array("A", "B"), 
                                                  Array("A"), true)
    crsmat1.debug_print()

    val crsmat2 = df3.toFrovedisSparseData(info)
    crsmat2.debug_print()

    // releasing the dataframe objects from frovedis side
    df1.release()
    df2.release()
    df3.release()
    rmat.release()
    //cmat.release()
    crsmat1.release()
    crsmat2.release()
    info.release()

    FrovedisServer.shut_down()
    sc.stop()
  }
}

