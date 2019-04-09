package com.nec.frovedis.sql;

import com.nec.frovedis.Jexrpc._
import com.nec.frovedis.mllib.ModelID

class DFtoSparseInfo extends java.io.Serializable {
  protected var uid: Long = -1

  def this(info_id: Long) = {
    this()
    uid = info_id
  }
  def release() : Unit = {
    if (uid != -1) {
      val fs = FrovedisServer.getServerInstance()
      JNISupport.releaseSparseConversionInfo(fs.master_node,uid)
      val info = JNISupport.checkServerException();
      if (info != "") throw new java.rmi.ServerException(info);
      uid = -1
    }
  }
  def load(dirname: String) : Unit = {
    release() 
    val info_id = ModelID.get() // getting unique id for conversion info to be registered at server side
    val fs = FrovedisServer.getServerInstance()
    JNISupport.loadSparseConversionInfo(fs.master_node,info_id,dirname)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    uid = info_id
  }
  def save(dirname: String) : Unit = {
    if (uid != -1) {
      val fs = FrovedisServer.getServerInstance()
      JNISupport.saveSparseConversionInfo(fs.master_node,uid,dirname)
      val info = JNISupport.checkServerException();
      if (info != "") throw new java.rmi.ServerException(info);
    }
  }
  def get() = uid
}

