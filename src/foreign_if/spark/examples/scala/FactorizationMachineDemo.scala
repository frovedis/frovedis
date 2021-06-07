package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.fm.{FactorizationMachine,FactorizationMachineModel}
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils
import org.apache.log4j.{Level, Logger}
import org.apache.spark.sql.SparkSession

object FactorizationMachineDemo {
  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)

    // -------- configurations --------
    val conf = new SparkConf().setAppName("FactorizationMachineDemo").setMaster("local[2]")
    val sc = new SparkContext(conf)

    val spark = SparkSession.builder.appName("KmeansExample").getOrCreate()      

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample libSVM file at Spark side--------
    var data = MLUtils.loadLibSVMFile(sc, "./input/libSVMFile.txt")
    val dataset = spark.read.format("libsvm").load("./input/libSVMFile.txt")  

    // -------- training with all parameters (RDD data/dataset)--------
    val model = FactorizationMachine.train(dataset,
                                           initStdev = 0.1,
                                           iter = 100,
                                           learnRate = 0.01, optimizer="SGD", 
                                           isRegression = false,dim = (true, true, 2),
                                           regParam = (0.0, 0.0, 0.0), 
                                           batchsize = 1)

    // -------- prediction --------
    //val X = data.map(_.features)  
    //model.predict(X).collect().foreach(println)  
    val predictionAndLabel = data.map(p => (model.predict(p.features), p.label))
    val accuracy = 1.0 * predictionAndLabel.filter(x => x._1 == x._2).count() / data.count()
    println("Test accuracy: " + accuracy)
    // -------- load/save --------
    model.save(sc, "./out/FactorizationMachineModel")

    // -------- clean-up --------
    model.release() 
   
    FrovedisServer.shut_down()
    sc.stop()
  }
}
