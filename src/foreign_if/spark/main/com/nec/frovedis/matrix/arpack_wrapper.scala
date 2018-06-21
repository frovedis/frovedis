package com.nec.frovedis.matrix;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.exrpc.FrovedisSparseData
import org.apache.spark.mllib.linalg.distributed.RowMatrix

object ARPACK {
  def computeSVD(data: RowMatrix, k: Int): GesvdResult = {
    val rddData = data.rows
    val fdata = new FrovedisSparseData(rddData)
    return computeSVDImpl(fdata,k,true)
  }
  def computeSVD(data: FrovedisSparseData, k: Int): GesvdResult = {
    return computeSVDImpl(data,k,false)
  }
  private def computeSVDImpl(data: FrovedisSparseData, 
                             k: Int,
                             inputMovable: Boolean): GesvdResult = {
    val fs = FrovedisServer.getServerInstance()
    val res = JNISupport.computeSVD(fs.master_node,data.get(),k,inputMovable)
    return new GesvdResult(res)
  }
} 
