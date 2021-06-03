package com.nec.frovedis.io;

import java.io.File
import scala.reflect.ClassTag
import org.apache.spark.SparkContext

object FrovedisIO {
  def createDir(path: String): Boolean = {
    return new File(path).mkdir()
  }
  def createFile(path: String): Boolean = {
    return new File(path).createNewFile()
  }
  def checkExists(path: String): Boolean = {
    return new File(path).exists
  }
  def saveDictionary[K,V](target: Map[K,V],
                          path: String): Unit = {
    val context = SparkContext.getOrCreate()
    context.parallelize(target.toSeq, 2).saveAsObjectFile(path)
  }
  def loadDictionary[K: ClassTag, V: ClassTag](path: String): Map[K,V] = {
    val context = SparkContext.getOrCreate()
    val target = context.objectFile[(K,V)](path)
                        .collectAsMap() // returns generic scala.collection.Map
                        .toMap          // to make it immutable Map
    return target
  }
}
