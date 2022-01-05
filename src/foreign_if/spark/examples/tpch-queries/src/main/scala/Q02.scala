package main.scala

import org.apache.log4j.{Level, Logger}
import org.apache.spark.sql.DataFrame
import org.apache.spark.SparkContext

//import org.apache.spark.sql.functions.min
import com.nec.frovedis.sql.functions.min

import com.nec.frovedis.matrix.TimeSpent
import com.nec.frovedis.sql.FrovedisDataFrame
import com.nec.frovedis.sql.implicits_._

/**
 * TPC-H Query 2
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
class Q02 extends TpchQuery {

  override def execute(sc: SparkContext, schemaProvider: TpchSchemaProvider): DataFrame = {

    // this is used to implicitly convert an RDD to a DataFrame.
    val sqlContext = new org.apache.spark.sql.SQLContext(sc)
    import sqlContext.implicits._
    import schemaProvider._

/*
 * -------- ORIGINAL --------
 *
    val europe = region.filter($"r_name" === "EUROPE")
      .join(nation, $"r_regionkey" === nation("n_regionkey"))
      .join(supplier, $"n_nationkey" === supplier("s_nationkey"))
      .join(partsupp, supplier("s_suppkey") === partsupp("ps_suppkey"))
    //.select($"ps_partkey", $"ps_supplycost", $"s_acctbal", $"s_name", $"n_name", $"s_address", $"s_phone", $"s_comment")

    val brass = part.filter(part("p_size") === 15 && part("p_type").endsWith("BRASS"))
      .join(europe, europe("ps_partkey") === $"p_partkey")
    //.cache

    val minCost = brass.groupBy(brass("ps_partkey"))
      .agg(min("ps_supplycost").as("min"))

    brass.join(minCost, brass("ps_partkey") === minCost("ps_partkey"))
      .filter(brass("ps_supplycost") === minCost("min"))
      .select("s_acctbal", "s_name", "n_name", "p_partkey", "p_mfgr", "s_address", "s_phone", "s_comment")
      .sort($"s_acctbal".desc, $"n_name", $"s_name", $"p_partkey")
      .limit(100)
*/

    val t = new TimeSpent(Level.INFO)
    val fregion = new FrovedisDataFrame(region, "r_name", "r_regionkey")
    val fnation = new FrovedisDataFrame(nation, "n_regionkey", "n_nationkey", "n_name")
    val fsupplier = new FrovedisDataFrame(supplier, "s_nationkey", "s_suppkey", "s_acctbal", "s_name", "s_address", "s_phone", "s_comment")
    val fpartsupp = new FrovedisDataFrame(partsupp, "ps_suppkey", "ps_partkey", "ps_supplycost")
    val fpart = new FrovedisDataFrame(part, "p_size", "p_type", "p_partkey", "p_mfgr")
    t.show("data transfer: ")

    val europe = fregion.filter($$"r_name" === "EUROPE")
      .join(fnation, $$"r_regionkey" === fnation("n_regionkey"))
      .join(fsupplier, $$"n_nationkey" === fsupplier("s_nationkey"))
      .join(fpartsupp, fsupplier("s_suppkey") === fpartsupp("ps_suppkey"))
    //.select($"ps_partkey", $"ps_supplycost", $"s_acctbal", $"s_name", $"n_name", $"s_address", $"s_phone", $"s_comment")

    val brass = fpart.filter(fpart("p_size") === 15 && fpart("p_type").endsWith("BRASS"))
      .join(europe, europe("ps_partkey") === $$"p_partkey")
    //.cache

    val minCost = brass.groupBy(brass("ps_partkey"))
      .agg(min("ps_supplycost").as("min"))

    val fret = brass.join(minCost, brass("ps_partkey") === minCost("ps_partkey"))
      .filter(brass("ps_supplycost") === minCost("min"))
      .select("s_acctbal", "s_name", "n_name", "p_partkey", "p_mfgr", "s_address", "s_phone", "s_comment")
      .sort($$"s_acctbal".desc, $$"n_name", $$"s_name", $$"p_partkey")
      .limit(100)
    t.show("query processing: ")

    val ret = fret.to_spark_DF()
    t.show("to_spark_DF: ")

    fregion.release()
    fnation.release()
    fsupplier.release()
    fpartsupp.release()
    fpart.release()
    return ret
  }
}
