package com.nec.frovedis.sql;

import com.nec.frovedis.Jexrpc._
import com.nec.frovedis.Jmllib.DummyDftable
import com.nec.frovedis.Jsql.{jPlatform, jDFTransfer}
import com.nec.frovedis.matrix.{GenericUtils, TimeSpent, DTYPE}
import com.nec.frovedis.matrix.{
  IntDvector, LongDvector, 
  FloatDvector, DoubleDvector,
  StringDvector, WordsNodeLocal,
  BoolDvector}
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.matrix.FrovedisColmajorMatrix
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.mllib.ModelID
import org.apache.spark.rdd.RDD
import org.apache.spark.SparkContext
import org.apache.spark.mllib.linalg.{Vector,Vectors}
import org.apache.spark.sql.SparkSession
import org.apache.spark.sql.Row
import org.apache.spark.sql.types._
import org.apache.spark.sql.functions.{col => sp_col}
import org.apache.spark.sql.DataFrame
import org.apache.spark.sql.catalyst.InternalRow
import org.apache.spark.sql.catalyst.expressions.UnsafeRow
import scala.collection.mutable.{Map => mMap}
import scala.collection.mutable.ArrayBuffer
import scala.collection.mutable.ListBuffer
import org.apache.log4j.{Level, Logger}

object DataManager extends java.io.Serializable { 
  val df_pool = new ListBuffer[FrovedisDataFrame]()
  val free_pool = new ListBuffer[FrovedisDataFrame]()
  val table = mMap[(Int, String), FrovedisDataFrame]() // (code, colName) -> fdf

  def insert(code: Int,
             fdf: FrovedisDataFrame,
             cols: Iterator[String]): Unit = {
    while(cols.hasNext) table((code, cols.next)) = fdf
    df_pool += fdf
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
      e.release_impl()
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

object Dvec {
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

object sDFTransfer extends java.io.Serializable {
  def copy_local_word_data(
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

  def copy_local_data(
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

  def execute(rddData: RDD[InternalRow],
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
}

object _jDFTransfer extends java.io.Serializable {

  def getStringsAsFlattenCharArray(baseObject: ArrayBuffer[Array[Byte]],
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

  def execute(rddData: RDD[InternalRow], // actually UnsafeRow
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
            case DTYPE.LONG => {
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

class FrovedisDataFrame extends java.io.Serializable {
  protected var fdata: Long = -1
  protected var cols: Array[String] = null
  protected var types: Array[Short] = null
  private var code: Int = 0
  var owned_cols: Array[String] = null
  private var removable = false

  // creating a frovedis dataframe from a spark dataframe -> User API
  def this(df: DataFrame) = {
    this()
    //load(df)
    //this.code = df.hashCode
    val cols = df.columns
    const_impl(df, cols)
  }
  def this(df: DataFrame, name: String, others: String*) = {
    this()
    val cols = (Array(name) ++ others).toArray
    const_impl(df, cols)
  }
  // internally used, not exposed to user
  def this(proxy: Long, cc: Array[String], tt: Array[Short]) = {
    this()
    fdata = proxy
    cols = cc.clone()
    types = tt.clone()
  }
  // internally used, not exposed to user
  def this(dummy: DummyDftable) = {
    this()
    fdata = dummy.dfptr
    cols = dummy.names.clone()
    types = dummy.types.clone()
    // casting ULONG -> LONG (since spark doesn't support Unsigned Long)
    val ulong_cols = new ArrayBuffer[String]()
    for (i <- 0 until types.size) {
      if (types(i) == DTYPE.ULONG) {
        ulong_cols += cols(i)        
        types(i) = DTYPE.LONG
      }
    }
    val ulong_sz = ulong_cols.size
    if (ulong_sz > 0) {
      val ctypes = new Array[Short](ulong_sz)
      for (i <- 0 until ulong_sz) ctypes(i) = DTYPE.LONG
      val fs = FrovedisServer.getServerInstance()
      val ret = JNISupport.castFrovedisDataframe(fs.master_node, fdata, 
                                       ulong_cols.toArray, ctypes, ulong_sz)
      fdata = ret.dfptr
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
    }
  }
  private def const_impl(df: DataFrame, cols: Array[String]): Unit = {
    val code = df.hashCode // assumed to be unique per spark dataframe object
    val base_ptr = new ArrayBuffer[Long]()
    val base_col = new ArrayBuffer[String]()
    val targets = new ArrayBuffer[String]()
    for (i <- 0 until cols.size) {
      val key = (code, cols(i))
      val fdf = DataManager.get(key)
      if (fdf == null) targets += cols(i) 
      else {
        if (fdf.get() != -1) { // not released forcibly
          base_ptr += fdf.get()
          base_col += cols(i)
        }
        else {
          targets += cols(i)
        }
      }
    }
    if (!targets.isEmpty) {
      val tarr = targets.toArray
      this.owned_cols = tarr
      val sdf = df.select(tarr.map(x => sp_col(x)):_*)
      optimized_load(sdf)
      DataManager.insert(code, this, tarr.toIterator)
    }
    else {
      this.removable = true; // since, it is just a copy of shared pointers
    }
    if (!base_ptr.isEmpty) { // TODO: correct order
      //println("*** cols hit ***")
      //base_col.foreach(println)
      val fs = FrovedisServer.getServerInstance()
      val dummy = JNISupport.copyColumn(fs.master_node, fdata, 
                                        base_ptr.toArray, 
                                        base_col.toArray,
                                        base_ptr.size)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      this.fdata = dummy.dfptr
      this.cols = dummy.names.clone()
      this.types = dummy.types.clone() // TODO: mark bool/ulong
    }
    this.code = df.hashCode
  }
  def is_removable() = removable

  // to release a frovedis dataframe from frovedis server -> User API
  def release () : Unit = {
    if (fdata != -1) {
      if(removable) release_impl()
      else          DataManager.add_release_entry(this)
    }
  }
  def release_impl () : Unit = {
    if (fdata != -1) {
      val fs = FrovedisServer.getServerInstance()
      JNISupport.releaseFrovedisDataframe(fs.master_node,fdata)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      fdata = -1
      cols = null
      types = null
      owned_cols = null
      code = 0
    }
  }
  // to display contents of a frovedis dataframe from frovedis server -> User API
  def show () : Unit = {
    if (fdata != -1) {
      //println("proxy: " + fdata)
      val fs = FrovedisServer.getServerInstance()
      JNISupport.showFrovedisDataframe(fs.master_node,fdata)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      println
    }
  }
  def show (truncate: Boolean = true) : Unit = show()

  def load (df: DataFrame) : Unit = {
    release()
    val name_type_pair = df.dtypes
    cols = name_type_pair.map(_._1)
    val tt = name_type_pair.map(_._2)
    val size = cols.size
    val dvecs = new Array[Long](size)
    types = new Array[Short](size)
    val rddData = df.queryExecution.toRdd
    val part_sizes = rddData.mapPartitions(x => Array(x.size).toIterator).persist
    for (i <- 0 to (size-1)) {
      //print("col_name: " + cols(i) + " col_type: " + tt(i) + "\n")
      val tname = tt(i)
      val dtype = TMAPPER.string2id(tname)
      types(i) = dtype
      dvecs(i) = Dvec.get(rddData, dtype, i, part_sizes)
    }
    val fs = FrovedisServer.getServerInstance()
    fdata = JNISupport.createFrovedisDataframe(fs.master_node,types,cols,dvecs,size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
  }
  // for loading specific columns; 
  // TODO: improve existing limitations: 
  //   - might cause bad-alloc in ve-memory when target columns are too large to fit
  def optimized_load (df: DataFrame) : Unit = {
    release()
    val name_type_pair = df.dtypes
    cols = name_type_pair.map(_._1)
    types = name_type_pair.map(_._2).map(x => TMAPPER.string2id(x))
    val ncol = cols.size
    val rddData = df.queryExecution.toRdd
    val offset = new Array[Int](ncol)
    var word_count = 0
    for (i <- 0 until ncol) {
      offset(i) = i + word_count
      word_count += (if (types(i) == DTYPE.WORDS) 1 else 0)
    }
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
    val fw_nodes = JNISupport.getWorkerInfoMulti(fs.master_node,
                                        block_sizes.map(_ * ncol), nproc)
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    t_log.show("server process preparation: ")
                                                      
    // (3) data transfer
    if (rddData.first.isInstanceOf[UnsafeRow]) {
      //val jRddData = rddData.map(x => x.asInstanceOf[UnsafeRow]).toJavaRDD
      //jDFTransfer.execute(jRddData, fw_nodes, vptrs, types, block_sizes, ncol, nproc)
      _jDFTransfer.execute(rddData, fw_nodes, vptrs, offset, types, block_sizes, ncol, nproc)
    } else {
      sDFTransfer.execute(rddData, fw_nodes, vptrs, offset, types, block_sizes, ncol, nproc)
    }
    t_log.show("[numpartition: " + npart + "] server side data transfer: ")

    fdata = JNISupport.createFrovedisDataframe2(fs.master_node, cols, types,
                                                ncol, vptrs, vptrs.size)
    info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    t_log.show("dataframe creation: ")
  }

  // --- Frovedis server side FILTER definition here ---
  // Usage: df.filter(df.col("colA") > 10)
  // Usage: df.filter(df("colA") > 10)
  // Usage: df.filter($$"colA" > 10)
  def filter(opt: FrovedisColumn): FrovedisDataFrame = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.filterFrovedisDataframe(fs.master_node,this.get(),opt.get())
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = new FrovedisDataFrame(proxy, cols, types)
    return ret
  }

  // alias for filter 
  def where(opt: FrovedisColumn) = filter(opt)

  // --- Frovedis server side SELECT definition here ---
  // Usage: df.select(Array("colA","colB"))
  def select(targets: Array[String]): FrovedisDataFrame = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    val size = targets.size
    val tt = getColumnTypes(targets) // throws exception, if colname not found
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.selectFrovedisDataframe(fs.master_node,get(),targets,size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisDataFrame(proxy,targets,tt)
  }

  // Usage: df.select("colA")
  // Usage: df.select("colA", "colB") ... any number of column names
  def select(must: String, optional: String*): FrovedisDataFrame = {
    val all = (Array(must) ++ optional).toArray.map(x => x.toString)
    return select(all)
  }

  // Usage: df.select($$"colA", $$"colB" + 1)
  def select(c: FrovedisColumn*): FrovedisDataFrame = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    val c_arr = c.toArray
    val size = c_arr.size
    val funcs = new Array[Long](size)
    val bool_cols_id = new ArrayBuffer[Int]()
    var aggcnt = 0
    for (i <- 0 until size) { 
      val tmp = c_arr(i)
      funcs(i) = tmp.get()
      if (tmp.isBOOL) bool_cols_id += i
      aggcnt = aggcnt + (if (tmp.isAGG) 1 else 0)
    }
    val fs = FrovedisServer.getServerInstance()
    var dummy: DummyDftable = null
    if (aggcnt > 0) {
      if (aggcnt != size)
        throw new IllegalArgumentException(
        "select: few non-aggregator functions are detected!")
      dummy = JNISupport.executeFrovedisAgg(fs.master_node, get(), funcs, size)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
    } else {
      dummy = JNISupport.fselectFrovedisDataframe(fs.master_node, get(),
                                                  funcs, size)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      // update types for bool columns
      for (i <- 0 until bool_cols_id.size) dummy.types(bool_cols_id(i)) = DTYPE.BOOL 
    }
    return new FrovedisDataFrame(dummy)
  } 

  private def sort_impl(targets: Array[String], 
                        isDescArr: Array[Int]): FrovedisDataFrame = {
    val size = targets.size
    for (i <- 0 until size) { 
      if(!hasColumn(targets(i))) 
        throw new IllegalArgumentException("No column named: " + targets(i)) 
    }
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.sortFrovedisDataframe(fs.master_node,get(),
                                                 targets,isDescArr,size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisDataFrame(proxy, cols, types)
  }

  def sort(c: FrovedisColumn*): FrovedisDataFrame = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    val all = c.toArray
    for (i <- 0 until all.size) {
      if (!all(i).isID) throw new java.lang.UnsupportedOperationException(
      s"Currently frovedis supports sort/orderBy based on only existing columns!")
    }
    val targets = all.map(x => x.toString)
    val isDescArr = all.map(x => x.getIsDesc)
    return sort_impl(targets, isDescArr)
  }
  
  def sort(must: String, optional: String*): FrovedisDataFrame = {
    val all = (Array(must) ++ optional).toArray.map(x => new FrovedisColumn(x))
    return sort(all:_*)
  }

  // alias of sort
  def orderBy(must: String, optional: String*) = sort(must, optional : _*)
  def orderBy(c: FrovedisColumn*) = sort(c : _*)
  
  // --- Frovedis server side GROUP_BY definition here ---
  // Usage: df.groupBy(Array("colA", "colB")) 
  def groupBy(targets: Array[String]): FrovedisGroupedDF = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    val size = targets.size
    val tt = getColumnTypes(targets) // throws exception, if colname not found
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.groupFrovedisDataframe(fs.master_node,get(),targets,size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisGroupedDF(proxy,cols,types,targets,tt)
  }

  // Usage: df.groupBy("colA") 
  // Usage: df.groupBy("colA", "colB") // ... any number of strings
  def groupBy(must: String, optional: String*): FrovedisGroupedDF = {
    val all = (Array(must) ++ optional).toArray.map(x => x.toString)
    return groupBy(all)
  }

  // Usage: df.groupBy($$"colA", $$"colB") // ... any number of cols in $$"name" form
  def groupBy(c: FrovedisColumn*): FrovedisGroupedDF = {
    val all = c.toArray
    val size = all.size
    val funcs = new Array[Long](size)
    val groupedCols = new Array[String](size)
    for (i <- 0 until size) {
      val fn = all(i)
      if (fn.isAGG) throw new IllegalArgumentException(
      s"aggregate functions are not allowed in groupBy!")
      funcs(i) = fn.get()
      groupedCols(i) = fn.colName
    }
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.fgroupFrovedisDataframe(fs.master_node,get(),funcs,size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisGroupedDF(proxy,cols,types,groupedCols)
  }

  private def getFrovedisJoinType(join_type: String): String = {
    // mapping of join type, spark-> "frovedis"
    val joinTypeMap = Map("inner" -> "inner",  
                          "left" -> "outer", 
                          "leftouter" -> "outer")
    if(!joinTypeMap.contains(join_type)) 
      throw new IllegalArgumentException("Unsupported join type: " + join_type)
    return joinTypeMap(join_type)
  }

  private def rename_helper(df: FrovedisDataFrame,
                   rsuf: String): (FrovedisDataFrame, Boolean) = {
    val common_key_cols = this.columns.toSet
                              .intersect(df.columns.toSet).toArray
    val rename_needed = common_key_cols.size > 0
    var df_right = df
    if (rename_needed){
      var common_key_cols_renamed = common_key_cols.map(x => x + rsuf)
      df_right = df_right.withColumnRenamed(common_key_cols,
                                            common_key_cols_renamed)  
    }
    return (df_right, rename_needed)
  }

  private def join_helper(df: FrovedisDataFrame,
                          opt: FrovedisColumn,
                          join_type: String, 
                          join_algo: String,
                          rsuf: String = "_right"): FrovedisDataFrame = {
    if (fdata == -1) throw new IllegalArgumentException("Invalid Frovedis left dataframe!")
    if (df.fdata == -1) throw new IllegalArgumentException("Invalid Frovedis right dataframe!")
    if (rsuf == "") throw new IllegalArgumentException("Suffix for renaming " +
                                                       "cannot be empty string!\n")
    var df_right = df
    var t_cols = this.cols ++ df_right.cols
    var t_types = this.types ++ df_right.types

    // all common-columns renamed with rsuf
    var tmp = rename_helper(df_right, rsuf)
    df_right = tmp._1
    var rename_needed = tmp._2

    if (rename_needed){
      t_cols = this.cols ++ df_right.cols
      t_types = this.types ++ df_right.types
    }
    
    var join_opt = opt
    if (opt.isID) {
      val cname = opt.colName
      join_opt = (col(cname) === col(cname + rsuf))
    } 
    val check_opt = true // checking is required at server side, since dfoperator is constructed on-the-fly
    val frov_join_type = getFrovedisJoinType(join_type)
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.joinFrovedisDataframes(fs.master_node,
                this.get(), df_right.get(), join_opt.get(), frov_join_type,
                join_algo, check_opt, rsuf)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = new FrovedisDataFrame(proxy, t_cols, t_types)
    return ret
  }

  // --- Frovedis server side JOIN definition here ---
  // --- When right is a Frovedis Dataframe ---
  // case 1. FrovedisColumn: from FrovedisColumn 
  // For example, left.join(right,left("A") === right("B"))
  //              left.join(right,left.col("A") === right.col("B"))
  //              left.join(right,$$"A" === $$"B")
  //              left.join(right,col("A") === col("B"))
  // FrovedisColumn: Only Column object 
  // For example, left.join(right,$$"A")]
  def join(df: FrovedisDataFrame, opt: FrovedisColumn,
           join_type: String, join_algo: String,
           rsuf: String) = join_helper(df, opt, join_type, join_algo, rsuf)

  def join(df: FrovedisDataFrame, opt: FrovedisColumn,
           join_type: String, 
           join_algo: String) = join_helper(df, opt, join_type, join_algo, "_right")

  def join(df: FrovedisDataFrame, opt: FrovedisColumn, 
           join_type: String) = join_helper(df, opt, join_type, "bcast", "_right")

  def join(df: FrovedisDataFrame, 
           opt: FrovedisColumn) = join_helper(df, opt, "inner", "bcast", "_right")

  // case 2. String: Only single key common in both [e.g., left.join(right,"A")]
  def join(df: FrovedisDataFrame, key: String,
           join_type: String, join_algo: String,
           rsuf: String) = join_helper(df, col(key) === col(key + rsuf), join_type, join_algo)

  def join(df: FrovedisDataFrame, key: String,
           join_type: String, 
           join_algo: String): FrovedisDataFrame = join(df, key, join_type, join_algo, "_right")

  def join(df: FrovedisDataFrame, key: String, 
           join_type: String): FrovedisDataFrame = join(df, key, join_type, "bcast", "_right")

  def join(df: FrovedisDataFrame, 
           key: String): FrovedisDataFrame = join(df, key, "inner", "bcast", "_right")

  // case 3. join with keys provided as scala Sequence
  def join(df: FrovedisDataFrame, keys: Seq[String],
           join_type: String, join_algo: String,
           rsuf: String): FrovedisDataFrame = {
    var ret: FrovedisDataFrame = null
    if (keys.size == 0) ret = crossJoin(df, rsuf)
    else {
      var opt: FrovedisColumn = (col(keys(0)) === col(keys(0) + rsuf)) 
      for(i <- 1 until keys.size) opt = opt && (col(keys(i)) === col(keys(i) + rsuf)) 
      ret = join_helper(df, opt, join_type, join_algo, rsuf)
    }
    return ret
  }

  def join(df: FrovedisDataFrame, keys: Seq[String],
           join_type: String, 
           join_algo: String): FrovedisDataFrame = join(df, keys, join_type, join_algo, "_right")
  
  def join(df: FrovedisDataFrame, keys: Seq[String],
           join_type: String): FrovedisDataFrame = join(df, keys, join_type, "bcast", "_right")
  
  def join(df: FrovedisDataFrame, 
           keys: Seq[String]): FrovedisDataFrame = join(df, keys, "inner", "bcast", "_right")
  
  def join(df: FrovedisDataFrame) = crossJoin(df)
  
  // --- When right is a Spark Dataframe ---
  // case 1. FrovedisColumn: from FrovedisColumn
  // For example left.join(right,left("A") === right("B"))
  //             left.join(right,left.col("A") === right.col("B"))
  //             left.join(right,$$"A" === $$"B")
  // FrovedisColumn: Only Column object 
  // For example, left.join(right,$$"A")]
  def join(df: DataFrame, opt: FrovedisColumn, 
           join_type: String, join_algo: String,
           rsuf: String): FrovedisDataFrame = {
    val fdf = new FrovedisDataFrame(df) // spark dataframe to frovedis dataframe
    val ret = join(fdf, opt, join_type, join_algo, rsuf)
    fdf.release()
    return ret
  }
  def join(df: DataFrame, opt: FrovedisColumn,
           join_type: String, 
           join_algo: String): FrovedisDataFrame = join(df, opt, join_type, join_algo, "_right")
 
  def join(df: DataFrame, opt: FrovedisColumn, 
           join_type: String): FrovedisDataFrame = join(df, opt, join_type, "bcast", "_right")
  
  def join(df: DataFrame, 
           opt: FrovedisColumn): FrovedisDataFrame = join(df, opt, "inner", "bcast", "_right")
  

  // case 2. String: Only single key common in both [e.g., left.join(right,"A")]
  def join(df: DataFrame, key: String, 
           join_type: String, join_algo: String,
           rsuf: String): FrovedisDataFrame = {
    val fdf = new FrovedisDataFrame(df) // spark dataframe to frovedis dataframe
    val ret = join(fdf, key, join_type, join_algo, rsuf)
    fdf.release()
    return ret
  }
  def join(df: DataFrame, key: String, join_type: String,
           join_algo: String): FrovedisDataFrame = join(df, key, join_type, join_algo, "_right")
  
  def join(df: DataFrame, key: String, 
           join_type: String): FrovedisDataFrame = join(df, key, join_type, "bcast", "_right")
  
  def join(df: DataFrame, 
           key: String): FrovedisDataFrame = join(df, key, "inner", "bcast", "_right")
  
  // case 3. join with keys provided as scala Sequence
  def join(df: DataFrame, keys: Seq[String],
           join_type: String, join_algo: String,
           rsuf: String): FrovedisDataFrame = {
    val fdf = new FrovedisDataFrame(df) // spark dataframe to frovedis dataframe
    val ret = join(fdf, keys, join_type, join_algo, rsuf)
    fdf.release()
    return ret
  }

  def join(df: DataFrame, keys: Seq[String],
           join_type: String, 
           join_algo: String): FrovedisDataFrame = join(df, keys, join_type, join_algo, "_right")
  
  def join(df: DataFrame, keys: Seq[String],
           join_type: String): FrovedisDataFrame = join(df, keys, join_type, "bcast", "_right")
  
  def join(df: DataFrame, 
           keys: Seq[String]): FrovedisDataFrame = join(df, keys, "inner", "bcast", "_right")
  
  def join(df: DataFrame) = crossJoin(df)
  

  // cross-join
  def crossJoin(df: FrovedisDataFrame, rsuf: String): FrovedisDataFrame = {
    if (fdata == -1) throw new IllegalArgumentException("Invalid Frovedis left dataframe!")
    if (rsuf == "") throw new IllegalArgumentException("Suffix for renaming " +
                                                       "cannot be empty string!\n")
    val fs = FrovedisServer.getServerInstance()
    val opt_proxy = JNISupport.getCrossDfopt(fs.master_node)
    var (df_right, rename_needed) = rename_helper(df, rsuf)
    var t_cols = this.cols ++ df_right.cols
    var t_types = this.types ++ df_right.types
    val check_opt = false // not need for cross-join
    val proxy = JNISupport.joinFrovedisDataframes(fs.master_node,
                this.get(), df_right.get(), opt_proxy, "inner",
                "bcast", check_opt, rsuf)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisDataFrame(proxy, t_cols, t_types) 
  }

  def crossJoin(df: FrovedisDataFrame): FrovedisDataFrame = crossJoin(df, "_right")

  def crossJoin(df: DataFrame, rsuf: String): FrovedisDataFrame = {
    val fdf = new FrovedisDataFrame(df) // spark dataframe to frovedis dataframe
    val ret = crossJoin(fdf, rsuf)
    fdf.release()
    return ret
  }

  def crossJoin(df: DataFrame): FrovedisDataFrame = crossJoin(df, "_right")
  
  def int2bool(a: Any): Any = {
    if (a == Int.MaxValue) return null
    if (a == 1) return true
    else return false
  }

  def combinePartitionsAsRow(arg0: Iterator[Row],
                            arg1: Iterator[Row]): Iterator[Row] = {
      var arr1 = arg0.toArray
      var arr2 = arg1.toArray
      var sz = arr1.size

      var arr3 = new Array[Row](sz)
      for (i <- 0 until sz) {
          arr3(i) = Row.merge(arr1(i), arr2(i))
      }
      return arr3.toIterator
  }

  def combine2RowRddsPartition(rdd1: RDD[Row], rdd2: RDD[Row]): RDD[Row] = {
      var resRdd = rdd1.zipPartitions(rdd2)(combinePartitionsAsRow)
      return resRdd
  }

  def combine_rows_as_partitions(arr: Seq[RDD[Any]]): RDD[Row] = {
      var cols_row = arr.map(r1 => r1.map(x=> Row(x)))
      var resRdd = cols_row.reduce(combine2RowRddsPartition)
      return resRdd
  }

  def to_spark_DF(): DataFrame = {
    val spark = SparkSession.builder().getOrCreate()
    if (this.columns.size == 0) return spark.emptyDataFrame

    if(fdata == -1)  throw new IllegalArgumentException("Invalid Frovedis Dataframe!\n")
    val size = cols.size
    var ret_lb = ListBuffer[RDD[_]]()
    val fs = FrovedisServer.getServerInstance()
    for (i <- 0 until size) {
      val tid = types(i)
      val cname = cols(i)
      val cptr = JNISupport.getDFColumnPointer(fs.master_node, fdata, cname, tid)
      var info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      var col_rdd =  tid match {
        case DTYPE.INT    => IntDvector.to_RDD(cptr)
        case DTYPE.LONG   => LongDvector.to_RDD(cptr)
        case DTYPE.FLOAT  => FloatDvector.to_RDD(cptr)
        case DTYPE.DOUBLE => DoubleDvector.to_RDD(cptr)
        case DTYPE.STRING => StringDvector.to_RDD(cptr)
        case DTYPE.WORDS => StringDvector.to_RDD(cptr) // cptr is dvector<string> even for WORDS
        case DTYPE.BOOL => IntDvector.to_RDD(cptr)
        case _  => throw new IllegalArgumentException("to_spark_DF: Invalid " +
                                                    "datatype encountered: %s !\n"
                                                    .format(tid))
      }
      val len = col_rdd.count
      JNISupport.releaseDFColumnPointer(fs.master_node, cptr, tid)
      info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      ret_lb += col_rdd
    }
    var ret = ret_lb.toArray.asInstanceOf[Array[RDD[Any]]]
    val maxValueMap = Map(DTYPE.INT -> Int.MaxValue, DTYPE.LONG -> Long.MaxValue,
                      DTYPE.FLOAT -> Float.MaxValue, DTYPE.DOUBLE -> Double.MaxValue,
                      DTYPE.STRING -> "NULL", DTYPE.WORDS -> "NULL", 
                      DTYPE.BOOL -> Int.MaxValue)

    for (i <- 0 until ret.size) {
      if (types(i) == DTYPE.BOOL ) ret(i) = ret(i).map(x => int2bool(x))
      else ret(i) = ret(i).map(x => { if (x == maxValueMap(types(i))) null else x})
    }

    val ncols = cols.size
    var df_schema = ListBuffer[StructField]()

    for (i <- 0 to (ncols-1)) {
      val tid = types(i)
      val cname = cols(i)
      var col_field =  tid match {
        case DTYPE.INT    => StructField(cname, IntegerType, true)
        case DTYPE.LONG   => StructField(cname, LongType, true)
        case DTYPE.FLOAT  => StructField(cname, FloatType, true)
        case DTYPE.DOUBLE => StructField(cname, DoubleType, true)
        case DTYPE.STRING => StructField(cname, StringType, true)
        case DTYPE.WORDS  => StructField(cname, StringType, true)
        case DTYPE.BOOL => StructField(cname, BooleanType, true)
        case _  => throw new IllegalArgumentException("to_spark_DF: Invalid " +
                                                    "datatype encountered: %s !\n"
                                                    .format(tid))
      }
      df_schema += col_field
    }
    val resRdd = combine_rows_as_partitions(ret.toSeq)
    val res_df = spark.createDataFrame(resRdd, StructType(df_schema.toArray))
    return res_df
  }

  private def get_types(): Array[String] = {
    val size = types.size
    var ret = new Array[String](size)
    for (i <- 0 to (size-1)) ret(i) = TMAPPER.id2string(types(i))
    return ret
  }

  def col(t: String) = new FrovedisColumn(t)

  def apply(t: String) = col(t)

  def get() = fdata
  def get_code() = code
  def columns = cols
  def dtypes = cols.zip(get_types())

  // for internal purpose
  private def dtypes_as_map: mMap[String, Short] = {
    val ret = mMap[String, Short]()
    for (i <- 0 until cols.size) ret(cols(i)) = types(i)
    return ret
  }

  // for internal purpose
  private def mark_boolean_columns(dt: mMap[String, Short], 
                                   dummy: DummyDftable): Unit = {
    val bool_id = DTYPE.BOOL
    val ncol = dummy.names.size
    for (i <- 0 until ncol) {
      val c = dummy.names(i)
      if (dt.contains(c) && dt(c) == bool_id) dummy.types(i) = bool_id
    }
  }

  def hasColumn(c: String) = cols.indexOf(c) != -1
  def getColumnType(c: String): Short = { 
    val index = cols.indexOf(c)
    if (index == -1)  throw new IllegalArgumentException("No column named: " + c)
    return types(index)
  }
  def getColumnTypes(c: Array[String]): Array[Short] = { 
    val size = c.length     
    var tt = new Array[Short](size)
    // getColumnType() will throw exception, if column name doesnt exist
    for (i <- 0 to (size-1)) tt(i) = getColumnType(c(i))
    return tt
  }
  // returns dataframe size
  def count(): Long = {
    if(fdata == -1)  throw new IllegalArgumentException("Invalid Frovedis Dataframe!\n")
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.getFrovedisDFSize(fs.master_node,get())
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    else return ret
  }
  def count(cname: Array[String]): Array[String] = {
    if(fdata == -1)  throw new IllegalArgumentException("Invalid Frovedis Dataframe!\n")
    val size = cname.length
    for (i <- 0 to (size-1))
      if(!hasColumn(cname(i)))
        throw new IllegalArgumentException("No column named: " + cname(i))
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.getFrovedisDFCounts(fs.master_node,get(),cname,size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    else return ret
   }
  def count(cname: String*): Array[String] = {
    val all = cname.toArray.map(x => x.toString)
    return count(all)
  }
  def avg(cname: Array[String]): Array[String] = {
    if(fdata == -1)  throw new IllegalArgumentException("Invalid Frovedis Dataframe!\n")
    val tids = getColumnTypes(cname) // throws exception, if colname not found
    if ((tids contains DTYPE.STRING) || (tids contains DTYPE.WORDS)) 
      throw new IllegalArgumentException("String-typed column given for avg!\n") 
    val size = cname.length
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.getFrovedisDFMeans(fs.master_node,get(),cname,size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    else return ret
  }
  def avg(cname: String*): Array[String] = {
    val all = cname.toArray.map(x => x.toString)
    return avg(all)
  }
  def sum(cname: Array[String],
          tids: Array[Short]): Array[String] = {
    if(fdata == -1)  throw new IllegalArgumentException("Invalid Frovedis Dataframe!\n")
    if ((tids contains DTYPE.STRING) || (tids contains DTYPE.WORDS)) 
      throw new IllegalArgumentException("String-typed column given for sum!\n") 
    val size = cname.length
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.getFrovedisDFTotals(fs.master_node,get(),cname,tids,size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    else return ret
  }
  def sum(cname: Array[String]): Array[String] = {
    val tt = getColumnTypes(cname) // throws exception, if colname not found
    return sum(cname, tt)
  }
  def sum(cname: String*): Array[String] = {
    val all = cname.toArray.map(x => x.toString)
    return sum(all)
  }
  def min(cname: Array[String],
          tids: Array[Short]): Array[String] = {
    if(fdata == -1)  throw new IllegalArgumentException("Invalid Frovedis Dataframe!\n")
    if ((tids contains DTYPE.STRING) || (tids contains DTYPE.WORDS)) 
      throw new IllegalArgumentException("String-typed column given for min!\n") 
    val size = cname.length
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.getFrovedisDFMins(fs.master_node,get(),cname,tids,size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    else return ret
  }
  def min(cname: Array[String]): Array[String] = {
    val tt = getColumnTypes(cname) // throws exception, if colname not found
    return min(cname, tt)
  }
  def min(cname: String*): Array[String] = {
    val all = cname.toArray.map(x => x.toString)
    return min(all)
  }
  def max(cname: Array[String],
          tids: Array[Short]): Array[String] = {
    if(fdata == -1)  throw new IllegalArgumentException("Invalid Frovedis Dataframe!\n")
    if ((tids contains DTYPE.STRING) || (tids contains DTYPE.WORDS)) 
      throw new IllegalArgumentException("String-typed column given for max!\n") 
    val size = cname.length
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.getFrovedisDFMaxs(fs.master_node,get(),cname,tids,size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    else return ret
 }
  def max(cname: Array[String]): Array[String] = {
    val tt = getColumnTypes(cname) // throws exception, if colname not found
    return max(cname, tt)
  }
  def max(cname: String*): Array[String] = {
    val all = cname.toArray.map(x => x.toString)
    return max(all)
  }
  def std(cname: Array[String]): Array[String] = {
    if(fdata == -1)  throw new IllegalArgumentException("Invalid Frovedis Dataframe!\n")
    val tids = getColumnTypes(cname) // throws exception, if colname not found
    if ((tids contains DTYPE.STRING) || (tids contains DTYPE.WORDS)) 
      throw new IllegalArgumentException("String-typed column given for avg!\n") 
    val size = cname.length
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.getFrovedisDFStds(fs.master_node,get(),cname,size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    else return ret
  }
  def std(cname: String*): Array[String] = {
    val all = cname.toArray.map(x => x.toString)
    return std(all)
  }
  def agg(targets: Array[FrovedisColumn]): FrovedisDataFrame = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    val size = targets.length
    val agg_proxies = new Array[Long](size)
    for(i <- 0 until size) {
      val func = targets(i)
      if (!func.isAGG) throw new IllegalArgumentException(func.colName + 
        " is not an aggregate function!\n")
      agg_proxies(i) = func.get()
    }
    val fs = FrovedisServer.getServerInstance()
    val dummy = JNISupport.executeFrovedisAgg(fs.master_node,get(),agg_proxies,size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisDataFrame(dummy) // TODO: mark bool (difficult to deduce)
  }
  def agg(x: FrovedisColumn, y: FrovedisColumn*): FrovedisDataFrame = {
    val arr = (Array(x) ++ y).toArray
    return agg(arr)
  }
  def get_numeric_dtypes(): Array[(Short,String)] = {
    val size = types.length
    var ret  = new ArrayBuffer[(Short,String)]()
    for(i <- 0 to (size-1)) {
      val tid = types(i)
      if((tid != DTYPE.STRING) && (tid != DTYPE.WORDS)) { // only-numeric
        val k = (tid,cols(i))
        ret += k
      }
    }
    return ret.toArray
  }
  def describe(): DataFrame = { // TODO: define in library to return frovedis dataframe
    val ct = get_numeric_dtypes()
    val tids = ct.map(_._1)
    val targets = ct.map(_._2)
    val size = targets.length
    val counts = Array("count") ++ count(targets)
    val totals = Array("sum") ++ sum(targets,tids)
    val avgs = Array("mean") ++ avg(targets)
    val stds = Array("stddev") ++ std(targets)
    val mins = Array("min") ++ min(targets,tids)
    val maxs = Array("max") ++ max(targets,tids)
    /* --- debug prints ---
    counts.foreach(println) 
    totals.foreach(println) 
    avgs.foreach(println) 
    mins.foreach(println) 
    maxs.foreach(println) 
    stds.foreach(println) 
    */
    val r1 = Row.fromSeq(counts.toSeq)
    val r2 = Row.fromSeq(totals.toSeq)
    val r3 = Row.fromSeq(avgs.toSeq)
    val r4 = Row.fromSeq(stds.toSeq)
    val r5 = Row.fromSeq(mins.toSeq)
    val r6 = Row.fromSeq(maxs.toSeq)
    val sc = SparkContext.getOrCreate()
    val data = sc.parallelize(Seq(r1,r2,r3,r4,r5,r6))
    val schemaArr = new Array[StructField](size+1)
    schemaArr(0) = StructField("summary", StringType, true)
    for(i <- 1 to size) {
      schemaArr(i) = StructField(targets(i-1), StringType, true)
    }
    val schema = StructType(schemaArr.toList)
    val spark = SparkSession.builder.getOrCreate()
    return spark.createDataFrame(data,schema)
  }
  def withColumnRenamed(name: Array[String], 
                        new_name: Array[String]): FrovedisDataFrame = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    if(name.length != new_name.length) 
      throw new IllegalArgumentException("Input size doesn't match!")
    val size = name.length
    val new_cols = cols.clone()
    for (i <- 0 to (size-1)) {
      val idx = cols.indexOf(name(i))
      if (idx == -1) throw new IllegalArgumentException("No column named: " + name(i))
      new_cols(idx) = new_name(i)
    }
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.renameFrovedisDataframe(fs.master_node, get(),
                                                   name, new_name, size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisDataFrame(proxy,new_cols,types)
  }
  def withColumnRenamed(name: String, new_name: String): FrovedisDataFrame = {
    return withColumnRenamed(Array(name), Array(new_name))
  }
  def toFrovedisRowmajorMatrix(name: Array[String]): FrovedisRowmajorMatrix = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    val size = name.size
    for (i <- 0 to (size-1)) {
      if(!hasColumn(name(i))) 
        throw new IllegalArgumentException("No column named: " + name(i))
    }
    val fs = FrovedisServer.getServerInstance()
    val dmat = JNISupport.DFToRowmajorMatrix(fs.master_node,get(),name,size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisRowmajorMatrix(dmat)
  }
  def toFrovedisColmajorMatrix(name: Array[String]): FrovedisColmajorMatrix = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    val size = name.size
    for (i <- 0 to (size-1)) {
      if(!hasColumn(name(i))) 
        throw new IllegalArgumentException("No column named: " + name(i))
    }
    val fs = FrovedisServer.getServerInstance()
    val dmat = JNISupport.DFToColmajorMatrix(fs.master_node,get(),name,size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisColmajorMatrix(dmat)
  }
  def toFrovedisSparseData(t_cols: Array[String],
                          cat_cols: Array[String],
                          need_info: Boolean = false): (FrovedisSparseData,DFtoSparseInfo) = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    val size1 = t_cols.size
    val size2 = cat_cols.size
    for (i <- 0 to (size1-1)) {
      if(!hasColumn(t_cols(i))) 
        throw new IllegalArgumentException("No column named: " + t_cols(i))
    }
    for (i <- 0 to (size2-1)) {
      if(!(t_cols contains cat_cols(i))) 
        throw new IllegalArgumentException("Target column list doesn't contain categorical column: " + cat_cols(i))
    }
    val fs = FrovedisServer.getServerInstance()
    val info_id = ModelID.get() // getting unique id for info to be registered at server side
    val dmat = JNISupport.DFToCRSMatrix(fs.master_node,get(),t_cols,size1,
                                        cat_cols,size2,info_id)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)

    val crs_mat = new FrovedisSparseData(dmat)
    val conv_info = new DFtoSparseInfo(info_id)

    if(need_info) return (crs_mat,conv_info)
    else {
      conv_info.release()
      return (crs_mat,null)
    }
  }

  def toFrovedisSparseData(conv_info: DFtoSparseInfo): FrovedisSparseData = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    if(conv_info.get() == -1) throw new IllegalArgumentException("Invalid sparse conversion info!")
    val fs = FrovedisServer.getServerInstance()
    val dmat = JNISupport.DFToCRSMatrixUsingInfo(fs.master_node,get(),conv_info.get())
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisSparseData(dmat)
  }

  def withColumn(cname: String, 
                 op: FrovedisColumn): FrovedisDataFrame = { 
    val fs = FrovedisServer.getServerInstance()
    val dummy = JNISupport.executeDFfunc(fs.master_node, get(), cname, op.get())
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    mark_boolean_columns(this.dtypes_as_map, dummy)
    if (op.isBOOL) { // mark newly added column (cname) as BOOL
      val ncol = dummy.names.size
      dummy.types(ncol - 1) = DTYPE.BOOL
    }
    return new FrovedisDataFrame(dummy)
  }

  def append_column(cname: String, op: FrovedisColumn): this.type = { // append in-place
    val tmp = withColumn(cname, op)
    this.fdata = tmp.fdata
    this.cols = tmp.cols.clone()
    this.types = tmp.types.clone()
    tmp.fdata = -1 // to avoid it being released
    this
  }

  def drop(targets: Array[String]): FrovedisDataFrame = {
    val select_targets = this.columns.toSet.diff(targets.toSet).toArray
    return select(select_targets)
  }

  def drop(targets: FrovedisColumn*): FrovedisDataFrame = {
    return drop(targets.toArray.map(x => x.toString))
  }

  def drop_inplace(targets: Array[String]): this.type = {
    val fs = FrovedisServer.getServerInstance()
    JNISupport.dropDFColsInPlace(fs.master_node, get(), 
                                 targets, targets.size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    // TODO: need to remove cols from this.cols and this.types ?
    this
  }

  def distinct(): FrovedisDataFrame = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.getDistinct(fs.master_node, get())
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisDataFrame(proxy, cols, types)
  }

  def dropDuplicates(colNames: Array[String], keep: String = "first"): FrovedisDataFrame = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    val fs = FrovedisServer.getServerInstance()
    if (!Array("first", "last").contains(keep))
      throw new IllegalArgumentException("Unsupported Value for" +
                                        " 'keep' parameter: " + keep)
    val dummy = JNISupport.dropDuplicates(fs.master_node, get(), colNames,
                                          colNames.size, keep)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    mark_boolean_columns(this.dtypes_as_map, dummy)
    return new FrovedisDataFrame(dummy)
  }

  def dropDuplicates(col1: String, cols: String*): 
    FrovedisDataFrame = dropDuplicates(Array(col1) ++ cols) 

  def dropDuplicates(cols: Seq[String]): 
    FrovedisDataFrame = dropDuplicates(cols.toArray) 

  def limit(n: Int): FrovedisDataFrame = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    if (n < 0)
      throw new IllegalArgumentException("The limit value must be equal to" +
                                        " or greater than 0, but got: " + n)
    val fs = FrovedisServer.getServerInstance()
    val dummy = JNISupport.limitDF(fs.master_node, get(), n)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    mark_boolean_columns(this.dtypes_as_map, dummy)
    return new FrovedisDataFrame(dummy)
  }

  // not full-proof definition (just for proxy check) 
  override def equals(obj: Any): Boolean = {
    var df2 = obj.asInstanceOf[FrovedisDataFrame]
    return this.get() == df2.get()
  }
}
