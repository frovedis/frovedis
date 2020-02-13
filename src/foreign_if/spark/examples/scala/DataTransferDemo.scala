package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils
import org.apache.spark.mllib.regression.LabeledPoint
import org.apache.spark.mllib.linalg.{Vectors,Vector}
import org.apache.log4j.{Level, Logger}
    
object DataTransferDemo {
  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)

    // -------- configurations --------
    val conf = new SparkConf().setAppName("DataTransferDemo").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // --- with sparse data loaded from file ---
    // -------- data loading from sample libSVM file at Spark side--------
    val s_data = MLUtils.loadLibSVMFile(sc, "./input/libSVMFile.txt")

    // returns MemPair java object containing crs_matrix and dvector heads
    println("converting spark sparse RDD[LabeledPoint] => FrovedisLabeledPoint")
    val fd = new FrovedisLabeledPoint(s_data)
    fd.debug_print() 
    fd.release()

    // --- with dense data created manually ---
    val arr = new Array[LabeledPoint](4)  // allocate an array of LabelPoints with size 4 
    // filling array elements manually with dense Vectors for features
    arr(0) = new LabeledPoint(1.0,Vectors.dense(1.0,0.0,0.0,1.0,1.0,0.0))
    arr(1) = new LabeledPoint(0.0,Vectors.dense(1.0,0.0,1.0,0.0,1.0,0.0))
    arr(2) = new LabeledPoint(0.0,Vectors.dense(1.0,0.0,0.0,1.0,0.0,1.0))
    arr(3) = new LabeledPoint(1.0,Vectors.dense(1.0,0.0,0.0,1.0,1.0,0.0))
    // scattering the array to available spark workers to make RDD[LabeledPoint]
    val s_data2 = sc.parallelize(arr) // it is same as feliss::make_dvector_scatter()
    // returns MemPair java object containing colmajor_matrix and dvector heads
    println("converting spark dense RDD[LabeledPoint] => FrovedisLabeledPoint")
    val fd2 = new FrovedisLabeledPoint(s_data2)
    fd2.debug_print() 
    fd2.release()

    // returns MemPair java object containing only crs_matrix
    println("converting spark sparse RDD[Vector] => FrovedisSparseData")
    val vec = s_data.map(_.features)
    val fd3 = new FrovedisSparseData(vec)
    fd3.debug_print() 
    fd3.release()
    
    FrovedisServer.shut_down()
    sc.stop()
  }
}
