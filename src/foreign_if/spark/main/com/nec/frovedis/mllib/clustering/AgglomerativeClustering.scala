package com.nec.frovedis.mllib.clustering;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModel}
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.Vector
import org.apache.spark.SparkContext

class AgglomerativeClustering(var linkage: String) 
  extends java.io.Serializable {
 
  def this() = this("average") 

  def setLinkage(linkage: String): this.type = {
    val link: List[String] = List("ward", "single", "average","multiple")
    val check = link.contains(linkage)
    require(check == true , 
    s"linkage Must be ward, single, multiple or average but found ${linkage}")
    this.linkage = linkage
    this
  }

  def run(data: RDD[Vector]): AgglomerativeModel = {
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
  def run(data: FrovedisSparseData): AgglomerativeModel = {
    return run(data, false)
  }
  def run(data: FrovedisSparseData, 
          movable: Boolean): AgglomerativeModel = {
    val mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    JNISupport.callFrovedisACA(fs.master_node,
                               data.get(),mid,linkage,
                               movable,false)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new AgglomerativeModel(mid)
  } 
  def run(data: FrovedisRowmajorMatrix): AgglomerativeModel = {
    return run(data, false)
  }
  def run(data: FrovedisRowmajorMatrix, 
          movable: Boolean): AgglomerativeModel = {
    val mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    JNISupport.callFrovedisACA(fs.master_node,
                               data.get(),mid,linkage,
                               movable,true)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new AgglomerativeModel(mid)
  }
}

object AgglomerativeClustering {
  def train(data: RDD[Vector],
            linkage : String): AgglomerativeModel = {
    return new AgglomerativeClustering()
                   .setLinkage(linkage)
                   .run(data)
  }
  def train(data: RDD[Vector]): AgglomerativeModel = {
    return train(data,"average")
  }
  def train(data: FrovedisSparseData, 
            linkage: String): AgglomerativeModel = {
    return new AgglomerativeClustering()
                  .setLinkage(linkage)
                  .run(data,false)
  }
  def train(data: FrovedisSparseData): AgglomerativeModel = {
    return train(data,"average")
  }
  def train(data: FrovedisRowmajorMatrix,
            linkage: String): AgglomerativeModel = {
    return new AgglomerativeClustering()
                  .setLinkage(linkage)
                  .run(data,false)
  }
  def train(data: FrovedisRowmajorMatrix): AgglomerativeModel = {
    return train(data,"average")
  }
}

class AgglomerativeModel(model_Id: Int) 
                         extends GenericModel(model_Id, M_KIND.ACM) {
  /* no member variable */  
  /* show/release/save are handled in GenericModel */
  def predict(ncluster: Int): Array[Int] = {
    val fs = FrovedisServer.getServerInstance()
    return JNISupport.FrovedisACMPredict(fs.master_node,mid,ncluster)
  }
}

object AgglomerativeModel {
  def load(sc: SparkContext, path: String): AgglomerativeModel = load(path)
  def load(path: String): AgglomerativeModel = {
    val model_id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    val labels = JNISupport.loadFrovedisACM(fs.master_node,model_id,path)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new AgglomerativeModel(model_id)
  }
}
