package com.nec.frovedis.sql;

import com.nec.frovedis.Jexrpc._
import com.nec.frovedis.Jmllib.DummyDftable
import com.nec.frovedis.matrix.DTYPE
import com.nec.frovedis.matrix.{
  IntDvector, LongDvector, 
  FloatDvector, DoubleDvector,
  StringDvector, WordsNodeLocal,
  BoolDvector}
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.matrix.FrovedisColmajorMatrix
import com.nec.frovedis.matrix.TimeSpent
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.mllib.ModelID
import org.apache.spark.rdd.RDD
import org.apache.spark.SparkContext
import org.apache.spark.mllib.linalg.{Vector,Vectors}
import org.apache.spark.sql.SparkSession
import org.apache.spark.sql.Row
import org.apache.spark.sql.types._
import org.apache.spark.sql.DataFrame
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

  def toTypedDvector(df: DataFrame, dtype: Short, i: Int, 
                     part_sizes: RDD[Int]): Long = {
    val col_name = df.columns(i)
    val t0 = new TimeSpent(Level.DEBUG)
    return dtype match {
        case DTYPE.INT => { 
           val null_replaced = df.select(col_name).na.fill(Int.MaxValue)
           t0.show("null_replaced: ")
           val data = null_replaced.map(_.getInt(0)).rdd
           t0.show("get rdd: ")
           val ret = IntDvector.get(data, part_sizes)
           t0.show("get intDvector: ")
           ret
        }
        case DTYPE.LONG => { 
           val null_replaced = df.select(col_name).na.fill(Long.MaxValue)
           t0.show("null_replaced: ")
           val data = null_replaced.map(_.getLong(0)).rdd
           t0.show("get rdd: ")
           val ret = LongDvector.get(data, part_sizes)
           t0.show("get longDvector: ")
           ret
        }
        case DTYPE.FLOAT => { 
           val null_replaced = df.select(col_name).na.fill(Float.MaxValue)
           t0.show("null_replaced: ")
           val data = null_replaced.map(_.getFloat(0)).rdd
           t0.show("get rdd: ")
           val ret = FloatDvector.get(data, part_sizes)
           t0.show("get floatDvector: ")
           ret
        }
        case DTYPE.DOUBLE => { 
           val null_replaced = df.select(col_name).na.fill(Double.MaxValue)
           t0.show("null_replaced: ")
           val data = null_replaced.map(_.getDouble(0)).rdd
           t0.show("get rdd: ")
           val ret = DoubleDvector.get(data, part_sizes)
           t0.show("get doubleDvector: ")
           ret
        }
        case DTYPE.STRING => {
           val null_replaced = df.select(col_name).na.fill("NULL")
           t0.show("null_replaced: ")
           val data = null_replaced.map(_.getString(0)).rdd
           t0.show("get rdd: ")
           val ret = StringDvector.get(data, part_sizes)
           t0.show("get stringDvector: ")
           ret
        }
        case DTYPE.WORDS => { // use instead of StringDvector for better performance
           val null_replaced = df.select(col_name).na.fill("NULL")
           t0.show("null_replaced: ")
           val data = null_replaced.map(_.getString(0)).rdd
           t0.show("get rdd: ")
           val ret = WordsNodeLocal.get(data, part_sizes)
           t0.show("get wordsNodeLocal: ")
           ret
        }
        case DTYPE.BOOL => {
           // data is passed as int-vector
           var data = df.select(col_name).map(x => bool2int(x(0))).rdd
           t0.show("get rdd: ")
           val ret = IntDvector.get(data, part_sizes)
           t0.show("get booleanDvector: ")
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

class FrovedisDataFrame extends java.io.Serializable {
  protected var fdata: Long = -1
  protected var cols: Array[String] = null
  protected var types: Array[Short] = null

  // creating a frovedis dataframe from a spark dataframe -> User API
  def this(df: DataFrame) = {
    this ()
    load (df)
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
  // loading data from a spark dataframe to a frovedis dataframe -> User API
  def load (df: DataFrame) : Unit = {
    /** releasing the old data (if any) */
    release()
    cols = df.dtypes.map(_._1)
    val tt = df.dtypes.map(_._2)
    val size = cols.size
    var dvecs = new Array[Long](size)
    types = new Array[Short](size)
    val part_sizes = df.rdd.mapPartitions(x => Array(x.size).toIterator).persist
    for (i <- 0 to (size-1)) {
      //print("col_name: " + cols(i) + " col_type: " + tt(i) + "\n")
      val tname = tt(i)
      val dtype = TMAPPER.string2id(tname)
      types(i) = dtype
      dvecs(i) = TMAPPER.toTypedDvector(df, dtype, i, part_sizes)
    }
    val fs = FrovedisServer.getServerInstance()
    fdata = JNISupport.createFrovedisDataframe(fs.master_node,types,cols,dvecs,size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
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
      x.toString
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
