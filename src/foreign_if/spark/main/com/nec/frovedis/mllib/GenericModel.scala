package com.nec.frovedis.mllib;

import com.nec.frovedis.Jexrpc.{Node,FrovedisServer,JNISupport}
import com.nec.frovedis.matrix.Utils._
import com.nec.frovedis.matrix.ScalaCRS
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import org.apache.spark.SparkContext
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.Vector

object M_KIND {
  val GLM:    Short = 0
  val LR:    Short = 1
  val SVM:    Short = 2
  val LNRM:   Short = 3
  val MFM:    Short = 4
  val KMEANS: Short = 5
  val DTM:    Short = 6
  val NBM:    Short = 7
  val FMM:    Short = 8
  val FPM:    Short = 9
  val FPR:    Short = 10
  val ACM:    Short = 11
  val SCM:    Short = 12
  val SEM:    Short = 13
  val SPARSE_CONV_INFO: Short = 14
  val W2V: Short = 15
  val DBSCAN: Short = 16
  val KNN: Short = 17
  val KNC: Short = 18
  val KNR: Short = 19
  val LDA: Short = 20
  val LDASP: Short = 21
  val RFM: Short = 22
  val GBT: Short = 23
  val SVR: Short = 24
  val KSVC: Short = 25
  val RR: Short = 26
  val LSR: Short = 27
  val GMM: Short = 28
  val STANDARDSCALER: Short = 29  
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
  protected val fs = FrovedisServer.getServerInstance() 

  def debug_print() : Unit = {
    JNISupport.showFrovedisModel(fs.master_node,mid,mkind)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
  }
  def save(sc: SparkContext, path: String) : Unit = save(path) 
  def save(path: String) : Unit = {
    JNISupport.saveFrovedisModel(fs.master_node,mid,mkind,path) 
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
  }
  def release() : Unit = {
    // It is serializable class. Thus an if check must be performed before the below JNI call.
    // To-Do: if(iam == rank0), only then call below JNI method 
    // [What is the Spark way of getting selfid()?]
    // Currently, the same is taken care using a deleted-model tracker at Frovedis side.
    JNISupport.releaseFrovedisModel(fs.master_node,mid,mkind)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    //println("[scala] model[" + mid + "] is finalized.")
  }
  //override def finalize() = release()
}

class GenericModelWithPredict(modelId: Int,
                              modelKind: Short)
  extends GenericModel(modelId, modelKind) {

  def predict(X: Vector): Double = {
    require(this.mid > 0, "predict() is called before training ")
    var dproxy: Long = -1
    var pred: Double = 0
    val isDense = X.getClass.toString() matches ".*DenseVector*."
    if (isDense) {
      val nrow: Long = 1
      val ncol: Long = X.size
      val rmjr_arr = X.toArray
      dproxy = JNISupport.loadFrovedisWorkerRmajorData(fs.master_node,nrow,ncol,rmjr_arr)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
    }
    else {
      val scalaCRS = new ScalaCRS(Array(X), X.size)
      dproxy = JNISupport.loadFrovedisWorkerData(fs.master_node,
                                                 scalaCRS.nrows,
                                                 scalaCRS.ncols,
                                                 scalaCRS.off,
                                                 scalaCRS.idx,
                                                 scalaCRS.data)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
    }
    pred = JNISupport.genericSinglePredict(fs.master_node, dproxy, this.mid, 
                                           this.mkind, isDense)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return pred
  }

  def predict(X: RDD[Vector]): RDD[Double] = {
    require(this.mid > 0, "predict() is called before training ")
    val isDense = X.first.getClass.toString() matches ".*DenseVector*."
    if (isDense) {
      val fdata = new FrovedisRowmajorMatrix(X)
      val res = predict(fdata)
      fdata.release()
      return res
    }
    else {
      val fdata = new FrovedisSparseData(X)
      val res = predict(fdata)
      fdata.release()
      return res
    }
  }

  // dense predict()
  def predict(X: FrovedisRowmajorMatrix): RDD[Double] = {
    require(this.mid > 0, "predict() is called before training ")
    val prob = false
    val isDense = true
    val res = JNISupport.genericPredict(fs.master_node, X.get(),
                                        this.mid, this.mkind,
                                        isDense, prob)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val context = SparkContext.getOrCreate()
    return context.parallelize(res)
  }

  // sparse predict()
  def predict(X: FrovedisSparseData): RDD[Double] = {
    require(this.mid > 0, "predict() is called before training ")
    val prob = false
    val isDense = false
    val res = JNISupport.genericPredict(fs.master_node, X.get(),
                                        this.mid, this.mkind,
                                        isDense, prob)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val context = SparkContext.getOrCreate()
    return context.parallelize(res)
  }

/*
  def predict_proba(X: RDD[Vector]): RDD[Double] = {
    require(this.mid > 0, "predict_proba() is called before training ")
    val isDense = X.first.getClass.toString() matches ".*DenseVector*."
    if (isDense) {
      val fdata = new FrovedisRowmajorMatrix(X)
      val res = predict_proba(fdata)
      fdata.release()
      return res
    }
    else {
      val fdata = new FrovedisSparseData(X)
      val res = predict_proba(fdata)
      fdata.release()
      return res
    }
  }

  // dense predict_proba()
  def predict_proba(X: FrovedisRowmajorMatrix): RDD[Double] = {
    require(this.mid > 0, "predict_proba() is called before training ")
    val prob = true
    val isDense = true
    val res = JNISupport.genericPredict(fs.master_node, X.get(),
                                        this.mid, this.mkind,
                                        isDense, prob)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val context = SparkContext.getOrCreate()
    return context.parallelize(res)
  }

  // sparse predict_proba()
  def predict_proba(X: FrovedisSparseData): RDD[Double] = {
    require(this.mid > 0, "predict_proba() is called before training ")
    val prob = true
    val isDense = false
    val res = JNISupport.genericPredict(fs.master_node, X.get(),
                                        this.mid, this.mkind,
                                        isDense, prob)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val context = SparkContext.getOrCreate()
    return context.parallelize(res)
  }
*/
}
