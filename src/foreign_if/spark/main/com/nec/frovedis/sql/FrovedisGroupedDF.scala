package com.nec.frovedis.sql;

import com.nec.frovedis.Jexrpc._
import com.nec.frovedis.Jmllib.DummyDftable
import com.nec.frovedis.matrix.DTYPE
import scala.collection.mutable.ArrayBuffer

class FrovedisGroupedDF extends java.io.Serializable {
  protected var fdata: Long = -1
  protected var cols: Array[String] = null
  protected var types: Array[Short] = null
  protected var groupedCols: Array[String] = null
  protected var groupedTypes: Array[Short] = null
  protected var numericCols: Array[String] = null

  def this(proxy: Long, 
           cc: Array[String], tt: Array[Short], 
           g_cc: Array[String], g_tt: Array[Short]) = {
    this()
    fdata = proxy
    cols = cc.clone()
    types = tt.clone()
    groupedCols = g_cc.clone()
    groupedTypes = g_tt.clone()
  }
  def release () : Unit = {
    if (fdata != -1) {
      val fs = FrovedisServer.getServerInstance()
      JNISupport.releaseFrovedisGroupedDF(fs.master_node,fdata)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      fdata = -1
      cols = null
      types = null
      groupedCols = null
      groupedTypes = null
      numericCols = null
    }
  }
  def getNumericCols(): Array[String] = {
    if(this.numericCols == null) {
      var numericColsBuffer = ArrayBuffer[String]() 
      for (i <- 0 until cols.length) {
        if ( !groupedCols.contains(cols(i)) && types(i) != DTYPE.STRING ){
          numericColsBuffer += cols(i)
        }
      }
      this.numericCols = numericColsBuffer.toArray      
    }
    return this.numericCols
  }
  def getColumnType(c: String): Short = {
    val index = cols.indexOf(c)
    if (index == -1)  throw new IllegalArgumentException("No column named: " + c)
    return types(index)
  }
  def hasColumn(c: String) = cols.indexOf(c) != -1
  def get() = fdata

  // --- aggregate functions ---
  def agg(targets: Array[FrovedisColumn]): FrovedisDataFrame = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    val size = targets.length
    val aggp  = new ArrayBuffer[Long]()
    val funcp = new ArrayBuffer[Long]()
    for(i <- 0 until size) {
      val t = targets(i)
      if (t.isAGG)  aggp += t.get()
      else {
        // exclude dffunctions which is already included in groupedCols
        if (!(groupedCols contains t.colName)) funcp += t.get()
      }
    }
    val aggp_arr  = aggp.toArray
    var funcp_arr = funcp.toArray
    val fs = FrovedisServer.getServerInstance()
    var dummy: DummyDftable = null
    if (funcp_arr.size > 0) {
      // convert groupedCols as FrovedisColumn
      val gsize = groupedCols.size
      val gcolsp = new Array[Long](gsize)
      for (i <- 0 until gsize) gcolsp(i) = functions.col(groupedCols(i)).get()
      // merge groupedCols with input dffunctions
      funcp_arr = (gcolsp ++ funcp_arr).toArray
      if (aggp_arr.size > 0) {
        dummy = JNISupport.aggFselectFrovedisGroupedData(fs.master_node, get(),
                                                 funcp_arr, funcp_arr.size,
                                                 aggp_arr, aggp_arr.size)
      } else {
        dummy = JNISupport.FselectFrovedisGroupedData(fs.master_node, get(),
                                              funcp_arr, funcp_arr.size)
      }
    } else {
      dummy = JNISupport.aggSelectFrovedisGroupedData(fs.master_node, get(),
                                              groupedCols, groupedCols.size,
                                              aggp_arr, aggp_arr.size)
    }
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisDataFrame(dummy)
  }
  def agg(x: FrovedisColumn, y: FrovedisColumn*): 
    FrovedisDataFrame = agg((Array(x) ++ y).toArray)

  def select(targets: Array[String]): FrovedisDataFrame = {
    val size = targets.length
    val types = new Array[Short](size)
    for (i <- 0 to (size-1)) types(i) = getColumnType(targets(i))
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.selectFrovedisGroupedData(fs.master_node,get(),
                                                     targets, size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisDataFrame(proxy,targets,types)
  }
  // Usage: df.select("colA")
  // Usage: df.select("colA", "colB") ... any number of column names
  def select(must: String, opt: String*): 
    FrovedisDataFrame = select((Array(must) ++ opt).toArray)
  
  // --- count ---
  def count(): FrovedisDataFrame = agg(functions.count(groupedCols(0)).as("count"))

  // --- max ---
  def max(cnames: Array[String]): FrovedisDataFrame = {
    val fagg_args = new Array[FrovedisColumn](cnames.length)
    for (i <- 0 until cnames.length) fagg_args(i) = functions.max(cnames(i))
    return agg(fagg_args)
  }

  def max(): FrovedisDataFrame = max(getNumericCols)

  // Usage: FrovedisGroupedDF-object.max("colA", "colB") // ... any number of strings
  def max(must: String, opt: String*): 
    FrovedisDataFrame = max((Array(must) ++ opt).toArray)

  // Usage: FrovedisGroupedDF-object.max($$"colA", $$"colB") // ... any number of cols in $$"name" form
  def max(must: FrovedisColumn, opt: FrovedisColumn*): FrovedisDataFrame = {
    return agg((Array(must) ++ opt).toArray.map(x => functions.max(x)))
  }

  // --- min ---
  def min(cnames: Array[String]): FrovedisDataFrame = {
    val fagg_args = new Array[FrovedisColumn](cnames.length)
    for (i <- 0 until cnames.length) fagg_args(i) = functions.min(cnames(i))
    return agg(fagg_args)
  }

  def min(): FrovedisDataFrame = min(getNumericCols)

  // Usage: FrovedisGroupedDF-object.min("colA", "colB") // ... any number of strings
  def min(must: String, opt: String*):
    FrovedisDataFrame = min((Array(must) ++ opt).toArray)
  
  // Usage: FrovedisGroupedDF-object.min($$"colA", $$"colB") // ... any number of cols in $$"name" form
  def min(must: FrovedisColumn, opt: FrovedisColumn*): FrovedisDataFrame = {
    return agg((Array(must) ++ opt).toArray.map(x => functions.min(x)))
  }

  // --- sum ---
  def sum(cnames: Array[String]): FrovedisDataFrame = {
    val fagg_args = new Array[FrovedisColumn](cnames.length)
    for (i <- 0 until cnames.length) fagg_args(i) = functions.sum(cnames(i))
    return agg(fagg_args)
  }

  def sum(): FrovedisDataFrame = sum(getNumericCols)
  
  // Usage: FrovedisGroupedDF-object.sum("colA", "colB") // ... any number of strings
  def sum(must: String, opt: String*): 
    FrovedisDataFrame = sum((Array(must) ++ opt).toArray)
 
  // Usage: FrovedisGroupedDF-object.sum($$"colA", $$"colB") // ... any number of cols in $$"name" form
  def sum(must: FrovedisColumn, opt: FrovedisColumn*): FrovedisDataFrame = {
    return agg((Array(must) ++ opt).toArray.map(x => functions.sum(x)))
  }

  // --- avg ---
  def avg(cnames: Array[String]): FrovedisDataFrame = {
    val fagg_args = new Array[FrovedisColumn](cnames.length)
    for (i <- 0 until cnames.length) fagg_args(i) = functions.avg(cnames(i))
    return agg(fagg_args)
  }

  def avg(): FrovedisDataFrame = avg(getNumericCols)
  
  // Usage: FrovedisGroupedDF-object.avg("colA", "colB") // ... any number of strings
  def avg(must: String, opt: String*): 
    FrovedisDataFrame = avg((Array(must) ++ opt).toArray)
  
  // Usage: FrovedisGroupedDF-object.avg($$"colA", $$"colB") // ... any number of cols in $$"name" form
  def avg(must: FrovedisColumn, opt: FrovedisColumn*): FrovedisDataFrame = {
    return agg((Array(must) ++ opt).toArray.map(x => functions.avg(x)))
  }

  // --- mean ---
  def mean(cnames: Array[String]): FrovedisDataFrame = {
    val fagg_args = new Array[FrovedisColumn](cnames.length)
    for (i <- 0 until cnames.length) fagg_args(i) = functions.mean(cnames(i))
    return agg(fagg_args)
  }

  def mean(): FrovedisDataFrame = mean(getNumericCols)
  
  // Usage: FrovedisGroupedDF-object.mean("colA", "colB") // ... any number of strings
  def mean(must: String, opt: String*): 
    FrovedisDataFrame = mean((Array(must) ++ opt).toArray)
  
  // Usage: FrovedisGroupedDF-object.mean($$"colA", $$"colB") // ... any number of cols in $$"name" form
  def mean(must: FrovedisColumn, opt: FrovedisColumn*): FrovedisDataFrame = {
    return agg((Array(must) ++ opt).toArray.map(x => functions.mean(x)))
  }
}
