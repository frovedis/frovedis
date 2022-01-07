package main.scala

import org.apache.log4j.{Level, Logger}
import org.apache.spark.sql.DataFrame
import org.apache.spark.SparkContext
//import org.apache.spark.sql.functions.countDistinct
//import org.apache.spark.sql.functions.udf
import com.nec.frovedis.sql.functions.countDistinct

import com.nec.frovedis.matrix.TimeSpent
import com.nec.frovedis.sql.FrovedisDataFrame
import com.nec.frovedis.sql.implicits_._

/**
 * TPC-H Query 16
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
class Q16 extends TpchQuery {

  override def execute(sc: SparkContext, schemaProvider: TpchSchemaProvider): DataFrame = {

    // this is used to implicitly convert an RDD to a DataFrame.
    val sqlContext = new org.apache.spark.sql.SQLContext(sc)
    import sqlContext.implicits._
    import schemaProvider._

/*
 * -------- ORIGINAL --------
 *
    val decrease = udf { (x: Double, y: Double) => x * (1 - y) } // not used
    val complains = udf { (x: String) => x.matches(".*Customer.*Complaints.*") }
    val polished = udf { (x: String) => x.startsWith("MEDIUM POLISHED") }
    val numbers = udf { (x: Int) => x.toString().matches("49|14|23|45|19|3|36|9") }

    val fparts = part.filter(($"p_brand" !== "Brand#45") && !polished($"p_type") &&
      numbers($"p_size"))
      .select($"p_partkey", $"p_brand", $"p_type", $"p_size")

    supplier.filter(!complains($"s_comment"))
      // .select($"s_suppkey")
      .join(partsupp, $"s_suppkey" === partsupp("ps_suppkey"))
      .select($"ps_partkey", $"ps_suppkey")
      .join(fparts, $"ps_partkey" === fparts("p_partkey"))
      .groupBy($"p_brand", $"p_type", $"p_size")
      .agg(countDistinct($"ps_suppkey").as("supplier_count"))
      .sort($"supplier_count".desc, $"p_brand", $"p_type", $"p_size")
*/

    val t = new TimeSpent(Level.INFO)
    val fsupplier = new FrovedisDataFrame(supplier, "s_suppkey", "s_comment")
    val fpartsupp = new FrovedisDataFrame(partsupp, "ps_suppkey", "ps_partkey")
    val fpart = new FrovedisDataFrame(part, "p_brand", "p_size", "p_type", "p_partkey")
    t.show("data transfer: ")

    val Ccomplains = $$"s_comment".contains("%Customer%Complaints%") 
    val Cpolished = $$"p_type".startsWith("MEDIUM POLISHED")
    val Cnumbers = ($$"p_size" === 49) || ($$"p_size" === 14) || 
                   ($$"p_size" === 23) || ($$"p_size" === 45) ||
                   ($$"p_size" === 19) || ($$"p_size" === 3)  ||
                   ($$"p_size" === 36) || ($$"p_size" === 9)

    val fparts = fpart.filter(
      ($$"p_brand" !== "Brand#45") && !Cpolished && Cnumbers)
      .select($$"p_partkey", $$"p_brand", $$"p_type", $$"p_size")

    val fret = fsupplier.filter(!Ccomplains)
      // .select($$"s_suppkey")
      .join(fpartsupp, $$"s_suppkey" === fpartsupp("ps_suppkey"))
      .select($$"ps_partkey", $$"ps_suppkey")
      .join(fparts, $$"ps_partkey" === fparts("p_partkey"))
      .groupBy($$"p_brand", $$"p_type", $$"p_size")
      .agg(countDistinct($$"ps_suppkey").as("supplier_count"))
      .sort($$"supplier_count".desc, $$"p_brand", $$"p_type", $$"p_size")
    t.show("query processing: ")

    val ret = fret.to_spark_DF()
    t.show("to_spark_DF: ")

    fsupplier.release()
    fpartsupp.release()
    fpart.release()
    return ret
  }
}
