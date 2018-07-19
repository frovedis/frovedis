package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils

object DataTransferDemo {
  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("DataTransferDemo").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample libSVM file at Spark side--------
    val s_data = MLUtils.loadLibSVMFile(sc, "./input/libSVMFile.txt")

    // returns MemPair java object containing crs_matrix and dvector heads
    println("converting spark RDD[LabeledPoint] => FrovedisLabeledPoint")
    val fd = new FrovedisLabeledPoint(s_data)
    fd.debug_print() 
    fd.release()
    
    // returns MemPair java object containing only crs_matrix
    println("converting spark RDD[Vector] => FrovedisSparseData")
    val vec = s_data.map(_.features)
    val fd2 = new FrovedisSparseData(vec)
    fd2.debug_print() 
    fd2.release()
    
    FrovedisServer.shut_down()
    sc.stop()
  }
}
