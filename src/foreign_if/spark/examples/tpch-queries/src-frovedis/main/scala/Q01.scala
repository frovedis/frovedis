package main.scala

import org.apache.log4j.{Level, Logger}
import org.apache.spark.SparkContext
import org.apache.spark.sql.DataFrame
/*
import org.apache.spark.sql.functions.count
import org.apache.spark.sql.functions.sum
import org.apache.spark.sql.functions.avg
import org.apache.spark.sql.functions.udf
*/
import com.nec.frovedis.sql.functions.count
import com.nec.frovedis.sql.functions.sum
import com.nec.frovedis.sql.functions.avg
import com.nec.frovedis.sql.functions.lit

import com.nec.frovedis.matrix.TimeSpent
import com.nec.frovedis.sql.FrovedisDataFrame
import com.nec.frovedis.sql.implicits_._

/**
 * TPC-H Query 1
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
class Q01 extends TpchQuery {

  override def execute(sc: SparkContext, schemaProvider: TpchSchemaProvider): DataFrame = {

    // this is used to implicitly convert an RDD to a DataFrame.
    val sqlContext = new org.apache.spark.sql.SQLContext(sc)
    import sqlContext.implicits._
    import schemaProvider._

/*
 * -------- ORIGINAL --------
 *
    val decrease = udf { (x: Double, y: Double) => x * (1 - y) }
    val increase = udf { (x: Double, y: Double) => x * (1 + y) }

    lineitem.filter($"l_shipdate" <= "1998-09-02")
      .groupBy($"l_returnflag", $"l_linestatus")
      .agg(sum($"l_quantity"), sum($"l_extendedprice"),
        sum(decrease($"l_extendedprice", $"l_discount")),
        sum(increase(decrease($"l_extendedprice", $"l_discount"), $"l_tax")),
        avg($"l_quantity"), avg($"l_extendedprice"), avg($"l_discount"), count($"l_quantity"))
      .sort($"l_returnflag", $"l_linestatus")
*/

    val t = new TimeSpent(Level.INFO)
    val flineitem = new FrovedisDataFrame(lineitem, "l_shipdate", "l_returnflag", 
                                          "l_linestatus", "l_discount", "l_quantity", "l_extendedprice", "l_tax")
    t.show("data transfer: ")

    val Cdecrease = $$"l_extendedprice" * (lit(1) - $$"l_discount")
    val Cincrease = Cdecrease * (lit(1) + $$"l_tax")

    val fret = flineitem.filter($$"l_shipdate" <= "1998-09-02")
      .groupBy($$"l_returnflag", $$"l_linestatus")
      .agg(sum($$"l_quantity"), sum($$"l_extendedprice"),
        sum(Cdecrease), sum(Cincrease),
        avg($$"l_quantity"), avg($$"l_extendedprice"), avg($$"l_discount"), count($$"l_quantity"))
      .sort($$"l_returnflag", $$"l_linestatus")
    t.show("query processing: ")

    val ret = fret.to_spark_DF()
    t.show("to_spark_DF: ")
    if (SHOW_OUT) ret.show()

    flineitem.release()
    return ret
  }
}
