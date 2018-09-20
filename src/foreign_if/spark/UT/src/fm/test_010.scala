package test.scala;

import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.fm.{FactorizationMachine,FMConfig, FactorizationMachineModel}
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils
import java.io.FileReader
import java.io.FileNotFoundException
import java.io.IOException
import sys.process._

// Objective : Test of Load() API

object GenericTest {
  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("FactorizationMachineDemo").setMaster("local[1]")
    val sc = new SparkContext(conf)
    
    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample libSVM file at Spark side--------
    var data = MLUtils.loadLibSVMFile(sc, "./input/fm/libSVMFile.txt")
    
    // -------- training  with RDD data ---------
    val model = FactorizationMachine.train(data,initStdev = 0.1,iter = 100,learnRate = 0.1,optimizer="SGD",
                                           isRegression = false,dim = (true, true, 8), 
                                           regParam = (0.0, 0.0, 0.1),batchsize = 2000)

    var os_stat = "rm -rf ./out/FMModel" .!
    model.save(sc, "./out/FMModel")
    var isFileMissing = false
    try{
         val f = new FileReader("./out/FMModel")
      } catch {
         case ex: FileNotFoundException =>{
            isFileMissing = true
         }
      }

    if(isFileMissing) println("Failed")
    else println("Passed")

    // val model2 = FactorizationMachineModel.load(sc, "./out/FMModel") // currently not supported (kept for future)

    // -------- clean-up --------
    os_stat = "rm -rf ./out/FMModel" .!
    model.release() 
    FrovedisServer.shut_down()
    sc.stop()
  }
}



