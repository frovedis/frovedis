package com.nec.frovedis.sql;

import scala.reflect.ClassTag
import org.apache.spark.SparkContext
import org.apache.spark.sql.SparkSession
import org.apache.spark.sql.DataFrame
import com.nec.frovedis.Jexrpc._
import OPTYPE._

object DummyDF {
  var dummy: DataFrame = null
  def get(): DataFrame = {
    if (dummy == null) {
      val spark = SparkSession.builder.getOrCreate()
      import spark.implicits._
      val sc = SparkContext.getOrCreate()
      dummy = sc.parallelize(Array(1,2,3,4)).toDF("one")
    }
    return dummy
  }
}

class FrovedisTypedColumn extends java.io.Serializable {
  protected var col_name: String = null
  protected var type_name: Short = 0
  private var isDesc: Int = 0

  def this(name: String, tid: Short) = {
    this()
    col_name = name
    type_name = tid
    isDesc = 0
  }
  // if not same class, then it would be assumed as an 'immed' operation
  private def isImmed(arg: Any): Boolean = {
    if (ClassTag(this.getClass) == ClassTag(arg.getClass)) return false
    val dummy_spark_df = DummyDF.get() 
    val spark_col = dummy_spark_df("one")
    if (ClassTag(arg.getClass) == ClassTag(spark_col.getClass)) return false
    else return true
  }
  def >(arg: Any): DFOperator = {
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.getDFOperator(fs.master_node,toString(),
                                         arg.toString(),dtype(),GT,isImmed(arg))
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new DFOperator(proxy)
  }
  def >=(arg: Any): DFOperator = {
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.getDFOperator(fs.master_node,toString(),
                                         arg.toString(),dtype(),GE,isImmed(arg))
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new DFOperator(proxy)
  }
  def <(arg: Any): DFOperator = {
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.getDFOperator(fs.master_node,toString(),
                                         arg.toString(),dtype(),LT,isImmed(arg))
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new DFOperator(proxy)
  }
  def <=(arg: Any): DFOperator = {
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.getDFOperator(fs.master_node,toString(),
                                         arg.toString(),dtype(),LE,isImmed(arg))
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new DFOperator(proxy)
  }
  def ===(arg: Any): DFOperator = {
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.getDFOperator(fs.master_node,toString(),
                                         arg.toString(),dtype(),EQ,isImmed(arg))
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new DFOperator(proxy)
  }
  def !==(arg: Any): DFOperator = {
    val fs = FrovedisServer.getServerInstance()
    val proxy = JNISupport.getDFOperator(fs.master_node,toString(),
                                         arg.toString(),dtype(),NE,isImmed(arg))
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new DFOperator(proxy)
  }
  // TODO: support Unary ! opearator
  
  def getIsDesc() = isDesc
  def setIsDesc(isDesc: Int): this.type = {
    this.isDesc = isDesc
    this
  }
  def asc(): this.type  = setIsDesc(0)
  def desc(): this.type = setIsDesc(1)

  override def toString() = col_name
  def cname() = col_name
  def dtype() = type_name
}

