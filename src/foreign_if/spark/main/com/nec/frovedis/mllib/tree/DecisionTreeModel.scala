package com.nec.frovedis.mllib.tree;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModelWithPredict}
import org.apache.spark.SparkContext

class DecisionTreeModel(val model_Id: Int) 
  extends GenericModelWithPredict(model_Id, M_KIND.DTM){
  
  // to be implemented...
  /*
  def to_spark_model(sc: SparkContext): org.apache.spark.mllib.tree.model.DecisionTreeModel = {

  }
  */
}
 
object DecisionTreeModel {
  def load(sc: SparkContext, path: String): DecisionTreeModel = load(path)
  def load(path: String): DecisionTreeModel = {
    val modelId = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    JNISupport.loadFrovedisModel(fs.master_node,modelId,M_KIND.DTM,path)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new DecisionTreeModel(modelId)
  }
}

