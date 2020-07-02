package com.nec.frovedis.sql;

class FrovedisAggr extends java.io.Serializable {
  var col_name: String = null
  var asCol_name: String = null
  var func_name: String = null
  def this(c: String, f: String) = {
    this()
    col_name = c
    func_name = f
  }
  def as(asCol: String): this.type = {
    asCol_name = asCol
    return this
  }
}

object functions {
  def max(col: String) = new FrovedisAggr(col,"max")
  def min(col: String) = new FrovedisAggr(col,"min")
  def sum(col: String) = new FrovedisAggr(col,"sum")
  def avg(col: String) = new FrovedisAggr(col,"avg")
  def mean(col: String) = new FrovedisAggr(col,"mean")
  def std(col: String) = new FrovedisAggr(col,"std")
  def count(col: String) = new FrovedisAggr(col,"count")
  def max(col: FrovedisColumn) = new FrovedisAggr(col.toString(),"max")
  def min(col: FrovedisColumn) = new FrovedisAggr(col.toString(),"min")
  def sum(col: FrovedisColumn) = new FrovedisAggr(col.toString(),"sum")
  def avg(col: FrovedisColumn) = new FrovedisAggr(col.toString(),"avg")
  def mean(col: FrovedisColumn) = new FrovedisAggr(col.toString(),"mean")
  def std(col: FrovedisColumn) = new FrovedisAggr(col.toString(),"std")
  def count(col: FrovedisColumn) = new FrovedisAggr(col.toString(),"count")
  def col(colName: String): FrovedisColumn = new FrovedisColumn(colName)
  def column(colName: String): FrovedisColumn = new FrovedisColumn(colName)
  def asc(columnName: String): FrovedisColumn = new FrovedisColumn(columnName).asc
  def desc(columnName: String): FrovedisColumn = new FrovedisColumn(columnName).desc
}
