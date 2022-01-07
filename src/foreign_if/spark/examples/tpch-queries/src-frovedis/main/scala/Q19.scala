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
 * TPC-H Query 19
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
class Q19 extends TpchQuery {

  override def execute(sc: SparkContext, schemaProvider: TpchSchemaProvider): DataFrame = {

    // this is used to implicitly convert an RDD to a DataFrame.
    val sqlContext = new org.apache.spark.sql.SQLContext(sc)
    import sqlContext.implicits._
    import schemaProvider._

/*
 * -------- ORIGINAL --------
 *
    val sm = udf { (x: String) => x.matches("SM CASE|SM BOX|SM PACK|SM PKG") }
    val md = udf { (x: String) => x.matches("MED BAG|MED BOX|MED PKG|MED PACK") }
    val lg = udf { (x: String) => x.matches("LG CASE|LG BOX|LG PACK|LG PKG") }

    val decrease = udf { (x: Double, y: Double) => x * (1 - y) }

    // project part and lineitem first?
    part.join(lineitem, $"l_partkey" === $"p_partkey")
      .filter(($"l_shipmode" === "AIR" || $"l_shipmode" === "AIR REG") &&
        $"l_shipinstruct" === "DELIVER IN PERSON")
      .filter(
        (($"p_brand" === "Brand#12") &&
          sm($"p_container") &&
          $"l_quantity" >= 1 && $"l_quantity" <= 11 &&
          $"p_size" >= 1 && $"p_size" <= 5) ||
          (($"p_brand" === "Brand#23") &&
            md($"p_container") &&
            $"l_quantity" >= 10 && $"l_quantity" <= 20 &&
            $"p_size" >= 1 && $"p_size" <= 10) ||
            (($"p_brand" === "Brand#34") &&
              lg($"p_container") &&
              $"l_quantity" >= 20 && $"l_quantity" <= 30 &&
              $"p_size" >= 1 && $"p_size" <= 15))
      .select(decrease($"l_extendedprice", $"l_discount").as("volume"))
      .agg(sum("volume"))
*/

    val t = new TimeSpent(Level.INFO)
    val flineitem = new FrovedisDataFrame(lineitem, "l_partkey", "l_shipmode", 
                                          "l_shipinstruct", "l_quantity", 
                                          "l_extendedprice", "l_discount")
    val fpart = new FrovedisDataFrame(part, "p_size", "p_brand", "p_container", "p_partkey")
    t.show("data transfer: ")

    val Csm = ($$"p_container" === "SM CASE") || ($$"p_container" === "SM BOX") ||
              ($$"p_container" === "SM PACK") || ($$"p_container" === "SM PKG")
    val Cmd = ($$"p_container" === "MED BAG") || ($$"p_container" === "MED BOX") ||
              ($$"p_container" === "MED PKG") || ($$"p_container" === "MED PACK")
    val Clg = ($$"p_container" === "LG CASE") || ($$"p_container" === "LG BOX") ||
              ($$"p_container" === "LG PACK") || ($$"p_container" === "LG PKG")

    val Cdecrease = $$"l_extendedprice" * (lit(1) - $$"l_discount")

    val fret = fpart.join(flineitem, $$"l_partkey" === $$"p_partkey")
      .filter(($$"l_shipmode" === "AIR" || $$"l_shipmode" === "AIR REG") &&
        $$"l_shipinstruct" === "DELIVER IN PERSON")
      .filter(
        (($$"p_brand" === "Brand#12") &&
          Csm && $$"l_quantity" >= 1 && $$"l_quantity" <= 11 &&
          $$"p_size" >= 1 && $$"p_size" <= 5) ||
          (($$"p_brand" === "Brand#23") &&
            Cmd && $$"l_quantity" >= 10 && $$"l_quantity" <= 20 &&
            $$"p_size" >= 1 && $$"p_size" <= 10) ||
            (($$"p_brand" === "Brand#34") &&
              Clg && $$"l_quantity" >= 20 && $$"l_quantity" <= 30 &&
              $$"p_size" >= 1 && $$"p_size" <= 15))
      .select(Cdecrease.as("volume"))
      .agg(sum("volume"))
    t.show("query processing: ")

    val ret = fret.to_spark_DF()
    t.show("to_spark_DF: ")
    if (SHOW_OUT) ret.show()

    flineitem.release()
    fpart.release()
    return ret
  }
}
