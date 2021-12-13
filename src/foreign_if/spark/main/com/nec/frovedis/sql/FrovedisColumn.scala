package com.nec.frovedis.sql;

import com.nec.frovedis.Jexrpc._
import com.nec.frovedis.matrix.DTYPE
import scala.collection.mutable


object ColKind extends java.io.Serializable {
  val DFID:   Short = 0
  val DFFUNC: Short = 1
  val DFAGG:  Short = 2
}

object OPTYPE extends java.io.Serializable {
  // --- conditional ---
  val EQ:        Short = 1
  val NE:        Short = 2
  val GT:        Short = 3
  val GE:        Short = 4
  val LT:        Short = 5
  val LE:        Short = 6
  // --- special conditional ---
  val AND:       Short = 11
  val OR:        Short = 12
  val NOT:       Short = 13
  val LIKE:      Short = 14
  val NLIKE:     Short = 15
  val ISNULL:    Short = 16
  val ISNOTNULL: Short = 17
  // --- mathematical ---
  val ADD:       Short = 21
  val SUB:       Short = 22
  val MUL:       Short = 23
  val IDIV:      Short = 24
  val FDIV:      Short = 25
  val MOD:       Short = 26
  val POW:       Short = 27
}

class FrovedisColumn extends java.io.Serializable {
  private var col_name: String = null
  private var kind: Short = ColKind.DFID
  private var proxy: Long = 0
  private var isDesc: Int = 0
  def this(n: String) = {
    this()
    col_name = n
    kind = ColKind.DFID
    val fs = FrovedisServer.getServerInstance()
    proxy = JNISupport.getIDDFfunc(fs.master_node, col_name)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
  }

  // right can be FrovedisColumn, Spark column including literals
  def this(left: FrovedisColumn, right: Any, opt: Short, kind: Short) = { 
    this()
    this.kind = kind
    val right_str = right.toString
    this.col_name = get_name(left.toString, right_str, opt)
    var leftp = left.proxy
    val is_immed = checkIsImmed(right)
    val fs = FrovedisServer.getServerInstance()
    if (is_immed) {
      this.proxy = JNISupport.getOptImmedDFfunc(fs.master_node, leftp, 
                                                right_str, DTYPE.detect(right),
                                                opt, col_name)
    } else {
      var tmp: FrovedisColumn = null
      if (!right.isInstanceOf[FrovedisColumn]) tmp = new FrovedisColumn(right_str) // for spark column
      else tmp = right.asInstanceOf[FrovedisColumn] // casting from Any
      var rightp = tmp.proxy
      this.proxy = JNISupport.getOptDFfunc(fs.master_node, leftp, rightp,
                                           opt, col_name)
    }
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
  }

  private def get_name(left: String, right: String, opt: Short): String = {
    return opt match {
      case OPTYPE.EQ => "(" + left + " = " + right + ")"
      case OPTYPE.NE => "(NOT (" + left + " = " + right + "))"
      case OPTYPE.GT => "(" + left + " > " + right + ")"
      case OPTYPE.GE => "(" + left + " >= " + right + ")"
      case OPTYPE.LT => "(" + left + " < " + right + ")"
      case OPTYPE.LE => "(" + left + " <= " + right + ")"
      case OPTYPE.AND => "(" + left + " AND " + right + ")"
      case OPTYPE.OR => "(" + left + " OR " + right + ")"
      case OPTYPE.NOT => "(NOT " + left + ")"
      case OPTYPE.LIKE => "(" + left + " LIKE " + right + ")"
      case OPTYPE.NLIKE => "(NOT (" + left + " LIKE " + right + "))"
      case OPTYPE.ISNULL => "(" + left + " IS NULL)"
      case OPTYPE.ISNOTNULL => "(" + left + " IS NOT NULL)"
      case OPTYPE.ADD => "(" + left + " + " + right + ")"
      case OPTYPE.SUB => "(" + left + " - " + right + ")"
      case OPTYPE.MUL => "(" + left + " * " + right + ")"
      case OPTYPE.IDIV => "(" + left + " // " + right + ")"
      case OPTYPE.FDIV => "(" + left + " / " + right + ")"
      case OPTYPE.MOD => "(" + left + " % " + right + ")"
      case OPTYPE.POW => "(" + left + " ** " + right + ")"
      case _ => throw new IllegalArgumentException("Unsupported opt-type: " + opt)
    }
  }

  def >   (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.GT,  ColKind.DFFUNC) 
  def >=  (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.GE,  ColKind.DFFUNC) 
  def <   (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.LT,  ColKind.DFFUNC) 
  def <=  (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.LE,  ColKind.DFFUNC) 
  def === (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.EQ,  ColKind.DFFUNC) 
  def !== (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.NE,  ColKind.DFFUNC) 
  def =!= (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.NE,  ColKind.DFFUNC) 

  def &&  (arg: FrovedisColumn) = new FrovedisColumn(this, arg, OPTYPE.AND, ColKind.DFFUNC) 
  def and (arg: FrovedisColumn) = new FrovedisColumn(this, arg, OPTYPE.AND, ColKind.DFFUNC) 
  def ||  (arg: FrovedisColumn) = new FrovedisColumn(this, arg, OPTYPE.OR,  ColKind.DFFUNC) 
  def or  (arg: FrovedisColumn) = new FrovedisColumn(this, arg, OPTYPE.OR,  ColKind.DFFUNC) 

  def like       (arg: String) = new FrovedisColumn(this, arg, OPTYPE.LIKE,  ColKind.DFFUNC) 
  def not_like   (arg: String) = new FrovedisColumn(this, arg, OPTYPE.NLIKE, ColKind.DFFUNC) // added specially
  def startsWith (arg: String) = new FrovedisColumn(this, arg + "%", OPTYPE.LIKE, ColKind.DFFUNC)
  def endsWith   (arg: String) = new FrovedisColumn(this, "%" + arg, OPTYPE.LIKE, ColKind.DFFUNC)

  def dummy     = new FrovedisColumn("0") // for dummy right
  def unary_!   = new FrovedisColumn(this, dummy, OPTYPE.NOT, ColKind.DFFUNC) 
  def isNull    = new FrovedisColumn(this, dummy, OPTYPE.ISNULL, ColKind.DFFUNC) 
  def isNotNull = new FrovedisColumn(this, dummy, OPTYPE.ISNOTNULL, ColKind.DFFUNC) 

  // TODO: support other mathematical operators...
  def + (right: Any) = new FrovedisColumn(this, right, OPTYPE.ADD,  ColKind.DFFUNC)
  def - (right: Any) = new FrovedisColumn(this, right, OPTYPE.SUB,  ColKind.DFFUNC)
  def * (right: Any) = new FrovedisColumn(this, right, OPTYPE.MUL,  ColKind.DFFUNC)
  def / (right: Any) = new FrovedisColumn(this, right, OPTYPE.FDIV, ColKind.DFFUNC)
  def % (right: Any) = new FrovedisColumn(this, right, OPTYPE.MOD,  ColKind.DFFUNC)
  def **(right: Any) = new FrovedisColumn(this, right, OPTYPE.POW,  ColKind.DFFUNC)

  def unary_- : FrovedisColumn = { // special case
    val ret = new FrovedisColumn()
    ret.kind = ColKind.DFFUNC
    val right_str = "-1.0"
    ret.col_name = "(- " + this.toString + ")"
    val fs = FrovedisServer.getServerInstance()
    ret.proxy = JNISupport.getOptImmedDFfunc(fs.master_node, this.proxy, 
                                             right_str, DTYPE.DOUBLE,
                                             OPTYPE.MUL, col_name)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return ret
  }

  def as(new_name: String): this.type = {
    this.col_name = new_name
    val fs = FrovedisServer.getServerInstance()
    JNISupport.setDFfuncAsColName(fs.master_node, proxy, col_name)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    this
  }
  def getIsDesc() = isDesc
  def setIsDesc(isDesc: Int): this.type = {
    this.isDesc = isDesc
    this
  }
  def asc(): this.type  = setIsDesc(0)
  def desc(): this.type = setIsDesc(1)
  private def checkIsImmed(arg: Any): Boolean = { // TODO: support literal case
    return !(arg.isInstanceOf[com.nec.frovedis.sql.FrovedisColumn] ||
             arg.isInstanceOf[org.apache.spark.sql.ColumnName])
  }
  def get() = proxy
  def colName = col_name 
  def isID = (kind == ColKind.DFID)
  def isFUNC = (kind == ColKind.DFFUNC)
  def isAGG = (kind == ColKind.DFAGG)
  override def toString = col_name
}

object implicits_ {
  implicit class StringToFrovedisColumn(val sc: StringContext) {
    // spark defines "$" operator, whereas the same is supported 
    // by "$$" operator in Frovedis from Spark
    def $$(args: Any*) = new FrovedisColumn(sc.s(args: _*))
  }
}

