package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.feature.StandardScaler
import org.apache.log4j.{Level, Logger}
import org.apache.spark.rdd.RDD
import org.apache.spark.sql.SparkSession
import org.apache.spark.mllib.linalg.Vectors
import org.apache.spark.mllib.util.MLUtils

object StandardScalerDemo {
  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)

    // -------- configurations --------
    val spark = SparkSession.builder.appName("StandardScalerExample").master("local[2]").getOrCreate()
    val sc = spark.sparkContext      
            
    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- RDD loading from sample file at Spark side--------
    val rdd_vec = sc.textFile("./input/kmeans_data.txt")
                    .map(x => Vectors.dense(x.split(' ').map(_.toDouble)))

    // -------- Dataset loading from sample file at Spark side--------//  
    //val dataset = spark.read.format("libsvm").load("./input/libSVMFile.txt") //DataFrame
      
    val ss = new StandardScaler().setWithMean(true).setWithStd(true).setSamStd(true)
    val ssmodel = ss.fit(rdd_vec)
    
    //Get mean and stddev
    val means = ssmodel.mean
    println("Mean:")  
    means.foreach(println)
    val stds = ssmodel.stddev
    println("\nStandard Deviation:")  
    stds.foreach(println)
      
    //Transform  
    ssmodel.transform(rdd_vec)
    ssmodel.inverse_transform(rdd_vec)

    //Release  
    ssmodel.release()

    FrovedisServer.shut_down()
    sc.stop()
  }
}

