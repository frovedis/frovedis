package com.nec.frovedis.matrix;

import com.nec.frovedis.Jexrpc._
import com.nec.frovedis.matrix.Utils._
import org.apache.spark.rdd.RDD

//TODO: Not needed
case class info(proxy: Long, size: Long)

object DTYPE {
  val INT:    Short = 1
  val LONG:   Short = 2
  val FLOAT:  Short = 3
  val DOUBLE: Short = 4
  val STRING: Short = 5
  val BOOL:   Short = 6
}

object IntDvector {
  private def copy_local_data(index: Int, data: Iterator[Int],
                              t_node: Node) : Iterator[info] = {
    val darr = data.toArray
    val size = darr.size
    //println("index: " + index + ", size: " + size)
    val proxy = JNISupport.loadFrovedisWorkerIntVector(t_node,size,darr)
    //mapPartitionsWithIndex needs to return an Iterator object
    return Array(info(proxy,size)).toIterator
  }
  
  def get(data: RDD[Int]) : Long = {
    //data.collect.foreach(println)
    val fs = FrovedisServer.getServerInstance()
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    val wdata = data.repartition2(fs.worker_size)
    val ret = wdata.mapPartitionsWithIndex(
              (i,x) => copy_local_data(i,x,fw_nodes(i))).collect
    val proxies = ret.map(_.proxy)
    val sizes = ret.map(_.size)
    return JNISupport.createFrovedisDvector(fs.master_node,proxies,sizes,
                                          ret.size,DTYPE.INT)
  }
}

object LongDvector {
  private def copy_local_data(index: Int, data: Iterator[Long],
                              t_node: Node) : Iterator[info] = {
    val darr = data.toArray
    val size = darr.size
    //println("index: " + index + ", size: " + size)
    val proxy = JNISupport.loadFrovedisWorkerLongVector(t_node,size,darr)
    //mapPartitionsWithIndex needs to return an Iterator object
    return Array(info(proxy,size)).toIterator
  }

  def get(data: RDD[Long]) : Long = {
    //data.collect.foreach(println)
    val fs = FrovedisServer.getServerInstance()
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    val wdata = data.repartition2(fs.worker_size)
    val ret = wdata.mapPartitionsWithIndex(
              (i,x) => copy_local_data(i,x,fw_nodes(i))).collect
    val proxies = ret.map(_.proxy)
    val sizes = ret.map(_.size)
    return JNISupport.createFrovedisDvector(fs.master_node,proxies,sizes,
                                          ret.size,DTYPE.LONG)
  }
}

object FloatDvector {
  private def copy_local_data(index: Int, data: Iterator[Float],
                              t_node: Node) : Iterator[info] = {
    val darr = data.toArray
    val size = darr.size
    //println("index: " + index + ", size: " + size)
    val proxy = JNISupport.loadFrovedisWorkerFloatVector(t_node,size,darr)
    //mapPartitionsWithIndex needs to return an Iterator object
    return Array(info(proxy,size)).toIterator
  }
  
  def get(data: RDD[Float]) : Long = {
    //data.collect.foreach(println)
    val fs = FrovedisServer.getServerInstance()
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    val wdata = data.repartition2(fs.worker_size)
    val ret = wdata.mapPartitionsWithIndex(
              (i,x) => copy_local_data(i,x,fw_nodes(i))).collect
    val proxies = ret.map(_.proxy)
    val sizes = ret.map(_.size)
    return JNISupport.createFrovedisDvector(fs.master_node,proxies,sizes,
                                          ret.size,DTYPE.FLOAT)
  }
}

object DoubleDvector {
  private def copy_local_data(index: Int, data: Iterator[Double],
                              t_node: Node) : Iterator[info] = {
    val darr = data.toArray
    val size = darr.size
    //println("index: " + index + ", size: " + size)
    val proxy = JNISupport.loadFrovedisWorkerDoubleVector(t_node,size,darr)
    //mapPartitionsWithIndex needs to return an Iterator object
    return Array(info(proxy,size)).toIterator
  }
  
  def get(data: RDD[Double]) : Long = {
    //data.collect.foreach(println)
    val fs = FrovedisServer.getServerInstance()
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    val wdata = data.repartition2(fs.worker_size)
    val ret = wdata.mapPartitionsWithIndex(
              (i,x) => copy_local_data(i,x,fw_nodes(i))).collect
    val proxies = ret.map(_.proxy)
    val sizes = ret.map(_.size)
    return JNISupport.createFrovedisDvector(fs.master_node,proxies,sizes,
                                          ret.size,DTYPE.DOUBLE)
  }
}

object StringDvector {
  private def copy_local_data(index: Int, data: Iterator[String],
                              t_node: Node) : Iterator[info] = {
    val darr = data.toArray
    val size = darr.size
    //println("index: " + index + ", size: " + size)
    val proxy = JNISupport.loadFrovedisWorkerStringVector(t_node,size,darr)
    //mapPartitionsWithIndex needs to return an Iterator object
    return Array(info(proxy,size)).toIterator
  }

  def get(data: RDD[String]) : Long = {
    //data.collect.foreach(println)
    val fs = FrovedisServer.getServerInstance()
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    val wdata = data.repartition2(fs.worker_size)
    val ret = wdata.mapPartitionsWithIndex(
              (i,x) => copy_local_data(i,x,fw_nodes(i))).collect
    val proxies = ret.map(_.proxy)
    val sizes = ret.map(_.size)
    return JNISupport.createFrovedisDvector(fs.master_node,proxies,sizes,
                                          ret.size,DTYPE.STRING)
  }
}

object BoolDvector {
  private def copy_local_data(index: Int, data: Iterator[Boolean],
                              t_node: Node) : Iterator[info] = {
    val darr = data.toArray
    val size = darr.size
    //println("index: " + index + ", size: " + size)
    val proxy = JNISupport.loadFrovedisWorkerBoolVector(t_node,size,darr)
    //mapPartitionsWithIndex needs to return an Iterator object
    return Array(info(proxy,size)).toIterator
  }
  
  def get(data: RDD[Boolean]) : Long = {
    //data.collect.foreach(println)
    val fs = FrovedisServer.getServerInstance()
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    val wdata = data.repartition2(fs.worker_size)
    val ret = wdata.mapPartitionsWithIndex(
              (i,x) => copy_local_data(i,x,fw_nodes(i))).collect
    val proxies = ret.map(_.proxy)
    val sizes = ret.map(_.size)
    return JNISupport.createFrovedisDvector(fs.master_node,proxies,sizes,
                                          ret.size,DTYPE.BOOL)
  }
}

