package com.nec.frovedis.sql;

import com.nec.frovedis.Jexrpc._
import com.nec.frovedis.matrix.DTYPE
import com.nec.frovedis.matrix.{IntDvector,LongDvector}
import com.nec.frovedis.matrix.{FloatDvector,DoubleDvector}
import com.nec.frovedis.matrix.{StringDvector,BoolDvector}
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.matrix.FrovedisColmajorMatrix
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.mllib.ModelID
import org.apache.spark.rdd.RDD
import org.apache.spark.SparkContext
import org.apache.spark.mllib.linalg.{Vector,Vectors}
import org.apache.spark.sql.SparkSession
import org.apache.spark.sql.Row
import org.apache.spark.sql.types.StructField
import org.apache.spark.sql.types._
import org.apache.spark.sql.DataFrame
import scala.collection.mutable.ArrayBuffer

object TMAPPER {

  val func2id = Map("sum" -> DTYPE.NONE, "max" -> DTYPE.NONE, "min" -> DTYPE.NONE,
                    "avg" -> DTYPE.DOUBLE, "count" -> DTYPE.LONG)

  val id2val = Map(DTYPE.INT -> "IntegerType",   DTYPE.LONG -> "LongType",
                   DTYPE.FLOAT -> "FloatType",   DTYPE.DOUBLE -> "DoubleType",
                   DTYPE.STRING -> "StringType", DTYPE.BOOL -> "BooleanType")

  val val2id = Map("IntegerType" -> DTYPE.INT,    "LongType" -> DTYPE.LONG,
                   "FloatType"   -> DTYPE.FLOAT,  "DoubleType" -> DTYPE.DOUBLE,
                   "StringType"  -> DTYPE.STRING, "BooleanType" -> DTYPE.BOOL)

  val spark = SparkSession.builder.getOrCreate()
  import spark.implicits._

  def toTypedDvector(df: DataFrame, tname: String, i: Int): Long = {
    return tname match {
        case "IntegerType" => { 
           val data = df.map(_.getInt(i)).rdd
           IntDvector.get(data)
        }
        case "LongType" => { 
           val data = df.map(_.getLong(i)).rdd
           LongDvector.get(data)
        }
        case "FloatType" => { 
           val data = df.map(_.getFloat(i)).rdd
           FloatDvector.get(data)
        }
        case "DoubleType" => { 
           val data = df.map(_.getDouble(i)).rdd
           DoubleDvector.get(data)
        }
        case "StringType" => {
           val data = df.map(_.getString(i)).rdd
           StringDvector.get(data)
        }
        case "BooleanType" => {
           val data = df.map(_.getBoolean(i)).rdd
           BoolDvector.get(data)
        }
        case _ => throw new IllegalArgumentException("Unsupported type: " + tname)
      }
  }
}

object DFConverter {
  def setEachIntPartition(nid: Int, 
                          wnode: Node,
                          dptr: Long): Iterator[Int] = {
    val lvec = JNISupport.getLocalIntVector(wnode,dptr)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return lvec.toIterator
  }
  def to_Int_RDD(proxy: Long, cname: String,
                 dummy: RDD[Boolean],
                 mnode: Node, nodes: Array[Node]): RDD[Int] = {
    val eps = JNISupport.getLocalIntColumnPointers(mnode, proxy, cname)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = dummy.mapPartitionsWithIndex((i,x) =>
                    setEachIntPartition(i,nodes(i),eps(i)))
    return ret
  }
  def setEachLongPartition(nid: Int, 
                          wnode: Node,
                          dptr: Long): Iterator[Long] = {
    val lvec = JNISupport.getLocalLongVector(wnode,dptr)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return lvec.toIterator
  }
  def to_Long_RDD(proxy: Long, cname: String,
                 dummy: RDD[Boolean],
                 mnode: Node, nodes: Array[Node]): RDD[Long] = {
    val eps = JNISupport.getLocalLongColumnPointers(mnode, proxy, cname)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = dummy.mapPartitionsWithIndex((i,x) =>
                    setEachLongPartition(i,nodes(i),eps(i)))
    return ret
  }
  def setEachFloatPartition(nid: Int, 
                          wnode: Node,
                          dptr: Long): Iterator[Float] = {
    val lvec = JNISupport.getLocalFloatVector(wnode,dptr)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return lvec.toIterator
  }
  def to_Float_RDD(proxy: Long, cname: String,
                 dummy: RDD[Boolean],
                 mnode: Node, nodes: Array[Node]): RDD[Float] = {
    val eps = JNISupport.getLocalFloatColumnPointers(mnode, proxy, cname)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = dummy.mapPartitionsWithIndex((i,x) =>
                    setEachFloatPartition(i,nodes(i),eps(i)))
    return ret
  }
  def setEachDoublePartition(nid: Int, 
                          wnode: Node,
                          dptr: Long): Iterator[Double] = {
    val lvec = JNISupport.getLocalDoubleVector(wnode,dptr)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return lvec.toIterator
  }
  def to_Double_RDD(proxy: Long, cname: String,
                 dummy: RDD[Boolean],
                 mnode: Node, nodes: Array[Node]): RDD[Double] = {
    val eps = JNISupport.getLocalDoubleColumnPointers(mnode, proxy, cname)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = dummy.mapPartitionsWithIndex((i,x) =>
                    setEachDoublePartition(i,nodes(i),eps(i)))
    return ret
  }
  def setEachStringPartition(nid: Int, 
                          wnode: Node,
                          dptr: Long): Iterator[String] = {
    val lvec = JNISupport.getLocalStringVector(wnode,dptr)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return lvec.toIterator
  }
  def to_String_RDD(proxy: Long, cname: String,
                 dummy: RDD[Boolean],
                 mnode: Node, nodes: Array[Node]): RDD[String] = {
    val eps = JNISupport.getLocalStringColumnPointers(mnode, proxy, cname)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val ret = dummy.mapPartitionsWithIndex((i,x) =>
                    setEachStringPartition(i,nodes(i),eps(i)))
    return ret
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
  // loading data from a spark dataframe to a frovedis dataframe -> User API
  def load (df: DataFrame) : Unit = {
    /** releasing the old data (if any) */
    release()
    cols = df.dtypes.map(_._1)
    val tt = df.dtypes.map(_._2)
    val size = cols.size
    var dvecs = new Array[Long](size)
    types = new Array[Short](size)
    for (i <- 0 to (size-1)) {
      //print("col_name: " + cols(i) + " col_type: " + tt(i) + "\n")
      val tname = tt(i)
      types(i) = TMAPPER.val2id(tname)
      dvecs(i) = TMAPPER.toTypedDvector(df,tname,i)
    }
    val fs = FrovedisServer.getServerInstance()
    fdata = JNISupport.createFrovedisDataframe(fs.master_node,types,cols,dvecs,size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
  }
  // --- Frovedis server side FILTER definition here ---
  // Usage: df.filter(df.col("colA") > 10)
  // Usage: df.filter(df("colA") > 10)
  def filter(opt: DFOperator): FrovedisDataFrame = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.filterFrovedisDataframe(fs.master_node,this.get(),opt.get())
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisDataFrame(proxy,cols,types)
  }
  // Usage: df.filter($$"colA" > 10)
  def filter(expr: Expr): FrovedisDataFrame = {
    val dfopt = new DFOperator(expr,cols,types)
    val ret = filter(dfopt)
    dfopt.release()
    return ret
  }
  // Usage: df.when(df.col("colA") > 10)
  // Usage: df.when(df("colA") > 10)
  def when(opt: DFOperator): FrovedisDataFrame = {
    return filter(opt)
  }
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
  // Usage: df.select($$"colA", $$"colB")
  def select(c: FrovedisColumn*): FrovedisDataFrame = {
    val all = c.toArray.map(x => x.toString)
    return select(all)
  } // TODO: Support of expression like [df.select($"colA", $"colB" + 1)]

  // --- Frovedis server side SORT definition here ---
  // Usage: df.sort(Array("colA", "colB"), true) // for descending sorting
  // Usage: df.sort(Array("colA", "colB"))       // for ascending sorting
  def sort(targets: Array[String], isDesc: Boolean = false): FrovedisDataFrame = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    val size = targets.size
    for (i <- 0 to (size-1)) 
      if(!hasColumn(targets(i))) 
        throw new IllegalArgumentException("No column named: " + targets(i)) 
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.sortFrovedisDataframe(fs.master_node,get(),targets,size,isDesc)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisDataFrame(proxy,cols,types)
  }
  // Usage: df.sort("colA") 
  // Usage: df.sort("colA", "colB") // ... any number of strings
  def sort(must: String, optional: String*): FrovedisDataFrame = {
    val all = (Array(must) ++ optional).toArray.map(x => x.toString)
    return sort(all)
  }
  // Usage: df.sort($$"colA", $$"colB") // ... any number of cols in $$"name" form
  def sort(c: FrovedisColumn*): FrovedisDataFrame = {
    val all = c.toArray.map(x => x.toString)
    return sort(all)
  } // TODO: Support column-wise asc/desc sorting [e.g., df.select($"colA".asc, $"colB".desc)]

  // --- Frovedis server side GROUP_BY definition here ---
  // Usage: df.groupBy(Array("colA", "colB")) 
  def groupBy(targets: Array[String]): FrovedisDataFrame = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    val size = targets.size
    val tt = getColumnTypes(targets) // throws exception, if colname not found
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.groupFrovedisDataframe(fs.master_node,get(),targets,size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisDataFrame(proxy,targets,tt)
  }
  // Usage: df.groupBy("colA") 
  // Usage: df.groupBy("colA", "colB") // ... any number of strings
  def groupBy(must: String, optional: String*): FrovedisDataFrame = {
    val all = (Array(must) ++ optional).toArray.map(x => x.toString)
    return groupBy(all)
  }
  // Usage: df.groupBy($$"colA", $$"colB") // ... any number of cols in $$"name" form
  def groupBy(c: FrovedisColumn*): FrovedisDataFrame = {
    val all = c.toArray.map(x => x.toString)
    return groupBy(all)
  }

  // --- Frovedis server side JOIN definition here ---
  // --- When right is a Frovedis Dataframe ---
  // case 1. DFOperator: from FrovedisTypedColumn 
  // For example, left.join(right,left("A") === right("B"))
  //              left.join(right,left.col("A") === right.col("B"))
  def join(df: FrovedisDataFrame, opt: DFOperator, 
           join_type: String, join_algo: String): FrovedisDataFrame = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.joinFrovedisDataframes(fs.master_node,
                this.get(),df.get(),opt.get(),join_type,join_algo)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    // TODO: remove redundant names
    val new_cols = this.cols ++ df.cols
    val new_types = this.types ++ df.types
    return new FrovedisDataFrame(proxy,new_cols,new_types)
  }
  def join(df: FrovedisDataFrame, opt: DFOperator, join_type: String): FrovedisDataFrame = {
    return join(df,opt,join_type,"bcast") 
  }
  def join(df: FrovedisDataFrame, opt: DFOperator): FrovedisDataFrame = {
    return join(df,opt,"inner","bcast")
  }

  // case 2. Expr: from FrovedisColumn 
  // For example, left.join(right,$$"A" === $$"B")
  def join(df: FrovedisDataFrame, exp: Expr,
           join_type: String, join_algo: String): FrovedisDataFrame = {
    // converting key to DFOperator
    //exp.display() // debug
    val t_cols = this.cols ++ df.cols
    val t_types = this.types ++ df.types
    val opt = new DFOperator(exp,t_cols,t_types) 
    val ret = join(df, opt, join_type, join_algo)
    opt.release()
    return ret
  }
  def join(df: FrovedisDataFrame, exp: Expr, join_type: String): FrovedisDataFrame = {
    return join(df,exp,join_type,"bcast")
  }
  def join(df: FrovedisDataFrame, exp: Expr): FrovedisDataFrame = {
    return join(df,exp,"inner","bcast")
  }

  // case 3. String: Only single key common in both [e.g., left.join(right,"A")]
  def join(df: FrovedisDataFrame, key: String,
           join_type: String, join_algo: String): FrovedisDataFrame = {
    // converting key to DFOperator
    val opt = (this.col(key) === df.col(key)) 
    val ret = join(df, opt, join_type, join_algo)
    opt.release()
    return ret
  }
  def join(df: FrovedisDataFrame, key: String, join_type: String): FrovedisDataFrame = {
    return join(df, key, join_type, "bcast")
  }
  def join(df: FrovedisDataFrame, key: String): FrovedisDataFrame = {
    return join(df, key, "inner", "bcast")
  }

  // case 4. FrovedisColumn: Only Column object [e.g., left.join(right,$$"A")]
  def join(df: FrovedisDataFrame, col: FrovedisColumn,
           join_type: String, join_algo: String): FrovedisDataFrame = {
    return join(df, col.toString(), join_type, join_algo)
  }
  def join(df: FrovedisDataFrame, col: FrovedisColumn, join_type: String): FrovedisDataFrame = {
    return join(df, col.toString(), join_type, "bcast")
  }
  def join(df: FrovedisDataFrame, col: FrovedisColumn): FrovedisDataFrame = {
    return join(df, col.toString(), "inner", "bcast")
  }

  // --- When right is a Spark Dataframe ---
  // case 1. DFOperator: from FrovedisTypedColumn 
  // For example left.join(right,left("A") === right("B"))
  //             left.join(right,left.col("A") === right.col("B"))
  def join(df: DataFrame, opt: DFOperator, 
           join_type: String, join_algo: String): FrovedisDataFrame = {
    val fdf = new FrovedisDataFrame(df) // spark dataframe to frovedis dataframe
    val ret = join(fdf, opt, join_type, join_algo)
    fdf.release()
    return ret
  }
  def join(df: DataFrame, opt: DFOperator, join_type: String): FrovedisDataFrame = {
    return join(df, opt, join_type, "bcast")
  }
  def join(df: DataFrame, opt: DFOperator): FrovedisDataFrame = {
    return join(df, opt, "inner", "bcast")
  }

  // case 2. Expr: from FrovedisColumn [e.g., left.join(right,$$"A" === $$"B")]
  def join(df: DataFrame, exp: Expr, 
           join_type: String, join_algo: String): FrovedisDataFrame = {
    val fdf = new FrovedisDataFrame(df) // spark dataframe to frovedis dataframe
    val ret = join(fdf, exp, join_type, join_algo)
    fdf.release()
    return ret
  }
  def join(df: DataFrame, exp: Expr, join_type: String): FrovedisDataFrame = {
    return join(df, exp, join_type, "bcast")
  }
  def join(df: DataFrame, exp: Expr): FrovedisDataFrame = {
    return join(df, exp, "inner", "bcast")
  }

  // case 3. String: Only single key common in both [e.g., left.join(right,"A")]
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

  // case 4. FrovedisColumn: Only Column object [e.g., left.join(right,$$"A")]
  def join(df: DataFrame, col: FrovedisColumn, 
           join_type: String, join_algo: String): FrovedisDataFrame = {
    return join(df, col.toString(), join_type, join_algo)
  }
  def join(df: DataFrame, col: FrovedisColumn, join_type: String): FrovedisDataFrame = {
    return join(df, col.toString(), join_type, "bcast")
  }
  def join(df: DataFrame, col: FrovedisColumn): FrovedisDataFrame = {
    return join(df, col.toString(), "inner", "bcast")
  }

/** 
  * TODO: difficulty in implementation due to 
  * datatype dependency of the dataframe columns and 
  * number of candidates for zipping
  def to_spark_DF(context: SparkContext): DataFrame = {
    if(fdata == -1)  throw new IllegalArgumentException("Invalid Frovedis Dataframe!\n")
    val size = cols.size
    val fs = FrovedisServer.getServerInstance()
    val mnode = fs.master_node
    val wnodes = JNISupport.getWorkerInfo(mnode)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val tmp = new Array[Boolean](wnodes.size)
    val dummy = context.parallelize(tmp,wnodes.size)
    var ret = new Array[Any](size)
    for (i <- 0 to (size-1)) {
      val tid = types(i)
      val cname = cols(i)
      ret(i) =  tid match {
        case DTYPE.INT    => DFConverter.to_Int_RDD(get(),cname,dummy,mnode,wnodes) 
        case DTYPE.LONG   => DFConverter.to_Long_RDD(get(),cname,dummy,mnode,wnodes) 
        case DTYPE.FLOAT  => DFConverter.to_Float_RDD(get(),cname,dummy,mnode,wnodes) 
        case DTYPE.DOUBLE => DFConverter.to_Double_RDD(get(),cname,dummy,mnode,wnodes) 
        case DTYPE.STRING => DFConverter.to_String_RDD(get(),cname,dummy,mnode,wnodes) 
      }
    }
    val spark = SparkSession.builder().getOrCreate()
    import spark.implicits._
    return size match {
      case 1 => ret(0).toDF(cols(0)) 
      case 2 => ret(0).zip(ret(1)).toDF(cols(0),cols(1))
      case 3 => ret(0) zip ret(1) zip ret(2) map {
                  case ((a,b),c) => (a,b,c)
                }.toDF(cols(0),cols(1),cols(2))
      case 4 => ret(0) zip ret(1) zip ret(2) zip ret(3) map {
                  case (((a,b),c),d) => (a,b,c,d)
                }.toDF(cols(0),cols(1),cols(2),cols(3))
      case 5 => ret(0) zip ret(1) zip ret(2) zip ret(3) zip ret(4) map {
                  case ((((a,b),c),d),e) => (a,b,c,d,e)
                }.toDF(cols(0),cols(1),cols(2),cols(3),cols(4))
    }
  }
*/

  private def get_types(): Array[String] = {
    val size = types.size
    var ret = new Array[String](size)
    for (i <- 0 to (size-1)) ret(i) = TMAPPER.id2val(types(i))
    return ret
  }

  def col(t: String): FrovedisTypedColumn = {
    return new FrovedisTypedColumn(t,getColumnType(t))
  }
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
    if (tids contains DTYPE.STRING) 
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
    if (tids contains DTYPE.STRING) 
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
    if (tids contains DTYPE.STRING) 
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
    if (tids contains DTYPE.STRING) 
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
  def std(cname: Array[String],
          tids: Array[Short]): Array[String] = {
    if(fdata == -1)  throw new IllegalArgumentException("Invalid Frovedis Dataframe!\n")
    if (tids contains DTYPE.STRING)
      throw new IllegalArgumentException("String-typed column given for std!\n")
    val size = cname.length
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.getFrovedisDFStds(fs.master_node,get(),cname,tids,size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    else return ret
  }
  def std(cname: Array[String]): Array[String] = {
    val tt = getColumnTypes(cname) // throws exception, if colname not found
    return std(cname, tt)
  }
  def std(cname: String*): Array[String] = {
    val all = cname.toArray.map(x => x.toString)
    return std(all)
  }
  def get_numeric_dtypes(): Array[(Short,String)] = {
    val size = types.length
    var ret  = new ArrayBuffer[(Short,String)]()
    for(i <- 0 to (size-1)) {
      val tid = types(i)
      if(tid != DTYPE.STRING) { // only-numeric
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
    val stds = Array("stddev") ++ std(targets,tids)
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
    val proxy = JNISupport.renameFrovedisDataframe(fs.master_node,get(),
                                                   name,new_name,size)
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
}
