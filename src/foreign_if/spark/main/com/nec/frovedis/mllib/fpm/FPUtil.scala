package com.nec.frovedis.mllib.fpm;

import scala.collection.mutable.ArrayBuffer
import org.apache.spark.sql.SparkSession
import org.apache.spark.rdd.RDD
import org.apache.spark.sql.DataFrame
import scala.reflect.ClassTag
import scala.reflect.runtime.universe._

object FPUtil {
  private def get_table[T:ClassTag](data: Iterator[(Array[T],Long)]): 
              Iterator[(Int,T)] = {
    val darr = data.toArray
    var ret = new ArrayBuffer[(Int,T)]()
    for (i <- 0 to (darr.length-1)) {
      val id = darr(i)._2.intValue
      val items = darr(i)._1
      for (j <- 0 to (items.length-1)) {
        val p = (id, items(j))
        ret += p
      }
    }
    return ret.toIterator
  }
  def convert_to_spark_dataframe[T: ClassTag: TypeTag](tr: RDD[Array[T]]): 
        DataFrame = {
    val spark = SparkSession.builder().getOrCreate()
    import spark.implicits._
    return tr.zipWithIndex()
             .mapPartitions(get_table).toDF("trans_id","item")
  }
}

