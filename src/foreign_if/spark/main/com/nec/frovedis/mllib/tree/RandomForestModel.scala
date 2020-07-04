package com.nec.frovedis.mllib.tree;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.io.FrovedisIO
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModelWithPredict}
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.Vector
import org.apache.spark.SparkContext
import org.apache.log4j.LogManager
import scala.collection.immutable.Map

class RandomForestModel(val model_Id: Int,
                        val logic: Map[Double, Double]) 
  extends GenericModelWithPredict(model_Id, M_KIND.RFM){
  protected val enc_logic: Map[Double,Double] = logic

  override def predict(data: Vector) : Double = {
    val ret = super.predict(data)
    return if (enc_logic != null) enc_logic(ret) else ret
  }
  override def predict(data: FrovedisRowmajorMatrix) : RDD[Double] = {
    val ret = super.predict(data)
    return if (enc_logic != null) ret.map(x => enc_logic(x)) else ret
  }
  override def predict(data: FrovedisSparseData) : RDD[Double] = {
    val ret = super.predict(data)
    return if (enc_logic != null) ret.map(x => enc_logic(x)) else ret
  }
  override def save(path: String) : Unit = {
    val context = SparkContext.getOrCreate()
    save(context, path)
  }
  override def save(sc: SparkContext, path: String) : Unit = {
    val success = FrovedisIO.createDir(path)
    require(success, "Another model named " + path + " already exists!")
    super.save(path + "/model")
    if (enc_logic != null)
      sc.parallelize(logic.toSeq, 2).saveAsObjectFile(path + "/label_map_spk")
  }
  def numTrees(): Int = {
    val fs = FrovedisServer.getServerInstance()
    val res: Int = JNISupport.rfNumTrees(fs.master_node, this.mid)
    return res
  }
  def totalNumNodes(): Int = {
    val fs = FrovedisServer.getServerInstance()
    val res: Int = JNISupport.rfTotalNumNodes(fs.master_node, this.mid)
    return res
  }
  override def toString(): String = {
    val fs = FrovedisServer.getServerInstance()
    val res: String = JNISupport.rfToString(fs.master_node, this.mid)
    return res
  }
}
 
object RandomForestModel {
  def load(path: String): RandomForestModel = {
    val context = SparkContext.getOrCreate()
    load(context, path)
  }
  def load(sc: SparkContext, path: String): RandomForestModel = {
    val exist = FrovedisIO.checkExists(path)
    require(exist, "No model named " + path + " is found!")
    val modelId = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    JNISupport.loadFrovedisModel(fs.master_node, modelId,
                                 M_KIND.RFM, path + "/model")
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    var logic:  Map[Double, Double] = null
    val logicExist = FrovedisIO.checkExists(path + "/label_map_spk")
    if (logicExist){
      logic = sc.objectFile[(Double, Double)](path + "/label_map_spk")
                  .collectAsMap // returns generic scala.collection.Map
                  .toMap        // to make it immutable Map
    }
    else{
      val log = LogManager.getRootLogger
      log.info(s"Label encoding logic is not found: $path is loaded as a Regressor model")
    }
    return new RandomForestModel(modelId, logic)
  }
}
