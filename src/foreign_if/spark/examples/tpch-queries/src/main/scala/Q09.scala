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
 * TPC-H Query 9
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
class Q09 extends TpchQuery {

  override def execute(sc: SparkContext, schemaProvider: TpchSchemaProvider): DataFrame = {

    // this is used to implicitly convert an RDD to a DataFrame.
    val sqlContext = new org.apache.spark.sql.SQLContext(sc)
    import sqlContext.implicits._
    import schemaProvider._

/*
 * -------- ORIGINAL --------
 *
    val getYear = udf { (x: String) => x.substring(0, 4) }
    val expr = udf { (x: Double, y: Double, v: Double, w: Double) => x * (1 - y) - (v * w) }

    val linePart = part.filter($"p_name".contains("green"))
      .join(lineitem, $"p_partkey" === lineitem("l_partkey"))

    val natSup = nation.join(supplier, $"n_nationkey" === supplier("s_nationkey"))

    linePart.join(natSup, $"l_suppkey" === natSup("s_suppkey"))
      .join(partsupp, $"l_suppkey" === partsupp("ps_suppkey")
        && $"l_partkey" === partsupp("ps_partkey"))
      .join(order, $"l_orderkey" === order("o_orderkey"))
      .select($"n_name", getYear($"o_orderdate").as("o_year"),
        expr($"l_extendedprice", $"l_discount", $"ps_supplycost", $"l_quantity").as("amount"))
      .groupBy($"n_name", $"o_year")
      .agg(sum($"amount"))
      .sort($"n_name", $"o_year".desc)
*/

    val t = new TimeSpent(Level.INFO)
    val fpart = new FrovedisDataFrame(part, "p_name", "p_partkey")
    val flineitem = new FrovedisDataFrame(lineitem, "l_partkey", "l_suppkey", "l_orderkey", 
                                          "l_extendedprice", "l_discount", "l_quantity")
    val fnation = new FrovedisDataFrame(nation, "n_nationkey", "n_name")
    val fsupplier = new FrovedisDataFrame(supplier, "s_suppkey", "s_nationkey")
    val fpartsupp = new FrovedisDataFrame(partsupp, "ps_suppkey", "ps_partkey", "ps_supplycost")
    val fcustomer = new FrovedisDataFrame(customer, "c_nationkey", "c_custkey")
    val forder = new FrovedisDataFrame(order, "o_orderdate", "o_orderkey")
    t.show("data transfer: ")

    val CgetYear = $$"o_orderdate".substr(0, 4)
    val Cexpr = $$"l_extendedprice" * (lit(1) - $$"l_discount") - ($$"ps_supplycost" * $$"l_quantity")

    val linePart = fpart.filter($$"p_name".contains("green"))
      .join(flineitem, $$"p_partkey" === flineitem("l_partkey"))

    val natSup = fnation.join(fsupplier, $$"n_nationkey" === fsupplier("s_nationkey"))

    val fret = linePart.join(natSup, $$"l_suppkey" === natSup("s_suppkey"))
      .join(fpartsupp, $$"l_suppkey" === fpartsupp("ps_suppkey")
        && $$"l_partkey" === fpartsupp("ps_partkey"))
      .join(forder, $$"l_orderkey" === forder("o_orderkey"))
      .select($$"n_name", CgetYear.as("o_year"), Cexpr.as("amount"))
      .groupBy($$"n_name", $$"o_year")
      .agg(sum($$"amount"))
      .sort($$"n_name", $$"o_year".desc)
    t.show("query processing: ")

    val ret = fret.to_spark_DF()
    t.show("to_spark_DF: ")
    if (SHOW_OUT) ret.show()

    fpart.release()
    flineitem.release()
    fnation.release()
    fsupplier.release()
    fpartsupp.release()
    fcustomer.release()
    forder.release()
    return ret
  }
}
