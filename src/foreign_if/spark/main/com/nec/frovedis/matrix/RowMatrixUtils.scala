package com.nec.frovedis.matrix;

import scala.reflect.ClassTag
import org.apache.spark.rdd.RDD
import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.exrpc.FrovedisSparseData
import org.apache.spark.mllib.linalg.Vector
import org.apache.spark.mllib.linalg.Matrix
import org.apache.spark.mllib.linalg.distributed.RowMatrix
import org.apache.spark.mllib.linalg.SingularValueDecomposition
import scala.math.max

object SVD {
  def compute(data: FrovedisSparseData, 
              k: Int,
              use_shrink: Boolean): GesvdResult = {
    val movable = false // user given frovedis data
    return computeImpl(data, k, use_shrink, movable)
  }
  def compute(data: FrovedisRowmajorMatrix, 
              k: Int,
              use_shrink: Boolean): GesvdResult = {
    val movable = false // user given frovedis data
    return computeImpl(data, k, use_shrink, movable)
  }
  def computeImpl(data: FrovedisSparseData,
                  k: Int,
                  use_shrink: Boolean,
                  inputMovable: Boolean): GesvdResult = {
    val isDense = false
    val fs = FrovedisServer.getServerInstance()
    val res = JNISupport.computeSVD(fs.master_node,data.get(),
                                    k,isDense,inputMovable,use_shrink)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new GesvdResult(res)
  }
  def computeImpl(data: FrovedisRowmajorMatrix,
                  k: Int,
                  use_shrink: Boolean,
                  inputMovable: Boolean): GesvdResult = {
    val isDense = true
    val fs = FrovedisServer.getServerInstance()
    val res = JNISupport.computeSVD(fs.master_node,data.get(),
                                    k,isDense,inputMovable,use_shrink)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new GesvdResult(res)
  }
}

object EigenValueDecomposition {
  //user gives frovedis data as input
  def eigsh(data: FrovedisSparseData,
            k: Int, sigma: Float, which: String,
            maxIterations: Int, tol: Double,
            mode: String): EvdResult = {
    var maxiter = maxIterations
    if(maxIterations == Int.MaxValue){
      maxiter = max(300, k * 3)
    }
    val movable = false // user given frovedis data
    return eigshImpl(data, k, sigma, which, maxiter, tol, mode, movable)
  }
  //user gives frovedis data as input
  def eigsh(data: FrovedisRowmajorMatrix,
            k: Int, sigma: Float, which: String,
            maxIterations: Int, tol: Double,
            mode: String): EvdResult = {
    val movable = false // user given frovedis data
    var maxiter = maxIterations
    if(maxIterations == Int.MaxValue){
      maxiter = max(300, k * 3)
    }
    return eigshImpl(data, k, sigma, which, maxiter, tol, mode, movable)
  }
  //user gives frovedis data as input
  def eigsh(data: RDD[Vector], k: Int, sigma: Float,
            which: String, maxIterations: Int, 
            tol: Double, mode: String): EvdResult = {
    var maxiter = maxIterations
    if(maxIterations == Int.MaxValue){
      maxiter = max(300, k * 3)
    }
    val isDense = data.first.getClass.toString() matches ".*DenseVector*."
    if(isDense){
      val fdata = new FrovedisRowmajorMatrix(data)
      return eigshImpl(fdata, k, sigma, which, maxiter, tol, mode, true)
    }
    else {
      val fdata = new FrovedisSparseData(data)
      return eigshImpl(fdata, k, sigma, which, maxiter, tol, mode, true)
    }
  }
  def eigshImpl(data: FrovedisSparseData,
                k: Int, sigma: Float, which: String,
                maxIterations: Int, tol: Double,
                mode: String, inputMovable: Boolean): EvdResult = {
    val isDense = false
    val n = data.numCols().toInt
    val m = data.numRows().toInt
    require(m == n, "Expected a squared matrix")
    require(k > 0 && k <= n, s"Requested k singular values but got k=$k and numCols=$n.")
    require(Set("LM", "SM", "LA", "SA", "BE").contains(which), 
                s"which must be one of LM, SM, LA, SA, or BE")
    require(mode == "normal", "Currenly normal mode is only supported!")
    require(sigma == Float.MaxValue, "Currently sigma is only supported for dense data")
    val fs = FrovedisServer.getServerInstance()
    val res = JNISupport.eigsh(fs.master_node, data.get(),
                               k, sigma, which, maxIterations, 
                               tol, isDense, inputMovable)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new EvdResult(res)
  }
  def eigshImpl(data: FrovedisRowmajorMatrix,
                k: Int, sigma: Float, which: String,                
                maxIterations: Int, tol: Double,
                mode: String, inputMovable: Boolean): EvdResult = {
    val isDense = true
    val n = data.numCols().toInt
    val m = data.numRows().toInt
    require(m == n, "Expected a squared matrix")
    require(k > 0 && k <= n, s"Requested k singular values but got k=$k and numCols=$n.")
    require(Set("LM", "SM", "LA", "SA", "BE").contains(which), 
                s"which must be one of LM, SM, LA, SA, or BE")
    require(mode == "normal", "Currenly normal mode is only supported!")
    val fs = FrovedisServer.getServerInstance()
    val res = JNISupport.eigsh(fs.master_node, data.get(),
                               k, sigma, which, maxIterations, 
                               tol, isDense, inputMovable)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new EvdResult(res)
  }
}

object PCA {
  def compute(data: FrovedisRowmajorMatrix,
              k: Int,
              need_variance: Boolean = false): FrovedisPCAModel = {
    return computeImpl(data, k, need_variance, false)
  }
  def computeImpl(data: FrovedisRowmajorMatrix,
                  k: Int,
                  need_variance: Boolean,
                  inputMovable: Boolean): FrovedisPCAModel = {
    val fs = FrovedisServer.getServerInstance()
    val res = JNISupport.computePCA(fs.master_node,data.get(),k,inputMovable)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new FrovedisPCAModel(res, need_variance)
  }
}

object RowMatrixUtils extends java.io.Serializable {
  implicit class RowMatrixPlus(data: RowMatrix) {
    def computeSVDUsingFrovedis(k: Int, 
                                use_shrink: Boolean = false): GesvdResult = {
      val rddData = data.rows
      // judging type of Vector
      val isDense = rddData.first.getClass.toString() matches ".*DenseVector*."
      if(isDense) {
        val fdata = new FrovedisRowmajorMatrix(rddData)
        return SVD.computeImpl(fdata,k,use_shrink,true)
      }
      else {
        val fdata = new FrovedisSparseData(rddData)
        return SVD.computeImpl(fdata,k,use_shrink,true)
      }
    }
    def eigshUsingFrovedis(k: Int, sigma: Float = Float.MaxValue,
                           which: String = "LM",
                           maxIterations: Int = Int.MaxValue,
                           tol: Double = 1e-10,
                           mode: String = "normal"): EvdResult = {
      val rddData = data.rows
      var maxiter = maxIterations
      if(maxIterations == Int.MaxValue){
        maxiter = max(300, k * 3)
      }
      // judging type of Vector
      val isDense = rddData.first.getClass.toString() matches ".*DenseVector*."
      if(isDense) {
        val fdata = new FrovedisRowmajorMatrix(rddData)
        return EigenValueDecomposition.eigshImpl(fdata, k, sigma, which, 
                                                 maxiter, tol, mode, true)
      }
      else {
        val fdata = new FrovedisSparseData(rddData)
        return EigenValueDecomposition.eigshImpl(fdata, k, sigma, which, 
                                                 maxiter, tol, mode, true)
      }
    }
    def computePrincipalComponentsUsingFrovedis(k: Int): FrovedisPCAModel = {
      val rddData = data.rows
      val fdata = new FrovedisRowmajorMatrix(rddData)
      return PCA.computeImpl(fdata,k,false,true)
    }
    def computePrincipalComponentsAndExplainedVarianceUsingFrovedis(k: Int): 
      FrovedisPCAModel = {
      val rddData = data.rows
      val fdata = new FrovedisRowmajorMatrix(rddData)
      return PCA.computeImpl(fdata,k,true,true)
    }
  }

  def computeSVD(data: RowMatrix, k: Int,
                 use_shrink: Boolean = false): GesvdResult = {
    return data.computeSVDUsingFrovedis(k, use_shrink)
  }

  def eigsh(data: RowMatrix, k: Int, sigma: Float = Float.MaxValue,
            which: String = "LM", maxIterations: Int = Int.MaxValue,
            tol: Double = 1e-10, mode: String = "normal"): EvdResult = {
    return data.eigshUsingFrovedis(k, sigma, which, maxIterations, tol, mode)
  }

  def computePrincipalComponents(data: RowMatrix, k: Int): FrovedisPCAModel = {
    return data.computePrincipalComponentsUsingFrovedis(k)
  }

  def computePrincipalComponentsAndExplainedVariance(data: RowMatrix, k: Int): 
    FrovedisPCAModel = {
    return data.computePrincipalComponentsAndExplainedVarianceUsingFrovedis(k)
  }
}

