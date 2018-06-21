package com.nec.frovedis.mllib;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import org.apache.spark.SparkContext

object M_KIND {
  val GLM:    Short = 0
  val LRM:    Short = 1
  val SVM:    Short = 2
  val LNRM:   Short = 3
  val MFM:    Short = 4
  val KMEANS: Short = 5
}

object ModelID {
  private var mid: Int = 0
  // A threshold value, assuming it is safe to re-iterate
  // ModelID after reaching this value without affecting the
  // registered models at Frovedis server side.
  private val ID_MAX: Int = 1 << 15
  def get() : Int = {
    mid = (mid + 1) % ID_MAX
    return mid
  }
}  

class GenericModel(modelId: Int,
                   modelKind: Short) extends java.io.Serializable {
  protected val mid: Int = modelId
  protected val mkind: Short = modelKind

  def debug_print() : Unit = {
    val fs = FrovedisServer.getServerInstance() 
    JNISupport.showFrovedisModel(fs.master_node,mid,mkind)
  }
  def save(sc: SparkContext, path: String) : Unit = save(path) 
  def save(path: String) : Unit = {
    val fs = FrovedisServer.getServerInstance()
    JNISupport.saveFrovedisModel(fs.master_node,mid,mkind,path) 
  }
  def release() : Unit = {
    val fs = FrovedisServer.getServerInstance() 
    // It is serializable class. Thus an if check must be performed before the below JNI call.
    // To-Do: if(iam == rank0), only then call below JNI method 
    // [What is the Spark way of getting selfid()?]
    // Currently, the same is taken care using a deleted-model tracker at Frovedis side.
    JNISupport.releaseFrovedisModel(fs.master_node,mid,mkind)
    //println("[scala] model[" + mid + "] is finalized.")
  }
  //override def finalize() = release()
}
