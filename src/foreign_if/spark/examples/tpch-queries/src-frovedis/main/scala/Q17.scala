package main.scala

import org.apache.log4j.{Level, Logger}
import org.apache.spark.sql.DataFrame
import org.apache.spark.SparkContext
//import org.apache.spark.sql.functions.avg
//import org.apache.spark.sql.functions.sum
//import org.apache.spark.sql.functions.udf
import com.nec.frovedis.sql.functions.avg
import com.nec.frovedis.sql.functions.sum
import com.nec.frovedis.sql.functions.lit

import com.nec.frovedis.matrix.TimeSpent
import com.nec.frovedis.sql.FrovedisDataFrame
import com.nec.frovedis.sql.implicits_._

/**
 * TPC-H Query 17
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
class Q17 extends TpchQuery {

  override def execute(sc: SparkContext, schemaProvider: TpchSchemaProvider): DataFrame = {

    // this is used to implicitly convert an RDD to a DataFrame.
    val sqlContext = new org.apache.spark.sql.SQLContext(sc)
    import sqlContext.implicits._
    import schemaProvider._

/*
 * -------- ORIGINAL --------
 *
    val mul02 = udf { (x: Double) => x * 0.2 }

    val flineitem = lineitem.select($"l_partkey", $"l_quantity", $"l_extendedprice")

    val fpart = part.filter($"p_brand" === "Brand#23" && $"p_container" === "MED BOX")
      .select($"p_partkey")
      .join(lineitem, $"p_partkey" === lineitem("l_partkey"), "left_outer")
    // select

    fpart.groupBy("p_partkey")
      .agg(mul02(avg($"l_quantity")).as("avg_quantity"))
      .select($"p_partkey".as("key"), $"avg_quantity")
      .join(fpart, $"key" === fpart("p_partkey"))
      .filter($"l_quantity" < $"avg_quantity")
      .agg(sum($"l_extendedprice") / 7.0)
*/

    val t = new TimeSpent(Level.INFO)
    val flineitem = new FrovedisDataFrame(lineitem, "l_partkey", "l_quantity", "l_extendedprice")
    val fpart = new FrovedisDataFrame(part, "p_brand", "p_container", "p_partkey")
    t.show("data transfer: ")

    val Cmul02 = $$"x" * lit(0.2)

    val fparts = fpart.filter($$"p_brand" === "Brand#23" && $$"p_container" === "MED BOX")
      .select($$"p_partkey")
      .join(flineitem, $$"p_partkey" === flineitem("l_partkey"), "left_outer")

    val fret = fparts.groupBy("p_partkey")
      .agg(avg($$"l_quantity").as("x"))
      .select($$"p_partkey".as("key"), Cmul02.as("avg_quantity"))
      .join(fparts, $$"key" === fparts("p_partkey"))
      .filter($$"l_quantity" < $$"avg_quantity")
      //.agg(sum($$"l_extendedprice") / 7.0) // operation on aggregator is not supported
      .agg(sum($$"l_extendedprice").as("y"))
      .select($$"y" / 7.0)
    t.show("query processing: ")

    val ret = fret.to_spark_DF()
    t.show("to_spark_DF: ")
    if (SHOW_OUT) ret.show()

    flineitem.release()
    fpart.release()
    return ret
  }
}
