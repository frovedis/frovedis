package main.scala

import org.apache.log4j.{Level, Logger}
import org.apache.spark.sql.DataFrame
import org.apache.spark.SparkContext
//import org.apache.spark.sql.functions.count
//import org.apache.spark.sql.functions.udf
import com.nec.frovedis.sql.functions.count

import com.nec.frovedis.matrix.TimeSpent
import com.nec.frovedis.sql.FrovedisDataFrame
import com.nec.frovedis.sql.implicits_._

/**
 * TPC-H Query 13
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
class Q13 extends TpchQuery {

  override def execute(sc: SparkContext, schemaProvider: TpchSchemaProvider): DataFrame = {

    // this is used to implicitly convert an RDD to a DataFrame.
    val sqlContext = new org.apache.spark.sql.SQLContext(sc)
    import sqlContext.implicits._
    import schemaProvider._

/*
 * -------- ORIGINAL --------
 *
    val special = udf { (x: String) => x.matches(".*special.*requests.*") }

    customer.join(order, $"c_custkey" === order("o_custkey")
      && !special(order("o_comment")), "left_outer")
      .groupBy($"o_custkey")
      .agg(count($"o_orderkey").as("c_count"))
      .groupBy($"c_count")
      .agg(count($"o_custkey").as("custdist"))
      .sort($"custdist".desc, $"c_count".desc)
*/
    val t = new TimeSpent(Level.INFO)
    val fcustomer = new FrovedisDataFrame(customer, "c_custkey")
    val forder = new FrovedisDataFrame(order, "o_custkey", "o_comment", "o_orderkey")
    t.show("data transfer: ")

    val Cspecial = $$"o_comment".like("%special%requests%")

    val fret = fcustomer.join(forder, $$"c_custkey" === forder("o_custkey"), "left_outer")
      .filter(!Cspecial)
      .groupBy($$"o_custkey")
      .agg(count($$"o_orderkey").as("c_count"))
      .groupBy($$"c_count")
      .agg(count($$"o_custkey").as("custdist"))
      .sort($$"custdist".desc, $$"c_count".desc)
    t.show("query processing: ")

    val ret = fret.to_spark_DF()
    t.show("to_spark_DF: ")
    if (SHOW_OUT) ret.show()

    fcustomer.release()
    forder.release()
    return ret
  }
}
