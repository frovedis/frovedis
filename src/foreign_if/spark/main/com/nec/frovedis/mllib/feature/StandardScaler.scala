package com.nec.frovedis.mllib.feature;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModelWithPredict}
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModel}
import org.apache.spark.rdd.RDD
import org.apache.spark.SparkContext
import org.apache.spark.sql.{Dataset, Row}
import org.apache.spark.sql.functions.col
import org.apache.spark.sql.SparkSession
import org.apache.spark.mllib.linalg.Vectors
import org.apache.spark.mllib.linalg.Vector
import org.apache.spark.ml.linalg.{Vector => MLVector}

class StandardScalerModel(val model_id: Int)
  extends GenericModel(model_id, M_KIND.STANDARDSCALER) {

  private var _mean: Array[Double] = null
  private var _stddev: Array[Double] = null

  var mean: Array[Double] = getMean()
  var stddev: Array[Double] = getStd()

  private def getMean(): Array[Double] = {
    if(_mean == null){ 
      val fs = FrovedisServer.getServerInstance()
      _mean = JNISupport.getScalerMean(fs.master_node, model_id)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
    }
    return _mean
  }

  private def getStd(): Array[Double] = {
    if(_stddev == null){
      val fs = FrovedisServer.getServerInstance()
      _stddev = JNISupport.getScalerStd(fs.master_node, model_id)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
    }
    return _stddev
  }


  def transform(fdata: FrovedisRowmajorMatrix): FrovedisRowmajorMatrix = {
    val fs = FrovedisServer.getServerInstance()
    val res = JNISupport.callScalerTransform(fs.master_node, 
                                             fdata.get(),
                                             model_id,
                                             true)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisRowmajorMatrix(res)
  }

  def transform(fdata: FrovedisSparseData): FrovedisSparseData = {
    val fs = FrovedisServer.getServerInstance()
    val res = JNISupport.callScalerTransform(fs.master_node,
                                             fdata.get(),
                                             model_id,
                                             false)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisSparseData(res)
  }

  def transform(data: RDD[Vector]): RDD[Vector] = {
    val isDense = data.first.getClass.toString() matches ".*DenseVector*."
    if(isDense) {
      var emb: FrovedisRowmajorMatrix = null
      val fdata = new FrovedisRowmajorMatrix(data)
      emb = transform(fdata)
      fdata.release()
      return emb.to_spark_RowMatrix(data.context).rows
    }
    else {
      val sdata = new FrovedisSparseData(data)
      val sparse_res = transform(sdata)
      sdata.release()
      return sparse_res.to_spark_sparse_matrix()
    } 
  }

  def transform(dataset: Dataset[_]): RDD[Vector]  = {
    var rdd_vec = dataset.select(col("features")).rdd.map {
      case Row(features: MLVector) => Vectors.fromML(features)
    }
    val res = transform(rdd_vec)
    return res
  }

  def inverse_transform(fdata: FrovedisRowmajorMatrix): FrovedisRowmajorMatrix = {
    val fs = FrovedisServer.getServerInstance()
    val res = JNISupport.callScalerInverseTransform(fs.master_node,
                                             fdata.get(),
                                             model_id,
                                             true)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisRowmajorMatrix(res)
  }

  def inverse_transform(fdata: FrovedisSparseData): FrovedisSparseData = {
    val fs = FrovedisServer.getServerInstance()
    val res = JNISupport.callScalerInverseTransform(fs.master_node,
                                             fdata.get(),
                                             model_id,
                                             false)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisSparseData(res)
  }

  def inverse_transform(data: RDD[Vector]): RDD[Vector] = {
    val isDense = data.first.getClass.toString() matches ".*DenseVector*."

    if(isDense) {
      var emb: FrovedisRowmajorMatrix = null
      val fdata = new FrovedisRowmajorMatrix(data)
      emb = inverse_transform(fdata)
      fdata.release()
      return emb.to_spark_RowMatrix(data.context).rows
    }
    else {
      val sdata = new FrovedisSparseData(data)
      val sparse_res = inverse_transform(sdata)
      sdata.release()
      return sparse_res.to_spark_sparse_matrix()
    }
  }

  def inverse_transform(dataset: Dataset[_]): RDD[Vector] = {
    var rdd_vec = dataset.select(col("features")).rdd.map {
      case Row(features: MLVector) => Vectors.fromML(features)
    }
    val res = inverse_transform(rdd_vec)
    return res
  }

}

class StandardScaler(var with_mean: Boolean,
                     var with_std: Boolean,
                     var sam_std: Boolean) {

  def this() = this(true, true, true)

  def setWithMean(with_mean: Boolean): this.type = {
    this.with_mean = with_mean
    this
  }

  def setWithStd(with_std: Boolean): this.type = {
    this.with_std = with_std
    this
  }

  def setSamStd(sam_std: Boolean): this.type = {
    this.sam_std = sam_std
    this
  }

  def fit(data: FrovedisRowmajorMatrix): StandardScalerModel = {
    val model_id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    val res = JNISupport.callFrovedisScaler(fs.master_node, 
                                    data.get(),with_mean,
                                     with_std,sam_std, 
                                            model_id,true)    
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new StandardScalerModel(model_id)
  }

  def fit(data: FrovedisSparseData): StandardScalerModel = {
    val model_id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    val res = JNISupport.callFrovedisScaler(fs.master_node,
                                            data.get(),with_mean,
                                            with_std,sam_std,
                                            model_id,false)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new StandardScalerModel(model_id)
  }

  def fit(data: RDD[Vector]): StandardScalerModel = {
    val model_id = ModelID.get()
    val isDense = data.first.getClass.toString() matches ".*DenseVector*."
    if(isDense) {
      val fdata = new FrovedisRowmajorMatrix(data) //convert rdd[vector] to frovedis rowmajor matrix
      return fit(fdata)
    }
    else {
      val sdata = new FrovedisSparseData(data) //convert rdd[vector] to frovedis Sparse Data
      return fit(sdata)
    }
    return new StandardScalerModel(model_id)
  }


  def fit(dataset: Dataset[_]): StandardScalerModel = { 
    //Convert Dataset to RDD[Vector]
    var rdd_vec = dataset.select(col("features")).rdd.map {
      case Row(features: MLVector) => Vectors.fromML(features)
    }
    return fit(rdd_vec) 
  }  

}
