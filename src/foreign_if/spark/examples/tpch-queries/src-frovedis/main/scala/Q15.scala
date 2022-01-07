package main.scala

import org.apache.log4j.{Level, Logger}
import org.apache.spark.sql.DataFrame
import org.apache.spark.SparkContext
//import org.apache.spark.sql.functions.max
//import org.apache.spark.sql.functions.sum
//import org.apache.spark.sql.functions.udf
import com.nec.frovedis.sql.functions.max
import com.nec.frovedis.sql.functions.sum
import com.nec.frovedis.sql.functions.lit

import com.nec.frovedis.matrix.TimeSpent
import com.nec.frovedis.sql.FrovedisDataFrame
import com.nec.frovedis.sql.implicits_._

/**
 * TPC-H Query 15
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
class Q15 extends TpchQuery {

  override def execute(sc: SparkContext, schemaProvider: TpchSchemaProvider): DataFrame = {

    // this is used to implicitly convert an RDD to a DataFrame.
    val sqlContext = new org.apache.spark.sql.SQLContext(sc)
    import sqlContext.implicits._
    import schemaProvider._

/*
 * -------- ORIGINAL --------
 *
    val decrease = udf { (x: Double, y: Double) => x * (1 - y) }

    val revenue = lineitem.filter($"l_shipdate" >= "1996-01-01" &&
      $"l_shipdate" < "1996-04-01")
      .select($"l_suppkey", decrease($"l_extendedprice", $"l_discount").as("value"))
      .groupBy($"l_suppkey")
      .agg(sum($"value").as("total"))
    // .cache

    revenue.agg(max($"total").as("max_total"))
      .join(revenue, $"max_total" === revenue("total"))
      .join(supplier, $"l_suppkey" === supplier("s_suppkey"))
      .select($"s_suppkey", $"s_name", $"s_address", $"s_phone", $"total")
      .sort($"s_suppkey")
*/
    val t = new TimeSpent(Level.INFO)
    val flineitem  = new FrovedisDataFrame(lineitem, "l_shipdate", "l_suppkey", "l_extendedprice", "l_discount")
    val fsupplier = new FrovedisDataFrame(supplier, "s_suppkey", "s_name", "s_address", "s_phone")
    t.show("data transfer: ")

    val Cdecrease = $$"l_extendedprice" * (lit(1) - $$"l_discount") 

    val revenue = flineitem.filter($$"l_shipdate" >= "1996-01-01" &&
      $$"l_shipdate" < "1996-04-01")
      .select($$"l_suppkey", Cdecrease.as("value"))
      .groupBy($$"l_suppkey")
      .agg(sum($$"value").as("total"))
    // .cache

    val fret = revenue.agg(max($$"total").as("max_total"))
      .join(revenue, $$"max_total" === revenue("total"))
      .join(fsupplier, $$"l_suppkey" === fsupplier("s_suppkey"))
      .select($$"s_suppkey", $$"s_name", $$"s_address", $$"s_phone", $$"total")
      .sort($$"s_suppkey")
    t.show("query processing: ")

    val ret = fret.to_spark_DF()
    t.show("to_spark_DF: ")
    if (SHOW_OUT) ret.show()

    flineitem.release()
    fsupplier.release()
    return ret
  }
}
