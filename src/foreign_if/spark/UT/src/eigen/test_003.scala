package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.matrix._
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.{Vector, Vectors}
import org.apache.spark.mllib.linalg.Matrix
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix

/* 
OBJECTIVE: Testing Frovedis scala wrapper function eigsh(), member of EigenValueDecomposition singleton object with
           FrovedisRowmajorMatrix data
*/

object GenericTest {
  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("GenericTest").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- create dense data --------
    val data = Array(Vectors.dense( 2.0,-1.0, 0.0, 0.0,-1.0, 0.0),
                     Vectors.dense(-1.0, 3.0,-1.0, 0.0,-1.0, 0.0),
                     Vectors.dense( 0.0,-1.0, 2.0,-1.0, 0.0, 0.0),
                     Vectors.dense( 0.0, 0.0,-1.0, 3.0,-1.0,-1.0),
                     Vectors.dense(-1.0,-1.0, 0.0,-1.0, 3.0, 0.0),
                     Vectors.dense( 0.0, 0.0, 0.0,-1.0, 0.0, 1.0))
    
    val rows = sc.parallelize(data)
    val fdata = new FrovedisRowmajorMatrix(rows)
    
    // ------- testing --------
    var isException = false
    try{
      val res = EigenValueDecomposition.eigsh(fdata, k = 3, sigma = 1.0f, tol = 1e-10, 
                                              maxIterations = 300, which = "LM", mode = "normal")
      val evd2 = res.to_spark_result(sc)
      res.release()
    }
    catch {
      case e: Exception => isException = true
    }
    if(isException) println("Failed")
    else println("Passed")
    FrovedisServer.shut_down()
    sc.stop()
  }
}

