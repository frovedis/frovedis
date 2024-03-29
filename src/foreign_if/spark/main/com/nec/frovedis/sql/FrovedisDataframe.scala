package com.nec.frovedis.sql;

import com.nec.frovedis.Jexrpc._
import com.nec.frovedis.Jmllib.DummyDftable
import com.nec.frovedis.matrix.{TimeSpent, DTYPE}
import com.nec.frovedis.matrix.{
  IntDvector, LongDvector, 
  FloatDvector, DoubleDvector,
  StringDvector, WordsNodeLocal,
  BoolDvector}
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.matrix.FrovedisColmajorMatrix
import com.nec.frovedis.Jsql.FrovedisDataFrameFinalizer
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.mllib.ModelID
import org.apache.spark.rdd.RDD
import org.apache.spark.SparkContext
import org.apache.spark.mllib.linalg.{Vector,Vectors}
import org.apache.spark.sql.SparkSession
import org.apache.spark.sql.Row
import org.apache.spark.sql.types._
import org.apache.spark.sql.functions.{col => sp_col}
import org.apache.spark.sql.functions.{unix_timestamp, from_unixtime, to_date, to_timestamp}
import org.apache.spark.sql.DataFrame
import org.apache.spark.sql.vectorized.ColumnarBatch
import org.apache.spark.sql.catalyst.InternalRow
import scala.collection.mutable.{Map => mMap}
import scala.collection.mutable.ArrayBuffer
import scala.collection.mutable.ListBuffer
import org.apache.log4j.Level
import java.util.TimeZone

class FrovedisDataFrame extends java.io.Serializable {
  protected var fdata: Long = -1
  protected var cols: Array[String] = null
  protected var types: Array[Short] = null
  private var code: Int = 0
  var owned_cols: Array[String] = null
  @transient var ref: FrovedisDataFrameFinalizer = null
  var mem_size: Long = 0
  
  // creating a frovedis dataframe from a spark dataframe -> User API
  def this(df: DataFrame) = {
    this()
    //load(df)
    //this.code = df.hashCode
    val cols = df.columns
    const_impl(df, cols)
    this.ref = FrovedisDataFrameFinalizer.addObject(this)
  }
  def this(df: DataFrame, name: String, others: String*) = {
    this()
    val cols = (Array(name) ++ others).toArray
    const_impl(df, cols)
    this.ref = FrovedisDataFrameFinalizer.addObject(this)
  }
  // internally used, not exposed to user
  def this(proxy: Long, cc: Array[String], tt: Array[Short]) = {
    this()
    fdata = proxy
    cols = cc.clone()
    types = tt.clone()
    this.ref = FrovedisDataFrameFinalizer.addObject(this)
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
    this.ref = FrovedisDataFrameFinalizer.addObject(this)
  }
  private def const_impl(df: DataFrame, cols: Array[String]): this.type = {
    val t1 = new TimeSpent(Level.DEBUG)
    val dtypes_map = df.dtypes.toMap
    val code = df.hashCode // assumed to be unique per spark dataframe object
    val base_ptr = new ArrayBuffer[Long]()
    val base_col = new ArrayBuffer[String]()
    val numeric_targets = new ArrayBuffer[String]()
    val non_numeric_targets = new ArrayBuffer[String]()
    var toSend = true
    val numeric_types = Array("IntegerType", "LongType", "FloatType", "DoubleType", "BooleanType")
    for (i <- 0 until cols.size) {
      val c = cols(i)
      val key = (code, c)
      val fdf = DFMemoryManager.get(key)
      if (fdf == null) toSend = true
      else {
        if (fdf.get() != -1) { // not released forcibly
          base_ptr += fdf.get()
          base_col += c 
          toSend = false
        }
        else {
          toSend = true
        }
      }
      if (toSend) {
        if (numeric_types.contains(dtypes_map(c))) numeric_targets += c
        else                                       non_numeric_targets += c
      }
    }
    t1.show("column look-up: ")

    var columnar_targets = new Array[String](0)
    if (!numeric_targets.isEmpty) columnar_targets = numeric_targets.toArray

    var irow_targets = new Array[String](0)
    if (!non_numeric_targets.isEmpty) irow_targets = non_numeric_targets.toArray

    if (!columnar_targets.isEmpty) {
      val sdf = df.select(columnar_targets.map(x => sp_col(x)):_*)
      val columnar = sDFTransfer.get_columnar(sdf)
      t1.show("columnar creation: ")
      if (columnar != null) {
        this.owned_cols = columnar_targets
        columnar_load(sdf, columnar_targets, columnar) // TODO: send cols, types, columnar
      }
      else { // columnar creation might fail, if sdf doesn't have ColumnarSupport
        irow_targets = irow_targets ++ columnar_targets
      }
    }

    if (!irow_targets.isEmpty) {
      if (this.owned_cols != null) this.owned_cols = this.owned_cols ++ irow_targets
      else                         this.owned_cols = irow_targets
      val sdf = df.select(irow_targets.map(x => sp_col(x)):_*)
      optimized_load(sdf)
    }

    val t2 = new TimeSpent(Level.DEBUG)
    if (!base_ptr.isEmpty) { // TODO: correct column order
      //println("*** cols hit ***")
      //base_col.foreach(println)
      copy_column_inplace(this.fdata, base_ptr.toArray, base_col.toArray)
      t2.show("column copy: ")
    }

    val fs = FrovedisServer.getServerInstance()
    this.mem_size = JNISupport.calcMemorySize(fs.master_node, this.fdata)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    t2.show("memory size calculation: ")

    if (owned_cols != null) {
      DFMemoryManager.insert(code, this, owned_cols.toIterator)
      t2.show("DFMemoryManager registration: ")
    }
    this.code = code
    this
  }

  private def copy_column_inplace(fdata: Long, 
                                  base_dfs: Array[Long],
                                  t_cols: Array[String]): this.type = {
    require(base_dfs.size == t_cols.size, "copy_column_inplace: size mismatch detected!")
    val fs = FrovedisServer.getServerInstance()
    val dummy = JNISupport.copyColumn(fs.master_node, fdata, 
                                      base_dfs, t_cols, base_dfs.size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    this.fdata = dummy.dfptr
    this.cols = dummy.names.clone()
    this.types = dummy.types.clone() // TODO: mark bool/ulong
    this
  }

  private def copy_column_inplace(fdata: Long, 
                                  base_df: Long,
                                  t_cols: Array[String]): this.type = {
    val size = t_cols.size
    val base_dfs = new Array[Long](size)
    for(i <- 0 until size) base_dfs(i) = base_df
    copy_column_inplace(fdata, base_dfs, t_cols)
    this
  }

  def is_removable = (owned_cols == null)

  // to release a frovedis dataframe from frovedis server -> User API
  def release () : Unit = {
    if (fdata != -1) {
      // non-removable: loaded from spark dataframe, hence kept for future use
      // would be managed by DFMemoryManager
      if (!is_removable) DFMemoryManager.add_release_entry(this) 
      else {
        ref.release()
        ref = null
        fdata = -1
        cols = null
        types = null
        owned_cols = null
        code = 0
        mem_size = 0
      }
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

  @deprecated ("load: it has slow performance issue and limited support for DataType")
  def load (df: DataFrame) : this.type = {
    release()
    val name_type_pair = df.dtypes
    this.cols = name_type_pair.map(_._1)
    this.types = name_type_pair.map(_._2).map(x => TMAPPER.string2id(x))

    val ncol = cols.size
    val dvecs = new Array[Long](ncol)    
    val rddData = sDFTransfer.toInternalRow(df)
    val part_sizes = rddData.mapPartitions(x => Array(x.size).toIterator).persist
    val do_align = false // will take care by append_column
    for (i <- 0 until ncol) dvecs(i) = Dvec.get(rddData, types(i), i, part_sizes, do_align)

    val fs = FrovedisServer.getServerInstance()
    this.fdata = JNISupport.createFrovedisDataframe(fs.master_node, types, 
                                                    cols, dvecs, ncol)
    var info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    this.code = df.hashCode
    this
  }

  // TODO: improve existing limitations: 
  //   - might cause bad-alloc in ve-memory when target columns in 'df' are too large to fit
  def optimized_load (df: DataFrame) : this.type = {
    release()

    val t1 = new TimeSpent(Level.DEBUG)
    val rddData = sDFTransfer.toInternalRow(df)
    t1.show("[optimized_load] toRdd: ")

    val name_type_pair = df.dtypes
    val cols = name_type_pair.map(_._1)
    val types = name_type_pair.map(_._2).map(x => TMAPPER.string2id(x))
    val ncol = cols.size
    val offset = new Array[Int](ncol)
    var word_count = 0
    for (i <- 0 until ncol) {
      offset(i) = i + word_count
      word_count += (if (types(i) == DTYPE.WORDS) 1 else 0)
    }
    t1.show("[optimized_load] initialization: ")

    val proxy = sDFTransfer.load_rows(rddData, cols, types, 
                                      word_count, offset, t1)

    if (this.fdata == -1) { // empty case
      this.fdata = proxy
      this.cols = cols
      this.types = types
    } else {
      copy_column_inplace(this.fdata, proxy, cols) // this.fdata[cols] = proxy[cols]
      val fs = FrovedisServer.getServerInstance()
      JNISupport.releaseFrovedisDataframe(fs.master_node, proxy)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      t1.show("[optimized_load] copy column: ")
    }
    this.code = df.hashCode
    this
  }

  def columnar_load (df: DataFrame, tcols: Array[String],
                     columnar: RDD[ColumnarBatch]): this.type = {
    release()
    val t1 = new TimeSpent(Level.DEBUG)
    val dfcols = df.columns
    val name_type_map = df.dtypes.toMap
    val cols = tcols
    val types = cols.map(x => TMAPPER.string2id(name_type_map(x)))
    val colIds = cols.map(x => dfcols.indexOf(x))
    val ncol = cols.size
    //for (i <- 0 until ncol) println(cols(i) + " -> " + types(i) + " -> " + colIds(i))
    val offset = new Array[Int](ncol)
    var word_count = 0
    for (i <- 0 until ncol) {
      offset(i) = i + word_count
      word_count += (if (types(i) == DTYPE.WORDS) 1 else 0)
    }
    t1.show("[columnar_load] initialization: ")
    val proxy = sDFTransfer.load_columnar(columnar, cols, colIds, types,
                                          word_count, offset, t1)
    if (this.fdata == -1) { // empty case
      this.fdata = proxy
      this.cols = cols
      this.types = types
    } else {
      copy_column_inplace(this.fdata, proxy, cols) // this.fdata[cols] = proxy[cols]
      val fs = FrovedisServer.getServerInstance()
      JNISupport.releaseFrovedisDataframe(fs.master_node, proxy)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      t1.show("[columnar_load] copy column: ")
    }
    this.code = df.hashCode
    this
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
    return new FrovedisDataFrame(proxy, targets, tt)
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
    var aggcnt = 0
    for (i <- 0 until size) { 
      val tmp = c_arr(i)
      funcs(i) = tmp.get()
      aggcnt = aggcnt + (if (tmp.isAGG) 1 else 0)
    }
    val fs = FrovedisServer.getServerInstance()
    var dummy: DummyDftable = null
    if (aggcnt > 0) {
      if (aggcnt != size)
        throw new IllegalArgumentException(
        "select: few non-aggregator functions are detected!")
      dummy = JNISupport.executeFrovedisAgg(fs.master_node, get(), funcs, size)
    } else {
      dummy = JNISupport.fselectFrovedisDataframe(fs.master_node, get(),
                                                  funcs, size)
    }
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)

    // --- update column types ---
    val name_type_map = this.dtypes_as_map
    for (i <- 0 until size) {
      val org_nm = c_arr(i).orgName
      val dtype = c_arr(i).get_dtype()
      if (dtype != DTYPE.NONE) dummy.types(i) = dtype
      else if (name_type_map.contains(org_nm)) dummy.types(i) = name_type_map(org_nm)
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
                          "left_outer" -> "outer")
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
    val ns_to_s = 1000L * 1000L * 1000L
    val ts_offset = TimeZone.getDefault().getRawOffset() * 1000L * 1000L // offset in nanoseconds
    var need_conversion = false
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
        case DTYPE.DATETIME | DTYPE.TIMESTAMP => {
          need_conversion = true
          LongDvector.to_RDD(cptr).map(x => (x - ts_offset) / ns_to_s)
        }
        case _  => throw new IllegalArgumentException("to_spark_DF: Invalid " +
                                                    "datatype encountered: %s !\n"
                                                    .format(tid))
      }
      JNISupport.releaseDFColumnPointer(fs.master_node, cptr, tid)
      info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      ret_lb += col_rdd
    }
    var ret = ret_lb.toArray.asInstanceOf[Array[RDD[Any]]]
    val maxValueMap = Map(DTYPE.INT -> Int.MaxValue, DTYPE.LONG -> Long.MaxValue,
                      DTYPE.FLOAT -> Float.MaxValue, DTYPE.DOUBLE -> Double.MaxValue,
                      DTYPE.STRING -> "NULL", DTYPE.WORDS -> "NULL", 
                      DTYPE.BOOL -> Int.MaxValue, DTYPE.DATETIME -> Long.MaxValue,
                      DTYPE.TIMESTAMP -> Long.MaxValue)

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
        case DTYPE.DATETIME => StructField(cname, LongType, true)
        case DTYPE.TIMESTAMP => StructField(cname, LongType, true)
        case _  => throw new IllegalArgumentException("to_spark_DF: Invalid " +
                                                    "datatype encountered: %s !\n"
                                                    .format(tid))
      }
      df_schema += col_field
    }
    val resRdd = combine_rows_as_partitions(ret.toSeq)
    var res_df = spark.createDataFrame(resRdd, StructType(df_schema.toArray))

    if (need_conversion) {
      val dtypes_map = this.dtypes_as_map
      val new_columns =
        res_df.columns
              .map(x => dtypes_map(x) match {
                case DTYPE.DATETIME  => to_date(to_timestamp(sp_col(x))).as(x)
                case DTYPE.TIMESTAMP => to_timestamp(sp_col(x)).as(x)
                case _               => sp_col(x)
              })
      res_df = res_df.select(new_columns:_*)
    }
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
  private def mark_boolean_and_timestamp_columns(
   dt: mMap[String, Short], dummy: DummyDftable): Unit = {
    val bool_id = DTYPE.BOOL
    val timestamp_id = DTYPE.TIMESTAMP
    
    val ncol = dummy.names.size
    for (i <- 0 until ncol) {
      val c = dummy.names(i)
      if (dt.contains(c)) {
        if (dt(c) == bool_id) dummy.types(i) = bool_id
        else if (dt(c) == timestamp_id) dummy.types(i) = timestamp_id
      }
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
    return new FrovedisDataFrame(proxy, new_cols, types)
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
    mark_boolean_and_timestamp_columns(this.dtypes_as_map, dummy)
    if (op.get_dtype() != DTYPE.NONE) { // mark newly added column dtype correctly
      val ncol = dummy.names.size
      dummy.types(ncol - 1) = op.get_dtype()
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
    mark_boolean_and_timestamp_columns(this.dtypes_as_map, dummy)
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
    return new FrovedisDataFrame(dummy.dfptr, cols, types)
  }

  // not full-proof definition (just for proxy check) 
  override def equals(obj: Any): Boolean = {
    var df2 = obj.asInstanceOf[FrovedisDataFrame]
    return this.get() == df2.get()
  }
}
