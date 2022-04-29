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
 * TPC-H Query 8
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
class Q08 extends TpchQuery {

  override def execute(sc: SparkContext, schemaProvider: TpchSchemaProvider): DataFrame = {

    // this is used to implicitly convert an RDD to a DataFrame.
    val sqlContext = new org.apache.spark.sql.SQLContext(sc)
    import sqlContext.implicits._
    import schemaProvider._

/*
 * -------- ORIGINAL --------
 *
    val getYear = udf { (x: String) => x.substring(0, 4) }
    val decrease = udf { (x: Double, y: Double) => x * (1 - y) }
    val isBrazil = udf { (x: String, y: Double) => if (x == "BRAZIL") y else 0 }

    val fregion = region.filter($"r_name" === "AMERICA")
    val forder = order.filter($"o_orderdate" <= "1996-12-31" && $"o_orderdate" >= "1995-01-01")
    val fpart = part.filter($"p_type" === "ECONOMY ANODIZED STEEL")

    val nat = nation.join(supplier, $"n_nationkey" === supplier("s_nationkey"))

    val line = lineitem.select($"l_partkey", $"l_suppkey", $"l_orderkey",
      decrease($"l_extendedprice", $"l_discount").as("volume")).
      join(fpart, $"l_partkey" === fpart("p_partkey"))
      .join(nat, $"l_suppkey" === nat("s_suppkey"))

    nation.join(fregion, $"n_regionkey" === fregion("r_regionkey"))
      .select($"n_nationkey")
      .join(customer, $"n_nationkey" === customer("c_nationkey"))
      .select($"c_custkey")
      .join(forder, $"c_custkey" === forder("o_custkey"))
      .select($"o_orderkey", $"o_orderdate")
      .join(line, $"o_orderkey" === line("l_orderkey"))
      .select(getYear($"o_orderdate").as("o_year"), $"volume",
        isBrazil($"n_name", $"volume").as("case_volume"))
      .groupBy($"o_year")
      .agg(sum($"case_volume") / sum("volume"))
      .sort($"o_year")
*/

    val t = new TimeSpent(Level.INFO)
    val flineitem = new FrovedisDataFrame(lineitem, "l_partkey", "l_suppkey", "l_orderkey", "l_extendedprice", "l_discount")
    val fnation = new FrovedisDataFrame(nation, "n_regionkey", "n_nationkey", "n_name")
    val fsupplier = new FrovedisDataFrame(supplier, "s_suppkey", "s_nationkey")
    val fcustomer = new FrovedisDataFrame(customer, "c_nationkey", "c_custkey")
    val forder = new FrovedisDataFrame(order, "o_orderdate", "o_custkey", "o_orderkey")
    val fregion = new FrovedisDataFrame(region, "r_name", "r_regionkey")
    val fpart = new FrovedisDataFrame(part, "p_type", "p_partkey")
    t.show("data transfer: ")

    val CgetYear = $$"o_orderdate".year
    val Cdecrease = $$"l_extendedprice" * (lit(1) - $$"l_discount")
    val CisBrazil = when($$"n_name" === "BRAZIL", $$"volume").otherwise(lit(0))

    val fregions = fregion.filter($$"r_name" === "AMERICA")
    val forders = forder.filter($$"o_orderdate" <= "1996-12-31" && $$"o_orderdate" >= "1995-01-01")
    val fparts = fpart.filter($$"p_type" === "ECONOMY ANODIZED STEEL")

    val nat = fnation.join(fsupplier, $$"n_nationkey" === fsupplier("s_nationkey"))

    val line = flineitem.select($$"l_partkey", $$"l_suppkey", $$"l_orderkey",
                                Cdecrease.as("volume"))
                        .join(fparts, $$"l_partkey" === fparts("p_partkey"))
                        .join(nat, $$"l_suppkey" === nat("s_suppkey"))

    val fret = fnation.join(fregions, $$"n_regionkey" === fregions("r_regionkey"))
      .select($$"n_nationkey")
      .join(fcustomer, $$"n_nationkey" === fcustomer("c_nationkey"))
      .select($$"c_custkey")
      .join(forders, $$"c_custkey" === forders("o_custkey"))
      .select($$"o_orderkey", $$"o_orderdate")
      .join(line, $$"o_orderkey" === line("l_orderkey"))
      .select(CgetYear.as("o_year"), $$"volume", CisBrazil.as("case_volume"))
      .groupBy($$"o_year")
      //.agg(sum($$"case_volume") / sum("volume")) // operation on aggregator is not supported
      .agg(sum($$"case_volume").as("x"), sum("volume").as("y"))
      .select($$"o_year", $$"x" / $$"y")
      .sort($$"o_year")
    t.show("query processing: ")

    val ret = fret.to_spark_DF()
    t.show("to_spark_DF: ")
    if (SHOW_OUT) ret.show()

    flineitem.release()
    fnation.release()
    fsupplier.release()
    fcustomer.release()
    forder.release()
    fregion.release()
    fpart.release()
    return ret
  }
}
