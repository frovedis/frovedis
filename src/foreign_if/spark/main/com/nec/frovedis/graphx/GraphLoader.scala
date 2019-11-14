package com.nec.frovedis.graphx;

import org.apache.spark.SparkContext
import org.apache.spark.storage.StorageLevel

object GraphLoader {
  def edgeListFile(
      sc: SparkContext,
      path: String,
      canonicalOrientation: Boolean = false,
      numEdgePartitions: Int = -1,
      edgeStorageLevel: StorageLevel = StorageLevel.MEMORY_ONLY,
      vertexStorageLevel: StorageLevel = StorageLevel.MEMORY_ONLY): Graph = {
      val spark_graph = org.apache.spark.graphx.GraphLoader.edgeListFile(sc, path,
                      canonicalOrientation, numEdgePartitions,
                      edgeStorageLevel, vertexStorageLevel)
      return new Graph(spark_graph)
  }
}

