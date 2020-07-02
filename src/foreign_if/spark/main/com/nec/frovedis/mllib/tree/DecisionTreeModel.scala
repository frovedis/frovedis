package com.nec.frovedis.mllib.tree;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.io.FrovedisIO
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModelWithPredict2}
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.Vector
import org.apache.spark.SparkContext
import scala.collection.immutable.Map

class DecisionTreeModel(val model_Id: Int,
                        val logic: Map[Double, Double]) 
  extends GenericModelWithPredict2(model_Id, M_KIND.DTM){
  protected val enc_logic: Map[Double,Double] = logic

  override def predict(data: FrovedisRowmajorMatrix) : RDD[Double] = {
    val ret = super.predict(data)
    return ret.map(x => enc_logic(x))
  }
  override def predict(data: FrovedisSparseData) : RDD[Double] = {
    val ret = super.predict(data)
    return ret.map(x => enc_logic(x))
  }
  override def save(path: String) : Unit = {
    val context = SparkContext.getOrCreate()
    save(context, path)
  }
  override def save(sc: SparkContext, path: String) : Unit = {
    val success = FrovedisIO.createDir(path)
    require(success, "Another model named " + path + " already exists!")
    super.save(path + "/model")
    sc.parallelize(logic.toSeq, 2).saveAsObjectFile(path + "/label_map_spk")
  }  
  /*
  def to_spark_model(sc: SparkContext): 
    org.apache.spark.mllib.tree.model.DecisionTreeModel = {
    // to be implemented...
  }
  */
}
 
object DecisionTreeModel {
  def load(path: String): DecisionTreeModel = {
    val context = SparkContext.getOrCreate()
    load(context, path)
  }
  def load(sc: SparkContext, path: String): DecisionTreeModel = {
    val exist = FrovedisIO.checkExists(path)
    require(exist, "No model named " + path + " is found!")
    val modelId = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    JNISupport.loadFrovedisModel(fs.master_node, modelId, 
                                 M_KIND.DTM, path + "/model")
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val logic = sc.objectFile[(Double, Double)](path + "/label_map_spk")
                  .collectAsMap // returns generic scala.collection.Map
                  .toMap        // to make it immutable Map
    return new DecisionTreeModel(modelId, logic)
  }
}

