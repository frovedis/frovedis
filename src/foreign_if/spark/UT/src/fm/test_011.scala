package test.scala;

import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import org.apache.spark.mllib.evaluation.RegressionMetrics
import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.fm.{FactorizationMachine,FMConfig, FactorizationMachineModel}
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils

object GenericTest {

  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("FactorizationMachineDemo").setMaster("local[1]")
    val sc = new SparkContext(conf)
    
    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample libSVM file at Spark side--------
    var data = MLUtils.loadLibSVMFile(sc, "./input/fm/regression.txt")
    
    // -------- training  with RDD data ---------
    val r_model = FactorizationMachine.train(data,initStdev = 0.1,iter = 100,learnRate = 0.1,optimizer="SGD",
                                           isRegression = true,dim = (true, true, 8),
                                           regParam = (0.0, 0.0, 0.1),batchsize = 2000)

    val f_model = FactorizationMachine.train(data,initStdev = 0.1,iter = 100)


    //-------- test ---------
                  
    val r_predictionAndLabel = data.map(p => (r_model.predict(p.features), p.label))
    val r_accuracy = 1.0 * r_predictionAndLabel.filter(x => x._1 == x._2).count() / data.count()

    val f_predictionAndLabel = data.map(p => (f_model.predict(p.features), p.label))
    val f_accuracy = 1.0 * f_predictionAndLabel.filter(x => x._1 == x._2).count() / data.count()
     
    if(r_accuracy == f_accuracy ) println("Passed")
    else println("Failed")

    // -------- clean-up --------
    r_model.release() 
    f_model.release() 
    FrovedisServer.shut_down()
    sc.stop()
  }
}



