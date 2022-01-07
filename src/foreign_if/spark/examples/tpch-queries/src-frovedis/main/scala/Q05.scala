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
 * TPC-H Query 5
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
class Q05 extends TpchQuery {

  override def execute(sc: SparkContext, schemaProvider: TpchSchemaProvider): DataFrame = {

    // this is used to implicitly convert an RDD to a DataFrame.
    val sqlContext = new org.apache.spark.sql.SQLContext(sc)
    import sqlContext.implicits._
    import schemaProvider._

/* 
 * -------- ORIGINAL --------
 *
    val decrease = udf { (x: Double, y: Double) => x * (1 - y) }

    val forders = order.filter($"o_orderdate" < "1995-01-01" && $"o_orderdate" >= "1994-01-01")

    region.filter($"r_name" === "ASIA")
      .join(nation, $"r_regionkey" === nation("n_regionkey"))
      .join(supplier, $"n_nationkey" === supplier("s_nationkey"))
      .join(lineitem, $"s_suppkey" === lineitem("l_suppkey"))
      .select($"n_name", $"l_extendedprice", $"l_discount", $"l_orderkey", $"s_nationkey")
      .join(forders, $"l_orderkey" === forders("o_orderkey"))
      .join(customer, $"o_custkey" === customer("c_custkey") && $"s_nationkey" === customer("c_nationkey"))
      .select($"n_name", decrease($"l_extendedprice", $"l_discount").as("value"))
      .groupBy($"n_name")
      .agg(sum($"value").as("revenue"))
      .sort($"revenue".desc)
*/

    val t = new TimeSpent(Level.INFO)
    val forder = new FrovedisDataFrame(order, "o_orderdate", "o_custkey", "o_orderkey")
    val fregion = new FrovedisDataFrame(region, "r_name", "r_regionkey")
    val fnation = new FrovedisDataFrame(nation, "n_regionkey", "n_nationkey", "n_name")
    val fsupplier = new FrovedisDataFrame(supplier, "s_nationkey", "s_suppkey")
    val flineitem  = new FrovedisDataFrame(lineitem, "l_suppkey", "l_extendedprice", "l_discount", "l_orderkey")
    val fcustomer = new FrovedisDataFrame(customer, "c_custkey", "c_nationkey")
    t.show("data transfer: ")

    val Cdecrease = $$"l_extendedprice" * (lit(1) - $$"l_discount") 

    val forders = forder.filter($$"o_orderdate" < "1995-01-01" && $$"o_orderdate" >= "1994-01-01")

    val fret = fregion.filter($$"r_name" === "ASIA")
      .join(fnation, $$"r_regionkey" === fnation("n_regionkey"))
      .join(fsupplier, $$"n_nationkey" === fsupplier("s_nationkey"))
      .join(flineitem, $$"s_suppkey" === flineitem("l_suppkey"))
      .select($$"n_name", $$"l_extendedprice", $$"l_discount", $$"l_orderkey", $$"s_nationkey")
      .join(forders, $$"l_orderkey" === forders("o_orderkey"))
      .join(fcustomer, $$"o_custkey" === fcustomer("c_custkey") && $$"s_nationkey" === fcustomer("c_nationkey"))
      .select($$"n_name", Cdecrease.as("value"))
      .groupBy($$"n_name")
      .agg(sum($$"value").as("revenue"))
      .sort($$"revenue".desc)
    t.show("query processing: ")

    val ret = fret.to_spark_DF()
    t.show("to_spark_DF: ")

    forder.release()
    fregion.release()
    fnation.release()
    fsupplier.release()
    flineitem.release()
    fcustomer.release()
    return ret
  }
}
