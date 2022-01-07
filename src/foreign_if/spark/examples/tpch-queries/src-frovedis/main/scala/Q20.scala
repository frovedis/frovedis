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
 * TPC-H Query 20
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
class Q20 extends TpchQuery {

  override def execute(sc: SparkContext, schemaProvider: TpchSchemaProvider): DataFrame = {

    // this is used to implicitly convert an RDD to a DataFrame.
    val sqlContext = new org.apache.spark.sql.SQLContext(sc)
    import sqlContext.implicits._
    import schemaProvider._

/*
 * -------- ORIGINAL --------
 *
    val forest = udf { (x: String) => x.startsWith("forest") }

    val flineitem = lineitem.filter($"l_shipdate" >= "1994-01-01" && $"l_shipdate" < "1995-01-01")
      .groupBy($"l_partkey", $"l_suppkey")
      .agg((sum($"l_quantity") * 0.5).as("sum_quantity"))

    val fnation = nation.filter($"n_name" === "CANADA")
    val nat_supp = supplier.select($"s_suppkey", $"s_name", $"s_nationkey", $"s_address")
      .join(fnation, $"s_nationkey" === fnation("n_nationkey"))

    part.filter(forest($"p_name"))
      .select($"p_partkey").distinct
      .join(partsupp, $"p_partkey" === partsupp("ps_partkey"))
      .join(flineitem, $"ps_suppkey" === flineitem("l_suppkey") && $"ps_partkey" === flineitem("l_partkey"))
      .filter($"ps_availqty" > $"sum_quantity")
      .select($"ps_suppkey").distinct
      .join(nat_supp, $"ps_suppkey" === nat_supp("s_suppkey"))
      .select($"s_name", $"s_address")
      .sort($"s_name")
*/
    val t = new TimeSpent(Level.INFO)
    val flineitem  = new FrovedisDataFrame(lineitem, "l_shipdate", "l_partkey", "l_suppkey", "l_quantity")
    val fnation = new FrovedisDataFrame(nation, "n_nationkey", "n_name")
    val fsupplier = new FrovedisDataFrame(supplier, "s_nationkey", "s_suppkey", "s_name", "s_address")
    val fpartsupp = new FrovedisDataFrame(partsupp, "ps_suppkey", "ps_partkey", "ps_availqty")
    val fpart = new FrovedisDataFrame(part, "p_name", "p_partkey")
    t.show("data transfer: ")

    val Cforest = $$"p_name".startsWith("forest")

    val flineitems = flineitem.filter($$"l_shipdate" >= "1994-01-01" && $$"l_shipdate" < "1995-01-01")
      .groupBy($$"l_partkey", $$"l_suppkey")
      //.agg((sum($$"l_quantity") * 0.5).as("sum_quantity")) // operation on aggregator is not supported
      .agg(sum($$"l_quantity").as("x"))
      .select($$"l_partkey", $$"l_suppkey", ($$"x" * 0.5).as("sum_quantity"))

    val fnations = fnation.filter($$"n_name" === "CANADA")
    val nat_supp = fsupplier.select($$"s_suppkey", $$"s_name", $$"s_nationkey", $$"s_address")
      .join(fnations, $$"s_nationkey" === fnations("n_nationkey"))

    val fret = fpart.filter(Cforest)
      .select($$"p_partkey").distinct
      .join(fpartsupp, $$"p_partkey" === fpartsupp("ps_partkey"))
      .join(flineitems, $$"ps_suppkey" === flineitems("l_suppkey") && 
                        $$"ps_partkey" === flineitems("l_partkey"))
      .filter($$"ps_availqty" > $$"sum_quantity")
      .select($$"ps_suppkey").distinct
      .join(nat_supp, $$"ps_suppkey" === nat_supp("s_suppkey"))
      .select($$"s_name", $$"s_address")
      .sort($$"s_name")
    t.show("query processing: ")

    val ret = fret.to_spark_DF()
    t.show("to_spark_DF: ")

    flineitem.release()
    fnation.release()
    fsupplier.release()
    fpartsupp.release()
    fpart.release()
    return ret
  }
}
