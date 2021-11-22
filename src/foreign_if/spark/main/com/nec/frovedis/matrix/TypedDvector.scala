package com.nec.frovedis.matrix;

import Array._
import scala.reflect.ClassTag
import com.nec.frovedis.Jexrpc._
import org.apache.spark.rdd.RDD
import org.apache.spark.SparkContext
import org.apache.log4j.{Level, Logger}

object DTYPE {
  val NONE:   Short = 0
  val INT:    Short = 1
  val LONG:   Short = 2
  val FLOAT:  Short = 3
  val DOUBLE: Short = 4
  val STRING: Short = 5
  val BOOL:   Short = 6
  val ULONG:  Short = 7
  val WORDS:  Short = 8
}

object TransferData {
  //private def copy_local_data[T: ClassTag](
    //index: Int, destId: Int,
    //w_node: Node, vptr: Long, localId: Long, 
    //data: Iterator[T], size: Int, dtype: Short): Unit = {

  private def copy_local_data(
    index: Int, destId: Int,
    w_node: Node, vptr: Long, localId: Long, 
    data: Iterator[Any], size: Int, dtype: Short): Unit = {
    
    //println("dest[" + destId + "::" + localId + "] partition-" +
    //       index + " of size: " + size + " is being copied!")

    val t0 = new TimeSpent(Level.TRACE)
    dtype match { // w_node::vptr[index] = data
        case DTYPE.INT => { 
          //val iArr = new Array[Int](size); data.copyToArray(iArr)
          val iArr = new Array[Int](size)
          for(i <- 0 until size) iArr(i) = data.next.asInstanceOf[Int]
          t0.show("iterator to int-array: ")
          JNISupport.loadFrovedisWorkerIntVector(w_node, vptr, localId, iArr,
                                                 iArr.size) 
        }
        case DTYPE.LONG => { 
          //val lArr = new Array[Long](size); data.copyToArray(lArr)
          val lArr = new Array[Long](size)
          for(i <- 0 until size) lArr(i) = data.next.asInstanceOf[Long]
          t0.show("iterator to long-array: ")
          JNISupport.loadFrovedisWorkerLongVector(w_node, vptr, localId, lArr,
                                                  lArr.size) 
        }
        case DTYPE.FLOAT => { 
          //val fArr = new Array[Float](size); data.copyToArray(fArr)
          val fArr = new Array[Float](size)
          for(i <- 0 until size) fArr(i) = data.next.asInstanceOf[Float]
          t0.show("iterator to float-array: ")
          JNISupport.loadFrovedisWorkerFloatVector(w_node, vptr, localId, fArr,
                                                   fArr.size) 
        }
        case DTYPE.DOUBLE => { 
          //val dArr = new Array[Double](size); data.copyToArray(dArr)
          val dArr = new Array[Double](size)
          for(i <- 0 until size) dArr(i) = data.next.asInstanceOf[Double]
          t0.show("iterator to double-array: ")
          JNISupport.loadFrovedisWorkerDoubleVector(w_node, vptr, localId, dArr, 
                                                    dArr.size) 
        }
        /* 
        case DTYPE.STRING => { 
          //val sArr = new Array[String](size); data.copyToArray(sArr)
          val sArr = new Array[String](size)
          for(i <- 0 until size) sArr(i) = data.next.asInstanceOf[String]
          t0.show("iterator to string-array: ")
          JNISupport.loadFrovedisWorkerStringVector(w_node, vptr, localId, sArr, 
                                                    sArr.size)
        }
        */
        case DTYPE.STRING => { 
          val sArr = new Array[Array[Char]](size)
          for(i <- 0 until size) sArr(i) = (data.next.asInstanceOf[String]).toCharArray
          t0.show("iterator to array-of-CharArray: ")
          //JNISupport.loadFrovedisWorkerCharArrayVector(w_node, vptr, localId, sArr, 
          //                                             sArr.size)

          //val flat_sArr = sArr.flatten
          //val sizes_arr = sArr.map(x => x.size)
          val (flat_sArr, sizes_arr) = GenericUtils.flatten(sArr)
          t0.show("array-of-CharArray flatten: ")
          JNISupport.loadFrovedisWorkerCharArray(w_node, vptr, localId, 
                                                 flat_sArr, sizes_arr, 
                                                 flat_sArr.size, size) 
        }
        case DTYPE.BOOL => { 
          //val bArr = new Array[Boolean](size); data.copyToArray(bArr)
          val bArr = new Array[Boolean](size)
          for(i <- 0 until size) bArr(i) = data.next.asInstanceOf[Boolean]
          t0.show("iterator to boolean-array: ")
          JNISupport.loadFrovedisWorkerBoolVector(w_node, vptr, localId, bArr, 
                                                  bArr.size) 
        }
        case _ => throw new IllegalArgumentException("Unsupported type: " + dtype)
    }
    val err = JNISupport.checkServerException()
    if (err != "") throw new java.rmi.ServerException(err)
    t0.show("spark-worker to frovedis-rank local data copy: ")
  }
  def execute[T: ClassTag](data: RDD[T], dtype: Short): Long = {
    val part_sizes = data.mapPartitions({ 
      case(x) => 
        val t0 = new TimeSpent(Level.TRACE)
        val ret = Array(x.size).toIterator
        t0.show("partition sizes extraction: ")
        ret 
    }) 
    return execute(data, part_sizes, dtype)
  }
  def execute[T: ClassTag](data: RDD[T], part_sizes: RDD[Int],
                           dtype: Short): Long = {
    val t_log = new TimeSpent(Level.DEBUG)

    // (1) allocate
    val fs = FrovedisServer.getServerInstance()
    val nproc = fs.worker_size
    val npart = data.getNumPartitions
    val block_sizes = GenericUtils.get_block_sizes(npart, nproc)
    val vptrs = JNISupport.allocateLocalVector(fs.master_node, block_sizes, 
                                               nproc, dtype)
    var info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    t_log.show("server memory allocation: ")

    // (2) prepare server side ranks for processing N parallel requests
    val fw_nodes = JNISupport.getWorkerInfoMulti(fs.master_node,
                                                 block_sizes, nproc)
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    t_log.show("server process preparation: ")

    // (3) data transfer
    val tmp = data.zipPartitions(part_sizes, 
              preservesPartitioning=true) ({ 
                case(x, x_sz) => 
                  val t0 = new TimeSpent(Level.TRACE)
                  val ret = x_sz ++ x // (Int ++ T) -> Any
                  t0.show("zip partition: ")
                  ret 
              })

    //val psizes = part_sizes.collect
    //val ret = data.mapPartitionsWithIndex({
    val ret = tmp.mapPartitionsWithIndex({
                 case (index, x) =>
                     //val size = psizes(index)
                     val size = x.next.asInstanceOf[Int]
                     val (destId, myst) = GenericUtils.index2Dest(index, block_sizes)
                     val localId = index - myst
                     copy_local_data(index, destId, fw_nodes(destId), vptrs(destId),
                                     localId, x, size, dtype)
                     Array(true).toIterator // SUCCESS (since need to return an iterator)
                     //Array((destId, size.toLong)).toIterator
                 })
    ret.count // for action
    t_log.show("server side data transfer: ")

/*
    // sizes calculation when part_sizes is collected
    var k: Int = 0
    var sizes = new Array[Long](nproc)
    for (i <- 0 until nproc) {
      sizes(i) = 0
      val bsz = block_sizes(i).toInt
      for (j <- 0 until bsz) sizes(i) += psizes(k + j)
      k += bsz
    }
    t_log.show("sizes calc: ")
*/

/*
    // sizes calculation when (destId, size) pair is returned
    val tmpsz = new Array[(Int, Long)](nproc)
    for (i <- 0 until nproc) tmpsz(i) = (i, 0L)
    val ctxt = SparkContext.getOrCreate()
    val sizes = ctxt.parallelize(tmpsz).union(ret).reduceByKey(_ + _)
                    .sortByKey().collect.map(_._2)
    t_log.show("sizes calc: ")
*/

    // (4) merge chunks and create dvector
    //val ret_p = JNISupport.createFrovedisDvectorWithSizesVerification(
    //              fs.master_node, vptrs, sizes, nproc, dtype)
    val ret_p = JNISupport.createFrovedisDvector(
                  fs.master_node, vptrs, nproc, dtype)
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    t_log.show("create dvector: ")

    return ret_p
  }
}

object IntDvector {
  def get(data: RDD[Int]) = TransferData.execute(data, DTYPE.INT)
  def get(data: RDD[Int], part_sizes: RDD[Int]): Long = {
    TransferData.execute(data, part_sizes, DTYPE.INT)
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
  def get(data: RDD[Long]) = TransferData.execute(data, DTYPE.LONG)
  def get(data: RDD[Long], part_sizes: RDD[Int]): Long = {
    return TransferData.execute(data, part_sizes, DTYPE.LONG)
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
  def get(data: RDD[Float]) = TransferData.execute(data, DTYPE.FLOAT)
  def get(data: RDD[Float], part_sizes: RDD[Int]): Long = {
    return TransferData.execute(data, part_sizes, DTYPE.FLOAT)
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
  def get(data: RDD[Double]) = TransferData.execute(data, DTYPE.DOUBLE)
  def get(data: RDD[Double], part_sizes: RDD[Int]): Long = {
    return TransferData.execute(data, part_sizes, DTYPE.DOUBLE)
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

object StringDvector { // actually node_local<words>
  def get(data: RDD[String]): Long = {
    //return TransferData.execute(data, DTYPE.STRING)
    val part_sizes = data.mapPartitions({ 
      case(x) => 
        val t0 = new TimeSpent(Level.TRACE)
        val ret = Array(x.size).toIterator
        t0.show("partition sizes extraction: ")
        ret 
    })
    return get(data, part_sizes) 
  }
  def get(data: RDD[String], part_sizes: RDD[Int]): Long = {
    //return TransferData.execute(data, part_sizes, DTYPE.STRING)
    val tmp = data.zipPartitions(part_sizes, 
              preservesPartitioning=true) ({ 
                case(x, x_sz) => 
                  val t0 = new TimeSpent(Level.TRACE)
                  val ret = x_sz ++ x // (Int ++ T) -> Any
                  t0.show("zip partition: ")
                  ret 
              })

    val intData = tmp.mapPartitions({ case(x) =>
      val t0 = new TimeSpent(Level.TRACE)
      val size = x.next.asInstanceOf[Int]
      val sArr = new Array[Array[Char]](size)
      for(i <- 0 until size) sArr(i) = (x.next.asInstanceOf[String]).toCharArray
      t0.show("iterator to array-of-CharArray: ")
      //val flat_sArr = sArr.flatten.map(x => x.toInt)
      val (flat_sArr, sizes_arr) = GenericUtils.flatten_charArray_asIntArray(sArr)
      t0.show("array-of-CharArray flatten as IntArray: ")
      flat_sArr.toIterator 
    })

    val t0 = new TimeSpent(Level.DEBUG)
    val sizeData = data.map(x => x.size)
    val sizesDvec = IntDvector.get(sizeData, part_sizes)
    t0.show("size dvector creation: ")

    val intDataDvec = IntDvector.get(intData)
    t0.show("char data dvector creation: ")

    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.createNodeLocalOfWords(fs.master_node, 
                                          intDataDvec, sizesDvec)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    t0.show("node_local<words> creation: ")
    return proxy
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
  def get(data: RDD[Boolean]): Long = {
    //return TransferData.execute(data, DTYPE.BOOL)
    val t0 = new TimeSpent(Level.DEBUG)
    val iData: RDD[Int] = data.map(x => if(x) 1 else 0).persist
    iData.count // for above action to take place
    t0.show("RDD[Boolean] -> RDD[Int] conversion: ")
    return TransferData.execute(iData, DTYPE.INT)
  }
  def get(data: RDD[Boolean], part_sizes: RDD[Int]): Long = {
    //return TransferData.execute(data, part_sizes, DTYPE.BOOL)
    val t0 = new TimeSpent(Level.DEBUG)
    val iData: RDD[Int] = data.map(x => if(x) 1 else 0).persist
    iData.count // for above action to take place
    t0.show("RDD[Boolean] -> RDD[Int] conversion: ")
    return TransferData.execute(iData, part_sizes, DTYPE.INT)
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

