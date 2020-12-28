package com.nec.frovedis.matrix;

import scala.reflect.ClassTag
import org.apache.spark.rdd.RDD
import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.exrpc.FrovedisSparseData
import org.apache.spark.mllib.linalg.Vector
import org.apache.spark.mllib.linalg.Matrix
import org.apache.spark.mllib.linalg.distributed.RowMatrix
import org.apache.spark.mllib.linalg.SingularValueDecomposition

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

  def computePrincipalComponents(data: RowMatrix, k: Int): FrovedisPCAModel = {
    return data.computePrincipalComponentsUsingFrovedis(k)
  }

  def computePrincipalComponentsAndExplainedVariance(data: RowMatrix, k: Int): 
    FrovedisPCAModel = {
    return data.computePrincipalComponentsAndExplainedVarianceUsingFrovedis(k)
  }
}

