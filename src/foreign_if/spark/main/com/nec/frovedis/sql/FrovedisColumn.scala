package com.nec.frovedis.sql;

// --------
// A typeless column information for a Frovedis DataFrame
// When type of the column is not known, it will model Expr object
// --------

class FrovedisColumn extends java.io.Serializable {
  protected var col_name: String = null
  private var isDesc: Int = 0
  def this(n: String) = {
    this()
    col_name = n
    isDesc = 0
  }
  def >  (arg: Any) = new Expr(col_name, arg.toString(), OPTYPE.GT)
  def >= (arg: Any) = new Expr(col_name, arg.toString(), OPTYPE.GE)
  def <  (arg: Any) = new Expr(col_name, arg.toString(), OPTYPE.LT)
  def <= (arg: Any) = new Expr(col_name, arg.toString(), OPTYPE.LE)
  def ===(arg: Any) = new Expr(col_name, arg.toString(), OPTYPE.EQ)
  def !==(arg: Any) = new Expr(col_name, arg.toString(), OPTYPE.NE)
  //TODO: Add Unary not [def !(arg: Any)]

  def getIsDesc() = isDesc
  def setIsDesc(isDesc: Int): this.type = {
    this.isDesc = isDesc
    this
  }
  def asc(): this.type  = setIsDesc(0)
  def desc(): this.type = setIsDesc(1)
  override def toString() = col_name
}

object implicits_ {
  implicit class StringToFrovedisColumn(val sc: StringContext) {
    // spark defines "$" operator, whereas the same is supported 
    // by "$$" operator in Frovedis from Spark
    def $$(args: Any*) = new FrovedisColumn(sc.s(args: _*))
  }
}

