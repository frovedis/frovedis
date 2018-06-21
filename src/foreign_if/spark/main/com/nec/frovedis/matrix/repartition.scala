package com.nec.frovedis.matrix;

import scala.reflect.ClassTag
import org.apache.spark.rdd.RDD
import org.apache.spark.Partitioner

class MyPartitioner (nparts: Int, nelems: Long) extends Partitioner {
  def numPartitions = nparts
  def getPartition(key: Any): Int = {
    val k = key.asInstanceOf[Long]
    val size = getChunkSize()
    if (size == 0) return 0
    else return (k / size).intValue
  }
  private def getChunkSize(): Int = {
    if (nelems == 0) return 0
    else return ((nelems-1)/nparts + 1).intValue
  }
}

object Utils extends java.io.Serializable {
  implicit class RddRepartition[T: ClassTag](data: RDD[T]) {
    def repartition2(t_nparts: Int): RDD[T] = {
      val nparts = data.getNumPartitions
      if (nparts == t_nparts) return data
      else {
        val nelems = data.count
        val iData = data.zipWithIndex().map({case (v,i) => (i,v)})
        return iData.partitionBy(new MyPartitioner(t_nparts,nelems))
                    .map({case (k,v) => v })
      }
    }
  }
}
