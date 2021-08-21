package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.matrix._
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.{Vector, Vectors}
import org.apache.spark.mllib.linalg.Matrix
import com.nec.frovedis.exrpc.FrovedisSparseData


//OBJECTIVE: Testing Frovedis scala wrapper function eigsh(), a member of EigenValueDecomposition singleton object with FrovedisSparseData


object GenericTest {
  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("GenericTest").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- create sparse data --------
    val data = Array(Vectors.sparse(6, Seq((0, 2.0), (1, -1.0), (4, -1.0))),
                     Vectors.sparse(6, Seq((0, -1.0), (1, 3.0), (2, -1.0), (4, -1.0))),
                     Vectors.sparse(6, Seq((1, -1.0), (2, 2.0), (3, -1.0))),
                     Vectors.sparse(6, Seq((2, -1.0), (3, 3.0), (4, -1.0), (5, -1.0))),
                     Vectors.sparse(6, Seq((0, -1.0), (1, -1.0), (3, -1.0), (4, 3.0))),
                     Vectors.sparse(6, Seq((3, -1.0), (5, 1.0))))

    val rows = sc.parallelize(data)
    val fdata = new FrovedisSparseData(rows)
    
    // ------- testing --------
    var isException = false
    try{
      val res = EigenValueDecomposition.eigsh(fdata, k = 3, sigma = 0.0f, tol = 1e-10, 
                                              maxIterations = 300, which = "LM", mode = "normal")
      val evd2 = res.to_spark_result(sc)
      res.release()
    }
    catch {
      case e: Exception => isException = true
    }
    if(isException) println("Passed")
    else println("Failed")
    FrovedisServer.shut_down()
    sc.stop()
  }
}

