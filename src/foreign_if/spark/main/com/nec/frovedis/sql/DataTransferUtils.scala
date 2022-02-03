package com.nec.frovedis.sql;

import com.nec.frovedis.Jexrpc._
import com.nec.frovedis.Jmatrix.configs
import com.nec.frovedis.Jsql.{jPlatform, jDFTransfer}
import com.nec.frovedis.matrix.{GenericUtils, TimeSpent, DTYPE}
import com.nec.frovedis.matrix.{
  IntDvector, LongDvector,
  FloatDvector, DoubleDvector,
  StringDvector, WordsNodeLocal,
  BoolDvector}
import com.nec.frovedis.Jmatrix.OffHeapArray
import org.apache.log4j.Level
import org.apache.spark.rdd.RDD
import org.apache.spark.sql.catalyst.InternalRow
import org.apache.spark.sql.catalyst.expressions.UnsafeRow
import org.apache.spark.sql.execution.exchange.EnsureRequirements
import org.apache.spark.sql.execution.{QueryExecution, PlanSubqueries, 
                                       CollapseCodegenStages, SparkPlan}
import org.apache.spark.sql.catalyst.rules.Rule
import org.apache.spark.sql.vectorized.ColumnarBatch
import org.apache.spark.sql.DataFrame
import scala.collection.mutable.{Map => mMap}
import scala.collection.mutable.ArrayBuffer
import scala.collection.mutable.ListBuffer

object DFMemoryManager extends java.io.Serializable { 
  val df_pool = new ListBuffer[FrovedisDataFrame]()
  val free_pool = new ListBuffer[FrovedisDataFrame]()
  val table = mMap[(Int, String), FrovedisDataFrame]() // (code, colName) -> fdf

  def insert(code: Int,
             fdf: FrovedisDataFrame,
             cols: Iterator[String]): Unit = {
    while(cols.hasNext) table((code, cols.next)) = fdf
    df_pool += fdf
    //println("inserted table size: " + fdf.mem_size + " MB")
  }

  def get(key: (Int, String)): FrovedisDataFrame = {
    if (table.contains(key)) {
      val ret = table(key)
      hit(ret)
      return ret
    }
    else return null
  }

  def add_release_entry(fdf: FrovedisDataFrame): Unit = {
    free_pool += fdf
  }

  def release(): Unit = {
    var e: FrovedisDataFrame = null
    if (!free_pool.isEmpty) {
      e = free_pool.remove(0)
      val idx = df_pool.indexOf(e) // should not be -1
      if (idx != -1) df_pool.remove(idx)
    }
    else {
      if (!df_pool.isEmpty) e = df_pool.remove(0)
    }

    if (e != null) { // entry found either in free_pool or in df_pool
      val code = e.get_code()
      val cols = e.owned_cols
      for (i <- 0 until cols.size) {
        val key = (code, cols(i))
        if (table.contains(key)) table.remove(key)
        //else println(key + ": not found!")
      }
      //println("releasing: " + e.get())
      e.ref.release() // forced release
    }
  }

  def show_release_target(): Unit = {
    if (!free_pool.isEmpty) {
      val cols = free_pool(0).owned_cols
      for (i <- 0 until cols.size) print(cols(i) + " ")
      println
    }
    else if (!df_pool.isEmpty) {
      val cols = df_pool(0).owned_cols
      for (i <- 0 until cols.size) print(cols(i) + " ")
      println
    }
  }

  def hit(e: FrovedisDataFrame): Unit = {
   val pool_idx = df_pool.indexOf(e)
   df_pool.remove(pool_idx)
   df_pool += e // remove and add in front

   // if e is already added to be freed, hit it to bring it front of the pool
   val free_idx = free_pool.indexOf(e)
   if (free_idx != -1) {
     free_pool.remove(free_idx)
     free_pool += e // and append last to mark it latest
   }
  }

  def show(): Unit = {
    println("table: ")
    table.map(x => println("[code: " + x._1._1 + "; col: " + x._1._2 + "] => proxy: " + x._2.get()))

    println("to_free: ")
    for(i <- 0 until free_pool.size) print(free_pool(i).get() + " ")
    println
  }
}

// for loading each target column as dvector one-by-one
object Dvec extends java.io.Serializable {
  def get(rddData: RDD[InternalRow], dtype: Short, i: Int,
          part_sizes: RDD[Int]): Long = {
    val t0 = new TimeSpent(Level.DEBUG)
    return dtype match {
        case DTYPE.BOOL => {
           val data = rddData.mapPartitions(x => x.map(y => if(y.isNullAt(i)) Int.MaxValue else y.getInt(i)))
           val ret = IntDvector.get(data, part_sizes)
           t0.show("get intDvector: ")
           ret
        }
        case DTYPE.INT => {
           val data = rddData.mapPartitions(x => x.map(y => if(y.isNullAt(i)) Int.MaxValue else y.getInt(i)))
           val ret = IntDvector.get(data, part_sizes)
           t0.show("get intDvector: ")
           ret
        }
        case DTYPE.LONG => {
           val data = rddData.mapPartitions(x => x.map(y => if(y.isNullAt(i)) Long.MaxValue else y.getLong(i)))
           val ret = LongDvector.get(data, part_sizes)
           t0.show("get longDvector: ")
           ret
        }
        case DTYPE.FLOAT => {
           val data = rddData.mapPartitions(x => x.map(y => if(y.isNullAt(i)) Float.MaxValue else y.getFloat(i)))
           val ret = FloatDvector.get(data, part_sizes)
           t0.show("get floatDvector: ")
           ret
        }
        case DTYPE.DOUBLE => {
           val data = rddData.mapPartitions(x => x.map(y => if(y.isNullAt(i)) Double.MaxValue else y.getDouble(i)))
           val ret = DoubleDvector.get(data, part_sizes)
           t0.show("get doubleDvector: ")
           ret
        }
        case DTYPE.STRING => {
           val data = rddData.mapPartitions(x => x.map(y => if(y.isNullAt(i)) "NULL" else y.getString(i)))
           val ret = StringDvector.get(data, part_sizes)
           t0.show("get stringDvector: ")
           ret
        }
        case DTYPE.WORDS => { // use instead of StringDvector for better performance
           val data = rddData.mapPartitions(x => x.map(y => if(y.isNullAt(i)) "NULL" else y.getString(i)))
           val ret = WordsNodeLocal.get(data, part_sizes)
           t0.show("get wordsNodeLocal: ")
           ret
        }
        case _ => throw new IllegalArgumentException("Unsupported type: " + TMAPPER.id2string(dtype))
    }
  }
}

// for loading entire dataframe at a time in optimized way
object sDFTransfer extends java.io.Serializable {

  def get_columnar(df: DataFrame): RDD[ColumnarBatch] = {
    val qe = new QueryExecution(df.sparkSession, df.queryExecution.logical) {
      override protected def preparations: Seq[Rule[SparkPlan]] = {
        Seq(
          PlanSubqueries(sparkSession),
          EnsureRequirements,               // dependency with spark >= 3.1.1
          CollapseCodegenStages()           // dependency with spark >= 3.1.1
        )
      }
    }
    var ret: RDD[ColumnarBatch] = null
    try {
      ret = qe.executedPlan.executeColumnar()
    } catch { // IllegalStateException etc...
      case e1: IllegalStateException => {ret = null} 
      case other: Throwable => {ret = null}
    }
    return ret
  }

  def load_columnar(columnar: RDD[ColumnarBatch],
                    cols: Array[String],
                    colIds: Array[Int],
                    types: Array[Short],
                    word_count: Int,
                    offset: Array[Int]): Long = {
    val ncol = cols.size
    val t_log = new TimeSpent(Level.DEBUG)

    // (1) allocate
    val fs = FrovedisServer.getServerInstance()
    val nproc = fs.worker_size
    val npart = columnar.getNumPartitions
    val block_sizes = GenericUtils.get_block_sizes(npart, nproc)
    val vptrs = JNISupport.allocateLocalVectors2(fs.master_node, block_sizes,
                                                 nproc, types, ncol) // vptrs: (ncol + no-of-words) x nproc
    var info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    t_log.show("server memory allocation: ")

    // (2) prepare server side ranks for processing N parallel requests
    JNISupport.lockParallel()
    var scale = ncol;
    if (configs.rawsend_enabled) scale = (ncol + word_count) * 2
    val fw_nodes = JNISupport.getWorkerInfoMulti(fs.master_node,
                                        block_sizes.map(_ * scale), nproc)
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    t_log.show("server process preparation: ")
                                                      
    // (3) data transfer
    transfer_columnar_by_batched_partition(columnar, colIds, fw_nodes, 
      vptrs, offset, types, block_sizes, ncol, nproc, word_count)
    t_log.show("[numpartition: " + npart + "] server side data transfer: ")
    JNISupport.unlockParallel()

    val fdata = JNISupport.createFrovedisDataframe2(fs.master_node, cols, types,
                                                    ncol, vptrs, vptrs.size)
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    t_log.show("dataframe creation: ")
    return fdata
  }

  def load_rows(rddData: RDD[InternalRow],
                cols: Array[String],
                types: Array[Short],
                word_count: Int,
                offset: Array[Int]): Long = {
    val ncol = cols.size
    val t_log = new TimeSpent(Level.DEBUG)

    // (1) allocate
    val fs = FrovedisServer.getServerInstance()
    val nproc = fs.worker_size
    val npart = rddData.getNumPartitions
    val block_sizes = GenericUtils.get_block_sizes(npart, nproc)
    val vptrs = JNISupport.allocateLocalVectors2(fs.master_node, block_sizes,
                                                 nproc, types, ncol) // vptrs: (ncol + no-of-words) x nproc
    var info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    t_log.show("server memory allocation: ")

    // (2) prepare server side ranks for processing N parallel requests
    JNISupport.lockParallel()
    val fw_nodes = JNISupport.getWorkerInfoMulti(fs.master_node,
                                        block_sizes.map(_ * ncol), nproc)
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    t_log.show("server process preparation: ")
                                                      
    // (3) data transfer
    execute(rddData, fw_nodes, vptrs, offset, types, block_sizes, ncol, nproc)
    t_log.show("[numpartition: " + npart + "] server side data transfer: ")
    JNISupport.unlockParallel()

    val fdata = JNISupport.createFrovedisDataframe2(fs.master_node, cols, types,
                                                    ncol, vptrs, vptrs.size)
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    t_log.show("dataframe creation: ")
    return fdata
  }

  def execute(rddData: RDD[InternalRow],
              fw_nodes: Array[Node],
              vptrs: Array[Long],
              offset: Array[Int],
              types: Array[Short],
              block_sizes: Array[Long],
              ncol: Int, nproc: Int): Unit = {
    if (rddData.first.isInstanceOf[UnsafeRow]) {
      //val jRddData = rddData.map(x => x.asInstanceOf[UnsafeRow]).toJavaRDD
      //jDFTransfer.execute(jRddData, fw_nodes, vptrs, types, block_sizes, ncol, nproc)
      transfer_unsafe_row(rddData, fw_nodes, vptrs, offset, 
                          types, block_sizes, ncol, nproc)
    } 
    else {
      transfer_internal_row(rddData, fw_nodes, vptrs, offset, 
                            types, block_sizes, ncol, nproc)
    }
  }

  private def transfer_columnar_by_batch(
              columnar: RDD[ColumnarBatch],
              colIds: Array[Int],
              fw_nodes: Array[Node],
              vptrs: Array[Long],
              offset: Array[Int],
              types: Array[Short],
              block_sizes: Array[Long],
              ncol: Int, nproc: Int): Unit = {
    val ret = columnar.zipWithIndex().map({ case(batch, index) =>
        val t0 = new TimeSpent(Level.TRACE)
        val (destId, myst) = GenericUtils.index2Dest(index, block_sizes)
        val localId = index - myst
        val w_node = fw_nodes(destId)
        jDFTransfer.transfer_batch_data(batch, colIds, w_node, vptrs, offset, types,
                                        ncol, nproc, destId, localId, t0)
        Array(true).toIterator
    })
    ret.count // for action
  }

  private def transfer_columnar_by_batched_partition(
              columnar: RDD[ColumnarBatch],
              colIds: Array[Int],
              fw_nodes: Array[Node],
              vptrs: Array[Long],
              offset: Array[Int],
              types: Array[Short],
              block_sizes: Array[Long],
              ncol: Int, nproc: Int,
              word_count: Int): Unit = {
    val ret = columnar.mapPartitionsWithIndex({ case(index, batches) =>
        val t0 = new TimeSpent(Level.TRACE)
        val (destId, myst) = GenericUtils.index2Dest(index, block_sizes)
        val localId = index - myst
        val w_node = fw_nodes(destId)

        val n_targets = ncol + word_count
        val bufs = new ArrayBuffer[Array[OffHeapArray]]()
        var tot_sizes = new Array[Int](n_targets)

        val t1 = new TimeSpent(Level.DEBUG)
        val t2 = new TimeSpent(Level.DEBUG)
        val alloc_t = new TimeSpent(Level.DEBUG)
        val copy_t = new TimeSpent(Level.DEBUG)
        val full_copy_t = new TimeSpent(Level.DEBUG)

        var cond = batches.hasNext
        while(cond) {
          val batch: ColumnarBatch = batches.next
          val out = jDFTransfer.copy_batch_data(batch, colIds, offset, types, 
                                                ncol, word_count, t0,
                                                alloc_t, copy_t)

          t1.lap_start()
          for(i <- 0 until n_targets) tot_sizes(i) += out(i).size()
          bufs += out
          t1.lap_stop()

          t2.lap_start()
          cond = batches.hasNext
          t2.lap_stop()
        }

        alloc_t.show_lap("[partition: " + index + "] a. memory allocation: ")
        copy_t.show_lap("[partition: " + index + "] b. copy to OffHeapArray: ")
        t1.show_lap("[partition: " + index + "] c. total batch-size calculation: ")
        t2.show_lap("[partition: " + index + "] d. has next: ")
        full_copy_t.show("[partition: " + index + "] (a + b + c + d + other): total batch-copy: ")

        val nbatches = bufs.size 
        for (i <- 0 until ncol) {
          val row_offset = offset(i)
          val vptr = vptrs(row_offset * nproc + destId)
          val tsize = tot_sizes(row_offset)
          types(i) match {
            case DTYPE.INT | DTYPE.BOOL => {
              t1.lap_start()
              var cur = 0
              val arr = new OffHeapArray(tsize, DTYPE.INT)
              for(j <- 0 until nbatches) {
                val tmp = bufs(j)(row_offset)
                arr.putInts(cur, tmp.size(), tmp.get(), 0)
                cur += tmp.size()
                tmp.freeMemory() 
              }
              t1.lap_stop()
              t0.show("int/bool buffer flattening: ")

              t2.lap_start()
              JNISupport.loadFrovedisWorkerTypedVector(w_node, vptr, localId, 
                        arr.get(), tsize, DTYPE.INT, configs.rawsend_enabled)
              arr.freeMemory()
              t2.lap_stop()
            }
            case DTYPE.LONG => {
              var cur = 0
              t1.lap_start()
              val arr = new OffHeapArray(tsize, DTYPE.LONG)
              for(j <- 0 until nbatches) {
                val tmp = bufs(j)(row_offset)
                arr.putLongs(cur, tmp.size(), tmp.get(), 0)
                cur += tmp.size()
                tmp.freeMemory() 
              }
              t1.lap_stop()
              t0.show("long buffer flattening: ")

              t2.lap_start()
              JNISupport.loadFrovedisWorkerTypedVector(w_node, vptr, localId,
                        arr.get(), tsize, DTYPE.LONG, configs.rawsend_enabled)
              arr.freeMemory()
              t2.lap_stop()
            }
            case DTYPE.FLOAT => {
              var cur = 0
              t1.lap_start()
              val arr = new OffHeapArray(tsize, DTYPE.FLOAT)
              for(j <- 0 until nbatches) {
                val tmp = bufs(j)(row_offset)
                arr.putFloats(cur, tmp.size(), tmp.get(), 0)
                cur += tmp.size()
                tmp.freeMemory()
              }
              t1.lap_stop()
              t0.show("float buffer flattening: ")

              t2.lap_start()
              JNISupport.loadFrovedisWorkerTypedVector(w_node, vptr, localId,
                        arr.get(), tsize, DTYPE.FLOAT, configs.rawsend_enabled)
              arr.freeMemory()
              t2.lap_stop()
            }
            case DTYPE.DOUBLE => {
              var cur = 0
              t1.lap_start()
              val arr = new OffHeapArray(tsize, DTYPE.DOUBLE)
              for(j <- 0 until nbatches) {
                val tmp = bufs(j)(row_offset)
                arr.putDoubles(cur, tmp.size(), tmp.get(), 0)
                cur += tmp.size()
                tmp.freeMemory()
              }
              t1.lap_stop()
              t0.show("double buffer flattening: ")

              t2.lap_start()
              JNISupport.loadFrovedisWorkerTypedVector(w_node, vptr, localId,
                        arr.get(), tsize, DTYPE.DOUBLE, configs.rawsend_enabled)
              arr.freeMemory()
              t2.lap_stop()
            }
            case DTYPE.WORDS => {
              t1.lap_start()
              var cur = 0
              var cur2 = 0
              val next_row_offset = row_offset + 1
              val tsize2 = tot_sizes(next_row_offset)
              val char_arr = new OffHeapArray(tsize, DTYPE.BYTE)
              val sz_arr = new OffHeapArray(tsize2, DTYPE.INT)
              for(j <- 0 until nbatches) {
                val tmp = bufs(j)(row_offset)
                char_arr.putBytes(cur, tmp.size(), tmp.get(), 0)
                cur += tmp.size()
                tmp.freeMemory()

                val tmp2 = bufs(j)(next_row_offset)
                sz_arr.putInts(cur2, tmp2.size(), tmp2.get(), 0)
                cur2 += tmp2.size()
                tmp2.freeMemory()
              }
              t1.lap_stop()
              t0.show("words buffer flattening: ")

              t2.lap_start()
              val sptr = vptrs(next_row_offset * nproc + destId)
              JNISupport.loadFrovedisWorkerByteSizePair2(w_node, vptr, sptr, localId, 
                char_arr.get(), sz_arr.get(), tsize, tsize2, configs.rawsend_enabled)
              char_arr.freeMemory()
              sz_arr.freeMemory()
              t2.lap_stop()
            }
            case _ => throw new IllegalArgumentException(
                      "[columnar_load] Unsupported type: " + TMAPPER.id2string(types(i)))
          }
          val err = JNISupport.checkServerException()
          if (err != "") throw new java.rmi.ServerException(err)
          t0.show("spark-worker to frovedis-rank local data copy: ")
        }

        t1.show_lap("[partition: " + index + "] batch data flattening: ")
        t2.show_lap("[partition: " + index + "] spark-to-frovedis data transfer: ")
        Array(true).toIterator
    })
    ret.count // for action
  }

  private def copy_local_word_data(
    index: Int, destId: Int,
    w_node: Node, vptr: Long, sptr: Long, localId: Long,
    data: ArrayBuffer[Any], size: Int): Unit = {

    val t0 = new TimeSpent(Level.TRACE)
    val sArr = new Array[Array[Char]](size)
    for(i <- 0 until size) sArr(i) = (data(i).asInstanceOf[String]).toCharArray
    t0.show("ArrayBuffer[String] to array-of-CharArray: ")

    val (flat_sArr, sizes_arr) = GenericUtils.flatten(sArr)
    t0.show("array-of-CharArray flatten: ")
    JNISupport.loadFrovedisWorkerCharSizePair(w_node, vptr, sptr, localId,
                                              flat_sArr, sizes_arr,
                                              flat_sArr.size, size)
    val err = JNISupport.checkServerException()
    if (err != "") throw new java.rmi.ServerException(err)
    t0.show("spark-worker to frovedis-rank local word data copy: ")
  }

  private def copy_local_data(
    index: Int, destId: Int,
    w_node: Node, vptr: Long, localId: Long,
    data: ArrayBuffer[Any], size: Int, dtype: Short): Unit = {

    //println("dest[" + destId + "::" + localId + "] partition-" +
    //       index + " of size: " + size + " is being copied!")

    val t0 = new TimeSpent(Level.TRACE)
    dtype match { // w_node::vptr[index] = data
        case DTYPE.INT | DTYPE.BOOL => {
          val iArr = data.asInstanceOf[ArrayBuffer[Int]].toArray
          t0.show("buffer to int-array: ")
          JNISupport.loadFrovedisWorkerIntVector(w_node, vptr, localId, iArr,
                                                 iArr.size)
        }
        case DTYPE.LONG => {
          val lArr = data.asInstanceOf[ArrayBuffer[Long]].toArray
          t0.show("buffer to long-array: ")
          JNISupport.loadFrovedisWorkerLongVector(w_node, vptr, localId, lArr,
                                                  lArr.size)
        }
        case DTYPE.FLOAT => {
          val fArr = data.asInstanceOf[ArrayBuffer[Float]].toArray
          t0.show("buffer to float-array: ")
          JNISupport.loadFrovedisWorkerFloatVector(w_node, vptr, localId, fArr,
                                                   fArr.size)
        }
        case DTYPE.DOUBLE => {
          val dArr = data.asInstanceOf[ArrayBuffer[Double]].toArray
          t0.show("buffer to double-array: ")
          JNISupport.loadFrovedisWorkerDoubleVector(w_node, vptr, localId, dArr,
                                                    dArr.size)
        }
        case DTYPE.STRING => {
          val sArr = data.asInstanceOf[ArrayBuffer[String]].toArray
          t0.show("buffer to string-array: ")
          JNISupport.loadFrovedisWorkerStringVector(w_node, vptr, localId, sArr,
                                                    sArr.size)
        }
        case _ => throw new IllegalArgumentException(
                  "[optimized_load] Unsupported type: " + TMAPPER.id2string(dtype))
    }
    val err = JNISupport.checkServerException()
    if (err != "") throw new java.rmi.ServerException(err)
    t0.show("spark-worker to frovedis-rank local data copy: ")
  }

  private def transfer_internal_row(
              rddData: RDD[InternalRow],
              fw_nodes: Array[Node],
              vptrs: Array[Long],
              offset: Array[Int],
              types: Array[Short],
              block_sizes: Array[Long],
              ncol: Int, nproc: Int): Long = {
    val ret = rddData.mapPartitionsWithIndex({ case(index, x) =>
        val t0 = new TimeSpent(Level.TRACE)
        val mat = new Array[ArrayBuffer[Any]](ncol)
        for(i <- 0 until ncol) mat(i) = new ArrayBuffer[Any]()
        var k: Int = 0
        while(x.hasNext) {
          val row: InternalRow = x.next
          for (i <- 0 until ncol) {
            val tmp = types(i) match {
              case DTYPE.INT    => if(row.isNullAt(i)) Int.MaxValue else row.getInt(i)
              case DTYPE.BOOL   => if(row.isNullAt(i)) Int.MaxValue else row.getInt(i)
              case DTYPE.LONG   => if(row.isNullAt(i)) Long.MaxValue else row.getLong(i)
              case DTYPE.FLOAT  => if(row.isNullAt(i)) Float.MaxValue else row.getFloat(i)
              case DTYPE.DOUBLE => if(row.isNullAt(i)) Double.MaxValue else row.getDouble(i)
              case DTYPE.STRING => if(row.isNullAt(i)) "NULL" else row.getString(i)
              case DTYPE.WORDS  => if(row.isNullAt(i)) "NULL" else row.getString(i)
              case _ => throw new IllegalArgumentException(
                        "[optimized_load] Unsupported type: " + TMAPPER.id2string(types(i)))
            }
            mat(i) += tmp
          }
          k += 1
        }
        t0.show("to matrix buffer: ")

        val (destId, myst) = GenericUtils.index2Dest(index, block_sizes)
        val localId = index - myst

        for (i <- 0 until ncol) {
          val row_offset = offset(i)
          val vptr = vptrs(row_offset * nproc + destId)
          if (types(i) != DTYPE.WORDS) {
            copy_local_data(index, destId, fw_nodes(destId),
                            vptr, localId, mat(i), k, types(i))
          } else {
            val next_row_offset = row_offset + 1
            val sptr = vptrs(next_row_offset * nproc + destId)
            copy_local_word_data(index, destId, fw_nodes(destId),
                                 vptr, sptr, localId, mat(i), k)
          }
        }
        t0.show("data transfer: ")
        Array(true).toIterator // SUCCESS (since need to return an iterator)
    })
    return ret.count // for action
  }

  private def getStringsAsFlattenCharArray(baseObject: ArrayBuffer[Array[Byte]],
                                   baseOffset: ArrayBuffer[Long],
                                   numFields: Int,
                                   ordinal: Int,
                                   sizes: Array[Int]): Array[Char] = {
    require(ordinal >= 0 && ordinal < numFields, "ordinal: out-of-bound!\n")
    val size = baseObject.size
    var flatten_size = 0
    for (i <- 0 until size) flatten_size += sizes(i)

    var cur = 0;
    val ret = new Array[Char](flatten_size)
    for (i <- 0 until size) {
      val data = baseObject(i)
      if (jPlatform.isNullAt(data, baseOffset(i), numFields, ordinal)) {
        ret(cur + 0) = 'N'
        ret(cur + 1) = 'U'
        ret(cur + 2) = 'L'
        ret(cur + 3) = 'L'
        cur += 4
      }
      else {
        val offsetAndSize = jPlatform.getLong(data, baseOffset(i), numFields, ordinal)
        val offset = (offsetAndSize >> 32).asInstanceOf[Int]
        val str_size = (offsetAndSize).asInstanceOf[Int]
        for(j <- 0 until str_size) ret(cur + j) = data(offset + j).asInstanceOf[Char];
        cur += str_size;
      }
    }
    return ret
  }

  private def transfer_unsafe_row(
              rddData: RDD[InternalRow], // actually UnsafeRow
              fw_nodes: Array[Node],
              vptrs: Array[Long],
              offset: Array[Int],
              types: Array[Short],
              block_sizes: Array[Long],
              ncol: Int, nproc: Int): Long = {
    val ret = rddData.mapPartitionsWithIndex({ case(index, x) =>
        val t0 = new TimeSpent(Level.TRACE)
        var k = 0
        val obj = new ArrayBuffer[Array[Byte]]()
        val off = new ArrayBuffer[Long]()
        while(x.hasNext) {
          val row = x.next.asInstanceOf[UnsafeRow]
          obj += row.getBytes()       // row.getBaseObject()
          off += row.getBaseOffset()
          k += 1
        }
        t0.show("baseObject, baseOffset extraction: ")

        val (destId, myst) = GenericUtils.index2Dest(index, block_sizes)
        val localId = index - myst
        val w_node = fw_nodes(destId)

        for (i <- 0 until ncol) {
          val row_offset = offset(i)
          val vptr = vptrs(row_offset * nproc + destId)
          types(i) match {
            case DTYPE.INT | DTYPE.BOOL => {
              val iArr = new Array[Int](k)
              for (j <- 0 until k) iArr(j) = jPlatform.getInt(obj(j), off(j), ncol, i)
              t0.show("buffer to int-array: ")
              JNISupport.loadFrovedisWorkerIntVector(w_node, vptr, localId, iArr, k)
            }
            case DTYPE.LONG | DTYPE.DATETIME | DTYPE.TIMESTAMP => {
              val lArr = new Array[Long](k)
              for (j <- 0 until k) lArr(j) = jPlatform.getLong(obj(j), off(j), ncol, i)
              t0.show("buffer to long-array: ")
              JNISupport.loadFrovedisWorkerLongVector(w_node, vptr, localId, lArr, k)
            }
            case DTYPE.FLOAT => {
              val fArr = new Array[Float](k)
              for (j <- 0 until k) fArr(j) = jPlatform.getFloat(obj(j), off(j), ncol, i)
              t0.show("buffer to float-array: ")
              JNISupport.loadFrovedisWorkerFloatVector(w_node, vptr, localId, fArr, k)
            }
            case DTYPE.DOUBLE => {
              val dArr = new Array[Double](k)
              for (j <- 0 until k) dArr(j) = jPlatform.getDouble(obj(j), off(j), ncol, i)
              t0.show("buffer to double-array: ")
              JNISupport.loadFrovedisWorkerDoubleVector(w_node, vptr, localId, dArr, k)
            }
            case DTYPE.STRING => {
              val sArr = new Array[String](k)
              for (j <- 0 until k) sArr(j) = jPlatform.getString(obj(j), off(j), ncol, i)
              t0.show("buffer to string-array: ")
              JNISupport.loadFrovedisWorkerStringVector(w_node, vptr, localId, sArr, k)
            }
            case DTYPE.WORDS => {
              val szArr = new Array[Int](k)
              for (j <- 0 until k) szArr(j) = jPlatform.getStringSize(obj(j), off(j), ncol, i)
              val cArr = getStringsAsFlattenCharArray(obj, off, ncol, i, szArr)
              t0.show("buffer to array of char-size pair (words): ")
              val next_row_offset = row_offset + 1 // size is stored in next row of vptrs matrix
              val sptr = vptrs(next_row_offset * nproc + destId)
              JNISupport.loadFrovedisWorkerCharSizePair(w_node, vptr, sptr, localId,
                                                        cArr, szArr, cArr.size, k)
            }
            case _ => throw new IllegalArgumentException(
                      "[optimized_load] Unsupported type: " + TMAPPER.id2string(types(i)))
          }
          val err = JNISupport.checkServerException()
          if (err != "") throw new java.rmi.ServerException(err)
          t0.show("spark-worker to frovedis-rank local data copy: ")
        }
        Array(true).toIterator
    })
    ret.count // for action
  }

}


