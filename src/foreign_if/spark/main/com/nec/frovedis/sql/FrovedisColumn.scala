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
  val ID:        Short = 0 // for column-name or scalar
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
  val IF       : Short = 18
  val ELIF     : Short = 19
  val ELSE     : Short = 20
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
  val aDSUM:     Short = 51
  val aDCNT:     Short = 52
  val aFST:      Short = 53
  val aLST:      Short = 54
}

class FrovedisColumn extends java.io.Serializable {
  private var init_name: String = null
  private var col_name: String = null // would be updated if as() is performed
  private var kind: Short = ColKind.DFID
  private var opType: Short = OPTYPE.ID
  private var proxy: Long = 0
  private var isBool: Boolean = false
  private var isDesc: Int = 0
  private var dtype: Short = 0 // would be set for scalar

  def this(n: String) = {
    this()
    this.init_name = n
    this.col_name = n
    this.opType = OPTYPE.ID
    this.kind = ColKind.DFID
    val fs = FrovedisServer.getServerInstance()
    this.proxy = JNISupport.getIDDFfunc(fs.master_node, col_name)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
  }

  // right can be FrovedisColumn, Spark column including literals
  def this(left: FrovedisColumn, right: Any, 
           opt: Short, cond: Boolean = false) = { 
    this()
    this.opType = opt
    this.isBool = cond
    this.kind = ColKind.DFFUNC
    var right_str = right.toString
    this.init_name = get_name(left.col_name, right_str, opt)
    this.col_name = this.init_name
    if (left.isSCALAR) // TODO: support left as literal
      throw new java.lang.UnsupportedOperationException(
      this.col_name + ": 'left as literal' is currently not supported!\n")
    var leftp = left.proxy
    val is_immed = checkIsImmed(right)
    val fs = FrovedisServer.getServerInstance()
    var tmp: FrovedisColumn = null
    if (right.isInstanceOf[FrovedisColumn]) tmp = right.asInstanceOf[FrovedisColumn] // casting from Any
    if (is_immed) {
      var im_dt: Short = 0
      if (tmp == null) im_dt = DTYPE.detect(right) 
      else { // tmp is an instance of FrovedisColumn and it must be a SCALAR
        if (tmp.colName.equals("NULL") && this.isBool)  
          throw new IllegalArgumentException(this.col_name + 
          ": is not supported! Use isNull/isNotNull instead.\n")
        im_dt = tmp.get_dtype()
        right_str = tmp.init_name // col_name might be changed due to as()
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

  def get_immed(n: String, dtype: Short): FrovedisColumn = {
    val ret = new FrovedisColumn()
    ret.init_name = n
    ret.col_name = n
    ret.opType = OPTYPE.ID
    ret.kind = ColKind.DFSCALAR
    ret.dtype = dtype
    ret.isBool = (dtype == DTYPE.BOOL)
    val fs = FrovedisServer.getServerInstance()
    ret.proxy = JNISupport.getIMDFfunc(fs.master_node, ret.col_name, dtype)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return ret
  }

  def get_agg(agg: Short, ignoreNulls: Boolean = true): FrovedisColumn = {
    val ret = new FrovedisColumn()
    ret.opType = agg
    ret.kind = ColKind.DFAGG
    ret.init_name = get_name(this.col_name, "", agg)
    ret.col_name = ret.init_name
    val fs = FrovedisServer.getServerInstance()
    ret.proxy = JNISupport.getDFagg(fs.master_node, this.proxy,
                                    agg, ret.col_name, ignoreNulls)
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
      case OPTYPE.IF   => "CASE WHEN " + left + " THEN " + right + " END"
      case OPTYPE.ELIF => {
        val left2  = left.substring(0, left.length() - 4)
        val right2 = right.substring(4, right.length())
        left2 + right2
      }
      case OPTYPE.ELSE => {
        val left2 = left.substring(0, left.length() - 4)
        left2 + " ELSE " + right + " END"
      } 
      // --- mathematical ---
      case OPTYPE.ADD => "(" + left + " + " + right + ")"
      case OPTYPE.SUB => "(" + left + " - " + right + ")"
      case OPTYPE.MUL => "(" + left + " * " + right + ")"
      case OPTYPE.IDIV => "(" + left + " // " + right + ")"
      case OPTYPE.FDIV => "(" + left + " / " + right + ")"
      case OPTYPE.MOD => "(" + left + " % " + right + ")"
      case OPTYPE.POW => "POWER(" + left + ", " + right + ")"
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
      case OPTYPE.aDSUM => "sum(DISTINCT " + left + ")"
      case OPTYPE.aDCNT => "count(DISTINCT " + left + ")" // spark defaults: count(left)
      case OPTYPE.aFST  => "first(" + left + ")"
      case OPTYPE.aLST  => "last(" + left + ")"
      case _ => throw new IllegalArgumentException("Unsupported opt-type: " + opt)
    }
  }

  def >   (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.GT, true) 
  def >=  (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.GE, true) 
  def <   (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.LT, true) 
  def <=  (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.LE, true) 
  def === (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.EQ, true) 
  def !== (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.NE, true) 
  def =!= (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.NE, true) 

  def when (left: FrovedisColumn, arg: Any): FrovedisColumn = { // else-if when case
    if (this.opType != OPTYPE.IF && this.opType != OPTYPE.ELIF) {
      throw new IllegalArgumentException(
      s"when() can only be applied on a Column previously generated by when()")
    }
    val fs = FrovedisServer.getServerInstance()
    val ret = new FrovedisColumn(left, arg, OPTYPE.IF, true) // constructs a IF clause
    ret.init_name = get_name(this.col_name, ret.col_name, OPTYPE.ELIF) // updates name
    ret.col_name = ret.init_name
    ret.opType = OPTYPE.ELIF // updates opType
    ret.proxy = JNISupport.appendWhenCondition(fs.master_node, this.proxy, // updates proxy
                                               ret.proxy, ret.col_name)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return ret
  }

  def otherwise (arg: Any): FrovedisColumn = {
    if (this.opType != OPTYPE.IF && this.opType != OPTYPE.ELIF) {
      throw new IllegalArgumentException(
      s"otherwise() can only be applied on a Column previously generated by when()")
    }
    return new FrovedisColumn(this, arg, OPTYPE.ELSE, true)
  }

  def &&  (arg: FrovedisColumn) = new FrovedisColumn(this, arg, OPTYPE.AND, true) 
  def and (arg: FrovedisColumn) = new FrovedisColumn(this, arg, OPTYPE.AND, true) 
  def ||  (arg: FrovedisColumn) = new FrovedisColumn(this, arg, OPTYPE.OR, true) 
  def or  (arg: FrovedisColumn) = new FrovedisColumn(this, arg, OPTYPE.OR, true) 

  def like       (arg: String) = new FrovedisColumn(this, arg, OPTYPE.LIKE, true) 
  def not_like   (arg: String) = new FrovedisColumn(this, arg, OPTYPE.NLIKE, true) // added specially
  def startsWith (arg: String) = new FrovedisColumn(this, arg + "%", OPTYPE.LIKE, true)
  def endsWith   (arg: String) = new FrovedisColumn(this, "%" + arg, OPTYPE.LIKE, true)

  def dummy     = new FrovedisColumn("0") // for dummy right
  def unary_!   = new FrovedisColumn(this, dummy, OPTYPE.NOT, true) 
  def isNull    = new FrovedisColumn(this, dummy, OPTYPE.ISNULL, true) 
  def isNotNull = new FrovedisColumn(this, dummy, OPTYPE.ISNOTNULL, true) 

  // TODO: support other mathematical operators...
  def + (right: Any)  = new FrovedisColumn(this, right, OPTYPE.ADD)
  def - (right: Any)  = new FrovedisColumn(this, right, OPTYPE.SUB)
  def * (right: Any)  = new FrovedisColumn(this, right, OPTYPE.MUL)
  def / (right: Any)  = new FrovedisColumn(this, right, OPTYPE.FDIV)
  def % (right: Any)  = new FrovedisColumn(this, right, OPTYPE.MOD)
  def **(right: Any)  = new FrovedisColumn(this, right, OPTYPE.POW)

  def unary_- : FrovedisColumn = { // special case
    val ret = new FrovedisColumn()
    ret.opType = OPTYPE.MUL
    ret.kind = ColKind.DFFUNC
    val right_str = "-1"
    ret.init_name = "(- " + this.toString + ")"
    ret.col_name = ret.init_name
    val fs = FrovedisServer.getServerInstance()
    ret.proxy = JNISupport.getOptImmedDFfunc(fs.master_node, this.proxy, 
                                             right_str, DTYPE.INT,
                                             OPTYPE.MUL, col_name)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return ret
  }
  def as(new_name: String): this.type = {
    this.col_name = new_name // init_name remains same
    val fs = FrovedisServer.getServerInstance()
    if (isAGG)
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
  def value    = init_name
  def isBOOL   = isBool
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
    if (x == null) new FrovedisColumn().get_immed("NULL", DTYPE.STRING)
    else           new FrovedisColumn().get_immed(x.toString, DTYPE.detect(x))
  }
  def when(left: FrovedisColumn, right: Any) = 
    new FrovedisColumn(left, right, OPTYPE.IF, true)

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

  def first    (col: String) = new FrovedisColumn(col)
                                             .get_agg(OPTYPE.aFST, false)
  def last     (col: String) = new FrovedisColumn(col)
                                             .get_agg(OPTYPE.aLST, false)
  def first    (col: String, ignoreNulls: Boolean) = new FrovedisColumn(col)
                                             .get_agg(OPTYPE.aFST, ignoreNulls)
  def last     (col: String, ignoreNulls: Boolean) = new FrovedisColumn(col)
                                             .get_agg(OPTYPE.aLST, ignoreNulls)

  def sumDistinct   (col: String) = new FrovedisColumn(col)
                                             .get_agg(OPTYPE.aDSUM)
  def countDistinct (col: String) = new FrovedisColumn(col)
                                             .get_agg(OPTYPE.aDCNT)

  def max      (col: FrovedisColumn) = col.get_agg(OPTYPE.aMAX)
  def min      (col: FrovedisColumn) = col.get_agg(OPTYPE.aMIN)
  def sum      (col: FrovedisColumn) = col.get_agg(OPTYPE.aSUM)
  def avg      (col: FrovedisColumn) = col.get_agg(OPTYPE.aAVG)
  def variance (col: FrovedisColumn) = col.get_agg(OPTYPE.aVAR)
  def sem      (col: FrovedisColumn) = col.get_agg(OPTYPE.aSEM)
  def stddev   (col: FrovedisColumn) = col.get_agg(OPTYPE.aSTD)
  def mad      (col: FrovedisColumn) = col.get_agg(OPTYPE.aMAD)
  def count    (col: FrovedisColumn) = col.get_agg(OPTYPE.aCNT)

  def first    (col: FrovedisColumn) = col.get_agg(OPTYPE.aFST, false)
  def last     (col: FrovedisColumn) = col.get_agg(OPTYPE.aLST, false)
  def first    (col: FrovedisColumn, ignoreNulls: Boolean) = 
    col.get_agg(OPTYPE.aFST, ignoreNulls)
  def last     (col: FrovedisColumn, ignoreNulls: Boolean) = 
    col.get_agg(OPTYPE.aLST, ignoreNulls)

  def sumDistinct   (col: FrovedisColumn) = col.get_agg(OPTYPE.aDSUM)
  def countDistinct (col: FrovedisColumn) = col.get_agg(OPTYPE.aDCNT)

  // --- alias aggregate functions ---
  def mean(col: String)                = avg(col)
  def mean(col: FrovedisColumn)        = avg(col)
  def stddev_samp(col: String)         = stddev(col)
  def stddev_samp(col: FrovedisColumn) = stddev(col)
  def var_samp(col: String)            = variance(col)
  def var_samp(col: FrovedisColumn)    = variance(col)
  def pow(col: FrovedisColumn, right: Any) = col ** right
}

