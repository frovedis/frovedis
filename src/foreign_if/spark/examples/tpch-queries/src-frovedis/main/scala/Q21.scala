package main.scala

import org.apache.log4j.{Level, Logger}
import org.apache.spark.sql.DataFrame
import org.apache.spark.SparkContext
//import org.apache.spark.sql.functions.countDistinct
//import org.apache.spark.sql.functions.max
//import org.apache.spark.sql.functions.count
//import org.apache.spark.sql.functions.udf
import com.nec.frovedis.sql.functions.countDistinct
import com.nec.frovedis.sql.functions.max
import com.nec.frovedis.sql.functions.count

import com.nec.frovedis.matrix.TimeSpent
import com.nec.frovedis.sql.FrovedisDataFrame
import com.nec.frovedis.sql.implicits_._

/**
 * TPC-H Query 21
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
class Q21 extends TpchQuery {

  override def execute(sc: SparkContext, schemaProvider: TpchSchemaProvider): DataFrame = {

    // this is used to implicitly convert an RDD to a DataFrame.
    val sqlContext = new org.apache.spark.sql.SQLContext(sc)
    import sqlContext.implicits._
    import schemaProvider._

/*
 * -------- ORIGINAL --------
 *
    val fsupplier = supplier.select($"s_suppkey", $"s_nationkey", $"s_name")

    val plineitem = lineitem.select($"l_suppkey", $"l_orderkey", $"l_receiptdate", $"l_commitdate")
    //cache

    val flineitem = plineitem.filter($"l_receiptdate" > $"l_commitdate")
    // cache

    val line1 = plineitem.groupBy($"l_orderkey")
      .agg(countDistinct($"l_suppkey").as("suppkey_count"), max($"l_suppkey").as("suppkey_max"))
      .select($"l_orderkey".as("key"), $"suppkey_count", $"suppkey_max")

    val line2 = flineitem.groupBy($"l_orderkey")
      .agg(countDistinct($"l_suppkey").as("suppkey_count"), max($"l_suppkey").as("suppkey_max"))
      .select($"l_orderkey".as("key"), $"suppkey_count", $"suppkey_max")

    val forder = order.select($"o_orderkey", $"o_orderstatus")
      .filter($"o_orderstatus" === "F")

    nation.filter($"n_name" === "SAUDI ARABIA")
      .join(fsupplier, $"n_nationkey" === fsupplier("s_nationkey"))
      .join(flineitem, $"s_suppkey" === flineitem("l_suppkey"))
      .join(forder, $"l_orderkey" === forder("o_orderkey"))
      .join(line1, $"l_orderkey" === line1("key"))
      .filter($"suppkey_count" > 1 || ($"suppkey_count" == 1 && $"l_suppkey" == $"max_suppkey"))
      .select($"s_name", $"l_orderkey", $"l_suppkey")
      .join(line2, $"l_orderkey" === line2("key"), "left_outer")
      .select($"s_name", $"l_orderkey", $"l_suppkey", $"suppkey_count", $"suppkey_max")
      .filter($"suppkey_count" === 1 && $"l_suppkey" === $"suppkey_max")
      .groupBy($"s_name")
      .agg(count($"l_suppkey").as("numwait"))
      .sort($"numwait".desc, $"s_name")
      .limit(100)
*/

    val t = new TimeSpent(Level.INFO)
    val flineitem  = new FrovedisDataFrame(lineitem, "l_suppkey", "l_orderkey", "l_receiptdate", "l_commitdate")
    val fsupplier = new FrovedisDataFrame(supplier, "s_nationkey", "s_suppkey", "s_name")
    val forder =  new FrovedisDataFrame(order, "o_orderkey", "o_orderstatus")
    val fnation = new FrovedisDataFrame(nation, "n_nationkey", "n_name")
    t.show("data transfer: ")

    val flineitems = flineitem.filter($$"l_receiptdate" > $$"l_commitdate")
    // cache

    val line1 = flineitem.groupBy($$"l_orderkey")
      .agg(countDistinct($$"l_suppkey").as("suppkey_count"), max($$"l_suppkey").as("suppkey_max"))
      .select($$"l_orderkey".as("key"), $$"suppkey_count", $$"suppkey_max")

    val line2 = flineitems.groupBy($$"l_orderkey")
      .agg(countDistinct($$"l_suppkey").as("suppkey_count"), max($$"l_suppkey").as("suppkey_max"))
      .select($$"l_orderkey".as("key"), $$"suppkey_count", $$"suppkey_max")

    val forders = forder.filter($$"o_orderstatus" === "F")

    val fret = fnation.filter($$"n_name" === "SAUDI ARABIA")
      .join(fsupplier, $$"n_nationkey" === fsupplier("s_nationkey"))
      .join(flineitems, $$"s_suppkey" === flineitems("l_suppkey"))
      .join(forders, $$"l_orderkey" === forders("o_orderkey"))
      .join(line1, $$"l_orderkey" === line1("key"))
      .filter($$"suppkey_count" > 1 || ($$"suppkey_count" === 1 && $$"l_suppkey" === $$"suppkey_max"))
      .select($$"s_name", $$"l_orderkey", $$"l_suppkey")
      .join(line2, $$"l_orderkey" === line2("key"), "left_outer")
      .select($$"s_name", $$"l_orderkey", $$"l_suppkey", $$"suppkey_count", $$"suppkey_max")
      .filter($$"suppkey_count" === 1 && $$"l_suppkey" === $$"suppkey_max")
      .groupBy($$"s_name")
      .agg(count($$"l_suppkey").as("numwait"))
      .sort($$"numwait".desc, $$"s_name")
      .limit(100)
    t.show("query processing: ")

    val ret = fret.to_spark_DF()
    t.show("to_spark_DF: ")
    if (SHOW_OUT) ret.show()

    flineitem.release()
    fnation.release()
    fsupplier.release()
    forder.release()
    return ret
  }
}
