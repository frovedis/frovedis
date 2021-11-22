package com.nec.frovedis.sql;

// --------
// A typeless column information for a Frovedis DataFrame
// When type of the column is not known, it will model Expr object
// --------

class FrovedisColumn extends java.io.Serializable {
  protected var col_name: String = null
  private var isDesc: Int = 0
  private var as_name: String = null
  def this(n: String) = {
    this()
    col_name = n
  }
  
  def >  (arg: Any) = new Expr(col_name, arg.toString(), OPTYPE.GT, checkIsImmed(arg)) 
  def >= (arg: Any) = new Expr(col_name, arg.toString(), OPTYPE.GE, checkIsImmed(arg))
  def <  (arg: Any) = new Expr(col_name, arg.toString(), OPTYPE.LT, checkIsImmed(arg))
  def <= (arg: Any) = new Expr(col_name, arg.toString(), OPTYPE.LE, checkIsImmed(arg))
  def ===(arg: Any) = new Expr(col_name, arg.toString(), OPTYPE.EQ, checkIsImmed(arg))
  def !==(arg: Any) = new Expr(col_name, arg.toString(), OPTYPE.NE, checkIsImmed(arg))
  def like(pattern: String) = new Expr(col_name, pattern, OPTYPE.LIKE, true)
  def startsWith(pattern: String) = new Expr(col_name, pattern + "%", OPTYPE.LIKE, true)
  def endsWith(pattern: String) = new Expr(col_name, "%" + pattern, OPTYPE.LIKE, true)

  def as(new_name: String): this.type = {
    as_name = new_name // actual rename would take place on action (like select etc.)
    this
  }
  def asName: String = as_name 
  def getIsDesc() = isDesc
  def setIsDesc(isDesc: Int): this.type = {
    this.isDesc = isDesc
    this
  }
  def asc(): this.type  = setIsDesc(0)
  def desc(): this.type = setIsDesc(1)
  override def toString() = col_name
  private def checkIsImmed(arg: Any): Boolean = {
    return !(arg.isInstanceOf[com.nec.frovedis.sql.FrovedisColumn] ||
             arg.isInstanceOf[org.apache.spark.sql.ColumnName])
  }
}

object implicits_ {
  implicit class StringToFrovedisColumn(val sc: StringContext) {
    // spark defines "$" operator, whereas the same is supported 
    // by "$$" operator in Frovedis from Spark
    def $$(args: Any*) = new FrovedisColumn(sc.s(args: _*))
  }
}

