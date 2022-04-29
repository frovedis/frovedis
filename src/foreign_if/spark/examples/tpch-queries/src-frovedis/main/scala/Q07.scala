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
 * TPC-H Query 7
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
class Q07 extends TpchQuery {

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

    // cache fnation

    val fnation = nation.filter($"n_name" === "FRANCE" || $"n_name" === "GERMANY")
    val fline = lineitem.filter($"l_shipdate" >= "1995-01-01" && $"l_shipdate" <= "1996-12-31")

    val supNation = fnation.join(supplier, $"n_nationkey" === supplier("s_nationkey"))
      .join(fline, $"s_suppkey" === fline("l_suppkey"))
      .select($"n_name".as("supp_nation"), $"l_orderkey", $"l_extendedprice", $"l_discount", $"l_shipdate")

    fnation.join(customer, $"n_nationkey" === customer("c_nationkey"))
      .join(order, $"c_custkey" === order("o_custkey"))
      .select($"n_name".as("cust_nation"), $"o_orderkey")
      .join(supNation, $"o_orderkey" === supNation("l_orderkey"))
      .filter($"supp_nation" === "FRANCE" && $"cust_nation" === "GERMANY"
        || $"supp_nation" === "GERMANY" && $"cust_nation" === "FRANCE")
      .select($"supp_nation", $"cust_nation",
        getYear($"l_shipdate").as("l_year"),
        decrease($"l_extendedprice", $"l_discount").as("volume"))
      .groupBy($"supp_nation", $"cust_nation", $"l_year")
      .agg(sum($"volume").as("revenue"))
      .sort($"supp_nation", $"cust_nation", $"l_year")
*/

    val t = new TimeSpent(Level.INFO)
    val flineitem = new FrovedisDataFrame(lineitem, "l_shipdate", "l_suppkey", "l_orderkey",
                                          "l_discount", "l_extendedprice")
    val fnation = new FrovedisDataFrame(nation, "n_nationkey", "n_name")
    val fsupplier = new FrovedisDataFrame(supplier, "s_suppkey", "s_nationkey")
    val fcustomer = new FrovedisDataFrame(customer, "c_nationkey", "c_custkey")
    val forder = new FrovedisDataFrame(order, "o_custkey", "o_orderkey")
    t.show("data transfer: ")

    val CgetYear = $$"l_shipdate".year
    val Cdecrease = $$"l_extendedprice" * (lit(1) - $$"l_discount")

    val fnations = fnation.filter($$"n_name" === "FRANCE" || $$"n_name" === "GERMANY")
    val flines = flineitem.filter($$"l_shipdate" >= "1995-01-01" && $$"l_shipdate" <= "1996-12-31")

    val supNation = fnations.join(fsupplier, $$"n_nationkey" === fsupplier("s_nationkey"))
      .join(flines, $$"s_suppkey" === flines("l_suppkey"))
      .select($$"n_name".as("supp_nation"), $$"l_orderkey", $$"l_extendedprice", $$"l_discount", $$"l_shipdate")

    val fret = fnations.join(fcustomer, $$"n_nationkey" === fcustomer("c_nationkey"))
      .join(forder, $$"c_custkey" === forder("o_custkey"))
      .select($$"n_name".as("cust_nation"), $$"o_orderkey")
      .join(supNation, $$"o_orderkey" === supNation("l_orderkey"))
      .filter($$"supp_nation" === "FRANCE" && $$"cust_nation" === "GERMANY"
        || $$"supp_nation" === "GERMANY" && $$"cust_nation" === "FRANCE")
      .select($$"supp_nation", $$"cust_nation",
        CgetYear.as("l_year"),
        Cdecrease.as("volume"))
      .groupBy($$"supp_nation", $$"cust_nation", $$"l_year")
      .agg(sum($$"volume").as("revenue"))
      .sort($$"supp_nation", $$"cust_nation", $$"l_year")
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
