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
import com.nec.frovedis.Jmatrix.FlexibleOffHeapArray
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
        case DTYPE.LONG | DTYPE.DATETIME | DTYPE.TIMESTAMP => {
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
    var npart = columnar.getNumPartitions
    if (configs.transfer_by_batch) npart = columnar.count.toInt
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
    if (configs.transfer_by_batch) {
      transfer_columnar_by_batch(columnar, colIds, fw_nodes, 
        vptrs, offset, types, block_sizes, ncol, nproc)
      t_log.show("[numbatch: " + npart + "] server side data transfer: ")
    } else {
      transfer_columnar_by_batched_partition(columnar, colIds, fw_nodes, 
        vptrs, offset, types, block_sizes, ncol, nproc, word_count)
      t_log.show("[numpartition: " + npart + "] server side data transfer: ")
    }
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
    var scale = ncol;
    if (configs.rawsend_enabled) scale = (ncol + word_count) * 2
    val fw_nodes = JNISupport.getWorkerInfoMulti(fs.master_node,
                                        block_sizes.map(_ * scale), nproc)
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    t_log.show("server process preparation: ")
                                                      
    // (3) data transfer
    transfer_rows_by_partition(rddData, fw_nodes, vptrs, offset, types, 
                               block_sizes, ncol, nproc, word_count)
    t_log.show("[numpartition: " + npart + "] server side data transfer: ")
    JNISupport.unlockParallel()

    val fdata = JNISupport.createFrovedisDataframe2(fs.master_node, cols, types,
                                                    ncol, vptrs, vptrs.size)
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    t_log.show("dataframe creation: ")
    return fdata
  }

  def transfer_rows_by_partition(
              rddData: RDD[InternalRow],
              fw_nodes: Array[Node],
              vptrs: Array[Long],
              offset: Array[Int],
              types: Array[Short],
              block_sizes: Array[Long],
              ncol: Int, nproc: Int,
              word_count: Int): Unit = {
    if (rddData.first.isInstanceOf[UnsafeRow]) {
      //val jRddData = rddData.map(x => x.asInstanceOf[UnsafeRow]).toJavaRDD
      //jDFTransfer.execute(jRddData, fw_nodes, vptrs, types, block_sizes, ncol, nproc)
      transfer_unsafe_row(rddData, fw_nodes, vptrs, offset, 
                          types, block_sizes, ncol, nproc, word_count)
    } 
    else {
      transfer_internal_row(rddData, fw_nodes, vptrs, offset, 
                            types, block_sizes, ncol, nproc, word_count)
    }
  }

  // transfer data batch-by-batch 
  //   -> pros: limited memory used for OffHeapArray
  //   -> cons: too many exrpc calls (no. of batches times) would be involved, so very slow performance
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

  // transfer data partition-by-partition (combining batches in each partition)
  //   -> pros: very less exrpc calls (no. of partitions times) would be involved
  //   -> cons: large memory used for FlexibleOffHeapArray
  // due to performance benefit, this version is currently 
  // used as default (configs.transfer_by_batch = false)...
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
        val t1 = new TimeSpent(Level.DEBUG)
        val t2 = new TimeSpent(Level.DEBUG)
        val copy_t = new TimeSpent(Level.DEBUG)
        val t0 = new TimeSpent(Level.TRACE)

        val (destId, myst) = GenericUtils.index2Dest(index, block_sizes)
        val localId = index - myst
        val w_node = fw_nodes(destId)

        val n_targets = ncol + word_count
        val out = new Array[FlexibleOffHeapArray](n_targets)
        val buf_size = configs.get_default_buffer_size();
        val str_size = configs.get_default_string_size();
        for (i <- 0 until ncol) {
          val row_offset = offset(i)
          types(i) match {
            case DTYPE.INT | 
                 DTYPE.BOOL   => out(row_offset) = new FlexibleOffHeapArray(buf_size, DTYPE.INT) 
            case DTYPE.LONG   => out(row_offset) = new FlexibleOffHeapArray(buf_size, DTYPE.LONG) 
            case DTYPE.FLOAT  => out(row_offset) = new FlexibleOffHeapArray(buf_size, DTYPE.FLOAT) 
            case DTYPE.DOUBLE => out(row_offset) = new FlexibleOffHeapArray(buf_size, DTYPE.DOUBLE) 
            case DTYPE.STRING => {
              throw new IllegalArgumentException(
              "[transfer_columnar_by_batched_partition] 'String' type is not supported!")
            }
            case DTYPE.WORDS  => { 
              out(row_offset) = new FlexibleOffHeapArray(buf_size * str_size, DTYPE.BYTE) 
              out(row_offset + 1) = new FlexibleOffHeapArray(buf_size, DTYPE.INT) 
            }
            case _ => throw new IllegalArgumentException(
                      "[transfer_columnar_by_batched_partition] Unsupported type: " + TMAPPER.id2string(types(i)))
          }          
        }
        t1.show("[partition: " + index + "] a. memory allocation: ")

        val full_copy_t = new TimeSpent(Level.DEBUG)
        var cond = batches.hasNext
        while(cond) {
          val batch: ColumnarBatch = batches.next
          jDFTransfer.copy_batch_data(batch, colIds, offset, types, 
                                      ncol, out, t0, copy_t)
          t1.lap_start()
          cond = batches.hasNext
          t1.lap_stop()
        }
        copy_t.show_lap("[partition: " + index + "] b. copy to FlexibleOffHeapArray: ")
        t1.show_lap("[partition: " + index + "] c. has next: ")
        full_copy_t.show("[partition: " + index + "] (a + b + c + other): total batch-copy: ")

        for (i <- 0 until ncol) {
          val row_offset = offset(i)
          val vptr = vptrs(row_offset * nproc + destId)
          val arr = out(row_offset).getFlattenMemory()
          types(i) match {
            case DTYPE.INT | DTYPE.BOOL => {
              t1.lap_start()
              JNISupport.loadFrovedisWorkerTypedVector(w_node, vptr, localId, 
                        arr.get(), arr.get_active_length(), DTYPE.INT, 
                        configs.rawsend_enabled)
              t1.lap_stop()
            }
            case DTYPE.LONG => {
              t1.lap_start()
              JNISupport.loadFrovedisWorkerTypedVector(w_node, vptr, localId,
                        arr.get(), arr.get_active_length(), DTYPE.LONG, 
                        configs.rawsend_enabled)
              t1.lap_stop()
            }
            case DTYPE.FLOAT => {
              t1.lap_start()
              JNISupport.loadFrovedisWorkerTypedVector(w_node, vptr, localId,
                        arr.get(), arr.get_active_length(), DTYPE.FLOAT, 
                        configs.rawsend_enabled)
              t1.lap_stop()
            }
            case DTYPE.DOUBLE => {
              t1.lap_start()
              JNISupport.loadFrovedisWorkerTypedVector(w_node, vptr, localId,
                        arr.get(), arr.get_active_length(), DTYPE.DOUBLE, 
                        configs.rawsend_enabled)
              t1.lap_stop()
            }
            case DTYPE.STRING => {
              throw new IllegalArgumentException(
              "[transfer_columnar_by_batched_partition] 'String' type is not supported!")
            }
            case DTYPE.WORDS => {
              val next_row_offset = row_offset + 1
              val sptr = vptrs(next_row_offset * nproc + destId)
              val arr2 = out(next_row_offset).getFlattenMemory()
              t1.lap_start()
              JNISupport.loadFrovedisWorkerByteSizePair2(
                w_node, vptr, sptr, localId, 
                arr.get(), arr2.get(), 
                arr.get_active_length(), arr2.get_active_length(), 
                configs.rawsend_enabled)
              t1.lap_stop()
              out(next_row_offset).freeMemory()
            }
            case _ => throw new IllegalArgumentException(
                      "[transfer_columnar_by_batched_partition] Unsupported type: " + TMAPPER.id2string(types(i)))
          }
          out(row_offset).freeMemory()
          val err = JNISupport.checkServerException()
          if (err != "") throw new java.rmi.ServerException(err)
          t0.show("spark-worker to frovedis-rank local data copy: ")
        }
        t1.show_lap("[partition: " + index + "] spark-to-frovedis data transfer: ")
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
                  "[load_rows] Unsupported type: " + TMAPPER.id2string(dtype))
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
              ncol: Int, nproc: Int,
              word_count: Int): Long = {
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
              case DTYPE.LONG | DTYPE.DATETIME | DTYPE.TIMESTAMP  =>
                                   if(row.isNullAt(i)) Long.MaxValue else row.getLong(i)
              case DTYPE.FLOAT  => if(row.isNullAt(i)) Float.MaxValue else row.getFloat(i)
              case DTYPE.DOUBLE => if(row.isNullAt(i)) Double.MaxValue else row.getDouble(i)
              case DTYPE.STRING => if(row.isNullAt(i)) "NULL" else row.getString(i)
              case DTYPE.WORDS  => if(row.isNullAt(i)) "NULL" else row.getString(i)
              case _ => throw new IllegalArgumentException(
                        "[load_rows] Unsupported type: " + TMAPPER.id2string(types(i)))
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
              ncol: Int, nproc: Int,
              word_count: Int): Long = {
    val ret = rddData.mapPartitionsWithIndex({ case(index, x) =>

        val (destId, myst) = GenericUtils.index2Dest(index, block_sizes)
        val localId = index - myst
        val w_node = fw_nodes(destId)

        val n_targets = ncol + word_count
        val out = new Array[FlexibleOffHeapArray](n_targets)
        val buf_size = configs.get_default_buffer_size();
        val str_size = configs.get_default_string_size();
        val t1 = new TimeSpent(Level.DEBUG)

        for (i <- 0 until ncol) {
          val row_offset = offset(i)
          types(i) match {
            case DTYPE.INT | DTYPE.BOOL => {
              out(row_offset) = new FlexibleOffHeapArray(buf_size, DTYPE.INT)
            }
            case DTYPE.LONG | DTYPE.DATETIME | DTYPE.TIMESTAMP => {
              out(row_offset) = new FlexibleOffHeapArray(buf_size, DTYPE.LONG)
            }
            case DTYPE.FLOAT  => out(row_offset) = new FlexibleOffHeapArray(buf_size, DTYPE.FLOAT)
            case DTYPE.DOUBLE => out(row_offset) = new FlexibleOffHeapArray(buf_size, DTYPE.DOUBLE)
            case DTYPE.STRING => {
              throw new IllegalArgumentException(
              "[transfer_unsafe_row] 'String' type is not supported!")
            }
            case DTYPE.WORDS  => {
              out(row_offset) = new FlexibleOffHeapArray(buf_size * str_size, DTYPE.BYTE)
              out(row_offset + 1) = new FlexibleOffHeapArray(buf_size, DTYPE.INT)
            }
            case _ => throw new IllegalArgumentException(
                      "[transfer_unsafe_row] Unsupported type: " + TMAPPER.id2string(types(i)))
          }
        }
        t1.show("[partition: " + index + "] a. memory allocation: ")

        val copy_t = new TimeSpent(Level.DEBUG)
        val full_copy_t = new TimeSpent(Level.DEBUG)
        var cond = x.hasNext
        val nullstr: Array[Byte] = Array('N', 'U', 'L', 'L')

        while(cond) {
          val row = x.next.asInstanceOf[UnsafeRow]

          copy_t.lap_start()
          for (i <- 0 until ncol) {
            val row_offset = offset(i)
            val isnull = row.isNullAt(i)
            types(i) match {
              case DTYPE.INT | DTYPE.BOOL   => {
                val x = if (isnull) Int.MaxValue else row.getInt(i)
                out(row_offset).putInt(x)
              }
              case DTYPE.LONG | DTYPE.DATETIME | DTYPE.TIMESTAMP => {
                val x = if (isnull) Long.MaxValue else row.getLong(i)
                out(row_offset).putLong(x)
              }
              case DTYPE.FLOAT  => {
                val x = if (isnull) Float.MaxValue else row.getFloat(i)
                out(row_offset).putFloat(x)
              }
              case DTYPE.DOUBLE => {
                val x = if (isnull) Double.MaxValue else row.getDouble(i)
                out(row_offset).putDouble(x)
              }
              case DTYPE.STRING   => {
                throw new IllegalArgumentException(
                "[transfer_unsafe_row] 'String' type is not supported!")
              }
              case DTYPE.WORDS   => {
                val x = if (isnull) nullstr else row.getBinary(i)
                out(row_offset).putBytes(x)
                out(row_offset + 1).putInt(x.size)
              }
              case _ => throw new IllegalArgumentException(
                        "[transfer_unsafe_row] Unsupported type: " + TMAPPER.id2string(types(i)))
            }
          }
          copy_t.lap_stop()

          t1.lap_start()
          cond = x.hasNext
          t1.lap_stop()
        }
        copy_t.show_lap("[partition: " + index + "] b. copy to FlexibleOffHeapArray: ")
        t1.show_lap("[partition: " + index + "] c. has next: ")
        full_copy_t.show("[partition: " + index + "] (a + b + c + other): total batch-copy: ")

        for (i <- 0 until ncol) {
          val row_offset = offset(i)
          val vptr = vptrs(row_offset * nproc + destId)
          val arr = out(row_offset).getFlattenMemory()
          types(i) match {
            case DTYPE.INT | DTYPE.BOOL => {
              t1.lap_start()
              JNISupport.loadFrovedisWorkerTypedVector(w_node, vptr, localId,
                        arr.get(), arr.get_active_length(), DTYPE.INT,
                        configs.rawsend_enabled)
              t1.lap_stop()
            }
            case DTYPE.LONG | DTYPE.DATETIME | DTYPE.TIMESTAMP => {
              t1.lap_start()
              JNISupport.loadFrovedisWorkerTypedVector(w_node, vptr, localId,
                        arr.get(), arr.get_active_length(), DTYPE.LONG,
                        configs.rawsend_enabled)
              t1.lap_stop()
            }
            case DTYPE.FLOAT => {
              t1.lap_start()
              JNISupport.loadFrovedisWorkerTypedVector(w_node, vptr, localId,
                        arr.get(), arr.get_active_length(), DTYPE.FLOAT,
                        configs.rawsend_enabled)
              t1.lap_stop()
            }
            case DTYPE.DOUBLE => {
              t1.lap_start()
              JNISupport.loadFrovedisWorkerTypedVector(w_node, vptr, localId,
                        arr.get(), arr.get_active_length(), DTYPE.DOUBLE,
                        configs.rawsend_enabled)
              t1.lap_stop()
            }
            case DTYPE.STRING   => {
              throw new IllegalArgumentException(
              "[transfer_unsafe_row] 'String' type is not supported!")
            }
            case DTYPE.WORDS => {
              val next_row_offset = row_offset + 1
              val sptr = vptrs(next_row_offset * nproc + destId)
              val arr2 = out(next_row_offset).getFlattenMemory()
              t1.lap_start()
              JNISupport.loadFrovedisWorkerByteSizePair2(
                w_node, vptr, sptr, localId,
                arr.get(), arr2.get(),
                arr.get_active_length(), arr2.get_active_length(),
                configs.rawsend_enabled)
              t1.lap_stop()
              out(next_row_offset).freeMemory()
            }
            case _ => throw new IllegalArgumentException(
                      "[transfer_unsafe_row] Unsupported type: " + TMAPPER.id2string(types(i)))
          }
          out(row_offset).freeMemory()
          val err = JNISupport.checkServerException()
          if (err != "") throw new java.rmi.ServerException(err)
        }
        t1.show_lap("[partition: " + index + "] spark-to-frovedis data transfer: ")
        Array(true).toIterator
    })
    ret.count // for action
  }

}
