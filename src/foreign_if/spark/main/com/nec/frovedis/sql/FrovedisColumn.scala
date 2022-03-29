package com.nec.frovedis.sql;

import com.nec.frovedis.Jexrpc._
import com.nec.frovedis.matrix.DTYPE
import scala.collection.mutable
import org.apache.spark.sql.catalyst.parser.CatalystSqlParser
import org.apache.spark.sql.types._
import java.util.TimeZone
import java.time.LocalDate

object TMAPPER extends java.io.Serializable {
  val spk2frov_namedDT = Map("IntegerType" -> "int", "LongType" -> "long",
                             "FloatType" -> "float", "DoubleType" -> "double",
                             "StringType" -> "dic_string", 
                             "BooleanType" -> "boolean",
                             "DateType" -> "datetime",
                             "TimestampType" -> "datetime")

  val castedName = Map("int" -> "INT", "long" -> "BIGINT",
                       "float" -> "FLOAT", "double" -> "DOUBLE",
                       "dic_string" -> "STRING", "string" -> "STRING",
                       "boolean" -> "BOOLEAN", "datetime" -> "DATETIME")

  val id2field = Map(DTYPE.INT -> IntegerType,   DTYPE.LONG -> LongType,
                     DTYPE.FLOAT -> FloatType,   DTYPE.DOUBLE -> DoubleType,
                     DTYPE.STRING -> StringType, DTYPE.WORDS -> StringType,
                     DTYPE.BOOL -> BooleanType,  DTYPE.DATETIME -> DateType,
                     DTYPE.TIMESTAMP -> TimestampType)

  val id2string = Map(DTYPE.INT -> "IntegerType",   DTYPE.LONG -> "LongType",
                      DTYPE.FLOAT -> "FloatType",   DTYPE.DOUBLE -> "DoubleType",
                      DTYPE.STRING -> "StringType", DTYPE.WORDS -> "StringType",
                      DTYPE.BOOL -> "BooleanType", DTYPE.DATETIME -> "DateType",
                      DTYPE.TIMESTAMP -> "TimestampType")

  // string2id: only used in dataframe load and in cast
  // used WORDS instead of STRING, while loading string column (RDD[STRING]) as Dvector for better performance
  // simply enable ["StringType"  -> DTYPE.STRING] if you want to use the STRING type instead
  val string2id = Map("IntegerType" -> DTYPE.INT,    "LongType" -> DTYPE.LONG,
                   "FloatType"   -> DTYPE.FLOAT,  "DoubleType" -> DTYPE.DOUBLE,
                   //"StringType"  -> DTYPE.STRING, 
                   "StringType"  -> DTYPE.WORDS,
                   "BooleanType" -> DTYPE.BOOL,
                   "DateType" -> DTYPE.DATETIME,
                   "TimestampType" -> DTYPE.TIMESTAMP)
}

object ColKind extends java.io.Serializable {
  val DFID:     Short = 0
  val DFFUNC:   Short = 1
  val DFAGG:    Short = 2
  val DFSCALAR: Short = 3
}

object OPTYPE extends java.io.Serializable {
  val NONE:      Short = -1 // for unknown operator
  val ID:        Short = 0  // for column-name or scalar
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
  val CAST:       Short = 100
  val ISNAN:      Short = 101
  // --- string ---
  val SUBSTR:     Short = 102
  val SUBSTRINDX: Short = 103
  val UPPER:      Short = 104
  val LOWER:      Short = 105
  val LEN:        Short = 106
  val CHARLEN:    Short = 107
  val REV:        Short = 108
  val TRIM:       Short = 109
  val TRIMWS:     Short = 110
  val LTRIM:      Short = 111
  val LTRIMWS:    Short = 112
  val RTRIM:      Short = 113
  val RTRIMWS:    Short = 114
  val ASCII:      Short = 115
  val REPEAT:     Short = 116
  val CONCAT:     Short = 117
  val LPAD:       Short = 118
  val RPAD:       Short = 119
  val LOCATE:     Short = 120
  val INSTR:      Short = 121
  val REPLACE:    Short = 122
  val INITCAP:    Short = 123
  // --- date ---
  val GETYEAR:       Short = 201
  val GETMONTH:      Short = 202
  val GETDAYOFMONTH: Short = 203
  val GETHOUR:       Short = 204
  val GETMINUTE:     Short = 205
  val GETSECOND:     Short = 206
  val GETQUARTER:    Short = 207
  val GETDAYOFWEEK:  Short = 208
  val GETDAYOFYEAR:  Short = 209
  val GETWEEKOFYEAR: Short = 210
  val ADDDATE:       Short = 211
  val ADDMONTHS:     Short = 212
  val SUBDATE:       Short = 213
  val DATEDIFF:      Short = 214
  val MONTHSBETWEEN: Short = 215
  val NEXTDAY:       Short = 216
  val TRUNCMONTH:    Short = 217
  val TRUNCYEAR:     Short = 218
  val TRUNCWEEK:     Short = 219
  val TRUNCQUARTER:  Short = 220
  val TRUNCHOUR:     Short = 221
  val TRUNCMINUTE:   Short = 222
  val TRUNCSECOND:   Short = 223
  val DATEFORMAT:    Short = 224
  // --- date/time casting ---
  val TODATE:        Short = 300 
  val TODATEWS:      Short = 301
  val TOTIMESTAMP:   Short = 302
  val TOTIMESTAMPWS: Short = 303
}

class FrovedisColumn extends java.io.Serializable {
  // org_name: the name assigned during the object construction
  // col_name: would be updated if as() is performed, otherwise same as org_name
  private var org_name: String = null 
  private var col_name: String = null 
  private var kind: Short = ColKind.DFID
  private var opType: Short = OPTYPE.ID
  private var proxy: Long = 0
  private var isDesc: Int = 0

  // would be set for scalar and others for which type can be detected
  private var dtype: Short = DTYPE.NONE  
  private var value: Any = null // would be set only for scalar

  def this(n: String) = {
    this()
    this.col_name = n
    this.org_name = this.col_name
    this.opType = OPTYPE.ID
    this.kind = ColKind.DFID
    val fs = FrovedisServer.getServerInstance()
    this.proxy = JNISupport.getIDDFfunc(fs.master_node, col_name)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
  }

  // right can be FrovedisColumn, Spark column including literals
  def this(left: FrovedisColumn, right: Any, opt: Short, dtype: Short) = {
    this()
    this.opType = opt
    this.dtype = dtype
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
    this.org_name = this.col_name
    if (left.isAGG) throw new java.lang.UnsupportedOperationException(
                    this.col_name +  ": is not supported with aggregator!")
    if (right2.isAGG) throw new java.lang.UnsupportedOperationException(
                      this.col_name +  ": is not supported with aggregator!")

    var rev_op = false
    var im_op = false
    if (!left.isSCALAR && right2.isSCALAR) { 
      im_op = true
      rev_op = false
    }
    else if (left.isSCALAR && !right2.isSCALAR) { 
      im_op = true
      rev_op = true
    }
    else { // either both scalar (immed-column) or both column
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

  /*
   * --- for unary operations ---
   * supported opt:  OPTYPE.NOT, OPTYPE.ISNULL, OPTYPE.ISNOTNULL, 
   *                 OPTYPE.ISNAN, OPTYPE.UPPER, OPTYPE.LOWER, 
   *                 OPTYPE.LEN, OPTYPE.CHARLEN, 
   *                 OPTYPE.ASCII, OPTYPE.INITCAP,
   *                 OPTYPE.REV, OPTYPE.TRIM, OPTYPE.LTRIM, OPTYPE.RTRIM,
   *                 OPTYPE.GETYEAR, OPTYPE.GETMONTH, OPTYPE.GETDAYOFMONTH,
   *                 OPTYPE.GETHOUR, OPTYPE.GETMINUTE, OPTYPE.GETSECOND, 
   *                 OPTYPE.GETQUARTER, OPTYPE.GETDAYOFWEEK, 
   *                 OPTYPE.GETDAYOFYEAR, OPTYPE.GETWEEKOFYEAR, 
   *                 OPTYPE.TRUNCYEAR, OPTYPE.TRUNCMONTH, OPTYPE.TRUNCWEEK,
   *                 OPTYPE.TRUNCQUARTER, OPTYPE.TRUNCHOUR, OPTYPE.TRUNCMINUTE,
   *                 OPTYPE.TRUNCSECOND
   */
  def this(left: FrovedisColumn, opt: Short, dtype: Short) = {
    this()
    this.opType = opt
    this.dtype = dtype
    this.kind = ColKind.DFFUNC
    this.col_name = get_name(left.col_name, "", opt)
    this.org_name = this.col_name
    if (left.isAGG) throw new java.lang.UnsupportedOperationException(
                    this.col_name +  ": is not supported with aggregator!")

    val fs = FrovedisServer.getServerInstance()
    this.proxy = JNISupport.getOptDFfunc(fs.master_node, left.proxy, -1, 
                                         opt, col_name)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
  }

  /*
   * --- for operations with right-argument as String ---
   * supported opt:  OPTYPE.CAST, OPTYPE.LIKE, OPTYPE.NLIKE,
   *                 OPTYPE.TRIMWS, OPTYPE.LTRIMWS, OPTYPE.RTRIMWS,
   *                 OPTYPE.DATEFORMAT
   */
  def this(left: FrovedisColumn, right: String, opt: Short, dtype: Short) = {
    this()
    this.opType = opt
    this.dtype = dtype
    this.kind = ColKind.DFFUNC
    this.col_name = get_name(left.col_name, right, opt)
    this.org_name = this.col_name
    if (left.isAGG) throw new java.lang.UnsupportedOperationException(
                    this.col_name +  ": is not supported with aggregator!")

    val rev_op = false
    val fs = FrovedisServer.getServerInstance()
    this.proxy = JNISupport.getOptImmedDFfunc(fs.master_node, left.proxy, 
                                              right, DTYPE.STRING,
                                              opt, col_name, rev_op)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
  }

  /*
   * --- for operations with right-argument as int ---
   * supported opt:  OPTYPE.ADDDATE, OPTYPE.ADDMONTHS, OPTYPE.SUBDATE, 
   *                 OPTYPE.NEXTDAY, OPTYPE.REPEAT
   *                 
   */
  def this(left: FrovedisColumn, right: Int, opt: Short, dtype: Short) = {
    this()
    this.opType = opt
    this.dtype = dtype
    this.kind = ColKind.DFFUNC
    this.col_name = get_name(left.col_name, right.toString, opt)
    this.org_name = this.col_name
    if (left.isAGG) throw new java.lang.UnsupportedOperationException(
                    this.col_name +  ": is not supported with aggregator!")

    val fs = FrovedisServer.getServerInstance()
    this.proxy = JNISupport.getOptIntImmedDFfunc(fs.master_node, left.proxy,
                                                 right, opt, col_name)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
  }

  def concat_multi(cols: Array[FrovedisColumn],
                   sep: String, with_sep: Boolean): FrovedisColumn = {
    val ret = new FrovedisColumn()
    ret.opType = OPTYPE.CONCAT
    ret.dtype = DTYPE.STRING
    ret.kind = ColKind.DFFUNC
    if (with_sep) {
      var name = "concat_ws(" + sep
      for (i <- 0 until cols.size) name += ", " + cols(i).col_name
      name += ")"
      ret.col_name = name
    } else {
      var name = "concat("
      for (i <- 0 until cols.size) {
        if (i == 0) name += cols(i).col_name
        else        name += ", " + cols(i).col_name
      }
      name += ")"
      ret.col_name = name
    }
    ret.org_name = ret.col_name
    val fs = FrovedisServer.getServerInstance()
    val proxies = cols.map(x => x.proxy)
    ret.proxy = JNISupport.getOptConcat(fs.master_node, proxies, 
                                        proxies.size, sep, with_sep, 
                                        ret.col_name)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return ret
  }

  def to_datetime(opt: Short, format: String, dtype: Short): FrovedisColumn = {
    val ret = new FrovedisColumn()
    ret.opType = opt
    ret.dtype = dtype
    ret.kind = ColKind.DFFUNC
    var to_type = ""

    if (opt == OPTYPE.TODATE) {
      ret.col_name = get_name(this.col_name, "", opt)
      to_type = TMAPPER.spk2frov_namedDT("DateType")
    } else if (opt == OPTYPE.TOTIMESTAMP) {
      ret.col_name = get_name(this.col_name, "", opt)
      to_type = TMAPPER.spk2frov_namedDT("TimestampType")
    } else { // with format
      ret.col_name = get_name(this.col_name, format, opt)
      to_type = "datetime:" + DateTimeUtils.parse_format(format)
    }
    ret.org_name = ret.col_name
    if (this.isAGG) throw new java.lang.UnsupportedOperationException(
                    ret.col_name +  ": is not supported with aggregator!")

    val rev_op = false
    val fs = FrovedisServer.getServerInstance()
    ret.proxy = JNISupport.getOptImmedDFfunc(fs.master_node, this.proxy,
                                             to_type, DTYPE.STRING,
                                             OPTYPE.CAST, ret.col_name, rev_op)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return ret
  }

  def as_immed(n: Any): this.type = {
    if (n == null) {
      this.col_name = "NULL"
      this.dtype = DTYPE.INT // null would be constructed as INTMAX at server side
    } else {
      this.col_name = n.toString()
      this.dtype = DTYPE.detect(n)
    }
    this.org_name = this.col_name
    this.value = n
    this.opType = OPTYPE.ID
    this.kind = ColKind.DFSCALAR
    val fs = FrovedisServer.getServerInstance()
    this.proxy = JNISupport.getIMDFfunc(fs.master_node, col_name, dtype)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    this
  }

  def get_agg(agg: Short, dtype: Short, 
              ignoreNulls: Boolean = true): FrovedisColumn = {
    val ret = new FrovedisColumn()
    ret.opType = agg
    ret.dtype = dtype
    ret.kind = ColKind.DFAGG
    ret.col_name = get_name(this.col_name, "", agg)
    ret.org_name = ret.col_name
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
      case OPTYPE.ISNAN => "isnan(" + left + ")"
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
      // --- date ---
      case OPTYPE.GETYEAR => "year(" + left + ")"
      case OPTYPE.GETMONTH => "month(" + left + ")"
      case OPTYPE.GETDAYOFMONTH => "dayofmonth(" + left + ")"
      case OPTYPE.GETHOUR => "hour(" + left + ")"
      case OPTYPE.GETMINUTE => "minute(" + left + ")"
      case OPTYPE.GETSECOND => "second(" + left + ")"
      case OPTYPE.GETQUARTER => "quarter(" + left + ")"
      case OPTYPE.GETDAYOFWEEK => "dayofweek(" + left + ")"
      case OPTYPE.GETDAYOFYEAR => "dayofyear(" + left + ")"
      case OPTYPE.GETWEEKOFYEAR => "weekofyear(" + left + ")"
      case OPTYPE.ADDDATE => "date_add(" + left + ", " + right + ")" 
      case OPTYPE.ADDMONTHS => "add_months(" + left + ", " + right + ")" 
      case OPTYPE.SUBDATE => "date_sub(" + left + ", " + right + ")"
      case OPTYPE.DATEDIFF => "datediff(" + left + ", " + right + ")" 
      case OPTYPE.MONTHSBETWEEN => "months_between(" + left + ", " + right + ")" 
      case OPTYPE.NEXTDAY => "next_day(" + left + ", " + right + ")"
      case OPTYPE.TRUNCMONTH => "trunc(" + left + ", Month)"
      case OPTYPE.TRUNCYEAR => "trunc(" + left + ", Year)"
      case OPTYPE.TRUNCWEEK => "trunc(" + left + ", Week)"
      case OPTYPE.TRUNCQUARTER => "trunc(" + left + ", Quarter)"
      case OPTYPE.TRUNCHOUR => "date_trunc(Hour, " + left + ")"
      case OPTYPE.TRUNCMINUTE => "date_trunc(Minute, " + left + ")"
      case OPTYPE.TRUNCSECOND => "date_trunc(Second, " + left + ")"      
      case OPTYPE.DATEFORMAT => "date_format(" + left + ", " + right + ")"
      // --- cast ---
      case OPTYPE.CAST => "CAST(" + left + " AS " + TMAPPER.castedName(right) + ")"
      case OPTYPE.TODATE => "to_date(" + left + ")"
      case OPTYPE.TODATEWS => "to_date(" + left + ", " + right + ")"
      case OPTYPE.TOTIMESTAMP => "to_timestamp(" + left + ")"
      case OPTYPE.TOTIMESTAMPWS => "to_timestamp(" + left + ", " + right + ")"
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
      // --- string ---
      // few string functions like substring, substring_index etc. are defined separately...
      case OPTYPE.UPPER   => "upper(" + left + ")"
      case OPTYPE.LOWER   => "lower(" + left + ")"
      case OPTYPE.LEN     => "length(" + left + ")"
      case OPTYPE.CHARLEN => "char_length(" + left + ")"
      case OPTYPE.ASCII   => "ascii(" + left + ")"
      case OPTYPE.INITCAP => "initcap(" + left + ")"
      case OPTYPE.REPEAT  => "repeat(" + left + ", " + right + ")"
      case OPTYPE.REV     => "reverse(" + left + ")"
      case OPTYPE.TRIM    => "trim(" + left + ")"
      case OPTYPE.TRIMWS  => "TRIM(BOTH " + right + " FROM " + left + ")"
      case OPTYPE.LTRIM   => "ltrim(" + left + ")"
      case OPTYPE.LTRIMWS => "TRIM(LEADING " + right + " FROM " + left + ")"
      case OPTYPE.RTRIM   => "rtrim(" + left + ")"
      case OPTYPE.RTRIMWS => "TRIM(TRAILING " + right + " FROM " + left + ")"
      case _ => throw new IllegalArgumentException("Unsupported opt-type: " + opt)
    }
  }

  def >   (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.GT, DTYPE.BOOL) 
  def >=  (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.GE, DTYPE.BOOL) 
  def <   (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.LT, DTYPE.BOOL) 
  def <=  (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.LE, DTYPE.BOOL) 
  def === (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.EQ, DTYPE.BOOL) 
  def !== (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.NE, DTYPE.BOOL) 
  def =!= (arg: Any) = new FrovedisColumn(this, arg, OPTYPE.NE, DTYPE.BOOL)
 
  def cast(to: DataType): FrovedisColumn = {
    val to_str = to.toString()
    return new FrovedisColumn(this, TMAPPER.spk2frov_namedDT(to_str),
                              OPTYPE.CAST, TMAPPER.string2id(to_str))
  }
  def cast(to: String): FrovedisColumn = cast(CatalystSqlParser.parseDataType(to))

  def substr(startPos: Int, len: Int): FrovedisColumn = {
    val ret = new FrovedisColumn()
    ret.col_name = "substring(" + this.col_name + ", " + startPos + ", " + len + ")"
    ret.org_name = ret.col_name
    ret.opType = OPTYPE.SUBSTR
    ret.dtype = DTYPE.STRING
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
    ret.org_name = ret.col_name
    ret.opType = OPTYPE.SUBSTR
    ret.dtype = DTYPE.STRING
    ret.kind = ColKind.DFFUNC
    val fs = FrovedisServer.getServerInstance()
    ret.proxy = JNISupport.getColSubstrFunc(fs.master_node, this.proxy,
                                            startPos.proxy, len.proxy, ret.col_name)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return ret
  }

  def substr_index(delim: String, count: Int): FrovedisColumn = {
    val ret = new FrovedisColumn()
    ret.col_name = "substring_index(" + this.col_name + ", " + delim + ", " + count + ")"
    ret.org_name = ret.col_name
    ret.opType = OPTYPE.SUBSTRINDX
    ret.dtype = DTYPE.STRING
    ret.kind = ColKind.DFFUNC
    val fs = FrovedisServer.getServerInstance()
    ret.proxy = JNISupport.getImmedSubstrIndexFunc(fs.master_node, this.proxy,
                                                   delim, count, ret.col_name)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return ret
  }

  def pad_col(len: Int, value: String, is_left: Boolean): FrovedisColumn = {
    val ret = new FrovedisColumn()
    if (is_left) {
      ret.opType = OPTYPE.LPAD 
      ret.col_name = "lpad(" + this.col_name + ", " + len + ", " + value + ")"
    } else {
      ret.opType = OPTYPE.RPAD 
      ret.col_name = "rpad(" + this.col_name + ", " + len + ", " + value + ")"
    }
    ret.org_name = ret.col_name
    ret.dtype = DTYPE.STRING
    ret.kind = ColKind.DFFUNC
    val fs = FrovedisServer.getServerInstance()
    ret.proxy = JNISupport.getImmedPadFunc(fs.master_node, this.proxy,
                                           len, value, ret.col_name, is_left)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return ret
  }

  def locate(substr: String, pos: Int, opt: Short): FrovedisColumn = {
    val name = "locate(" + substr + ", " + this.col_name + ", " + pos + ")"
    if (pos < 1) return functions.lit(0).as(name) // quick return case

    val ret = new FrovedisColumn()
    ret.opType = opt
    if (opt == OPTYPE.LOCATE) {
      ret.col_name = name
    } else {
      ret.col_name = "instr(" + this.col_name + ", " + substr + ")"
    }
    ret.org_name = ret.col_name
    ret.dtype = DTYPE.INT
    ret.kind = ColKind.DFFUNC
    val fs = FrovedisServer.getServerInstance()
    ret.proxy = JNISupport.getImmedLocateFunc(fs.master_node, this.proxy,
                                              substr, pos, ret.col_name)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return ret
  }

  def replace(from: String, to: String): FrovedisColumn = {
    val ret = new FrovedisColumn()
    ret.col_name = "replace(" + this.col_name + ", " + from + ", " + to + ")"
    ret.org_name = ret.col_name
    ret.opType = OPTYPE.REPLACE
    ret.dtype = DTYPE.STRING
    ret.kind = ColKind.DFFUNC
    val fs = FrovedisServer.getServerInstance()
    ret.proxy = JNISupport.getImmedReplaceFunc(fs.master_node, this.proxy,
                                               from, to, ret.col_name)
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
    val ret = new FrovedisColumn(left, arg, OPTYPE.IF, DTYPE.BOOL) // constructs a IF clause
    ret.col_name = get_name(this.col_name, ret.col_name, OPTYPE.ELIF) // updates name
    ret.org_name = ret.col_name
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
    return new FrovedisColumn(this, arg, OPTYPE.ELSE, DTYPE.BOOL)
  }

  def &&  (arg: FrovedisColumn) = 
    new FrovedisColumn(this, arg, OPTYPE.AND, DTYPE.BOOL) 
  def and (arg: FrovedisColumn) = 
    new FrovedisColumn(this, arg, OPTYPE.AND, DTYPE.BOOL) 
  def ||  (arg: FrovedisColumn) = 
    new FrovedisColumn(this, arg, OPTYPE.OR, DTYPE.BOOL) 
  def or  (arg: FrovedisColumn) = 
    new FrovedisColumn(this, arg, OPTYPE.OR, DTYPE.BOOL) 

  def like (arg: String) = 
    new FrovedisColumn(this, arg, OPTYPE.LIKE, DTYPE.BOOL) 
  def not_like (arg: String) = 
    new FrovedisColumn(this, arg, OPTYPE.NLIKE, DTYPE.BOOL) // added specially
  def startsWith (arg: String) = 
    new FrovedisColumn(this, arg + "%", OPTYPE.LIKE, DTYPE.BOOL)
  def endsWith (arg: String) = 
    new FrovedisColumn(this, "%" + arg, OPTYPE.LIKE, DTYPE.BOOL)
  def contains (arg: String) = 
    new FrovedisColumn(this, "%" + arg + "%", OPTYPE.LIKE, DTYPE.BOOL)

  def unary_!    = new FrovedisColumn(this, OPTYPE.NOT, DTYPE.BOOL)
  def isNull     = new FrovedisColumn(this, OPTYPE.ISNULL, DTYPE.BOOL)
  def isNotNull  = new FrovedisColumn(this, OPTYPE.ISNOTNULL, DTYPE.BOOL)
  def isNaN      = new FrovedisColumn(this, OPTYPE.ISNAN, DTYPE.BOOL)
  def year       = new FrovedisColumn(this, OPTYPE.GETYEAR, DTYPE.INT)
  def month      = new FrovedisColumn(this, OPTYPE.GETMONTH, DTYPE.INT)
  def dayofmonth = new FrovedisColumn(this, OPTYPE.GETDAYOFMONTH, DTYPE.INT)
  def hour       = new FrovedisColumn(this, OPTYPE.GETHOUR, DTYPE.INT)
  def minute     = new FrovedisColumn(this, OPTYPE.GETMINUTE, DTYPE.INT)
  def second     = new FrovedisColumn(this, OPTYPE.GETSECOND, DTYPE.INT)
  def quarter    = new FrovedisColumn(this, OPTYPE.GETQUARTER, DTYPE.INT)
  def dayofweek  = new FrovedisColumn(this, OPTYPE.GETDAYOFWEEK, DTYPE.INT)
  def dayofyear  = new FrovedisColumn(this, OPTYPE.GETDAYOFYEAR, DTYPE.INT)
  def weekofyear = new FrovedisColumn(this, OPTYPE.GETWEEKOFYEAR, DTYPE.INT)

  def date_add  (right: Int) = 
    new FrovedisColumn(this, right, OPTYPE.ADDDATE, DTYPE.DATETIME)
  def date_add  (right: FrovedisColumn) = 
    new FrovedisColumn(this, right, OPTYPE.ADDDATE, DTYPE.DATETIME)
  def add_months  (right: Int) = 
    new FrovedisColumn(this, right, OPTYPE.ADDMONTHS, DTYPE.DATETIME) 
  def add_months  (right: FrovedisColumn) = 
    new FrovedisColumn(this, right, OPTYPE.ADDMONTHS, DTYPE.DATETIME)
  def date_sub  (right: Int) = 
    new FrovedisColumn(this, right, OPTYPE.SUBDATE, DTYPE.DATETIME)
  def date_sub  (right: FrovedisColumn) = 
    new FrovedisColumn(this, right, OPTYPE.SUBDATE, DTYPE.DATETIME)
  def months_between  (right: FrovedisColumn) = 
    new FrovedisColumn(this, right, OPTYPE.MONTHSBETWEEN, DTYPE.DOUBLE)
  def months_between  (right: String) = // added specially
    new FrovedisColumn(this, right, OPTYPE.MONTHSBETWEEN, DTYPE.DOUBLE)
  def datediff  (right: FrovedisColumn) = 
    new FrovedisColumn(this, right, OPTYPE.DATEDIFF, DTYPE.INT) 
  def datediff  (right: String) =  // added specially
    new FrovedisColumn(this, right, OPTYPE.DATEDIFF, DTYPE.INT) 
  
  def next_day(dayOfWeek: String): FrovedisColumn = {
    val day_to_int = Map("sun" -> 1, "mon" -> 2, "tue" -> 3,
                         "wed" -> 4, "thu" -> 5, "fri" -> 6, 
                         "sat" -> 7)
    val res_name  = "next_day(" + this.col_name + ", " + dayOfWeek + ")" 
    val sub = dayOfWeek.toLowerCase().substring(0, 3)
    if (!day_to_int.contains(sub)) {
      return functions.lit(null).as(res_name) // TODO: make null of DATETIME
    } else {
      return new FrovedisColumn(this, day_to_int(sub), 
             OPTYPE.NEXTDAY, DTYPE.DATETIME).as(res_name)
    }
  }

  def next_day(dayOfWeek: FrovedisColumn): FrovedisColumn= {
    require(!dayOfWeek.isAGG, "next_day: 'dayOfWeek' cannot be an aggregate function!")
    val res_name = "next_day(" + this.col_name + ", " + dayOfWeek.col_name + ")"
    val subcol = functions.lower(dayOfWeek).substr(0, 3)
    val num_col =  functions.when(subcol === "sun", 1)
                            .when(subcol === "mon", 2)
                            .when(subcol === "tue", 3)
                            .when(subcol === "wed", 4)
                            .when(subcol === "thu", 5)
                            .when(subcol === "fri", 6)
                            .when(subcol === "sat", 7)
    return new FrovedisColumn(this, num_col, 
      OPTYPE.NEXTDAY, DTYPE.DATETIME).as(res_name)
  }

  private def get_trunc_opt(format: String): Short = {
    return format.toLowerCase() match {
      case "year" | "yyyy" | "yy"  => OPTYPE.TRUNCYEAR
      case "month" | "mon" | "mm"  => OPTYPE.TRUNCMONTH
      case "week"    => OPTYPE.TRUNCWEEK
      case "quarter" => OPTYPE.TRUNCQUARTER
      case "hour"    => OPTYPE.TRUNCHOUR
      case "minute"  => OPTYPE.TRUNCMINUTE
      case "second"  => OPTYPE.TRUNCSECOND
      case _         => OPTYPE.NONE
    }
  }

  def trunc(format: String): FrovedisColumn = {
    val res_name = "trunc(" + this.col_name + ", " + format +  ")"
    val opt = get_trunc_opt(format)
    if (opt == OPTYPE.NONE) {
      return functions.lit(null).as(res_name) // TODO: make null of DATETIME
    } else {
      return new FrovedisColumn(this, opt, DTYPE.DATETIME).as(res_name)
    }
  }

  def date_trunc(format: String): FrovedisColumn = {
    val res_name = "date_trunc(" + format + ", "+  this.col_name + ")"
    val opt = get_trunc_opt(format)
    if (opt == OPTYPE.NONE) {
      return functions.lit(null).as(res_name) // TODO: make null of DATETIME
    } else {
      return new FrovedisColumn(this, opt, DTYPE.TIMESTAMP).as(res_name)
    }
  }

  def date_format(format: String) = 
    new FrovedisColumn(this, DateTimeUtils.parse_format(format), 
                       OPTYPE.DATEFORMAT, DTYPE.STRING)

  def last_day(): FrovedisColumn = {
    val res_name = "last_day(" + this.col_name + ")"
    return add_months(1).trunc("month").date_sub(1).as(res_name)
  }

  // TODO: support other mathematical operators like abs() etc.
  // dtype cannot be detected for these operators
  def + (right: Any)  = new FrovedisColumn(this, right, OPTYPE.ADD, DTYPE.NONE)
  def - (right: Any)  = new FrovedisColumn(this, right, OPTYPE.SUB, DTYPE.NONE)
  def * (right: Any)  = new FrovedisColumn(this, right, OPTYPE.MUL, DTYPE.NONE)
  def / (right: Any)  = new FrovedisColumn(this, right, OPTYPE.FDIV, DTYPE.NONE)
  def % (right: Any)  = new FrovedisColumn(this, right, OPTYPE.MOD, DTYPE.NONE)
  def **(right: Any)  = new FrovedisColumn(this, right, OPTYPE.POW, DTYPE.NONE)

  def unary_- : FrovedisColumn = { // special case
    val ret = new FrovedisColumn()
    ret.opType = OPTYPE.MUL
    ret.kind = ColKind.DFFUNC
    val right_str = "-1"
    ret.col_name = "(- " + this.toString + ")"
    ret.org_name = ret.col_name
    val is_rev = false
    val fs = FrovedisServer.getServerInstance()
    ret.proxy = JNISupport.getOptImmedDFfunc(fs.master_node, this.proxy, 
                                             right_str, DTYPE.INT,
                                             OPTYPE.MUL, ret.col_name, is_rev)
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
  def orgName  = org_name 
  def isID     = (kind == ColKind.DFID)
  def isFUNC   = (kind == ColKind.DFFUNC)
  def isAGG    = (kind == ColKind.DFAGG)
  def isSCALAR = (kind == ColKind.DFSCALAR)
  def get_dtype() = dtype
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
    new FrovedisColumn(left, right, OPTYPE.IF, DTYPE.BOOL)

  def max      (col: String) = 
    new FrovedisColumn(col).get_agg(OPTYPE.aMAX, DTYPE.NONE)
  def min      (col: String) = 
    new FrovedisColumn(col).get_agg(OPTYPE.aMIN, DTYPE.NONE)
  def sum      (col: String) = 
    new FrovedisColumn(col).get_agg(OPTYPE.aSUM, DTYPE.NONE)
  def avg      (col: String) = 
    new FrovedisColumn(col).get_agg(OPTYPE.aAVG, DTYPE.DOUBLE)
  def variance (col: String) = 
    new FrovedisColumn(col).get_agg(OPTYPE.aVAR, DTYPE.DOUBLE)
  def sem      (col: String) = 
    new FrovedisColumn(col).get_agg(OPTYPE.aSEM, DTYPE.DOUBLE)
  def stddev   (col: String) = 
    new FrovedisColumn(col).get_agg(OPTYPE.aSTD, DTYPE.DOUBLE)
  def mad      (col: String) = 
    new FrovedisColumn(col).get_agg(OPTYPE.aMAD, DTYPE.DOUBLE)
  def count    (col: String): FrovedisColumn = {
    // server side result is of ULONG type, but since spark doesn't support
    // ULONG, the same is casted back to LONG in 
    // FrovedisDateFrame constructor with dummy input
    if (col.equals("*")) {
      return new FrovedisColumn(col).get_agg(OPTYPE.aSIZE, DTYPE.LONG)
    } else {
      return new FrovedisColumn(col).get_agg(OPTYPE.aCNT, DTYPE.LONG)
    }
  }

  def first (col: String) = 
    new FrovedisColumn(col).get_agg(OPTYPE.aFST, DTYPE.NONE, false)
  def last  (col: String) = 
    new FrovedisColumn(col).get_agg(OPTYPE.aLST, DTYPE.NONE, false)
  def first (col: String, ignoreNulls: Boolean) = 
    new FrovedisColumn(col).get_agg(OPTYPE.aFST, DTYPE.NONE, ignoreNulls)
  def last  (col: String, ignoreNulls: Boolean) = 
    new FrovedisColumn(col).get_agg(OPTYPE.aLST, DTYPE.NONE, ignoreNulls)

  def sumDistinct   (col: String) = 
    new FrovedisColumn(col).get_agg(OPTYPE.aDSUM, DTYPE.NONE)
  def countDistinct (col: String) = 
    new FrovedisColumn(col).get_agg(OPTYPE.aDCNT, DTYPE.LONG)

  def max      (col: FrovedisColumn) = col.get_agg(OPTYPE.aMAX, col.get_dtype())
  def min      (col: FrovedisColumn) = col.get_agg(OPTYPE.aMIN, col.get_dtype())
  def sum      (col: FrovedisColumn) = col.get_agg(OPTYPE.aSUM, col.get_dtype())
  def avg      (col: FrovedisColumn) = col.get_agg(OPTYPE.aAVG, DTYPE.DOUBLE)
  def variance (col: FrovedisColumn) = col.get_agg(OPTYPE.aVAR, DTYPE.DOUBLE)
  def sem      (col: FrovedisColumn) = col.get_agg(OPTYPE.aSEM, DTYPE.DOUBLE)
  def stddev   (col: FrovedisColumn) = col.get_agg(OPTYPE.aSTD, DTYPE.DOUBLE)
  def mad      (col: FrovedisColumn) = col.get_agg(OPTYPE.aMAD, DTYPE.DOUBLE)
  def count    (col: FrovedisColumn) = col.get_agg(OPTYPE.aCNT, DTYPE.LONG)

  def first    (col: FrovedisColumn) = 
    col.get_agg(OPTYPE.aFST, col.get_dtype(), false)
  def last     (col: FrovedisColumn) = 
    col.get_agg(OPTYPE.aLST, col.get_dtype(), false)
  def first    (col: FrovedisColumn, ignoreNulls: Boolean) = 
    col.get_agg(OPTYPE.aFST, col.get_dtype(), ignoreNulls)
  def last     (col: FrovedisColumn, ignoreNulls: Boolean) = 
    col.get_agg(OPTYPE.aLST, col.get_dtype(), ignoreNulls)

  def sumDistinct   (col: FrovedisColumn) = 
    col.get_agg(OPTYPE.aDSUM, col.get_dtype())
  def countDistinct (col: FrovedisColumn) = 
    col.get_agg(OPTYPE.aDCNT, DTYPE.LONG)

  def isnull(e: FrovedisColumn) = e.isNull
  def isnan(e: FrovedisColumn)  = e.isNaN

  // --- date/time related functions ---
  def year(e: FrovedisColumn)         = e.year
  def month(e: FrovedisColumn)        = e.month
  def dayofmonth(e: FrovedisColumn)   = e.dayofmonth
  def hour(e: FrovedisColumn)         = e.hour
  def minute(e: FrovedisColumn)       = e.minute
  def second(e: FrovedisColumn)       = e.second
  def quarter(e: FrovedisColumn)      = e.quarter
  def dayofweek(e: FrovedisColumn)    = e.dayofweek
  def dayofyear(e: FrovedisColumn)    = e.dayofyear
  def weekofyear(e: FrovedisColumn)   = e.weekofyear

  def date_add(e: FrovedisColumn, right: Int) = e.date_add(right)
  def date_add(e: FrovedisColumn, right: FrovedisColumn) = e.date_add(right)
  def add_months(e: FrovedisColumn, right: Int) = e.add_months(right)
  def add_months(e: FrovedisColumn, right: FrovedisColumn) = e.add_months(right)
  def date_sub(e: FrovedisColumn, right: Int) = e.date_sub(right)
  def date_sub(e: FrovedisColumn, right: FrovedisColumn) = e.date_sub(right)
  def months_between(e: FrovedisColumn, 
                     right: String) = e.months_between(right)
  def months_between(e: FrovedisColumn, 
                     right: FrovedisColumn) = e.months_between(right)
  def datediff(e: FrovedisColumn, right: String) = e.datediff(right)
  def datediff(e: FrovedisColumn, right: FrovedisColumn) = e.datediff(right)
  def next_day(e: FrovedisColumn, dayOfWeek: String) = e.next_day(dayOfWeek)
  def next_day(e: FrovedisColumn, 
               dayOfWeek: FrovedisColumn) = e.next_day(dayOfWeek)
  def trunc(e: FrovedisColumn, format: String) = e.trunc(format)
  def date_trunc(format: String, e: FrovedisColumn) = e.date_trunc(format)
  def date_format(e: FrovedisColumn, format: String) = e.date_format(format)
  def last_day(e: FrovedisColumn) = e.last_day
  def current_timestamp(): FrovedisColumn = {
    val ts = (System.currentTimeMillis() + 
              TimeZone.getDefault().getRawOffset()) * 1000000L
    return lit(ts).cast("timestamp").as("current_timestamp()")
  }
  def current_date(): FrovedisColumn = {
    val numdays = LocalDate.now().toEpochDay()
    val ts = numdays * 24L * 3600 * 1000 * 1000 * 1000
    return lit(ts).cast("date").as("current_date()")
  }
  def to_date(e: FrovedisColumn) = 
    e.to_datetime(OPTYPE.TODATE, "", DTYPE.DATETIME)
  def to_date(e: FrovedisColumn, fmt: String) = 
    e.to_datetime(OPTYPE.TODATEWS, fmt, DTYPE.DATETIME)

  def to_timestamp(e: FrovedisColumn) = 
    e.to_datetime(OPTYPE.TOTIMESTAMP, "", DTYPE.TIMESTAMP)
  def to_timestamp(e: FrovedisColumn, fmt: String) = 
    e.to_datetime(OPTYPE.TOTIMESTAMPWS, fmt, DTYPE.TIMESTAMP)

  // --- alias aggregate functions ---
  def mean(col: String)                = avg(col)
  def mean(col: FrovedisColumn)        = avg(col)
  def stddev_samp(col: String)         = stddev(col)
  def stddev_samp(col: FrovedisColumn) = stddev(col)
  def var_samp(col: String)            = variance(col)
  def var_samp(col: FrovedisColumn)    = variance(col)
  def pow(col: FrovedisColumn, right: Any) = col ** right

  // --- string related functions ---
  def substring(str: FrovedisColumn,
                pos: Int, len: Int) = str.substr(pos, len) 

  def substring_index(str: FrovedisColumn, 
                      delim: String, 
                      count: Int) = str.substr_index(delim, count)

  def upper(e: FrovedisColumn) = 
    new FrovedisColumn(e, OPTYPE.UPPER, DTYPE.STRING)
  def lower(e: FrovedisColumn) = 
    new FrovedisColumn(e, OPTYPE.LOWER, DTYPE.STRING)
  def reverse(e: FrovedisColumn) = 
    new FrovedisColumn(e, OPTYPE.REV, DTYPE.STRING)
  def initcap(e: FrovedisColumn) = 
    new FrovedisColumn(e, OPTYPE.INITCAP, DTYPE.STRING)
  def char_length(e: FrovedisColumn) = 
    new FrovedisColumn(e, OPTYPE.CHARLEN, DTYPE.INT)
  def length(e: FrovedisColumn)  = char_length(e) // returns no. of characters
  // returns no. of bytes (3 bytes for 1 Japanese character)
  //def length(e: FrovedisColumn) = new FrovedisColumn(e, OPTYPE.LEN, DTYPE.INT)
  def ascii(e: FrovedisColumn) = new FrovedisColumn(e, OPTYPE.ASCII, DTYPE.INT)
  def repeat(e: FrovedisColumn, 
             n: Int) = new FrovedisColumn(e, n, OPTYPE.REPEAT, DTYPE.STRING) 

  def trim(e: FrovedisColumn) = 
    new FrovedisColumn(e, OPTYPE.TRIM, DTYPE.STRING)
  def trim(e: FrovedisColumn, trimString: String) = 
    new FrovedisColumn(e, trimString, OPTYPE.TRIMWS, DTYPE.STRING)

  def ltrim(e: FrovedisColumn) = 
    new FrovedisColumn(e, OPTYPE.LTRIM, DTYPE.STRING)
  def ltrim(e: FrovedisColumn, trimString: String) = 
    new FrovedisColumn(e, trimString, OPTYPE.LTRIMWS, DTYPE.STRING)

  def rtrim(e: FrovedisColumn) = 
    new FrovedisColumn(e, OPTYPE.RTRIM, DTYPE.STRING)
  def rtrim(e: FrovedisColumn, trimString: String) = 
    new FrovedisColumn(e, trimString, OPTYPE.RTRIMWS, DTYPE.STRING)

  def concat(e: FrovedisColumn*): FrovedisColumn = {
    return new FrovedisColumn().concat_multi(e.toArray, "", false)
  }

  def concat_ws(sep: String, e: FrovedisColumn*): FrovedisColumn = {
    return new FrovedisColumn().concat_multi(e.toArray, sep, true)
  }

  def lpad(e: FrovedisColumn, 
           len: Int, pad: String) = e.pad_col(len, pad, true)
  def lpad(e: FrovedisColumn, len: Int) = e.pad_col(len, " ", true)

  def rpad(e: FrovedisColumn, 
           len: Int, pad: String) = e.pad_col(len, pad, false)
  def rpad(e: FrovedisColumn, len: Int) = e.pad_col(len, " ", false)

  def instr (e: FrovedisColumn, 
             substr: String) = e.locate(substr, 1, OPTYPE.INSTR)
  def locate(substr: String, e: FrovedisColumn, 
             pos: Int) = e.locate(substr, pos, OPTYPE.LOCATE)
  def locate(substr: String, e: FrovedisColumn): FrovedisColumn =
             locate(substr, e, 1)

  def replace(e: FrovedisColumn, 
              from: String, to: String) = e.replace(from, to)
}

