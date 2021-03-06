package test.scala;

import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import org.apache.spark.mllib.evaluation.RegressionMetrics
import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.fm.{FactorizationMachine,FMConfig, FactorizationMachineModel}
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils
import org.apache.spark.ml.evaluation.MulticlassClassificationEvaluator

object GenericTest {

  //testing accuracy with RDD and FrovedisLabeledPoint data

  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("FactorizationMachineDemo").setMaster("local[1]")
    val sc = new SparkContext(conf)
    
    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample libSVM file at Spark side--------
    var data = MLUtils.loadLibSVMFile(sc, "./input/fm/libSVMFile.txt")
    
    // -------- training  with RDD data ---------
    val r_model = FactorizationMachine.train(data,initStdev = 10.1,iter = 100,
                                             stepSize = 0.01, optimizer="SGD",
                                             isRegression = true,
                                             fitIntercept = true,
                                             fitLinear = true,
                                             factorSize = 8,
                                             regParam = (1.2, 1.2, 1.1),
                                             miniBatchFraction = 0.7,
                                             seed = 1)

    // ------------- training with Frovedis data---------
     val fdata = new FrovedisLabeledPoint(data)
     val f_model = FactorizationMachine.train(fdata,initStdev = 10.1,iter = 100,
                                             stepSize = 0.01, optimizer="SGD",
                                             isRegression = true,
                                             fitIntercept = true,
                                             fitLinear = true,
                                             factorSize = 8,
                                             regParam = (1.2, 1.2, 1.1),
                                             miniBatchFraction = 0.7,
                                             seed = 1)
    //-------- test ---------
                  
    val r_predictionAndLabel = data.map(p => (r_model.predict(p.features), p.label))
    val r_accuracy = 1.0 * r_predictionAndLabel.filter(x => x._1 == x._2).count() / data.count()
     
    val f_predictionAndLabel = data.map(p => (f_model.predict(p.features), p.label))
    val f_accuracy = 1.0 * f_predictionAndLabel.filter(x => x._1 == x._2).count() / data.count()

    if(r_accuracy == f_accuracy) println("Passed")
    else println("Failed")   

    // -------- clean-up --------
    r_model.release() 
    f_model.release()
    FrovedisServer.shut_down()
    sc.stop()
  }
}



