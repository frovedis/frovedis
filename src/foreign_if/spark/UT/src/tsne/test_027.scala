package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.manifold.TSNE
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.linalg.Vectors
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix

// OBJECTIVE: To test Frovedis scala wrapper of TSNE.run with FrovedisRowmajorMatrix

object GenericTest {
  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("GenericTest").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample file at Spark side--------
    val s_data = sc.textFile("./input/tsne_data.txt")
                   .map(s => Vectors.dense(s.split(' ').map(_.toDouble)))
    val fdata = new FrovedisRowmajorMatrix(s_data)

    // ------- training --------
    var isException = false
    try{
      val algo = new TSNE().setInit("random")
      val m1 = algo.run(fdata)
      m1.release()
    }
    catch {
      case e: Exception => isException = true
    }
    if(isException) println("Failed")
    else println("Passed")

    fdata.release()
    FrovedisServer.shut_down()
    sc.stop()
  }
}
