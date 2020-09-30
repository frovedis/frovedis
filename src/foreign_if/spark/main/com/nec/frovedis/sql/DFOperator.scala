package com.nec.frovedis.sql;

import com.nec.frovedis.Jexrpc._

class DFOperator extends java.io.Serializable {
  protected var proxy: Long = -1

  def this(pro: Long) = {
    this()
    proxy = pro
  }
  def this(e: Expr, cols: Array[String], types: Array[Short]) = {
    this()
    proxy = e.get_proxy(cols,types)
  }

  def release () : Unit = {
    if (proxy != -1) {
      val fs = FrovedisServer.getServerInstance()
      JNISupport.releaseFrovedisDFOperator(fs.master_node,proxy)
      val info = JNISupport.checkServerException();
      if (info != "") throw new java.rmi.ServerException(info);
      proxy = -1
    }
  }
  def &&(opt: DFOperator): DFOperator = {
    val fs = FrovedisServer.getServerInstance()
    val r_proxy = JNISupport.getDFAndOperator(fs.master_node,proxy,opt.proxy)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    new DFOperator(r_proxy)
  }

  def and(opt: DFOperator): DFOperator = &&(opt)

  def ||(opt: DFOperator): DFOperator = {
    val fs = FrovedisServer.getServerInstance()
    val r_proxy = JNISupport.getDFOrOperator(fs.master_node,proxy,opt.proxy)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    new DFOperator(r_proxy)
  }

  def or(opt: DFOperator): DFOperator = ||(opt)

  def unary_! : DFOperator = {
    val fs = FrovedisServer.getServerInstance()
    val r_proxy = JNISupport.getDFNotOperator(fs.master_node,proxy)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    new DFOperator(r_proxy)
  }

  def get() = proxy
}

