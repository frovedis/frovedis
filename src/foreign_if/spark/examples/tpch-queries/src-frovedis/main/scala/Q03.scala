package main.scala

import org.apache.log4j.{Level, Logger}
import org.apache.spark.sql.DataFrame
import org.apache.spark.SparkContext
//import org.apache.spark.sql.functions.sum
//import org.apache.spark.sql.functions.udf
import com.nec.frovedis.sql.functions.sum
import com.nec.frovedis.sql.functions.lit

import com.nec.frovedis.matrix.TimeSpent
import com.nec.frovedis.sql.FrovedisDataFrame
import com.nec.frovedis.sql.implicits_._

/**
 * TPC-H Query 3
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
class Q03 extends TpchQuery {

  override def execute(sc: SparkContext, schemaProvider: TpchSchemaProvider): DataFrame = {

    // this is used to implicitly convert an RDD to a DataFrame.
    val sqlContext = new org.apache.spark.sql.SQLContext(sc)
    import sqlContext.implicits._
    import schemaProvider._

/*
 * -------- ORIGINAL --------
 *
    val decrease = udf { (x: Double, y: Double) => x * (1 - y) }

    val fcust = customer.filter($"c_mktsegment" === "BUILDING")
    val forders = order.filter($"o_orderdate" < "1995-03-15")
    val flineitems = lineitem.filter($"l_shipdate" > "1995-03-15")

    fcust.join(forders, $"c_custkey" === forders("o_custkey"))
      .select($"o_orderkey", $"o_orderdate", $"o_shippriority")
      .join(flineitems, $"o_orderkey" === flineitems("l_orderkey"))
      .select($"l_orderkey",
        decrease($"l_extendedprice", $"l_discount").as("volume"),
        $"o_orderdate", $"o_shippriority")
      .groupBy($"l_orderkey", $"o_orderdate", $"o_shippriority")
      .agg(sum($"volume").as("revenue"))
      .sort($"revenue".desc, $"o_orderdate")
      .limit(10)
*/

    val t = new TimeSpent(Level.INFO)
    val fcustomer = new FrovedisDataFrame(customer, "c_mktsegment", "c_custkey")
    val forder = new FrovedisDataFrame(order, "o_orderdate", "o_custkey", "o_shippriority", "o_orderkey")
    val flineitem  = new FrovedisDataFrame(lineitem, "l_shipdate", "l_orderkey", "l_extendedprice", "l_discount")
    t.show("data transfer: ")

    val Cdecrease = $$"l_extendedprice" * (lit(1) - $$"l_discount")

    val fcust = fcustomer.filter($$"c_mktsegment" === "BUILDING")
    val forders = forder.filter($$"o_orderdate" < "1995-03-15")
    val flineitems = flineitem.filter($$"l_shipdate" > "1995-03-15")

    val fret = fcust.join(forders, $$"c_custkey" === forders("o_custkey"))
      .select($$"o_orderkey", $$"o_orderdate", $$"o_shippriority")
      .join(flineitems, $$"o_orderkey" === flineitems("l_orderkey"))
      .select($$"l_orderkey",
        Cdecrease.as("volume"),
        $$"o_orderdate", $$"o_shippriority")
      .groupBy($$"l_orderkey", $$"o_orderdate", $$"o_shippriority")
      .agg(sum($$"volume").as("revenue"))
      .sort($$"revenue".desc, $$"o_orderdate")
      .limit(10)
    t.show("query processing: ")

    val ret = fret.to_spark_DF()
    t.show("to_spark_DF: ")

    fcustomer.release()
    forder.release()
    flineitem.release()
    return ret
  }
}
