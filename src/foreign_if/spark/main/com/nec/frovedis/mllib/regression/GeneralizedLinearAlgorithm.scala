package com.nec.frovedis.mllib.regression;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModelWithPredict}
import com.nec.frovedis.matrix.ENUM

class GeneralizedLinearModel(modelId: Int,
                             modelKind: Short,
                             nftr: Long,
                             ncls: Int,
                             thr: Double)
  extends GenericModelWithPredict(modelId, modelKind) {
  protected val numFeatures: Long = nftr
  protected val numClasses: Int = ncls
  protected var threshold: Double = thr

  override def toString() : String = {
    val str = s"numFeatures: " + numFeatures + 
              s", numClasses: " + numClasses + ", threshold: " + threshold
    return str
  }
  def getThreshold(): Double = threshold 
  def setThreshold(thr: Double) : Unit = {
    threshold = thr
    JNISupport.setFrovedisGLMThreshold(fs.master_node,mid,mkind,thr)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
  }
  def clearThreshold() : Unit = setThreshold(ENUM.NONE); // predict-probability
}

