package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.matrix.RowMatrixUtils._
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.{Vector, Vectors}
import org.apache.spark.mllib.linalg.Matrix
import org.apache.spark.mllib.linalg.SingularValueDecomposition
import org.apache.spark.mllib.linalg.distributed.RowMatrix
import org.apache.log4j.{Level, Logger}

object EVDDemo {
  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)

    // -------- configurations --------
    val conf = new SparkConf().setAppName("EVDDemo").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    val data = Array(Vectors.dense( 2.0,-1.0, 0.0, 0.0,-1.0, 0.0),
                     Vectors.dense(-1.0, 3.0,-1.0, 0.0,-1.0, 0.0),
                     Vectors.dense( 0.0,-1.0, 2.0,-1.0, 0.0, 0.0),
                     Vectors.dense( 0.0, 0.0,-1.0, 3.0,-1.0,-1.0),
                     Vectors.dense(-1.0,-1.0, 0.0,-1.0, 3.0, 0.0),
                     Vectors.dense( 0.0, 0.0, 0.0,-1.0, 0.0, 1.0))

    val rows = sc.parallelize(data)
    val mat: RowMatrix = new RowMatrix(rows)

    // (SPARK WAY) Compute the top 3 eigen values and corresponding eigen vectors
    // Here,when k < RDD.numCols()/3, then mode = 'local-eigs' to compute eigenvalues and eigenvectors
    val evd1: SingularValueDecomposition[RowMatrix, Matrix] = mat.computeSVD(3)
    val s1: Vector = evd1.s            // eigenvalues
    val V1: Matrix = evd1.V            // eigenvector
    println("spark::eigenvalues ")
    println(s1.toString)
    println("spark::eigenvectors ")
    println(V1.toString)

    // (FROVEDIS WAY) Compute the top 3 eigen values and corresponding eigen vectors
    // API as a member method of Spark RowMatrix
    val res = mat.eigshUsingFrovedis(3) // compute eigenvalues and eigenvectors always
    // can also be called as a method of RowMatrixUtils
    // import com.nec.frovedis.matrix.RowMatrixUtils
    // val res = RowMatrixUtils.eigsh(mat,3)
    val evd2 = res.to_spark_result(sc)
    val s2: Vector = evd2.s            // eigenvalues
    val V2: Matrix = evd2.V            // eigenvectors
    println("frovedis::eigenvalues ")
    println(s2.toString)
    println("frovedis::eigenvectors ")
    println(V2.toString)
    res.release()

    FrovedisServer.shut_down()
    sc.stop()
  }
}
