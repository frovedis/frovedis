package com.nec.frovedis.mllib.fpm;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.Jmatrix.DummyFreqItemset
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModel}
import com.nec.frovedis.sql.FrovedisDataFrame
import com.nec.frovedis.sql.implicits_._
import scala.collection.mutable.ArrayBuffer
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.sql.SparkSession
import org.apache.spark.rdd.RDD
import org.apache.spark.sql.DataFrame
import org.apache.spark.mllib.fpm.FPGrowth.FreqItemset
import org.apache.spark.mllib.fpm.AssociationRules.Rule

// newly added parameters: treeDepth, compression_point, memOptLevel
class FPGrowth  private (var minSupport: Double,
                         var numPartitions: Int,
                         var treeDepth: Int,
                         var compressionPoint: Int,
                         var memOptLevel: Int) 
  extends java.io.Serializable {
 
  def this() = this(0.3, 1, Int.MaxValue, 4, 0)

  def setMinSupport(minSupport: Double): this.type = {
    require(minSupport >= 0.0 && minSupport <= 1.0,
      s"Minimal support level must be in range [0, 1] but got ${minSupport}")
    this.minSupport = minSupport
    this
  }
  def setTreeDepth(treeDepth: Int): this.type = {
    require(treeDepth >= 1,
      s"treeDepth value must be >= 1, but got ${treeDepth}")
    this.treeDepth = treeDepth
    this
  }
  def setCompressionPoint(compressionPoint: Int): this.type = {
    require(compressionPoint >= 2,
      s"compressionPoint value must be >= 2, but got ${compressionPoint}")
    this.compressionPoint = compressionPoint
    this
  }
  def setMemOptLevel(memOptLevel: Int): this.type = {
    require(memOptLevel == 0 || memOptLevel == 1,
      s"memOptLevel value must be 0 or 1, but got ${memOptLevel}")
    this.memOptLevel = memOptLevel
    this
  }
  def setNumPartitions(numPartitions: Int): this.type = {
    require(numPartitions > 0,
      s"Number of partitions must be positive but got ${numPartitions}")
    this.numPartitions = numPartitions
    this
  }
  private def get_table(data: Iterator[(Array[Int],Long)]): Iterator[(Int,Int)] = {
    val darr = data.toArray
    var ret = new ArrayBuffer[(Int,Int)]()
    for (i <- 0 to (darr.length-1)) {
      val id = darr(i)._2.intValue
      val items = darr(i)._1
      for (j <- 0 to (items.length-1)) {
        val p = (id, items(j))
        ret += p
      }
    }
    return ret.toIterator
  }
  private def convert_to_spark_dataframe(tr: RDD[Array[Int]]): DataFrame = {
    val spark = SparkSession.builder().getOrCreate()
    import spark.implicits._
    return tr.zipWithIndex().mapPartitions(get_table).toDF("trans_id","item")
  }
  def run(data:RDD[Array[Int]]): FPGrowthModel = {
    var sdata = convert_to_spark_dataframe(data)
    var fdata = new FrovedisDataFrame(sdata)
    return run(fdata, true)
  }
  def run(fdata:FrovedisDataFrame): FPGrowthModel = {
    return run(fdata, false) 
  }
  def run(fdata:FrovedisDataFrame, movable : Boolean ): FPGrowthModel = {
    val model_Id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    val fis_cnt = JNISupport.callFrovedisFPM(fs.master_node, fdata.get(),
                               minSupport, treeDepth, compressionPoint,
                               memOptLevel, model_Id, movable)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new FPGrowthModel(model_Id, fis_cnt)
  }
}

object FPGrowth {
  def train(data: RDD[Array[Int]],
            minSupport: Double,
            numPartitions: Int,
            treeDepth: Int,
            compressionPoint: Int,
            memOptLevel: Int): FPGrowthModel = {
    return new FPGrowth().setMinSupport(minSupport)
                         .setNumPartitions(numPartitions)
                         .setTreeDepth(treeDepth)
                         .setCompressionPoint(compressionPoint)
                         .setMemOptLevel(memOptLevel)
                         .run(data)
  }
  def train(data: RDD[Array[Int]],
            minSupport: Double,
            numPartitions: Int,
            treeDepth: Int,
            compressionPoint: Int): FPGrowthModel = {
    return train(data, minSupport, numPartitions, treeDepth, compressionPoint, 0)
  }
  def train(data: RDD[Array[Int]],
            minSupport: Double,
            numPartitions: Int,
            treeDepth: Int): FPGrowthModel = {
    return train(data, minSupport, numPartitions, treeDepth, 4, 0)
  }
  def train(data: RDD[Array[Int]],
            minSupport: Double,
            numPartitions: Int): FPGrowthModel = {
    return train(data, minSupport, numPartitions, Int.MaxValue, 4, 0)
  }
  def train(data: RDD[Array[Int]],
            minSupport: Double): FPGrowthModel =  {
    return train(data, minSupport, 1, Int.MaxValue, 4, 0)
  }
  def train(data: RDD[Array[Int]]): FPGrowthModel =  {
    return train(data, 0.3, 1, Int.MaxValue, 4, 0)
  }

  // --- frovedis input ---
  def train(data: FrovedisDataFrame,
            minSupport: Double,
            numPartitions: Int,
            treeDepth: Int,
            compressionPoint: Int,
            memOptLevel: Int): FPGrowthModel = {
    return new FPGrowth().setMinSupport(minSupport)
                         .setNumPartitions(numPartitions)
                         .setTreeDepth(treeDepth)
                         .setCompressionPoint(compressionPoint)
                         .setMemOptLevel(memOptLevel)
                         .run(data)
  }
  def train(data: FrovedisDataFrame,
            minSupport: Double,
            numPartitions: Int,
            treeDepth: Int,
            compressionPoint: Int): FPGrowthModel = {
    return train(data, minSupport, numPartitions, treeDepth, compressionPoint, 0)
  }
  def train(data: FrovedisDataFrame,
            minSupport: Double,
            numPartitions: Int,
            treeDepth: Int): FPGrowthModel = {
    return train(data, minSupport, numPartitions, treeDepth, 4, 0)
  }
  def train(data: FrovedisDataFrame,
            minSupport: Double,
            numPartitions: Int): FPGrowthModel = {
    return train(data, minSupport, numPartitions, Int.MaxValue, 4, 0)
  }
  def train(data: FrovedisDataFrame,
            minSupport: Double): FPGrowthModel =  {
    return train(data, minSupport, 1, Int.MaxValue, 4, 0)
  }
  def train(data: FrovedisDataFrame): FPGrowthModel =  {
    return train(data, 0.3, 1, Int.MaxValue, 4, 0)
  }
}

class FPGrowthModel (val model_Id: Int, val fis_count: Int) 
    extends GenericModel(model_Id, M_KIND.FPM) {
  def generateAssociationRules(minConfidence: Double): FPGrowthRule = {
    val model_Idr = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    val count = JNISupport.callFrovedisFPMR(fs.master_node, 
                minConfidence, model_Id, model_Idr)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new FPGrowthRule(model_Idr, count)
  }
  def generateAssociationRules(): FPGrowthRule = {
    return generateAssociationRules(0.8)
  }  
  def to_spark_model(sc:SparkContext): org.apache.spark.mllib.fpm.FPGrowthModel[Int] = {
    val fs = FrovedisServer.getServerInstance();
    var res = JNISupport.toSparkFPM(fs.master_node,mid)
    for (x <- res) x.debug_print()
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    val freq = res.map( x => new FreqItemset(x.item,x.count)) 
    return new org.apache.spark.mllib.fpm.FPGrowthModel (sc.parallelize(freq))
  }
}

object FPGrowthModel{                      
  def load(sc: SparkContext, path: String): FPGrowthModel = load(path)
  def load(path: String): FPGrowthModel = {
    val model_Id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    val fis_cnt = JNISupport.loadFPGrowthModel(fs.master_node, 
                  model_Id, M_KIND.FPM, path)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new FPGrowthModel(model_Id, fis_cnt)
  }
}

class FPGrowthRule (val model_Id: Int, val count: Int) 
                   extends GenericModel(model_Id, M_KIND.FPR) { 
}

object FPGrowthRule {
  def load(sc: SparkContext, path: String): FPGrowthRule = load(path)
  def load(path: String): FPGrowthRule = {
    val model_Id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    val cnt = JNISupport.loadFPGrowthModel(fs.master_node, 
                                model_Id, M_KIND.FPR, path)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new FPGrowthRule(model_Id, cnt)
  }
} 


