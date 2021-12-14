package com.nec.frovedis.sql;

import com.nec.frovedis.Jexrpc._
import com.nec.frovedis.matrix.DTYPE
import scala.collection.mutable


object ColKind extends java.io.Serializable {
  val DFID:     Short = 0
  val DFFUNC:   Short = 1
  val DFAGG:    Short = 2
  val DFSCALAR: Short = 3
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
  // --- aggregator ---
  val aMAX:      Short = 41
  val aMIN:      Short = 42
  val aSUM:      Short = 43
  val aAVG:      Short = 44
  val aVAR:      Short = 45
  val aSEM:      Short = 46
  val aSTD:      Short = 47
  val aMAD:      Short = 48
  val aCNT:      Short = 49
  val aSIZE:     Short = 50
}

class FrovedisColumn extends java.io.Serializable {
  private var col_name: String = null
  private var kind: Short = ColKind.DFID
  private var proxy: Long = 0
  private var isDesc: Int = 0
  private var dtype: Short = 0 // would be set for scalar

  def this(n: String) = {
    this()
    this.col_name = n
    this.kind = ColKind.DFID
    val fs = FrovedisServer.getServerInstance()
    this.proxy = JNISupport.getIDDFfunc(fs.master_node, col_name)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
  }

  // right can be FrovedisColumn, Spark column including literals
  def this(left: FrovedisColumn, right: Any, opt: Short) = { 
    this()
    this.kind = ColKind.DFFUNC
    val right_str = right.toString
    this.col_name = get_name(left.col_name, right_str, opt)
    if (left.isSCALAR) // TODO: support left as literal
      throw new IllegalArgumentException(this.col_name + ": 'left as literal' is currently not supported!\n")
    var leftp = left.proxy
    val is_immed = checkIsImmed(right)
    val fs = FrovedisServer.getServerInstance()
    var tmp: FrovedisColumn = null
    if (right.isInstanceOf[FrovedisColumn]) tmp = right.asInstanceOf[FrovedisColumn] // casting from Any
    if (is_immed) {
      var im_dt: Short = 0
      if (tmp == null) im_dt = DTYPE.detect(right) 
      else { // tmp is an instance of FrovedisColumn and it must be a SCALAR
        if (tmp.colName.equals("NULL"))  
          throw new IllegalArgumentException(this.col_name + 
          ": is not supported! Use isNull/isNotNull instead.\n")
        im_dt = tmp.get_dtype()
      }
      this.proxy = JNISupport.getOptImmedDFfunc(fs.master_node, leftp, 
                                                right_str, im_dt,
                                                opt, col_name)
    } else {
      if (!right.isInstanceOf[FrovedisColumn]) tmp = new FrovedisColumn(right_str) // for spark column
      var rightp = tmp.proxy
      this.proxy = JNISupport.getOptDFfunc(fs.master_node, leftp, rightp,
                                           opt, col_name)
    }
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
  }

  def mark_scalar(dtype: Short): this.type = {
    this.kind = ColKind.DFSCALAR
    this.dtype = dtype
    this
  }

  def get_agg(agg: Short): FrovedisColumn = {
    val ret = new FrovedisColumn()
    ret.kind = ColKind.DFAGG
    ret.col_name = get_name(this.col_name, "", agg)
    if (this.isSCALAR) // TODO: support this as literal
      throw new IllegalArgumentException(ret.col_name + ": 'aggregator on literal' is currently not supported!\n")
    val fs = FrovedisServer.getServerInstance()
    ret.proxy = JNISupport.getDFagg(fs.master_node, this.proxy,
                                    agg, ret.col_name)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return ret
  }

  private def get_name(left: String, right: String, opt: Short): String = {
    return opt match {
      // --- conditional ---
      case OPTYPE.EQ => "(" + left + " = " + right + ")"
      case OPTYPE.NE => "(NOT (" + left + " = " + right + "))"
      case OPTYPE.GT => "(" + left + " > " + right + ")"
      case OPTYPE.GE => "(" + left + " >= " + right + ")"
      case OPTYPE.LT => "(" + left + " < " + right + ")"
      case OPTYPE.LE => "(" + left + " <= " + right + ")"
      // --- special conditional ---
      case OPTYPE.AND => "(" + left + " AND " + right + ")"
      case OPTYPE.OR => "(" + left + " OR " + right + ")"
      case OPTYPE.NOT => "(NOT " + left + ")"
      case OPTYPE.LIKE => "(" + left + " LIKE " + right + ")"
      case OPTYPE.NLIKE => "(NOT (" + left + " LIKE " + right + "))"
      case OPTYPE.ISNULL => "(" + left + " IS NULL)"
      case OPTYPE.ISNOTNULL => "(" + left + " IS NOT NULL)"
      // --- mathematical ---
      case OPTYPE.ADD => "(" + left + " + " + right + ")"
      case OPTYPE.SUB => "(" + left + " - " + right + ")"
      case OPTYPE.MUL => "(" + left + " * " + right + ")"
      case OPTYPE.IDIV => "(" + left + " // " + right + ")"
      case OPTYPE.FDIV => "(" + left + " / " + right + ")"
      case OPTYPE.MOD => "(" + left + " % " + right + ")"
      case OPTYPE.POW => "(" + left + " ** " + right + ")"
      // --- aggregator ---
      case OPTYPE.aMAX  => "max(" + left + ")"
      case OPTYPE.aMIN  => "min(" + left + ")"
      case OPTYPE.aSUM  => "sum(" + left + ")"
      case OPTYPE.aAVG  => "avg(" + left + ")"
      case OPTYPE.aVAR  => "var_samp(" + left + ")"
      case OPTYPE.aSEM  => "sem(" + left + ")"
      case OPTYPE.aSTD  => "stddev_samp(" + left + ")"
      case OPTYPE.aMAD  => "mad(" + left + ")"
      case OPTYPE.aCNT  => "count(" + left + ")"
      case OPTYPE.aSIZE => "count(1)"
      case _ => throw new IllegalArgumentException("Unsupported opt-type: " + opt)
    }
  }

  def >   (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.GT) 
  def >=  (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.GE) 
  def <   (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.LT) 
  def <=  (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.LE) 
  def === (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.EQ) 
  def !== (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.NE) 
  def =!= (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.NE) 

  def &&  (arg: FrovedisColumn) = new FrovedisColumn(this, arg, OPTYPE.AND) 
  def and (arg: FrovedisColumn) = new FrovedisColumn(this, arg, OPTYPE.AND) 
  def ||  (arg: FrovedisColumn) = new FrovedisColumn(this, arg, OPTYPE.OR) 
  def or  (arg: FrovedisColumn) = new FrovedisColumn(this, arg, OPTYPE.OR) 

  def like       (arg: String) = new FrovedisColumn(this, arg, OPTYPE.LIKE) 
  def not_like   (arg: String) = new FrovedisColumn(this, arg, OPTYPE.NLIKE) // added specially
  def startsWith (arg: String) = new FrovedisColumn(this, arg + "%", OPTYPE.LIKE)
  def endsWith   (arg: String) = new FrovedisColumn(this, "%" + arg, OPTYPE.LIKE)

  def dummy     = new FrovedisColumn("0") // for dummy right
  def unary_!   = new FrovedisColumn(this, dummy, OPTYPE.NOT) 
  def isNull    = new FrovedisColumn(this, dummy, OPTYPE.ISNULL) 
  def isNotNull = new FrovedisColumn(this, dummy, OPTYPE.ISNOTNULL) 

  // TODO: support other mathematical operators...
  def + (right: Any) = new FrovedisColumn(this, right, OPTYPE.ADD)
  def - (right: Any) = new FrovedisColumn(this, right, OPTYPE.SUB)
  def * (right: Any) = new FrovedisColumn(this, right, OPTYPE.MUL)
  def / (right: Any) = new FrovedisColumn(this, right, OPTYPE.FDIV)
  def % (right: Any) = new FrovedisColumn(this, right, OPTYPE.MOD)
  def **(right: Any) = new FrovedisColumn(this, right, OPTYPE.POW)

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
    if (isSCALAR)
      throw new IllegalArgumentException("as: currently is not supported for literals!\n")
    else if (isAGG)
      JNISupport.setDFAggAsColName(fs.master_node, proxy, col_name)
    else
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
  private def checkIsImmed(arg: Any): Boolean = {
    if (arg.isInstanceOf[FrovedisColumn]) {
      return arg.asInstanceOf[FrovedisColumn].isSCALAR
    } else {
      return !(arg.isInstanceOf[org.apache.spark.sql.ColumnName])
    }
  }
  def get()    = proxy
  def colName  = col_name 
  def isID     = (kind == ColKind.DFID)
  def isFUNC   = (kind == ColKind.DFFUNC)
  def isAGG    = (kind == ColKind.DFAGG)
  def isSCALAR = (kind == ColKind.DFSCALAR)
  def get_dtype(): Short = {
    if (isSCALAR) return this.dtype
    else throw new IllegalArgumentException("get_dtype: cannot detect dtype for non-literals!\n")
  }
  override def toString = col_name
}

object implicits_ {
  implicit class StringToFrovedisColumn(val sc: StringContext) {
    // spark defines "$" operator, whereas the same is supported 
    // by "$$" operator in Frovedis from Spark
    def $$(args: Any*) = new FrovedisColumn(sc.s(args: _*))
  }
}

object functions extends java.io.Serializable {
  def not(e: FrovedisColumn)   = !e
  def col(col: String)         = new FrovedisColumn(col)
  def column(col: String)      = new FrovedisColumn(col)
  def asc(col: String)         = new FrovedisColumn(col).asc
  def desc(col: String)        = new FrovedisColumn(col).desc
  def lit(x: Any): FrovedisColumn = {
    if (x == null) return new FrovedisColumn("NULL").mark_scalar(DTYPE.INT) // null as INT.MAX
    else           return new FrovedisColumn(x.toString).mark_scalar(DTYPE.detect(x))
  }

  def max      (col: String) = new FrovedisColumn(col).get_agg(OPTYPE.aMAX)
  def min      (col: String) = new FrovedisColumn(col).get_agg(OPTYPE.aMIN)
  def sum      (col: String) = new FrovedisColumn(col).get_agg(OPTYPE.aSUM)
  def avg      (col: String) = new FrovedisColumn(col).get_agg(OPTYPE.aAVG)
  def variance (col: String) = new FrovedisColumn(col).get_agg(OPTYPE.aVAR)
  def sem      (col: String) = new FrovedisColumn(col).get_agg(OPTYPE.aSEM)
  def stddev   (col: String) = new FrovedisColumn(col).get_agg(OPTYPE.aSTD)
  def mad      (col: String) = new FrovedisColumn(col).get_agg(OPTYPE.aMAD)
  def count    (col: String): FrovedisColumn = {
    if (col.equals("*")) return new FrovedisColumn(col).get_agg(OPTYPE.aSIZE)
    else                 return new FrovedisColumn(col).get_agg(OPTYPE.aCNT)
  }

  def max      (col: FrovedisColumn) = col.get_agg(OPTYPE.aMAX)
  def min      (col: FrovedisColumn) = col.get_agg(OPTYPE.aMIN)
  def sum      (col: FrovedisColumn) = col.get_agg(OPTYPE.aSUM)
  def avg      (col: FrovedisColumn) = col.get_agg(OPTYPE.aAVG)
  def variance (col: FrovedisColumn) = col.get_agg(OPTYPE.aVAR)
  def sem      (col: FrovedisColumn) = col.get_agg(OPTYPE.aSEM)
  def stddev   (col: FrovedisColumn) = col.get_agg(OPTYPE.aSTD)
  def mad      (col: FrovedisColumn) = col.get_agg(OPTYPE.aMAD)
  def count    (col: FrovedisColumn) = col.get_agg(OPTYPE.aCNT)

  // --- alias aggregate functions ---
  def mean(col: String)                = avg(col)
  def mean(col: FrovedisColumn)        = avg(col)
  def stddev_samp(col: String)         = stddev(col)
  def stddev_samp(col: FrovedisColumn) = stddev(col)
  def var_samp(col: String)            = variance(col)
  def var_samp(col: FrovedisColumn)    = variance(col)
}

