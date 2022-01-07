package main.scala

import org.apache.log4j.{Level, Logger}
import org.apache.spark.sql.DataFrame
import org.apache.spark.SparkContext
//import org.apache.spark.sql.functions.count
import com.nec.frovedis.sql.functions.count

import com.nec.frovedis.matrix.TimeSpent
import com.nec.frovedis.sql.FrovedisDataFrame
import com.nec.frovedis.sql.implicits_._

/**
 * TPC-H Query 4
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
class Q04 extends TpchQuery {

  override def execute(sc: SparkContext, schemaProvider: TpchSchemaProvider): DataFrame = {

    // this is used to implicitly convert an RDD to a DataFrame.
    val sqlContext = new org.apache.spark.sql.SQLContext(sc)
    import sqlContext.implicits._
    import schemaProvider._

/*
 * -------- ORIGINAL --------
 *
    val forders = order.filter($"o_orderdate" >= "1993-07-01" && $"o_orderdate" < "1993-10-01")
    val flineitems = lineitem.filter($"l_commitdate" < $"l_receiptdate")
      .select($"l_orderkey")
      .distinct

    flineitems.join(forders, $"l_orderkey" === forders("o_orderkey"))
      .groupBy($"o_orderpriority")
      .agg(count($"o_orderpriority"))
      .sort($"o_orderpriority")
*/

    val t = new TimeSpent(Level.INFO)
    val forder = new FrovedisDataFrame(order, "o_orderdate", "o_orderpriority", "o_orderkey")
    val flineitem  = new FrovedisDataFrame(lineitem, "l_commitdate", "l_receiptdate", "l_orderkey")
    t.show("data transfer: ")

    val forders = forder.filter($$"o_orderdate" >= "1993-07-01" && $$"o_orderdate" < "1993-10-01")
    val flineitems = flineitem.filter($$"l_commitdate" < $$"l_receiptdate")
      .select($$"l_orderkey")
      .distinct

    val fret = flineitems.join(forders, $$"l_orderkey" === forders("o_orderkey"))
      .groupBy($$"o_orderpriority")
      .agg(count($$"o_orderpriority"))
      .sort($$"o_orderpriority")
    t.show("query processing: ")

    val ret = fret.to_spark_DF()
    t.show("to_spark_DF: ")

    forder.release()
    flineitem.release()
    return ret
  }
}
