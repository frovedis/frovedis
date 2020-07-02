package com.nec.frovedis.sql;

import com.nec.frovedis.Jexrpc._
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
  def agg(targets: Array[FrovedisAggr]): FrovedisDataFrame = {
    if(fdata == -1) throw new IllegalArgumentException("Invalid Frovedis dataframe!")
    val size = targets.length
    val aggCols = new Array[String](size)
    val aggFuncs = new Array[String](size)
    val aggAsCols = new Array[String](size)
    val aggAsTypes = new Array[Short](size)
    for (i <- 0 to (size-1)) {
      val c = targets(i).col_name
      require(hasColumn(c), "No column named: " + c)
      aggCols(i) = c
      aggFuncs(i) = targets(i).func_name
      var asnm = targets(i).asCol_name
      if (asnm == null) asnm = aggFuncs(i) + "(" + aggCols(i) + ")"
      aggAsCols(i) = asnm
      aggAsTypes(i) = TMAPPER.func2id(aggFuncs(i))
      if(aggAsTypes(i) == DTYPE.NONE) aggAsTypes(i) = getColumnType(aggCols(i))
    }
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.aggrFrovedisDataframe(fs.master_node,get(),
                                                 groupedCols, groupedCols.length,
                                                 aggFuncs,aggCols,aggAsCols,size)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val tot_cols  = this.groupedCols ++ aggAsCols
    val tot_types = this.groupedTypes ++ aggAsTypes
    return new FrovedisDataFrame(proxy,tot_cols,tot_types)
  }
  def agg(x: FrovedisAggr, y: FrovedisAggr*): FrovedisDataFrame = {
    val arr = (Array(x) ++ y).toArray
    return agg(arr)
  }

  // --- count ---
  def count(): FrovedisDataFrame = {
    val fagg_arg = functions.count(groupedCols(0)).as("count")
    return agg(fagg_arg)
  }

  // --- max ---
  def max(cnames: Array[String]): FrovedisDataFrame = {
    val fagg_args = new Array[FrovedisAggr](cnames.length)
    for (i <- 0 until cnames.length) fagg_args(i) = functions.max(cnames(i))
    return agg(fagg_args)
  }
  def max(): FrovedisDataFrame = {
    return max(getNumericCols)
  }
  // Usage: FrovedisGroupedDF-object.max("colA", "colB") // ... any number of strings
  def max(must: String, optional: String*): FrovedisDataFrame = {
    max((Array(must) ++ optional).toArray.map(x => x.toString))
  }
  // Usage: FrovedisGroupedDF-object.max($$"colA", $$"colB") // ... any number of cols in $$"name" form
  def max(must: FrovedisColumn, optional: FrovedisColumn*): FrovedisDataFrame = {
    max((Array(must) ++ optional).toArray.map(x => x.toString))
  }

  // --- min ---
  def min(cnames: Array[String]): FrovedisDataFrame = {
    val fagg_args = new Array[FrovedisAggr](cnames.length)
    for (i <- 0 until cnames.length) fagg_args(i) = functions.min(cnames(i))
    return agg(fagg_args)
  }
  def min(): FrovedisDataFrame = {
    return min(getNumericCols)
  }
  // Usage: FrovedisGroupedDF-object.min("colA", "colB") // ... any number of strings
  def min(must: String, optional: String*): FrovedisDataFrame = {
    min((Array(must) ++ optional).toArray.map(x => x.toString))
  }
  // Usage: FrovedisGroupedDF-object.min($$"colA", $$"colB") // ... any number of cols in $$"name" form
  def min(must: FrovedisColumn, optional: FrovedisColumn*): FrovedisDataFrame = {
    min((Array(must) ++ optional).toArray.map(x => x.toString))
  }

  // --- sum ---
  def sum(cnames: Array[String]): FrovedisDataFrame = {
    val fagg_args = new Array[FrovedisAggr](cnames.length)
    for (i <- 0 until cnames.length) fagg_args(i) = functions.sum(cnames(i))
    return agg(fagg_args)
  }
  def sum(): FrovedisDataFrame = {
    return sum(getNumericCols)
  }
  // Usage: FrovedisGroupedDF-object.sum("colA", "colB") // ... any number of strings
  def sum(must: String, optional: String*): FrovedisDataFrame = {
    sum((Array(must) ++ optional).toArray.map(x => x.toString))
  }
  // Usage: FrovedisGroupedDF-object.sum($$"colA", $$"colB") // ... any number of cols in $$"name" form
  def sum(must: FrovedisColumn, optional: FrovedisColumn*): FrovedisDataFrame = {
    sum((Array(must) ++ optional).toArray.map(x => x.toString))
  }

  // --- avg ---
  def avg(cnames: Array[String]): FrovedisDataFrame = {
    val fagg_args = new Array[FrovedisAggr](cnames.length)
    for (i <- 0 until cnames.length) fagg_args(i) = functions.avg(cnames(i))
    return agg(fagg_args)
  }
  def avg(): FrovedisDataFrame = {
    return avg(getNumericCols)
  }
  // Usage: FrovedisGroupedDF-object.avg("colA", "colB") // ... any number of strings
  def avg(must: String, optional: String*): FrovedisDataFrame = {
    avg((Array(must) ++ optional).toArray.map(x => x.toString))
  }
  // Usage: FrovedisGroupedDF-object.avg($$"colA", $$"colB") // ... any number of cols in $$"name" form
  def avg(must: FrovedisColumn, optional: FrovedisColumn*): FrovedisDataFrame = {
    avg((Array(must) ++ optional).toArray.map(x => x.toString))
  }

  // --- mean ---
  def mean(cnames: Array[String]): FrovedisDataFrame = {
    val fagg_args = new Array[FrovedisAggr](cnames.length)
    for (i <- 0 until cnames.length) fagg_args(i) = functions.mean(cnames(i))
    return agg(fagg_args)
  }
  def mean(): FrovedisDataFrame = {
    return mean(getNumericCols)
  }
  // Usage: FrovedisGroupedDF-object.mean("colA", "colB") // ... any number of strings
  def mean(must: String, optional: String*): FrovedisDataFrame = {
    mean((Array(must) ++ optional).toArray.map(x => x.toString))
  }
  // Usage: FrovedisGroupedDF-object.mean($$"colA", $$"colB") // ... any number of cols in $$"name" form
  def mean(must: FrovedisColumn, optional: FrovedisColumn*): FrovedisDataFrame = {
    mean((Array(must) ++ optional).toArray.map(x => x.toString))
  }

}
