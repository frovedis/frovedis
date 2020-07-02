package com.nec.frovedis.sql;

import com.nec.frovedis.Jexrpc._

object OPTYPE {
  val EQ: Short = 1
  val NE: Short = 2
  val GT: Short = 3
  val GE: Short = 4
  val LT: Short = 5
  val LE: Short = 6
  val AND: Short = 11
  val OR: Short = 12
}

class Expr extends java.io.Serializable {
  var st1: String = null
  var st2: String = null
  var op1: Expr = null
  var op2: Expr = null
  var opt: Short = 0
  var isTerminal: Boolean = false

  def this(o1: String, o2: String, operator: Short) = {
    this()
    st1 = o1
    st2 = o2
    opt = operator
    isTerminal = true
  }
  def this(o1: Expr, o2: Expr, operator: Short) = {
    this()
    op1 = o1
    op2 = o2
    opt = operator
    isTerminal = false 
  }
  def &&(e: Expr): Expr = {
    new Expr(this, e, OPTYPE.AND)
  }

  def and(e: Expr): Expr = &&(e)

  def ||(e: Expr): Expr = {
    new Expr(this, e, OPTYPE.OR)
  }

  def or(e: Expr): Expr = ||(e)

  private def get_opt(optid: Short): String = {
    return optid match {
      case OPTYPE.EQ => "=="
      case OPTYPE.NE => "!="
      case OPTYPE.GT => ">"
      case OPTYPE.GE => ">="
      case OPTYPE.LT => "<"
      case OPTYPE.LE => "<="
      case OPTYPE.AND => "&&"
      case OPTYPE.OR => "||"
      case _ => throw new IllegalArgumentException("Unsupported operator type: " + optid)
    }    
  }   
  def get_proxy(cols: Array[String], types: Array[Short]): Long = {
    val fs = FrovedisServer.getServerInstance()
    if(isTerminal) {
      var index = cols.indexOf(st1)
      if (index == -1)  throw new IllegalArgumentException("No column named: " + st1)
      val tid = types(index)    // getting type of the target column
      index = cols.indexOf(st2) // when "st2" is not a column name, its an 'immed' operation
      var isImmed = false
      if (index == -1)  isImmed = true
      val ret = JNISupport.getDFOperator(fs.master_node,st1,st2,tid,opt,isImmed)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      else return ret
   }
    else {
      val p1 = op1.get_proxy(cols,types)
      val p2 = op2.get_proxy(cols,types)
      // below calls release p1, p2 after getting combined operator
        val ret = opt match  {
        case OPTYPE.AND => JNISupport.getDFAndOperator(fs.master_node,p1,p2)
        case OPTYPE.OR  => JNISupport.getDFOrOperator(fs.master_node,p1,p2)
        case _ => throw new IllegalArgumentException("Unsupported logical operator type: " + opt)
      }
  
      val info1 = JNISupport.checkServerException()
      if (info1 != "") throw new java.rmi.ServerException(info1)
      else return ret
    } 
  }
  override def toString(): String = {
    var ret = ""
    if(isTerminal) return "(" + st1 + " " + get_opt(opt) + " " + st2 + ")"
    else {
      val r1 = op1.toString() 
      val r2 = op2.toString() 
      return "(" + r1 + " " + get_opt(opt) + " " + r2 + ")"
    }
  }
  def display(): Unit = println(toString())
}
