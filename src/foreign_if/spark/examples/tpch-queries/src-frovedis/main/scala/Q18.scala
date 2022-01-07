package main.scala

import org.apache.log4j.{Level, Logger}
import org.apache.spark.sql.DataFrame
import org.apache.spark.SparkContext
//import org.apache.spark.sql.functions.first
//import org.apache.spark.sql.functions.sum
//import org.apache.spark.sql.functions.udf
import com.nec.frovedis.sql.functions.first
import com.nec.frovedis.sql.functions.sum
import com.nec.frovedis.sql.functions.lit

import com.nec.frovedis.matrix.TimeSpent
import com.nec.frovedis.sql.FrovedisDataFrame
import com.nec.frovedis.sql.implicits_._

/**
 * TPC-H Query 18
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
class Q18 extends TpchQuery {

  override def execute(sc: SparkContext, schemaProvider: TpchSchemaProvider): DataFrame = {

    // this is used to implicitly convert an RDD to a DataFrame.
    val sqlContext = new org.apache.spark.sql.SQLContext(sc)
    import sqlContext.implicits._
    import schemaProvider._

/*
 * -------- ORIGINAL --------
 *
    lineitem.groupBy($"l_orderkey")
      .agg(sum($"l_quantity").as("sum_quantity"))
      .filter($"sum_quantity" > 300)
      .select($"l_orderkey".as("key"), $"sum_quantity")
      .join(order, order("o_orderkey") === $"key")
      .join(lineitem, $"o_orderkey" === lineitem("l_orderkey"))
      .join(customer, customer("c_custkey") === $"o_custkey")
      .select($"l_quantity", $"c_name", $"c_custkey", $"o_orderkey", $"o_orderdate", $"o_totalprice")
      .groupBy($"c_name", $"c_custkey", $"o_orderkey", $"o_orderdate", $"o_totalprice")
      .agg(sum("l_quantity"))
      .sort($"o_totalprice".desc, $"o_orderdate")
      .limit(100)
*/

    val t = new TimeSpent(Level.INFO)
    val flineitem = new FrovedisDataFrame(lineitem, "l_orderkey", "l_quantity")
    val fcustomer = new FrovedisDataFrame(customer, "c_name", "c_custkey")
    val forder = new FrovedisDataFrame(order, "o_orderdate", "o_custkey", "o_totalprice", "o_orderkey")
    t.show("data transfer: ")

    val fret = flineitem.groupBy($$"l_orderkey")
      .agg(sum($$"l_quantity").as("sum_quantity"))
      .filter($$"sum_quantity" > 300)
      .select($$"l_orderkey".as("key"), $$"sum_quantity")
      .join(forder, forder("o_orderkey") === $$"key")
      .join(flineitem, $$"o_orderkey" === flineitem("l_orderkey"))
      .join(fcustomer, fcustomer("c_custkey") === $$"o_custkey")
      .select($$"l_quantity", $$"c_name", $$"c_custkey", $$"o_orderkey", $$"o_orderdate", $$"o_totalprice")
      .groupBy($$"c_name", $$"c_custkey", $$"o_orderkey", $$"o_orderdate", $$"o_totalprice")
      .agg(sum("l_quantity"))
      .sort($$"o_totalprice".desc, $$"o_orderdate")
      .limit(100)
    t.show("query processing: ")

    val ret = fret.to_spark_DF()
    t.show("to_spark_DF: ")

    flineitem.release()
    fcustomer.release()
    forder.release()
    return ret
  }
}
