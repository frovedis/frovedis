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
 * TPC-H Query 11
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
class Q11 extends TpchQuery {

  override def execute(sc: SparkContext, schemaProvider: TpchSchemaProvider): DataFrame = {

    // this is used to implicitly convert an RDD to a DataFrame.
    val sqlContext = new org.apache.spark.sql.SQLContext(sc)
    import sqlContext.implicits._
    import schemaProvider._

/*
 * -------- ORIGINAL --------
 *
    val mul = udf { (x: Double, y: Int) => x * y }
    val mul01 = udf { (x: Double) => x * 0.0001 }

    val tmp = nation.filter($"n_name" === "GERMANY")
      .join(supplier, $"n_nationkey" === supplier("s_nationkey"))
      .select($"s_suppkey")
      .join(partsupp, $"s_suppkey" === partsupp("ps_suppkey"))
      .select($"ps_partkey", mul($"ps_supplycost", $"ps_availqty").as("value"))
    // .cache()

    val sumRes = tmp.agg(sum("value").as("total_value"))

    tmp.groupBy($"ps_partkey").agg(sum("value").as("part_value"))
      .join(sumRes, $"part_value" > mul01($"total_value"))
      .sort($"part_value".desc)
*/

    val t = new TimeSpent(Level.INFO)
    val fnation = new FrovedisDataFrame(nation, "n_nationkey", "n_name")
    val fsupplier = new FrovedisDataFrame(supplier, "s_nationkey", "s_suppkey")
    val fpartsupp = new FrovedisDataFrame(partsupp, "ps_suppkey", "ps_partkey", "ps_supplycost", "ps_availqty")
    t.show("data transfer: ")

    val Cmul = $$"ps_supplycost" * $$"ps_availqty"
    val Cmul01 = $$"total_value" * lit(0.0001)

    val tmp = fnation.filter($$"n_name" === "GERMANY")
      .join(fsupplier, $$"n_nationkey" === fsupplier("s_nationkey"))
      .select($$"s_suppkey")
      .join(fpartsupp, $$"s_suppkey" === fpartsupp("ps_suppkey"))
      .select($$"ps_partkey", Cmul.as("value"))
    // .cache()

    val sumRes = tmp.agg(sum("value").as("total_value"))

    val fret = tmp.groupBy($$"ps_partkey").agg(sum("value").as("part_value"))
      .join(sumRes, $$"part_value" > Cmul01)
      .sort($$"part_value".desc)
    t.show("query processing: ")

    val ret = fret.to_spark_DF()
    t.show("to_spark_DF: ")
    if (SHOW_OUT) ret.show()

    fnation.release()
    fsupplier.release()
    fpartsupp.release()
    return ret
  }
}
