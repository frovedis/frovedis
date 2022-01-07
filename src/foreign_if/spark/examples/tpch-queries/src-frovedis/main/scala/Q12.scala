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
 * TPC-H Query 12
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
class Q12 extends TpchQuery {

  override def execute(sc: SparkContext, schemaProvider: TpchSchemaProvider): DataFrame = {

    // this is used to implicitly convert an RDD to a DataFrame.
    val sqlContext = new org.apache.spark.sql.SQLContext(sc)
    import sqlContext.implicits._
    import schemaProvider._

/*
 * -------- ORIGINAL --------
 *
    val mul = udf { (x: Double, y: Double) => x * y } // *** UNUSED ***
    val highPriority = udf { (x: String) => if (x == "1-URGENT" || x == "2-HIGH") 1 else 0 }
    val lowPriority = udf { (x: String) => if (x != "1-URGENT" && x != "2-HIGH") 1 else 0 }

    lineitem.filter((
      $"l_shipmode" === "MAIL" || $"l_shipmode" === "SHIP") &&
      $"l_commitdate" < $"l_receiptdate" &&
      $"l_shipdate" < $"l_commitdate" &&
      $"l_receiptdate" >= "1994-01-01" && $"l_receiptdate" < "1995-01-01")
      .join(order, $"l_orderkey" === order("o_orderkey"))
      .select($"l_shipmode", $"o_orderpriority")
      .groupBy($"l_shipmode")
      .agg(sum(highPriority($"o_orderpriority")).as("sum_highorderpriority"),
        sum(lowPriority($"o_orderpriority")).as("sum_loworderpriority"))
      .sort($"l_shipmode")
*/

    val t = new TimeSpent(Level.INFO)
    val forder = new FrovedisDataFrame(order, "o_orderkey", "o_orderpriority")
    val flineitem  = new FrovedisDataFrame(lineitem, "l_shipmode", "l_shipdate", "l_commitdate", "l_receiptdate", "l_orderkey")
    t.show("data transfer: ")

    val ChighPriority = when(($$"o_orderpriority" === "1-URGENT") || ($$"o_orderpriority" === "2-HIGH"), lit(1)).otherwise(lit(0))
    val ClowPriority = when(($$"o_orderpriority" =!= "1-URGENT") && ($$"o_orderpriority" =!= "2-HIGH"), lit(1)).otherwise(lit(0))

    val fret = flineitem.filter((
      $$"l_shipmode" === "MAIL" || $$"l_shipmode" === "SHIP") &&
      $$"l_commitdate" < $$"l_receiptdate" &&
      $$"l_shipdate" < $$"l_commitdate" &&
      $$"l_receiptdate" >= "1994-01-01" && $$"l_receiptdate" < "1995-01-01")
      .join(forder, $$"l_orderkey" === forder("o_orderkey"))
      .select($$"l_shipmode", $$"o_orderpriority")
      .groupBy($$"l_shipmode")
      .agg(sum(ChighPriority).as("sum_highorderpriority"),
           sum(ClowPriority).as("sum_loworderpriority"))
      .sort($$"l_shipmode")
    t.show("query processing: ")

    val ret = fret.to_spark_DF()
    t.show("to_spark_DF: ")

    forder.release()
    flineitem.release()
    return ret
  }
}
