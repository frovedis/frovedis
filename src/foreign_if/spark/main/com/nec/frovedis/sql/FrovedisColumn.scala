package com.nec.frovedis.sql;

import com.nec.frovedis.Jexrpc._
import scala.collection.mutable.ArrayBuffer
// --------
// A typeless column information for a Frovedis DataFrame
// When type of the column is not known, it will model Expr object
// --------

class FrovedisColumn extends java.io.Serializable {
  protected var col_name: String = null
  private var isDesc: Int = 0
  private var as_name: String = null
  private var dffunc_proxy: Long = 0
  private var is_opt: Boolean = false
  def this(n: String) = {
    this()
    col_name = n
    is_opt = false
    val fs = FrovedisServer.getServerInstance()
    dffunc_proxy = JNISupport.getIDDFfunc(fs.master_node, col_name)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
  }
  def this(left: FrovedisColumn, right: Any, opt: Short) = { 
    // right can be FrovedisColumn, Spark column including literals
    this()
    is_opt = true
    val sign = get_opt_mark(opt)
    val right_str = right.toString()
    col_name = "(" + left.col_name + sign + right_str + ")"

    var leftp = left.dffunc_proxy
    val is_immed = checkIsImmed(right)
    val fs = FrovedisServer.getServerInstance()
    if (is_immed) {
      // right_str would be casted as double...
      // TODO: check if type information can be used
      dffunc_proxy = JNISupport.getOptImmedDFfunc(fs.master_node, leftp, 
                                                  right_str, opt, col_name)
    } else {
      var tmp: FrovedisColumn = null
      if (!right.isInstanceOf[FrovedisColumn]) tmp = new FrovedisColumn(right_str) // for spark column
      else tmp = right.asInstanceOf[FrovedisColumn] // casting from Any
      var rightp = tmp.dffunc_proxy
      dffunc_proxy = JNISupport.getOptDFfunc(fs.master_node, leftp, rightp,
                                             opt, col_name)
    }
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
  }

  private def get_opt_mark(opt: Short): String = {
    return opt match {
      case OPTYPE.ADD => " + " 
      case OPTYPE.SUB => " - " 
      case OPTYPE.MUL => " * " 
      case OPTYPE.IDIV => " // " 
      case OPTYPE.FDIV => " / " 
      case OPTYPE.MOD => " % " 
      case OPTYPE.POW => " ** " 
      case OPTYPE.GT => " > " 
      case OPTYPE.GE => " >= " 
      case OPTYPE.LT => " < " 
      case OPTYPE.LE => " <= " 
      case OPTYPE.EQ => " === " 
      case OPTYPE.NE => " !== " 
      case _ => throw new IllegalArgumentException("Unsupported opt-type: " + opt)
    }
  }

  def >  (arg: Any) = new Expr(col_name, arg.toString(), OPTYPE.GT, checkIsImmed(arg)) 
  def >= (arg: Any) = new Expr(col_name, arg.toString(), OPTYPE.GE, checkIsImmed(arg))
  def <  (arg: Any) = new Expr(col_name, arg.toString(), OPTYPE.LT, checkIsImmed(arg))
  def <= (arg: Any) = new Expr(col_name, arg.toString(), OPTYPE.LE, checkIsImmed(arg))
  def ===(arg: Any) = new Expr(col_name, arg.toString(), OPTYPE.EQ, checkIsImmed(arg))
  def !==(arg: Any) = new Expr(col_name, arg.toString(), OPTYPE.NE, checkIsImmed(arg))

  def +(right: Any) = new FrovedisColumn(this, right, OPTYPE.ADD)
  def -(right: Any) = new FrovedisColumn(this, right, OPTYPE.SUB)
  def *(right: Any) = new FrovedisColumn(this, right, OPTYPE.MUL)
  def /(right: Any) = new FrovedisColumn(this, right, OPTYPE.FDIV)
  // TODO: support other operators...


  def like(pattern: String) = new Expr(col_name, pattern, OPTYPE.LIKE, true)
  def startsWith(pattern: String) = new Expr(col_name, pattern + "%", OPTYPE.LIKE, true)
  def endsWith(pattern: String) = new Expr(col_name, "%" + pattern, OPTYPE.LIKE, true)

  def as(new_name: String): this.type = {
    if (is_opt) {
      // rename isformation is updated in dffunction; so reflected during execute()
      val fs = FrovedisServer.getServerInstance()
      JNISupport.setDFfuncAsColName(fs.master_node, dffunc_proxy, col_name)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      //col_name = new_name // TODO: uncomment after supporting the set in server
      as_name = new_name // actual rename would take place on action (like select etc.)
    } else {
      as_name = new_name // actual rename would take place on action (like select etc.)
    }
    this
  }
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
  def isOpt = is_opt 
  def asName = as_name 
  def get_dffunc = dffunc_proxy
}

object implicits_ {
  implicit class StringToFrovedisColumn(val sc: StringContext) {
    // spark defines "$" operator, whereas the same is supported 
    // by "$$" operator in Frovedis from Spark
    def $$(args: Any*) = new FrovedisColumn(sc.s(args: _*))
  }
}

object DFColUtils {
  def get_opt_cols(cols: Array[FrovedisColumn]): Array[String] = {
    val ret = new ArrayBuffer[String]()
    for(i <- 0 until cols.size) if (cols(i).isOpt) ret += cols(i).toString
    return ret.toArray
  }
  def get_rename_targets(cols: Array[FrovedisColumn]): 
    (Array[String], Array[String]) = {
    var names = new ArrayBuffer[String]()
    var new_names = new ArrayBuffer[String]()
    for(i <- 0 until cols.size) {
      val c = cols(i)
      if (c.asName != null) {
        names += c.toString
        new_names += c.asName
      }
    }
    return (names.toArray, new_names.toArray)
  }
}
