package main.scala

import org.apache.log4j.{Level, Logger}
import org.apache.spark.sql.DataFrame
import org.apache.spark.SparkContext
//import org.apache.spark.sql.functions.sum
//import org.apache.spark.sql.functions.udf
import com.nec.frovedis.sql.functions.sum
import com.nec.frovedis.sql.functions.lit
import com.nec.frovedis.sql.functions.when

import com.nec.frovedis.matrix.TimeSpent
import com.nec.frovedis.sql.FrovedisDataFrame
import com.nec.frovedis.sql.implicits_._

/**
 * TPC-H Query 14
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
class Q14 extends TpchQuery {

  override def execute(sc: SparkContext, schemaProvider: TpchSchemaProvider): DataFrame = {

    // this is used to implicitly convert an RDD to a DataFrame.
    val sqlContext = new org.apache.spark.sql.SQLContext(sc)
    import sqlContext.implicits._
    import schemaProvider._

/*
 * -------- ORIGINAL --------
 *
    val reduce = udf { (x: Double, y: Double) => x * (1 - y) }
    val promo = udf { (x: String, y: Double) => if (x.startsWith("PROMO")) y else 0 }

    part.join(lineitem, $"l_partkey" === $"p_partkey" &&
      $"l_shipdate" >= "1995-09-01" && $"l_shipdate" < "1995-10-01")
      .select($"p_type", reduce($"l_extendedprice", $"l_discount").as("value"))
      .agg(sum(promo($"p_type", $"value")) * 100 / sum($"value"))
*/

    val t = new TimeSpent(Level.INFO)
    val fpart = new FrovedisDataFrame(part, "p_type", "p_partkey")
    val flineitem = new FrovedisDataFrame(lineitem, "l_partkey", "l_shipdate", "l_discount", "l_extendedprice")
    t.show("data transfer: ")

    val Creduce = $$"l_extendedprice" * (lit(1) - $$"l_discount") 
    val Cpromo = when($$"p_type".startsWith("PROMO"), $$"value").otherwise(lit(0))

    val fret = fpart.join(flineitem, $$"l_partkey" === $$"p_partkey")
      .filter($$"l_shipdate" >= "1995-09-01" && $$"l_shipdate" < "1995-10-01")
      .select($$"p_type", Creduce.as("value"))
      //.agg(sum(Cpromo * 100 / sum($$"value")) // operator on aggregator is not supported!
      .agg(sum(Cpromo * 100).as("x"), sum($$"value").as("y"))
      .select($$"x" / $$"y")
    t.show("query processing: ")

    val ret = fret.to_spark_DF()
    t.show("to_spark_DF: ")

    fpart.release()
    flineitem.release()
    return ret
  }
}
