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
import org.apache.spark.sql.DataFrame
import org.apache.spark.sql.catalyst.InternalRow
import org.apache.spark.sql.catalyst.expressions.UnsafeRow
import scala.collection.mutable.ArrayBuffer
import scala.collection.mutable.ListBuffer
import org.apache.log4j.{Level, Logger}

object TMAPPER {
  val func2id = Map("sum" -> DTYPE.NONE, "max" -> DTYPE.NONE, "min" -> DTYPE.NONE,
                    "mean" -> DTYPE.DOUBLE, "avg" -> DTYPE.DOUBLE, 
                    "std" -> DTYPE.DOUBLE,  "count" -> DTYPE.LONG)

  val id2field = Map(DTYPE.INT -> IntegerType,   DTYPE.LONG -> LongType,
                     DTYPE.FLOAT -> FloatType,   DTYPE.DOUBLE -> DoubleType,
                     DTYPE.STRING -> StringType, DTYPE.WORDS -> StringType,
                     DTYPE.BOOL -> BooleanType)

  val id2string = Map(DTYPE.INT -> "IntegerType",   DTYPE.LONG -> "LongType",
                      DTYPE.FLOAT -> "FloatType",   DTYPE.DOUBLE -> "DoubleType",
                      DTYPE.STRING -> "StringType", DTYPE.WORDS -> "StringType",
                      DTYPE.BOOL -> "BooleanType")

  // string2id: only used in dataframe load
  // used WORDS instead of STRING, while loading string column (RDD[STRING]) as Dvector for better performance
  // simply enable ["StringType"  -> DTYPE.STRING] if you want to use the STRING type instead
  val string2id = Map("IntegerType" -> DTYPE.INT,    "LongType" -> DTYPE.LONG,
                   "FloatType"   -> DTYPE.FLOAT,  "DoubleType" -> DTYPE.DOUBLE,
                   //"StringType"  -> DTYPE.STRING, 
                   "StringType"  -> DTYPE.WORDS,
                   "BooleanType" -> DTYPE.BOOL)

  val spark = SparkSession.builder.getOrCreate()
  import spark.implicits._

  def bool2int(a: Any): Int = {
      if(a == null) return Int.MaxValue
      if(a == true) return 1
      else return 0
  }
 
  def toTypedDvector(rddData: RDD[InternalRow], dtype: Short, i: Int,
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

object converter {
  def cast(v: String, to_type: Short): Any = {
    return to_type match {
      case DTYPE.INT => v.toInt
      case DTYPE.LONG => v.toLong
      case DTYPE.FLOAT => v.toFloat
      case DTYPE.DOUBLE => v.toDouble
      case DTYPE.BOOL => v.toBoolean
      case DTYPE.STRING => v
      case _ => throw new IllegalArgumentException("Unsupported type: " + TMAPPER.id2string(to_type))
    }
  }
}

object DFConverter {
  def toDF(row: Array[Row], schema: StructType): DataFrame = {
    val spark = SparkSession.builder.getOrCreate()
    val ctxt = SparkContext.getOrCreate()
    return spark.createDataFrame(ctxt.parallelize(row), schema)
  }
}

object sDFTransfer extends java.io.Serializable {
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
              //case DTYPE.STRING => if(row.isNullAt(i)) "NULL" row.getString(i)
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
          val vptr = vptrs(i * nproc + destId)
          //println("col: " + cols(i) + "; dtype: " + types(i) + "; vptr: " + vptr)
          copy_local_data(index, destId, fw_nodes(destId),
                          vptr, localId, mat(i), k, types(i))
        }
        t0.show("data transfer: ")
        Array(true).toIterator // SUCCESS (since need to return an iterator)
    })
    return ret.count // for action
  }
}

object _jDFTransfer extends java.io.Serializable {
  def execute(rddData: RDD[InternalRow], // actually UnsafeRow
              fw_nodes: Array[Node],
              vptrs: Array[Long],
              types: Array[Short],
              block_sizes: Array[Long],
              ncol: Int, nproc: Int): Long = {
    val ret = rddData.mapPartitionsWithIndex({ case(index, x) =>
        val t0 = new TimeSpent(Level.TRACE)
        var k = 0
        val obj = new ArrayBuffer[Object]()
        val off = new ArrayBuffer[Long]()
        while(x.hasNext) {
          val row = x.next.asInstanceOf[UnsafeRow]
          obj += row.getBytes() // row.getBaseObject()
          off += row.getBaseOffset()
          k += 1
        }
        t0.show("baseObject, baseOffset extraction: ")

        val (destId, myst) = GenericUtils.index2Dest(index, block_sizes)
        val localId = index - myst
        val w_node = fw_nodes(destId)

        for (i <- 0 until ncol) {
          val vptr = vptrs(i * nproc + destId)
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

  // creating a frovedis dataframe from a spark dataframe -> User API
  def this(df: DataFrame) = {
    this()
    load(df)
  }
  def this(df: DataFrame, name: String, others: String*) = {
    this()
    val sdf = df.select(name, others:_*)
    optimized_load(sdf) // temporary method
  }
  // internally used, not for user
  def this(proxy: Long, cc: Array[String], tt: Array[Short]) = {
    this()
    fdata = proxy
    cols = cc.clone()
    types = tt.clone()
  }
  def this(dummy: DummyDftable) = {
    this()
    fdata = dummy.dfptr
    cols = dummy.names.clone()
    types = dummy.types.clone()
  }
  // to release a frovedis dataframe from frovedis server -> User API
  def release () : Unit = {
    if (fdata != -1) {
      val fs = FrovedisServer.getServerInstance()
      JNISupport.releaseFrovedisDataframe(fs.master_node,fdata)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      fdata = -1
      cols = null
      types = null
    }
  }
  // to display contents of a frovedis dataframe from frovedis server -> User API
  def show () : Unit = {
    if (fdata != -1) {
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
    cols = df.dtypes.map(_._1)
    val tt = df.dtypes.map(_._2)
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
      dvecs(i) = TMAPPER.toTypedDvector(rddData, dtype, i, part_sizes)
    }
    val fs = FrovedisServer.getServerInstance()
    fdata = JNISupport.createFrovedisDataframe(fs.master_node,types,cols,dvecs,size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
  }
  // for loading specific columns; 
  // TODO: improve existing limitations: 
  //   - doesn't support StringType column
  //   - assumes null is absent in target columns
  //   - use some intermediate copies (memory-image copy can be done)
  //   - might cause bad-alloc in ve-memory when target columns are too large to fit
  def optimized_load (df: DataFrame) : Unit = {
    release()
    cols = df.dtypes.map(_._1)
    types = df.dtypes.map(_._2).map(x => TMAPPER.string2id(x))
    val ncol = cols.size
    val rddData = df.queryExecution.toRdd
    val t_log = new TimeSpent(Level.DEBUG)

    // (1) allocate
    val fs = FrovedisServer.getServerInstance()
    val nproc = fs.worker_size
    val npart = rddData.getNumPartitions
    val block_sizes = GenericUtils.get_block_sizes(npart, nproc)
    val vptrs = JNISupport.allocateLocalVectors2(fs.master_node, block_sizes,
                                                 nproc, types, ncol) // vptrs: ncol x nproc
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
      _jDFTransfer.execute(rddData, fw_nodes, vptrs, types, block_sizes, ncol, nproc)
    } else {
      sDFTransfer.execute(rddData, fw_nodes, vptrs, types, block_sizes, ncol, nproc)
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
  def filter(expr: Expr): FrovedisDataFrame = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    val opt = new DFOperator(expr,cols,types)
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.filterFrovedisDataframe(fs.master_node,this.get(),opt.get())
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = new FrovedisDataFrame(proxy, cols, types)
    opt.release()
    return ret
  }
  // Usage: df.when(df.col("colA") > 10)
  // Usage: df.when(df("colA") > 10)
  // Usage: df.when($$"colA" > 10)
  def when(expr: Expr): FrovedisDataFrame = {
    return filter(expr)
  }

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
    val all = c.toArray
    val cols = all.map({ case(x) => 
      if (x.isOpt) append_column(x.toString, x)
      x.colName
    })
    var ret = select(cols)

    // for dropping appended columns in this
    val drop_targets = DFColUtils.get_opt_cols(all)
    if (drop_targets.size > 0) drop_inplace(drop_targets)

    // for renaming in response to as()...
    val rename_t = DFColUtils.get_rename_targets(all) 
    if (rename_t._1.size > 0) ret = ret.withColumnRenamed(rename_t._1, rename_t._2)

    return ret
  } 

  private def sort_impl(targets: Array[String], 
                        isDescArr: Array[Int]): FrovedisDataFrame = {
    val size = targets.size
    for (i <- 0 to (size-1)) 
      if(!hasColumn(targets(i))) 
        throw new IllegalArgumentException("No column named: " + targets(i)) 
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.sortFrovedisDataframe(fs.master_node,get(),
                                                 targets,isDescArr,size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisDataFrame(proxy, cols, types)
  }
  def sort(c: FrovedisColumn*): FrovedisDataFrame = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    val targets = c.toArray.map(x => x.toString)
    val isDescArr = c.toArray.map(x => x.getIsDesc)
    return sort_impl(targets, isDescArr)
  }
  
  def sort(must: String, optional: String*): FrovedisDataFrame = {
    val all = (Array(must) ++ optional).toArray.map(x => new FrovedisColumn(x))
    return sort(all:_*)
  }

  def orderBy(must: String, optional: String*): 
    FrovedisDataFrame = sort(must, optional : _*)
  def orderBy(c: FrovedisColumn*): FrovedisDataFrame = sort(c : _*)
  

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
    val all = c.toArray.map(x => x.toString)
    return groupBy(all)
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
                          .intersect(df.columns.toSet)
                          .toArray
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
                         exp: Expr,
                         join_type: String, 
                         join_algo: String,
                         rsuf: String = "_right"): FrovedisDataFrame = {
    var df_right = df
    var t_cols = this.cols ++ df_right.cols
    var t_types = this.types ++ df_right.types
    val dfopt_proxy = exp.get_proxy(t_cols, t_types, rsuf)

    val fs = FrovedisServer.getServerInstance()
    val opt = new DFOperator(dfopt_proxy)

    // all common-columns renamed with rsuf
    var tmp = rename_helper(df_right, rsuf)
    df_right = tmp._1
    var rename_needed = tmp._2

    if (rename_needed){
      t_cols = this.cols ++ df_right.cols
      t_types = this.types ++ df_right.types
    }
     
    val frov_join_type = getFrovedisJoinType(join_type)
    val proxy = JNISupport.joinFrovedisDataframes(fs.master_node,
                this.get(), df_right.get(), opt.get(), frov_join_type,
                join_algo)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = new FrovedisDataFrame(proxy, t_cols, t_types)
    opt.release()
    return ret
  }

  // --- Frovedis server side JOIN definition here ---
  // --- When right is a Frovedis Dataframe ---
  // case 1. Expr: from FrovedisColumn 
  // For example, left.join(right,left("A") === right("B"))
  //              left.join(right,left.col("A") === right.col("B"))
  //              left.join(right,$$"A" === $$"B")
  //              left.join(right,col("A") === col("B"))
  def join(df: FrovedisDataFrame, exp: Expr,
           join_type: String, join_algo: String,
           rsuf: String): FrovedisDataFrame = {
    if(fdata == -1)
      throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    if (rsuf == "") throw new IllegalArgumentException("Suffix for renaming "
                                                  + "cannot be empty string!\n")
    return this.join_helper(df, exp, join_type, join_algo, rsuf)
  }
  def join(df: FrovedisDataFrame, exp: Expr,
           join_type: String, join_algo: String): FrovedisDataFrame = {
    return join(df, exp, join_type, join_algo, "_right")
  }
  def join(df: FrovedisDataFrame, exp: Expr, join_type: String): FrovedisDataFrame = {
    return join(df, exp, join_type, "bcast", "_right")
  }
  def join(df: FrovedisDataFrame, exp: Expr): FrovedisDataFrame = {
    return join(df, exp, "inner", "bcast", "_right")
  }

  // case 2. String: Only single key common in both [e.g., left.join(right,"A")]
  def join(df: FrovedisDataFrame, key: String,
           join_type: String, join_algo: String): FrovedisDataFrame = {
    var keyCol = new FrovedisColumn(key)
    val ret = join(df, keyCol, join_type, join_algo)
    return ret
  }
  def join(df: FrovedisDataFrame, key: String, join_type: String): FrovedisDataFrame = {
    return join(df, key, join_type, "bcast")
  }
  def join(df: FrovedisDataFrame, key: String): FrovedisDataFrame = {
    return join(df, key, "inner", "bcast")
  }

  // case 3. FrovedisColumn: Only Column object [e.g., left.join(right,$$"A")]
  def join(df: FrovedisDataFrame, col: FrovedisColumn,
           join_type: String, join_algo: String): FrovedisDataFrame = {
    // converting FrovedisColumn to Expr object
    return join(df, (col === col), join_type, join_algo)
  }
  def join(df: FrovedisDataFrame, col: FrovedisColumn, join_type: String): FrovedisDataFrame = {
    return join(df, col, join_type, "bcast")
  }
  def join(df: FrovedisDataFrame, col: FrovedisColumn): FrovedisDataFrame = {
    return join(df, col, "inner", "bcast")
  }

  // case 4. join with keys provided as scala Sequence
  def join(df: FrovedisDataFrame, keys: Seq[String],
           join_type: String, join_algo: String,
           rsuf: String): FrovedisDataFrame = {
    if (rsuf == "") throw new IllegalArgumentException("Suffix for renaming "
                                                  + "cannot be empty string!\n")
    if(keys.size == 0)
        return crossJoin(df, rsuf)
    var exp: Expr = ( col(keys(0)) === col(keys(0)) ) 
    for(i <- 1 until keys.size){
      exp = exp && ( col(keys(i)) === col(keys(i)) ) 
    }
    return this.join_helper(df, exp, join_type, join_algo, rsuf)
  }
  def join(df: FrovedisDataFrame, keys: Seq[String],
           join_type: String, join_algo: String): FrovedisDataFrame = {
    return join(df, keys, join_type, join_algo, "_right")
  }
  def join(df: FrovedisDataFrame, keys: Seq[String],
           join_type: String): FrovedisDataFrame = {  
    return join(df, keys, join_type, "bcast", "_right")
  }
  def join(df: FrovedisDataFrame, keys: Seq[String]): FrovedisDataFrame = {
    return join(df, keys, "inner", "bcast", "_right")
  }
  def join(df: FrovedisDataFrame): FrovedisDataFrame = {
    return crossJoin(df)
  }
  
  // --- When right is a Spark Dataframe ---
  // case 1. Expr: from FrovedisColumn
  // For example left.join(right,left("A") === right("B"))
  //             left.join(right,left.col("A") === right.col("B"))
  //             left.join(right,$$"A" === $$"B")
  def join(df: DataFrame, exp: Expr, 
           join_type: String, join_algo: String,
           rsuf: String): FrovedisDataFrame = {
    if (rsuf == "") throw new IllegalArgumentException("Suffix for renaming "
                                                  + "cannot be empty string!\n")
    val fdf = new FrovedisDataFrame(df) // spark dataframe to frovedis dataframe
    val ret = join(fdf, exp, join_type, join_algo, rsuf)
    fdf.release()
    return ret
  }
  def join(df: DataFrame, exp: Expr,
           join_type: String, join_algo: String): FrovedisDataFrame = {
    return join(df, exp, join_type, join_algo, "_right")
  }
  def join(df: DataFrame, exp: Expr, join_type: String): FrovedisDataFrame = {
    return join(df, exp, join_type, "bcast", "_right")
  }
  def join(df: DataFrame, exp: Expr): FrovedisDataFrame = {
    return join(df, exp, "inner", "bcast", "_right")
  }

  // case 2. String: Only single key common in both [e.g., left.join(right,"A")]
  def join(df: DataFrame, key: String, 
           join_type: String, join_algo: String): FrovedisDataFrame = {
    val fdf = new FrovedisDataFrame(df) // spark dataframe to frovedis dataframe
    val ret = join(fdf, key, join_type, join_algo)
    fdf.release()
    return ret
  }
  def join(df: DataFrame, key: String, join_type: String): FrovedisDataFrame = {
    return join(df, key, join_type, "bcast")
  }
  def join(df: DataFrame, key: String): FrovedisDataFrame = {
    return join(df, key, "inner", "bcast")
  }

  // case 3. FrovedisColumn: Only Column object [e.g., left.join(right,$$"A")]
  def join(df: DataFrame, col: FrovedisColumn, 
           join_type: String, join_algo: String): FrovedisDataFrame = {
    val fdf = new FrovedisDataFrame(df) // spark dataframe to frovedis dataframe
    return join(fdf, col, join_type, join_algo)
  }
  def join(df: DataFrame, col: FrovedisColumn, join_type: String): FrovedisDataFrame = {
    return join(df, col, join_type, "bcast")
  }
  def join(df: DataFrame, col: FrovedisColumn): FrovedisDataFrame = {
    return join(df, col, "inner", "bcast")
  }

  // case 4. join with keys provided as scala Sequence
  def join(df: DataFrame, keys: Seq[String],
           join_type: String, join_algo: String,
           rsuf: String): FrovedisDataFrame = {
    if (rsuf == "") throw new IllegalArgumentException("Suffix for renaming "
                                                  + "cannot be empty string!\n")
    val fdf = new FrovedisDataFrame(df) // spark dataframe to frovedis dataframe
    return join(fdf, keys, join_type, join_algo, rsuf)
  }
  def join(df: DataFrame, keys: Seq[String],
           join_type: String, join_algo: String): FrovedisDataFrame = {
    return join(df, keys, join_type, join_algo, "_right")
  }
  def join(df: DataFrame, keys: Seq[String],
           join_type: String): FrovedisDataFrame = {  
    return join(df, keys, join_type, "bcast", "_right")
  }
  def join(df: DataFrame, keys: Seq[String]): FrovedisDataFrame = {
    return join(df, keys, "inner", "bcast", "_right")
  }
  def join(df: DataFrame): FrovedisDataFrame = {
    return crossJoin(df)
  }

  // cross-join
  def crossJoin(df: FrovedisDataFrame, rsuf: String): FrovedisDataFrame = {
    if (rsuf == "") throw new IllegalArgumentException("Suffix for renaming "
                                                  + "cannot be empty string!\n")
    val fs = FrovedisServer.getServerInstance()
    val dfopt_proxy = JNISupport.getCrossDfopt(fs.master_node)
    val opt = new DFOperator(dfopt_proxy)
    var (df_right, rename_needed) = rename_helper(df, rsuf)
    var t_cols = this.cols ++ df_right.cols
    var t_types = this.types ++ df_right.types

    val proxy = JNISupport.joinFrovedisDataframes(fs.master_node,
                this.get(), df_right.get(), opt.get(), "inner",
                "bcast")
    opt.release()
    return new FrovedisDataFrame(proxy, t_cols, t_types) 
  }
  def crossJoin(df: FrovedisDataFrame): FrovedisDataFrame = {
    return crossJoin(df, "_right")
  }

  def crossJoin(df: DataFrame, rsuf: String): FrovedisDataFrame = {
    if (rsuf == "") throw new IllegalArgumentException("Suffix for renaming "
                                                  + "cannot be empty string!\n")
    val fdf = new FrovedisDataFrame(df) // spark dataframe to frovedis dataframe
    return crossJoin(fdf, rsuf)
  }
  def crossJoin(df: DataFrame): FrovedisDataFrame = {
    return crossJoin(df, "_right")
  }
  
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
  def columns = cols
  def dtypes = cols.zip(get_types())
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
  def agg(targets: Array[FrovedisAggr]): DataFrame = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    val size = targets.length
    val res = new Array[Any](size)
    val field = new Array[StructField](size)
      for (i <- 0 to (size-1)) {
        val cname = targets(i).col_name
        val func = targets(i).func_name
        require(hasColumn(cname), "No column named: " + cname)
        var asnm = targets(i).asCol_name
        if (asnm == null) asnm = func + "(" + cname + ")"
        var ret_type = TMAPPER.func2id(func)
        if(ret_type == DTYPE.NONE) ret_type = getColumnType(cname)
        res(i) = func match {
          case "max" => converter.cast(max(cname)(0), ret_type)
          case "min" => converter.cast(min(cname)(0), ret_type)
          case "avg" => converter.cast(avg(cname)(0), ret_type)
          case "sum" => converter.cast(sum(cname)(0), ret_type)
          case "std" => converter.cast(std(cname)(0), ret_type)
          case "count" => converter.cast(count(cname)(0), ret_type)
      }
      field(i) = StructField(asnm, TMAPPER.id2field(ret_type), true)      
    }
    val row = Row.fromSeq(res.toSeq)
    val schema = StructType(field)
    return DFConverter.toDF(Array(row), schema)
  }
  def agg(x: FrovedisAggr, y: FrovedisAggr*): DataFrame = {
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
  def describe(): DataFrame = {
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
  def withColumn(cname: String, op: FrovedisColumn): FrovedisDataFrame = { // append in-place
    val fs = FrovedisServer.getServerInstance()
    val dummy = JNISupport.executeDFfunc(fs.master_node, get(), op.get_dffunc)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
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
}
