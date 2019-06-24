package com.nec.frovedis.mllib.clustering;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModel}
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.Vector
import org.apache.spark.SparkContext

class SpectralEmbedding(var nComponent: Int, 
                        var gamma: Double, 
                        var normLaplacian: Boolean,
                        var precomputed: Boolean,
                        var mode: Int,
                        var dropFirst: Boolean) extends java.io.Serializable {
  def this() = this(2,1.0,true,false,1,true)

  def setNumComponent(nComponent: Int): this.type = {
    require(nComponent > 0 ,
      s"nComponent must be greater than 0 but got  ${nComponent}")
    this.nComponent = nComponent
    this
  }
  def setGamma(gamma: Double): this.type = {
    this.gamma = gamma
    this
  }
  def setPrecomputed(precomputed: Boolean): this.type = {
    this.precomputed = precomputed
    this
  }
  def setNormLaplacian(normLaplacian: Boolean): this.type = {
    this.normLaplacian = normLaplacian
    this
  }
  def setMode(mode: Int): this.type = {
    require(mode == 1 || mode == 3 ,
      s"mode must be either 1 or 3, but got  ${mode}")
    this.mode = mode
    this
  }
  def setDropFirst(dropFirst: Boolean): this.type = {
    this.dropFirst = dropFirst
    this
  }

  def run(data: RDD[Vector]): SpectralEmbeddingModel = {
    val isDense = data.first.getClass.toString() matches ".*DenseVector*."
    if (isDense) {
      val fdata = new FrovedisRowmajorMatrix(data)
      return run(fdata,true)
    }
    else {
      val fdata = new FrovedisSparseData(data)
      return run(fdata,true)
    }
  }
  def run(data: FrovedisSparseData): SpectralEmbeddingModel = {
    return run(data, false)
  }
  def run(data: FrovedisSparseData,
          movable: Boolean): SpectralEmbeddingModel = {
    val mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    JNISupport.callFrovedisSEA(fs.master_node,
                               data.get(),
                               nComponent,gamma,
                               normLaplacian,mid,
                               precomputed,mode,dropFirst,
                               movable,false)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new SpectralEmbeddingModel(mid)
  }
  def run(data: FrovedisRowmajorMatrix): SpectralEmbeddingModel = {
    return run(data, false)
  }
  def run(data: FrovedisRowmajorMatrix,
          movable: Boolean): SpectralEmbeddingModel = {
    val mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    JNISupport.callFrovedisSEA(fs.master_node,
                               data.get(),
                               nComponent,gamma,
                               normLaplacian,mid,
                               precomputed,mode,dropFirst,
                               movable,true)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new SpectralEmbeddingModel(mid)
  }
}

object SpectralEmbedding{
  //  --- RDD ---
  def train(data: RDD[Vector] , 
            nComponent: Int,  
            gamma: Double, 
            normLaplacian: Boolean,
            precomputed: Boolean,
            mode: Int,
            dropFirst: Boolean): SpectralEmbeddingModel = {
     return new SpectralEmbedding()
               .setNumComponent(nComponent)
               .setGamma(gamma)
               .setNormLaplacian(normLaplacian)
               .setPrecomputed(precomputed)
               .setMode(mode)
               .setDropFirst(dropFirst)
               .run(data)
  }
  def train(data: RDD[Vector],
            nComponent: Int, 
            gamma: Double, 
            normLaplacian: Boolean,
            precomputed: Boolean,
            mode: Int): SpectralEmbeddingModel = {
    return train(data,nComponent,gamma,normLaplacian,precomputed,mode,true)
  }
  def train(data: RDD[Vector],
            nComponent: Int, 
            gamma: Double, 
            normLaplacian: Boolean,
            precomputed: Boolean): SpectralEmbeddingModel = {
    return train(data,nComponent,gamma,normLaplacian,precomputed,1,true)
  }
  def train(data: RDD[Vector],
            nComponent: Int, 
            gamma: Double, 
            normLaplacian: Boolean): SpectralEmbeddingModel = {
    return train(data,nComponent,gamma,normLaplacian,false,1,true)
  }
  def train(data: RDD[Vector],
            nComponent: Int, 
            gamma: Double): SpectralEmbeddingModel = {
    return train(data,nComponent,gamma,true,false,1,true)
  }
  def train(data: RDD[Vector],
            nComponent: Int): SpectralEmbeddingModel = {
     return train(data,nComponent,1.0,true,false,1,true)
  }
  def train(data: RDD[Vector]): SpectralEmbeddingModel = {
    return train(data,2,1.0,true,false,1,true)
  }

  // --- SparseData Train ---
  def train(data: FrovedisSparseData, 
            nComponent: Int,  
            gamma: Double,  
            normLaplacian: Boolean,
            precomputed: Boolean,
            mode: Int,
            dropFirst: Boolean): SpectralEmbeddingModel = {
    return new SpectralEmbedding()
          .setNumComponent(nComponent)
          .setGamma(gamma)
          .setNormLaplacian(normLaplacian)
          .setPrecomputed(precomputed)
          .setMode(mode)
          .setDropFirst(dropFirst)
          .run(data,false)
  }
  def train(data: FrovedisSparseData, 
            nComponent: Int,  
            gamma: Double,
            normLaplacian: Boolean,
            precomputed: Boolean,
            mode: Int): SpectralEmbeddingModel = {
    return train(data,nComponent,gamma,normLaplacian,precomputed,mode,true)
  }
  def train(data: FrovedisSparseData, 
            nComponent: Int,  
            gamma: Double,
            normLaplacian: Boolean,
            precomputed: Boolean): SpectralEmbeddingModel = {
    return train(data,nComponent,gamma,normLaplacian,precomputed,1,true)
  }
  def train(data: FrovedisSparseData, 
            nComponent: Int,  
            gamma: Double,
            normLaplacian: Boolean): SpectralEmbeddingModel = {
    return train(data,nComponent,gamma,normLaplacian,false,1,true)
  }
  def train(data: FrovedisSparseData,
            nComponent: Int, 
            gamma: Double): SpectralEmbeddingModel = {
    return train(data,nComponent,gamma,true,false,1,true)
  }
  def train(data: FrovedisSparseData, 
            nComponent: Int): SpectralEmbeddingModel = {
    return train(data,nComponent,1.0,true,false,1,true)
  }
  def train(data: FrovedisSparseData): SpectralEmbeddingModel = {
    return train(data,2,1.0,true,false,1,true)
  }
  
  // --- RowmajorData Train --- 
  def train(data: FrovedisRowmajorMatrix, 
            nComponent: Int, 
            gamma: Double, 
            normLaplacian: Boolean, 
            precomputed: Boolean,
            mode: Int,
            dropFirst: Boolean): SpectralEmbeddingModel = {
    return new SpectralEmbedding()
          .setNumComponent(nComponent)
          .setGamma(gamma)
          .setNormLaplacian(normLaplacian)
          .setPrecomputed(precomputed)
          .setMode(mode)
          .setDropFirst(dropFirst)
          .run(data,false)
  }
  def train(data: FrovedisRowmajorMatrix,  
            nComponent: Int,  
            gamma: Double, 
            normLaplacian: Boolean,
            precomputed: Boolean,
            mode: Int): SpectralEmbeddingModel = {
    return train(data,nComponent,gamma,normLaplacian,precomputed,mode,true)
  }
  def train(data: FrovedisRowmajorMatrix,  
            nComponent: Int,  
            gamma: Double, 
            normLaplacian: Boolean,
            precomputed: Boolean): SpectralEmbeddingModel = {
    return train(data,nComponent,gamma,normLaplacian,precomputed,1,true)
  }
  def train(data: FrovedisRowmajorMatrix,  
            nComponent: Int,  
            gamma: Double, 
            normLaplacian: Boolean): SpectralEmbeddingModel = {
    return train(data,nComponent,gamma,normLaplacian,false,1,true)
  }
  def train(data: FrovedisRowmajorMatrix, 
            nComponent: Int,  
            gamma: Double): SpectralEmbeddingModel = {
    return train(data,nComponent,gamma,true,false,1,true)
  }
  def train(data: FrovedisRowmajorMatrix, 
            nComponent: Int): SpectralEmbeddingModel = {
    return train(data,nComponent,1.0,true,false,1,true)
  }
  def train(data: FrovedisRowmajorMatrix): SpectralEmbeddingModel = {
    return train(data,2,1.0,true,false,1,true)
  }
}

class SpectralEmbeddingModel(model_Id: Int)
      extends GenericModel(model_Id, M_KIND.SEM) {

  def get_affinity_matrix(): FrovedisRowmajorMatrix = {
    val fs = FrovedisServer.getServerInstance()
    val dmat = JNISupport.getSEMAffinityMatrix(fs.master_node,model_Id)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisRowmajorMatrix(dmat)
  }
  def get_embedding_matrix(): FrovedisRowmajorMatrix = {
    val fs = FrovedisServer.getServerInstance()
    val dmat = JNISupport.getSEMEmbeddingMatrix(fs.master_node,model_Id)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisRowmajorMatrix(dmat)
  }
}

object SpectralEmbeddingModel {
  def load(sc: SparkContext, path: String): SpectralEmbeddingModel = load(path)
  def load(path: String): SpectralEmbeddingModel = {
    val model_id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    JNISupport.loadFrovedisModel(fs.master_node,model_id,M_KIND.SEM,path)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new SpectralEmbeddingModel(model_id)
  }
}

