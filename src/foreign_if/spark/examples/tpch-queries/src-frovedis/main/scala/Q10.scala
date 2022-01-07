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
 * TPC-H Query 10
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
class Q10 extends TpchQuery {

  override def execute(sc: SparkContext, schemaProvider: TpchSchemaProvider): DataFrame = {

    // this is used to implicitly convert an RDD to a DataFrame.
    val sqlContext = new org.apache.spark.sql.SQLContext(sc)
    import sqlContext.implicits._
    import schemaProvider._

/*
 * -------- ORIGINAL --------
 *
    val decrease = udf { (x: Double, y: Double) => x * (1 - y) }

    val flineitem = lineitem.filter($"l_returnflag" === "R")

    order.filter($"o_orderdate" < "1994-01-01" && $"o_orderdate" >= "1993-10-01")
      .join(customer, $"o_custkey" === customer("c_custkey"))
      .join(nation, $"c_nationkey" === nation("n_nationkey"))
      .join(flineitem, $"o_orderkey" === flineitem("l_orderkey"))
      .select($"c_custkey", $"c_name",
        decrease($"l_extendedprice", $"l_discount").as("volume"),
        $"c_acctbal", $"n_name", $"c_address", $"c_phone", $"c_comment")
      .groupBy($"c_custkey", $"c_name", $"c_acctbal", $"c_phone", $"n_name", $"c_address", $"c_comment")
      .agg(sum($"volume").as("revenue"))
      .sort($"revenue".desc)
      .limit(20)
*/

    val t = new TimeSpent(Level.INFO)
    val flineitem = new FrovedisDataFrame(lineitem, "l_returnflag", "l_extendedprice", "l_discount", "l_orderkey")
    val forder = new FrovedisDataFrame(order, "o_orderdate", "o_orderkey", "o_custkey")
    val fcustomer = new FrovedisDataFrame(customer, "c_nationkey", "c_custkey", "c_acctbal", "c_address", "c_phone", "c_comment", "c_name")
    val fnation = new FrovedisDataFrame(nation, "n_nationkey", "n_name")
    t.show("data transfer: ")

    val Cdecrease = $$"l_extendedprice" * (lit(1) - $$"l_discount")
    val flineitems = flineitem.filter($$"l_returnflag" === "R")

    val fret = forder.filter($$"o_orderdate" < "1994-01-01" && $$"o_orderdate" >= "1993-10-01")
      .join(fcustomer, $$"o_custkey" === fcustomer("c_custkey"))
      .join(fnation, $$"c_nationkey" === fnation("n_nationkey"))
      .join(flineitems, $$"o_orderkey" === flineitems("l_orderkey"))
      .select($$"c_custkey", $$"c_name", Cdecrease.as("volume"),
        $$"c_acctbal", $$"n_name", $$"c_address", $$"c_phone", $$"c_comment")
      .groupBy($$"c_custkey", $$"c_name", $$"c_acctbal", $$"c_phone", $$"n_name", $$"c_address", $$"c_comment")
      .agg(sum($$"volume").as("revenue"))
      .sort($$"revenue".desc)
      .limit(20)
    t.show("query processing: ")

    val ret = fret.to_spark_DF()
    t.show("to_spark_DF: ")
    if (SHOW_OUT) ret.show()

    flineitem.release()
    forder.release()
    fcustomer.release()
    fnation.release()
    return ret
  }
}
