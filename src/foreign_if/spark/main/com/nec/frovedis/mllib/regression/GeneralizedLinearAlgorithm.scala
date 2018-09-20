package com.nec.frovedis.mllib.regression;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModelWithPredict}

object FROVEDIS {
  val NONE = 0xDEAD
}

class GeneralizedLinearModel(modelId: Int,
                             modelKind: Short,
                             nftr: Long,
                             ncls: Int,
                             icpt: Double,
                             thr: Double) 
  extends GenericModelWithPredict(modelId,modelKind) {
  protected val numFeatures: Long = nftr
  protected val numClasses: Int = ncls
  protected var intercept: Double = icpt
  protected var threshold: Double = thr

  override def toString() : String = {
    val str = s"intercept: " + intercept + ", numFeatures: " + numFeatures + 
              s", numClasses: " + numClasses + ", threshold: " + threshold
    return str
  }
  def getThreshold(): Double = threshold 
  def setThreshold(thr: Double) : Unit = {
    threshold = thr
    val fs = FrovedisServer.getServerInstance() 
    JNISupport.setFrovedisGLMThreshold(fs.master_node,mid,mkind,thr)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
  }
  def clearThreshold() : Unit = setThreshold(FROVEDIS.NONE); // predict-probability
}

