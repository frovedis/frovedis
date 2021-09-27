package com.nec.frovedis.mllib.fpm;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.Jmatrix.DummyFreqItemset
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModel}
import com.nec.frovedis.sql.FrovedisDataFrame
import com.nec.frovedis.sql.implicits_._
import scala.collection.mutable.ArrayBuffer
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.sql.SparkSession
import org.apache.spark.sql.functions.collect_set
import org.apache.spark.rdd.RDD
import org.apache.spark.sql.DataFrame
import org.apache.spark.sql.expressions.Window
import org.apache.spark.sql.functions.{row_number, array, lit, when, explode}
import org.apache.spark.mllib.fpm.FPGrowth.FreqItemset
import org.apache.spark.mllib.fpm.AssociationRules.Rule
import scala.reflect.runtime.universe._
import scala.reflect.ClassTag

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
  def run(data: DataFrame): FPGrowthModel = {
    val spark = SparkSession.builder().getOrCreate()
    import spark.implicits._
    val window = Window.orderBy(lit('A'))
    val sdata = data.withColumn("trans_id", row_number.over(window))
    val s_df = sdata.select($"trans_id", explode($"item"))
                    .withColumnRenamed("col", "item")
    var fdata = new FrovedisDataFrame(s_df)
    return run(fdata)
  }
  def run(data:RDD[Array[String]], dummy: Null = null): FPGrowthModel = {
    var sdata = FPUtil.convert_to_spark_dataframe(data)
    var fdata = new FrovedisDataFrame(sdata)
    return run(fdata, true)
  }
  def run(data:RDD[Array[Int]]): FPGrowthModel = {
    var sdata = FPUtil.convert_to_spark_dataframe(data)
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
  // String RDD
  def train(data: RDD[Array[String]],
            minSupport: Double,
            numPartitions: Int,
            treeDepth: Int,
            compressionPoint: Int,
            memOptLevel: Int) 
            (implicit tag: TypeTag[String]) : FPGrowthModel = {
    return new FPGrowth().setMinSupport(minSupport)
                         .setNumPartitions(numPartitions)
                         .setTreeDepth(treeDepth)
                         .setCompressionPoint(compressionPoint)
                         .setMemOptLevel(memOptLevel)
                         .run(data)
  }
  def train(data: RDD[Array[String]],
            minSupport: Double,
            numPartitions: Int,
            treeDepth: Int,
            compressionPoint: Int) 
            (implicit tag: TypeTag[String]) : FPGrowthModel = {
    return train(data, minSupport, numPartitions, 
                 treeDepth, compressionPoint, 0)
  }
  def train(data: RDD[Array[String]],
            minSupport: Double,
            numPartitions: Int,
            treeDepth: Int) (implicit tag: TypeTag[String]) : FPGrowthModel = {
    return train(data, minSupport, numPartitions, treeDepth, 4, 0)
  }
  def train(data: RDD[Array[String]],
            minSupport: Double,
            numPartitions: Int) 
            (implicit tag: TypeTag[String]) : FPGrowthModel = {
    return train(data, minSupport, numPartitions, Int.MaxValue, 4, 0)
  }
  def train(data: RDD[Array[String]],
            minSupport: Double) 
            (implicit tag: TypeTag[String]) : FPGrowthModel =  {
    return train(data, minSupport, 1, Int.MaxValue, 4, 0)
  }
  def train(data: RDD[Array[String]]) 
            (implicit tag: TypeTag[String]) : FPGrowthModel =  {
    return train(data, 0.3, 1, Int.MaxValue, 4, 0)
  }
  // Int RDD
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
    return train(data, minSupport, numPartitions, 
                 treeDepth, compressionPoint, 0)
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
    return train(data, minSupport, numPartitions, 
                 treeDepth, compressionPoint, 0)
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
  // --- spark df input ---
  def train(data: DataFrame,
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
  def train(data: DataFrame,
            minSupport: Double,
            numPartitions: Int,
            treeDepth: Int,
            compressionPoint: Int): FPGrowthModel = {
    return train(data, minSupport, numPartitions, 
                 treeDepth, compressionPoint, 0)
  }
  def train(data: DataFrame,
            minSupport: Double,
            numPartitions: Int,
            treeDepth: Int): FPGrowthModel = {
    return train(data, minSupport, numPartitions, treeDepth, 4, 0)
  }
  def train(data: DataFrame,
            minSupport: Double,
            numPartitions: Int): FPGrowthModel = {
    return train(data, minSupport, numPartitions, Int.MaxValue, 4, 0)
  }
  def train(data: DataFrame,
            minSupport: Double): FPGrowthModel =  {
    return train(data, minSupport, 1, Int.MaxValue, 4, 0)
  }
  def train(data: DataFrame): FPGrowthModel =  {
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
  private def adjust_predictions(pred: FrovedisDataFrame, 
                                 fdata: FrovedisDataFrame): 
                                 org.apache.spark.sql.DataFrame = {
    var spred = pred.to_spark_DF()
    var sdata = fdata.to_spark_DF()
    spred = spred.groupBy("trans_id").agg(collect_set("consequent"))
    sdata = sdata.groupBy("trans_id").agg(collect_set("item"))
    var y_df = sdata.join(spred, sdata("trans_id") === spred("trans_id"), 
                          "outer")
                    .drop("trans_id")
                    .withColumnRenamed("collect_set(item)", "item")
                    .withColumnRenamed("collect_set(consequent)", "predictions")
    val myCol = y_df("predictions")
    return y_df.withColumn("predictions", 
                           when(myCol.isNull, 
                                array().cast("array<integer>"))
                                       .otherwise(myCol))
  }
  def transform(data: DataFrame): DataFrame = {
    val spark = SparkSession.builder().getOrCreate()
    import spark.implicits._
    val window = Window.orderBy(lit('A'))
    val sdata = data.withColumn("trans_id", row_number.over(window))
    val s_df = sdata.select($"trans_id", explode($"item"))
                    .withColumnRenamed("col", "item")
    var fdata = new FrovedisDataFrame(s_df)
    return transform(fdata)
  }
  def transform(data:RDD[Array[String]], dummy: Null = null): DataFrame = {
    var sdata = FPUtil.convert_to_spark_dataframe(data)
    var fdata = new FrovedisDataFrame(sdata)
    return transform(fdata)
  }
  def transform(data: RDD[Array[Int]]): DataFrame = {
    var sdata = FPUtil.convert_to_spark_dataframe(data)
    var fdata = new FrovedisDataFrame(sdata)
    return transform(fdata)
  }
  def transform(fdata: FrovedisDataFrame): DataFrame = {
    val fs = FrovedisServer.getServerInstance()
    val pred = JNISupport.FPTransform(fs.master_node,
                fdata.get(), model_Id)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    var dummy_df = new FrovedisDataFrame(pred.dfptr, pred.names, pred.types)
    return adjust_predictions(dummy_df, fdata)
  }
  def to_spark_model(sc:SparkContext): 
      org.apache.spark.mllib.fpm.FPGrowthModel[Int] = {
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


