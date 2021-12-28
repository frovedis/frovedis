package com.nec.frovedis.sql;

import com.nec.frovedis.Jexrpc._
import com.nec.frovedis.matrix.DTYPE
import scala.collection.mutable
import org.apache.spark.sql.catalyst.parser.CatalystSqlParser
import org.apache.spark.sql.types._

object TMAPPER extends java.io.Serializable {
  val spk2frov_namedDT = Map("IntegerType" -> "int", "LongType" -> "long",
                             "FloatType" -> "float", "DoubleType" -> "double",
                             "StringType" -> "dic_string", 
                             "BooleanType" -> "boolean")

  val castedName = Map("int" -> "INT", "long" -> "BIGINT",
                       "float" -> "FLOAT", "double" -> "DOUBLE",
                       "dic_string" -> "STRING", "string" -> "STRING",
                       "boolean" -> "BOOLEAN")

  val id2field = Map(DTYPE.INT -> IntegerType,   DTYPE.LONG -> LongType,
                     DTYPE.FLOAT -> FloatType,   DTYPE.DOUBLE -> DoubleType,
                     DTYPE.STRING -> StringType, DTYPE.WORDS -> StringType,
                     DTYPE.BOOL -> BooleanType)

  val id2string = Map(DTYPE.INT -> "IntegerType",   DTYPE.LONG -> "LongType",
                      DTYPE.FLOAT -> "FloatType",   DTYPE.DOUBLE -> "DoubleType",
                      DTYPE.STRING -> "StringType", DTYPE.WORDS -> "StringType",
                      DTYPE.BOOL -> "BooleanType")

  // string2id: only used in dataframe load and in cast
  // used WORDS instead of STRING, while loading string column (RDD[STRING]) as Dvector for better performance
  // simply enable ["StringType"  -> DTYPE.STRING] if you want to use the STRING type instead
  val string2id = Map("IntegerType" -> DTYPE.INT,    "LongType" -> DTYPE.LONG,
                   "FloatType"   -> DTYPE.FLOAT,  "DoubleType" -> DTYPE.DOUBLE,
                   //"StringType"  -> DTYPE.STRING, 
                   "StringType"  -> DTYPE.WORDS,
                   "BooleanType" -> DTYPE.BOOL)
}

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
  // --- other ---
  val CAST:      Short = 100
  val SUBSTR:    Short = 101
}

class FrovedisColumn extends java.io.Serializable {
  private var col_name: String = null // would be updated if as() is performed
  private var kind: Short = ColKind.DFID
  private var opType: Short = OPTYPE.ID
  private var proxy: Long = 0
  private var isBool: Boolean = false
  private var isDesc: Int = 0
  private var dtype: Short = 0  // would be set only for scalar
  private var value: Any = null // would be set for scalar

  def this(n: String) = {
    this()
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

    var right2: FrovedisColumn = null
    val right_is_immed = checkIsImmed(right)
    if (right_is_immed) {
      if (right.isInstanceOf[FrovedisColumn]) right2 = right.asInstanceOf[FrovedisColumn] 
      else right2 = new FrovedisColumn().as_immed(right)
    } 
    else { // column
      if (right.isInstanceOf[FrovedisColumn]) right2 = right.asInstanceOf[FrovedisColumn] 
      else right2 = new FrovedisColumn(right.toString) // spark column -> frovedis column
    }
    this.col_name = get_name(left.col_name, right2.col_name, opt)

    var rev_op = false
    var im_op = false
    if (!left.isSCALAR && right2.isSCALAR) { 
      im_op = true
      rev_op = false
    }
    else if (left.isSCALAR && !right2.isSCALAR) { // right can be dummy 
      val unary = Array(OPTYPE.NOT, OPTYPE.ISNULL, OPTYPE.ISNOTNULL)
      if (unary contains opt) { // right2 is dummy
        im_op = false
        rev_op = false
      }
      else {
        im_op = true
        rev_op = true
      }
    }
    else { // either both scalar (immed-column)  or both column
      im_op = false
      rev_op = false
    }

    val fs = FrovedisServer.getServerInstance()
    if (im_op) {
      var leftp: Long = 0
      var right_str: String = null
      var right_dtype: Short = 0
      if (rev_op) {
        leftp = right2.proxy
        right_str = left.get_value_as_string() // col_name might be changed due to as()
        right_dtype = left.get_dtype()
      }
      else {
        leftp = left.proxy
        right_str = right2.get_value_as_string() // col_name might be changed due to as()
        right_dtype = right2.get_dtype()
      }
      this.proxy = JNISupport.getOptImmedDFfunc(fs.master_node, leftp, 
                                                right_str, right_dtype,
                                                opt, col_name, rev_op)
    }
    else {
      var leftp = left.proxy
      var rightp = right2.proxy
      this.proxy = JNISupport.getOptDFfunc(fs.master_node, leftp, rightp,
                                           opt, col_name)

    }
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
  }

  def as_immed(n: Any): this.type = {
    if (n == null) {
      this.col_name = "NULL"
      this.dtype = DTYPE.INT // null would be constructed as INTMAX at server side
    } else {
      this.col_name = n.toString()
      this.dtype = DTYPE.detect(n)
    }
    this.value = n
    this.isBool = (dtype == DTYPE.BOOL)
    this.opType = OPTYPE.ID
    this.kind = ColKind.DFSCALAR
    val fs = FrovedisServer.getServerInstance()
    this.proxy = JNISupport.getIMDFfunc(fs.master_node, col_name, dtype)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    this
  }

  def get_agg(agg: Short, ignoreNulls: Boolean = true): FrovedisColumn = {
    val ret = new FrovedisColumn()
    ret.opType = agg
    ret.kind = ColKind.DFAGG
    ret.col_name = get_name(this.col_name, "", agg)
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
      case OPTYPE.CAST => "CAST(" + left + " AS " + TMAPPER.castedName(right) + ")"
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
 
  def cast(to: DataType): FrovedisColumn = {
    val dt = TMAPPER.spk2frov_namedDT(to.toString)
    new FrovedisColumn(this, dt, OPTYPE.CAST, dt.equals("boolean"))
  }
  def cast(to: String): FrovedisColumn = cast(CatalystSqlParser.parseDataType(to))

  def substr(startPos: Int, len: Int): FrovedisColumn = {
    val ret = new FrovedisColumn()
    ret.col_name = "substring(" + this.col_name + ", " + startPos + ", " + len + ")"
    ret.opType = OPTYPE.SUBSTR
    ret.kind = ColKind.DFFUNC
    val fs = FrovedisServer.getServerInstance()
    ret.proxy = JNISupport.getImmedSubstrFunc(fs.master_node, this.proxy, 
                                              startPos, len, ret.col_name)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return ret
  }

  def substr(startPos: FrovedisColumn, len: FrovedisColumn): FrovedisColumn = {
    require(!startPos.isAGG, "substr: 'startPos' cannot be an aggregate function!")
    require(!len.isAGG, "substr: 'len' cannot be an aggregate function!")

    val ret = new FrovedisColumn()
    ret.col_name = "substring(" + this.col_name + ", " + 
                   startPos.col_name + ", " + len.col_name + ")"
    ret.opType = OPTYPE.SUBSTR
    ret.kind = ColKind.DFFUNC
    val fs = FrovedisServer.getServerInstance()
    ret.proxy = JNISupport.getColSubstrFunc(fs.master_node, this.proxy,
                                            startPos.proxy, len.proxy, ret.col_name)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return ret
  }

  def when (left: FrovedisColumn, arg: Any): FrovedisColumn = { // else-if when case
    if (this.opType != OPTYPE.IF && this.opType != OPTYPE.ELIF) {
      throw new IllegalArgumentException(
      s"when() can only be applied on a Column previously generated by when()")
    }
    val fs = FrovedisServer.getServerInstance()
    val ret = new FrovedisColumn(left, arg, OPTYPE.IF, true) // constructs a IF clause
    ret.col_name = get_name(this.col_name, ret.col_name, OPTYPE.ELIF) // updates name
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

  // use "dummy" as right in order to mark the operation as not-immed
  def dummy     = new FrovedisColumn("0") // for dummy right
  def unary_!   = new FrovedisColumn(this, dummy, OPTYPE.NOT, true)
  def isNull    = new FrovedisColumn(this, dummy, OPTYPE.ISNULL, true)
  def isNotNull = new FrovedisColumn(this, dummy, OPTYPE.ISNOTNULL, true)

  // TODO: support other mathematical operators like abs() etc.
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
    ret.col_name = "(- " + this.toString + ")"
    val is_rev = false
    val fs = FrovedisServer.getServerInstance()
    ret.proxy = JNISupport.getOptImmedDFfunc(fs.master_node, this.proxy, 
                                             right_str, DTYPE.INT,
                                             OPTYPE.MUL, col_name, is_rev)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return ret
  }
  def as(new_name: String): this.type = {
    this.col_name = new_name
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
  def isBOOL   = isBool
  def isID     = (kind == ColKind.DFID)
  def isFUNC   = (kind == ColKind.DFFUNC)
  def isAGG    = (kind == ColKind.DFAGG)
  def isSCALAR = (kind == ColKind.DFSCALAR)
  def get_dtype(): Short = {
    if (isSCALAR) return dtype
    else throw new IllegalArgumentException("get_dtype: cannot detect dtype for non-literals!\n")
  }
  def get_value(): Any = {
    if (isSCALAR) return value
    else throw new IllegalArgumentException("get_value: can only be obtained for literals!\n")
  }
  def get_value_as_string(): String = {
    if (isSCALAR) return if (value == null) "NULL" else this.value.toString()
    else throw new IllegalArgumentException("get_value: can only be obtained for literals!\n")
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
  def lit(x: Any)              = new FrovedisColumn().as_immed(x)
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

