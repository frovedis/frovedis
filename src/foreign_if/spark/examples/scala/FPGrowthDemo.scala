package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.fpm.{FPGrowth,FPGrowthModel}
import org.apache.spark.sql.SparkSession
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.rdd.RDD
import org.apache.log4j.{Level, Logger}

object FPGrowthDemo {
  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)
    
    // -------- configurations --------
    val conf = new SparkConf().setAppName("FPGrowthDemo").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line 
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample transaction file at Spark side--------
    var s_data = sc.textFile("./input/transaction.txt")
    val tr: RDD[Array[Int]] = s_data.map(s => s.trim.split(' ').map(x => x.toInt))

    // -------- training --------
    val fpgrowth = new FPGrowth().setMinSupport(0.2)
    val model = fpgrowth.run(tr)
    
    // -------- short-cut of the above call --------
    //val model = FPGrowth.train(tr, 0.6)
    model.debug_print()

    val rule = model.generateAssociationRules(0.5)
    rule.debug_print()

    // -------- clean-up --------
    model.release()
    FrovedisServer.shut_down()
    sc.stop()
 } 
}
  
