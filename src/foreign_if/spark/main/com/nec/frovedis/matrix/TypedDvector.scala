package com.nec.frovedis.matrix;

import com.nec.frovedis.Jexrpc._
import com.nec.frovedis.matrix.Utils._
import org.apache.spark.rdd.RDD
import org.apache.spark.SparkContext

//TODO: Not needed
case class info(proxy: Long, size: Long)

object DTYPE {
  val NONE:   Short = 0
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
    val err = JNISupport.checkServerException()
    if (err != "") throw new java.rmi.ServerException(err)
    //mapPartitionsWithIndex needs to return an Iterator object
    return Array(info(proxy,size)).toIterator
  }
  def get(data: RDD[Int]) : Long = {
    //data.collect.foreach(println)
    val fs = FrovedisServer.getServerInstance()
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    var info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val wdata = data.repartition2(fs.worker_size)
    val ret = wdata.mapPartitionsWithIndex(
              (i,x) => copy_local_data(i,x,fw_nodes(i))).collect
    val proxies = ret.map(_.proxy)
    val sizes = ret.map(_.size)
    val ret_p = JNISupport.createFrovedisDvector(fs.master_node,proxies,sizes,
                                          ret.size,DTYPE.INT)
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    else return ret_p
  }
  private def setEachPartition(nid: Int,
                               wnode: Node,
                               dptr: Long): Iterator[Int] = {
    val lvec = JNISupport.getFrovedisWorkerIntVector(wnode, dptr)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return lvec.toIterator
  }
  def to_RDD(proxy: Long): RDD[Int] = {
    val fs = FrovedisServer.getServerInstance()
    val eps = JNISupport.getLocalVectorPointers(fs.master_node, proxy, DTYPE.INT)
    var info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val dummy = new Array[Boolean](eps.size)
    val ctxt = SparkContext.getOrCreate()
    val dist_dummy = ctxt.parallelize(dummy, eps.size)
    val nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = dist_dummy.mapPartitionsWithIndex((i,x) =>
                    setEachPartition(i,nodes(i),eps(i))).cache()
    return ret
  }
}

object LongDvector {
  private def copy_local_data(index: Int, data: Iterator[Long],
                              t_node: Node) : Iterator[info] = {
    val darr = data.toArray
    val size = darr.size
    //println("index: " + index + ", size: " + size)
    val proxy = JNISupport.loadFrovedisWorkerLongVector(t_node,size,darr)
    val err = JNISupport.checkServerException()
    if (err != "") throw new java.rmi.ServerException(err)
    //mapPartitionsWithIndex needs to return an Iterator object
    return Array(info(proxy,size)).toIterator
  }
  def get(data: RDD[Long]) : Long = {
    //data.collect.foreach(println)
    val fs = FrovedisServer.getServerInstance()
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    var info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info)
    val wdata = data.repartition2(fs.worker_size)
    val ret = wdata.mapPartitionsWithIndex(
              (i,x) => copy_local_data(i,x,fw_nodes(i))).collect
    val proxies = ret.map(_.proxy)
    val sizes = ret.map(_.size)
    val ret_p = JNISupport.createFrovedisDvector(fs.master_node,proxies,sizes,
                                          ret.size,DTYPE.LONG)
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    else return ret_p
  }
  private def setEachPartition(nid: Int,
                               wnode: Node,
                               dptr: Long): Iterator[Long] = {
    val lvec = JNISupport.getFrovedisWorkerLongVector(wnode, dptr)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return lvec.toIterator
  }
  def to_RDD(proxy: Long): RDD[Long] = {
    val fs = FrovedisServer.getServerInstance()
    val eps = JNISupport.getLocalVectorPointers(fs.master_node, proxy, DTYPE.LONG)
    var info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val dummy = new Array[Boolean](eps.size)
    val ctxt = SparkContext.getOrCreate()
    val dist_dummy = ctxt.parallelize(dummy, eps.size)
    val nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = dist_dummy.mapPartitionsWithIndex((i,x) =>
                    setEachPartition(i,nodes(i),eps(i))).cache()
    return ret
  }
}

object FloatDvector {
  private def copy_local_data(index: Int, data: Iterator[Float],
                              t_node: Node) : Iterator[info] = {
    val darr = data.toArray
    val size = darr.size
    //println("index: " + index + ", size: " + size)
    val proxy = JNISupport.loadFrovedisWorkerFloatVector(t_node,size,darr)
    val err = JNISupport.checkServerException()
    if (err != "") throw new java.rmi.ServerException(err)
    //mapPartitionsWithIndex needs to return an Iterator object
    return Array(info(proxy,size)).toIterator
  }
  def get(data: RDD[Float]) : Long = {
    //data.collect.foreach(println)
    val fs = FrovedisServer.getServerInstance()
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    var info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val wdata = data.repartition2(fs.worker_size)
    val ret = wdata.mapPartitionsWithIndex(
              (i,x) => copy_local_data(i,x,fw_nodes(i))).collect
    val proxies = ret.map(_.proxy)
    val sizes = ret.map(_.size)
    val ret_p =  JNISupport.createFrovedisDvector(fs.master_node,proxies,sizes,
                                          ret.size,DTYPE.FLOAT)
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    else return ret_p
  }
  private def setEachPartition(nid: Int,
                               wnode: Node,
                               dptr: Long): Iterator[Float] = {
    val lvec = JNISupport.getFrovedisWorkerFloatVector(wnode, dptr)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return lvec.toIterator
  }
  def to_RDD(proxy: Long): RDD[Float] = {
    val fs = FrovedisServer.getServerInstance()
    val eps = JNISupport.getLocalVectorPointers(fs.master_node, proxy, DTYPE.FLOAT)
    var info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val dummy = new Array[Boolean](eps.size)
    val ctxt = SparkContext.getOrCreate()
    val dist_dummy = ctxt.parallelize(dummy, eps.size)
    val nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = dist_dummy.mapPartitionsWithIndex((i,x) =>
                    setEachPartition(i,nodes(i),eps(i))).cache()
    return ret
  }
}

object DoubleDvector {
  private def copy_local_data(index: Int, data: Iterator[Double],
                              t_node: Node) : Iterator[info] = {
    val darr = data.toArray
    val size = darr.size
    //println("index: " + index + ", size: " + size)
    val proxy = JNISupport.loadFrovedisWorkerDoubleVector(t_node,size,darr)
    val err = JNISupport.checkServerException()
    if (err != "") throw new java.rmi.ServerException(err)
    //mapPartitionsWithIndex needs to return an Iterator object
    return Array(info(proxy,size)).toIterator
  }
  def get(data: RDD[Double]) : Long = {
    //data.collect.foreach(println)
    val fs = FrovedisServer.getServerInstance()
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    var info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val wdata = data.repartition2(fs.worker_size)
    val ret = wdata.mapPartitionsWithIndex(
              (i,x) => copy_local_data(i,x,fw_nodes(i))).collect
    val proxies = ret.map(_.proxy)
    val sizes = ret.map(_.size)
    val ret_p = JNISupport.createFrovedisDvector(fs.master_node,proxies,sizes,
                                          ret.size,DTYPE.DOUBLE)
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info) 
    else return ret_p
  }
  private def setEachPartition(nid: Int,
                               wnode: Node,
                               dptr: Long): Iterator[Double] = {
    val lvec = JNISupport.getFrovedisWorkerDoubleVector(wnode, dptr)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return lvec.toIterator
  }
  def to_RDD(proxy: Long): RDD[Double] = {
    val fs = FrovedisServer.getServerInstance()
    val eps = JNISupport.getLocalVectorPointers(fs.master_node, proxy, DTYPE.DOUBLE)
    var info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val dummy = new Array[Boolean](eps.size)
    val ctxt = SparkContext.getOrCreate()
    val dist_dummy = ctxt.parallelize(dummy, eps.size)
    val nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = dist_dummy.mapPartitionsWithIndex((i,x) =>
                    setEachPartition(i,nodes(i),eps(i))).cache()
    return ret
  }
}

object StringDvector {
  private def copy_local_data(index: Int, data: Iterator[String],
                              t_node: Node) : Iterator[info] = {
    val darr = data.toArray
    val size = darr.size
    //println("index: " + index + ", size: " + size)
    val proxy = JNISupport.loadFrovedisWorkerStringVector(t_node,size,darr)
    val err = JNISupport.checkServerException()
    if (err != "") throw new java.rmi.ServerException(err)
    //mapPartitionsWithIndex needs to return an Iterator object
    return Array(info(proxy,size)).toIterator
  }
  def get(data: RDD[String]) : Long = {
    //data.collect.foreach(println)
    val fs = FrovedisServer.getServerInstance()
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    var info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val wdata = data.repartition2(fs.worker_size)
    val ret = wdata.mapPartitionsWithIndex(
              (i,x) => copy_local_data(i,x,fw_nodes(i))).collect
    val proxies = ret.map(_.proxy)
    val sizes = ret.map(_.size)
    val ret_p = JNISupport.createFrovedisDvector(fs.master_node,proxies,sizes,
                                          ret.size,DTYPE.STRING)
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    else return ret_p
  }
  private def setEachPartition(nid: Int,
                               wnode: Node,
                               dptr: Long): Iterator[String] = {
    val lvec = JNISupport.getFrovedisWorkerStringVector(wnode, dptr)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return lvec.toIterator
  }
  def to_RDD(proxy: Long): RDD[String] = {
    val fs = FrovedisServer.getServerInstance()
    val eps = JNISupport.getLocalVectorPointers(fs.master_node, proxy, DTYPE.STRING)
    var info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val dummy = new Array[Boolean](eps.size)
    val ctxt = SparkContext.getOrCreate()
    val dist_dummy = ctxt.parallelize(dummy, eps.size)
    val nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = dist_dummy.mapPartitionsWithIndex((i,x) =>
                    setEachPartition(i,nodes(i),eps(i))).cache()
    return ret
  }
}

object BoolDvector {
  private def copy_local_data(index: Int, data: Iterator[Boolean],
                              t_node: Node) : Iterator[info] = {
    val darr = data.toArray
    val size = darr.size
    //println("index: " + index + ", size: " + size)
    val proxy = JNISupport.loadFrovedisWorkerBoolVector(t_node,size,darr)
    val err = JNISupport.checkServerException()
    if (err != "") throw new java.rmi.ServerException(err)
    //mapPartitionsWithIndex needs to return an Iterator object
    return Array(info(proxy,size)).toIterator
  }
  def get(data: RDD[Boolean]) : Long = {
    //data.collect.foreach(println)
    val fs = FrovedisServer.getServerInstance()
    val fw_nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    var info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    val wdata = data.repartition2(fs.worker_size)
    val ret = wdata.mapPartitionsWithIndex(
              (i,x) => copy_local_data(i,x,fw_nodes(i))).collect
    val proxies = ret.map(_.proxy)
    val sizes = ret.map(_.size)
    val ret_p = JNISupport.createFrovedisDvector(fs.master_node,proxies,sizes,
                                          ret.size,DTYPE.BOOL)
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    else return ret_p
  }
  private def setEachPartition(nid: Int,
                               wnode: Node,
                               dptr: Long): Iterator[Boolean] = {
    val lvec = JNISupport.getFrovedisWorkerBoolVector(wnode, dptr)
    val size = lvec.size.intValue
    var ret = new Array[Boolean](size)
    for (i <- 0 until size) ret(i) = (lvec(i) == 1)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return ret.toIterator
  }
  def to_RDD(proxy: Long): RDD[Boolean] = {
    val fs = FrovedisServer.getServerInstance()
    val eps = JNISupport.getLocalVectorPointers(fs.master_node, proxy, DTYPE.BOOL)
    var info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val dummy = new Array[Boolean](eps.size)
    val ctxt = SparkContext.getOrCreate()
    val dist_dummy = ctxt.parallelize(dummy, eps.size)
    val nodes = JNISupport.getWorkerInfo(fs.master_node) // native call
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = dist_dummy.mapPartitionsWithIndex((i,x) =>
                    setEachPartition(i,nodes(i),eps(i))).cache()
    return ret
  }
}

