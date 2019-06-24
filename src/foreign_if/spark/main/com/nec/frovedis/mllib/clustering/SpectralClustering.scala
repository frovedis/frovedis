package com.nec.frovedis.mllib.clustering;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModel}
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.Vector
import org.apache.spark.SparkContext

class SpectralClustering(var nCluster: Int,
                         var nIteration: Int,
                         var nComponent: Int, 
                         var gamma: Double, 
                         var eps: Double, 
                         var normLaplacian: Boolean,
                         var precomputed: Boolean,
                         var mode: Int,
                         var dropFirst: Boolean) extends java.io.Serializable {
  def this() = this(2,100,2,1.0,0.01,true,false,1,false)
 
  def setNumIteration(nIteration: Int ): this.type = {
    require(nIteration > 0 ,
      s"nIteration must be greater than 0 but got  ${nIteration}")
    this.nIteration = nIteration
    this
  }

  def setNumCluster(nCluster: Int): this.type = {
    require(nCluster > 0 ,
      s"nCluster must be greater than 0 but got  ${nCluster}")
    this.nCluster = nCluster
    this
  }

  def setNumComponent(nComponent: Int): this.type = {
    require(nComponent > 0 ,
      s"nComponent must be greater than 0 but got  ${nComponent}")
    this.nComponent = nComponent
    this
  }

  def setEps(eps: Double): this.type = {
    require(eps > 0.0 ,
      s"eps must be greater than 0 but got  ${eps}")
    this.eps = eps
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

  def run(data:RDD[Vector]): SpectralClusteringModel = {
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
  def run(data: FrovedisSparseData): SpectralClusteringModel = {
    return run(data, false)
  }
  def run(data: FrovedisSparseData,
          movable: Boolean): SpectralClusteringModel = {
    val mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    val ret =  JNISupport.callFrovedisSCA(fs.master_node,
                                  data.get(),nCluster,nIteration,
                                  nComponent,eps,gamma,
                                  normLaplacian,mid,
                                  precomputed,mode,dropFirst,
                                  movable,false)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new SpectralClusteringModel(mid).setLabels(ret)
  }
  def run(data: FrovedisRowmajorMatrix): SpectralClusteringModel = {
    return run(data, false)
  }
  def run(data: FrovedisRowmajorMatrix,
          movable: Boolean): SpectralClusteringModel = {
    val mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    val ret =  JNISupport.callFrovedisSCA(fs.master_node,
                                  data.get(),nCluster,nIteration,
                                  nComponent,eps,gamma,
                                  normLaplacian,mid,
                                  precomputed,mode,dropFirst,
                                  movable,true)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new SpectralClusteringModel(mid).setLabels(ret)
  }
}

object SpectralClustering{
  //  --- RDD ---
  def train(data: RDD[Vector] , 
            nCluster: Int,  nIteration: Int ,
            nComponent: Int,  
            gamma: Double, eps: Double, 
            normLaplacian: Boolean,
            precomputed: Boolean,
            mode: Int, 
            dropFirst: Boolean): SpectralClusteringModel = {
     return new SpectralClustering()
               .setNumCluster(nCluster)
               .setNumIteration(nIteration)
               .setNumComponent(nComponent)
               .setEps(eps)
               .setGamma(gamma)
               .setNormLaplacian(normLaplacian)
               .setPrecomputed(precomputed)
               .setMode(mode)
               .setDropFirst(dropFirst)
               .run(data)
  }
  def train(data: RDD[Vector] , 
            nCluster: Int,  nIteration: Int ,
            nComponent: Int,  
            gamma: Double, eps: Double, 
            normLaplacian: Boolean,
            precomputed: Boolean,
            mode: Int): SpectralClusteringModel = {
    return train(data,nCluster,nIteration,nComponent,gamma,eps,normLaplacian,precomputed,mode,false)
  }
  def train(data: RDD[Vector] , 
            nCluster: Int,  nIteration: Int ,
            nComponent: Int,  
            gamma: Double, eps: Double, 
            normLaplacian: Boolean,
            precomputed: Boolean): SpectralClusteringModel = {
    return train(data,nCluster,nIteration,nComponent,gamma,eps,normLaplacian,precomputed,1,false)
  }
  def train(data: RDD[Vector],
            nCluster: Int,
            nIteration: Int,
            nComponent: Int, 
            gamma: Double, eps: Double, 
            normLaplacian: Boolean): SpectralClusteringModel = {
    return train(data,nCluster,nIteration,nComponent,gamma,eps,normLaplacian,false,1,false)
  }
  def train(data: RDD[Vector],
            nCluster: Int,
            nIteration: Int,
            nComponent: Int, 
            gamma: Double, eps: Double): SpectralClusteringModel = {
    return train(data,nCluster,nIteration,nComponent,gamma,eps,true,false,1,false)
  }
  def train(data: RDD[Vector],
            nCluster: Int,
            nIteration: Int,
            nComponent: Int, 
            gamma: Double): SpectralClusteringModel = {
     return train(data,nCluster,nIteration,nComponent,gamma,0.01,true,false,1,false)
  }
  def train(data: RDD[Vector],
            nCluster: Int,
            nIteration: Int,
            nComponent: Int): SpectralClusteringModel = {
     return train(data,nCluster,nIteration,nComponent,1.0,0.01,true,false,1,false)
  }
  def train(data: RDD[Vector],
            nCluster: Int,
            nIteration: Int): SpectralClusteringModel = {
     return train(data,nCluster,nIteration,2,1.0,0.01,true,false,1,false)
  }
  def train(data: RDD[Vector],
            nCluster: Int): SpectralClusteringModel = {
    return train(data,nCluster,100,2,1.0,0.01,true,false,1,false)
  }
  def train(data: RDD[Vector]): SpectralClusteringModel = {
    return train(data,2,100,2,1.0,0.01,true,false,1,false)
  }

  // --- SparseData Train ---
  def train(data: FrovedisSparseData, nCluster: Int, 
            nIteration: Int,
            nComponent: Int,  
            gamma: Double, eps: Double, 
            normLaplacian: Boolean,
            precomputed: Boolean,
            mode: Int,
            dropFirst: Boolean): SpectralClusteringModel = {
    return new SpectralClustering()
          .setNumCluster(nCluster)
          .setNumIteration(nIteration)
          .setNumComponent(nComponent)
          .setEps(eps)
          .setGamma(gamma)
          .setNormLaplacian(normLaplacian)
          .setPrecomputed(precomputed)
          .setMode(mode)
          .setDropFirst(dropFirst)
          .run(data,false)
  }

  def train(data: FrovedisSparseData, nCluster: Int,
            nIteration: Int,
            nComponent: Int,
            gamma: Double, eps: Double,
            normLaplacian: Boolean,
            precomputed: Boolean,
            mode: Int): SpectralClusteringModel = {
    return train(data,nCluster,nIteration,nComponent,gamma,eps,normLaplacian,precomputed,mode,false)
  }

  def train(data: FrovedisSparseData, nCluster: Int, 
            nIteration: Int,
            nComponent: Int,  
            gamma: Double, eps: Double,
            normLaplacian: Boolean,
            precomputed: Boolean): SpectralClusteringModel = {
    return train(data,nCluster,nIteration,nComponent,gamma,eps,normLaplacian,precomputed,1,false)
  }
  def train(data: FrovedisSparseData, nCluster: Int,
            nIteration: Int,
            nComponent: Int,
            gamma: Double, eps: Double,
            normLaplacian: Boolean): SpectralClusteringModel = {
    return train(data,nCluster,nIteration,nComponent,gamma,eps,normLaplacian,false,1,false)
  }
  def train(data: FrovedisSparseData,nCluster: Int, 
            nIteration: Int,
            nComponent: Int, 
            gamma: Double, eps: Double): SpectralClusteringModel = {
    return train(data,nCluster,nIteration,nComponent,gamma,eps,true,false,1,false)
  }
  def train(data: FrovedisSparseData, nCluster: Int,
            nIteration: Int, nComponent: Int,
            gamma: Double): SpectralClusteringModel = {
    return train(data,nCluster,nIteration,nComponent,gamma,0.01,true,false,1,false)
  }
  def train(data: FrovedisSparseData, nCluster: Int,
            nIteration: Int, nComponent:Int): SpectralClusteringModel = {
    return train(data,nCluster,nIteration,nComponent,1.0,0.01,true,false,1,false)
  }
  def train(data: FrovedisSparseData, nCluster: Int,
            nIteration: Int ): SpectralClusteringModel = {
    return train(data,nCluster,nIteration,2,1.0,0.01,true,false,1,false)
  }
  def train(data: FrovedisSparseData ,
            nCluster: Int): SpectralClusteringModel = {
    return train(data,nCluster,100,2,1.0,0.01,true,false,1,false)
  }
  def train(data: FrovedisSparseData): SpectralClusteringModel = {
    return train(data,2,100,2,1.0,0.01,true,false,1,false)
  }
  
  // --- RowmajorData Train --- 
  def train(data: FrovedisRowmajorMatrix, nCluster: Int,
            nIteration: Int,
            nComponent: Int, 
            gamma: Double, eps: Double, 
            normLaplacian: Boolean, 
            precomputed: Boolean,
            mode: Int,
            dropFirst: Boolean): SpectralClusteringModel = {
    return new SpectralClustering()
          .setNumCluster(nCluster)
          .setNumIteration(nIteration)
          .setNumComponent(nComponent)
          .setEps(eps)
          .setGamma(gamma)
          .setNormLaplacian(normLaplacian)
          .setPrecomputed(precomputed)
          .setMode(mode)
          .setDropFirst(dropFirst)
          .run(data,false)
  }
  def train(data: FrovedisRowmajorMatrix, nCluster: Int,
            nIteration: Int,
            nComponent: Int,
            gamma: Double, eps: Double,
            normLaplacian: Boolean,
            precomputed: Boolean,
            mode: Int): SpectralClusteringModel = {
    return train(data,nCluster,nIteration,nComponent,gamma,eps,normLaplacian,precomputed,mode,false)
  }
  def train(data: FrovedisRowmajorMatrix, nCluster: Int,
            nIteration: Int,
            nComponent: Int,
            gamma: Double, eps: Double,
            normLaplacian: Boolean,
            precomputed: Boolean): SpectralClusteringModel = {
    return train(data,nCluster,nIteration,nComponent,gamma,eps,normLaplacian,precomputed,1,false)
  }
  def train(data: FrovedisRowmajorMatrix, nCluster: Int, 
            nIteration: Int,
            nComponent: Int,  
            gamma: Double, eps: Double,
            normLaplacian: Boolean): SpectralClusteringModel = {
    return train(data,nCluster,nIteration,nComponent,gamma,eps,normLaplacian,false,1,false)
  }
  def train(data: FrovedisRowmajorMatrix, nCluster: Int, 
            nIteration: Int,
            nComponent: Int,  
            gamma: Double, eps: Double): SpectralClusteringModel = {
    return train(data,nCluster,nIteration,nComponent,gamma,eps,true,false,1,false)
  }
  def train(data: FrovedisRowmajorMatrix, nCluster: Int,
            nIteration: Int, 
            nComponent: Int,
            gamma: Double): SpectralClusteringModel = {
    return train(data,nCluster,nIteration,nComponent,gamma,0.01,true,false,1,false)
  }
  def train(data: FrovedisRowmajorMatrix, nCluster: Int,
            nIteration: Int, nComponent: Int): SpectralClusteringModel = {
    return train(data,nCluster,nIteration,nComponent,1.0,0.01,true,false,1,false)
  }
  def train(data: FrovedisRowmajorMatrix, nCluster: Int,
            nIteration: Int ): SpectralClusteringModel = {
    return train(data,nCluster,nIteration,2,1.0,0.01,true,false,1,false)
  }
  def train(data: FrovedisRowmajorMatrix ,
            nCluster: Int): SpectralClusteringModel = {
    return train(data,nCluster,100,2,1.0,0.01,true,false,1,false)
  }
  def train(data: FrovedisRowmajorMatrix): SpectralClusteringModel = {
    return train(data,2,100,2,1.0,0.01,true,false,1,false)
  }
}

class SpectralClusteringModel(model_Id: Int)
      extends GenericModel(model_Id, M_KIND.SCM) {
  var labels: Array[Int] = null

  def setLabels(labels: Array[Int]): this.type = {
    this.labels = labels
    this
  }
  def get_affinity_matrix(): FrovedisRowmajorMatrix = {
    val fs = FrovedisServer.getServerInstance()
    val dmat = JNISupport.getSCMAffinityMatrix(fs.master_node,model_Id)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisRowmajorMatrix(dmat)
  }
}

object SpectralClusteringModel {
  def load(sc: SparkContext, path: String): SpectralClusteringModel = load(path)
  def load(path: String): SpectralClusteringModel = {
    val model_id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.loadFrovedisSCM(fs.master_node,model_id,path)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new SpectralClusteringModel(model_id).setLabels(ret)
  }
}

