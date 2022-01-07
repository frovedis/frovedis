package main.scala

import org.apache.log4j.{Level, Logger}
import org.apache.spark.sql.DataFrame
import org.apache.spark.SparkContext
/*
import org.apache.spark.sql.functions.avg
import org.apache.spark.sql.functions.count
import org.apache.spark.sql.functions.sum
import org.apache.spark.sql.functions.udf
*/
import com.nec.frovedis.sql.functions.avg
import com.nec.frovedis.sql.functions.count
import com.nec.frovedis.sql.functions.sum

import com.nec.frovedis.matrix.TimeSpent
import com.nec.frovedis.sql.FrovedisDataFrame
import com.nec.frovedis.sql.implicits_._

/**
 * TPC-H Query 22
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
class Q22 extends TpchQuery {

  override def execute(sc: SparkContext, schemaProvider: TpchSchemaProvider): DataFrame = {

    // this is used to implicitly convert an RDD to a DataFrame.
    val sqlContext = new org.apache.spark.sql.SQLContext(sc)
    import sqlContext.implicits._
    import schemaProvider._

/*
 * -------- ORIGINAL --------
 *
    val sub2 = udf { (x: String) => x.substring(0, 2) }
    val phone = udf { (x: String) => x.matches("13|31|23|29|30|18|17") }
    val isNull = udf { (x: Any) => println(x); true }

    val fcustomer = customer.select($"c_acctbal", $"c_custkey", sub2($"c_phone").as("cntrycode"))
      .filter(phone($"cntrycode"))

    val avg_customer = fcustomer.filter($"c_acctbal" > 0.0)
      .agg(avg($"c_acctbal").as("avg_acctbal"))

    order.groupBy($"o_custkey")
      .agg($"o_custkey").select($"o_custkey")
      .join(fcustomer, $"o_custkey" === fcustomer("c_custkey"), "right_outer")
      //.filter("o_custkey is null")
      .filter($"o_custkey".isNull)
      .join(avg_customer)
      .filter($"c_acctbal" > $"avg_acctbal")
      .groupBy($"cntrycode")
      .agg(count($"c_acctbal"), sum($"c_acctbal"))
      .sort($"cntrycode")
*/

    val t = new TimeSpent(Level.INFO)
    val fcustomer = new FrovedisDataFrame(customer, "c_acctbal", "c_custkey", "c_phone")
    val forder = new FrovedisDataFrame(order, "o_custkey")
    t.show("data transfer: ")

    val Csub2 = $$"c_phone".substr(0, 2)
    val Cphone = ($$"cntrycode" === "13") || ($$"cntrycode" === "31") ||
                 ($$"cntrycode" === "23") || ($$"cntrycode" === "29") ||
                 ($$"cntrycode" === "30") || ($$"cntrycode" === "18") ||
                 ($$"cntrycode" === "17")

    val fcustomers = fcustomer.select($$"c_acctbal", $$"c_custkey", Csub2.as("cntrycode"))
                              .filter(Cphone)

    val avg_customer = fcustomers.filter($$"c_acctbal" > 0.0)
                                 .agg(avg($$"c_acctbal").as("avg_acctbal"))

    val forders = forder.groupBy($$"o_custkey").select("o_custkey")

    val fret = fcustomers.join(forders, $$"o_custkey" === fcustomers("c_custkey"), "left_outer")
      .filter($$"o_custkey".isNull)
      .join(avg_customer)
      .filter($$"c_acctbal" > $$"avg_acctbal")
      .groupBy($$"cntrycode")
      .agg(count($$"c_acctbal"), sum($$"c_acctbal"))
      .sort($$"cntrycode")
    t.show("query processing: ")

    val ret = fret.to_spark_DF()
    t.show("to_spark_DF: ")

    fcustomer.release()
    forder.release()
    return ret
  }
}
